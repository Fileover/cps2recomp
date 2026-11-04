// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The picture at the game's level (docs/NATIVE_VIDEO.md): what the games' drawing code decides each frame, handed
// over as it decides it, in place of the board's registers and memories that video.cpp reads.
//
// Two sides make the same records. The game's side: Capcom's video library, rewritten once (video/lifts.cpp), says
// what the game's code tells the board, and the records are made from that and from the game's own memory. The
// board's side: a decoder makes them from what the board model was given, as video.cpp reads it. With --video check,
// every record from the board is compared with the one the game's side gave for it; a difference, or a record only
// one side made, is reported.
//
// Native so far: the palette's uploads, and the object list the board takes at each frame.
#pragma once

#include "../machine/romset.h"

#include <cstdint>
#include <cstdio>

namespace cps2recomp {

class M68K;

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

    // The board's side: the upload the palette chip makes now, from its registers and video RAM
    // (as Video::CopyPalette reads them)
    PaletteUpload DecodePalette(const uint8_t *videoRam, const uint16_t *cpsA, const uint16_t *cpsB, const Board &board);
    // ... the object list it takes, from the bank of object memory it latches and the object registers
    ObjectList DecodeObjects(const uint8_t *bank, const uint16_t *objectRegs);

    // The game's side: a buffer of colours in video RAM as the game left it
    // (`pages` and the buffer's place as the game's code works them out)
    PaletteUpload GamePalette(const uint8_t *videoRam, uint32_t source, uint8_t pages);

    // The registers the game's code talks to the video through: the object registers (400000-40000B), the object
    // bank (8040E0), CPS-A and CPS-B (at 8001xx, and again at 8041xx)
    bool IsVideoRegister(uint32_t address);

} // namespace native_video

// The two sides compared (--video check)
class NativeVideo {
public:
    bool checking = false;
    uint64_t frame = 0;                // (for the reports: the machine's frame)
    const uint8_t *videoRam = nullptr; // (the game's side reads the buffers the game fills there)
    uint8_t paletteControl = 0x30;     // (where the CPS-B's palette control is in its registers: Board::paletteControl)

    // The game's side: an instruction of the game's (a hook's), or a rewrite of the library, is about to write
    // `value` to the video register at `address` (a byte, or a word). The game's side keeps what the game said, and
    // makes the records it means: the palette's upload, the object list handed over.
    void GameWrites(M68K &c, uint32_t address, uint16_t value, bool byte);

    // The board's side: what the board did
    void BoardPalette(const native_video::PaletteUpload &upload);
    void BoardObjects(const native_video::ObjectList &list);

    // At the end of a run: the counts, and false if anything differed or was made by code not native yet
    bool Report(FILE *out) const;

private:
    void Palette(const native_video::PaletteUpload &upload);

    // What the game has said (its words to the registers), its list handed over, and its upload not yet made
    uint16_t m_objectRegs[6] = {}, m_cpsA[32] = {}, m_cpsB[32] = {};
    int m_bank = -1; // (-1: not said yet)
    uint16_t m_list[native_video::kObjectWords] = {};
    native_video::PaletteUpload m_pending;
    bool m_hasPending = false;

    uint64_t m_same = 0, m_differ = 0, m_boardOnly = 0, m_gameOnly = 0;
    uint64_t m_firstBoardOnly = 0;
    uint64_t m_listsSame = 0, m_listsDiffer = 0, m_listsBefore = 0;
};

// The rewrites and hooks of Capcom's video library that the game's game.cfg asks for (video/lifts.cpp)
void RegisterVideoLifts();

} // namespace cps2recomp
