// SPDX-License-Identifier: GPL-3.0-only
//
// Street Fighter Alpha: names for what is known of the game's memory and routines, each found by experiment in
// this game. Its variables are reached from A5 = FF8000, as in Alpha 2, but most are elsewhere: Alpha 2 is not
// made from this game's program, so nothing is taken from there unchecked.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

#include <cstdint>

namespace sfa {

// Credits paid for and not yet used, a byte (found by RAM dumps after one, two and three coins put in: 0, 1, 2,
// 3; the board's settings memory starts at one coin a credit). Alpha 2 keeps its own at FF8076.
constexpr uint32_t kCredits = 0xFF8170;

} // namespace sfa
