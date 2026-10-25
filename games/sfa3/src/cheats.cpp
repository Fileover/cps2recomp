// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 3: cheats (optional features: the launcher's Features page and the in-game menu switch them).
// Made-up play needs them to go up the ladder to its end ('make explore', game.cfg's explore features).
#include "session.h"
#include "sfa3.h"

namespace cps2recomp {

namespace {
    using namespace sfa3;

    // Which player's record an address is (0, 1), or -1
    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // 0240D2: `damage` (D4, its low word) is about to come off the health of the player whose record is A1 (anything
    // else's is left alone). Health below nothing after it is a knockout, so one-hit knockouts take off more than a
    // full bar; with no knockouts (a training session, the modes that want it) a hit takes no more than the health
    // there is.
    bool TakeDamage(M68K &c) {
        const int player = PlayerOf(c.a[1]);
        if (player < 0) {
            return false;
        }
        uint32_t &damage = c.d[4];
        if (FeatureOn(player == 0 ? "no_damage" : "no_damage_2")) {
            SetLow16(damage, 0);
        } else if (player == 1 && FeatureOn("one_hit")) {
            SetLow16(damage, kHealthFull + 1);
        }
        const int16_t health = static_cast<int16_t>(Ram16(Player(player) + kHealth));
        if (session::NoKnockOut() && static_cast<int16_t>(damage & 0xFFFF) > health) {
            SetLow16(damage, static_cast<uint32_t>(health > 0 ? health : 0));
        }
        return false;
    }
} // namespace

void RegisterCheats() {
    AddHook(HookAt(kTakeDamage), "TakeDamage", TakeDamage);
    // 009BFE: subq.b #1 off the frames of the round's second (at nothing a second goes). With the cheat the frame is
    // given back first, so the clock stands still.
    AddHook(HookAt(kClockTick), "ClockTick", [](M68K &) {
        if (FeatureOn("infinite_time") || session::NoClock()) {
            SetRam8(kClockFrames, static_cast<uint8_t>(Ram8(kClockFrames) + 1));
        }
        return false;
    });
    Game &game = TheGame();
    game.features.push_back({"no_damage", "Cheats", "No damage", "Player 1's health does not go down."});
    game.features.push_back({"no_damage_2", "Cheats", "No damage, player 2", "Player 2's health does not go down (the opponent's, in a one-player game)."});
    game.features.push_back({"one_hit", "Cheats", "One-hit knockouts", "Player 2 (the opponent, in a one-player game) goes down with the first hit."});
    game.features.push_back({"infinite_time", "Cheats", "Infinite time", "The round's clock stands still."});
}

} // namespace cps2recomp
