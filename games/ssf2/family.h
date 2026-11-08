// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Street Fighter II as one of the games built from Super Street Fighter II Turbo's sources (games/ssf2t/src;
// places.h there): where the places those sources name are in this game, and what is this game's own.
#pragma once

#include <array>
#include <cstdint>

#include "game.h"

namespace ssf2t::family {

struct Place {
    uint32_t ssf2t, here;
};
inline constexpr Place kPlaceList[] = {
    // places: begin (tools/places.py; a line ending "by hand" is kept)
    {0x002EE2, 0x002D88}, // an instruction, 12 on alike
    {0x002F50, 0x002DF6}, // an instruction, 15 on alike
    {0x0043F4, 0x0040DC}, // an instruction, 7 on alike
    {0x00490A, 0x004488}, // moved as the code before or after it, 6 on alike
    {0x0049B0, 0x004534}, // an instruction, 6 on alike
    {0x004A58, 0x0045D2}, // an instruction, 6 on alike
    {0x00502A, 0x004A54}, // a place the code names so (1/1)
    {0x008FFC, 0x008380}, // moved as the code before or after it, 1 on alike; as many at 00836E: check
    {0x00D69C, 0x00C11A}, // the same entry of a jump table, 9 on alike
    {0x00D714, 0x00C192}, // the same entry of a jump table, 2 on alike
    {0x00E93A, 0x00CF70}, // an instruction, 2 on alike
    {0x00EA5C, 0x00D00C}, // an instruction, 3 on alike
    {0x00FCE2, 0x00DE98}, // an instruction, 6 on alike
    {0x00FCE6, 0x00DE9C}, // an instruction, 5 on alike
    {0x00FD62, 0x00DF0C}, // an instruction, 6 on alike
    {0x00FD66, 0x00DF10}, // an instruction, 5 on alike
    {0x00FDEE, 0x00DF8C}, // an instruction, 5 on alike
    {0x00FE0C, 0x00DFA6}, // an instruction, 6 on alike
    {0x00FE74, 0x00E018}, // an instruction, 3 on alike
    {0x0102F2, 0x00E5D2}, // the panel's names and faces of both players (lea 709090, 7090D8; portmap's own pair) by hand
    {0x0104B2, 0x000000}, // none: no routine makes the faces' palettes apart (livetag.cpp) by hand
    {0x078030, 0x06AB1A}, // an instruction, 6 on alike
    {0x07804A, 0x06AB48}, // an instruction, 4 on alike
    {0x078052, 0x06AB50}, // an instruction, 2 on alike
    {0x078270, 0x06AE52}, // an instruction, 3 on alike
    {0x0782D4, 0x06AEA4}, // an instruction, 4 on alike
    {0x07BB82, 0x06DDBE}, // an instruction, 10 on alike
    {0x07BC12, 0x06DE2C}, // an instruction, 9 on alike
    {0x07D6F4, 0x06F8B0}, // an instruction, 2 on alike
    {0x07E36E, 0x070434}, // an instruction, 4 on alike
    {0x07F1C6, 0x0711BC}, // a place the code names so (1/1)
    {0x0BE646, 0x0921EE}, // an instruction, 3 on alike
    {0x0BE678, 0x09220C}, // an instruction, 6 on alike
    {0x0BEBA6, 0x092634}, // sub.w d6,0x2A(a2) in the thrown thing's hit (the same code around it) by hand
    {0x0BEBDA, 0x09264E}, // move.b 0x7(a3),0x19C(a2) after it, where its ways meet by hand
    {0x0BFAF2, 0x093CDC}, // a place the code names so (3/3)
    {0x0BFB2C, 0x093D0C}, // the first character's first set (its count of palettes): the table at 093CDC by hand
    {0x0C6320, 0x0961FC}, // an instruction, 11 on alike
    {0x0CC4E4, 0x09BC80}, // the same entry of a jump table, 4 on alike
    {0x0CC630, 0x09BD98}, // the camera's place up (the jump table's entry; no test of a flag of its own first) by hand
    {0x0CCC5C, 0x09C3B2}, // an instruction, 11 on alike
    {0x100000, 0x100000}, // a place the code names so (12/12)
    {0x24EAE8, 0x1ADC06}, // a place the code names so (1/1)
    {0xFF0A7A, 0xFF0A72}, // a variable the code names so (4/4)
    {0xFF0A7E, 0xFF0A76}, // a variable the code names so (3/3)
    {0xFF8004, 0xFF8004}, // a variable the code names so (84/84)
    {0xFF8008, 0xFF8008}, // a variable the code names so (87/88)
    {0xFF800C, 0xFF800C}, // a variable the code names so (118/118)
    {0xFF8086, 0xFF8086}, // a variable the code names so (15/15)
    {0xFF808A, 0xFF808A}, // a variable the code names so (15/15)
    {0xFF82AE, 0xFF82AE}, // a variable the code names so (1/1)
    {0xFF82B6, 0xFF82B6}, // a variable the code names so (1/1)
    {0xFF82D0, 0xFF82D0}, // a variable the code names so (1/1)
    {0xFF82D3, 0xFF82D3}, // a variable between two that moved alike
    {0xFF82DA, 0xFF82DA}, // a variable the code names so (4/4)
    {0xFF82DD, 0xFF82DD}, // a variable the code names so (1/1)
    {0xFF82DF, 0xFF82DF}, // a variable the code names so (20/20)
    {0xFF82E9, 0xFF82E9}, // a variable the code names so (35/35)
    {0xFF82F0, 0xFF82F0}, // a variable the code names so (24/24)
    {0xFF8342, 0xFF8342}, // a variable the code names so (1/1)
    {0xFF83A9, 0x000000}, // none: no speeds (board.cpp) by hand
    {0xFF844E, 0xFF83CE}, // a variable the code names so (51/51)
    {0xFF8C4E, 0xFF8BCE}, // a variable the code names so (45/45)
    {0xFF8C54, 0xFF8BD4}, // a variable the code names so (3/3)
    {0xFF8C56, 0xFF8BD6}, // a variable the code names so (3/3)
    {0xFF8C58, 0xFF8BD8}, // a variable the code names so (3/3)
    {0xFF8C5A, 0xFF8BDA}, // a variable the code names so (3/3)
    {0xFF8DCE, 0xFF8CCE}, // a variable the code names so (23/23)
    {0xFF8DCF, 0xFF8CCF}, // a variable the code names so (2/2)
    {0xFF8DD2, 0xFF8CD2}, // a variable the code names so (38/41)
    {0xFF8DDC, 0xFF8CDC}, // a variable the code names so (7/7)
    {0xFF8DDE, 0xFF8CDE}, // a variable the code names so (9/9)
    {0xFF8ECE, 0xFF8DCE}, // a variable the code names so (24/24)
    {0xFF8ED4, 0xFF8DD4}, // a variable the code names so (38/39)
    {0xFF8ED8, 0xFF8DD8}, // a variable the code names so (27/28)
    {0xFF8F90, 0x000000}, // none: the camera up has no flag of its own holding it (widescreen.cpp) by hand
    {0xFF8FEA, 0xFF8EEA}, // a variable between two that moved alike
    {0xFF8FEC, 0xFF8EEC}, // a variable between two that moved alike
    {0xFF8FEE, 0xFF8EEE}, // a variable between two that moved alike
    {0xFF8FF0, 0xFF8EF0}, // a variable between two that moved alike
    {0xFF954C, 0xFF944C}, // a variable the code names so (4/4)
    {0xFF9580, 0xFF9480}, // a variable the code names so (5/5)
    {0xFF97A2, 0xFF96A2}, // a variable the code names so (2/2)
    {0xFFA9A2, 0xFFA8A2}, // a variable the code names so (3/3)
    // places: end
};
inline constexpr auto kPlaces = std::to_array(kPlaceList);

// Super Turbo's own, which this game has not: the super combo gauge (its byte is never used here, src/training.cpp), the
// hidden characters (Akuma and the old versions: src/choosing.cpp), the four speeds (src/board.cpp). (Its player's
// record is Super Turbo's to +3B1: every offset the sources name, compared over the instructions portmap paired.)
inline constexpr bool kTurbo = false;

// The sound program's table of songs (src/music.cpp): its version is Super Turbo's (1.04), but the table is at 7C16 (the
// count, 0x1FF, at 7C10: the program's ld hl,7C10 at 01E1, where Super Turbo's reads 9000), its songs in the same order.
// Its entries give a song's place with a bank byte of 10 (108411: 0A71 makes the bank from it, which goes round the
// ROM's eight), so only the bits of the 128 KB ROM are the place: 63 songs are music so, Super Turbo's but five
inline constexpr uint32_t kSongTable = 0x7C16, kSongAddresses = 0x1FFFF;
// Its own songs (CPS2RECOMP_SOUNDTRACK_LOG over attract mode with its sound and the ladder): the introduction and title
// 33, player select 34, the bonus stages 3B; the last fight has M. Bison's stage's tune (0C), none of its own
inline constexpr uint16_t kTuneTitle = 0x33, kTuneChoose = 0x34, kTuneFinal = 0, kTuneBonus = 0x3B;

// The characters' colour sets (src/colours.cpp): eight, chosen as in Super Turbo (00CF70 on: the six buttons, Start, a
// button held), each a count of palettes and the palettes, from the table at 093CDC (a word a character, to its list of
// eight sets): one palette for most, three for Ryu, Guile, Ken, Chun-Li, Sagat and Dee Jay, eight for M. Bison (his
// cape's colours). Read from the ROM.
inline constexpr const char *kColourSetNames[] = {"LP", "MP", "HP", "LK", "MK", "HK", "Start", "Held"};
inline constexpr int kPalettesInSet = 8;
inline int PalettesOf(int number) {
    constexpr uint32_t kTable = 0x093CDC;
    const auto word = [](uint32_t at) { return static_cast<int>((cps2recomp::Program(at)[0] << 8) | cps2recomp::Program(at)[1]); };
    const uint32_t list = kTable + static_cast<uint32_t>(word(kTable + static_cast<uint32_t>(number) * 2));
    return word(list + static_cast<uint32_t>(word(list)));
}
inline constexpr int (*kPalettesOf)(int) = PalettesOf;
inline constexpr const char *kColourFile = "ssfe.04";
inline constexpr uint32_t kColourFileAt = 0x080000, kColourFileSize = 0x80000, kColourFileSameFrom = 0x080000, kColourFileSameTo = 0x090000;
inline constexpr const char *kColourFileWhose = "Super Street Fighter II's, World 931005";
inline constexpr const char *kColourFeatureWords =
    "Characters in the colours of the files in the \"palettes\" folder in the game's data folder: a ssfe.04 changed with "
    "PalMod, and text files, one a character (Cammy.txt), which go before it. With no files the game's own colours. They are "
    "taken when a player is coloured: at the next round.";
inline constexpr const char *kColourHelp =
    "# To change them, copy this file to the palettes folder and change the colours there. Each set is\n"
    "# chosen by the button that chose the character ([LP] to [HK]), Start or a button held ([Held]), and\n"
    "# has one to eight lines of 16 colours (by character); the character is drawn in them. The 16th colour\n"
    "# of a line is never seen. A colour is #RRGGBB or #RGB, rounded to the board's 16 levels of each; -\n"
    "# keeps the game's. Sets and lines that are left out keep the game's colours.\n";

} // namespace ssf2t::family
