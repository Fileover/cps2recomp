// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 3: the "Alternative modes" page, attract mode and the stage viewer are the toolkit's
// (fight/alternative.h); this says where the game keeps what they need, which parts of its loop are which, and makes
// the match a mode wants (its characters and stage) where the game makes one.
//
// The game's loop (kMode, traced with no coin in; the steps between are fades): 0C and 0E the notices, 10 the
// introduction (the title at its end), 14 the title and a demonstration fight (kMatchStep 2 while it is fought), 18
// the score rankings, 1A-24 more rankings and notices, 26 the advisory, 2A another demonstration fight, 2E the rankings
// again, then 0C and on.
#include "alternative.h"
#include "session.h"
#include "sfa3.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace sfa3;

    enum : uint16_t { kLoopStart = 0x0C, kIntro = 0x10, kFirstFight = 0x14, kSecondFight = 0x2A, kLoopEnd = 0x32 };

    // The parts of the loop left out for what is to be shown
    bool AttractStep(int show) {
        const uint16_t part = Ram16(kMode);
        if (part < kLoopStart) {
            return true; // (the loop's own start)
        }
        if (show == modes::kShowIntro) {
            if (part < kIntro || part > kIntro + 2) {
                modes::GoTo(kIntro);
            }
        } else if (part < kFirstFight) {
            modes::GoTo(kFirstFight);
        } else if (part > kFirstFight + 2 && part < kSecondFight) {
            modes::GoTo(kSecondFight);
        } else if (part > kSecondFight + 2) {
            modes::GoTo(kFirstFight);
        }
        return false;
    }

    // 00B7EE: the match has been made (00B778: the computer's next opponent for one player, or the two players' own
    // characters), the stage about to be taken from a character. The characters and colours a mode wants are put in
    // place.
    bool MatchMade(M68K &) {
        const MatchWanted &wanted = modes::Wanted();
        for (int n = 0; n < 2; n++) {
            if (wanted.characters[n] >= 0 && wanted.characters[n] < kCharacters) {
                SetRam8(Player(n) + kCharacter, static_cast<uint8_t>(wanted.characters[n]));
            }
            if (wanted.colours[n] >= 0) {
                SetRam8(Player(n) + kColourChoice, static_cast<uint8_t>(wanted.colours[n]));
            }
        }
        return false;
    }

    // 0090B8: a round is set up, the stage taken from the one the match making chose (A5+174, which the match making
    // sets more than one way). The one wanted (a mode's, the training session's) put in its place first.
    bool StageTaken(M68K &) {
        const int stage = session::WantedStage();
        if (stage >= 1 && stage <= kStages) {
            SetRam16(kStageNext, static_cast<uint16_t>(kStageNumbers[stage - 1] * 2));
        }
        return false;
    }
} // namespace

bool ThePanel(uint32_t, uint16_t attributes) {
    const unsigned set = attributes & 0x1F;
    return (set >= 0x15 && set <= 0x18) || (set >= 0x1A && set <= 0x1F);
}
bool IsStagePiece(uint16_t attributes) {
    const unsigned set = attributes & 0x1F;
    return set >= 0x10 && set <= 0x13;
}
bool NotTheStage(uint32_t, uint16_t attributes) {
    return !IsStagePiece(attributes);
}

void RegisterModes() {
    modes::Description d;
    d.steps = kMode;
    d.gameChoosing = kGameChoosing;
    d.gameMatch = kGameMatch;
    d.matchRound = kMatchRound;
    d.roundFought = kRoundFought;
    d.credits = kCredits;
    d.playersIn = kPlayersIn;
    d.soundQueue = kSoundQueue;
    d.soundQueueEnd = kSoundQueueEnd;
    d.tuneOff = kTuneOff;
    d.player1 = kPlayer1;
    d.playerSize = kPlayerSize;
    d.x = kX;
    d.stageNames.assign(std::begin(kStageNames), std::end(kStageNames));
    d.stageTunes.assign(std::begin(kStageTunes), std::end(kStageTunes));
    d.otherTuneNames = {"Character select", "Versus"};
    d.otherTunes = {kTuneChoose, kTuneVersus};
    d.cameraLeft = kCameraLeft;
    d.cameraRight = kCameraRight;
    d.notTheStage = NotTheStage;
    d.viewerLayers = 0xF & ~1u; // (the panel is objects: all the scroll screens are the stage's)
    d.wideSide = WideSide;
    d.wideMargin = WideMargin;
    d.wideForSpare = WideForSpare;
    d.wideForPort = WideForPort;
    // (the board starts up through steps 2 to 8 and then begins its loop; a coin in or a game begun ends it)
    d.attractBegun = [](int frames) { return frames >= 480; };
    d.attractOver = [] { return Ram8(kPlayersIn) != 0 || Ram8(kCredits) != 0; };
    d.attractStep = AttractStep;
    d.attractWords = "The game's own show on a loop, with no coin in: the introduction, the title, demonstration fights, the "
                     "rankings.";
    d.attractShowWords = "What attract mode plays: everything the board does, the introduction over and over, or one "
                         "demonstration fight after another.";
    d.startTraining = session::StartSession;
    d.modesWords = "Other ways to run the game: training, attract mode, viewers for the stages and the characters, the colour "
                   "editor, survival, endless battle, ring out, the tag team battle.";
    modes::Register(d);
    AddHook(HookAt(0x00B7EE), "MatchMade", MatchMade);
    AddHook(HookAt(0x0090B8), "StageTaken", StageTaken);
}

} // namespace cps2recomp
