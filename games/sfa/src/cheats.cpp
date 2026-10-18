// SPDX-License-Identifier: GPL-3.0-only
//
// Street Fighter Alpha: cheats (optional features: the launcher's Features page and the in-game menu switch
// them). Made-up play needs them to go up the ladder to its end ('make explore', game.cfg's explore features).
#include "sfa.h"

namespace cps2recomp {

namespace {
    using namespace sfa;

    // Which player's record an address is (0, 1), or -1
    int PlayerOf(uint32_t record) {
        record &= 0xFFFFFF;
        return record == Player(0) ? 0 : record == Player(1) ? 1 : -1;
    }

    // D3 is the damage about to come off the health of the player whose record is `victim`
    bool DamageTo(M68K &c, uint32_t victim) {
        const int player = PlayerOf(victim);
        if (player == 0 && FeatureOn("no_damage")) {
            SetLow16(c.d[3], 0);
        } else if (player == 1 && FeatureOn("one_hit")) {
            SetLow16(c.d[3], kHealthFull + 1);
        }
        return false;
    }

    // The hit's routine also takes one point off by itself (00FA20-00FA26, the way in from 00F8E2 and 00F906)
    // without D3: for no damage, player 1's health is noted where that starts and put back where the two ways
    // meet again (00FA2A)
    int16_t kept = 0;
    bool keeping = false;
} // namespace

void RegisterCheats() {
    // 00FA06: sub.w d3,0x40(a1), a hit's damage off the victim's health (A1)
    AddHook(0x00FA06, "HitDamage", [](M68K &c) { return DamageTo(c, c.a[1]); });
    // 021148: sub.w d3,0x40(a4), a throw's
    AddHook(0x021148, "ThrowDamage", [](M68K &c) { return DamageTo(c, c.a[4]); });
    AddHook(0x00FA20, "HitPoint", [](M68K &c) {
        keeping = PlayerOf(c.a[1]) == 0 && FeatureOn("no_damage");
        kept = static_cast<int16_t>(Ram16((c.a[1] & 0xFFFFFF) + kHealth));
        return false;
    });
    AddHook(0x00FA2A, "HitPointDone", [](M68K &c) {
        if (keeping) {
            SetRam16((c.a[1] & 0xFFFFFF) + kHealth, static_cast<uint16_t>(kept));
            keeping = false;
        }
        return false;
    });
    Game &game = TheGame();
    game.features.push_back({"no_damage", "Cheats", "No damage", "Player 1's health does not go down."});
    game.features.push_back({"one_hit", "Cheats", "One-hit knockouts", "Player 2 (the opponent, in a one-player game) goes down with the first hit."});
}

} // namespace cps2recomp
