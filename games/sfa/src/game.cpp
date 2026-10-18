// SPDX-License-Identifier: GPL-3.0-only
//
// Street Fighter Alpha: what the game's own sources add to the port (see PROGRESS.md).
#include "game.h"

namespace cps2recomp {

void RegisterCoins(); // coins.cpp
void CoinsFrame(Machine &machine);
void RegisterCheats(); // cheats.cpp

void RegisterGame() {
    RegisterCoins();
    RegisterCheats();
    TheGame().frame = [](Machine &machine) {
        CoinsFrame(machine);
    };
}

} // namespace cps2recomp
