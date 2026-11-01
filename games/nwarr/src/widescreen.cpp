// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Night Warriors: widescreen, as Pocket Fighter, the Alpha games and Vampire Savior have it. A fight is shown 512
// columns wide where the board shows 384 (16:9 for 4:3, the height and the size of everything as they were): more of
// the stage on each side. The fight is played in that picture too: the players can go to its edges, and the camera
// stops where its edges come to the stage's ends (64 nearer the middle at each end). The players' own ends in the
// stage stay the board's (the camera 64 further in, the picture's edge 64 further out). With the feature off none of
// that is there and the game is the board's. It is off in every preset: it changes how the game plays.
//
// With "Invisible walls" (widescreen_walls) as well, the fight is shown in the wide picture but played as on the
// board: nothing here touches the rules (the margin is 0), and near a stage's end the view is kept inside the stage.
//
// The stage's art was found drawn beyond the board's picture (CPS2RECOMP_WIDE=64 over fights on several stages), black
// only past the stage's ends; the panel's objects stay where the board puts them, which is the middle of the wide
// picture. Where a far layer is drawn narrower, the holes are filled (Video::Wide::fillHoles).
//
// The rules the wide picture changes, each with the margin in it, are the game's own code done here when the margin is
// not 0 (with it 0 the game's code runs): the camera's step across (its last part, 0FF780 to the right and 0FF7BA to
// the left, keeps it inside its ends: words at A5+D98 and A5+D9A, 0x100 and 0x280) and keeping a player inside the
// picture (006BAC, which also lets the ring out's player over an end: battle.h).
#include "battle.h"
#include "alternative.h"
#include "nwarr.h"
#include "state.h"

#include <algorithm>

namespace cps2recomp {

namespace {
    using namespace nwarr;

    constexpr int kSide = Video::kMaxSide;
    constexpr uint32_t kCameraLeftEnd = At(0xFF8D98), kCameraRightEnd = At(0xFF8D9A);
    // The camera object's (FF8F08): its place across (kCameraX), the step it last took, and which way (0 left, 4 right)
    constexpr uint32_t kCameraObject = At(0xFF8F08), kCameraStep = 0x34, kCameraWay = 0x2C;
    // A player's: half its width, a byte 1 at the picture's left edge and 2 at its right, the other player's record (a
    // word), and bytes the keep-in tests (006BAC)
    constexpr uint32_t kHalfWidth = 0x1C0, kAtEdge = family::kAtEdge, kOpponent = 0x200, kNotKept = 0x1E6, kHeld = 0x4C;
    constexpr uint32_t kBothPushed = At(0xFF8308); // (A5+308: set, a player held at an edge pushes the other with it)

    bool g_play = false;  // fights are played in the wide picture
    bool g_shown = false; // fights are shown in the wide picture (played in it, or played as on the board)
    int g_side = 0, g_shift = 0, g_cameraBefore = 0;

    int Margin() {
        return g_play ? kSide : 0;
    }

    // --- the camera ---

    // 0FF780: the camera (A6) steps D1 to the right, no further than its right end (here 64 nearer the middle)
    bool CameraRight(M68K &c) {
        if (!Margin()) {
            return false;
        }
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        const uint16_t was = Ram16(camera + 0x10), step = static_cast<uint16_t>(c.d[1]);
        const uint16_t end = static_cast<uint16_t>(Ram16(kCameraRightEnd) - Margin());
        const uint16_t now = static_cast<uint16_t>(was + step);
        if (end < now) {
            SetRam16(camera + 0x10, end);
            SetRam16(camera + kCameraStep, static_cast<uint16_t>(end - was));
        } else {
            SetRam16(camera + 0x10, now);
            SetRam16(camera + kCameraStep, step);
        }
        SetRam8(camera + kCameraWay, 4);
        Return(c);
        return true;
    }
    // 0FF7BA: the camera steps D0 (below nothing) to the left, no further than its left end (64 nearer the middle)
    bool CameraLeft(M68K &c) {
        if (!Margin()) {
            return false;
        }
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        const uint16_t was = Ram16(camera + 0x10), step = static_cast<uint16_t>(c.d[0]);
        const uint16_t end = static_cast<uint16_t>(Ram16(kCameraLeftEnd) + Margin());
        const uint16_t now = static_cast<uint16_t>(was + step);
        if (end >= now) {
            SetRam16(camera + 0x10, end);
            SetRam16(camera + kCameraStep, static_cast<uint16_t>(end - was));
        } else {
            SetRam16(camera + 0x10, now);
            SetRam16(camera + kCameraStep, step);
        }
        SetRam8(camera + kCameraWay, 0);
        Return(c);
        return true;
    }

    // --- the players kept in ---

    // A player held at an edge pushes the other along with it: always while A5+308 is set, else when both are held
    // (+4C) (006BDA, 006C20)
    void PushOther(uint32_t player, int16_t by) {
        const uint32_t other = 0xFF0000 | Ram16(player + kOpponent);
        if (Ram8(kBothPushed) || (Ram8(player + kHeld) && Ram8(other + kHeld))) {
            SetRam16(other + kX, static_cast<uint16_t>(Ram16(other + kX) + by));
        }
    }

    // 006BAC: the player (A6) kept inside the picture, half its width in from each edge (the camera's X and 0x180 on);
    // its edge kept at +6F. Played wide, the edges are the wide picture's.
    bool KeepIn(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        if (RingOutLetsOut(player)) {
            Return(c);
            return true;
        }
        if (!Margin()) {
            return false;
        }
        if (Ram8(player + kNotKept)) {
            Return(c);
            return true;
        }
        SetRam8(player + kAtEdge, 0);
        const int16_t half = static_cast<int16_t>(Ram16(player + kHalfWidth)), camera = static_cast<int16_t>(Ram16(kCameraX));
        const int16_t left = static_cast<int16_t>(camera + half - Margin());
        if (left >= static_cast<int16_t>(Ram16(player + kX))) {
            SetRam8(player + kAtEdge, 1);
            const int16_t by = static_cast<int16_t>(left - static_cast<int16_t>(Ram16(player + kX)));
            SetRam16(player + kX, static_cast<uint16_t>(left));
            SetRam16(player + kX + 2, 0);
            PushOther(player, by);
        }
        const int16_t right = static_cast<int16_t>(camera + 0x180 - half + Margin());
        if (right <= static_cast<int16_t>(Ram16(player + kX))) {
            SetRam8(player + kAtEdge, 2);
            const int16_t by = static_cast<int16_t>(right - static_cast<int16_t>(Ram16(player + kX)));
            SetRam16(player + kX, static_cast<uint16_t>(right));
            SetRam16(player + kX + 2, 0);
            PushOther(player, by);
        }
        Return(c);
        return true;
    }

    // --- the picture ---

    int CameraLeftEnd() {
        return Ram16(kCameraLeftEnd);
    }
    int CameraRightEnd() {
        return Ram16(kCameraRightEnd);
    }
    bool InFight() {
        return Ram8(kPlayersIn) != 0 && Ram16(kMode) == kGameRunning && Ram16(kStep) == kGameMatch;
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
            // (the view's left edge is 64 left of the camera, and no further left than the camera's own end; with
            // invisible walls the camera goes nearer, and the board's picture is then to one side)
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

// The camera's place a frame ago, which the picture is drawn from, in a save state: a frame put back (a save state,
// online play's rollback) is drawn, and its players kept in, as it was the first time
void WidescreenState(Archive &ar) {
    ar.Pod(g_cameraBefore);
}

void WidescreenBeforeFrame(Machine &machine) {
    const int camera = static_cast<int16_t>(Ram16(kCameraX)), shown = g_cameraBefore;
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
    const int camera = static_cast<int16_t>(Ram16(kCameraX));
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

namespace {
    // Where a player's fighter stands in the picture, for the window's zoom (fight/alternative.h, modes::Focus): the
    // middle of its feet, the wide picture's columns counted (the floor at line 196 with the camera at its lowest)
    void FocusPlace(int player, int &x, int &y) {
        x = static_cast<int16_t>(Ram16(Player(player) + kX)) - static_cast<int16_t>(Ram16(kCameraX)) + WideStage();
        y = 196 - (static_cast<int16_t>(Ram16(Player(player) + kY)) - static_cast<int16_t>(kGround) - static_cast<int16_t>(Ram16(kCameraY)));
    }
} // namespace

void RegisterWidescreen() {
    modes::Focus(FocusPlace, 44, 128);
    TheGame().widestPicture = [] { return Video::kWidth + (FeatureOn("widescreen") ? 2 * kSide : 0); };
    AddHook(HookAt(At(0x0FF780)), "CameraRight", CameraRight);
    AddHook(HookAt(At(0x0FF7BA)), "CameraLeft", CameraLeft);
    AddHook(HookAt(At(0x006BAC)), "KeepIn", KeepIn);
    // (not in the Enhanced preset: it shows more of the stage than the board does, and a player who wants the game as
    // it is played in competition must not get that without asking)
    TheGame().features.push_back({"widescreen", "Display", "Widescreen",
                                  "Fights are shown and played in 16:9, with more of the stage on each side: the players can go to "
                                  "the wide picture's edges, so they can stand further apart than on the board. Off unless switched "
                                  "on here, in every preset: it changes how the game plays.",
                                  kFeatureFirstPage});
    TheGame().features.push_back({"widescreen_walls", "Display", "Invisible walls",
                                  "With widescreen: fights are still shown in 16:9, but played exactly as on the board. The players "
                                  "are stopped where the board's picture ends, by walls that are not seen, and the camera goes as far "
                                  "as the board's; near a stage's end the view stays on the stage, so the play area is then to one "
                                  "side of the picture. Nothing without widescreen.",
                                  0});
}

} // namespace cps2recomp
