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
// where the computer's step is about to run (07804A) for a player with no plan in its record (the long kPlan: the
// record is cleared as a match begins, 003616, and only the computer's set-up writes it, 07E3A8), that set-up is made
// first, as 07BB34 makes it. In the game itself a player only changes hands between rounds, at 0: this never runs.
#include "ssf2t.h"

#include <algorithm>
#include <iterator>

namespace cps2recomp {

namespace {
    using namespace ssf2t;

    constexpr uint32_t kComputerStep = 0x07804A;
    constexpr uint32_t kCharacterTables = 0x07D6F4, kPlan = 0x07E36E, kPlanRound = 0x07BB82;
    constexpr uint32_t kPlanWords = 0x24EAE8; // (two words a character, to +23A and +23C)
    constexpr uint32_t kPlanTable = 0x1E0;    // a player's long: the computer's plan for the character (07E3A8)

    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // 07804A: the computer's step is to come for the player at A6
    bool ComputerStep(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        if (PlayerOf(player) < 0 || Ram32(player + kPlanTable) != 0 || Ram8(player + kFighting) != 2) {
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
        return false;
    }
} // namespace

void RegisterComputer() {
    AddHook(HookAt(kComputerStep), "ComputerStep", ComputerStep);
}

} // namespace cps2recomp
