// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors: cheats (optional features: the launcher's Features page and the in-game menu switch them). Made-up
// play needs them to go up the ladder to its end ('make explore', game.cfg's explore features). As Pocket Fighter's.
#include "nwarr.h"
#include "session.h"

namespace cps2recomp {

namespace {
    using namespace nwarr;

    // Which player's record an address is (0, 1), or -1
    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // Every place the game takes health off a player (lst.py -r 'sub.w d4,0x42(aN)|subq.w #1,0x42(aN)': 35 of them,
    // the register the player is in, and whether it is a subq of one): a hit's (0C1384, the one most hits go
    // through), a throw's and a character's own moves' (each in its own routine), the thrown player's (A6), and the
    // slow drain of damage still to come (02C184: the word at +188 taken a point at a time). Health below nothing
    // after it is a knockout (0C1388), so one-hit knockouts take off more than a full bar; with no knockouts (a
    // training session, the modes that want it) a hit takes no more than the health there is.
    struct Site {
        uint32_t address;
        int reg;
        bool one; // subq.w #1
    };
    constexpr int a4 = 4, a6 = 6;
    constexpr Site kSites[] = {
        {HookAt(At(0x02C184)), a6, true}, {HookAt(At(0x02C738)), a4, false}, {HookAt(At(0x02ED20)), a6, false}, {HookAt(At(0x02F3F4)), a6, false},
        {HookAt(At(0x03029A)), a6, false}, {HookAt(At(0x0302C6)), a6, false}, {HookAt(At(0x0303BC)), a6, false}, {HookAt(At(0x03043A)), a6, false},
        {HookAt(At(0x030626)), a6, false}, {HookAt(At(0x030636)), a6, false}, {HookAt(At(0x0306F8)), a6, false}, {HookAt(At(0x0307BA)), a6, false},
        {HookAt(At(0x03096E)), a6, false}, {HookAt(At(0x030C38)), a6, false}, {HookAt(At(0x030E86)), a6, false}, {HookAt(At(0x031074)), a6, false},
        {HookAt(At(0x03112A)), a6, false}, {HookAt(At(0x03787A)), a4, false}, {HookAt(At(0x03AD7C)), a4, false}, {HookAt(At(0x03BD18)), a4, false},
        {HookAt(At(0x0421A0)), a4, false}, {HookAt(At(0x043C2C)), a4, false}, {HookAt(At(0x04A2EC)), a4, false}, {HookAt(At(0x04A4D4)), a4, false},
        {HookAt(At(0x04A5FE)), a4, false}, {HookAt(At(0x04C8E8)), a4, false}, {HookAt(At(0x04D528)), a4, false}, {HookAt(At(0x04D57C)), a4, false},
        {HookAt(At(0x04F3A0)), a4, false}, {HookAt(At(0x04F47E)), a4, false}, {HookAt(At(0x052142)), a4, false}, {HookAt(At(0x06811A)), a6, true},
        {HookAt(At(0x0681B4)), a6, true}, {HookAt(At(0x0C1384)), a4, false}, {HookAt(At(0x0DA878)), a6, true},
    };

    bool Damage(M68K &c) {
        const Site *found = nullptr;
        for (const Site &site : kSites) {
            found = site.address == c.pc ? &site : found;
        }
        if (!found) {
            return false;
        }
        const Site &site = *found;
        const int player = PlayerOf(c.a[site.reg]);
        if (player < 0) {
            return false;
        }
        const uint32_t health = Player(player) + kHealth;
        const bool kept = FeatureOn(player == 0 ? "no_damage" : "no_damage_2");
        if (site.one) {
            // (the drain: the point is given back first, to the bar's word beside it too)
            if (kept || (session::NoKnockOut() && static_cast<int16_t>(Ram16(health)) <= 1)) {
                SetRam16(health, static_cast<uint16_t>(Ram16(health) + 1));
                SetRam16(health + 2, static_cast<uint16_t>(Ram16(health + 2) + 1));
            }
            return false;
        }
        if (c.d[4] & 0xFFFF) {
            session::Hit(player, c.d[4] & 0xFFFF);
        }
        if (kept) {
            SetLow16(c.d[4], 0);
        } else if (player == 1 && FeatureOn("one_hit")) {
            SetLow16(c.d[4], kHealthFull + 1);
        }
        const int16_t left = static_cast<int16_t>(Ram16(health));
        if (session::NoKnockOut() && static_cast<int16_t>(c.d[4] & 0xFFFF) > left) {
            SetLow16(c.d[4], static_cast<uint32_t>(left > 0 ? left : 0));
        }
        return false;
    }
} // namespace

void RegisterCheats() {
    for (const Site &site : kSites) {
        AddHook(site.address, "Damage", Damage);
    }
    // 008798: subq.b #1 off the frames left of the round's second; at nothing a second comes off the clock (0087AC)
    // and the frames start again. With the cheat the frame is given back first, so the clock stands still.
    AddHook(HookAt(At(0x008798)), "ClockFrame", [](M68K &) {
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
