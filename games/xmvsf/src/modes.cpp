// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: the "Alternative modes" page, attract mode and the stage viewer are the toolkit's
// (fight/alternative.h); this says where the game keeps what they need. Its steps are not the Alpha games' four words
// (xmvsf.h), so it says itself when a round is fought, the players are choosing and a game is on.
#include "alternative.h"
#include "session.h"
#include "state.h"
#include "xmvsf.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace xmvsf;

    // Which part of the show's loop is on (xmvsf.h, kShowStep): the rest (notices, rankings), the introduction, the
    // fight after the title; and whether the title has been placed since the introduction began
    enum Part : uint8_t { kRest, kIntro, kFight };
    struct Show {
        Part part = kRest;
        bool title = false;
    } g_show;

    bool AttractStep(int show) {
        const bool match = Ram16(kGameStep) == kGameMatch;
        if (g_show.part == kRest && Ram8(kShowStep) == 2) {
            g_show = {kIntro, false};
        } else if (g_show.part == kIntro && g_show.title && match) {
            g_show = {kFight, false};
        } else if (g_show.part == kFight && !match) {
            g_show.part = kRest;
        }
        // (the parts not wanted run unseen)
        return show == modes::kShowIntro ? g_show.part != kIntro : g_show.part != kFight;
    }

    // 00BAEA: the stage of the next match has been taken (00BABC: from the table of stages at 088BFA, by a count that
    // goes on from match to match; the computer's last fight is on Apocalypse's, set after this at 00BD06). A mode's
    // or the session's stage is put in its place.
    bool StageChosen(M68K &) {
        const int stage = session::WantedStage();
        if (stage >= 1 && stage <= kStages) {
            SetRam8(kStage, static_cast<uint8_t>(stage - 1));
        }
        return false;
    }
} // namespace

// The show has placed the title (title.cpp's hook)
void AttractTitle() {
    g_show.title = true;
}

void ModesState(Archive &ar) {
    ar.Mark(0x53484F57); // SHOW
    ar.Pod(g_show);
}

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
    d.attractBegun = [](int frames) {
        g_show = Show();
        return frames > 100;
    };
    d.attractStep = AttractStep;
    d.attractWords = "The game's own show on a loop, with no coin in: the introduction, the title, a fight and the rankings.";
    d.attractOver = [] { return Ram8(kPlaying) != 0 || Ram8(kCredits) != 0; };
    d.startTraining = session::StartSession;
    d.modesWords = "Other ways to run the game: training, attract mode and a viewer for the stages.";
    d.stageNames.assign(std::begin(kStageNames), std::end(kStageNames));
    d.cameraEnds = [](int stage, int &left, int &right) { CameraEnds(stage - 1, left, right); };
    d.apart = 0x100; // (the players either side of the picture's middle: the camera's word + 0x40 + 0xC0)
    d.notTheStage = NotTheStage;
    modes::Register(d);
    AddHook(HookAt(0x00BAEA), "StageChosen", StageChosen);
}

} // namespace cps2recomp
