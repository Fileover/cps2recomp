// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Alien vs. Predator: "Straight to the title" (the Game page), as in the other games. When the board starts
// (switched on, or reset from the menu), everything before the title is run unseen and unheard, as fast as it
// goes: the board's start, the notices, the logos and the story. Nothing of the game is changed, only not shown.
//
// The show's step (avsp.h, kMode) goes 2 to C as the board starts and 0 at its end (about frame 405); then the
// attract's 2, 4 ... C (the story, to about frame 5295) and 10, the title, drawn when its own step is 8 (about
// 5948). With credits in (free play puts them in) the title asking for Start comes sooner, with the show's step
// past the board's start and credits there. A step past the title ends the skipping too.
#include "avsp.h"

namespace cps2recomp {

namespace {
    using namespace avsp;

    constexpr int kSkipMost = 9000; // frames after which the skipping is given up
    bool g_skipping = false, g_booted = false, g_fast = false;
    int g_frames = 0;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }
} // namespace

void TitleFrame(Machine &machine) {
    const uint16_t mode = Ram16(kMode);
    if (machine.frame <= 1 && !g_skipping) {
        g_skipping = FeatureOn("skip_intro");
        g_booted = false;
        g_frames = 0;
    }
    if (g_skipping) {
        g_booted = g_booted || (mode == 0 && machine.frame > 30);
        const bool title = g_booted && ((mode == kModeTitle && Ram16(kModeStep) >= 8) || mode > kModeTitle ||
                                        (Ram8(kCredits) != 0 && mode != 0));
        if (title || ++g_frames > kSkipMost) {
            g_skipping = false;
        }
    }
    Fast(g_skipping);
}

void RegisterTitle() {
    TheGame().features.push_back({"skip_intro", "Game", "Straight to the title",
                                  "When the game starts, and when it is reset, goes straight to the title: the board's start, the "
                                  "notices and the story are run unseen, in a second or so.",
                                  0});
}

} // namespace cps2recomp
