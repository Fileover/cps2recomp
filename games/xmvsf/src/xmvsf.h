// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: names for what is known of the game's memory and routines, each found by experiment in
// this game (PROGRESS.md). A5 is FF8000 as in Capcom's other CPS-2 games, but this game keeps its variables below it
// (A5-3000 on), and its players' records at FF4000.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

#include <cstdint>

namespace xmvsf {

using cps2recomp::M68K;

// --- where the game is ---

// A byte: 8 while a round is being fought (the damage routines test it before taking health: 015568, 015B58)
constexpr uint32_t kFight = 0xFF5000;
enum : uint8_t { kFighting = 0x08 };

// A byte, set once a game is being played (077FA2, after Start), and the byte after it: who is in, bit 0 player 1,
// bit 1 player 2 (078044 sets a player's bit as Start takes its credit; the attract mode's fights set 3)
constexpr uint32_t kPlaying = 0xFF5100, kPlayersIn = 0xFF5101;
// Words, each a step of the one before (traced with --watch over a game from Start): the game's (4 while the players
// choose their characters, 8 in a match: 00A3EA, 00B142), and the match's (2 then 4 as its first round begins)
constexpr uint32_t kGameStep = 0xFF5104, kMatchStep = 0xFF5106;
enum : uint16_t { kGameChoosing = 4, kGameMatch = 8 };

// The round's clock: seconds left, a BCD byte (99 at a round's start, 00A4AC; a second off at 00FB2C, sbcd), and the
// frames left of the second after it (counted down at 00FB14; set again from a table by the timer speed, 00FB3A).
// Found by RAM traces through a fight, then --watch.
constexpr uint32_t kRoundClock = 0xFF5008, kClockFrames = 0xFF5009;

// Credits paid for and not yet used, a byte (RAM dumps after one to four coins: 1-4; nine at most, 07811A)
constexpr uint32_t kCredits = 0xFF545F;

// The game's own free play, one of the operator's coin modes (the test menu's System page, its first option: 00-08
// one coin for 1-9 credits, ..., 12 free play). The board's start reads the settings record (kSettings, 079E7C) and
// keeps the mode in kCoinMode; free play sets kFreePlay (079F38), which the code that lets a player in tests in place
// of the credits (078016). Set alone it is the whole of free play: the title says FREE.
constexpr uint32_t kCoinMode = 0xFF5469, kFreePlay = 0xFF5460;
constexpr uint8_t kCoinFreePlay = 0x12;

// The operator's settings as the settings memory holds them, read into the game's variables at the board's start
// (079ECA on; called from 000E20, so the call returns to 000E26, where board.cpp and title.cpp hook). The offsets
// and where each goes are in board.cpp. The attract mode places the title's picture (its parts 4, 5 and 6, 06BA02)
// at 075BF2: the show has reached the title.
constexpr uint32_t kSettings = 0xFFFAA4;

// --- the players ---

// Four records, 0x400 long: player 1's first character at FF4000, player 2's at FF4400, then their second characters
// at FF4800 and FF4C00 (named in the FBNeo training script for this set; confirmed here by the health each shows)
constexpr uint32_t kPlayer1 = 0xFF4000, kPlayerSize = 0x400, kPartners = 0x800;
inline uint32_t Player(int n) {
    return kPlayer1 + static_cast<uint32_t>(n) * kPlayerSize;
}
// Which side a record is (0, 1), or -1 for none of the four
inline int SideOf(uint32_t record) {
    record &= 0xFFFFFF;
    if (record < kPlayer1 || record >= kPlayer1 + 4 * kPlayerSize || (record - kPlayer1) % kPlayerSize) {
        return -1;
    }
    return static_cast<int>((record - kPlayer1) / kPlayerSize) & 1;
}
// Health, a word (0x90 full: 0101F0); a hit takes it at 0155A2 and 015B6A (sub.w d5,0x210(a6), the victim in A6)
constexpr uint32_t kHealth = 0x210;
constexpr uint16_t kHealthFull = 0x90;
constexpr uint32_t kRedHealth = 0x21A; // what comes back while the character rests (016934)
constexpr uint32_t kPartner = 0x224;   // long: the record of the same side's other character (0155C4)

// --- for hooks ---

inline void SetLow16(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFF0000) | (value & 0xFFFF);
}

} // namespace xmvsf
