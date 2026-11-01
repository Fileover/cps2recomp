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

    // The player is free to do anything: standing, walking or crouching, and not attacking
    bool CanAct(uint32_t player) {
        const uint8_t state = Ram8(player + kState);
        return (state == kStateStand || state == kStateCrouch) && Ram8(player + kAttack) == 0;
    }

    // 02A220: a player's fight step (02A200 runs it for player 1, then player 2, A6 the record), where it goes on by
    // its state; seen as it starts, the record is as the last step left it, which is what the picture showed
    bool PlayerStep(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        const int n = PlayerOf(player);
        if (n < 0 || Ram8(player + 3) != 2) {
            return false;
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
    d.stageChoices = modes::StageChoices({"The game's choice"});
    d.dummy.downAfter = 34;
    session::Register(d);
    AddHook(HookAt(kPlayerStep), "PlayerStep", PlayerStep);
}

} // namespace cps2recomp
