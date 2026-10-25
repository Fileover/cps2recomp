// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Super Street Fighter II Turbo (Super Street Fighter II X in Japan): names for what is known of the game's memory and
// routines, each found by experiment in this game (PROGRESS.md). Its variables are reached from A5 = FF8000, as in the
// Alpha games, but it is an older program: the steps, the players' records and the panel are laid out its own way,
// and nothing is taken from the later games unchecked. Some facts were first named by the FBNeo training script for
// this game (peon2/fbneo-training-mode, games/ssf2t); each was looked at again here.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

#include <cstdint>

namespace ssf2t {

using cps2recomp::M68K;

// --- where the game is ---

// Words from A5, each a step (in twos) of the one before. The first is 2 once a game is being played (the board's
// start and its show step through it before: title.cpp). The second is 0 while the players choose their characters
// and 4 during the match (2 for a frame between). The third is the match's: 2 and 4 the versus screen, 6 and 8 a
// round's introduction, 0A the round being fought, 0C to 10 its end (then 6 again for the next round). Traced with
// --watch through a one-player game.
constexpr uint32_t kMode = 0xFF8000, kGameStep = 0xFF8004, kMatchStep = 0xFF8008, kRoundStep = 0xFF800C;
enum : uint16_t { kModeGame = 2, kGameChoosing = 0, kGameMatch = 4, kMatchFought = 0x0A };

// Credits paid for and not yet used, a byte (RAM dumps after one, two and three coins: 1, 2, 3; written at 002418)
constexpr uint32_t kCredits = 0xFF82E9;

// Who is in: a byte, bit 0 player 1, bit 1 player 2 (RAM dumps before and after each Start: 0, 1, 3)
constexpr uint32_t kPlayersIn = 0xFF82F0;

// The game's own free play. The operator's coin mode (the test menu's System Configuration, Coin: 00 to 0E in twos
// one coin for one to six credits and two to four coins a credit, 10 a coin to start and one to continue) is kept in
// kCoinMode; the board's start (00823A, the record copied at 00831C) takes its two coin words from a table by it
// (008536) and sets kFreePlay for mode 12, which the menu never offers. The game tests kFreePlay wherever it counts
// credits (the title says FREE PLAY, and Start lets a player in). Seen with the byte poked to 1.
constexpr uint32_t kCoinMode = 0xFF8342, kFreePlay = 0xFF82DF, kCoinsPerCredit1 = 0xFF82AE, kCoinsPerCredit2 = 0xFF82B6;
constexpr uint8_t kCoinFreePlay = 0x12;
constexpr uint16_t kFreePlayCoins = 0x0101; // (the table's word for mode 12)

// The round's clock: seconds left, a BCD byte (99 at a round's start, 00D780), and the frames left of the second in the
// byte after it (40 a second here: 00FE74 counts it down, and at nothing a second is taken off at 00FE8A). Found with
// --watch through a fight; FBNeo's script names the first.
constexpr uint32_t kRoundClock = 0xFF8DCE, kClockFrames = 0xFF8DCF;
constexpr uint32_t kClockTick = 0x00FE74; // subq.b #1 off the second's frames

// --- the players ---

// Each player's record, 0x400 apart, the first at FF844E (A5+44E: cleared at 003616, 0x380 bytes each, with the
// second at A5+84E); FBNeo's script has them there too
constexpr uint32_t kPlayer1 = 0xFF844E, kPlayerSize = 0x400;
inline uint32_t Player(int n) {
    return kPlayer1 + static_cast<uint32_t>(n) * kPlayerSize;
}
// Health: a word, 0x90 full, below nothing knocked out (FFFF); the word after it is what it was before the last hit
// (07AC78 compares the two to see a hit has landed, and makes them alike at 07AD08)
constexpr uint32_t kHealth = 0x2A, kHealthBefore = 0x2C;
constexpr uint16_t kHealthFull = 0x90;

// --- routines (addresses in the program) ---

// A hit's damage off the victim's health: sub.w d6,0x2A(a2), D6 the damage, A2 the victim; 0BE646 in the hit of a
// player's attack (0BE580: A6 the attacker, A3 the attack) and 0BEBA6 in that of something thrown (0BEB0E: A6 what was
// thrown). Found counting the writers of both players' health over fights.
constexpr uint32_t kHitDamage = 0x0BE646, kThrownDamage = 0x0BEBA6;
// A throw's damage, D4 off the victim's (A3) health: at once (00490A, and 0049B0 in 00492C), or put by in the victim's
// word kDamagePut (004A58, in 004A4C: at once a knockout if it is more than the health) and taken off as the throw ends
// (004C98 half of it, 07AB6E, 07ACEA). Found as the other writers of the health over made-up play with no damage.
inline constexpr uint32_t kThrowDamage[] = {0x00490A, 0x0049B0, 0x004A58};
constexpr uint32_t kDamagePut = 0x78;

// --- for hooks ---

inline void SetLow16(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFF0000) | (value & 0xFFFF);
}
inline void SetLow8(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFFFF00) | (value & 0xFF);
}

} // namespace ssf2t
