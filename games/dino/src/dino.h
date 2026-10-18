// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cadillacs and Dinosaurs (dino, World 930201): names for what is known of the game's memory, with how each was
// found (PROGRESS.md has more). The game keeps its variables at A5 = FF8000.
#pragma once

#include "game.h"
#include "lift.h"
#include "machine.h"

namespace cps2recomp::dino {

constexpr uint32_t kVars = 0xFF8000; // A5

// Each of the three coin chutes' records (16 bytes from FF847A; 006C62 walks them), with its credits at +3 (byte;
// found from dumps after coins). A new board has a chute for each player, each with credits of its own.
constexpr uint32_t kChute = kVars + 0x47A, kChuteSize = 0x10, kChuteCredits = 3;

// A value that goes 10, 30, 40, 50 as the board starts (traced): 50 when the title is drawn (about frame 312). It
// counts on through the rest of the show, so only the board's start is told by it.
constexpr uint32_t kShow = kVars + 0x35;
constexpr uint8_t kShowTitle = 0x50;

// The game's step (byte: 4 as a game begins, 0C while it is played, 08 between stages, 1C at a stage's end;
// 0 in the attract) and the stage (byte, 0-7; 00ABE8 steps it on as a stage ends; 00004CEE clears it, with the rest
// of the game's variables, as a game begins: 00A9EC, returning to 00A9F2)
constexpr uint32_t kStep = kVars + 0x4D0, kStage = kVars + 0x4D9;
constexpr int kStages = 8;

// The stage's clock: minutes and seconds (BCD bytes), counted down at 0103C0 each time the frame count (byte, from
// 0x32) runs out
constexpr uint32_t kClockMinutes = kVars + 0x4E8, kClockSeconds = kVars + 0x4E9, kClockFrames = kVars + 0x4EA;

// The players: three records of 0x180 bytes from FFB274 (the code finds them as 3274(a5), 33F4(a5), 3574(a5))
constexpr int kPlayers = 3;
constexpr uint32_t kPlayer = kVars + 0x3274, kPlayerSize = 0x180;
inline uint32_t Player(int n) {
    return kPlayer + kPlayerSize * static_cast<uint32_t>(n);
}
constexpr uint32_t kInPlay = 0x00;  // byte: 1 while the player is in the game
constexpr uint32_t kHealth = 0x6D;  // byte: health (found standing still while hit: 64 down to 0, 64 again with a life)
constexpr uint8_t kHealthFull = 0x64;
constexpr uint32_t kLives = 0xA3;   // byte: lives left after this one (01BFD6 takes one)

} // namespace cps2recomp::dino
