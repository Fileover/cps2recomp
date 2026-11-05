#version 450
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
// The picture's second pass: each of the frame's object tiles (native_render.h, ObjectTile; front to back) an instance,
// a 16x16 quad where the board puts it, at a depth by its place in the list, so that with the depth test the front one
// is the one left at each pixel, whatever order they are drawn in (the board's: the first object to cover a pixel keeps
// it). As native_render.cpp's DrawObjects.
layout(std430, set = 0, binding = 0) readonly buffer Tiles { uint tiles[]; };   // 4 words a tile (gpu_picture.cpp)
layout(std430, set = 0, binding = 1) readonly buffer Frame { uint frame[]; };   // the offsets across and down, the tiles' count, the width, the graphics' size
layout(location = 0) out vec2 within;       // (the pixel of the tile, 0-16 across and down)
layout(location = 1) flat out uint tile;
layout(location = 3) flat out uint graphicsSize;
layout(location = 2) flat out uint looks;   // the pen of its colour set's first (bits 0-15), flips (16, 17), marked (19), level (20-22)

void main() {
    const uint i = uint(gl_InstanceIndex), corner = uint(gl_VertexIndex);
    const uint w0 = tiles[i * 4u], w1 = tiles[i * 4u + 1u];
    tile = tiles[i * 4u + 2u];
    looks = tiles[i * 4u + 3u];
    graphicsSize = frame[4];
    const int xOffset = int(frame[0]), yOffset = int(frame[1]);
    int x = int(w0 & 0xFFFFu), y = int(w0 >> 16);
    if ((looks & 0x80000u) != 0u) { // (marked to take the offsets)
        x += xOffset;
        y += yOffset;
    }
    const int sy = (y + 16 - yOffset) & 0x3FF;           // (the raster line its top is on)
    const int sx = ((x + 64 - xOffset - 48) & 0x3FF) - 16; // (the picture's column its left edge is at)
    // (two triangles: corners 0-5)
    const vec2 corners[6] = vec2[6](vec2(0, 0), vec2(16, 0), vec2(0, 16), vec2(16, 0), vec2(16, 16), vec2(0, 16));
    const vec2 c = corners[corner];
    within = c;
    const vec2 pixel = vec2(float(sx), float(sy - 16)) + c;
    const float width = float(frame[3]);
    const float depth = (float(i) + 0.5) / float(max(frame[2], 1u));
    gl_Position = vec4(pixel.x / width * 2.0 - 1.0, 1.0 - pixel.y / 224.0 * 2.0, depth, 1.0);
}
