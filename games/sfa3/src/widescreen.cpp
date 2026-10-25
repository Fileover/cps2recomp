// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 3: widescreen (not yet).
#include "sfa3.h"

namespace cps2recomp {

bool WidePlay() {
    return false;
}
int WideMargin() {
    return 0;
}
int WideSide() {
    return 0;
}
int WideStage() {
    return 0;
}
void WideForSpare(Machine &) {}
void WideForPort(Machine &) {}
void WidescreenBeforeFrame(Machine &) {}
void RegisterWidescreen() {}

} // namespace cps2recomp
