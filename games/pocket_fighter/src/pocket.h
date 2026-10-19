// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Gem Fighter Mini Mix (Pocket Fighter): names for what is known of the game's memory and routines, each found
// by experiment in this game (PROGRESS.md). Its variables are reached from A5 = FF8000, as in the Alpha games.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

#include <cstdint>

namespace pocket {

using cps2recomp::M68K;

// --- where the game is ---

// Words from A5, each a step (in twos) of the one before, as in the Alpha games: the first is 2 once a game is being
// played (and the board's start and its show step through it before: title.cpp); the next 2 while the players choose
// their characters and 4 during the match; under that the match's and the round's steps. Traced with --watch.
constexpr uint32_t kMode = 0xFF8000, kGameStep = 0xFF8004, kMatchStep = 0xFF8008, kRoundStep = 0xFF800C;

// The round's clock: seconds left, a byte (99 at a round's start; a second off at 0083E6, subq.b #1,0x189(a5)), with
// its BCD copy for the picture in the byte before (FF8188) and the frames left of the second after (FF818A, counted
// down at 0083DC). Found with --watch through a fight.
constexpr uint32_t kRoundClock = 0xFF8189, kClockFrames = 0xFF818A;

// Credits paid for and not yet used, a byte (RAM dumps after two, four and six coins: 1, 2, 3; a new board's settings
// memory asks two coins a credit). The coins toward the next credit are at FF8143.
constexpr uint32_t kCredits = 0xFF80F6;

// The game's own free play, one of the operator's coin modes (the test menu's System page: 00-08 one coin for 1-9
// credits, 09-10 two to nine coins a credit, 11 two coins to start and one to continue, 12 free play; 09 on a new
// board). The board's start reads the mode (001796) into kCoinMode, and for free play sets kFreePlay and the two coin
// words to 0101 (001814); the other modes leave kFreePlay clear and take their coin words from a table (00228C). The
// game tests kFreePlay wherever it counts credits.
constexpr uint32_t kCoinMode = 0xFF8117, kFreePlay = 0xFF811C, kCoinsPerCredit1 = 0xFF80F2, kCoinsPerCredit2 = 0xFF80FA;
constexpr uint8_t kCoinFreePlay = 0x12;

// --- the players ---

// Each player's record, 0x400 long, the first at FF8400 (as in the Alpha games)
constexpr uint32_t kPlayer1 = 0xFF8400, kPlayerSize = 0x400;
inline uint32_t Player(int n) {
    return kPlayer1 + static_cast<uint32_t>(n) * kPlayerSize;
}
// A player's health, a word (0x90 at a round's start, 0082F4; below nothing a knockout, 015CE6); +42 the bar shown.
// Every hit and throw takes its damage off at 015D6E (sub.w d4,0x40(a1)), the only writer seen with --watch on both
// players' health through fights and made-up play.
constexpr uint32_t kHealth = 0x40;
constexpr uint16_t kHealthFull = 0x90;

// --- for hooks ---

inline void SetLow16(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFF0000) | (value & 0xFFFF);
}

} // namespace pocket
