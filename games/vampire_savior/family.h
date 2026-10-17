// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior as one of the games made from its program, which build its sources (src/): what is its own.
// The places its sources name are its own, so there is no list of them (src/places.h).
#pragma once

#include <array>
#include <cstdint>

namespace vsav::family {

struct Place {
    uint32_t vsav, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

// The characters a player chooses, by the number the game keeps for each (a player's record, +382), with
// what this set calls them and, if another, what the American and European sets do
struct Character {
    uint8_t number;
    const char *name, *otherName;
};
inline constexpr Character kRoster[] = {
    {0x00, "Bulleta", "BB Hood"}, {0x01, "Demitri", nullptr},    {0x02, "Gallon", "Talbain"},     {0x03, "Victor", nullptr},
    {0x04, "Zabel", "Lord Raptor"}, {0x05, "Morrigan", nullptr}, {0x06, "Anakaris", nullptr},     {0x07, "Felicia", nullptr},
    {0x08, "Bishamon", nullptr},  {0x09, "Aulbath", "Rikuo"},   {0x0A, "Sasquatch", nullptr},    {0x0C, "Q-Bee", nullptr},
    {0x0D, "Lei-Lei", "Hsien-Ko"}, {0x0E, "Lilith", nullptr},   {0x0F, "Jedah", nullptr},
};
// The hidden ones (src/choosing.cpp): Gallon's and Bishamon's numbers with bit 4
inline constexpr Character kHidden[] = {{0x12, "Dark Gallon", "Dark Talbain"}, {0x18, "Oboro Bishamon", nullptr}};
inline constexpr uint8_t kDarkGallon = 0x12, kOboroBishamon = 0x18;

// The stages, in the order of their numbers (the word at FF8100 is the number doubled)
inline constexpr const char *kStageNames[] = {
    "Feast of the Damned", "Concrete Cave", "Tower of Arrogance", "Red Thirst", "Deserted Chateau", "Abaraya",
    "Vanity Paradise", "War Agony", "Forever Torment", "Green Scream", "Iron Horse, Iron Terror", "Fetus of God",
};

// The ROM file whose colours PalMod changes, where it is in the program, and which set it is
inline constexpr const char *kColoursFile = "vm3j.10b", *kColoursFileWhose = "the Japanese set's, 970519";

} // namespace vsav::family
