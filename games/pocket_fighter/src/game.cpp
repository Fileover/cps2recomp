// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: what the game's own sources add to the port (see PROGRESS.md).
#include "game.h"

namespace cps2recomp {

void RegisterCoins(); // coins.cpp
void CoinsFrame(Machine &machine);
void RegisterCheats(); // cheats.cpp
void RegisterTitle(); // title.cpp
void TitleFrame(Machine &machine);
void RegisterBoard(); // board.cpp
void BoardBeforeFrame(Machine &machine);
void BoardState(Archive &ar);
void RegisterMusic(); // music.cpp

void RegisterGame() {
    RegisterCoins();
    RegisterCheats();
    RegisterTitle();
    RegisterBoard();
    RegisterMusic();
    TheGame().beforeFrame = [](Machine &machine) {
        BoardBeforeFrame(machine);
    };
    TheGame().frame = [](Machine &machine) {
        TitleFrame(machine);
        CoinsFrame(machine);
    };
    // (what the sources keep between frames, with save states and run-ahead's frames taken back)
    TheGame().state = [](Archive &ar) {
        BoardState(ar);
    };
    // Its own, never set for the other games by the collection's launcher: Damage level has Super Puzzle Fighter II
    // Turbo's choices (Low, Normal, High, Maximum), where it means the counter gems an attack sends
    OwnFeatures({"board_damage"});
}

} // namespace cps2recomp
