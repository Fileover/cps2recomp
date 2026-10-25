// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Street Fighter II Turbo: its music, for a custom soundtrack (port/soundtrack.h, machine/soundtrack.h). The
// sound program (driver 1.04) keeps a table of its songs at 0x9006, four bytes a song, as the later drivers do: the
// word at 0x9000 is how many (0x1FF), and a song whose data starts with a 0 is music (68 of them).
#include "game.h"

namespace cps2recomp {

void RegisterMusic() {
    TheGame().songTable = 0x9006;
    TheGame().songCount = 0x1FF;
}

} // namespace cps2recomp
