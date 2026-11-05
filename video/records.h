// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The native interface's records (docs/NATIVE_VIDEO.md; native.h has how they are made and compared)
#pragma once

#include "../machine/romset.h"

#include <cstdint>
#include <cstdio>

namespace cps2recomp {

namespace native_video {

    // The palette has six pages of 512 colours: objects, scroll screens 1-3, and two for stars
    constexpr int kPages = 6, kPageColours = 0x200;

    // An upload of the palette: the board takes these pages of colours (a colour is 4 bits each of brightness,
    // red, green, blue) from the game's buffer; the pages not taken keep the colours they had
    struct PaletteUpload {
        uint8_t pages = 0;   // bit n: page n is taken
        uint32_t source = 0; // where the buffer is (the 68000's address)
        uint16_t colours[kPages][kPageColours] = {};
        bool operator==(const PaletteUpload &) const = default;
        // (what of it differs from another, for a report)
        void Describe(const PaletteUpload &other, FILE *out) const;
    };

    // The object list the board draws a frame from: 1,024 entries of four words (x, with the entry's priority in
    // its top bits; y; tile; attributes), as the game's code put them, and what the game's code said of all of them:
    // the priority control and the offsets across and down
    constexpr int kObjectWords = 0x1000;
    struct ObjectList {
        uint16_t words[kObjectWords] = {};
        uint16_t priority = 0, xOffset = 0, yOffset = 0;
        bool operator==(const ObjectList &) const = default;
        void Describe(const ObjectList &other, FILE *out) const;
    };

    // What the CPS-2's picture is drawn from on a line, but for the palette (taken at its upload) and the object list
    // (taken as the frame starts): where the three scroll screens' maps are and where each is scrolled to; the row
    // scroll's table, the place in it and whether it is on (the video control); the layers' order and which are on
    // (the layer control); and the objects' offsets. Its words are the registers'.
    struct Layers {
        uint16_t maps[3] = {}, rows = 0, scroll[3][2] = {}, rowOffset = 0, videoControl = 0, layerControl = 0;
        uint16_t xOffset = 0, yOffset = 0;
        bool operator==(const Layers &) const = default;
        void Describe(const Layers &other, FILE *out) const;
    };

    // The board's side: the upload the palette chip makes now, from its registers and video RAM
    // (as Video::CopyPalette reads them)
    PaletteUpload DecodePalette(const uint8_t *videoRam, const uint16_t *cpsA, const uint16_t *cpsB, const Board &board);
    // ... the object list it takes, from the bank of object memory it latches and the object registers
    ObjectList DecodeObjects(const uint8_t *bank, const uint16_t *objectRegs);
    // ... the layers a line is drawn with, from CPS-A's and CPS-B's registers and the object registers (the game's side
    // makes them the same way, from what the game said to each)
    Layers DecodeLayers(const uint16_t *cpsA, const uint16_t *cpsB, const uint16_t *objectRegs, uint8_t layerControl);

    // The game's side: a buffer of colours in video RAM as the game left it
    // (`pages` and the buffer's place as the game's code works them out)
    PaletteUpload GamePalette(const uint8_t *videoRam, uint32_t source, uint8_t pages);

    // The registers the game's code talks to the video through: the object registers (400000-40000B), the object
    // bank (8040E0), CPS-A and CPS-B (at 8001xx, and again at 8041xx)
    bool IsVideoRegister(uint32_t address);

} // namespace native_video

} // namespace cps2recomp
