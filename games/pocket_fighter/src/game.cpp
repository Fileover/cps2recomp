// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix: what the game's own sources add to the port (see PROGRESS.md).
#include "game.h"
#include "battle.h"
#include "pocket.h"
#include "modes.h"
#include "session.h"

namespace {
cps2recomp::Machine *g_board = nullptr;
} // namespace

namespace pocket {

cps2recomp::Machine &Board() {
    return *g_board;
}

int HalfWidth(uint32_t player) {
    using namespace cps2recomp;
    const uint8_t push = Ram8(player + kPush);
    if (!push || !g_board) {
        return 0;
    }
    return static_cast<int>(g_board->cpu.Read(Ram32(player + kPushTable) + push * kBoxSize + kBoxWidth, 1)) / 2;
}

} // namespace pocket

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
void RegisterModes(); // modes.cpp
void RegisterTraining(); // training.cpp
void RegisterBattle(); // battle.cpp

void RegisterGame() {
    RegisterModes();
    RegisterTraining();
    RegisterBattle();
    RegisterCoins();
    RegisterCheats();
    RegisterTitle();
    RegisterBoard();
    RegisterMusic();
    TheGame().beforeFrame = [](Machine &machine) {
        g_board = &machine;
        BoardBeforeFrame(machine);
        session::BeforeFrame(machine);
        modes::BeforeFrame(machine);
        BattleBeforeFrame(machine);
    };
    // (the training aids see what the frame did before the cheats put things back)
    TheGame().frame = [](Machine &machine) {
        session::Frame(machine);
        modes::Frame(machine);
        BattleFrame(machine);
        TitleFrame(machine);
        CoinsFrame(machine);
    };
    // (run-ahead: not in the modes, whose workings a save state does not wholly hold)
    TheGame().noRunAhead = [] { return modes::AttractMode() || modes::StageViewer() || BattleMode(); };
    // (what the sources keep between frames, with save states and run-ahead's frames taken back)
    TheGame().state = [](Archive &ar) {
        BoardState(ar);
        session::State(ar);
        modes::State(ar);
        BattleState(ar);
    };
    // Its own, never set for the other games by the collection's launcher: Damage level has Super Puzzle Fighter II
    // Turbo's choices (Low, Normal, High, Maximum), where it means the counter gems an attack sends
    OwnFeatures({"board_damage", "survival_best", "ringout_best"});
}

} // namespace cps2recomp
