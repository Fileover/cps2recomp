// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: what the game's own sources add to the port (see PROGRESS.md).
#include "alternative.h"
#include "colours.h"
#include "game.h"
#include "session.h"
#include "xmvsf.h"

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
void ModesState(Archive &ar);
void RegisterTraining(); // training.cpp
void RegisterChoosing(); // choosing.cpp
void RegisterModel(); // model.cpp
void ModelBeforeFrame(Machine &machine);
void ModelFrame(Machine &machine);
void ModelState(Archive &ar);
bool ModelViewer();
void RegisterColours(); // colours.cpp
void ChoosingBeforeFrame(Machine &machine);

void RegisterGame() {
    RegisterModes();
    RegisterTraining();
    RegisterChoosing();
    RegisterModel();
    RegisterColours();
    RegisterCoins();
    RegisterCheats();
    RegisterTitle();
    RegisterBoard();
    RegisterMusic();
    TheGame().beforeFrame = [](Machine &machine) {
        BoardBeforeFrame(machine);
        ChoosingBeforeFrame(machine);
        session::BeforeFrame(machine);
        modes::BeforeFrame(machine);
        colours::BeforeFrame(machine);
        ModelBeforeFrame(machine);
    };
    // (the training aids see what the frame did before the cheats put things back)
    TheGame().frame = [](Machine &machine) {
        session::Frame(machine);
        modes::Frame(machine);
        ModelFrame(machine);
        TitleFrame(machine);
        CoinsFrame(machine);
    };
    // (what the sources keep between frames, with save states and run-ahead's frames taken back)
    TheGame().state = [](Archive &ar) {
        BoardState(ar);
        session::State(ar);
        modes::State(ar);
        ModesState(ar);
        ModelState(ar);
    };
    // (run-ahead: not in the modes, whose workings a save state does not wholly hold)
    TheGame().noRunAhead = [] { return modes::AttractMode() || modes::StageViewer() || ModelViewer(); };
    // With the arcade timing, the game's own code runs in place of the rewrites but where they carry what the port adds
    TheGame().rewritesNeeded = [] { return session::InSession() || modes::AttractMode() || modes::StageViewer() || ModelViewer(); };
    // Its own, never set for the other games by the collection's launcher: Damage level has other choices in the Alpha
    // games (Low, Medium, Normal, High), and the animations are this game's
    OwnFeatures({"board_damage", "model_animation", "colours_animation"});
}

} // namespace cps2recomp
