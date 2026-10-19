// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The
// sound program (driver 1.30, as Vampire Savior's) keeps a table of its songs at 0x9006, four bytes a song: the word
// at 0x9000 is how many (0x7ED), and a song whose data starts with a 0 is music. Its pieces are named by number until
// they are found by playing.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x7ED;
}

} // namespace cps2recomp
