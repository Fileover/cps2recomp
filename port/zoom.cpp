// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The camera's zoom (zoom.h).
#include "zoom.h"

#include "../runtime/game.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace port {

namespace {
    // How much of the way to what is wanted each picture goes: the size (most of the way in about a third of a
    // second), and the place looked at when it changes fighter (the camera is on its fighter otherwise)
    constexpr float kSizeEase = 0.15f, kPlaceEase = 0.2f;
    constexpr int kLabelTicks = 150; // (two and a half seconds)
    constexpr float kPanStep = 2.0f;  // the picture's pixels a step of Pan moves the camera (a step a picture)
    constexpr float kPanMost = 160.0f;
} // namespace

bool Zoom::Possible() {
    return cps2recomp::TheGame().focus != nullptr;
}

void Zoom::Said() {
    if (m_size == 0) {
        snprintf(m_label, sizeof(m_label), "ZOOM OFF");
    } else {
        snprintf(m_label, sizeof(m_label), "ZOOM %gx  PLAYER %d", static_cast<double>(kSizes[m_size]), m_player + 1);
    }
    m_labelTicks = kLabelTicks;
}

void Zoom::Press(int player) {
    if (!Possible()) {
        return;
    }
    if (m_size == 0 || player == m_player) {
        m_size = (m_size + 1) % kSizeCount;
    }
    if (m_size == 0 || player != m_player) {
        m_panX = m_panY = 0; // (framed afresh: the moved camera was for the other fighter)
    }
    m_player = player;
    Said();
}

void Zoom::Pan(int across, int down) {
    if (m_size == 0) {
        return;
    }
    m_panX = std::clamp(m_panX + across * kPanStep, -kPanMost, kPanMost);
    m_panY = std::clamp(m_panY + down * kPanStep, -kPanMost, kPanMost);
}

void Zoom::Set(int size, int player) {
    m_size = std::clamp(size, 0, kSizeCount - 1);
    m_player = player;
    m_jump = true;
    Said();
}

void Zoom::Follow(int width, int height) {
    if (m_labelTicks > 0) {
        m_labelTicks--;
    }
    int x = 0, y = 0;
    const bool seen = m_size > 0 && Possible() && cps2recomp::TheGame().focus(m_player, x, y);
    const float wanted = seen ? kSizes[m_size] : 1.0f;
    const float wantedX = seen ? x + m_panX : width / 2.0f, wantedY = seen ? y + m_panY : height / 2.0f;
    if (m_jump || !m_settled) {
        // (from nothing, the place is where the size starts from, and the size eases; asked for, both are there)
        m_offX = m_offY = 0;
        m_settled = seen;
        m_wasSeen = seen;
        m_followed = m_player;
        if (m_jump && seen) {
            m_shown = wanted;
            m_jump = false;
        }
    }
    m_shown += (wanted - m_shown) * kSizeEase;
    if (std::fabs(wanted - m_shown) < 0.002f) {
        m_shown = wanted;
    }
    // The camera is on the fighter, but for a gap left where it was when it changed fighter (or went back to the
    // middle), which closes as the size eases
    if (seen != m_wasSeen || m_player != m_followed) {
        m_offX = m_x - wantedX;
        m_offY = m_y - wantedY;
    }
    m_wasSeen = seen;
    m_followed = m_player;
    m_offX *= 1 - kPlaceEase;
    m_offY *= 1 - kPlaceEase;
    m_x = wantedX + m_offX;
    m_y = wantedY + m_offY;
    if (m_shown == 1.0f && !seen) {
        m_settled = false; // (out again: the next zoom starts at its fighter)
    }
}

bool Zoom::View(int width, int height, SDL_FRect &view) const {
    if (m_shown <= 1.0f) {
        return false;
    }
    // (the place looked at in the middle, always: past the picture's edges is black)
    view.w = width / m_shown;
    view.h = height / m_shown;
    view.x = m_x - view.w / 2;
    view.y = m_y - view.h / 2;
    return true;
}

} // namespace port
