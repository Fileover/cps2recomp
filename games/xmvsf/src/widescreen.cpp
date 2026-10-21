// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: widescreen, as the Alpha games and Vampire Savior have it. A fight is shown 512 columns
// wide where the board shows 384 (16:9 for 4:3, the height and the size of everything as they were): more of the stage
// on each side. The fight is played in that picture too: the characters can go to its edges, and the camera stops where
// its edges come to the stage's ends (64 nearer the middle at each end). With the feature off none of that is there
// and the game is the board's. It is off in every preset: it changes how the game plays.
//
// With "Invisible walls" (widescreen_walls) as well, the fight is shown in the wide picture but played as on the
// board: the characters are stopped where the board's picture ends and the camera goes as far as the board's does;
// near a stage's end the view is kept inside the stage, so the board's picture is then off the middle of the wide one.
//
// The two rules the wide picture changes are the game's own code with the margin in it, in place of the game's while
// the fight is played wide: the camera kept between its ends (074488, the end of its step), and a character kept
// inside the picture (010806; the ring out lets one over an end).
#include "battle.h"
#include "alternative.h"
#include "xmvsf.h"

#include <algorithm>

namespace cps2recomp {

namespace {
    using namespace xmvsf;

    constexpr int kSide = Video::kMaxSide;
    constexpr uint32_t kPlanes = 0x01E7EC; // longs by a character's byte +30: the camera's word of the plane it stands in
    constexpr uint32_t kAtEdge = 0xA9;     // a character's: 1 at the picture's right edge, 2 at its left

    bool g_play = false;  // fights are played in the wide picture
    bool g_shown = false; // fights are shown in the wide picture (played in it, or played as on the board)
    int g_side = 0, g_shift = 0, g_cameraBefore = 0;

    int Margin() {
        return g_play ? kSide : 0;
    }

    // 074488: the end of the camera's step (A6 its record): its place across and up kept between its ends; played wide,
    // 64 nearer the middle across; a mode's camera wanted beyond its ends is not kept in at all
    bool KeepCamera(M68K &c) {
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        if (modes::CameraAt().beyondEnds) {
            Return(c);
            return true;
        }
        if (!g_play) {
            return false;
        }
        const int x = static_cast<int16_t>(Ram16(camera + kCameraX));
        const int left = static_cast<int16_t>(Ram16(camera + kCameraLeft)) + kSide, right = static_cast<int16_t>(Ram16(camera + kCameraRight)) - kSide;
        SetRam16(camera + kCameraX, static_cast<uint16_t>(std::clamp(x, left, std::max(left, right))));
        const int y = static_cast<int16_t>(Ram16(camera + kCameraY));
        const int top = static_cast<int16_t>(Ram16(camera + kCameraTop)), bottom = static_cast<int16_t>(Ram16(camera + kCameraTop + 2));
        SetRam16(camera + kCameraY, static_cast<uint16_t>(std::clamp(y, top, std::max(top, bottom))));
        Return(c);
        return true;
    }

    // 010806: a character (A6) kept inside the picture of the plane it stands in: between 0x55 and 0x1AB on from the
    // plane's camera word (the board's picture's edges less 0x15); played wide, 64 further out on each side. The ring
    // out lets one knocked about over an end.
    bool KeepIn(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        if (RingOutLetsOut(player)) {
            SetRam8(player + kAtEdge, 0);
            Return(c);
            return true;
        }
        if (!g_play) {
            return false;
        }
        SetRam8(player + kAtEdge, 0);
        const uint8_t plane = Ram8(player + 0x30);
        if (plane) {
            const uint32_t word = (static_cast<uint32_t>(c.Read(kPlanes + plane, 1)) << 16) | c.Read(kPlanes + plane + 2, 1);
            const int camera = static_cast<int16_t>(Ram16(word & 0xFFFFFF));
            const int x = static_cast<int16_t>(Ram16(player + kX));
            if (x <= camera + 0x55 - kSide) {
                SetRam16(player + kX, static_cast<uint16_t>(camera + 0x55 - kSide));
                SetRam8(player + kAtEdge, 2);
            } else if (x >= camera + 0x1AB + kSide) {
                SetRam16(player + kX, static_cast<uint16_t>(camera + 0x1AB + kSide));
                SetRam8(player + kAtEdge, 1);
            }
        }
        Return(c);
        return true;
    }

    // --- the picture ---

    int CameraLeftEnd() {
        return static_cast<int16_t>(Ram16(CameraRecord() + kCameraLeft));
    }
    int CameraRightEnd() {
        return static_cast<int16_t>(Ram16(CameraRecord() + kCameraRight));
    }
    bool InFight() {
        return Ram8(kPlaying) != 0 && Ram16(kGameStep) == kGameMatch && Ram8(kFight) != 0;
    }
    bool OnStage(int camera) {
        return CameraRightEnd() >= CameraLeftEnd() + 2 * kSide && camera >= CameraLeftEnd() && camera <= CameraRightEnd();
    }

    // An object's place: the panel's stay in the middle, the stage's and the fighters' go with the view
    int MoveObject(uint32_t tile, uint16_t attributes, int) {
        return ThePanel(tile, attributes) ? 0 : g_shift;
    }

    // The wide picture's switches for the frame about to run (the port's, or a spare's): the camera's place now and
    // where it was for the picture being made (a frame behind)
    void SetWide(Machine &machine, int camera, int shown) {
        g_side = g_shift = 0;
        if (g_shown && OnStage(camera) && OnStage(shown)) {
            g_side = kSide;
            // (the view's left edge is 64 left of the board's, and no further left than the camera's own end allows:
            // nearer the end than that, the stage is drawn to the side by what is missing)
            g_shift = shown - std::clamp<int>(shown, CameraLeftEnd() + kSide, CameraRightEnd() - kSide);
        }
        Video::Wide &wide = machine.video.wide;
        wide.side = g_side;
        wide.scroll[0] = wide.scroll[1] = wide.scroll[2] = g_shift;
        wide.object = g_side ? MoveObject : nullptr;
        wide.tile = nullptr;
        wide.fillHoles = g_side != 0;
    }

    struct Kept {
        int side, shift;
    } g_kept;
} // namespace

bool WidePlay() {
    return g_play;
}
int WideMargin() {
    return Margin();
}
int WideSide() {
    return g_side;
}
int WideStage() {
    return g_side + g_shift;
}

void WidescreenBeforeFrame(Machine &machine) {
    const int camera = static_cast<int16_t>(Ram16(CameraRecord() + kCameraX)), shown = g_cameraBefore;
    g_cameraBefore = camera;
    // (fights only; the stage viewer keeps the board's picture. With invisible walls the rules stay the board's)
    g_shown = FeatureOn("widescreen") && InFight() && !modes::StageViewer();
    g_play = g_shown && !FeatureOn("widescreen_walls");
    SetWide(machine, camera, shown);
}

// A spare machine's frame (the modes' other machines) is drawn as wide as the port's picture is, so that the two can
// be put together: from the spare's own camera (where it is now, for want of where it was)
void WideForSpare(Machine &machine) {
    g_kept = {g_side, g_shift};
    const int camera = static_cast<int16_t>(Ram16(CameraRecord() + kCameraX));
    SetWide(machine, camera, camera);
    if (g_side != g_kept.side) {
        Video::Wide &wide = machine.video.wide;
        g_side = wide.side = g_kept.side;
        g_shift = 0;
        wide.scroll[0] = wide.scroll[1] = wide.scroll[2] = 0;
        wide.object = g_side ? MoveObject : nullptr;
    }
}
void WideForPort(Machine &) {
    g_side = g_kept.side;
    g_shift = g_kept.shift;
}

void RegisterWidescreen() {
    TheGame().widestPicture = [] { return Video::kWidth + (FeatureOn("widescreen") ? 2 * kSide : 0); };
    AddHook(HookAt(0x074488), "KeepCamera", KeepCamera);
    AddHook(HookAt(0x010806), "KeepIn", KeepIn);
    TheGame().features.push_back({"widescreen", "Display", "Widescreen",
                                  "Fights are shown and played in 16:9, with more of the stage on each side: the characters can go to "
                                  "the wide picture's edges, so they can stand further apart than on the board. Off unless switched "
                                  "on here, in every preset: it changes how the game plays.",
                                  kFeatureFirstPage});
    TheGame().features.push_back({"widescreen_walls", "Display", "Invisible walls",
                                  "With widescreen: fights are still shown in 16:9, but played exactly as on the board. The characters "
                                  "are stopped where the board's picture ends, by walls that are not seen, and the camera goes as far "
                                  "as the board's; near a stage's end the view stays on the stage, so the play area is then to one "
                                  "side of the picture. Nothing without widescreen.",
                                  0});
}

} // namespace cps2recomp
