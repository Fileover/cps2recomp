// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: the "Alternative modes" page, attract mode and the stage viewer are the toolkit's
// (fight/alternative.h); this says where the game keeps what they need. Its steps are not the Alpha games' four words
// (xmvsf.h), so it says itself when a round is fought, the players are choosing and a game is on.
#include "alternative.h"
#include "session.h"
#include "xmvsf.h"

namespace cps2recomp {

void RegisterModes() {
    using namespace xmvsf;
    modes::Description d;
    d.roundFoughtNow = [] { return Ram8(kPlaying) != 0 && Ram16(kGameStep) == kGameMatch && Ram8(kFight) == kFighting; };
    d.choosingNow = [] { return Ram8(kPlaying) != 0 && Ram16(kGameStep) == kGameChoosing && Ram8(kPlayersIn) == 3; };
    d.running = [] { return Ram8(kPlaying) != 0; };
    d.credits = kCredits;
    d.playersIn = kPlayersIn;
    d.soundQueue = kSoundQueue;
    d.soundQueueEnd = kSoundQueueEnd;
    d.soundQueueMask = kSoundQueueMask;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.xLong = true;
    d.attractBegun = [](int frames) { return frames > 100; };
    d.attractOver = [] { return Ram8(kPlaying) != 0 || Ram8(kCredits) != 0; };
    d.startTraining = session::StartSession;
    d.modesWords = "Other ways to run the game: training and attract mode.";
    modes::Register(d);
}

} // namespace cps2recomp
