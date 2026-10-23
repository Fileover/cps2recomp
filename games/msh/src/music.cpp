// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Marvel Super Heroes: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound program
// (driver 1.05) keeps a table of its songs at 0x9006, four bytes a song, as Street Fighter Alpha's 1.05A does: the
// word at 0x9000 is how many (0x400), and a song whose data starts with a 0 is music.
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x400;
}

} // namespace cps2recomp
