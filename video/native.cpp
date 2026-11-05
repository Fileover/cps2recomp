// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.

#include "native.h"

#include "../machine/m68k.h"

#include <cstring>

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

    ObjectList DecodeObjects(const uint8_t *bank, const uint16_t *objectRegs) {
        ObjectList list;
        for (int n = 0; n < kObjectWords; n++) {
            list.words[n] = Word(&bank[n * 2]);
        }
        // (the registers Video reads: 400004 the priority control, 400008 and 40000A the offsets)
        list.priority = objectRegs[2];
        list.xOffset = objectRegs[4];
        list.yOffset = objectRegs[5];
        return list;
    }

    void ObjectList::Describe(const ObjectList &other, FILE *out) const {
        fprintf(out, "  priority %04X (the other: %04X), offsets %04X %04X (the other: %04X %04X)\n", priority, other.priority, xOffset,
                yOffset, other.xOffset, other.yOffset);
        int shown = 0;
        for (int n = 0; n < kObjectWords; n += 4) {
            if (memcmp(&words[n], &other.words[n], 8) != 0 && shown++ < 8) {
                fprintf(out, "  entry %d: %04X %04X %04X %04X (the other: %04X %04X %04X %04X)\n", n / 4, words[n], words[n + 1], words[n + 2],
                        words[n + 3], other.words[n], other.words[n + 1], other.words[n + 2], other.words[n + 3]);
            }
        }
    }

    Layers DecodeLayers(const uint16_t *cpsA, const uint16_t *cpsB, const uint16_t *objectRegs, uint8_t layerControl) {
        // (CPS-A's registers, by word: 1-3 the maps, 4 the row scroll's table, 6-11 the scrolls across and down,
        // 0x10 the row scroll's place, 0x11 the video control; video.cpp)
        Layers layers;
        for (int n = 0; n < 3; n++) {
            layers.maps[n] = cpsA[1 + n];
            layers.scroll[n][0] = cpsA[6 + 2 * n];
            layers.scroll[n][1] = cpsA[7 + 2 * n];
        }
        layers.rows = cpsA[4];
        layers.rowOffset = cpsA[0x10];
        layers.videoControl = cpsA[0x11];
        layers.layerControl = cpsB[layerControl / 2];
        layers.xOffset = objectRegs[4];
        layers.yOffset = objectRegs[5];
        return layers;
    }

    void Layers::Describe(const Layers &other, FILE *out) const {
        fprintf(out, "  maps %04X %04X %04X (the other: %04X %04X %04X), rows %04X (%04X) at %04X (%04X), video control %04X (%04X)\n",
                maps[0], maps[1], maps[2], other.maps[0], other.maps[1], other.maps[2], rows, other.rows, rowOffset, other.rowOffset,
                videoControl, other.videoControl);
        fprintf(out, "  scrolls %04X,%04X %04X,%04X %04X,%04X (the other: %04X,%04X %04X,%04X %04X,%04X), layer control %04X (%04X), "
                     "object offsets %04X,%04X (%04X,%04X)\n",
                scroll[0][0], scroll[0][1], scroll[1][0], scroll[1][1], scroll[2][0], scroll[2][1], other.scroll[0][0], other.scroll[0][1],
                other.scroll[1][0], other.scroll[1][1], other.scroll[2][0], other.scroll[2][1], layerControl, other.layerControl, xOffset,
                yOffset, other.xOffset, other.yOffset);
    }

    bool IsVideoRegister(uint32_t address) {
        address &= 0xFFFFFF;
        const uint32_t chip = address & ~0x4000u;
        return (address >= 0x400000 && address < 0x40000C) || address == 0x8040E0 || address == 0x8040E1 ||
               (chip >= 0x800100 && chip < 0x800180);
    }

} // namespace native_video

void NativeVideo::Reset(const uint16_t *cpsA, const uint16_t *cpsB, const uint16_t *objectRegs) {
    memcpy(m_cpsA, cpsA, sizeof(m_cpsA));
    memcpy(m_cpsB, cpsB, sizeof(m_cpsB));
    memcpy(m_objectRegs, objectRegs, sizeof(m_objectRegs));
    m_bank = -1;
    m_hasPending = false;
}

void NativeVideo::GameWrites(M68K &c, uint32_t address, uint16_t value, bool byte) {
    address &= 0xFFFFFF;
    // (a byte is half of its word: the board's, Machine::Write8)
    auto put = [&](uint16_t &word) {
        if (!byte) {
            word = value;
        } else if (address & 1) {
            word = static_cast<uint16_t>((word & 0xFF00) | (value & 0xFF));
        } else {
            word = static_cast<uint16_t>((word & 0x00FF) | (value << 8));
        }
        return word;
    };
    if (address >= 0x400000 && address < 0x40000C) {
        put(m_objectRegs[(address - 0x400000) / 2]);
        return;
    }
    if (address == 0x8040E0 || address == 0x8040E1) {
        if (byte && address == 0x8040E0) {
            return; // (the bank is the low byte's)
        }
        // The game hands over the list it has filled (in the bank it reaches at 708000) when it names the other bank;
        // naming the bank shown, it keeps the list it handed over last
        const int bank = value & 1;
        if (checking && bank != m_bank) {
            for (int n = 0; n < native_video::kObjectWords; n++) {
                m_list[n] = c.bus.Read16(0x708000 + static_cast<uint32_t>(n) * 2);
            }
        }
        m_bank = bank;
        return;
    }
    const uint32_t chip = address & ~0x4000u;
    if (chip >= 0x800100 && chip < 0x800140) {
        const unsigned index = (chip & 0x3E) / 2;
        const uint16_t now = put(m_cpsA[index]);
        if (index == 0x0A / 2 && checking) {
            // The palette's base: the chip takes the palette from there, the pages the game last asked for
            Palette(native_video::GamePalette(videoRam, 0x900000 + ((static_cast<uint32_t>(now) << 8) & 0x3FC00),
                                              static_cast<uint8_t>(m_cpsB[paletteControl / 2])));
        }
        return;
    }
    if (chip >= 0x800140 && chip < 0x800180) {
        put(m_cpsB[(chip & 0x3E) / 2]);
    }
}

void NativeVideo::BoardObjects(const native_video::ObjectList &list) {
    if (!checking) {
        return;
    }
    if (m_bank < 0) {
        m_listsBefore++; // (the board takes a list each frame from the start; the game's first comes later)
        return;
    }
    native_video::ObjectList mine;
    memcpy(mine.words, m_list, sizeof(mine.words));
    mine.priority = m_objectRegs[2];
    mine.xOffset = m_objectRegs[4];
    mine.yOffset = m_objectRegs[5];
    m_raster.Objects(mine); // (the frame's objects, the game's side's)
    if (mine == list) {
        m_listsSame++;
        return;
    }
    if (m_listsDiffer++ < 10) {
        fprintf(stderr, "video check: frame %llu: the object list differs; the game's side:\n", static_cast<unsigned long long>(frame));
        mine.Describe(list, stderr);
    }
}

void NativeVideo::BoardLayers(const native_video::Layers &layers, int line) {
    if (!checking) {
        return;
    }
    const native_video::Layers mine = native_video::DecodeLayers(m_cpsA, m_cpsB, m_objectRegs, layerControl);
    if (mine == layers) {
        m_linesSame++;
        return;
    }
    if (m_linesDiffer++ < 10) {
        fprintf(stderr, "video check: frame %llu, line %d: the layers' registers differ; the game's side:\n", static_cast<unsigned long long>(frame),
                line);
        mine.Describe(layers, stderr);
    }
}

void NativeVideo::BoardLine(int line, const uint32_t *pixels, bool plain) {
    if (!checking) {
        return;
    }
    if (!plain || m_bank < 0) {
        m_picturesNot++; // (a picture changed by the game's sources, or before the game's first list: not compared)
        return;
    }
    uint32_t mine[native_video::Raster::kWidth];
    m_raster.Line(line, native_video::DecodeLayers(m_cpsA, m_cpsB, m_objectRegs, layerControl), videoRam, mine);
    int x = 0;
    while (x < native_video::Raster::kWidth && (mine[x] & 0xFFFFFF) == (pixels[x] & 0xFFFFFF)) {
        x++;
    }
    if (x == native_video::Raster::kWidth) {
        m_picturesSame++;
        return;
    }
    if (m_picturesDiffer++ < 10) {
        fprintf(stderr, "video check: frame %llu, line %d: the picture drawn from the game's side differs from column %d: %06X (the "
                        "board's: %06X)\n",
                static_cast<unsigned long long>(frame), line, x, mine[x] & 0xFFFFFF, pixels[x] & 0xFFFFFF);
    }
}

void NativeVideo::Palette(const native_video::PaletteUpload &upload) {
    if (!checking) {
        return;
    }
    m_raster.Upload(upload); // (the game's side's palette: its uploads')
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
    fprintf(out, "video check: object lists: %llu the same from the game's side and the board's, %llu differ (and %llu taken before the "
                 "game gave its first)\n",
            static_cast<unsigned long long>(m_listsSame), static_cast<unsigned long long>(m_listsDiffer),
            static_cast<unsigned long long>(m_listsBefore));
    fprintf(out, "video check: lines: %llu drawn with the same layers' registers as the game's side has, %llu not\n",
            static_cast<unsigned long long>(m_linesSame), static_cast<unsigned long long>(m_linesDiffer));
    fprintf(out, "video check: picture: %llu lines drawn from the game's side's records the same as the board's, %llu not (and %llu "
                 "not compared)\n",
            static_cast<unsigned long long>(m_picturesSame), static_cast<unsigned long long>(m_picturesDiffer),
            static_cast<unsigned long long>(m_picturesNot));
    return m_differ == 0 && m_gameOnly == 0 && m_boardOnly == 0 && m_listsDiffer == 0 && m_linesDiffer == 0 && m_picturesDiffer == 0;
}

} // namespace cps2recomp
