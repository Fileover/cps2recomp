// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 3: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound
// program (driver 1.71) keeps a table of its songs at 0x9006, four bytes a song, as the earlier drivers do, but its
// music's data begin as its effects' do: the game asks for its tunes from 00 to 2F (each after the command FF07), and
// the songs from 30 on are effects and voices (machine/soundtrack.h, musicBelow).
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0xAC0;
    TheGame().musicBelow = 0x30;
}

} // namespace cps2recomp
