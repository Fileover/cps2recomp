// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Giga Wing: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The sound program (Capcom's
// driver 1.40) keeps a table of its songs at 0x9006, four bytes a song, after a word of how many there are (0xAF) at
// 0x9000; a song whose data starts with a 0 is music.
#include "gigawing.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0xAF;
}

} // namespace cps2recomp
