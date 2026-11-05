#version 450
#extension GL_GOOGLE_include_directive : require
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
// The picture's first pass (docs/NATIVE_VIDEO.md, the GPU renderer): at each pixel the three scroll screens in the layer
// control's order, back to front, from the game's maps and row scroll's table in video RAM and the graphics ROMs;
// out: the pen of the front one drawn there (the backdrop's, 0xBFF, if none) and the marks of those drawn (a bit for
// each place in the order). As native_render.cpp's DrawScroll, a pixel at a time.
#include "common.glsl"
layout(location = 0) out uint result;
layout(std430, set = 2, binding = 0) readonly buffer Graphics { uint graphics[]; };
layout(std430, set = 2, binding = 1) readonly buffer Video { uint video[]; };
layout(std430, set = 2, binding = 2) readonly buffer Lines { uint lines[]; };
// (a line's tiles given beyond the board's columns: the column its left edge is at (16 bits) and the screen (above);
// then the tile and its attributes)
layout(std430, set = 2, binding = 3) readonly buffer Overrides { uint overrides[]; };

uint Word(uint at) {
    return (BYTE(video, at) << 8) | BYTE(video, at + 1u);
}

void main() {
    const uint x = uint(gl_FragCoord.x), line = uint(gl_FragCoord.y);
    const uint rasterLine = line + 16u;
    const uint l = line * L_WORDS;
    const uint control = lines[l + L_LAYER_CONTROL], videoControl = lines[l + L_VIDEO_CONTROL];
    uint order[3] = uint[3]((control >> 6) & 3u, (control >> 8) & 3u, (control >> 10) & 3u);
    const uint l3 = (control >> 12) & 3u;
    if (order[0] == 0u) { order[0] = order[1]; order[1] = 0u; }
    if (order[1] == 0u) { order[1] = order[2]; order[2] = 0u; }
    if (order[2] == 0u) { order[2] = l3; }
    uint pen = 0xBFFu, marks = 0u;
    for (uint n = 0u; n < 3u; n++) {
        const uint layer = order[n];
        bool on = layer == 1u ? (control & 2u) != 0u
                : layer == 2u ? (control & 4u) != 0u && (videoControl & 4u) != 0u
                : layer == 3u ? (control & 8u) != 0u && (videoControl & 8u) != 0u : false;
        if (!on) {
            continue;
        }
        const uint which = layer - 1u;
        const uint base = (lines[l + L_MAPS + which] * 256u) & ~0x3FFFu & 0x3FFFFu;
        const uint shift = 3u + which, size = 1u << shift, mapMask = (64u << shift) - 1u;
        uint scrollX = lines[l + L_SCROLL + which * 2u];
        const uint scrollY = lines[l + L_SCROLL + which * 2u + 1u];
        if (which == 1u && (videoControl & 1u) != 0u) {
            const uint rows = (lines[l + L_ROWS] * 256u) & ~0x7FFu & 0x3FFFFu;
            scrollX += Word(rows + (((rasterLine + lines[l + L_ROW_OFFSET]) & 0x3FFu) * 2u));
        }
        const uint y = (rasterLine + scrollY) & mapMask, row = y >> shift;
        const int ownLeft = int(lines[l + L_OWN_LEFT + which]);
        const uint mx = uint(64 - ownLeft + int(scrollX) + int(x)) & mapMask; // (the map's column of pixels this pixel shows)
        const uint column = mx >> shift, k = mx & (size - 1u);
        const int tileAt = int(x) - int(k); // (the picture's column the tile's left edge is at)
        uint index;
        if (which == 0u) index = (row & 0x1Fu) + (column << 5) + ((row & 0x20u) << 6);
        else if (which == 1u) index = (row & 0x0Fu) + (column << 4) + ((row & 0x30u) << 6);
        else index = (row & 0x07u) + (column << 3) + ((row & 0x38u) << 6);
        uint code = Word(base + index * 4u);
        uint attr = Word(base + index * 4u + 2u);
        if (tileAt < ownLeft || tileAt + int(size) > ownLeft + 384) {
            // (beyond the board's own columns: the game's sources' tile, if they gave one)
            const uint count = lines[l + L_OVERRIDES], o = line * OVERRIDES * 2u;
            for (uint n = 0u; n < count; n++) {
                const uint w = overrides[o + n * 2u];
                if ((w >> 16) == which && (int(w << 16) >> 16) == tileAt) {
                    code = overrides[o + n * 2u + 1u] & 0xFFFFu;
                    attr = overrides[o + n * 2u + 1u] >> 16;
                    break;
                }
            }
        }
        uint at, rowBytes;
        if (which == 0u) { at = 0x800000u + code * 64u + ((column & 1u) != 0u ? 4u : 0u); rowBytes = 8u; }
        else if (which == 1u) { at = 0x800000u + code * 128u; rowBytes = 8u; }
        else { code &= 0x3FFFu; at = 0x800000u + code * 512u; rowBytes = 16u; }
        const uint tileLine = y & (size - 1u);
        const bool flipX = (attr & 0x20u) != 0u, flipY = (attr & 0x40u) != 0u;
        at += (flipY ? size - 1u - tileLine : tileLine) * rowBytes;
        if (at + rowBytes > lines[L_GRAPHICS_SIZE]) {
            continue;
        }
        const uint p = flipX ? size - 1u - k : k;   // (the pixel of the tile's row: 8 to each 4 bytes, one plane each)
        const uint part = at + (p >> 3) * 4u, s = 7u - (p & 7u);
        const uint value = ((BYTE(graphics, part) >> s) & 1u) | (((BYTE(graphics, part + 1u) >> s) & 1u) << 1) |
                           (((BYTE(graphics, part + 2u) >> s) & 1u) << 2) | (((BYTE(graphics, part + 3u) >> s) & 1u) << 3);
        if (value != 15u) {
            pen = ((attr & 0x1Fu) + 0x20u * (which + 1u)) * 16u + value;
            marks |= 1u << n;
        }
    }
    result = pen | (marks << 16);
}
