// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior as one of the games made from its program (src/places.h): the places its sources name are its
// own, so there is no list of them.
#pragma once

#include <array>
#include <cstdint>

namespace vsav::family {

struct Place {
    uint32_t vsav, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

} // namespace vsav::family
