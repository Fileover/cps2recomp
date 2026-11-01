// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors: Darkstalkers' Revenge: names for what is known of the game's memory and routines, each found by
// experiment in this game (PROGRESS.md). Its variables are reached from A5 = FF8000. Its program is its own: no more
// than 1.4% of any other game's in the collection lines up with it (tools/portmap.py), Vampire Savior's 0.3%.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

#include <cstdint>

namespace nwarr {

using cps2recomp::M68K;

// --- where the game is ---

// Words from A5, each a step (in twos) of the one before. The first is the step of whatever the game is doing: the
// board's start steps it 2 to 10 (01F88C on, frames 72-441), then its show loops 2 to 1A (020BC0 on: 6 with the next
// word at 14 is the title, frames 2801-3304; 8 and on the fights it shows); a game started by Start runs its own
// routine (02527C, given at 0215F4), which steps it from 0 again. Traced with --watch.
constexpr uint32_t kMode = 0xFF8000, kStep = 0xFF8004, kStep2 = 0xFF8008, kStep3 = 0xFF800C;
enum : uint16_t { kBootLast = 0x10, kShowTitle = 0x06, kShowTitleStep = 0x14 };

// Credits paid for and not yet used, a byte (RAM dumps after one, two and three coins: 1, 2, 3; 004D38 adds a coin's,
// 021454 takes one as Start is pressed). It is the sixth byte of the coin record at A5+1AC (004D26 on). The board
// has no free play of its own: the test menu's coin modes are 1 coin for 1-9 credits, 2-9 coins a credit, and 2 coins
// to start with 1 to continue (stepped through, eighteen in all).
constexpr uint32_t kCredits = 0xFF81B2;

// The round's clock: seconds left, a BCD byte (99 at a round's start, 00876C; a second off at 0087AC, sbcd), and the
// frames left of the second (FF8E15, counted down at 008798 and set again from FF8E17, which the timer speed sets).
// Found with --watch through a fight (a second every 42 frames at the board's own speed).
constexpr uint32_t kRoundClock = 0xFF8E09, kClockFrames = 0xFF8E15;

// --- the players ---

// Each player's record, 0x500 long, the first at FF8388: the health word is at +42 (0x90 full, 00668A sets both),
// and the only writer of it in a fight is 0C1384 (sub.w d4,0x42(a4)), with --watch on it through a fight. The FBNeo
// training script for this set names the health byte FF83CB and FF88CB.
constexpr uint32_t kPlayer1 = 0xFF8388, kPlayerSize = 0x500;
inline uint32_t Player(int n) {
    return kPlayer1 + static_cast<uint32_t>(n) * kPlayerSize;
}
constexpr uint32_t kHealth = 0x42;
constexpr uint16_t kHealthFull = 0x90;

// --- the board's settings ---

// The settings record as read from the settings memory, at A5-6D12 (FF12EE); 01FC34 on copies it to the variables
// below and returns at 01FCDC. Each variable found by saving the test menu with that one option changed and diffing
// the game's memory after the next start.
constexpr uint32_t kSettingsCopied = 0x01FCDC;

// --- for hooks ---

inline void SetLow16(uint32_t &reg, uint32_t value) {
    reg = (reg & 0xFFFF0000) | (value & 0xFFFF);
}

} // namespace nwarr
