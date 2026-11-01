// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors as the first of the games made from its program, whose sources Darkstalkers builds too
// (games/nwarr/src; places.h there): its places are where its sources say (an empty list).
#pragma once

#include <array>
#include <cstdint>

namespace nwarr::family {

struct Place {
    uint32_t nwarr, here;
};
inline constexpr std::array<Place, 0> kPlaces = {};

} // namespace nwarr::family
