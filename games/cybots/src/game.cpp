// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cyberbots: Fullmetal Madness: what the game's own sources add to the port (see PROGRESS.md).
#include "game.h"
#include "cybots.h"
#include "alternative.h"
#include "battle.h"
#include "session.h"
#include "state.h"

namespace cps2recomp {

void RegisterCoins(); // coins.cpp
void CoinsFrame(Machine &machine);
void CoinsState(Archive &ar);
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
void RingOutState(Archive &ar);

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
        BoardBeforeFrame(machine);
        session::BeforeFrame(machine);
        modes::BeforeFrame(machine);
        BattleBeforeFrame(machine);
    };
    TheGame().frame = [](Machine &machine) {
        session::Frame(machine);
        modes::Frame(machine);
        BattleFrame(machine);
        TitleFrame(machine);
        CoinsFrame(machine);
    };
    // (what the sources keep between frames, with save states and run-ahead's frames taken back)
    TheGame().state = [](Archive &ar) {
        BoardState(ar);
        CoinsState(ar);
        session::State(ar);
        modes::State(ar);
        BattleState(ar);
        RingOutState(ar);
    };
    // (run-ahead: not in the modes, whose workings a save state does not wholly hold)
    TheGame().noRunAhead = [] { return modes::AttractMode() || modes::StageViewer() || BattleMode(); };
    // With the arcade timing, the game's own code runs in place of the rewrites but where they carry what the port adds
    TheGame().rewritesNeeded = [] { return session::InSession() || modes::AttractMode() || modes::StageViewer() || BattleMode(); };
    // Its own: the names and choices are this board's, never set for the other games by the collection's launcher
    OwnFeatures({"board_difficulty", "board_damage", "board_timer"});
}

} // namespace cps2recomp
