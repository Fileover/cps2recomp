// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 3: swapping a player's fighter in the middle of a fight (not yet).
#include "sfa3.h"

namespace cps2recomp {

bool CanTag(int) {
    return false;
}
bool TaggedIn(int) {
    return true;
}
void TagIn(int, int, int) {}
void TagDifficulty(int) {}
void TagOffStage(int, bool) {}
void TagUntouchable(int) {}
uint16_t TagCall(int) {
    return 0;
}

} // namespace cps2recomp
