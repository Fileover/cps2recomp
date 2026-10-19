// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#include "beat.h"

#include "state.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>

namespace cps2recomp::beat {

namespace {
    constexpr const char *kModes = "Alternative modes", *kAttract = "Attract mode", *kScore = "Score attack";
    constexpr int kStartMost = 9000; // frames after which getting to a game is given up
    constexpr int kShownFor = 6 * 60; // frames a score attack's words stay up

    Description g_d;

    enum class Mode : int32_t { None, Attract, Score };
    struct State {
        Mode mode = Mode::None;
        bool starting = false;
        int32_t frames = 0;
        // score attack: a credit put in, the game begun, its score as last seen, and the words shown (frames left)
        bool credited = false, played = false, over = false;
        uint32_t score = 0, best = 0;
        int32_t shown = 0;
        bool newBest = false;
    } g;
    bool g_fast = false;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }

    void End() {
        if (g.mode == Mode::None) {
            return;
        }
        g = State();
        Fast(false);
        SetHud({});
    }

    // --- the best score, beside settings.ini ---

    std::string BestPath() {
        const std::string &settings = SettingsPath();
        const size_t slash = settings.find_last_of('/');
        return (slash == std::string::npos ? std::string(".") : settings.substr(0, slash)) + "/score_attack.txt";
    }
    uint32_t ReadBest() {
        std::ifstream file(BestPath());
        std::string line;
        while (std::getline(file, line)) {
            unsigned long value = 0;
            if (std::sscanf(line.c_str(), "best = %lu", &value) == 1) {
                return static_cast<uint32_t>(value);
            }
        }
        return 0;
    }
    void WriteBest(uint32_t best) {
        if (SettingsPath().empty()) {
            return; // (a test run with no settings: nothing kept)
        }
        std::ofstream file(BestPath());
        file << "# The best score of the in-game menu's score attack\nbest = " << best << "\n";
    }

    std::string Grouped(uint32_t value) {
        std::string digits = std::to_string(value), text;
        for (size_t n = 0; n < digits.size(); n++) {
            if (n && (digits.size() - n) % 3 == 0) {
                text += ',';
            }
            text += digits[n];
        }
        return text;
    }

    // --- the modes ---

    void StartAttract(Machine &machine) {
        End();
        g.mode = Mode::Attract;
        g.starting = true;
        machine.Reset();
    }

    void StartScore(Machine &machine) {
        End();
        g.mode = Mode::Score;
        g.starting = true;
        g.best = ReadBest();
        machine.Reset();
    }

    void ScoreFrame() {
        if (g.over) {
            if (--g.shown <= 0) {
                End();
            }
            return;
        }
        if (g_d.inGame()) {
            // (the most it showed: a game clears the score before its game over is through)
            g.played = true;
            g.score = std::max(g.score, g_d.score());
        } else if (g.played) {
            // (the game is over)
            g.over = true;
            g.shown = kShownFor;
            g.newBest = g.score > g.best;
            if (g.newBest && !RunningAhead()) {
                g.best = g.score;
                WriteBest(g.best);
            }
        }
        if (g.shown > 0) {
            g.shown--;
        }
    }

    void ScoreHud() {
        if (g.mode != Mode::Score || g.starting) {
            return;
        }
        if (g.over) {
            const std::string text = g.newBest ? "A new best: " + Grouped(g.score) + "!"
                                               : "Score " + Grouped(g.score) + ", best " + Grouped(g.best);
            SetHud({{0.5f, 0.74f, 0, "Score attack over"}, {0.5f, 0.82f, 0, text}});
        } else if (g.shown > 0) {
            SetHud({{0.5f, 0.88f, 0, "Score attack: best " + Grouped(g.best)}});
        } else {
            SetHud({});
        }
    }
} // namespace

bool AttractMode() {
    return g.mode == Mode::Attract;
}
int AttractShow() {
    return g.mode == Mode::Attract ? FeatureValue("attract_show") : 0;
}
bool ScoreAttack() {
    return g.mode == Mode::Score;
}
bool Starting() {
    return g.mode != Mode::None && g.starting;
}
void EndModes() {
    End();
}

void BeforeFrame(Machine &machine) {
    if (g.mode == Mode::Attract) {
        // (Start or a coin ends it, and is the game's: a game can begin from the show)
        if ((machine.players[0] | machine.players[1] | machine.players[2]) & (kStart | kCoin)) {
            End();
            return;
        }
        for (uint16_t &held : machine.players) {
            held = 0;
        }
    } else if (g.mode == Mode::Score && g.starting) {
        // Unseen to the title, a credit in, and Start until the game takes it (the choice of character is the player's)
        Fast(true);
        for (uint16_t &held : machine.players) {
            held = 0;
        }
        if (++g.frames > kStartMost) {
            End();
            return;
        }
        if (!g.credited && g_d.canStart(machine)) {
            g_d.credit();
            g.credited = true;
        }
        if (g.credited) {
            // (Start until the credit is taken: the game is at its choice of character, which is seen)
            if (g_d.credits() == 0) {
                g.starting = false;
                g.shown = kShownFor;
                Fast(false);
            } else {
                machine.players[0] = (g.frames & 4) ? kStart : 0;
            }
        }
    } else if (g.mode == Mode::Score) {
        // (one player only: the others' Starts and coins are not taken)
        for (int n = 1; n < Machine::kMaxPlayers; n++) {
            machine.players[n] &= static_cast<uint16_t>(~(kStart | kCoin));
        }
        machine.players[0] &= static_cast<uint16_t>(~kCoin);
    }
}

void Frame(Machine &machine) {
    if (g.mode == Mode::Attract && g.starting) {
        // (the board's start, unseen unless everything is wanted)
        Fast(FeatureValue("attract_show") != 0);
        if (g_d.showBegun(machine) || ++g.frames > kStartMost) {
            g.starting = false;
            Fast(false);
        }
    } else if (g.mode == Mode::Score && !g.starting) {
        ScoreFrame();
    }
    ScoreHud();
}

void State(Archive &ar) {
    ar.Mark(0x42454154); // BEAT
    ar.Pod(g);
    if (!ar.Saving() && !RunningAhead()) {
        g_fast = !g.starting; // (put the window's running unseen as the state has it)
        Fast(g.starting);
    }
}

void Register(const Description &description) {
    g_d = description;
    Game &game = TheGame();
    game.pages.push_back({kModes, "", "Other ways to run the game: attract mode, score attack, viewers for the stages and the characters.", true});
    game.pages.push_back({kAttract, kModes, "The game's own show on a loop, with no coin in and its sound on: all of it, or one part."});
    game.pages.push_back({kScore, kModes, "One player and one credit from the first round, for the best score."});
    game.commands.push_back({"attract_start", kAttract, "Start",
                             "Starts attract mode: the game's own show, over and over. Start (or a coin) ends it.", StartAttract});
    std::vector<const char *> shows = {"Everything"};
    shows.insert(shows.end(), description.shows.begin(), description.shows.end());
    game.features.push_back({"attract_show", kAttract, "Shows", description.showsWords, 0, shows});
    game.commands.push_back({"score_start", kScore, "Start",
                             "Starts a game for player 1 alone: one credit and no continue, from the first round, with free play, "
                             "the cheats and \"Start at round\" left out. The best score is kept. The game over ends it.",
                             StartScore});
}

} // namespace cps2recomp::beat
