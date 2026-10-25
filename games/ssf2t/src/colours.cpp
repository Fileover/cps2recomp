// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Street Fighter II Turbo: the players' own colours for the characters and the colour editor, which are the
// toolkit's (fight/colours.h). These are the game's facts for them: where its characters' colours are, the ROM file
// PalMod changes, and how the editor shows a set on player 1 in the model viewer (model.cpp).
//
// A character's colours are in the ROM in sets of five palettes of 16 colours (words, 4 bits each of red, green and
// blue; the game puts the brightness in as it copies them), each set a word of how many palettes and then the
// palettes. A table at 0BFAF2 names each character's list of its sets (a word from the table each, by the character's
// number; Akuma is 16), and that list a set for each choice (kColourChoice: 0-5 the button that chose, 6 Start, 7 a
// button held, 8 and 9 the old version's; Akuma's only in 0A and 0B). When a round starts the game copies each player's
// set to the palettes the board shows (0BFA78: player 1's to object palettes 1-5 at 900020, player 2's to 10-14 at
// 900200). Found from the palettes a fight showed, looked for in the ROM, and the routine that copies them.
#include "colours.h"
#include "machine.h"
#include "model.h"
#include "ssf2t.h"

#include <iterator>
#include <vector>

namespace cps2recomp {

void StartModelViewer(Machine &machine); // model.cpp
bool ModelViewer();

namespace {
    using namespace ssf2t;

    constexpr uint32_t kColourTable = 0x0BFAF2;
    constexpr int kSets = 10, kPalettesInSet = 5;
    constexpr uint32_t kColoursFrom = 0x0BFB2C, kColoursTo = 0x0C6320; // (every set, Akuma's last)
    constexpr uint32_t kPaletteBuffer = 0x900000; // (the objects' palettes, 0x20 bytes a colour set)
    constexpr int kFirstPalette = 1;              // player 1's

    uint16_t Word(uint32_t address) {
        const uint8_t *at = Program(address); // (the data view: 0BFA94 reads it through A0)
        return static_cast<uint16_t>((at[0] << 8) | at[1]);
    }
    // A set's first palette (after its count)
    uint32_t SetAddress(int number, int set) {
        const uint32_t list = kColourTable + Word(kColourTable + static_cast<uint32_t>(number) * 2);
        return list + Word(list + static_cast<uint32_t>(set) * 2) + 2;
    }
    std::vector<colours::Range> Ranges() {
        return {{kColoursFrom, kColoursTo}};
    }

    // Player 1 in the set: its choice as the game keeps it; its palettes are in its own colour sets of the buffer
    uint32_t ShowSet(Machine &, int, int set, int &lines) {
        SetRam8(Player(0) + kColourChoice, static_cast<uint8_t>(set));
        lines = kPalettesInSet;
        return kPaletteBuffer + kFirstPalette * 0x20u;
    }

    // The stages as short as the panel needs them: whose they are
    std::vector<const char *> Backgrounds() {
        static const char *const names[kStages] = {"Ryu's",   "E. Honda's", "Blanka's", "Guile's",  "Ken's",    "Chun-Li's",
                                                   "Zangief's", "Dhalsim's", "M. Bison's", "Sagat's", "Balrog's", "Vega's",
                                                   "Cammy's", "T. Hawk's",  "Fei Long's", "Dee Jay's"};
        std::vector<const char *> list(std::begin(names), std::end(names));
        list.push_back("White");
        list.push_back("Black");
        return list;
    }
} // namespace

void RegisterColours() {
    colours::Description d;
    for (int n = 0; n < kCharacters; n++) {
        // (the names the game has in Japan, for files called by them: the boxer, the claw and the dictator go round)
        d.characters.push_back({n, kCharacterNames[n], n == 10 ? "M. Bison (Japan)" : n == 11 ? "Balrog (Japan)" : n == 8 ? "Vega (Japan)" : nullptr});
    }
    d.setNames = {"LP", "MP", "HP", "LK", "MK", "HK", "Start", "Held", "OldP", "OldK"};
    d.palettesInSet = kPalettesInSet;
    d.setAddress = SetAddress;
    d.ranges = Ranges;
    // The second of the set's program files has every character's colours; PalMod never changes its first part, which is
    // the game's code
    d.romFile = "sfxj.04a";
    d.romFileAt = 0x080000;
    d.romFileSize = 0x80000;
    d.same = {0x080000, 0x090000};
    d.romFileWhose = "Super Street Fighter II X's, 940223";
    d.featureDescription = "Characters in the colours of the files in the \"palettes\" folder in the game's data folder: a "
                           "sfxj.04a changed with PalMod, and text files, one a character (Cammy.txt), which go before it. "
                           "With no files the game's own colours. They are taken when a player is coloured: at the next round.";
    d.originalHelp = "# To change them, copy this file to the palettes folder and change the colours there. Each set is\n"
                     "# chosen by the button that chose the character ([LP] to [HK]), Start, a button held ([Held]) or the\n"
                     "# old version's code ([OldP], [OldK]), and has five lines of 16 colours; the character is drawn in\n"
                     "# them. The 16th colour of a line is never seen. A colour is #RRGGBB or #RGB, rounded to the board's\n"
                     "# 16 levels of each; - keeps the game's. Sets and lines that are left out keep the game's colours.\n";
    d.showSet = ShowSet;
    d.startViewer = StartModelViewer;
    d.viewerOn = ModelViewer;
    d.viewerCharacter = model::CharacterNumber;
    d.characterNumber = model::Number;
    d.animationStep = model::AnimationStep;
    d.animationNames = model::AnimationNames;
    d.animationForLine = [](Machine &, int line) { return model::AnimationForLine(line); };
    d.backgrounds = Backgrounds();
    colours::Register(d);
}

} // namespace cps2recomp
