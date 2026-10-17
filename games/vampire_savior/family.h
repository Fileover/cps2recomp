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

// The frame's lists of objects to draw that the objects of kind 12 have, one for each depth (src/picture.cpp)
inline constexpr int kTwelveLists = 7;

// Each stage's own tune (src/modes.cpp: the stage viewer's music)
inline constexpr uint16_t kStageTunes[] = {0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14};

// Where the hit test goes on to when an attack has hit (src/collision.cpp): HitLands (018230)
inline constexpr uint32_t kHitTestHit = 0x018230;

// How the fight's rules differ from game to game (src/fight.cpp, src/edges.cpp; 0: as in Vampire Savior):
// - a byte of the attacker's record that, set, has all of a hit's damage taken for good, and where (from A5) the game
//   keeps A0 while it looks
// - one more kind of attack that is an overhead
// - a byte of the victim's record that, set, lets it guard the attacks that cannot be guarded
// - how near (across) the other player has to be for a player to guard by being near
// - a byte of a player's record set (FF) when the player is kept in at the picture's edge
inline constexpr uint32_t kAllForGood = 0;
inline constexpr int32_t kAllForGoodKeep = 0;
inline constexpr uint8_t kOverheadToo = 0;
inline constexpr uint32_t kGuardsAnything = 0;
inline constexpr uint16_t kGuardNear = 0x80;
inline constexpr uint32_t kAtEdgeToo = 0;

// Tiles of the objects the port tells apart (src/machines.cpp, src/widescreen.cpp), from one to one less than the
// other: the shadows (colour set 22), the panel's faces beside the bars (drawn in the players' own colours), the red
// curtain between a match's screens (set 3)
inline constexpr uint32_t kShadowTiles[2] = {0xF8E0, 0xF920};
inline constexpr uint32_t kPanelTiles[2] = {0x13800, 0x14400};
inline constexpr uint32_t kCurtainTiles[2] = {0x2C600, 0x2FE00};

// The ROM file whose colours PalMod changes, where it is in the program, and which set it is
inline constexpr const char *kColoursFile = "vm3j.10b", *kColoursFileWhose = "the Japanese set's, 970519";

} // namespace vsav::family
