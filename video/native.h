// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The picture at the game's level (docs/NATIVE_VIDEO.md): what the games' drawing code decides each frame, handed
// over as it decides it, in place of the board's registers and memories that video.cpp reads.
//
// Two sides make the same records. The game's side: the shared rewrites of Capcom's video library (video/lifts.cpp)
// hand them over from the game's own variables. The board's side: a decoder makes them from what the board model
// was given (its registers and video RAM), as video.cpp would read it. With --video check, every record from the
// board is compared with the one the game's side gave for it; a difference, or a record only one side made, is
// reported.
//
// Native so far: the palette's uploads.
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

    // The board's side: the upload the palette chip makes now, from its registers and video RAM
    // (as Video::CopyPalette reads them)
    PaletteUpload DecodePalette(const uint8_t *videoRam, const uint16_t *cpsA, const uint16_t *cpsB, const Board &board);

    // The game's side: a buffer of colours in video RAM as the game left it
    // (`pages` and the buffer's place as the game's code works them out)
    PaletteUpload GamePalette(const uint8_t *videoRam, uint32_t source, uint8_t pages);

} // namespace native_video

// The two sides compared (--video check)
class NativeVideo {
public:
    bool checking = false;
    uint64_t frame = 0;                 // (for the reports: the machine's frame)
    const uint8_t *videoRam = nullptr;  // (the game's side reads the buffers the game fills there)
    // The game's side's own state: the pages the game last asked the chip to take (the lifts keep it)
    uint8_t gamePages = 0;

    // The game's side: what a lift is about to have the board do
    void Palette(const native_video::PaletteUpload &upload);
    // The board's side: what the board did
    void BoardPalette(const native_video::PaletteUpload &upload);

    // At the end of a run: the counts, and false if anything differed or was made by code not native yet
    bool Report(FILE *out) const;

private:
    native_video::PaletteUpload m_pending;
    bool m_hasPending = false;
    uint64_t m_same = 0, m_differ = 0, m_boardOnly = 0, m_gameOnly = 0;
    uint64_t m_firstBoardOnly = 0;
};

// The shared rewrites of Capcom's video library that the game's game.cfg asks for (video/lifts.cpp)
void RegisterVideoLifts();

} // namespace cps2recomp
