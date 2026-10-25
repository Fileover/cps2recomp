// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Street Fighter II Turbo: a player handed to the game's computer in the middle of a round (the fighting modes:
// fight/battle.h, which say who plays a fighter each frame by its byte kPerson).
//
// A player's routine (078030) runs a person's step (078270) or the computer's (07BB22) by that byte, and both keep how
// far on they are in the record's kFighting byte: 0 at a round's start, then 2 for the fight. The computer's first
// step (07BB34) sets up what the computer plays by (its tables for the character and the plan it follows: 07D6F4,
// 07E36E, 07BB82, the two words from 24EAE8), which a person's never does; a player handed over at 2 goes straight to
// the computer's fight step with none of it, and the computer's code reads its plan from nowhere (the game stops). So
// where the computer's step is about to run (07804A) for a player whose last step was a person's, that set-up is made
// first, as 07BB34 makes it. In the game itself a player only changes hands between rounds, at 0: this never runs.
#include "ssf2t.h"

#include "state.h"

#include <algorithm>
#include <iterator>

namespace cps2recomp {

namespace {
    using namespace ssf2t;

    constexpr uint32_t kPersonStep = 0x078036, kComputerStep = 0x07804A, kComputerStart = 0x07BB34;
    constexpr uint32_t kCharacterTables = 0x07D6F4, kPlan = 0x07E36E, kPlanRound = 0x07BB82;
    constexpr uint32_t kPlanWords = 0x24EAE8; // (two words a character, to +23A and +23C)
    bool g_computerReady[2] = {false, false}; // the computer's set-up is in the player's record (kept with the game's state)

    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // 07BB34 (the computer's first step of a round), 078036 (a person's step) and 07804A (the computer's, to come)
    bool ComputerStart(M68K &c) {
        const int n = PlayerOf(c.a[6]);
        if (n >= 0) {
            g_computerReady[n] = true;
        }
        return false;
    }
    bool PersonStep(M68K &c) {
        const int n = PlayerOf(c.a[6]);
        if (n >= 0) {
            g_computerReady[n] = false;
        }
        return false;
    }
    bool ComputerStep(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        const int n = PlayerOf(player);
        if (n < 0 || g_computerReady[n] || Ram8(player + kFighting) != 2) {
            return false;
        }
        // (07BB40-07BB4A, 07BB58 and 07BB5C-07BB74, the animation it starts left out: the player is in the middle of
        // whatever it was doing)
        uint32_t d[8], a[8];
        std::copy(std::begin(c.d), std::end(c.d), d);
        std::copy(std::begin(c.a), std::end(c.a), a);
        Call(c, kCharacterTables);
        SetRam8(player + 0x273, 2);
        Call(c, kPlan);
        Call(c, kPlanRound);
        const uint32_t words = kPlanWords + static_cast<uint32_t>(Ram8(player + kCharacter)) * 4;
        SetRam16(player + 0x23A, static_cast<uint16_t>(c.Read(words, 1)));
        SetRam16(player + 0x23C, static_cast<uint16_t>(c.Read(words + 2, 1)));
        std::copy(std::begin(d), std::end(d), c.d);
        std::copy(std::begin(a), std::end(a), c.a);
        g_computerReady[n] = true;
        return false;
    }
} // namespace

void ComputerState(Archive &ar) {
    ar.Mark(0x434F4D50); // COMP
    ar.Pod(g_computerReady);
}

void RegisterComputer() {
    AddHook(HookAt(kComputerStart), "ComputerStart", ComputerStart);
    AddHook(HookAt(kPersonStep), "PersonStep", PersonStep);
    AddHook(HookAt(kComputerStep), "ComputerStep", ComputerStep);
}

} // namespace cps2recomp
