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
#define L_WORDS 16u
// (after the 224 lines' records: the graphics ROMs' size, in bytes)
#define L_GRAPHICS_SIZE (224u * L_WORDS)
