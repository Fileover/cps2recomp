// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Giga Wing: what the game's own sources add to the port (see PROGRESS.md): free play, straight to the title, the
// cheats, and the modes (modes.cpp, practice.cpp).
#include "gigawing.h"
#include "shmup.h"
#include "state.h"

namespace cps2recomp {

void RegisterBoard(); // board.cpp
void BoardFrame(Machine &machine);
void RegisterMusic(); // music.cpp
void RegisterModes(); // modes.cpp
bool ChoicesLeftOut();
void ModesBeforeFrame(Machine &machine);
void ModesFrame(Machine &machine);
void ModesState(Archive &ar);

namespace {
    using namespace gigawing;

    // "Straight to the title": the board's start, its warning, the logos and the story run unseen. (Kept with the
    // state, as everything that changes while the game runs: online play runs frames again from it.)
    struct Title {
        bool skipping = false, fast = false;
        int32_t frames = 0;
    } g_title;

    void TitleFrame(Machine &machine) {
        // (a mode of the menu started: its start is its own, and so is running unseen)
        if (ChoicesLeftOut()) {
            g_title.skipping = false;
        } else {
            if (machine.frame <= 1 && !g_title.skipping) {
                g_title.skipping = FeatureOn("skip_intro");
                g_title.frames = 0;
            }
            const uint16_t mode = Ram16(kMode);
            const bool there = mode == kModeTitle || mode == kModeChoose || mode == kModePlay || Ram8(kPlayersIn) != 0;
            if (g_title.skipping && (there || ++g_title.frames > 6000)) {
                g_title.skipping = false;
            }
        }
        if (g_title.skipping != g_title.fast) {
            g_title.fast = g_title.skipping;
            SetFastForward(g_title.fast);
        }
    }

    void FreePlay() {
        if (FeatureOn("free_play") && !ChoicesLeftOut()) {
            SetRam8(kFreePlay, 1);
            return;
        }
        // (off: the game's own, from the coin mode, as its settings routine has it)
        const uint8_t own = Ram8(kCoinMode) == kCoinModeFreePlay ? 1 : 0;
        if ((Ram8(kFreePlay) != 0) != (own != 0)) {
            SetRam8(kFreePlay, own);
        }
    }

    bool PlayerIn(int n) {
        return (Ram8(kPlayersIn) >> n) & 1;
    }

    const char *const kInvincible[kPlayers] = {"invincible", "invincible_2"};

    // A ship is out of harm: the cheat, or a mode running the game unseen
    bool Invincible(int n) {
        return (FeatureOn(kInvincible[n]) && !ChoicesLeftOut()) || shmup::Protected();
    }

    // 02398C: a ship is hit (A2 the ship): not with invincibility, whose ship goes on as if missed
    bool ShipHit(M68K &c) {
        const uint32_t ship = c.a[2];
        for (int n = 0; n < kPlayers; n++) {
            if (ship == Ship(n) && Invincible(n)) {
                // (moveq #0,d0: no hit, and the shot goes on through; 029DE8 tests the flags)
                c.d[0] = 0;
                c.flagZ = true;
                c.flagN = c.flagV = c.flagC = false;
                Return(c);
                return true;
            }
        }
        return false;
    }

    // 023AAE: a ship (A0) touches an enemy's body: not with invincibility (the enemy is still marked as touched)
    bool ShipTouched(M68K &c) {
        const uint32_t ship = c.a[0];
        for (int n = 0; n < kPlayers; n++) {
            if (ship == Ship(n) && Invincible(n)) {
                c.pc = kShipTouched + 4;
                return true;
            }
        }
        return false;
    }

    // 04AE98: a force bomb is taken: not with infinite bombs (A1 points at the count; the bomb goes off all the same)
    bool BombTaken(M68K &c) {
        if (FeatureOn("infinite_bombs") && !ChoicesLeftOut()) {
            c.pc = kBombTaken + 2;
            return true;
        }
        return false;
    }

    // 008488: a game's start has put the stage at 0 (it is loaded when the stage begins, after the choice of fighter)
    bool GameBegins(M68K &) {
        const int stage = shmup::WantedStage() >= 0 ? shmup::WantedStage() : ChoicesLeftOut() ? 0 : FeatureValue("start_stage");
        if (stage > 0 && stage < kStages) {
            SetRam16(kStage, static_cast<uint16_t>(stage));
        }
        return false;
    }

    void Cheats() {
        if (Ram16(kMode) != kModePlay) {
            return;
        }
        for (int n = 0; n < kPlayers; n++) {
            if (FeatureOn("infinite_ships") && PlayerIn(n) && Ram16(kShips + 2 * n) < 2) {
                SetRam16(kShips + 2 * n, 2);
            }
        }
    }

    void Frame(Machine &machine) {
        BoardFrame(machine);
        ModesFrame(machine);
        FreePlay();
        if (!ChoicesLeftOut()) {
            Cheats(); // (free play and the cheats are not the board's own game)
        }
        TitleFrame(machine);
    }

    void State(Archive &ar) {
        ModesState(ar);
        ar.Mark(0x47494741); // GIGA
        ar.Pod(g_title);
    }
} // namespace

void RegisterGame() {
    Game &game = TheGame();
    RegisterMusic();
    RegisterBoard();
    RegisterModes();
    game.features.push_back({"free_play", "Game", "Free play", "Start works without a coin (the board's own free play).", kFeatureEnhanced});
    game.features.push_back({"skip_intro", "Game", "Straight to the title",
                             "When the game starts, and when it is reset, goes straight to the title: the board's start, its warning, "
                             "the logos and the story are run unseen.",
                             0});
    game.features.push_back({"infinite_ships", "Cheats", "Infinite ships", "A player who loses a ship always has another."});
    game.features.push_back({"infinite_bombs", "Cheats", "Infinite force bombs", "A force bomb goes off without being used up."});
    game.features.push_back({"invincible", "Cheats", "Invincibility", "Player 1's ship is never hit."});
    game.features.push_back({"invincible_2", "Cheats", "Invincibility, player 2", "Player 2's ship is never hit."});
    game.features.push_back({"start_stage", "Game", "Start at stage", "A new game starts at the beginning of the stage chosen.", 0,
                             {"Stage 1", "Stage 2", "Stage 3", "Stage 4", "Stage 5", "Stage 6", "Stage 7"}});
    AddHook(HookAt(kGameBegins), "GameBegins", GameBegins);
    AddHook(HookAt(kShipHitAnswer), "ShipHit", ShipHit);
    AddHook(HookAt(kShipTouched), "ShipTouched", ShipTouched);
    AddHook(HookAt(kBombTaken), "BombTaken", BombTaken);
    game.beforeFrame = ModesBeforeFrame;
    game.frame = Frame;
    game.state = State;
}

} // namespace cps2recomp
