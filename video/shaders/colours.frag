#version 450
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
// The picture's third pass: at each pixel, the front object's pen if its level puts it in front of the scroll screens'
// pens there (the masks, from the line's layer order and the objects' priority control), else the screens'; then the
// pen's colour. As native_render.cpp's Line.
layout(location = 0) out vec4 colour;
layout(set = 2, binding = 0) uniform usampler2D screens;  // the scroll screens' (holes filled): pen, marks above
layout(set = 2, binding = 1) uniform usampler2D objects;  // the second's: pen, level, top bit set where an object is
layout(set = 2, binding = 2) uniform usampler2D lines;    // a line's record a row (common.glsl)
layout(set = 2, binding = 3) uniform usampler2D palette;  // 64 x 48 colours: red, green, blue in the low three bytes

void main() {
    const ivec2 at = ivec2(gl_FragCoord.xy);
    const uint s = texelFetch(screens, at, 0).r;
    const uint o = texelFetch(objects, at, 0).r;
    uint pen = s & 0xFFFFu;
    if (texelFetch(lines, ivec2(21, at.y), 0).r != 0u) {
        // (a CPS-1's: a screen in front of the objects, if one drew; else the front object, but where the screen just
        // behind them has a high pen; else the screens behind: layers.frag, Cps1)
        if ((s & 0x80000000u) != 0u) {
            pen = (s >> 16) & 0xFFFu;
        } else if ((s & 0x2000u) != 0u && (o & 0x80000000u) != 0u && (s & 0x1000u) == 0u) {
            pen = o & 0xFFFFu;
        } else {
            pen = s & 0xFFFu;
        }
    } else if ((o & 0x80000000u) != 0u) {
        const uint control = texelFetch(lines, ivec2(12, at.y), 0).r, priority = texelFetch(lines, ivec2(13, at.y), 0).r;  // (L_LAYER_CONTROL, L_PRIORITY)
        uint l0 = (control >> 6) & 3u, l1 = (control >> 8) & 3u, l2 = (control >> 10) & 3u, l3 = (control >> 12) & 3u;
        uint p0 = (priority >> (4u * l0)) & 0xFu, p1 = (priority >> (4u * l1)) & 0xFu;
        uint p2 = (priority >> (4u * l2)) & 0xFu, p3 = (priority >> (4u * l3)) & 0xFu;
        if (l0 == 0u) { l0 = l1; l1 = 0u; p0 = p1; }
        if (l1 == 0u) { l1 = l2; l2 = 0u; p1 = p2; }
        if (l2 == 0u) { l2 = l3; l3 = 0u; p2 = p3; }
        const uint level = (o >> 16) & 7u;
        uint mask = 0u;
        if (level == 0u) {
            mask = 0xFFu;
        } else if (level <= p0 && level <= p1 && level <= p2) {
            mask = 0xFEu;
        } else {
            uint mask0 = 0xAAu, mask1 = 0xCCu;
            if (p0 > p1) mask0 &= ~0x88u;
            if (p0 > p2) mask0 &= ~0xA0u;
            if (p1 > p2) mask1 &= ~0xC0u;
            if (level <= p0) mask |= mask0;
            if (level <= p1) mask |= mask1;
            if (level <= p2) mask |= 0xF0u;
        }
        const uint marks = (s >> 16) & 7u;
        if (((mask >> marks) & 1u) == 0u) {
            pen = o & 0xFFFFu;
        }
    }
    const uint rgb = texelFetch(palette, ivec2(pen & 63u, pen >> 6), 0).r;
    colour = vec4(float(rgb & 0xFFu), float((rgb >> 8) & 0xFFu), float((rgb >> 16) & 0xFFu), 255.0) / 255.0;
}
