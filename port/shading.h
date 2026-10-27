// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// Shaders: the picture drawn through a RetroArch shader preset (a .slangp: CRT, scanlines, xBRZ, ScaleFX and the
// rest), by librashader (https://github.com/SnowflakePowered/librashader), through OpenGL. librashader is a library
// beside the program (librashader.dylib, .so, .dll), opened when a shader is first asked for: without it, or with
// a preset it cannot make, there is no shader and the picture goes to the window as it always did (present.h).
// librashader needs the window's own OpenGL, which SDL's GPU API keeps to itself, so a window that shows a shader
// is an OpenGL window (SDL_WINDOW_OPENGL) with nothing else drawing to it.
#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

namespace port {

class Shading {
public:
    ~Shading();
    // True if librashader is there to be used (it is opened the first time this is asked); false with the reason
    static bool Available(std::string &error);
    // Takes the window's OpenGL (an OpenGL window, SDL_WINDOW_OPENGL); false with the reason
    bool Open(SDL_Window *window, std::string &error);
    void Close();
    // The preset to draw with (a .slangp file); false with the reason, and the one before is kept
    bool Load(const std::string &preset, std::string &error);
    const std::string &Preset() const {
        return m_preset;
    }
    // The window's size in pixels
    void Size(int &width, int &height) const;
    // Shows a picture (`width` x `height` pixels, each red, green, blue and a spare byte in memory) in the
    // rectangle `where` of the window, the rest black: through the preset, or (`shade` false: the menu, which
    // is drawn larger than the game's picture) only fitted, smoothly
    void Show(const uint32_t *pixels, int width, int height, const SDL_FRect &where, bool shade);

private:
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_context = nullptr;
    void *m_chain = nullptr; // (librashader's libra_gl_filter_chain_t)
    std::string m_preset;
    // (the picture's texture, and the one the preset draws into, of `where`'s size; a framebuffer for each)
    uint32_t m_picture = 0, m_drawn = 0, m_pictureFrame = 0, m_drawnFrame = 0;
    int m_width = 0, m_height = 0, m_drawnWidth = 0, m_drawnHeight = 0;
    size_t m_frame = 0;
};

} // namespace port
