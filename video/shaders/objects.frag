#version 450
#extension GL_GOOGLE_include_directive : require
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
// An object tile's pixel: its pen, its level and that it is an object's; nothing (and no depth) where it is clear (pen 15)
#include "common.glsl"
layout(location = 0) in vec2 within;
layout(location = 1) flat in uint tile;
layout(location = 2) flat in uint looks;
layout(location = 3) flat in uint graphicsSize;
layout(location = 0) out uint result;
layout(std430, set = 2, binding = 0) readonly buffer Graphics { uint graphics[]; };

void main() {
    const uint k = min(uint(within.x), 15u), line = min(uint(within.y), 15u);
    const bool flipX = (looks & 0x10000u) != 0u, flipY = (looks & 0x20000u) != 0u;
    const uint at = tile * 128u + (flipY ? 15u - line : line) * 8u;
    if (at + 8u > graphicsSize) {
        discard;
    }
    const uint p = flipX ? 15u - k : k;
    const uint part = at + (p >> 3) * 4u, s = 7u - (p & 7u);
    const uint value = ((BYTE(graphics, part) >> s) & 1u) | (((BYTE(graphics, part + 1u) >> s) & 1u) << 1) |
                       (((BYTE(graphics, part + 2u) >> s) & 1u) << 2) | (((BYTE(graphics, part + 3u) >> s) & 1u) << 3);
    if (value == 15u) {
        discard;
    }
    // (the pen, the level above it, and the top bit: an object's)
    result = ((looks & 0xFFFFu) + value) | (((looks >> 20) & 7u) << 16) | 0x80000000u;
}
