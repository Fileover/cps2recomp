// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.

#include "gpu_picture.h"

#include "../video/picture_shaders.inc"

#include <algorithm>
#include <cstring>
#include <cstdlib>

namespace port {

using cps2recomp::NativeVideo;
using cps2recomp::native_video::Layers;
using cps2recomp::native_video::Raster;

namespace {
    constexpr int kWidth = Raster::kWidth, kHeight = 224;
    constexpr uint32_t kLineWords = 16;               // (video/shaders/common.glsl, L_WORDS)
    constexpr uint32_t kVideoBytes = 0x30000;
    constexpr uint32_t kPaletteWidth = 64, kPaletteHeight = 48;

    SDL_GPUShader *MakeShader(SDL_GPUDevice *device, SDL_GPUShaderStage stage, const unsigned char *spirv, size_t spirvSize,
                              const char *msl, Uint32 samplers, Uint32 storageBuffers) {
        SDL_GPUShaderCreateInfo info = {};
        if (SDL_GetGPUShaderFormats(device) & SDL_GPU_SHADERFORMAT_MSL) {
            info.format = SDL_GPU_SHADERFORMAT_MSL;
            info.code = reinterpret_cast<const Uint8 *>(msl);
            info.code_size = strlen(msl);
            info.entrypoint = "main0";
        } else {
            info.format = SDL_GPU_SHADERFORMAT_SPIRV;
            info.code = spirv;
            info.code_size = spirvSize;
            info.entrypoint = "main";
        }
        info.stage = stage;
        info.num_samplers = samplers;
        info.num_storage_buffers = storageBuffers;
        return SDL_CreateGPUShader(device, &info);
    }

    SDL_GPUTexture *MakeTexture(SDL_GPUDevice *device, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage, Uint32 width,
                                Uint32 height) {
        SDL_GPUTextureCreateInfo info = {};
        info.type = SDL_GPU_TEXTURETYPE_2D;
        info.format = format;
        info.usage = usage;
        info.width = width;
        info.height = height;
        info.layer_count_or_depth = 1;
        info.num_levels = 1;
        info.sample_count = SDL_GPU_SAMPLECOUNT_1;
        return SDL_CreateGPUTexture(device, &info);
    }

    SDL_GPUBuffer *MakeBuffer(SDL_GPUDevice *device, size_t size) {
        SDL_GPUBufferCreateInfo info = {};
        info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
        info.size = static_cast<Uint32>((size + 3) & ~size_t{3});
        return SDL_CreateGPUBuffer(device, &info);
    }

    // A line's record (video/shaders/common.glsl)
    void LineRecord(const Layers &l, uint16_t priority, uint32_t *out) {
        memset(out, 0, kLineWords * 4);
        for (int n = 0; n < 3; n++) {
            out[n] = l.maps[n];
            out[4 + 2 * n] = l.scroll[n][0];
            out[5 + 2 * n] = l.scroll[n][1];
        }
        out[3] = l.rows;
        out[10] = l.rowOffset;
        out[11] = l.videoControl;
        out[12] = l.layerControl;
        out[13] = priority;
    }
} // namespace

GpuPicture::~GpuPicture() {
    Close();
}

bool GpuPicture::Open(SDL_GPUDevice *device, const uint8_t *graphics, size_t size, std::string &error) {
    m_device = device;
    if (!m_device) {
        // (headless: SDL's video with its offscreen driver, which opens nothing, for a device with no window)
        if (!SDL_WasInit(SDL_INIT_VIDEO)) {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
                error = std::string("no video: ") + SDL_GetError();
                return false;
            }
        }
        m_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, false, nullptr);
        if (!m_device) {
            error = std::string("no GPU device: ") + SDL_GetError();
            return false;
        }
        m_ownDevice = true;
    }
    const SDL_GPUTextureUsageFlags target = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    if (!SDL_GPUTextureSupportsFormat(m_device, SDL_GPU_TEXTUREFORMAT_R32_UINT, SDL_GPU_TEXTURETYPE_2D, target) ||
        !SDL_GPUTextureSupportsFormat(m_device, SDL_GPU_TEXTUREFORMAT_D32_FLOAT, SDL_GPU_TEXTURETYPE_2D,
                                      SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)) {
        error = "the GPU has not got the targets the picture is drawn with (32-bit whole numbers, 32-bit depth)";
        return false;
    }
    m_graphicsSize = size;
    m_graphics = MakeBuffer(m_device, size);
    m_video = MakeBuffer(m_device, kVideoBytes);
    m_lines = MakeBuffer(m_device, (kHeight * kLineWords + 1) * 4);
    m_frame = MakeBuffer(m_device, 8 * 4);
    m_screens = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R32_UINT, target, kWidth, kHeight);
    m_objects = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R32_UINT, target, kWidth, kHeight);
    m_depth = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_D32_FLOAT, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET, kWidth, kHeight);
    m_lineTexture = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R32_UINT, SDL_GPU_TEXTUREUSAGE_SAMPLER, kLineWords, kHeight);
    m_palette = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R32_UINT, SDL_GPU_TEXTUREUSAGE_SAMPLER, kPaletteWidth, kPaletteHeight);
    m_picture = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, target, kWidth, kHeight);
    m_readouts = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER, kWidth, kHeight);
    m_clear = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER, kWidth, kHeight);
    m_shown = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, target, kWidth, kHeight);
    m_stack = MakeTexture(m_device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, target, kWidth, 2 * kHeight);
    SDL_GPUSamplerCreateInfo sampler = {};
    sampler.min_filter = sampler.mag_filter = SDL_GPU_FILTER_NEAREST;
    sampler.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    sampler.address_mode_u = sampler.address_mode_v = sampler.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    m_sampler = SDL_CreateGPUSampler(m_device, &sampler);
    if (!m_graphics || !m_video || !m_lines || !m_frame || !m_screens || !m_objects || !m_depth || !m_lineTexture || !m_palette ||
        !m_picture || !m_readouts || !m_clear || !m_shown || !m_stack || !m_sampler || !MakePipelines(error)) {
        if (error.empty()) {
            error = std::string("the picture's GPU resources: ") + SDL_GetError();
        }
        return false;
    }
    // The graphics ROMs, once
    SDL_GPUTransferBufferCreateInfo info = {};
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.size = static_cast<Uint32>((size + 3) & ~size_t{3});
    SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(m_device, &info);
    if (!transfer) {
        error = std::string("the graphics' upload: ") + SDL_GetError();
        return false;
    }
    auto *to = static_cast<uint8_t *>(SDL_MapGPUTransferBuffer(m_device, transfer, false));
    memset(to, 0, info.size);
    memcpy(to, graphics, size);
    SDL_UnmapGPUTransferBuffer(m_device, transfer);
    SDL_GPUCommandBuffer *commands = SDL_AcquireGPUCommandBuffer(m_device);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTransferBufferLocation from = {transfer, 0};
    SDL_GPUBufferRegion region = {m_graphics, 0, info.size};
    SDL_UploadToGPUBuffer(copy, &from, &region, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(commands);
    SDL_ReleaseGPUTransferBuffer(m_device, transfer);
    // (the readouts' picture: the transfer for it; and one that is clear all over, for a picture with none)
    info.size = kWidth * kHeight * 4;
    m_readoutsUpload = SDL_CreateGPUTransferBuffer(m_device, &info);
    if (!m_readoutsUpload) {
        error = std::string("the readouts' upload: ") + SDL_GetError();
        return false;
    }
    memset(SDL_MapGPUTransferBuffer(m_device, m_readoutsUpload, false), 0, info.size);
    SDL_UnmapGPUTransferBuffer(m_device, m_readoutsUpload);
    commands = SDL_AcquireGPUCommandBuffer(m_device);
    copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTextureTransferInfo clearFrom = {m_readoutsUpload, 0, kWidth, kHeight};
    SDL_GPUTextureRegion clearTo = {};
    clearTo.texture = m_clear;
    clearTo.w = kWidth;
    clearTo.h = kHeight;
    clearTo.d = 1;
    SDL_UploadToGPUTexture(copy, &clearFrom, &clearTo, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(commands);
    m_videoNow.assign(kVideoBytes, 0);
    return true;
}

bool GpuPicture::MakePipelines(std::string &error) {
    SDL_GPUShader *fill = MakeShader(m_device, SDL_GPU_SHADERSTAGE_VERTEX, k_fill_vert_spirv, sizeof(k_fill_vert_spirv), k_fill_vert_msl, 0, 0);
    SDL_GPUShader *layers = MakeShader(m_device, SDL_GPU_SHADERSTAGE_FRAGMENT, k_layers_frag_spirv, sizeof(k_layers_frag_spirv),
                                       k_layers_frag_msl, 0, 3);
    SDL_GPUShader *objectsVert = MakeShader(m_device, SDL_GPU_SHADERSTAGE_VERTEX, k_objects_vert_spirv, sizeof(k_objects_vert_spirv),
                                            k_objects_vert_msl, 0, 2);
    SDL_GPUShader *objectsFrag = MakeShader(m_device, SDL_GPU_SHADERSTAGE_FRAGMENT, k_objects_frag_spirv, sizeof(k_objects_frag_spirv),
                                            k_objects_frag_msl, 0, 1);
    SDL_GPUShader *colours = MakeShader(m_device, SDL_GPU_SHADERSTAGE_FRAGMENT, k_colours_frag_spirv, sizeof(k_colours_frag_spirv),
                                        k_colours_frag_msl, 4, 0);
    SDL_GPUShader *shown = MakeShader(m_device, SDL_GPU_SHADERSTAGE_FRAGMENT, k_shown_frag_spirv, sizeof(k_shown_frag_spirv),
                                      k_shown_frag_msl, 2, 0);
    bool ok = fill && layers && objectsVert && objectsFrag && colours && shown;
    if (ok) {
        SDL_GPUGraphicsPipelineCreateInfo info = {};
        info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
        info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
        SDL_GPUColorTargetDescription uintTarget = {};
        uintTarget.format = SDL_GPU_TEXTUREFORMAT_R32_UINT;
        info.target_info.color_target_descriptions = &uintTarget;
        info.target_info.num_color_targets = 1;
        info.vertex_shader = fill;
        info.fragment_shader = layers;
        m_layersPipeline = SDL_CreateGPUGraphicsPipeline(m_device, &info);
        info.vertex_shader = objectsVert;
        info.fragment_shader = objectsFrag;
        info.depth_stencil_state.enable_depth_test = true;
        info.depth_stencil_state.enable_depth_write = true;
        info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
        info.target_info.has_depth_stencil_target = true;
        info.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
        m_objectsPipeline = SDL_CreateGPUGraphicsPipeline(m_device, &info);
        info.depth_stencil_state = {};
        info.target_info.has_depth_stencil_target = false;
        SDL_GPUColorTargetDescription picture = {};
        picture.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        info.target_info.color_target_descriptions = &picture;
        info.vertex_shader = fill;
        info.fragment_shader = colours;
        m_coloursPipeline = SDL_CreateGPUGraphicsPipeline(m_device, &info);
        info.fragment_shader = shown;
        m_shownPipeline = SDL_CreateGPUGraphicsPipeline(m_device, &info);
        ok = m_layersPipeline && m_objectsPipeline && m_coloursPipeline && m_shownPipeline;
    }
    if (!ok) {
        error = std::string("the picture's shaders: ") + SDL_GetError();
    }
    for (SDL_GPUShader *shader : {fill, layers, objectsVert, objectsFrag, colours, shown}) {
        if (shader) {
            SDL_ReleaseGPUShader(m_device, shader);
        }
    }
    return ok;
}

void GpuPicture::Close() {
    if (!m_device) {
        return;
    }
    for (SDL_GPUBuffer *buffer : {m_graphics, m_video, m_lines, m_tiles, m_frame}) {
        if (buffer) {
            SDL_ReleaseGPUBuffer(m_device, buffer);
        }
    }
    for (SDL_GPUTexture *texture : {m_screens, m_objects, m_depth, m_lineTexture, m_palette, m_picture, m_readouts, m_clear, m_shown, m_stack}) {
        if (texture) {
            SDL_ReleaseGPUTexture(m_device, texture);
        }
    }
    for (SDL_GPUGraphicsPipeline *pipeline : {m_layersPipeline, m_objectsPipeline, m_coloursPipeline, m_shownPipeline}) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(m_device, pipeline);
        }
    }
    if (m_sampler) {
        SDL_ReleaseGPUSampler(m_device, m_sampler);
    }
    for (SDL_GPUTransferBuffer *transfer : {m_upload, m_download, m_readoutsUpload}) {
        if (transfer) {
            SDL_ReleaseGPUTransferBuffer(m_device, transfer);
        }
    }
    if (m_ownDevice) {
        SDL_DestroyGPUDevice(m_device);
    }
    m_device = nullptr;
    m_ownDevice = false;
    m_graphics = m_video = m_lines = m_tiles = m_frame = nullptr;
    m_tilesCapacity = 0;
    m_screens = m_objects = m_depth = m_lineTexture = m_palette = m_picture = m_readouts = m_clear = m_shown = m_stack = nullptr;
    m_sampler = nullptr;
    m_layersPipeline = m_objectsPipeline = m_coloursPipeline = m_shownPipeline = nullptr;
    m_upload = m_download = m_readoutsUpload = nullptr;
    m_uploadSize = 0;
}

bool GpuPicture::Drawable(const NativeVideo::FrameData &frame) {
    if (!frame.whole) {
        return false;
    }
    const auto &o = frame.options;
    if (o.side || o.scroll[0] || o.scroll[1] || o.scroll[2] || o.rows || o.object || o.tile || o.fillHoles || o.hideObject ||
        o.markObjects || o.markLayers || o.layerMask != 0xF) {
        return false;
    }
    // (the objects are placed once a frame: their offsets the same on every line)
    for (int line = 1; line < kHeight; line++) {
        if (frame.layers[line].xOffset != frame.layers[0].xOffset || frame.layers[line].yOffset != frame.layers[0].yOffset) {
            return false;
        }
    }
    return true;
}

void GpuPicture::Draw(SDL_GPUCommandBuffer *commands, const NativeVideo::FrameData &frame) {
    const Raster &raster = *frame.raster;
    const std::vector<cps2recomp::native_video::ObjectTile> &tiles = raster.Tiles();
    const auto &events = *frame.events;
    // The bands: where the frame has writes or uploads, at a line after the first
    std::vector<int> starts = {0};
    for (const NativeVideo::FrameEvent &e : events) {
        if (e.row > starts.back() && e.row < kHeight) {
            starts.push_back(e.row);
        }
    }
    starts.push_back(kHeight);
    // What goes up, laid out in one transfer buffer: video RAM; the lines' records (for the first pass, and as a texture
    // for the third); the tiles; the frame's numbers; and for each band its palette and its part of video RAM
    const size_t linesBytes = (kHeight * kLineWords + 1) * 4, tilesBytes = std::max<size_t>(tiles.size(), 1) * 16;
    const size_t paletteBytes = kPaletteWidth * kPaletteHeight * 4;
    const size_t need = kVideoBytes + linesBytes + tilesBytes + 32 + starts.size() * (paletteBytes + kVideoBytes);
    if (need > m_uploadSize) {
        if (m_upload) {
            SDL_ReleaseGPUTransferBuffer(m_device, m_upload);
        }
        SDL_GPUTransferBufferCreateInfo info = {};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        info.size = static_cast<Uint32>(need);
        m_upload = SDL_CreateGPUTransferBuffer(m_device, &info);
        m_uploadSize = need;
    }
    if (tiles.size() * 16 > m_tilesCapacity || !m_tiles) {
        if (m_tiles) {
            SDL_ReleaseGPUBuffer(m_device, m_tiles);
        }
        m_tilesCapacity = std::max<size_t>(tilesBytes, 16 * 4096);
        m_tiles = MakeBuffer(m_device, m_tilesCapacity);
    }
    auto *up = static_cast<uint8_t *>(SDL_MapGPUTransferBuffer(m_device, m_upload, true));
    size_t at = 0;
    const size_t videoAt = at;
    memcpy(up + at, frame.video, kVideoBytes);
    m_videoNow.assign(frame.video, frame.video + kVideoBytes);
    at += kVideoBytes;
    const size_t linesAt = at;
    auto *lineWords = reinterpret_cast<uint32_t *>(up + at);
    for (int line = 0; line < kHeight; line++) {
        LineRecord(frame.layers[line], raster.Priority(), &lineWords[line * kLineWords]);
    }
    lineWords[kHeight * kLineWords] = static_cast<uint32_t>(m_graphicsSize);
    at += linesBytes;
    const size_t tilesAt = at;
    auto *tileWords = reinterpret_cast<uint32_t *>(up + at);
    for (size_t n = 0; n < tiles.size(); n++) {
        const auto &t = tiles[n];
        tileWords[n * 4] = static_cast<uint32_t>(static_cast<uint16_t>(t.x)) | (static_cast<uint32_t>(static_cast<uint16_t>(t.y)) << 16);
        tileWords[n * 4 + 1] = 0;
        tileWords[n * 4 + 2] = t.tile;
        tileWords[n * 4 + 3] = t.colour | (t.flipX ? 1u << 16 : 0) | (t.flipY ? 1u << 17 : 0) | (t.relative ? 1u << 19 : 0) |
                               (static_cast<uint32_t>(t.level) << 20);
    }
    at += tilesBytes;
    const size_t frameAt = at;
    auto *frameWords = reinterpret_cast<uint32_t *>(up + at);
    frameWords[0] = frame.layers[0].xOffset;
    frameWords[1] = frame.layers[0].yOffset;
    frameWords[2] = static_cast<uint32_t>(tiles.size());
    frameWords[3] = kWidth;
    frameWords[4] = static_cast<uint32_t>(m_graphicsSize);
    at += 32;
    // Each band's palette and video RAM's range written before it
    Raster colours = raster; // (the palette, as the uploads change it band by band)
    struct Band {
        size_t paletteAt, videoAt;
        uint32_t videoFrom, videoSize;
    };
    std::vector<Band> bands;
    size_t next = 0;
    for (size_t b = 0; b + 1 < starts.size(); b++) {
        uint32_t low = kVideoBytes, high = 0;
        for (; next < events.size() && events[next].row <= starts[b]; next++) {
            const NativeVideo::FrameEvent &e = events[next];
            if (e.address & 0x80000000u) {
                colours.Upload((*frame.uploads)[e.address & 0x7FFFFFFF]);
            } else if (b > 0) {
                m_videoNow[e.address] = static_cast<uint8_t>(e.value >> 8);
                m_videoNow[e.address + 1] = static_cast<uint8_t>(e.value);
                low = std::min(low, e.address & ~3u);
                high = std::max(high, (e.address + 4) & ~3u);
            } else {
                // (before the first line: in the copy already, the writes having been made before it was taken)
            }
        }
        Band band = {at, 0, 0, 0};
        memcpy(up + at, colours.Colours(), paletteBytes);
        at += paletteBytes;
        if (high > low) {
            band.videoAt = at;
            band.videoFrom = low;
            band.videoSize = high - low;
            memcpy(up + at, &m_videoNow[low], high - low);
            at += high - low;
        }
        bands.push_back(band);
    }
    SDL_UnmapGPUTransferBuffer(m_device, m_upload);

    // Up: video RAM as the frame began, the lines, the tiles, the frame's numbers
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(commands);
    auto upload = [&](size_t from, SDL_GPUBuffer *buffer, uint32_t offset, size_t size) {
        SDL_GPUTransferBufferLocation location = {m_upload, static_cast<Uint32>(from)};
        SDL_GPUBufferRegion region = {buffer, offset, static_cast<Uint32>(size)};
        SDL_UploadToGPUBuffer(copy, &location, &region, false);
    };
    auto uploadTexture = [&](size_t from, SDL_GPUTexture *texture, Uint32 width, Uint32 height) {
        SDL_GPUTextureTransferInfo source = {m_upload, static_cast<Uint32>(from), width, height};
        SDL_GPUTextureRegion region = {};
        region.texture = texture;
        region.w = width;
        region.h = height;
        region.d = 1;
        SDL_UploadToGPUTexture(copy, &source, &region, false);
    };
    upload(videoAt, m_video, 0, kVideoBytes);
    upload(linesAt, m_lines, 0, linesBytes);
    uploadTexture(linesAt, m_lineTexture, kLineWords, kHeight);
    if (!tiles.empty()) {
        upload(tilesAt, m_tiles, 0, tiles.size() * 16);
    }
    upload(frameAt, m_frame, 0, 32);
    SDL_EndGPUCopyPass(copy);

    // The objects, once for the frame: the front one's pen and level at each pixel
    SDL_GPUViewport viewport = {0, 0, static_cast<float>(kWidth), static_cast<float>(kHeight), 0, 1};
    {
        SDL_GPUColorTargetInfo target = {};
        target.texture = m_objects;
        target.load_op = SDL_GPU_LOADOP_CLEAR;
        target.store_op = SDL_GPU_STOREOP_STORE;
        SDL_GPUDepthStencilTargetInfo depth = {};
        depth.texture = m_depth;
        depth.clear_depth = 1.0f;
        depth.load_op = SDL_GPU_LOADOP_CLEAR;
        depth.store_op = SDL_GPU_STOREOP_DONT_CARE;
        depth.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
        depth.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
        SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(commands, &target, 1, &depth);
        SDL_SetGPUViewport(pass, &viewport);
        if (!tiles.empty()) {
            SDL_BindGPUGraphicsPipeline(pass, m_objectsPipeline);
            SDL_GPUBuffer *vertexBuffers[2] = {m_tiles, m_frame};
            SDL_BindGPUVertexStorageBuffers(pass, 0, vertexBuffers, 2);
            SDL_BindGPUFragmentStorageBuffers(pass, 0, &m_graphics, 1);
            SDL_DrawGPUPrimitives(pass, 6, static_cast<Uint32>(tiles.size()), 0, 0);
        }
        SDL_EndGPURenderPass(pass);
    }
    // Band by band: its part of video RAM and its palette up, then the scroll screens and the colours on its lines
    for (size_t b = 0; b < bands.size(); b++) {
        const Band &band = bands[b];
        copy = SDL_BeginGPUCopyPass(commands);
        if (band.videoSize) {
            upload(band.videoAt, m_video, band.videoFrom, band.videoSize);
        }
        uploadTexture(band.paletteAt, m_palette, kPaletteWidth, kPaletteHeight);
        SDL_EndGPUCopyPass(copy);
        SDL_Rect scissor = {0, starts[b], kWidth, starts[b + 1] - starts[b]};
        {
            SDL_GPUColorTargetInfo target = {};
            target.texture = m_screens;
            target.load_op = b == 0 ? SDL_GPU_LOADOP_DONT_CARE : SDL_GPU_LOADOP_LOAD;
            target.store_op = SDL_GPU_STOREOP_STORE;
            SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(commands, &target, 1, nullptr);
            SDL_SetGPUViewport(pass, &viewport);
            SDL_SetGPUScissor(pass, &scissor);
            SDL_BindGPUGraphicsPipeline(pass, m_layersPipeline);
            SDL_GPUBuffer *buffers[3] = {m_graphics, m_video, m_lines};
            SDL_BindGPUFragmentStorageBuffers(pass, 0, buffers, 3);
            SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
            SDL_EndGPURenderPass(pass);
        }
        {
            SDL_GPUColorTargetInfo target = {};
            target.texture = m_picture;
            target.load_op = b == 0 ? SDL_GPU_LOADOP_DONT_CARE : SDL_GPU_LOADOP_LOAD;
            target.store_op = SDL_GPU_STOREOP_STORE;
            SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(commands, &target, 1, nullptr);
            SDL_SetGPUViewport(pass, &viewport);
            SDL_SetGPUScissor(pass, &scissor);
            SDL_BindGPUGraphicsPipeline(pass, m_coloursPipeline);
            SDL_GPUTextureSamplerBinding bindings[4] = {
                {m_screens, m_sampler}, {m_objects, m_sampler}, {m_lineTexture, m_sampler}, {m_palette, m_sampler}};
            SDL_BindGPUFragmentSamplers(pass, 0, bindings, 4);
            SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
            SDL_EndGPURenderPass(pass);
        }
    }
}

void GpuPicture::Keep(const NativeVideo::FrameData &frame) {
    m_kept.frame = frame.frame;
    m_kept.video.assign(frame.video, frame.video + kVideoBytes);
    m_kept.raster = *frame.raster;
    m_kept.events = *frame.events;
    m_kept.uploads = *frame.uploads;
    std::copy(frame.layers, frame.layers + kHeight, m_kept.layers);
    m_kept.options = frame.options;
    m_kept.drawable = Drawable(frame);
    m_hasKept = true;
}

SDL_GPUTexture *GpuPicture::DrawKept(SDL_GPUCommandBuffer *commands, const uint32_t *readouts, bool zoomed) {
    if (!CanDrawKept()) {
        return nullptr;
    }
    NativeVideo::FrameData frame;
    frame.frame = m_kept.frame;
    frame.video = m_kept.video.data();
    frame.raster = &m_kept.raster;
    frame.events = &m_kept.events;
    frame.uploads = &m_kept.uploads;
    frame.layers = m_kept.layers;
    frame.options = m_kept.options;
    Draw(commands, frame);
    if (readouts) {
        memcpy(SDL_MapGPUTransferBuffer(m_device, m_readoutsUpload, true), readouts, kWidth * kHeight * 4);
        SDL_UnmapGPUTransferBuffer(m_device, m_readoutsUpload);
        SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(commands);
        SDL_GPUTextureTransferInfo from = {m_readoutsUpload, 0, kWidth, kHeight};
        SDL_GPUTextureRegion to = {};
        to.texture = m_readouts;
        to.w = kWidth;
        to.h = kHeight;
        to.d = 1;
        SDL_UploadToGPUTexture(copy, &from, &to, true);
        SDL_EndGPUCopyPass(copy);
    }
    // The picture with the readouts over it; zoomed, the game's alone above that (rows 224-447)
    SDL_GPUTexture *out = zoomed ? m_stack : m_shown;
    SDL_GPUColorTargetInfo target = {};
    target.texture = out;
    target.load_op = SDL_GPU_LOADOP_DONT_CARE;
    target.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(commands, &target, 1, nullptr);
    SDL_BindGPUGraphicsPipeline(pass, m_shownPipeline);
    auto half = [&](int top, SDL_GPUTexture *over) {
        const SDL_GPUViewport viewport = {0, static_cast<float>(top), static_cast<float>(kWidth), static_cast<float>(kHeight), 0, 1};
        SDL_SetGPUViewport(pass, &viewport);
        SDL_GPUTextureSamplerBinding bindings[2] = {{m_picture, m_sampler}, {over, m_sampler}};
        SDL_BindGPUFragmentSamplers(pass, 0, bindings, 2);
        SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    };
    half(0, zoomed || !readouts ? m_clear : m_readouts);
    if (zoomed) {
        half(kHeight, readouts ? m_readouts : m_clear);
    }
    SDL_EndGPURenderPass(pass);
    return out;
}

void GpuPicture::Check(const NativeVideo::FrameData &frame) {
    if (!Drawable(frame)) {
        m_notDrawn++;
        return;
    }
    if (!m_download) {
        SDL_GPUTransferBufferCreateInfo info = {};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
        info.size = kWidth * kHeight * 4;
        m_download = SDL_CreateGPUTransferBuffer(m_device, &info);
    }
    SDL_GPUCommandBuffer *commands = SDL_AcquireGPUCommandBuffer(m_device);
    Draw(commands, frame);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTextureRegion region = {};
    region.texture = m_picture;
    region.w = kWidth;
    region.h = kHeight;
    region.d = 1;
    SDL_GPUTextureTransferInfo to = {m_download, 0, kWidth, kHeight};
    SDL_DownloadFromGPUTexture(copy, &region, &to);
    SDL_EndGPUCopyPass(copy);
    SDL_GPUFence *fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    SDL_WaitForGPUFences(m_device, true, &fence, 1);
    SDL_ReleaseGPUFence(m_device, fence);
    const auto *pixels = static_cast<const uint32_t *>(SDL_MapGPUTransferBuffer(m_device, m_download, false));
    int differ = 0, firstLine = -1, firstColumn = -1;
    uint32_t mine = 0, board = 0;
    for (int line = 0; line < kHeight; line++) {
        const uint32_t *row = &frame.board[static_cast<size_t>(line) * Raster::kMaxWidth];
        for (int x = 0; x < kWidth; x++) {
            // (red, green, blue, alpha bytes: the board's red, green, blue in the low three)
            if ((pixels[line * kWidth + x] & 0xFFFFFF) != row[x]) {
                if (firstLine < 0) {
                    firstLine = line;
                    firstColumn = x;
                    mine = pixels[line * kWidth + x] & 0xFFFFFF;
                    board = row[x];
                }
                differ++;
                break;
            }
        }
    }
    SDL_UnmapGPUTransferBuffer(m_device, m_download);
    if (!differ) {
        m_same++;
        return;
    }
    m_linesDiffer += static_cast<uint64_t>(differ);
    if (m_differ++ < 10) {
        fprintf(stderr, "video gpu-check: frame %llu: %d lines differ from the board's, the first at line %d, column %d: %06X (the "
                        "board's: %06X)\n",
                static_cast<unsigned long long>(frame.frame), differ, firstLine, firstColumn, mine, board);
    }
}

void GpuPicture::CheckShown(const uint32_t *readouts, const uint32_t *shown) {
    if (!m_download) {
        return; // (Check makes it, at the first frame drawn)
    }
    SDL_GPUCommandBuffer *commands = SDL_AcquireGPUCommandBuffer(m_device);
    SDL_GPUTexture *texture = DrawKept(commands, readouts, false);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTextureRegion region = {};
    region.texture = texture;
    region.w = kWidth;
    region.h = kHeight;
    region.d = 1;
    SDL_GPUTextureTransferInfo to = {m_download, 0, kWidth, kHeight};
    SDL_DownloadFromGPUTexture(copy, &region, &to);
    SDL_EndGPUCopyPass(copy);
    SDL_GPUFence *fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    SDL_WaitForGPUFences(m_device, true, &fence, 1);
    SDL_ReleaseGPUFence(m_device, fence);
    const auto *pixels = static_cast<const uint32_t *>(SDL_MapGPUTransferBuffer(m_device, m_download, false));
    bool any = false;
    int worst = 0, at = -1;
    for (int n = 0; n < kWidth * kHeight; n++) {
        any = any || (readouts[n] >> 24) != 0;
        for (int shift = 0; shift < 24; shift += 8) {
            const int d = std::abs(static_cast<int>((pixels[n] >> shift) & 0xFF) - static_cast<int>((shown[n] >> shift) & 0xFF));
            if (d > worst) {
                worst = d;
                at = n;
            }
        }
    }
    SDL_UnmapGPUTransferBuffer(m_device, m_download);
    m_shownWithReadouts += any ? 1 : 0;
    if (worst <= 1) {
        m_shownSame++;
        return;
    }
    if (m_shownDiffer++ < 10) {
        fprintf(stderr, "video gpu-check: frame %llu: shown with the readouts, %d off at line %d, column %d\n",
                static_cast<unsigned long long>(m_kept.frame), worst, at / kWidth, at % kWidth);
    }
}

bool GpuPicture::Report(FILE *out) const {
    fprintf(out, "video gpu-check: frames drawn on the GPU: %llu the same as the board's, %llu not (%llu lines); %llu not drawn there\n",
            static_cast<unsigned long long>(m_same), static_cast<unsigned long long>(m_differ), static_cast<unsigned long long>(m_linesDiffer),
            static_cast<unsigned long long>(m_notDrawn));
    fprintf(out, "video gpu-check: shown with the readouts over it: %llu within a step of the CPU's (%llu with readouts), %llu not\n",
            static_cast<unsigned long long>(m_shownSame), static_cast<unsigned long long>(m_shownWithReadouts),
            static_cast<unsigned long long>(m_shownDiffer));
    return m_differ == 0 && m_shownDiffer == 0;
}

} // namespace port
