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
// Native so far: the palette's uploads, the object list the board takes at each frame, and the layers' registers each
// line is drawn with.
#pragma once

#include "native_render.h"
#include "records.h"

#include <cstdint>
#include <cstdio>
#include <vector>

namespace cps2recomp {

class M68K;

// The two sides compared (--video check)
class NativeVideo {
public:
    // A frame as the GPU draws it (docs/NATIVE_VIDEO.md, the frame drawn at once): video RAM and the palette as they were
    // at its first line, the writes and uploads after that with the line they came before, each line's layers, the
    // frame's objects; and the board's lines, for a check
    struct FrameEvent {
        int16_t row;      // (the first line it is seen on)
        uint32_t address; // (video RAM's; or, for a palette upload, its number in the uploads, with the top bit set)
        uint16_t value;
    };
    struct FrameData {
        uint64_t frame = 0;
        const uint8_t *video = nullptr;                          // (0x30000 bytes)
        const native_video::Raster *raster = nullptr;            // (its colours, tiles and priority control)
        const std::vector<FrameEvent> *events = nullptr;
        const std::vector<native_video::PaletteUpload> *uploads = nullptr;
        const native_video::Layers *layers = nullptr;            // (224)
        native_video::PictureOptions options;
        const uint32_t *board = nullptr;                         // (224 lines of Raster::kMaxWidth)
    };
    // Told each frame drawn at once (the GPU's check, port/gpu_picture.h), before the CPU draws it
    void (*frameDrawn)(const FrameData &frame, void *user) = nullptr;
    void *frameUser = nullptr;

    bool checking = false;
    uint64_t frame = 0;                // (for the reports: the machine's frame)
    const uint8_t *videoRam = nullptr; // (the game's side reads the buffers the game fills there)
    uint8_t paletteControl = 0x30;     // (where the CPS-B's palette control is in its registers: Board::paletteControl)
    uint8_t layerControl = 0x26;       // (... its layer control: Board::layerControl)
    const uint8_t *cps1Masks = nullptr; // (a CPS-1's: where its priority masks are, Board::priority; a CPS-2's draw without them)

    // The board starts, and the game's side with it: the registers are the board's until the game says otherwise
    void Reset(const uint16_t *cpsA, const uint16_t *cpsB, const uint16_t *objectRegs);

    // The game's side: an instruction of the game's (a hook's), or a rewrite of the library, is about to write
    // `value` to the video register at `address` (a byte, or a word). The game's side keeps what the game said, and
    // makes the records it means: the palette's upload, the object list handed over.
    void GameWrites(M68K &c, uint32_t address, uint16_t value, bool byte);

    // The board's side: what the board did
    void BoardPalette(const native_video::PaletteUpload &upload);
    void BoardObjects(const native_video::ObjectList &list, const native_video::PictureOptions &options);
    void BoardLayers(const native_video::Layers &layers, int line);
    // ... a CPS-1's object list, from video RAM where its register says
    void BoardObjectsCps1(const native_video::ObjectList &list);
    // ... and a line it drew (Video::Width() pixels, as the game's sources set the picture: `options`), when it drew a
    // CPS-2's picture from its own graphics (Video::ComparablePicture): the line drawn from the game's side's records
    // alone (native_render.h) must be the same
    void BoardLine(int line, const uint32_t *pixels, const native_video::PictureOptions &options, bool comparable);
    void SetGraphics(const uint8_t *graphics, size_t size) {
        m_raster.SetGraphics(graphics, size);
    }
    // The game wrote a word of video RAM (its maps, its row scroll's table, its palettes' buffers): for the frame drawn
    // at once (below), with the line it came before
    void VideoWritten(uint32_t address);

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
    uint64_t m_linesSame = 0, m_linesDiffer = 0;
    native_video::Raster m_raster;
    uint64_t m_picturesSame = 0, m_picturesDiffer = 0, m_picturesNot = 0;

    // The frame drawn at once, as the GPU will draw it (docs/NATIVE_VIDEO.md): from video RAM and the palette as they
    // were when its first line was drawn, the writes and uploads after that put in at the line they came before, each
    // line's layers, and the frame's objects; its lines compared with the board's when the frame is over
    void FrameStart(const native_video::PictureOptions &options);
    void FrameEnd();
    bool m_inFrame = false, m_frameComparable = false;
    int m_frameRow = 0; // (the lines drawn so far)
    std::vector<uint8_t> m_frameVideo;
    native_video::Raster m_frameRaster;
    native_video::PictureOptions m_frameOptions;
    std::vector<FrameEvent> m_frameEvents;
    std::vector<native_video::PaletteUpload> m_frameUploads;
    native_video::Layers m_frameLayers[224];
    std::vector<uint32_t> m_frameBoard; // (the board's lines)
    uint64_t m_framesSame = 0, m_framesDiffer = 0, m_frameLinesDiffer = 0;
    uint64_t m_bands = 0, m_bandFrames = 0; int m_mostBands = 0; // (the lines a frame has writes before: its bands, less one)
};

// The rewrites and hooks of Capcom's video library that the game's game.cfg asks for (video/lifts.cpp)
void RegisterVideoLifts();

} // namespace cps2recomp
