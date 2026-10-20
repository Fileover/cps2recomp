// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// X-Men vs. Street Fighter: the model viewer (the toolkit's model/model.h), a mode of the menu's "Alternative modes"
// page: one character alone, on a stage or on plain white or black, at any size, standing as it does in a fight (and
// moved with player 1's controls), or going through any one of its animations.
//
// It is a round of a two-player match, reached unseen as the stage viewer's is (fight/alternative.h), with the second
// player put out of the way: not followed by the camera, out of the picture, and never drawn.
//
// A character's animations are in groups (010380 starts one: the group and the number in it, each a word of offsets),
// from a list a character at 08A918. An animation is frames of 0x10 bytes: a word for its picture, a word of how many
// frames of the game it is shown for (its top bit: the frame after it is the one the long after it names), the picture,
// two words more, and the number of its set of boxes in the character's table of them (16 bytes a set, the long at
// the record's +68). Showing one is putting its frame in the player's record each frame, as the game's own 001AE6 does,
// with the time left kept high so that the game's own stepping (001AC2) never comes round.
#include "colours.h"
#include "model.h"
#include "alternative.h"
#include "session.h"
#include "state.h"
#include "xmvsf.h"

#include <algorithm>
#include <iterator>

namespace cps2recomp {

namespace {
    using namespace xmvsf;

    constexpr const char *kModel = "Model viewer";
    constexpr int kStartMost = 9000; // frames after which getting to a round is given up

    constexpr uint32_t kAnimationLists = 0x08A918; // a long a character (by its word kCharacter, twice): its groups
    constexpr int kGroups = 26, kMostAnimations = 96;
    constexpr uint32_t kFrameBytes = 0x10, kFrameTime = 0x02, kFramePicture = 0x04, kFrameBoxes = 0x0E;
    constexpr uint32_t kTimer = 0x3E, kNextFrame = 0x38, kBoxSets = 0x68; // the record's

    struct State {
        bool on = false;
        bool starting = false;
        int32_t frames = 0;
    } g;
    bool g_fast = false;

    void Fast(bool on) {
        if (on != g_fast) {
            g_fast = on;
            SetFastForward(on);
        }
    }
    void End() {
        if (g.on) {
            g = State();
            modes::Wanted() = MatchWanted();
            Fast(false);
            model::End();
        }
    }

    uint32_t Read32(Machine &machine, uint32_t address) {
        return (static_cast<uint32_t>(machine.Read16(address)) << 16) | machine.Read16(address + 2);
    }
    bool InRom(uint32_t address) {
        return address >= 0x080000 && address < 0x380000 && !(address & 1);
    }
    bool IsFrame(Machine &machine, uint32_t frame) {
        return InRom(frame) && InRom(Read32(machine, frame + kFramePicture)) && (machine.Read16(frame + kFrameTime) & 0x7FFF) != 0;
    }
    // Where an animation of the character's starts (by its number in the menu: its word kCharacter is twice it)
    uint32_t AnimationAt(Machine &machine, int character, int group, int number) {
        const uint32_t list = Read32(machine, kAnimationLists + static_cast<uint32_t>(character) * 4);
        if (!InRom(list)) {
            return 0;
        }
        const uint32_t groupAt = list + machine.Read16(list + static_cast<uint32_t>(group) * 2);
        const uint32_t frame = groupAt + machine.Read16(groupAt + static_cast<uint32_t>(number) * 2);
        return IsFrame(machine, frame) ? frame : 0;
    }
    int FrameTime(Machine &machine, uint32_t frame) {
        return machine.Read16(frame + kFrameTime) & 0x7FFF;
    }
    uint32_t NextFrame(Machine &machine, uint32_t frame) {
        const uint32_t next = machine.Read16(frame + kFrameTime) & 0x8000 ? Read32(machine, frame + kFrameBytes) : frame + kFrameBytes;
        return next == frame || !IsFrame(machine, next) ? 0 : next;
    }
    // A frame put in player 1's record as 001AE6 puts it (back to standing: the character's first animation, which is
    // its standing, with the game's stepping let go on from it)
    void ShowFrame(Machine &machine, uint32_t frame) {
        const uint32_t player = Player(0);
        const bool standing = frame == 0;
        if (standing) {
            frame = AnimationAt(machine, Ram16(player + kCharacter) / 2, 0, 0);
            if (!frame) {
                return;
            }
        }
        SetRam32(player + kFrame, frame);
        SetRam16(player + 0x3C, machine.Read16(frame));
        SetRam32(player + 0x40, Read32(machine, frame + kFramePicture));
        SetRam32(player + 0x62, Read32(machine, frame + 0x08));
        SetRam16(player + 0x66, machine.Read16(frame + 0x0C));
        const uint32_t boxes = Ram32(player + kBoxSets) + static_cast<uint32_t>(machine.Read16(frame + kFrameBoxes)) * 16;
        for (uint32_t at = 0; at < 16; at += 4) {
            SetRam32(player + 0x70 + at, Read32(machine, boxes + at));
        }
        SetRam16(player + kTimer, standing ? 1 : 0x7FFF);
        SetRam32(player + kNextFrame, frame + kFrameBytes);
    }

    // Objects by what they are: for the frame's own picture, the stage's alone (NotTheStage); for the character's, the
    // fighters' sets that touch player 1 (model::Description::nearestOnly); for the picture as the board would show it,
    // everything but the panel and the second player's sets
    bool IsModel(uint32_t tile, uint16_t attributes) {
        const unsigned set = attributes & 0x1F;
        return set >= kPlayer1Set && set < kPlayer1Set + static_cast<unsigned>(kPalettesInSet) && !ThePanel(tile, attributes);
    }
    bool InAFight(uint32_t tile, uint16_t attributes) {
        const unsigned set = attributes & 0x1F;
        return !ThePanel(tile, attributes) && !(set >= kPlayer2Set && set < kPlayer2Set + static_cast<unsigned>(kPalettesInSet)) &&
               set != kShadowSet;
    }

    void StartModel(Machine &machine) {
        End();
        modes::End();
        EndBattle();
        SetFeature("training_mode", false);
        g.on = true;
        g.starting = true;
        model::Begin();
        modes::Wanted().stage = model::Plain(model::Background()) ? 2 : model::Background() + 1;
        const int who = model::Number(model::Character());
        modes::Wanted().characters[0] = modes::Wanted().partners[0] = who * 2;
        modes::Wanted().characters[1] = modes::Wanted().partners[1] = (who + 1) % kCharacters * 2;
        machine.Reset();
    }

    // The second player out of the way: far off to the right (the game keeps it at the picture's edge), with the camera
    // following the first alone (CameraWay)
    void PutAway() {
        SetRam16(Player(1) + kX, static_cast<uint16_t>(Ram16(Player(0) + kX) + 0x300));
    }
} // namespace

bool ModelViewer() {
    return g.on;
}

// 070232: the camera's step (A6 its record): it goes the way the byte kCameraWay says (or the one two after it, for
// some fights' ends). In the viewer, after player 1 alone, its own way of doing that; where a mode wants the camera
// (fight/battle.h, CameraWanted), there, and the game's step not made.
bool CameraWay(M68K &c) {
    if (g.on && !g.starting) {
        SetRam8(kCameraWay, kCameraFollowsOne);
        SetRam8(kCameraWay + 2, kCameraFollowsOne);
    }
    const CameraWanted &wanted = modes::CameraAt();
    const uint32_t camera = c.a[6] & 0xFFFFFF;
    if ((wanted.x >= 0 || wanted.y >= 0) && camera == CameraRecord()) {
        // (either where it is wanted, the other where it is; kept between its ends unless wanted beyond them)
        const auto place = [&](uint32_t word, int at, uint32_t ends) {
            const int low = static_cast<int16_t>(Ram16(camera + ends)), high = static_cast<int16_t>(Ram16(camera + ends + 2));
            SetRam16(camera + word, static_cast<uint16_t>(wanted.beyondEnds ? at : std::clamp(at, low, high)));
        };
        if (wanted.x >= 0) {
            place(kCameraX, wanted.x, kCameraLeft);
        }
        if (wanted.y >= 0) {
            place(kCameraY, wanted.y, kCameraTop);
        }
        Return(c);
        return true;
    }
    return false;
}
void StartModelViewer(Machine &machine) {
    StartModel(machine);
}

void ModelBeforeFrame(Machine &machine) {
    if (!g.on) {
        return;
    }
    modes::Wanted().stage = model::Plain(model::Background()) ? 2 : model::Background() + 1;
    if (g.starting) {
        Fast(true);
        if (modes::ToRound(machine, g.frames)) {
            g.starting = false;
            Fast(false);
            // (the character in the middle of the stage: the camera's middle, and the picture's)
            int left = 0, right = 0;
            CameraEnds(Ram8(kStage), left, right);
            SetRam32(Player(0) + kX, static_cast<uint32_t>((left + right) / 2 + kPictureLeft + 0xC0) << 16);
        } else if (g.frames > kStartMost) {
            End();
        }
        return;
    }
    machine.players[1] = 0;
    PutAway();
    if (colours::EditorAnimation()) {
        model::Scan(machine);
    }
    colours::EditorBeforeFrame(machine); // (takes player 1's controls, when it is on)
    SetRam8(kRoundClock, 0x99);
    model::BeforeFrame(machine);
}

void ModelFrame(Machine &machine) {
    if (!g.on) {
        return;
    }
    // (the board reset, by the menu or by another mode starting: the viewer is over)
    if (!g.starting && Ram8(kPlaying) == 0) {
        End();
        return;
    }
    if (g.starting) {
        return;
    }
    if (model::Changed()) {
        StartModel(machine);
        return;
    }
    machine.video.hideObject = NotTheStage;
    model::Frame(machine);
    colours::EditorFrame(machine);
}

// (Nothing of the viewer is kept in a save state: a state loaded while viewing ends the viewer, and what is loaded is
// the fight it holds.)
void ModelState(Archive &ar) {
    if (!ar.Saving() && !RunningAhead()) {
        End();
    }
}

void RegisterModel() {
    model::Description d;
    d.page = kModel;
    d.pageWords = "One character by itself, on a stage or on white or black, at any size, standing or going through any of its "
                  "animations.";
    d.resetWords = "Puts every choice of this page back: the first character and stage, its own size, standing, full speed, "
                   "repeating at once, one animation only. (While viewing, the viewer goes to that character and stage.)";
    d.animationWords = "What the character does: stands as in a fight, moved with player 1's controls, or goes through one of its "
                       "animations over and over. They are named by the game's groups of them and a number (A34); the ones a "
                       "character has not got are passed over. While one is showing, left and right go to the one before and the "
                       "one after, and its name is shown.";
    d.foregroundWords = "What the stage has in front of the fighters: in front of the character too, as in a fight, or behind it so "
                        "that all of it is seen. In front only at the character's own size, and not in the colour editor.";
    d.characters = {std::begin(kCharacterNames), std::end(kCharacterNames)};
    d.stages = {std::begin(kStageNames), std::end(kStageNames)};
    d.start = [](Machine &machine) {
        colours::EndEditor();
        StartModel(machine);
    };
    d.lists = kGroups;
    d.perList = kMostAnimations;
    d.animationAt = AnimationAt;
    d.isFrame = IsFrame;
    d.frameTime = FrameTime;
    d.nextFrame = NextFrame;
    d.showFrame = ShowFrame;
    d.animationChoice = colours::EditorAnimation;
    d.isModel = IsModel;
    d.inAFight = InAFight;
    d.nearestOnly = true;
    d.largeLift = 80; // (the characters are about twice as tall as the Alpha games')
    d.anchor = [](Machine &, int &x, int &y) {
        const uint32_t camera = CameraRecord();
        x = static_cast<int16_t>(Ram16(Player(0) + kX)) - static_cast<int16_t>(Ram16(camera + kCameraX)) - kPictureLeft;
        y = -1; // (its feet found from its picture)
    };
    d.shift = colours::EditorShift;
    d.linesInSet = kPalettesInSet;
    d.firstLine = [](Machine &) { return static_cast<int>(kPlayer1Set); };
    d.spareFrame = [](Machine &) { SetRam8(kRoundClock, 0x99); };
    model::Register(d);
    AddHook(HookAt(0x070232), "CameraWay", CameraWay);
}

} // namespace cps2recomp
