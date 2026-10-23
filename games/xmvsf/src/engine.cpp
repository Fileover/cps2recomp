// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: the game described to the toolkit's code for the engine of Capcom's Marvel games
// (fight/marvel.h): the training aids' view of the players and their boxes, the ring out's movements, widescreen and
// the model viewer are done there, from where this game keeps what they need (xmvsf.h).
#include "marvel.h"
#include "xmvsf.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace xmvsf;

    // A frame put in a player's record as 001AE6 puts it: a word for its picture, then (after its time) the picture,
    // two words more, and the number of its set of boxes in the character's table of them (16 bytes a set, the long at
    // the record's +68)
    void CopyFrame(Machine &machine, uint32_t player, uint32_t frame) {
        const auto long_ = [&](uint32_t at) { return (static_cast<uint32_t>(machine.Read16(at)) << 16) | machine.Read16(at + 2); };
        SetRam16(player + 0x3C, machine.Read16(frame));
        SetRam32(player + 0x40, long_(frame + 0x04));
        SetRam32(player + 0x62, long_(frame + 0x08));
        SetRam16(player + 0x66, machine.Read16(frame + 0x0C));
        const uint32_t boxes = Ram32(player + 0x68) + static_cast<uint32_t>(machine.Read16(frame + 0x0E)) * 16;
        for (uint32_t at = 0; at < 16; at += 4) {
            SetRam32(player + 0x70 + at, long_(boxes + at));
        }
    }
} // namespace

void DescribeEngine() {
    marvel::Engine e;
    e.player1 = kPlayer1;
    e.playerSize = kPlayerSize;
    e.records = 4; // (the point characters first, their partners after)
    e.x = kX;
    e.y = kY;
    e.speedX = kSpeedX;
    e.speedY = kSpeedY;
    e.pull = kPullY;
    e.control = kControl;
    e.frame = kFrame;
    e.flip = kFlip;
    e.character = kCharacter;
    e.action = kAction;
    e.attackPhase = kAttackPhase;
    e.hitStop = kHitStop;
    e.boxTable = kBoxTable;
    e.attackBoxes = kAttackBoxes;
    e.hurtBoxes = kHurtBoxes;
    e.pushBox = kPushBox;
    e.canHit = kCanHit;
    e.canBeHit = kCanBeHit;
    e.pushTable = kPushTable;
    e.thrownList = kThrownList;
    e.thrownListSpace = kThrownListSpace;
    e.panel[0] = kPanel[0];
    e.panel[1] = kPanel[1];
    e.characterNames.assign(std::begin(kCharacterNames), std::end(kCharacterNames));
    e.stageNames.assign(std::begin(kStageNames), std::end(kStageNames));
    e.cameraRecord = CameraRecord;
    e.pictureLeft = kPictureLeft;
    e.wall = kWall;
    e.planes = 0x01E7EC; // (longs by a character's byte +30: the camera's word of the plane it stands in)
    e.ground = [] { return static_cast<int>(kGround); };
    e.cameraEnds = CameraEnds;
    e.stage = kStage;
    e.cameraWay = kCameraWay;
    e.followsOne = kCameraFollowsOne;
    e.cameraWayTwice = true; // (070232 goes by the byte two after it when a player's +257 says)
    e.clock = kRoundClock;
    e.playing = [] { return Ram8(kPlaying) != 0; };
    e.inFight = [] { return Ram8(kPlaying) != 0 && Ram16(kGameStep) == kGameMatch && Ram8(kFight) != 0; };
    e.notTheStage = NotTheStage;
    e.thePanel = ThePanel;
    e.player1Set = kPlayer1Set;
    e.player2Set = kPlayer2Set;
    e.shadowSet = kShadowSet;
    e.palettes = kPalettesInSet;
    // (the animations: groups of words of offsets from a list a character at 08A918, which 010380 starts one from;
    // frames of 0x10 bytes, copied into the record by 001AE6)
    e.animationLists = 0x08A918;
    e.frameBytes = 0x10;
    e.groups = 26;
    e.perGroup = 96;
    e.copyFrame = CopyFrame;
    marvel::Describe(e);
}

} // namespace cps2recomp
