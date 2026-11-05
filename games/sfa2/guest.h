// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2 (sfa2u) as a guest: its stages behind another game's fighters (fight/guest_stages.h). Its
// facts, found for its port (src/sfa2.h, src/modes.cpp, src/training.cpp, src/fight.cpp, src/machines.cpp,
// src/widescreen.cpp): A5 = FF8000; the game's steps FF8000 (2 a game), FF8004 (2 the character choice, 4 the match),
// FF8008 (4 a round), FF800C (2 the round fought); credits FF8076; who is in FF80A8; the match's stage chosen at 00797C
// (D0's low byte, from one of the characters); the round's clock FF8109 (seconds, 99 at most); the camera asks where
// the players are at 01BFE0 (their outer edges, D0 and D1: far apart, it puts their middle in the picture's, 0xC0 on
// from its place) and 01C1D0 (their heights: D1, the higher, followed when more than 0x60 above its place); objects in
// colour sets below 27, and the panel's by their tiles, are not the stage's.
#pragma once

#include "guest_stages.h"

#include <array>

namespace sfa2_guest {

// Its stages, each a character's, in its order
inline constexpr std::array<const char *, 18> kStageNames{
    "Alpha 2: Ryu's (moonlit snow)",      "Alpha 2: Ken's (the yacht)",          "Alpha 2: Akuma's (the cave)",
    "Alpha 2: Charlie's (the hangar)",    "Alpha 2: Chun-Li's (the market)",     "Alpha 2: Adon's (the temple)",
    "Alpha 2: Sodom's (the truck)",       "Alpha 2: Guy's (the city street)",    "Alpha 2: Birdie's (the back alley)",
    "Alpha 2: Rose's (the harbour)",      "Alpha 2: M. Bison's (the airfield)",  "Alpha 2: Sagat's (the reclining Buddha)",
    "Alpha 2: Dan's (the cafe)",          "Alpha 2: Sakura's (the shopping street)", "Alpha 2: Rolento's (the mural)",
    "Alpha 2: Dhalsim's (the elephants)", "Alpha 2: Zangief's (the lanterns)",   "Alpha 2: Gen's (the ruins)"};

// The panel's objects (the bars, the names, the clock and the faces, the gauges): by their tiles, in whatever colours
// (0A400-0A6FF only outside the players' colour sets, 11-25)
inline bool ThePanel(uint32_t tile, uint16_t attributes) {
    const unsigned set = attributes & 0x1F;
    return (tile >= 0x1A400 && tile < 0x1AB00) || (tile >= 0x26A00 && tile < 0x26B00) ||
           (tile >= 0x0A400 && tile < 0x0A700 && (set < 11 || set > 25));
}

inline cps2recomp::guest_stages::Guest Description() {
    using namespace cps2recomp::guest_stages;
    Guest d;
    d.id = "sfa2";
    d.stageNames.assign(kStageNames.begin(), kStageNames.end());
    d.choosing = {{0xFF8000, 2}, {0xFF8004, 2}};
    d.match = {{0xFF8000, 2}, {0xFF8004, 4}};
    d.fought = {{0xFF8000, 2}, {0xFF8004, 4}, {0xFF8008, 4}, {0xFF800C, 2}};
    d.credits = 0xFF8076;
    d.playersIn = 0xFF80A8;
    d.stageAt = 0x00797C;
    d.setStage = [](cps2recomp::M68K &c, int stage) { c.d[0] = (c.d[0] & 0xFFFFFF00) | static_cast<uint8_t>(stage); };
    d.clock = 0xFF8109;
    d.clockFull = 99;
    d.notTheStage = [](uint32_t tile, uint16_t attributes) { return (attributes & 0x1F) < 27 || ThePanel(tile, attributes); };
    // Its camera's ends: 256 and 640, the players from 280 to 1000, the floor 0x28 up (as Vampire Savior's)
    d.cameraLeft = 256;
    d.cameraRight = 640;
    d.cameraAtX = 0xFF8290;
    d.cameraAtY = 0xFF8294;
    d.camera = Guest::Camera::Extents;
    d.across = 0x01BFE0;
    d.up = 0x01C1D0;
    d.middle = 0xC0;
    d.halfApart = 0x60;
    d.above = 0x60;
    return d;
}

} // namespace sfa2_guest
