// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 2 as one of the games made from its program, which build its sources (src/): what is its
// own. The places its sources name are its own, so there is no list of them (src/places.h).
#pragma once

#include <array>
#include <cstdint>

namespace sfa2::family {

struct Place {
    uint32_t sfa2, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

} // namespace sfa2::family
