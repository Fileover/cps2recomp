// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors: the training session is the toolkit's (fight/session.h); this says where the game keeps what it
// needs, and tells it from the game's own code what the players do: each player's step of a fight (frame data). As
// Pocket Fighter's.
#include "fight.h"
#include "alternative.h"
#include "nwarr.h"
#include "session.h"

#include <iterator>

namespace cps2recomp {

bool RingOutStep(M68K &c); // battle.cpp

namespace {
    using namespace nwarr;

    // Bytes of the player's choice: 2 once it has chosen its character (029012), and 4 once it has chosen normal or
    // automatic guard (028EBC), which the board asks only with its Mode select on (A5+203). Dumps before and after.
    constexpr uint32_t kChosen = 0x4A7, kModeChosen = 0x4B9, kModeSelect = 0xFF8203;
    bool Chosen(int n) {
        const uint32_t player = Player(n);
        return Ram8(player + kChosen) >= 2 && (Ram8(kModeSelect) == 0 || Ram8(player + kModeChosen) >= 4);
    }

    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // The boxes (0C0DC2 on, the hit test once a frame): a record's +5C points to its box tables, a word at the start for
    // each kind the distance to that kind's table; the frame (+1C) names a box of each kind by a byte: +10, +11, +12 the
    // three hurt boxes (kinds 0, 2, 4: 0C0FB6 on), +13 a fourth kind (6, 0C0FA4), +14 the attack (kind 8; 0C0E5A). A box
    // is words: across from the record's X (turned with it, +09), half its width, up from its Y, half its height
    // (0C154E); a hurt box 8 bytes, an attack 16. What the players throw is in records of the same layout, three a
    // player (A5+2CEE, 2DAE, 2E6E player 1's, A5+286E, 292E, 29EE player 2's: 0C0DEE on), in use while the first byte is
    // above 0 and +04 is 2 (0C0EB0).
    constexpr uint32_t kComputerScript = 0x220, kComputerStarts = 0x068CFC;
    constexpr uint32_t kBoxTables = 0x5C, kFrameBoxes = 0x10, kFrameAttack = 0x14;
    constexpr uint32_t kThrown[6] = {0xFFACEE, 0xFFADAE, 0xFFAE6E, 0xFFA86E, 0xFFA92E, 0xFFA9EE};

    void AddBox(M68K &c, std::vector<fight::StageBox> &boxes, fight::BoxKind kind, uint32_t record, int which, bool thrown) {
        const uint32_t frame = Ram32(record + kFrame);
        const uint8_t box = static_cast<uint8_t>(c.Read(frame + kFrameBoxes + static_cast<uint32_t>(which), 0));
        if (!box) {
            return;
        }
        const uint32_t tables = Ram32(record + kBoxTables);
        const uint32_t at = tables + static_cast<uint32_t>(static_cast<int16_t>(c.Read(tables + static_cast<uint32_t>(which) * 2, 1))) +
                            box * (which == 4 ? 16u : 8u);
        const int16_t across = static_cast<int16_t>(c.Read(at, 1));
        const int x = static_cast<int16_t>(Ram16(record + kX)) + (Ram8(record + kFacesLeft) ? -across : across);
        const int y = static_cast<int16_t>(Ram16(record + kY)) + static_cast<int16_t>(c.Read(at + 4, 1)) - kGround;
        const int halfWidth = static_cast<int16_t>(c.Read(at + 2, 1)), halfHeight = static_cast<int16_t>(c.Read(at + 6, 1));
        if (halfWidth > 0 && halfHeight > 0) {
            boxes.push_back({kind, x, y, halfWidth, halfHeight, thrown});
        }
    }

    std::vector<fight::StageBox> Boxes(Machine &machine) {
        M68K &c = machine.cpu;
        std::vector<fight::StageBox> boxes;
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            if (Ram8(player + 3) != 2) {
                continue;
            }
            for (int which = 0; which < 3; which++) {
                AddBox(c, boxes, fight::BoxKind::Hurt, player, which, false);
            }
            AddBox(c, boxes, fight::BoxKind::Attack, player, 4, false);
        }
        for (const uint32_t thrown : kThrown) {
            if (static_cast<int8_t>(Ram8(thrown)) > 0 && Ram8(thrown + 4) == 2) {
                AddBox(c, boxes, fight::BoxKind::Hurt, thrown, 0, true);
                AddBox(c, boxes, fight::BoxKind::Attack, thrown, 4, true);
            }
        }
        return boxes;
    }

    // The player is free to do anything: standing, walking or crouching, and not attacking
    bool CanAct(uint32_t player) {
        const uint8_t state = Ram8(player + kState);
        return (state == kStateStand || state == kStateCrouch) && Ram8(player + kAttack) == 0;
    }

    // 02A220: a player's fight step (02A200 runs it for player 1, then player 2, A6 the record), where it goes on by
    // its state; seen as it starts, the record is as the last step left it, which is what the picture showed
    bool PlayerStep(M68K &c) {
        if (RingOutStep(c)) { // (the ring out's own movements: battle.cpp)
            Return(c);
            return true;
        }
        const uint32_t player = c.a[6] & 0xFFFFFF;
        const int n = PlayerOf(player);
        if (n < 0 || Ram8(player + 3) != 2) {
            return false;
        }
        // A player handed to the computer in the middle of a fight (the modes do: battle.h) has no script of the
        // computer's yet, which the game gives only to one made for the computer (068CFC: 068D14 sets the pointers at
        // +220 and +224 from the character's table at 06C176); with none, the computer's play runs off into data. It
        // is given one here, as the game would have.
        if (!Ram8(player + kPerson) && !Ram32(player + kComputerScript)) {
            Call(c, kComputerStarts);
        }
        fight::StepView view;
        const uint8_t state = Ram8(player + kState);
        view.control = state == kStateReel                         ? fight::StepView::Reeling :
                       state <= kStateGuard                         ? fight::StepView::Own :
                                                                      fight::StepView::Held;
        view.frozen = Ram8(player + kHitStop) != 0;
        view.canAct = CanAct(player);
        view.attacking = Ram8(player + kAttack) != 0;
        view.jumping = state == kStateJump;
        view.attackOut = c.Read(Ram32(player + kFrame) + kFrameAttack, 0) != 0;
        session::Step(n, view);
        return false;
    }

    // What the game read off the panel for a player this frame, as Button bits
    uint16_t PanelHeld(int n) {
        const uint16_t word = Ram16(kControls[n]);
        uint16_t held = word & 0x0F;
        held |= static_cast<uint16_t>(((word >> 4) & 0x07) << 4); // punches
        held |= static_cast<uint16_t>(((word >> 8) & 0x07) << 7); // kicks
        return held;
    }

    fight::MoveLog::Start MoveStart(int n) {
        const uint32_t player = Player(n);
        const uint8_t who = Ram8(player + kCharacter);
        fight::MoveLog::Start start;
        start.character = who < kCharacters ? kCharacterNames[who] : "?";
        start.input = fight::Notation(PanelHeld(n), Ram8(player + kFacesLeft) != 0);
        start.air = Ram8(player + kState) == kStateJump || Ram16(player + kY) != kGround;
        start.id = Ram32(player + kFrame);
        return start;
    }
} // namespace

void RegisterTraining() {
    session::Description d;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.health = kHealth;
    d.healthFull = kHealthFull;
    d.clock = kRoundClock;
    d.clockFull = 0x99; // (the clock is BCD: 99 is 0x99)
    d.fighting = [] {
        return Ram8(kPlayersIn) != 0 && Ram16(kMode) == kGameRunning && Ram16(kStep) == kGameMatch && Ram16(kStep2) == kRoundFought;
    };
    d.cameraX = kCameraX;
    d.cameraY = kCameraY;
    d.cameraCopies = {kCameraX};
    d.startX[0] = kStartX[0];
    d.startX[1] = kStartX[1];
    d.stageLeft = kStageLeft;
    d.stageRight = kStageRight;
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.chosen = Chosen;
    d.panelHeld = PanelHeld;
    d.facingLeft = [](int n) { return Ram8(Player(n) + kFacesLeft) != 0; };
    d.moveStart = MoveStart;
    d.boxes = Boxes;
    d.pictureFloor = 196; // (the floor's line in the picture with the camera at its lowest: the players' feet)
    d.stageChoices = modes::StageChoices({"The game's choice"});
    d.dummy.downAfter = 34;
    session::Register(d);
    AddHook(HookAt(kPlayerStep), "PlayerStep", PlayerStep);
}

} // namespace cps2recomp
