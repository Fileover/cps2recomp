// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Street Fighter Alpha 3: widescreen, as the other Alpha games have it. A fight is shown 512 columns wide where the
// board shows 384 (16:9 for 4:3, the height and the size of everything as they were): more of the stage on each side.
// The fight is played in that picture too: the players can go to its edges, and the camera stops where its edges come
// to the stage's ends (64 nearer the middle at each end). With the feature off none of that is there and the game is
// the board's. It is off in every preset: it changes how the game plays.
//
// With "Invisible walls" (widescreen_walls) as well, the fight is shown in the wide picture but played as on the board:
// nothing here touches the rules (the margin is 0), so the players are stopped where the board's picture ends and the
// camera goes as far as the board's does. Only the picture is wider, and near a stage's end the view is kept inside
// the stage, so the board's picture is then off the middle of the wide one.
//
// The rules the wide picture changes are written again here as readable C++ (rewrites: lift.h), each with the margin
// as a line in it: the camera's place across and up (its ends 64 nearer the middle; a mode's wanted place: battle.h; a
// fighter off the stage not followed), and keeping the players inside the picture (64 further out on each side; the
// ring out lets one over an end; a fighter of the tag style off the stage let out).
#include "alternative.h"
#include "battle.h"
#include "sfa3.h"

#include <algorithm>
#include <cstdlib>

namespace cps2recomp {

bool TagIsOff(uint32_t player); // livetag.cpp

namespace {
    using namespace sfa3;

    constexpr int kWideSide = Video::kMaxSide;
    // The camera's record (the stage's own scroll screen, FF8280), and in it: where it is to be (the picture's left
    // edge and bottom, before its shake), its shake, its ends
    constexpr uint32_t kCamera = 0xFF8280;
    constexpr uint32_t kCameraPlaceX = 0x20, kCameraPlaceY = 0x24, kCameraShakeX = 0x50, kCameraShakeY = 0x54;
    constexpr uint32_t kCameraEndTop = 0x40, kCameraEndBottom = 0x42, kCameraEndLeft = 0x44, kCameraEndRight = 0x46;
    constexpr uint32_t kCameraHeld = 0xFF214A; // byte (A5-5EB6): set, the camera stays where it is
    constexpr uint32_t kAtEdge = 0xAC;         // a player's: 1 at the picture's left edge, 2 at its right
    constexpr uint32_t kNotKeptIn = 0x2AF;     // a player's: set, it is not kept inside the picture
    // Routines the rewrites call
    constexpr uint32_t kOuterEdges = 0x018654; // D0: the players' left edge, D1 their right (the camera follows these;
                                               // a player with +263 set is left out, 018700)
    constexpr uint32_t kHeights = 0x01877C;    // D1: the higher player's height (D0 the lower)
    constexpr uint32_t kCameraOf = 0x01BD5C;   // A3: the camera record of the object at A6 (by its +0E)
    constexpr uint16_t kPicture = 0x180;       // the board's picture across

    bool g_play = false;  // fights are played in the wide picture
    bool g_shown = false; // fights are shown in the wide picture (played in it, or played as on the board)
    int g_side = 0, g_shift = 0, g_cameraBefore = 0;

    int Margin() {
        return g_play ? kWideSide : 0;
    }

    // --- the camera ---

    // The players' edges the camera follows (018654), but with a fighter of the tag style off the stage left out: then
    // the other's own edges (as the game's 018750 and 018760 give one player's)
    void OuterEdges(M68K &c) {
        Call(c, kOuterEdges);
        for (int n = 0; n < 2; n++) {
            const uint32_t off = Player(n), on = Player(1 - n);
            if (TagIsOff(off) && !TagIsOff(on)) {
                const uint16_t x = Ram16(on + kX), half = Ram16(on + kHalfWidth);
                SetLow16(c.d[0], static_cast<uint16_t>(x - half));
                SetLow16(c.d[1], static_cast<uint16_t>(x + half));
            }
        }
    }
    void Heights(M68K &c) {
        Call(c, kHeights);
        for (int n = 0; n < 2; n++) {
            const uint32_t off = Player(n), on = Player(1 - n);
            if (TagIsOff(off) && !TagIsOff(on)) {
                SetLow16(c.d[0], Ram16(on + kY));
                SetLow16(c.d[1], Ram16(on + kY));
            }
        }
    }

    // The step's most: what is beyond it is cut to it, in D2 (an unsigned compare, as the game's `bls`)
    uint16_t Step(M68K &c, uint16_t by, uint16_t most) {
        if (by > most) {
            c.d[2] = most;
        } else {
            SetLow16(c.d[2], by);
        }
        return static_cast<uint16_t>(c.d[2]);
    }

    // 017F56: the camera's place across kept, with its shake
    void AcrossDone(M68K &c, uint32_t camera) {
        SetLow16(c.d[0], static_cast<uint16_t>(c.d[0] + Ram16(camera + kCameraShakeX)));
        SetRam16(camera + kCameraPlaceX, static_cast<uint16_t>(c.d[0]));
    }

    // 017E7E. The camera's place across: it follows the players' middle when they are far apart (C0 or more), and
    // otherwise moves only when one of them comes near the picture's edge; at most 5 a frame, and not past its ends (in
    // the wide picture, 64 nearer the middle). Then its shake.
    void CameraAcross(M68K &c) {
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        if (Ram8(kCameraHeld)) {
            SetLow16(c.d[0], Ram16(camera + kCameraPlaceX));
            AcrossDone(c, camera);
            return;
        }
        OuterEdges(c);
        const int margin = Margin();
        if (modes::CameraAt().x >= 0) {
            // (a mode wants the camera at a place of its own: it goes there, as far as its ends let it)
            const int end0 = Ram16(camera + kCameraEndLeft) + margin, end1 = Ram16(camera + kCameraEndRight) - margin;
            SetLow16(c.d[0], static_cast<uint16_t>(std::clamp(modes::CameraAt().x, end0, end1)));
            SetLow16(c.d[2], 0);
            AcrossDone(c, camera);
            return;
        }
        const uint16_t left = static_cast<uint16_t>(c.d[0]), right = static_cast<uint16_t>(c.d[1]);
        const uint16_t place = Ram16(camera + kCameraPlaceX);
        int16_t by = 0;
        int way = 0; // 1 right, -1 left, 0 stays
        if (static_cast<uint16_t>(right - left) >= 0xC0) {
            // (far apart: their middle in the middle)
            by = static_cast<int16_t>(static_cast<uint16_t>(static_cast<uint16_t>(right + left) >> 1) - place - 0xC0);
            way = by < 0 ? -1 : 1;
        } else if ((by = static_cast<int16_t>(right - place - 0x120)) >= 0) {
            way = 1;
        } else if ((by = static_cast<int16_t>(left - place - 0x60)) <= 0) {
            way = -1;
        }
        SetLow16(c.d[2], static_cast<uint16_t>(by));
        if (way == 0) {
            SetLow16(c.d[0], place);
        } else if (way > 0) {
            const uint16_t moved = static_cast<uint16_t>(place + Step(c, static_cast<uint16_t>(by), 5));
            SetRam16(camera + kCameraPlaceX, moved);
            // (the right end: in the wide picture 64 nearer the middle)
            const uint16_t end = static_cast<uint16_t>(Ram16(camera + kCameraEndRight) - margin);
            SetLow16(c.d[0], static_cast<int16_t>(end) <= static_cast<int16_t>(moved) ? end : moved);
        } else {
            const uint16_t moved = static_cast<uint16_t>(place - Step(c, static_cast<uint16_t>(-by), 5));
            SetRam16(camera + kCameraPlaceX, moved);
            const uint16_t end = static_cast<uint16_t>(Ram16(camera + kCameraEndLeft) + margin);
            SetLow16(c.d[0], static_cast<int16_t>(end) >= static_cast<int16_t>(moved) ? end : moved);
        }
        AcrossDone(c, camera);
    }

    // 017F60: the camera's place up kept, with its shake
    void UpDone(M68K &c, uint32_t camera) {
        SetLow16(c.d[0], static_cast<uint16_t>(c.d[0] + Ram16(camera + kCameraShakeY)));
        SetRam16(camera + kCameraPlaceY, static_cast<uint16_t>(c.d[0]));
    }

    // 017EFC. The camera's place up and down: it follows the higher player when that one is more than 96 above its
    // bottom (or below), at most 1 a frame, and not past its ends; then its shake. (A mode can want it at a place of its
    // own.)
    void CameraUp(M68K &c) {
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        if (Ram8(kCameraHeld)) {
            SetLow16(c.d[0], Ram16(camera + kCameraPlaceY));
            UpDone(c, camera);
            return;
        }
        Heights(c);
        if (modes::CameraAt().y >= 0) {
            const CameraWanted &wanted = modes::CameraAt();
            SetLow16(c.d[0], static_cast<uint16_t>(wanted.beyondEnds ? wanted.y :
                                                   std::clamp<int>(wanted.y, Ram16(camera + kCameraEndBottom), Ram16(camera + kCameraEndTop))));
            SetLow16(c.d[2], 0);
            UpDone(c, camera);
            return;
        }
        const uint16_t high = static_cast<uint16_t>(c.d[1]);
        const uint16_t place = Ram16(camera + kCameraPlaceY);
        const int16_t by = static_cast<int16_t>(high - place - 0x60);
        SetLow16(c.d[2], static_cast<uint16_t>(by));
        if (by > 0) {
            const uint16_t moved = static_cast<uint16_t>(place + Step(c, static_cast<uint16_t>(by), 1));
            SetRam16(camera + kCameraPlaceY, moved);
            const uint16_t end = Ram16(camera + kCameraEndTop);
            SetLow16(c.d[0], static_cast<int16_t>(end) <= static_cast<int16_t>(moved) ? end : moved);
        } else if (by < 0) {
            const uint16_t moved = static_cast<uint16_t>(place - Step(c, static_cast<uint16_t>(-by), 1));
            SetRam16(camera + kCameraPlaceY, moved);
            const uint16_t end = Ram16(camera + kCameraEndBottom);
            SetLow16(c.d[0], static_cast<int16_t>(end) >= static_cast<int16_t>(moved) ? end : moved);
        } else {
            SetLow16(c.d[0], place);
        }
        UpDone(c, camera);
    }

    // --- the players inside the picture ---

    // 02AE32: a player held at an edge while knocked up into the air is marked for it (+270)
    void HeldAtEdge(uint32_t player) {
        if (Ram8(player + kAtEdge) && (Ram8(player + kInAir) & 0x80) && !Ram8(player + 0x270)) {
            SetRam8(player + 0x270, 1);
        }
    }

    // 02ADA2. The player at A6 kept inside the picture (in the wide picture, its edges); a player pressed against an edge
    // while reeling passes the push back to the other.
    void KeepIn(M68K &c) {
        const uint32_t player = c.a[6] & 0xFFFFFF;
        SetRam8(player + kAtEdge, 0);
        if (Ram8(player + kNotKeptIn)) {
            return;
        }
        c.a[4] = static_cast<uint32_t>(static_cast<int16_t>(Ram16(player + kOpponent)));
        const uint32_t other = c.a[4] & 0xFFFFFF;
        uint16_t apart = static_cast<uint16_t>(Ram16(other + kX) - Ram16(player + kX));
        if (apart & 0x8000) {
            apart = static_cast<uint16_t>(0u - apart);
        }
        SetLow16(c.d[1], apart);
        Call(c, kCameraOf);
        // (the ring out: a player knocked about at an end of the stage goes over it; the tag style's fighter off the
        // stage goes out of the picture)
        if (RingOutLetsOut(player) || TagIsOff(player)) {
            return;
        }
        const uint32_t camera = c.a[3] & 0xFFFFFF;
        const uint16_t half = Ram16(player + kHalfWidth);
        SetLow16(c.d[2], half);
        const uint16_t shown = static_cast<uint16_t>(Ram16(camera + kX) - Margin());
        const uint16_t x = Ram16(player + kX);
        uint16_t over = static_cast<uint16_t>(x - half - shown);
        if (static_cast<uint16_t>(x - half) <= shown) {
            // (left: over is how far past, nothing or below)
            SetRam16(player + kX, static_cast<uint16_t>(x - over));
            SetRam8(player + kAtEdge, 1);
        } else {
            SetLow16(c.d[2], static_cast<uint16_t>(2 * half));
            over = static_cast<uint16_t>(over + 2 * half - (kPicture + 2 * Margin()));
            SetLow16(c.d[0], over);
            if (over & 0x8000) {
                return;
            }
            SetRam16(player + kX, static_cast<uint16_t>(x - over));
            SetRam8(player + kAtEdge, 2);
        }
        SetLow16(c.d[0], over);
        if (Ram16(player + kState) == 0x0202 && !Ram8(player + 0x239)) {
            bool pass = true;
            if (Ram8(player + kInAir)) {
                if (!Ram8(player + 0x23D)) {
                    pass = false;
                } else {
                    over = static_cast<uint16_t>(over - Ram16(player + kSpeedX));
                    SetLow16(c.d[0], over);
                }
            }
            if (pass && !Ram8(player + 0xAD) && (Ram8(other + 0x2D9) || (!Ram8(other + kInAir) && !Ram8(other + 0x274)))) {
                SetRam16(other + kX, static_cast<uint16_t>(Ram16(other + kX) - over));
            }
        }
        HeldAtEdge(player);
    }

    // 02AE4C. Both players (one holding the other: a throw) kept inside the picture together
    void KeepBothIn(M68K &c) {
        const uint32_t player = c.a[6];
        if (Ram8((player & 0xFFFFFF) + kNotKeptIn)) {
            return;
        }
        c.a[4] = c.a[0] = static_cast<uint32_t>(static_cast<int16_t>(Ram16((player & 0xFFFFFF) + kOpponent)));
        c.a[1] = player;
        const uint32_t other = c.a[4] & 0xFFFFFF;
        SetRam8(other + kAtEdge, 0);
        SetLow16(c.d[1], Ram16(other + kX));
        if (!(Ram16(other + kX) < Ram16((player & 0xFFFFFF) + kX))) {
            c.a[0] = player;
            c.a[1] = c.a[4];
        }
        Call(c, kCameraOf);
        const uint32_t leftOne = c.a[0] & 0xFFFFFF, rightOne = c.a[1] & 0xFFFFFF;
        const uint16_t shown = static_cast<uint16_t>(Ram16((c.a[3] & 0xFFFFFF) + kX) - Margin());
        uint16_t half = Ram16(leftOne + kHalfWidth);
        SetLow16(c.d[2], half);
        uint16_t over = static_cast<uint16_t>(Ram16(leftOne + kX) - half - shown);
        SetLow16(c.d[0], over);
        if (over & 0x8000) {
            SetRam16((player & 0xFFFFFF) + kX, static_cast<uint16_t>(Ram16((player & 0xFFFFFF) + kX) - over));
            SetRam16(other + kX, static_cast<uint16_t>(Ram16(other + kX) - over));
            SetRam8(other + kAtEdge, 1);
            return;
        }
        half = Ram16(rightOne + kHalfWidth);
        SetLow16(c.d[2], half);
        over = static_cast<uint16_t>(Ram16(rightOne + kX) - shown + half - (kPicture + 2 * Margin()));
        SetLow16(c.d[0], over);
        if (over & 0x8000) {
            return;
        }
        SetRam16((player & 0xFFFFFF) + kX, static_cast<uint16_t>(Ram16((player & 0xFFFFFF) + kX) - over));
        SetRam16(other + kX, static_cast<uint16_t>(Ram16(other + kX) - over));
        SetRam8(other + kAtEdge, 2);
    }

    // --- the picture ---

    int CameraLeftEnd() {
        return Ram16(kCamera + kCameraEndLeft);
    }
    int CameraRightEnd() {
        return Ram16(kCamera + kCameraEndRight);
    }
    bool InFight() {
        return Ram16(kMode) == 2 && Ram16(kGameStep) == kGameMatch && Ram16(kMatchStep) == kMatchRound;
    }
    bool OnStage(int camera) {
        return CameraRightEnd() >= CameraLeftEnd() + 2 * kWideSide && camera >= CameraLeftEnd() && camera <= CameraRightEnd();
    }

    // The two other scroll screens' places across (their records' +10) with the camera at each of its ends, by stage
    // (kStageNames' order; measured with a training session walked to both ends of each stage with invisible walls: the
    // right end reached, the left worked out from the place at 376, as the planes move in proportion to the camera). A
    // plane is painted for what the board shows of it at those places and the 384 columns from them; in the wide
    // picture, near a stage's end, more of it would show than that. So each is drawn moved by as much as its wide view
    // would reach past its painting (a picture-only change: nothing the game works out is touched), as Alpha 2's are.
    // Sagat's planes (the Buddha's too) and Dhalsim's back plane are left as they are ({0, 0}): the back ones are fixed
    // maps of 512 pixels that barely move (38 to 90, 54 to 90), and the columns of these three beyond the board's view
    // were never drawn, so a moved view shows holes in the middle of them; the holes at their ends are filled instead
    // (Video::wide.fillHoles).
    struct PlaneEnds {
        int16_t back[2], far[2]; // the 8-pixel and the 32-pixel scroll screens
    };
    constexpr PlaneEnds kPlaneEnds[kStages] = {
        {{24, 103}, {330, 566}},  {{309, 587}, {289, 606}}, {{25, 102}, {371, 524}},  {{336, 561}, {318, 577}},
        {{364, 531}, {299, 596}}, {{256, 640}, {280, 616}}, {{256, 640}, {256, 640}}, {{280, 616}, {304, 592}},
        {{298, 599}, {383, 512}}, {{273, 622}, {200, 375}}, {{265, 630}, {265, 630}}, {{0, 0}, {0, 0}},
        {{296, 601}, {382, 513}}, {{267, 628}, {312, 583}}, {{486, 652}, {280, 616}}, {{0, 0}, {339, 556}},
        {{289, 605}, {289, 605}}, {{270, 625}, {315, 581}}, {{265, 630}, {265, 630}}, {{16, 112}, {352, 544}},
        {{263, 632}, {288, 611}}, {{401, 494}, {355, 540}}, {{314, 582}, {333, 563}}, {{263, 632}, {286, 609}},
        {{283, 612}, {256, 384}}, {{267, 628}, {312, 583}}};
    constexpr uint32_t kPlanes[2] = {0xFF8200, 0xFF8300};
    int g_planeBefore[2] = {0, 0};

    // How far to draw a plane moved so that its wide view stays inside its painting
    int PlaneShift(const int16_t ends[2], int place) {
        const int lo = ends[0], hi = ends[1];
        if (hi <= lo) {
            return 0; // (going the other way, or round: as it is)
        }
        const int from = place - kWideSide, to = place + Video::kWidth + kWideSide; // (the wide view of it)
        const int painted = hi - lo + Video::kWidth, wide = Video::kWidth + 2 * kWideSide;
        if (painted < wide) {
            // (painted narrower than the wide picture: the shortfall split between its two sides, wherever the camera
            // is; such a plane is far off and moves little)
            return from - (lo - (wide - painted) / 2);
        }
        if (from < lo) {
            return from - lo;
        }
        if (to > hi + Video::kWidth) {
            return to - (hi + Video::kWidth);
        }
        return 0;
    }

    // An object's place: the panel's stay in the middle, the stage's and the fighters' go with the view
    int MoveObject(uint32_t tile, uint16_t attributes, int) {
        return ThePanel(tile, attributes) ? 0 : g_shift;
    }

    // The wide picture's switches for the frame about to run (the port's, or a spare's): the camera's place now and
    // where it was for the picture being made (a frame behind)
    void SetWide(Machine &machine, int camera, int shown, const int planesShown[2]) {
        g_side = g_shift = 0;
        if (g_shown && OnStage(camera) && OnStage(shown)) {
            g_side = kWideSide;
            // (the view's left edge is 64 left of the camera, and no further left than the camera's own end: nearer the
            // end than that, the stage is drawn to the side by what is missing. Played in the wide picture the camera
            // never goes there; with invisible walls it does, and the board's picture is then to one side)
            g_shift = shown - std::clamp<int>(shown, CameraLeftEnd() + kWideSide, CameraRightEnd() - kWideSide);
        }
        Video::Wide &wide = machine.video.wide;
        wide.side = g_side;
        // (the panel is objects: every scroll screen is the stage's and goes with the view)
        wide.scroll[0] = wide.scroll[1] = wide.scroll[2] = g_shift;
        // (the planes behind, kept inside their painting; each a frame behind, as the picture is)
        const int stage = StageOfNumber(Ram16(kStage) / 2) - 1;
        for (int n = 0; n < 2; n++) {
            if (g_side && stage >= 0) {
                const PlaneEnds &ends = kPlaneEnds[stage];
                // (from where the view of it already is: near a stage's end, with invisible walls, the whole view is
                // drawn moved by g_shift)
                wide.scroll[n == 0 ? 0 : 2] += PlaneShift(n == 0 ? ends.back : ends.far, planesShown[n] - g_shift);
            }
        }
        wide.object = g_side ? MoveObject : nullptr;
        wide.tile = nullptr;
        // (what is left unpainted, mirrored from the line's own picture; SFA3_NO_FILL for tests, to see what is left)
        wide.fillHoles = g_side != 0 && !getenv("SFA3_NO_FILL");
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
    const int camera = static_cast<int16_t>(Ram16(kCameraX)), shown = g_cameraBefore;
    g_cameraBefore = camera;
    int planesShown[2];
    for (int n = 0; n < 2; n++) {
        planesShown[n] = g_planeBefore[n];
        g_planeBefore[n] = static_cast<int16_t>(Ram16(kPlanes[n] + kX));
    }
    // (fights only, the alternative modes' too; the stage viewer keeps the board's. The modes that lay one machine's
    // picture over another's draw the other as wide: WideForSpare)
    g_shown = FeatureOn("widescreen") && InFight() && !modes::StageViewer();
    g_play = g_shown && !FeatureOn("widescreen_walls");
    SetWide(machine, camera, shown, planesShown);
}

// A spare machine's frame (the modes' other machines) is drawn as wide as the port's picture is, so that the two can be
// put together: from the spare's own camera (where it is now, for want of where it was)
void WideForSpare(Machine &machine) {
    g_kept = {g_side, g_shift};
    const int camera = static_cast<int16_t>(Ram16(kCameraX));
    const int planes[2] = {static_cast<int16_t>(Ram16(kPlanes[0] + kX)), static_cast<int16_t>(Ram16(kPlanes[1] + kX))};
    SetWide(machine, camera, camera, planes);
    // (exactly as wide as the port's picture, even where the spare is not on its stage yet)
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
    using namespace sfa3;
    // Where a player's fighter stands in the picture, for the window's zoom (fight/alternative.h, modes::Focus): the
    // middle of its feet, the wide picture's columns counted
    void FocusPlace(int player, int &x, int &y) {
        x = static_cast<int16_t>(Ram16(Player(player) + kX)) - static_cast<int16_t>(Ram16(kCameraX)) + WideStage();
        y = 239 - (static_cast<int16_t>(Ram16(Player(player) + kY)) - static_cast<int16_t>(Ram16(kCameraY)));
    }
} // namespace

void RegisterWidescreen() {
    modes::Focus(FocusPlace, 44, 112);
    TheGame().widestPicture = [] { return Video::kWidth + (FeatureOn("widescreen") ? 2 * kWideSide : 0); };
    AddRewrite(0x017E7E, "CameraAcross", CameraAcross, kFreeFlags, 195);
    AddRewrite(0x017EFC, "CameraUp", CameraUp, kFreeFlags, 178);
    AddRewrite(0x02ADA2, "KeepIn", KeepIn, kFreeFlags, 235);
    AddRewrite(0x02AE4C, "KeepBothIn", KeepBothIn, kFreeFlags, 240);
    // (not in the Enhanced preset: it shows more of the stage than the board does, and a player who wants the game as it
    // is played in competition must not get that without asking)
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
