// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: the fighting modes of the menu's "Alternative modes" page (survival, the endless battle,
// the ring out) are the toolkit's (fight/battle.h); this says where the game keeps what they need.
#include "battle.h"
#include "modes.h"
#include "pocket.h"

#include <iterator>

namespace cps2recomp {

void RegisterBattle() {
    using namespace pocket;
    battle::Description d;
    d.result = kRoundResult;
    d.cameraX = kCameraX;
    d.cameraY = kCameraY;
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.playerRecords = 2;
    d.thrown = kThrown;
    d.thrownSize = kThrownSize;
    d.thrownRecords = kThrownRecords;
    d.active = kActive;
    d.x = kX;
    d.y = kY;
    d.character = kCharacter;
    d.computer = kComputer;
    d.control = kControl;
    d.controlOwn = kControlOwn;
    d.halfWidthOf = HalfWidth;
    d.health = kHealth;
    d.healthFull = kHealthFull;
    d.healthBack = 0x18; // (a sixth of it all)
    d.stages = kStages;
    d.stageToRound = 2300; // (about 2,100 from switching on, and a few)
    d.characterSpan = kCharacters;
    d.choosable = [](int) { return true; };
    d.rosterNumber = [](int choice) { return choice >= 1 && choice <= kCharacters ? choice - 1 : -1; };
    d.fighterChoices = {"Random"};
    d.fighterChoices.insert(d.fighterChoices.end(), std::begin(kCharacterNames), std::end(kCharacterNames));
    d.stageChoices = modes::StageChoices({"Shuffle", "Scroll"});
    d.inGame = [] { return Ram16(kMode) == 2; };
    d.choosing = modes::Choosing;
    d.toRound = modes::ToRound;
    d.playSound = modes::PlaySound;
    d.wanted = modes::Wanted;
    d.cameraAt = modes::CameraAt;
    d.endModes = modes::End;
    d.stageViewer = modes::StageViewer;
    d.wideSide = [] { return 0; };
    d.wideMargin = [] { return 0; };
    d.notTheStage = NotTheStage;
    d.thePanel = ThePanel;
    d.notTheFighters = NotTheFighters;
    d.playerSets = [](int n) { return 0x07u << (Ram8(Player(n) + kColourSet) & 0x1F); };
    d.inAir = [](uint32_t player) { return Ram16(player + kY) != kGround; };
    d.atJumpTop = [](uint32_t player) { return Ram16(player + kY) != kGround && static_cast<int32_t>(Ram32(player + kSpeedY)) <= 0; };
    battle::Register(d);
}

} // namespace cps2recomp
