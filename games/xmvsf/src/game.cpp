// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: what the game's own sources add to the port (see PROGRESS.md).
#include "alternative.h"
#include "battle.h"
#include "colours.h"
#include "game.h"
#include "session.h"
#include "tag.h"
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
void RegisterBattle(); // battle.cpp
void RegisterWidescreen(); // widescreen.cpp
void WidescreenBeforeFrame(Machine &machine);
bool WidePlay();
void ChoosingBeforeFrame(Machine &machine);
void RegisterTag(); // tag.cpp

// 001608, the game's random number: a long divs (001614, about 150 cycles), then the number kept (00161A). The board
// takes an interrupt between any two instructions, and the frame's interrupt, which asks for a random number of its own
// (00069A), often comes during the divs: it then reads the number before this one is kept. Translated code takes
// interrupts only where a block starts; a block starting after the divs (where nothing is done) lets it take this one
// where the board does, or the game's numbers part from the board's (seen at frame 5,227 of the ladder movie).
static void RandomNumberAsTheBoardDoes() {
    AddHook(HookAt(0x001618), "AfterRandomDivide", [](M68K &) { return false; });
}

void RegisterGame() {
    RandomNumberAsTheBoardDoes();
    RegisterModes();
    RegisterTraining();
    RegisterChoosing();
    RegisterModel();
    RegisterColours();
    RegisterBattle();
    RegisterWidescreen();
    RegisterTag();
    RegisterCoins();
    RegisterCheats();
    RegisterTitle();
    RegisterBoard();
    RegisterMusic();
    TheGame().beforeFrame = [](Machine &machine) {
        tag::BeforeFrame(machine);
        BoardBeforeFrame(machine);
        ChoosingBeforeFrame(machine);
        WidescreenBeforeFrame(machine);
        session::BeforeFrame(machine);
        modes::BeforeFrame(machine);
        BattleBeforeFrame(machine);
        colours::BeforeFrame(machine);
        ModelBeforeFrame(machine);
    };
    // (the training aids see what the frame did before the cheats put things back)
    TheGame().frame = [](Machine &machine) {
        session::Frame(machine);
        modes::Frame(machine);
        BattleFrame(machine);
        ModelFrame(machine);
        TitleFrame(machine);
        CoinsFrame(machine);
        tag::Frame(machine);
    };
    // (what the sources keep between frames, with save states and run-ahead's frames taken back)
    TheGame().state = [](Archive &ar) {
        BoardState(ar);
        session::State(ar);
        modes::State(ar);
        ModesState(ar);
        BattleState(ar);
        ModelState(ar);
    };
    // (run-ahead: not in the modes, whose workings a save state does not wholly hold)
    TheGame().noRunAhead = [] { return modes::AttractMode() || modes::StageViewer() || BattleMode() || ModelViewer(); };
    // With the arcade timing, the game's own code runs in place of the rewrites but where they carry what the port adds
    TheGame().rewritesNeeded = [] {
        return session::InSession() || modes::AttractMode() || modes::StageViewer() || BattleMode() || ModelViewer() || WidePlay() ||
               FeatureOn("widescreen");
    };
    // Its own, never set for the other games by the collection's launcher: Damage level has other choices in the Alpha
    // games (Low, Medium, Normal, High); the best scores and the animations are this game's
    OwnFeatures({"board_damage", "survival_best", "ringout_best", "model_animation", "colours_animation"});
}

} // namespace cps2recomp
