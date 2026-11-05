// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
// (shared by the picture's shaders: a byte of a buffer of bytes read as 32-bit words, and a line's record's fields)
#define BYTE(buf, at) ((buf[(at) >> 2] >> (8u * ((at) & 3u))) & 0xFFu)
// A line's record (video/gpu_picture.cpp, kLineWords words): the layers' registers it is drawn with
#define L_MAPS 0u        // 3 words
#define L_ROWS 3u
#define L_SCROLL 4u      // 6 words: across and down for each screen
#define L_ROW_OFFSET 10u
#define L_VIDEO_CONTROL 11u
#define L_LAYER_CONTROL 12u
#define L_PRIORITY 13u   // (the object list's priority control)
#define L_SIDE 14u       // (a wider picture's columns each side: PictureOptions::side)
#define L_WIDTH 15u      // (the picture's width)
#define L_OWN_LEFT 16u   // 3 words: where the board's own columns start, for each screen (moved: the game's sources')
#define L_FILL_HOLES 19u
#define L_OVERRIDES 20u  // (how many tiles the game's sources give beyond the board's columns on the line)
#define L_CPS1 21u       // (a CPS-1's line: its layers' order and priority masks are another board's)
#define L_ENABLE 22u     // (a CPS-1's: the layer control's bits that switch scroll screens 1-3 on, a byte each)
#define L_LAYER_MASK 23u // (PictureOptions::layerMask: the layers drawn, bit 0 the objects, 1-3 the scroll screens)
#define L_OVER 28u       // (the objects alone over another picture: a guest's stage; out, their pixels and nothing else)
#define L_MASKS 24u      // (a CPS-1's: 4 words, its priority masks: the pens of a screen in front of the objects)
#define L_WORDS 32u
// (each line's tiles given beyond the board's columns: kOverrides of them, two words each)
#define OVERRIDES 32u
// (after the 224 lines' records: the graphics ROMs' size, in bytes; then another set's graphics' size, 0 if none)
#define L_GRAPHICS_SIZE (224u * L_WORDS)
#define L_EXTRA_SIZE (224u * L_WORDS + 1u)
