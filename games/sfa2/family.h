// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2 as one of the games made from its program, which build its sources (src/): what is its
// own. The places its sources name are its own, so there is no list of them (src/places.h).
#pragma once

#include <array>
#include <cstdint>

namespace sfa2::family {

struct Place {
    uint32_t sfa2, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

// Who a player is (a player's byte +82), by number: seen on the name plates with each number put in. 18 to 23 are
// other forms of six of them (the classic Chun-Li, Shin Akuma and Evil Ryu among them), each with its first form's
// stage.
inline constexpr int kCharacters = 24;
inline constexpr const char *kCharacterNames[kCharacters] = {
    "Ryu", "Ken", "Akuma", "Charlie", "Chun-Li", "Adon", "Sodom", "Guy", "Birdie", "Rose", "M. Bison", "Sagat", "Dan", "Sakura",
    "Rolento", "Dhalsim", "Zangief", "Gen", "Chun-Li (classic)", "Gen (another form)", "Shin Akuma", "Zangief (another form)",
    "Dhalsim (another form)", "Evil Ryu"};
// Whose other form each of those after the first eighteen is (src/livetag.cpp: its call as it tags out is its first
// form's)
inline constexpr uint8_t kFormOf[kCharacters - 18] = {0x04, 0x11, 0x02, 0x10, 0x0F, 0x00};

// The hidden characters the "Hidden characters" feature gives for one who chooses the first form (src/training.cpp),
// and what it says. (Gen's other form is left out: it is the style he changes to in a fight.)
struct HiddenForm {
    uint8_t first, other;
};
inline constexpr HiddenForm kHiddenForms[] = {{0, 23}, {2, 20}, {4, 18}, {15, 22}, {16, 21}};
inline constexpr const char *kHiddenWords =
    "Choosing Ryu, Akuma, Chun-Li, Dhalsim or Zangief gives their hidden form instead: Evil Ryu, Shin Akuma, the classic "
    "Chun-Li, and Dhalsim's and Zangief's other forms. For player 1, player 2 or both.";

// The ROM file whose colours PalMod changes, and which set it is (src/colours.cpp)
inline constexpr const char *kColoursFile = "sz2u.07", *kColoursFileWhose = "the USA set's, 960430";

} // namespace sfa2::family
