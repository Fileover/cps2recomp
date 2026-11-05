// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Darkstalkers (dstlk) as a guest: its stages behind another game's fighters (fight/guest_stages.h). Its facts, found
// for its port (family.h here; games/nwarr/src/nwarr.h has them at Night Warriors' places): A5 = FF8000; the game's
// steps FF8000, FF8004, FF8008; credits FF81D0; who is in FF81F9; a two-player match's stage set at 0059C6 from
// A5+13B0; the clock FF9409 (BCD); the camera an object at FF9508, stepped across at 0EDAF4 and up at 0EDC1A (from its
// frame, 0EDA94: its place at +10 and +14, the step at +34 and +36, which way at +2C and +2D); objects in colour sets
// below 14 are the panel's, the players' and their shadows' and sparks'.
#pragma once

#include "guest_stages.h"

#include <array>

namespace dstlk_guest {

// Its stages, each a character's, in its order
inline constexpr std::array<const char *, 12> kStageNames{
    "Darkstalkers: Demitri's",  "Darkstalkers: Jon Talbain's", "Darkstalkers: Victor's",   "Darkstalkers: Lord Raptor's",
    "Darkstalkers: Morrigan's", "Darkstalkers: Anakaris's",    "Darkstalkers: Felicia's",  "Darkstalkers: Bishamon's",
    "Darkstalkers: Rikuo's",    "Darkstalkers: Sasquatch's",   "Darkstalkers: Huitzil's", "Darkstalkers: Pyron's"};

inline cps2recomp::guest_stages::Guest Description() {
    using namespace cps2recomp::guest_stages;
    Guest d;
    d.id = "dstlk";
    d.stageNames.assign(kStageNames.begin(), kStageNames.end());
    // A game (FF8000 2): its character choice (FF8004 0), its match (FF8004 4), a round fought (FF8008 0A)
    d.choosing = {{0xFF8000, 2}, {0xFF8004, 0}};
    d.match = {{0xFF8000, 2}, {0xFF8004, 4}};
    d.fought = {{0xFF8000, 2}, {0xFF8004, 4}, {0xFF8008, 0x0A}};
    d.credits = 0xFF81D0;
    d.playersIn = 0xFF81F9;
    // 0059C6: a two-player match's stage, from A5+13B0 (the stage + 1)
    d.stageAt = 0x0059C6;
    d.setStage = [](cps2recomp::M68K &c, int stage) { c.ram[0x93B0] = static_cast<uint8_t>(stage + 1); };
    d.clock = 0xFF9409;
    d.clockFull = 0x99;
    d.notTheStage = [](uint32_t, uint16_t attributes) { return (attributes & 0x1F) < 0x14; };
    // Its camera's ends are this game's own: 256 and 640 (both walked to), the floor 0x28 up
    d.cameraLeft = 0x100;
    d.cameraRight = 0x280;
    d.cameraAtX = 0xFF9518;
    d.cameraAtY = 0xFF951C;
    d.camera = Guest::Camera::Steps;
    d.across = 0x0EDAF4;
    d.up = 0x0EDC1A;
    return d;
}

} // namespace dstlk_guest
