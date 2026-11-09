// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Street Fighter II Turbo as one of the games made from its program, which build its sources (src/): what is its
// own. The places its sources name are its own, so there is no list of them (src/places.h).
#pragma once

#include <array>
#include <cstdint>

namespace ssf2t::family {

struct Place {
    uint32_t ssf2t, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

// What Super Turbo has and Super Street Fighter II (games/ssf2, made from the same program) has not: the super combo
// gauge, the hidden characters (Akuma, the old versions), the four speeds of the Board page
inline constexpr bool kTurbo = true;

// The sound program's table of songs (src/music.cpp): the entry of song 0, four bytes a song (the count, 0x1FF, the word
// six bytes before)
inline constexpr uint32_t kSongTable = 0x9006, kSongAddresses = 0;
// Songs of its own: the introduction and title, player select, the final fight (and its faster version, one on)
inline constexpr uint16_t kTuneTitle = 0xD4, kTuneChoose = 0xD3, kTuneFinal = 0xD5, kTuneBonus = 0; // (no bonus stages)

// The characters' colour sets (src/colours.cpp): their names, in the game's order; five palettes in each, for every
// character; the program file they are in and the part of it that must be the game's; whose set it is
inline constexpr const char *kColourSetNames[] = {"LP", "MP", "HP", "LK", "MK", "HK", "Start", "Held", "OldP", "OldK"};
inline constexpr int kPalettesInSet = 5;
inline constexpr int (*kPalettesOf)(int) = nullptr;
inline constexpr const char *kColourFile = "sfxj.04a";
inline constexpr uint32_t kColourFileAt = 0x080000, kColourFileSize = 0x80000, kColourFileSameFrom = 0x080000, kColourFileSameTo = 0x090000;
inline constexpr const char *kColourFileWhose = "Super Street Fighter II X's, 940223";
inline constexpr const char *kColourFeatureWords =
    "Characters in the colours of the files in the \"palettes\" folder in the game's data folder: a sfxj.04a changed with "
    "PalMod, and text files, one a character (Cammy.txt), which go before it. With no files the game's own colours. They are "
    "taken when a player is coloured: at the next round.";
inline constexpr const char *kColourHelp =
    "# To change them, copy this file to the palettes folder and change the colours there. Each set is\n"
    "# chosen by the button that chose the character ([LP] to [HK]), Start, a button held ([Held]) or the\n"
    "# old version's code ([OldP], [OldK]), and has five lines of 16 colours; the character is drawn in\n"
    "# them. The 16th colour of a line is never seen. A colour is #RRGGBB or #RGB, rounded to the board's\n"
    "# 16 levels of each; - keeps the game's. Sets and lines that are left out keep the game's colours.\n";

} // namespace ssf2t::family
