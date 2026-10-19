// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Cadillacs and Dinosaurs: the stage viewer (the toolkit's beat/beat.h). A stage of the viewer is a part of an
// episode that a game can be continued from: the viewer begins a game there, unseen, and then has the camera.
//
// The camera is a record at FF873C (kCamera): its place across at +08 and up and down at +0C. Each episode is a list
// of the camera's segments (16 bytes each, from the table at 021B2A by episode): where each begins, what the camera
// does in it (+0B), and which part of the episode it is in (+0A, kept at +4A when it is the camera's: FF8786). A part
// that can be continued from has a place to start at in the table at 021580 (by episode, ten bytes a part); the
// camera's set-up (0206EA) starts every game at the first segment, and for the viewer the part's first segment is
// loaded after it (02099A) so that the set-up goes on from there. Each frame the camera routine works out its place
// and then draws the scroll screens' new columns and rows from 02126E on: while viewing, the place is the viewer's.
// The way each part's camera goes was found by playing it to its end; the last part (a room the game sets up itself)
// keeps the game's camera. Every segment's routine is passed by while viewing (CameraMoves).
#include "beat.h"
#include "dino.h"

#include <iterator>

namespace cps2recomp {

namespace {
    using namespace dino;

    constexpr uint32_t kCamera = kVars + 0x73C, kCameraX = kCamera + 0x08, kCameraY = kCamera + 0x0C;
    constexpr uint32_t kSegments = 0x021B2A;
    constexpr uint32_t kSegmentSize = 0x10, kSegmentPart = 0x0A;

    // A part's way: the camera's places across and up and down, in order (as the game has them when the part is
    // played to its end: found by doing so; where the game jumps, a slope)
    struct Place {
        int16_t x, y;
    };
    struct Stage {
        const char *name;
        uint8_t episode, part;
        Place way[8];
    };
    constexpr Place kEnd = {-1, -1};
    constexpr Stage kViewed[] = {
        {"Episode 1: Eastcoast 2513 (1)", 0, 0, {{0, 0x100}, {0x40A, 0x100}, {0x45F, 0xD5}, {0x570, 0xD5}, kEnd}},
        {"Episode 1: Eastcoast 2513 (2)", 0, 1, {{0, 0}, {0x210, 0}, kEnd}},
        {"Episode 1: Eastcoast 2513 (3)", 0, 2, {{0, 0x300}, {0, 0x100}, {0x1D0, 0x100}, kEnd}},
        {"Episode 2: The Swamp Forest (1)", 1, 0, {{0, 0x40}, {0xA00, 0x40}, {0xA30, 0x10}, {0xB70, 0x10}, kEnd}},
        {"Episode 2: The Swamp Forest (2)", 1, 2, {{0, 0}, {0x260, 0}, kEnd}},
        {"Episode 3: Waste Land", 2, 0, {{0, 0}, {0x4000, 0}, kEnd}},
        {"Episode 4: Jack's Garage (1)", 3, 0, {{0, 0}, {0x270, 0}, kEnd}},
        {"Episode 4: Jack's Garage (2)", 3, 1, {{0, 0}, {0x870, 0}, kEnd}},
        {"Episode 4: Jack's Garage (3)", 3, 2, {{0, 0}, {0x470, 0}, kEnd}},
        {"Episode 5: Village of Flame", 4, 0, {{0, 0x200}, {0xD51, 0x200}, {0xDB2, 0x19F}, {0xEF0, 0x19F}, {0xF10, 0x17F}, {0x11C0, 0x17F}, kEnd}},
        {"Episode 6: Jungle & Mine (1)", 5, 0, {{0, 0}, {0x770, 0}, kEnd}},
        {"Episode 6: Jungle & Mine (2)", 5, 1,
         {{0, 0x100}, {0x931, 0x100}, {0x951, 0xE0}, {0xCA4, 0xE0}, {0xD1D, 0xFF}, {0xE01, 0xFF}, {0xE20, 0x11E}, {0x1000, 0x11E}}},
        {"Episode 7: The Basement (1)", 6, 0, {{0, 0}, {0x9D0, 0}, kEnd}},
        {"Episode 7: The Basement (2)", 6, 1, {{0, 0x3700}, {0, 0x10}, kEnd}},
        {"Episode 7: The Basement (3)", 6, 2, {{0, 0}, {0x6F0, 0}, kEnd}},
        {"Episode 7: The Basement (4)", 6, 3, {{0, 0}, {0x4F0, 0}, kEnd}},
        {"Episode 8: Deep Deep Down (1)", 7, 0, {{0, 0}, {0x570, 0}, kEnd}},
        {"Episode 8: Deep Deep Down (2)", 7, 1, {{0, 0x300}, {0xBC0, 0x300}, kEnd}},
        {"Episode 8: Deep Deep Down (3)", 7, 2, {{0, 0x200}, {0x104, 0x200}, {0x2FD, 0x181}, {0x404, 0x181}, {0x5FD, 0x102}, {0xCB0, 0x102}, kEnd}},
        {"Episode 8: Deep Deep Down (4)", 7, 3, {kEnd}}, // (one room, which its own segment sets up: the game's camera)
    };
    constexpr int kMiddle = 0x80; // (the camera's place is the picture's left edge and 0x40: its middle is 0x80 on)

    int32_t g_x = -1, g_y = -1; // where the viewer has the camera (-1: not viewing)
    int32_t g_playing = 0; // frames the game has been played at the wanted part

    uint16_t Word(uint32_t address) {
        const uint8_t *p = Program(address);
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }

    // 02075E: the camera's set-up has loaded the episode's first segment; for a part after the first, its own first
    // segment instead (the set-up then takes the part's place to start at)
    bool CameraSetUp(M68K &c) {
        if (!beat::StageViewer()) {
            return false;
        }
        const Stage &stage = kViewed[beat::ViewedStage()];
        if (stage.part == 0) {
            return false;
        }
        const uint32_t list = kSegments + Word(kSegments + stage.episode * 2u);
        for (uint32_t n = 0; n < 64 && Word(list + n * kSegmentSize) != 0xFFFF; n++) {
            if (Program(list + n * kSegmentSize)[kSegmentPart] == stage.part) {
                SetRam8(c.a[6] + 0x6C, static_cast<uint8_t>(n));
                Call(c, 0x02099A);
                break;
            }
        }
        return false;
    }

    // 02091C: the camera's routine for its segment's way of moving (+05, from the table at 02092E) about to run. Some
    // hold the camera where it is until the wave of enemies there is beaten (mode 04, 021086), or for good (06, the
    // episode's end): while viewing none runs, and the camera goes straight on to be drawn where the viewer has it.
    bool CameraMoves(M68K &c) {
        if (g_x < 0 || !beat::StageViewer()) {
            return false;
        }
        c.pc = 0x02126E;
        return true;
    }

    // 02126E: the camera's place worked out, its new columns and rows to draw: while viewing, the viewer's place
    bool CameraPlaced(M68K &c) {
        if (g_x >= 0 && beat::StageViewer()) {
            SetRam16(c.a[6] + 0x08, static_cast<uint16_t>(g_x));
            if (g_y >= 0) {
                SetRam16(c.a[6] + 0x0C, static_cast<uint16_t>(g_y));
            }
        }
        return false;
    }

    // The enemies (and the other things the game moves) stand still while viewing: their two pools' routines
    // (023620: the 24 records of 0xC0 bytes after the players', FFB6F4; 0233E2: those from FF8874) return at once
    bool StandStill(M68K &c) {
        if (!beat::StageViewer() || beat::Starting()) {
            return false;
        }
        Return(c);
        return true;
    }

    bool ToStage(Machine &machine, int stage, int32_t &frames) {
        const Stage &want = kViewed[stage];
        if (Ram8(kStep) == 0) {
            g_playing = 0;
            // (a credit in once the title is up, Start, and the first character offered)
            if (Ram8(kShow) >= kShowTitle && Ram8(kChute + kChuteCredits) == 0 && frames < 2000 && !(frames & 0x100)) {
                SetRam8(kChute + kChuteCredits, 1);
            }
            machine.players[0] = (frames & 4) ? ((frames & 8) ? kStart : kPunch1) : 0;
            return false;
        }
        if (Ram8(kStep) != 0x0C || Ram8(kStage) != want.episode) {
            g_playing = 0;
            return false;
        }
        return ++g_playing > 120;
    }

    void View(Machine &, int, int x, int y) {
        g_x = x;
        g_y = y;
        // (the players out of harm, and in the middle of the picture, where the game's camera looks for them; and the
        // clock still)
        for (int n = 0; n < kPlayers; n++) {
            const uint32_t p = Player(n);
            if (Ram8(p + kInPlay) == 1) {
                SetRam8(p + kHealth, kHealthFull);
                if (x >= 0) {
                    SetRam16(p + kX, static_cast<uint16_t>(x + kMiddle));
                }
            }
        }
        SetRam8(kClockFrames, 0x32);
    }
} // namespace

// The episode a game the viewer begins starts at (-1: not the viewer's)
int ViewerEpisode() {
    return beat::StageViewer() ? kViewed[beat::ViewedStage()].episode : -1;
}

void ViewerFrame(Machine &) {
    if (!beat::StageViewer() || beat::Starting()) {
        g_x = g_y = -1;
    }
}

void DescribeViewer(beat::Description &d) {
    for (const Stage &stage : kViewed) {
        d.stages.push_back(stage.name);
    }
    d.toStage = ToStage;
    d.stagePath = [](int stage, std::vector<beat::Description::Place> &path) {
        for (const Place &place : kViewed[stage].way) {
            if (place.x < 0) {
                break;
            }
            path.push_back({place.x, place.y});
        }
    };
    d.stageCamera = [](int &x, int &y) {
        x = Ram16(kCameraX);
        y = Ram16(kCameraY);
    };
    d.view = View;
    d.viewLayers = 0xD; // (not the first scroll screen: the panel)
    AddHook(0x02075E, "CameraSetUp", CameraSetUp);
    AddHook(0x02091C, "CameraMoves", CameraMoves);
    AddHook(0x02126E, "CameraPlaced", CameraPlaced);
    AddHook(0x023620, "StandStill", StandStill);
    AddHook(0x0233E2, "StandStillToo", StandStill);
}

} // namespace cps2recomp
