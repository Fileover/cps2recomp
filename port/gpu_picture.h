// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The picture drawn on the GPU from the native interface's records (docs/NATIVE_VIDEO.md, the GPU renderer): a frame's
// record (NativeVideo::FrameData) into a texture, in three passes (video/shaders/): the scroll screens a pixel at a time,
// in bands where the game wrote its maps or the palette while the board drew; the objects as instanced tiles, the front
// one at each pixel kept by depth; the colours, with each object's level against the screens' pens. With the CPU's
// rasteriser (video/native_render.h) as the reference: --video gpu-check draws each frame here too, reads it back and
// compares it with the board's lines.
//
// So far: the board's own picture (no wider picture or other settings of a game's sources), with the objects' offsets
// the same on every line; other frames are not drawn here and are counted.
#pragma once

#include "../video/native.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace port {

class GpuPicture {
public:
    ~GpuPicture();
    // On `device` (the presenter's), or a device of its own when none (headless: no window); the graphics ROMs are put
    // on the GPU once
    bool Open(SDL_GPUDevice *device, const uint8_t *graphics, size_t size, std::string &error);
    void Close();

    // Whether a frame is one drawn here
    static bool Drawable(const cps2recomp::NativeVideo::FrameData &frame);
    // The frame drawn into Picture() (Raster::kWidth x 224, red, green, blue, alpha), in `commands`
    void Draw(SDL_GPUCommandBuffer *commands, const cps2recomp::NativeVideo::FrameData &frame);
    SDL_GPUTexture *Picture() const {
        return m_picture;
    }

    // --video gpu-check: the frame drawn, read back and compared with the board's lines
    void Check(const cps2recomp::NativeVideo::FrameData &frame);
    bool Report(FILE *out) const;

private:
    bool MakePipelines(std::string &error);
    SDL_GPUDevice *m_device = nullptr;
    bool m_ownDevice = false;
    size_t m_graphicsSize = 0;
    SDL_GPUBuffer *m_graphics = nullptr, *m_video = nullptr, *m_lines = nullptr, *m_tiles = nullptr, *m_frame = nullptr;
    size_t m_tilesCapacity = 0;
    SDL_GPUTexture *m_screens = nullptr, *m_objects = nullptr, *m_depth = nullptr, *m_lineTexture = nullptr, *m_palette = nullptr,
                   *m_picture = nullptr;
    SDL_GPUSampler *m_sampler = nullptr;
    SDL_GPUGraphicsPipeline *m_layersPipeline = nullptr, *m_objectsPipeline = nullptr, *m_coloursPipeline = nullptr;
    SDL_GPUTransferBuffer *m_upload = nullptr, *m_download = nullptr;
    size_t m_uploadSize = 0;
    std::vector<uint8_t> m_videoNow; // (video RAM as the frame's bands have it, put in as they go)
    uint64_t m_same = 0, m_differ = 0, m_notDrawn = 0, m_linesDiffer = 0;
};

} // namespace port
