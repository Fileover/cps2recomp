// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// A fighting game's training session, for every game on the board: the menu's "Start" (the board switched off and
// on, and the way to the character choice with two players worked unseen), player 1's controls choosing for both,
// the round kept as its fighting begins for "Start over" (the players put in the middle or a corner, sides swapped
// or not), the stage chosen, and the aids of fight.h put together: the combo readout, the input display, frame data,
// its meter and its log, hit boxes, health that comes back, and the dummy.
//
// A game's sources describe where the game keeps what this needs (Description), register it, and tell it from their
// hooks what the game does: each player's step of a fight (Step), a hit landing or being guarded (Contact), a hit's
// damage being taken (Hit). The game's own hook where it chooses the stage asks WantedStage, and its clock and its
// knockouts ask NoClock and NoKnockOut.
#pragma once

#include "fight.h"
#include "game.h"
#include "machine.h"

#include <cstdint>
#include <string>
#include <vector>

namespace cps2recomp {
class Archive;
}

namespace cps2recomp::session {

struct Description {
    // --- the game's memory ---
    uint32_t player1 = 0xFF8400, playerSize = 0x400, x = 0x10;
    bool xLong = false; // (a long with 16 bits of fraction, not a word)
    // Health: words, what the player has and (0: none) the most it can have back; the most in all
    uint32_t health = 0, healthCeiling = 0;
    uint16_t healthFull = 0;
    // The round's clock (a byte: seconds left) and its most: a round is being fought while it is in 1..most
    uint32_t clock = 0;
    uint8_t clockFull = 99;
    // The camera (words: the picture's left edge and bottom in the stage) and every copy of its place across the game
    // keeps (each set by "Start over")
    uint32_t cameraX = 0, cameraY = 0;
    std::vector<uint32_t> cameraCopies;
    // Where the players stand: at a round's start, and the ends they can go to; the camera's ends; how far apart
    // the players are put in a corner
    uint16_t startX[2] = {0, 0};
    int stageLeft = 0, stageRight = 0, cameraLeft = 0, cameraRight = 0;
    int cornerApart = 120;
    // The picture's line that height 0 in the stage is on, with the camera at the bottom (for the boxes)
    int pictureFloor = 239;

    // --- the game ---
    // A player has chosen its character (then player 1's controls choose for player 2)
    bool (*chosen)(int player) = nullptr;
    // What the game read off the panel for a player this frame, as Button bits, and whether the player faces left
    uint16_t (*panelHeld)(int player) = nullptr;
    bool (*facingLeft)(int player) = nullptr;
    // A move's start, for the frame data log (none: no log)
    fight::MoveLog::StartFn moveStart = nullptr;
    // The boxes the game's hit tests test (none: no hit boxes)
    std::vector<fight::StageBox> (*boxes)(Machine &machine) = nullptr;
    // For the dummy's guard: whether something of player 1's is near enough to guard, and whether it must be guarded
    // standing (none: the dummy guards whenever player 1 attacks)
    void (*guardView)(Machine &machine, bool &inRange, bool &standing) = nullptr;
    // Whether the dummy is knocked down (none: taken from how long it reels)
    bool (*knockedDown)() = nullptr;
    // The game's own readouts, beside the toolkit's (lines and boxes of the HUD)
    void (*hud)(Machine &machine, std::vector<HudLine> &lines, std::vector<HudBox> &boxes) = nullptr;
    // The wide picture: how far right the stage is drawn in it, and the columns it adds on each side (none: 0)
    int (*wideStage)() = nullptr;
    int (*wideSide)() = nullptr;
    int (*wideMargin)() = nullptr;
    // What the dummy can do of the game's own
    fight::DummyOptions dummy;
    // The aids of the game's own, as training aids (they work only in a session): their names
    std::vector<const char *> ownAids;
    // The stage choices (first "The game's choice")
    std::vector<const char *> stageChoices;
};

// The game's description, and the menu's training features and commands (on modes.h's training page)
void Register(const Description &description);

// The menu's "Start": the board switched off and on and taken to the character choice with two players
void StartSession(Machine &machine);
// A session is on
bool InSession();
// The stage the machine that is running is to fight on, for the game's choice of stage (0: the game's choice):
// what a mode wants (modes.h), else the session's "Stage"
int WantedStage();
// No clock, and nobody knocked out: a session, the stage viewer, or a fighting mode that wants it
bool NoClock();
bool NoKnockOut();

// From the game's hooks: a player's step of a fight; a hit landing on a player or guarded; a hit's damage taken
void Step(int player, const fight::StepView &view);
void Contact(int victim, bool guarded);
void Hit(int player, unsigned damage, unsigned comesBack = 0);
// What a player is doing (the frame data's view of it)
fight::Doing Now(int player);

// Around the port's machine's frame (the game's beforeFrame and frame), and its save states
void BeforeFrame(Machine &machine);
void Frame(Machine &machine);
void State(Archive &ar);

} // namespace cps2recomp::session
