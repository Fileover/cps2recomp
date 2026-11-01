// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors: the board's settings (the operator's, in its test menu: Configuration, System and Game) as features
// on the launcher's Board page, as in the other games. Each is "As the board is set" unless chosen, and a choice is
// what the test menu would have set: the game reads its settings from the settings memory as the board starts and
// copies them into the variables below (01FC34 on); the hook at 01FCDC, where that copy returns, puts the choices in
// those variables. The board's settings memory itself is not changed, nor what the test menu shows and saves.
//
// Each variable (from A5) found by saving the test menu with that option alone changed and diffing the game's memory
// after the next start; the values' names read from the test menu, each option stepped through (the board's own in
// brackets):
//   1F1 difficulty   0-7: 1 easiest to 8 hardest (3: 4 medium)      203 mode select  0 off, 1 on (on)
//   263 damage level 0-8: 1 minimum to 9 maximum (4: 5 medium)      20A join in      0 off, 1 on (on)
//   233 time speed   0-7: 1 slow to 8 fast (3: 4 normal)            209 event        0 off, 1 on (off; on also sets join
//   212 turbo        0 off, 1 on, 2 free select (free select)                          in off, as the test menu does)
//   237 max round    0-2: 1, 3, 5 rounds (3 rounds)                 1EB continue     0 off, 1 on (on)
//                                                                   1F0 demo sound   0 off, 1 on (on)
#include "nwarr.h"

#include "machine.h"
#include "state.h"

#include <iterator>

namespace cps2recomp {

namespace {
    constexpr uint32_t kVars = 0xFF8000; // (A5)
    constexpr const char *kPage = "Board";

    struct Setting {
        const char *name;
        uint32_t variable;
        int first; // the value of the first choice after "As the board is set"
    };
    constexpr Setting kSettings[] = {
        {"board_difficulty", kVars + 0x1F1, 0}, {"board_damage", kVars + 0x263, 0},   {"board_timer", kVars + 0x233, 0},
        {"board_turbo", kVars + 0x212, 0},      {"board_rounds", kVars + 0x237, 0},   {"board_mode_select", kVars + 0x203, 0},
        {"board_join_in", kVars + 0x20A, 0},    {"board_event", kVars + 0x209, 0},    {"board_continue", kVars + 0x1EB, 0},
        {"board_demo_sound", kVars + 0x1F0, 0},
    };
    constexpr uint32_t kJoinIn = kVars + 0x20A, kEvent = kVars + 0x209;

    // The board's own values of what was chosen, to put back for its test menu. (Kept with the game's state,
    // BoardState: they say what the variables in RAM hold, so a state loaded or run-ahead's frame taken back must
    // bring them back with the RAM.)
    uint8_t g_board[std::size(kSettings)];
    bool g_chosen = false; // the choices are in the variables
    bool g_again = false;  // they were taken out for the test menu, and go back in when a game begins

    // The choices put in the variables (the board's own kept first)
    void PutChoices() {
        g_chosen = false;
        for (size_t n = 0; n < std::size(kSettings); n++) {
            g_board[n] = Ram8(kSettings[n].variable);
        }
        for (const Setting &setting : kSettings) {
            const int choice = FeatureValue(setting.name);
            if (choice > 0) {
                SetRam8(setting.variable, static_cast<uint8_t>(setting.first + choice - 1));
                g_chosen = true;
            }
        }
        if (FeatureValue("board_event") > 0 && Ram8(kEvent) && FeatureValue("board_join_in") == 0) {
            SetRam8(kJoinIn, 0); // (the event has no join in, as the test menu sets it)
        }
    }

    // 01FCDC: the board's settings have been copied to the variables
    bool SettingsRead(M68K &) {
        g_again = false;
        PutChoices();
        return false; // (and on with the game's own code)
    }
    // 0215F4: Start pressed, a game begins (the game's own routine, 02527C, is given here)
    bool GameBegins(M68K &) {
        if (g_again) {
            g_again = false;
            PutChoices();
        }
        return false;
    }
} // namespace

// Before every frame. The board's test switch pressed (its test menu shows, changes and saves the variables) puts the
// board's own values back, so that what the test menu saves is the board's; leaving the test menu does not read the
// settings again, so the choices go back in when a game next begins.
void BoardBeforeFrame(Machine &machine) {
    if (g_chosen && machine.testSwitch) {
        for (size_t n = 0; n < std::size(kSettings); n++) {
            SetRam8(kSettings[n].variable, g_board[n]);
        }
        g_chosen = false;
        g_again = true;
    }
}

void BoardState(Archive &ar) {
    ar.Mark(0x424F4152); // BOAR
    ar.Bytes(g_board, sizeof g_board);
    ar.Pod(g_chosen);
    ar.Pod(g_again);
}

void RegisterBoard() {
    Game &game = TheGame();
    game.pages.push_back({kPage, "", "The board's own settings, as its test menu has them (Configuration): each as the board is set "
                                     "unless chosen here. Taken when the game starts or is reset."});
    constexpr const char *kAsSet = "As the board is set";
    game.features.push_back({"board_difficulty", kPage, "Difficulty", "How well the computer fights.", 0,
                             {kAsSet, "1 Easiest", "2 Very easy", "3 Easy", "4 Medium", "5 Medium hard", "6 Hard", "7 Very hard", "8 Hardest"}});
    game.features.push_back({"board_damage", kPage, "Damage level", "How much damage a hit does, 1 (least) to 9.", 0,
                             {kAsSet, "1 Minimum", "2", "3", "4", "5 Medium", "6", "7", "8", "9 Maximum"}});
    game.features.push_back({"board_timer", kPage, "Time count speed", "How fast the round's clock runs, 1 (slowest) to 8.", 0,
                             {kAsSet, "1 Slow", "2", "3", "4 Normal", "5", "6", "7", "8 Fast"}});
    game.features.push_back({"board_turbo", kPage, "Turbo", "The game's speed: normal, turbo, or the players' choice.", 0,
                             {kAsSet, "Off", "On", "Free select"}});
    game.features.push_back({"board_rounds", kPage, "Max round", "Rounds a match lasts at most.", 0,
                             {kAsSet, "1 round", "3 rounds", "5 rounds"}});
    game.features.push_back({"board_mode_select", kPage, "Mode select",
                             "Whether players choose between normal and automatic guard.", 0, {kAsSet, "Off", "On"}});
    game.features.push_back({"board_join_in", kPage, "Join-in", "Whether a second player can join a game against the computer.", 0,
                             {kAsSet, "Off", "On"}});
    game.features.push_back({"board_event", kPage, "Event", "Off, or one match only (and no join-in).", 0, {kAsSet, "Off", "On"}});
    game.features.push_back({"board_continue", kPage, "Continue", "Whether a game over can be continued.", 0, {kAsSet, "Off", "On"}});
    game.features.push_back({"board_demo_sound", kPage, "Demo sound", "Whether the attract mode plays sound.", 0, {kAsSet, "Off", "On"}});
    AddHook(HookAt(nwarr::kSettingsCopied), "SettingsRead", SettingsRead);
    AddHook(HookAt(0x0215F4), "GameBegins", GameBegins);
}

} // namespace cps2recomp
