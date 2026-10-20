// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: the training session is the toolkit's (fight/session.h); this says where the game keeps
// what it needs, and tells it from the game's own code what the players do: each player's step of a fight (frame
// data), a hit landing, and the boxes as the game's hit tests read them. A side's two characters take turns in the
// first two records (a tag or a knockout swaps the records: xmvsf.h), so the session follows the character on point;
// the one resting is looked after here (its health refilled with the point character's).
#include "alternative.h"
#include "fight.h"
#include "session.h"
#include "xmvsf.h"

#include <cstdio>

namespace cps2recomp {

namespace {
    using namespace xmvsf;

    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // The character is free to do anything: in charge of itself, and standing, crouching, walking or guarding
    bool CanAct(uint32_t player) {
        const uint8_t action = Ram8(player + kAction);
        return Ram16(player + kControl) == 0 && (action == kActionStand || action == kActionCrouch || action == kActionWalk ||
                                                 action == kActionGuard || action == kActionCrouchGuard);
    }

    bool AttackOut(uint32_t player) {
        return Ram8(player + kCanHit) != 0 &&
               ((Ram16(player + kAttackBoxes) & 0x1FFF) != 0 || (Ram16(player + kAttackBoxes + 2) & 0x1FFF) != 0);
    }

    uint8_t g_hitStop[2] = {0, 0}; // each point character's freeze as it last stepped

    // 00C0D8: a character's step, as the game goes on to what its record's state says (once a step for each of the
    // four, the point characters first of their sides); state 2 is fighting. Seen as the step starts, the record is as
    // the last step left it, which is what the picture showed and the hit tests tested.
    bool PlayerStep(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        const int n = PlayerOf(player);
        if (n < 0 || Ram16(player + 0x04) != 2) {
            return false;
        }
        // A hit landing freezes both characters: the one hit is told to the frame data as hit, or as guarding
        const uint8_t stop = Ram8(player + kHitStop);
        if (stop != 0 && g_hitStop[n] == 0 && !AttackOut(player)) {
            const uint8_t action = Ram8(player + kAction);
            session::Contact(n, action == kActionGuard || action == kActionCrouchGuard);
        }
        g_hitStop[n] = stop;
        fight::StepView view;
        view.control = Ram16(player + kControl) == 0 ? fight::StepView::Own : fight::StepView::Reeling;
        view.frozen = stop != 0;
        view.canAct = CanAct(player);
        view.attacking = (Ram8(player + kAttackPhase) & 0xC0) != 0;
        view.jumping = Ram8(player + kAction) == kActionJump || Ram16(player + kY) != kGround;
        view.attackOut = AttackOut(player);
        session::Step(n, view);
        return false;
    }

    // What the game read off the panel for a player this frame, as Button bits
    uint16_t PanelHeld(int n) {
        const uint16_t word = Ram16(kPanel[n]);
        return static_cast<uint16_t>((word & 0x7F) | (((word >> 8) & 0x07) << 7));
    }

    bool FacingLeft(int n) {
        return (Ram8(Player(n) + kFlip) & 1) == 0;
    }

    const char *CharacterName(uint32_t player) {
        const unsigned who = Ram16(player + kCharacter) / 2;
        return who < kCharacters ? kCharacterNames[who] : "?";
    }

    fight::MoveLog::Start MoveStart(int n) {
        const uint32_t player = Player(n);
        fight::MoveLog::Start start;
        start.character = CharacterName(player);
        start.input = fight::Notation(PanelHeld(n), FacingLeft(n));
        start.air = Ram16(player + kY) != kGround;
        start.id = Ram32(player + kFrame);
        return start;
    }

    // The boxes go to the session in its own terms: across as the picture's place plus the camera's word, and up, the
    // camera's word down being the picture's bottom (fight::ToPicture puts them back)
    constexpr int kFloorLine = 223;

    void AddBox(M68K &c, std::vector<fight::StageBox> &boxes, fight::BoxKind kind, uint32_t object, uint32_t box, bool thrown) {
        const int halfWidth = static_cast<int>(c.Read(box + 2, 1)), halfHeight = static_cast<int>(c.Read(box + 6, 1));
        if (halfWidth == 0 || halfHeight == 0) {
            return;
        }
        const uint8_t flip = Ram8(object + kFlip);
        const int offsetX = static_cast<int16_t>(c.Read(box + 0, 1)), offsetY = static_cast<int16_t>(c.Read(box + 4, 1));
        const int x = static_cast<int16_t>(Ram16(object + kX)) + ((flip & 1) ? -offsetX : offsetX);
        const int y = static_cast<int16_t>(Ram16(object + kY)) + ((flip & 2) && kind != fight::BoxKind::Push ? -offsetY : offsetY);
        // (the picture's place: across x - the camera's word - kPictureLeft, down y - the camera's word - 0x0F)
        const uint32_t camera = CameraRecord();
        const int cameraX = static_cast<int16_t>(Ram16(camera + kCameraX)), cameraY = static_cast<int16_t>(Ram16(camera + kCameraY));
        const int pictureY = y - cameraY - 0x0F;
        boxes.push_back({kind, x - kPictureLeft, kFloorLine - pictureY + cameraY, halfWidth, halfHeight, thrown});
    }

    void ObjectBoxes(M68K &c, std::vector<fight::StageBox> &boxes, uint32_t object, bool thrown, uint16_t character) {
        const uint32_t table = Ram32(object + kBoxTable);
        if (Ram8(object + kCanBeHit) != 0) {
            for (uint32_t n = 0; n < 4; n++) {
                if (const uint16_t id = Ram16(object + kHurtBoxes + 2 * n) & 0x1FFF) {
                    AddBox(c, boxes, fight::BoxKind::Hurt, object, table + id * 8u, thrown);
                }
            }
        }
        if (Ram8(object + kCanHit) != 0) {
            for (uint32_t n = 0; n < 2; n++) {
                const uint16_t id = Ram16(object + kAttackBoxes + 2 * n);
                // (the high bit: how near a throw reaches, drawn as an outline, and only while the character is free)
                if ((id & 0x1FFF) && (!(id & 0x8000) || Ram16(object + kControl) == 0)) {
                    AddBox(c, boxes, (id & 0x8000) ? fight::BoxKind::ThrowRange : fight::BoxKind::Attack, object,
                           table + (id & 0x1FFFu) * 8u, thrown);
                }
            }
        }
        if (!thrown) {
            const uint8_t push = Ram8(object + kPushBox);
            AddBox(c, boxes, fight::BoxKind::Push, object, c.Read(kPushTable + push * 2u, 2) + character * 4u, false);
        }
    }

    std::vector<fight::StageBox> Boxes(Machine &machine) {
        M68K &c = machine.cpu;
        std::vector<fight::StageBox> boxes;
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            if (Ram16(player + 0x04) != 2) {
                continue;
            }
            ObjectBoxes(c, boxes, player, false, Ram16(player + kCharacter));
            // What the player has thrown: the list's records, newest first, to the first that is not one
            const uint32_t list = kThrownList + static_cast<uint32_t>(n) * kThrownListSpace;
            for (uint32_t i = 1; i < 32; i++) {
                const uint32_t thrown = Ram32(list - 4 * i) & 0xFFFFFF;
                if (thrown < kPlayer1 + 4 * kPlayerSize || thrown >= 0xFFFF00) {
                    break;
                }
                ObjectBoxes(c, boxes, thrown, true, 0);
            }
        }
        return boxes;
    }

    // The game's own aids: the gauge kept full, and the partner's health refilled with the point character's
    void Hud(Machine &, std::vector<HudLine> &, std::vector<HudBox> &) {
        for (int n = 0; n < 2; n++) {
            const uint32_t player = Player(n);
            if (FeatureOn("gauge_refill")) {
                SetRam8(player + kGaugeLevel, kGaugeFull);
                SetRam16(player + kGaugeBar, kGaugeBarFull);
            }
            if (FeatureOn("health_refill")) {
                const uint32_t partner = player + kPartners;
                if (Ram16(partner + kHealth) != 0 && Ram16(partner + kHealth) < kHealthFull) {
                    SetRam16(partner + kHealth, kHealthFull);
                    SetRam16(partner + kRedHealth, kHealthFull);
                }
            }
        }
    }

    void Ends(int &stageLeft, int &stageRight, int &cameraLeft, int &cameraRight) {
        const uint32_t camera = CameraRecord();
        cameraLeft = static_cast<int16_t>(Ram16(camera + kCameraLeft));
        cameraRight = static_cast<int16_t>(Ram16(camera + kCameraRight));
        stageLeft = cameraLeft + kPictureLeft + kWall;
        stageRight = cameraRight + kPictureLeft + 384 - kWall;
    }
} // namespace

void RegisterTraining() {
    session::Description d;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.xLong = true;
    d.health = kHealth;
    d.healthCeiling = kRedHealth;
    d.healthFull = kHealthFull;
    d.clock = kRoundClock;
    d.clockFull = 0x99; // (in BCD: a round is being fought while it is 1 to 99)
    d.cameraXAt = [] { return CameraRecord() + kCameraX; };
    d.cameraYAt = [] { return CameraRecord() + kCameraY; };
    d.ends = Ends;
    d.startX[0] = kStartX[0];
    d.startX[1] = kStartX[1];
    d.pictureFloor = kFloorLine;
    d.fighting = [] {
        const uint8_t clock = Ram8(kRoundClock);
        return Ram8(kFight) == kFighting && clock > 0 && clock <= 0x99;
    };
    d.chosen = [](int n) { return Ram8(kChoosing + static_cast<uint32_t>(n) * kChoosingSize) >= kChoosingDone; };
    d.panelHeld = PanelHeld;
    d.facingLeft = FacingLeft;
    d.moveStart = MoveStart;
    d.boxes = Boxes;
    d.stageChoices = modes::StageChoices({"The game's choice"});
    d.hud = Hud;
    d.ownAids = {"gauge_refill"};
    d.dummy.downAfter = 34;
    Game &game = TheGame();
    game.features.push_back({"gauge_refill", modes::kTraining, "Gauge refills", "Both players' hyper combo gauge kept full."});
    session::Register(d);
    AddHook(HookAt(0x00C0D8), "PlayerStep", PlayerStep);
}

} // namespace cps2recomp
