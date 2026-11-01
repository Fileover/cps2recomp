// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors as the first of the games made from its program, whose sources Darkstalkers builds too
// (games/nwarr/src; places.h there): its places are where its sources say (an empty list).
#pragma once

#include <array>
#include <cstdint>

namespace nwarr::family {

struct Place {
    uint32_t nwarr, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

// --- what differs between the games made from the program (each game's own family.h; the sources' nwarr.h) ---

// The players' records: how far apart, and the bytes at their end (the character chosen, the colours chosen as a word
// and its low byte, 1 for a person, 4 once the guard mode is chosen (0: none asked), 1 or 2 at the picture's edge)
inline constexpr uint32_t kPlayerSize = 0x500;
inline constexpr uint32_t kChoice = 0x4A1, kColoursWord = 0x4A6, kColoursChosen = 0x4A7, kPerson = 0x49F, kModeChosen = 0x4B9,
                          kAtEdge = 0x6F;
// The step word FF8004 while the players choose
inline constexpr uint16_t kGameChoosing = 0x0C;

// The show's loop (FF8000): its parts up to kShowUnseen are run unseen in attract mode, the introduction is
// kIntroFirst-kIntroLast (the title in it: kShowTitle with FF8004 at kShowTitleStep on), the fights kFightFirst-kFightLast
inline constexpr uint16_t kShowUnseen = 0x02, kIntroFirst = 0x04, kIntroLast = 0x06, kFightFirst = 0x0E, kFightLast = 0x14;
inline constexpr uint16_t kShowTitle = 0x06, kShowTitleStep = 0x14;

// The characters by number (0: none), and the stages by number, each the stage of the character one on
inline constexpr int kCharacters = 15;
inline constexpr const char *kCharacterNames[kCharacters] = {"?",          "Demitri",  "Jon Talbain", "Victor",   "Lord Raptor",
                                                             "Morrigan",   "Anakaris", "Felicia",     "Bishamon", "Rikuo",
                                                             "Sasquatch",  "Huitzil",  "Pyron",       "Hsien-Ko", "Donovan"};
inline constexpr int kStages = 14;
inline constexpr const char *kStageNames[kStages] = {"Demitri's", "Jon Talbain's", "Victor's", "Lord Raptor's", "Morrigan's",
                                                     "Anakaris's", "Felicia's", "Bishamon's", "Rikuo's", "Sasquatch's",
                                                     "Huitzil's", "Pyron's", "Hsien-Ko's", "Donovan's"};

// What the players throw: how many records of 0xC0 from the first (the sources' kThrown)
inline constexpr int kThrownRecords = 9;

// Objects by colour set: the panel's are up to kPanelSets, the stage's pieces' from kStageSets; the faces beside the
// bars are in the players' sets, tiles kFaces to kFacesEnd
inline constexpr unsigned kPanelSets = 0x04, kStageSets = 0x14;
inline constexpr uint32_t kFaces = 0x28440, kFacesEnd = 0x284C0;

} // namespace nwarr::family
