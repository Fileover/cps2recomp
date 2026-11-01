// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound program
// (driver 1.05A) keeps a table of its songs at 0x9006, four bytes a song, as Marvel Super Heroes' does: the word at
// 0x9000 is how many (0x400), and a song whose data starts with a 0 is music.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x400;
    // Found by playing (CPS2RECOMP_SOUNDTRACK_LOG): the ladder with Lord Raptor, each stage held with --poke for the
    // four it did not reach, attract mode; `true` for the ones the board plays once
    TheGame().songs = {
        // Each stage's, the stage's own character's (08 on in the stages' order; Hsien-Ko's and Donovan's, new in this
        // game, are 03EC and 03ED)
        {0x008, "demitri", "Demitri's stage"},
        {0x009, "jon_talbain", "Jon Talbain's stage"},
        {0x00A, "victor", "Victor's stage"},
        {0x00B, "lord_raptor", "Lord Raptor's stage"},
        {0x00C, "morrigan", "Morrigan's stage"},
        {0x00D, "anakaris", "Anakaris's stage"},
        {0x00E, "felicia", "Felicia's stage"},
        {0x00F, "bishamon", "Bishamon's stage"},
        {0x010, "rikuo", "Rikuo's stage"},
        {0x011, "sasquatch", "Sasquatch's stage"},
        {0x012, "huitzil", "Huitzil's stage"},
        {0x013, "pyron", "Pyron's stage"},
        {0x3EC, "hsien_ko", "Hsien-Ko's stage"},
        {0x3ED, "donovan", "Donovan's stage"},
        // Around the matches
        {0x02B, "qsound", "The QSound notice", true},
        {0x001, "introduction", "The introduction and the title"},
        {0x005, "ranking", "The Best 5 ranking", true},
        {0x003, "select", "Character select"},
        {0x02F, "next_opponent", "The next opponent chosen", true},
        {0x002, "versus", "Versus", true},
        {0x017, "winner", "Winner", true},
        {0x023, "ending_start", "After the last fight, before the ending"},
        {0x3F5, "ending", "Ending (Lord Raptor's)"},
        {0x02A, "staff_roll", "Staff roll"},
    };
}

} // namespace cps2recomp
