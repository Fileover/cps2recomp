// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Other games' stages behind Alpha 2's fighters, as those games show them (fight/guest_stages.h: their boards run beside
// this one, game.cfg's guests): Darkstalkers' and Vampire Savior's (their facts in games/dstlk/guest.h,
// games/vampire_savior/guest.h). Here is what is this game's: its camera, when a stage is shown, its objects that are
// not its stage's, its stage tunes, and which stage is wanted.
//
// Which: a stage chosen by name (training's, versus's, the stage viewer's: the guests' after this game's eighteen), or,
// with the "Other games' stages" feature, an arcade match's stage is one of theirs, the same one for the same match.
#include "guest_stages.h"
#include "modes.h"
#include "session.h"
#include "sfa2.h"

#include "../../dstlk/guest.h"
#include "../../vampire_savior/guest.h"

namespace cps2recomp {

bool BattleMode(); // (fight/battle.h)
bool StageViewer(); // (modes.cpp)
bool NotTheStage(uint32_t tile, uint16_t attributes); // (machines.cpp)

namespace {
    using namespace sfa2;

    constexpr uint32_t kCameraX = At(0xFF8290), kCameraY = At(0xFF8294);

    // A match's stage: shown while a round is on (its versus screen and the choice before are the game's own)
    bool InMatch() {
        return Ram16(kMode) == 2 && Ram16(kGameStep) == kGameMatch && Ram16(kMatchStep) == kMatchRound;
    }

    // The stage wanted now, 1 the guests' first (Darkstalkers' are first: game.cfg's guests); 0 this game's own
    int Wanted() {
        if (BattleMode()) {
            return 0; // (the modes' stages are this game's own: its spare machines make them)
        }
        // (a stage chosen by name: training's, the viewer's; after this game's eighteen)
        const int wanted = session::WantedStage();
        if (wanted > kStages && wanted <= kStages + guest_stages::Stages()) {
            return wanted - kStages;
        }
        if (wanted >= 1 && wanted <= kStages) {
            return 0;
        }
        if (!FeatureOn("other_stages") || !InMatch()) {
            return 0;
        }
        // One of theirs, the same for the same match: from the stage the game chose and who is fought there (the
        // players' characters), the first of them whose game's set is there from that one on
        const int stages = guest_stages::Stages();
        const int from = (Ram16(kStage) / 2 * 7 + Ram8(Player(0) + kCharacter) * 3 + Ram8(Player(1) + kCharacter)) % stages;
        for (int n = 0; n < stages; n++) {
            const int stage = (from + n) % stages + 1;
            if (guest_stages::Available(stage)) {
                return stage;
            }
        }
        return 0;
    }
} // namespace

void RegisterStages() {
    Game &game = TheGame();
    // (Display: it changes nothing of the game's play; online, each player sees the stage of their own choice)
    game.features.push_back({"other_stages", "Display", "Other games' stages",
                             "An arcade match is fought on a stage of another game's, as that game shows it, its music too: "
                             "Darkstalkers' and Vampire Savior's, those whose ROM sets are there (beside this game's, or "
                             "chosen on their pages in the launcher). Training's, versus's and the stage viewer's lists have "
                             "them all after this game's own.",
                             0});
    guest_stages::Host host;
    host.cameraX = kCameraX;
    host.cameraY = kCameraY;
    host.cameraLeft = 256;
    host.cameraRight = 640;
    host.shown = [] { return StageViewer() || InMatch(); };
    host.viewer = StageViewer;
    host.notTheStage = NotTheStage;
    host.stageTunes.assign(std::begin(kStageTunes), std::end(kStageTunes));
    host.wanted = Wanted;
    host.feature = "other_stages";
    guest_stages::Register(host, {dstlk_guest::Description(), vampire_savior_guest::Description()});
}

} // namespace cps2recomp
