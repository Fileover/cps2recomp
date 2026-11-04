// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.

#include "native.h"

namespace cps2recomp {

namespace native_video {

    namespace {
        inline uint16_t Word(const uint8_t *p) {
            return static_cast<uint16_t>((p[0] << 8) | p[1]);
        }
        constexpr int kPaletteBase = 5; // CPS-A's register (word index)
    } // namespace

    // The chip walks the pages from its base: a page taken is 0x400 bytes of the buffer; a page left out after the
    // first one taken still takes its room (Video::CopyPalette)
    PaletteUpload GamePalette(const uint8_t *videoRam, uint32_t source, uint8_t pages) {
        PaletteUpload upload;
        upload.pages = pages & 0x3F;
        upload.source = source;
        uint32_t at = (source - 0x900000) & 0x3FFFF;
        const uint32_t first = at;
        for (int page = 0; page < kPages; page++) {
            if (upload.pages & (1u << page)) {
                for (int n = 0; n < kPageColours; n++, at += 2) {
                    upload.colours[page][n] = Word(&videoRam[at & 0x3FFFF]);
                }
            } else if (at != first) {
                at += 0x400;
            }
        }
        return upload;
    }

    PaletteUpload DecodePalette(const uint8_t *videoRam, const uint16_t *cpsA, const uint16_t *cpsB, const Board &board) {
        const uint32_t base = (static_cast<uint32_t>(cpsA[kPaletteBase]) * 256) & ~0x3FFu & 0x3FFFF;
        return GamePalette(videoRam, 0x900000 + base, static_cast<uint8_t>(cpsB[board.paletteControl / 2]));
    }

    void PaletteUpload::Describe(const PaletteUpload &other, FILE *out) const {
        fprintf(out, "  pages %02X (the other: %02X), buffer %06X (the other: %06X)\n", pages, other.pages, source, other.source);
        int shown = 0;
        for (int page = 0; page < kPages; page++) {
            for (int n = 0; n < kPageColours; n++) {
                if (colours[page][n] != other.colours[page][n] && shown++ < 8) {
                    fprintf(out, "  page %d colour %03X: %04X (the other: %04X)\n", page, n, colours[page][n], other.colours[page][n]);
                }
            }
        }
    }

} // namespace native_video

void NativeVideo::Palette(const native_video::PaletteUpload &upload) {
    if (!checking) {
        return;
    }
    if (m_hasPending) {
        m_gameOnly++;
        fprintf(stderr, "video check: frame %llu: a palette upload the game's side gave, the board did not make\n",
                static_cast<unsigned long long>(frame));
    }
    m_pending = upload;
    m_hasPending = true;
}

void NativeVideo::BoardPalette(const native_video::PaletteUpload &upload) {
    if (!checking) {
        return;
    }
    if (!m_hasPending) {
        // (code that is not native yet made it: counted, the first frame kept)
        if (m_boardOnly++ == 0) {
            m_firstBoardOnly = frame;
        }
        return;
    }
    m_hasPending = false;
    if (upload == m_pending) {
        m_same++;
        return;
    }
    if (m_differ++ < 10) {
        fprintf(stderr, "video check: frame %llu: the palette's upload differs; the game's side:\n", static_cast<unsigned long long>(frame));
        m_pending.Describe(upload, stderr);
    }
}

bool NativeVideo::Report(FILE *out) const {
    fprintf(out, "video check: palette uploads: %llu the same from the game's side and the board's, %llu differ, %llu only the game's side gave\n",
            static_cast<unsigned long long>(m_same), static_cast<unsigned long long>(m_differ), static_cast<unsigned long long>(m_gameOnly));
    if (m_boardOnly) {
        fprintf(out, "video check: %llu palette uploads made by the game's code that is not native yet (the first in frame %llu)\n",
                static_cast<unsigned long long>(m_boardOnly), static_cast<unsigned long long>(m_firstBoardOnly));
    }
    return m_differ == 0 && m_gameOnly == 0 && m_boardOnly == 0;
}

} // namespace cps2recomp
