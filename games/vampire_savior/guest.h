// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior (vsavj) as a guest: its stages behind another game's fighters (fight/guest_stages.h). Its facts, found
// for its port (src/vsav.h, src/training.cpp, src/machines.cpp, family.h): A5 = FF8000; the game's steps FF8000 (2 a
// game), FF8004 (2 the character choice, 4 the match), FF8008 (4 a round), FF800C (6 the round fought); credits FF8076;
// who is in FF808D; a two-player match's stage chosen at 00AE7C and put at FF8100 (the stage's number doubled) by
// 00AEC4, the routine's end at 00AEC8 (where the stage wanted goes in its place); the round's clock FF8109 (seconds, 99
// at most); the camera asks where the players are at 01E8F4 (their outer edges, D0 and D1) and 01E962 (their heights,
// D0 the lower, D1 the higher); objects in colour sets below 22, and the players' shadows, are not the stage's.
#pragma once

#include "guest_stages.h"

#include <array>

namespace vampire_savior_guest {

// Its stages, in the order of their numbers
inline constexpr std::array<const char *, 12> kStageNames{
    "Vampire Savior: Feast of the Damned", "Vampire Savior: Concrete Cave",   "Vampire Savior: Tower of Arrogance",
    "Vampire Savior: Red Thirst",          "Vampire Savior: Deserted Chateau", "Vampire Savior: Abaraya",
    "Vampire Savior: Vanity Paradise",     "Vampire Savior: War Agony",       "Vampire Savior: Forever Torment",
    "Vampire Savior: Green Scream",        "Vampire Savior: Iron Horse, Iron Terror", "Vampire Savior: Fetus of God"};

// The players' shadows: in colour set 22, the stage's first, by their tiles (family.h's kShadowTiles)
inline bool Shadow(uint32_t tile, uint16_t attributes) {
    return (attributes & 0x1F) == 22 && tile >= 0xF8E0 && tile < 0xF920;
}

inline cps2recomp::guest_stages::Guest Description() {
    using namespace cps2recomp::guest_stages;
    Guest d;
    d.id = "vampire_savior";
    d.stageNames.assign(kStageNames.begin(), kStageNames.end());
    d.choosing = {{0xFF8000, 2}, {0xFF8004, 2}};
    d.match = {{0xFF8000, 2}, {0xFF8004, 4}};
    d.fought = {{0xFF8000, 2}, {0xFF8004, 4}, {0xFF8008, 4}, {0xFF800C, 6}};
    d.credits = 0xFF8076;
    d.playersIn = 0xFF808D;
    // (at the end of the routine that chose it: its choice, a word at FF8100, made the one wanted)
    d.stageAt = 0x00AEC8;
    d.setStage = [](cps2recomp::M68K &c, int stage) { c.bus.Write16(0xFF8100, static_cast<uint16_t>(stage * 2)); };
    d.clock = 0xFF8109;
    d.clockFull = 99;
    d.notTheStage = [](uint32_t tile, uint16_t attributes) { return (attributes & 0x1F) < 22 || Shadow(tile, attributes); };
    // Its camera's ends: 256 and 640, the players from 280 to 1000, the floor 0x28 up (as Alpha 2's)
    d.cameraLeft = 256;
    d.cameraRight = 640;
    d.camera = Guest::Camera::Extents;
    d.across = 0x01E8F4;
    d.up = 0x01E962;
    d.middle = 0xC0;
    d.halfApart = 0x60;
    d.above = 0x60;
    return d;
}

} // namespace vampire_savior_guest
