// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Marvel Super Heroes: the game described to the toolkit's code for the engine of Capcom's Marvel games
// (fight/marvel.h): the training aids' view of the players and their boxes, the ring out's movements, widescreen and
// the model viewer are done there, from where this game keeps what they need (msh.h).
#include "marvel.h"
#include "msh.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace msh;

    // A frame put in a player's record as 001840 puts it: its picture, the long after the time, and the rest of the
    // frame's longs, which are its boxes' numbers (two attacks, four places it can be hit, one it can be thrown in)
    void CopyFrame(Machine &machine, uint32_t player, uint32_t frame) {
        const auto long_ = [&](uint32_t at) { return (static_cast<uint32_t>(machine.Read16(at)) << 16) | machine.Read16(at + 2); };
        SetRam16(player + 0x3C, machine.Read16(frame));
        SetRam32(player + 0x40, long_(frame + 0x04));
        SetRam32(player + 0x68, long_(frame + 0x08));
        SetRam32(player + 0x6C, long_(frame + 0x0C));
        SetRam32(player + 0x74, long_(frame + 0x10));
        SetRam32(player + 0x78, long_(frame + 0x14));
        SetRam32(player + 0x7C, long_(frame + 0x18));
        SetRam32(player + 0x80, long_(frame + 0x1C));
    }
} // namespace

void DescribeEngine() {
    marvel::Engine e;
    e.player1 = kPlayer1;
    e.playerSize = kPlayerSize;
    e.records = 2;
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
    e.planes = kPlanes;
    e.ground = Ground;
    e.cameraEnds = CameraEnds;
    e.stage = kStage;
    e.cameraWay = kCameraWay;
    e.followsOne = kCameraFollowsOne;
    e.clock = kRoundClock;
    e.playing = [] { return Ram8(kPlaying) != 0; };
    e.inFight = [] { return Ram8(kPlaying) != 0 && Ram16(kGameStep) == kGameMatch && Ram8(kFight) != 0; };
    e.notTheStage = NotTheStage;
    e.thePanel = ThePanel;
    e.player1Set = kPlayer1Set;
    e.player2Set = kPlayer2Set;
    e.shadowSet = kShadowSet;
    e.palettes = kPalettesInSet;
    // (the animations: 09E6FE, a long a character, read where 00DBC8 starts one; frames of 0x20 bytes, 001840)
    e.animationLists = 0x09E6FE;
    e.frameBytes = 0x20;
    e.groups = 30;
    e.perGroup = 96;
    e.romHigh = 0x400000;
    e.copyFrame = CopyFrame;
    marvel::Describe(e);
}

} // namespace cps2recomp
