// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The camera's zoom: the picture shown larger about one player's fighter, following it, at the sizes the model
// viewer draws a character at from its own to twice it (twice: the top of a tall one goes off the screen). A
// press for a player steps its size up, from 2x back to none; a press for the other player looks at that one
// instead, at the same size. The size and the place looked at go smoothly from one to the next, and what was
// chosen is said on the picture for a few seconds. Only where the game's sources say where a fighter is
// (runtime/game.h, Game::focus), and only while there is one: elsewhere the picture goes back to its own size.
// It is the window's alone: the game, its save states and its screenshots know nothing of it.
#pragma once

#include "present.h"

namespace port {

class Zoom {
public:
    static constexpr float kSizes[] = {1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
    static constexpr int kSizeCount = static_cast<int>(sizeof(kSizes) / sizeof(kSizes[0]));
    // Whether the game can be zoomed at all
    static bool Possible();

    // A press for player `player` (0 or 1)
    void Press(int player);
    // Straight to a size (0: none) about a player, with no easing (--zoom)
    void Set(int size, int player);
    // Each picture shown, `width` x `height`: the size and the place looked at a step nearer what is wanted
    void Follow(int width, int height);
    // The part of the picture shown, in its pixels; false when it is all of it
    bool View(int width, int height, SDL_FRect &view) const;
    // What was last chosen ("ZOOM 1.5x  PLAYER 1"), for a few seconds after it was ("": nothing to say)
    const char *Label() const {
        return m_labelTicks > 0 ? m_label : "";
    }

private:
    void Said();
    int m_size = 0, m_player = 0;
    float m_shown = 1.0f, m_x = 0, m_y = 0; // as shown now
    bool m_settled = false;                 // (the place is known: eased to from here on)
    bool m_jump = false;                    // (the next Follow goes straight there)
    int m_labelTicks = 0;
    char m_label[40] = {};
};

} // namespace port
