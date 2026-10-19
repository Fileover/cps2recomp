// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The character viewer of a beat-'em-up's "Alternative modes" page (beat.h), as the fighting games' model viewer has
// it: one of the players' characters by itself, on one of the stages or on plain white or black, at a size from a
// quarter to twenty-five times its own, moved with player 1's controls or going through any one of its animations.
//
// It is a game begun unseen at the stage chosen (as the stage viewer's are: modes.cpp) with the character chosen,
// the enemies standing still and unseen. The picture is made in two parts: the frame's own is the stage with
// nothing on it; the character is then drawn by itself (the board's objects alone, and of them only its own) and
// laid over the stage or the plain colour at the size wanted, about the place where it stands.
//
// A character's animations are lists of where each begins (the game's sources find them); showing one is putting
// its frames into the character's record each frame, as the game would, and keeping the game's own stepping off.
#include "beat.h"

#include "state.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <string>
#include <vector>

namespace cps2recomp::beat {

void BeginModel(Machine &machine, int stage); // modes.cpp

namespace {
    constexpr const char *kModel = "Character viewer";
    // The "Size" choice, in quarters: the character's own size first, then larger, and round to the smaller ones
    constexpr int kSizes[] = {4, 6, 8, 12, 16, 24, 32, 48, 64, 100, 1, 2, 3};
    // The "Speed" choice: a frame of the game in this many pictures of the screen
    constexpr int kSlowBy[] = {1, 2, 4, 8};
    constexpr int kListsMost = 8, kAList = 128; // (the "Animation" choice: up to 8 lists of up to 128, named A0...)
    constexpr int kAnimationChoices = 1 + kListsMost * kAList;
    constexpr int kFramesASecond = 60;
    constexpr int kNamedFor = 120;                 // frames an animation's name is shown for after it changes
    constexpr int kHeldFirst = 20, kHeldEvery = 8; // frames a direction is held before it repeats, and between repeats
    constexpr int kForGoodShown = 30;              // frames a frame shown for good is shown before the animation starts again

    Description g_d;

    struct ModelState {
        bool on = false;
        int32_t character = 0, background = 0; // the choices the game was begun with
        int32_t choice = 0;                    // the animation shown (0: none, the character stands)
        uint32_t first = 0, frame = 0;
        int32_t left = 0, held = 0, named = 0;
        int32_t stick = 0, stickHeld = 0;
        int32_t anchorX = 0, anchorY = -1; // where the character stood in the picture a frame ago
    } g;
    std::vector<std::vector<uint32_t>> g_lists; // (the character's, as the game's sources found them)
    int g_listsOf = -1;

    int Stages() {
        return static_cast<int>(g_d.stages.size());
    }
    int White() {
        return Stages();
    }
    int Black() {
        return Stages() + 1;
    }
    int Characters() {
        return static_cast<int>(g_d.characters.size());
    }

    const std::vector<std::vector<uint32_t>> &Lists(int character) {
        if (g_listsOf != character) {
            g_lists.clear();
            g_d.animations(character, g_lists);
            g_listsOf = character;
        }
        return g_lists;
    }
    // Where the animation of a choice begins (0: the character has none of that number)
    uint32_t AnimationAt(int character, int choice) {
        if (choice < 1 || choice >= kAnimationChoices) {
            return 0;
        }
        const auto &lists = Lists(character);
        const size_t list = static_cast<size_t>((choice - 1) / kAList), number = static_cast<size_t>((choice - 1) % kAList);
        return list < lists.size() && number < lists[list].size() ? lists[list][number] : 0;
    }
    // The animation after `choice` (or before it) that the character has, going round from the last to the first
    int Step(int character, int choice, int by) {
        for (int n = 0; n < kAnimationChoices; n++) {
            choice = 1 + (choice - 1 + by + (kAnimationChoices - 1)) % (kAnimationChoices - 1);
            if (AnimationAt(character, choice)) {
                return choice;
            }
        }
        return 0;
    }
    int FrameTime(uint32_t frame) {
        const int time = g_d.frameTime(frame);
        return time > 0 ? time : kForGoodShown;
    }

    void ResetChoices(Machine &) {
        for (const char *choice : {"model_character", "model_background", "model_size", "model_animation", "model_speed", "model_repeat",
                                   "model_all"}) {
            SetFeatureValue(choice, 0);
        }
    }

    // The animation chosen in the menu, a frame of it shown (or none: the character stands, player 1's to move)
    void ShowAnimation(Machine &machine) {
        const int character = g.character;
        int choice = std::clamp(FeatureValue("model_animation"), 0, kAnimationChoices - 1);
        if (choice && !AnimationAt(character, choice)) {
            choice = Step(character, choice, 1);
            SetFeatureValue("model_animation", choice);
        }
        const bool all = FeatureOn("model_all");
        if (all && !choice) {
            choice = Step(character, 0, 1);
            SetFeatureValue("model_animation", choice);
        }
        const uint16_t stick = machine.players[0] & (kLeft | kRight);
        if (choice && (stick == kLeft || stick == kRight)) {
            // (left and right: the animation before and the one after, at a press and then eight a second)
            g.stickHeld = g.stick == stick ? g.stickHeld + 1 : 0;
            if (g.stickHeld == 0 || (g.stickHeld >= kHeldFirst && (g.stickHeld - kHeldFirst) % kHeldEvery == 0)) {
                choice = Step(character, choice, stick == kRight ? 1 : -1);
                SetFeatureValue("model_animation", choice);
            }
        }
        g.stick = stick;
        const uint32_t first = AnimationAt(character, choice);
        if (!first) {
            if (g.choice) {
                g_d.showFrame(0);
            }
            g.choice = 0;
            g.named = 0;
            return; // (standing: player 1's controls are the character's)
        }
        for (int n = 1; n < Machine::kMaxPlayers; n++) {
            machine.players[n] = 0;
        }
        machine.players[0] = 0;
        if (choice != g.choice || first != g.first) {
            g.choice = choice;
            g.first = g.frame = first;
            g.left = FrameTime(first);
            g.held = 0;
            g.named = kNamedFor;
        } else if (g.held > 0) {
            g.held--;
        } else if (--g.left <= 0) {
            // (the frame's time is up: the next; the end, or a frame held for good, is the start again: after the wait
            // chosen, or the next animation with "Play all")
            uint32_t next = g_d.frameTime(g.frame) == 0 ? 0 : g_d.nextFrame(g.frame);
            const bool repeat = next == 0 || next == g.frame || next < g.frame;
            if (next == 0 || next == g.frame) {
                next = g.first;
            }
            const int wait = std::clamp(FeatureValue("model_repeat"), 0, 10) * kFramesASecond;
            if (repeat && wait && g.left == 0) {
                g.held = wait;
                g.left = -1;
            } else if (repeat && all) {
                SetFeatureValue("model_animation", Step(character, choice, 1));
                g.left = 0;
            } else {
                g.frame = next;
                g.left = FrameTime(next);
            }
        }
        g_d.showFrame(g.frame);
    }

    void NameAnimation() {
        if (g.choice && g.named > 0) {
            g.named--;
            char text[32];
            snprintf(text, sizeof(text), "%c%d", 'A' + (g.choice - 1) / kAList, (g.choice - 1) % kAList);
            SetHud({{0.5f, 0.04f, 0, text}});
        } else {
            SetHud({});
        }
    }

    // The frame's picture made: the stage (or a plain colour), and the character over it at the size wanted
    void Compose(Machine &machine) {
        static std::vector<uint32_t> model;
        const int width = machine.PictureWidth(), height = Video::kHeight;
        uint32_t *picture = machine.Picture();
        model.resize(static_cast<size_t>(width) * height);
        Video &video = machine.video;
        const unsigned layers = video.layerMask;
        const bool marked = video.markObjects;
        const auto hidden = video.hideObject;
        video.layerMask = 1;
        video.markObjects = true;
        video.hideObject = [](uint32_t tile, uint16_t attributes) { return !g_d.characterObject(tile, attributes); };
        for (int line = 0; line < height; line++) {
            video.RenderLine(line, &model[static_cast<size_t>(line) * width]);
        }
        video.layerMask = layers;
        video.markObjects = marked;
        video.hideObject = hidden;
        // (where it stands: across from the game's sources, and its feet the bottom of its picture as it first stood)
        int x = -1, y = -1;
        g_d.standing(x, y);
        if (y < 0 && g.anchorY < 0) {
            for (int line = height - 1; line >= 0 && g.anchorY < 0; line--) {
                for (int column = 0; column < width; column++) {
                    if (model[static_cast<size_t>(line) * width + column] & 0x80000000u) {
                        g.anchorY = line;
                        break;
                    }
                }
            }
        }
        if (x >= 0) {
            g.anchorX = x;
        }
        if (y >= 0) {
            g.anchorY = y;
        }
        if (g.background >= Stages()) {
            std::fill(picture, picture + static_cast<size_t>(width) * height, g.background == White() ? 0xFFFFFFu : 0u);
        }
        const int size = kSizes[std::clamp(FeatureValue("model_size"), 0, static_cast<int>(std::size(kSizes)) - 1)];
        const bool large = size > 8;
        const int anchorY = g.anchorY < 0 ? height - 32 : g.anchorY;
        const int fromX = g.anchorX, fromY = anchorY - (large ? 48 : 0);
        const int toX = large ? width / 2 : g.anchorX, toY = large ? height / 2 : anchorY;
        auto back = [size](int to, int about, int from) {
            const int quarters = (to - about) * 4 + 2;
            return from + (quarters >= 0 ? quarters / size : -((-quarters + size - 1) / size));
        };
        for (int py = 0; py < height; py++) {
            const int sourceY = back(py, toY, fromY);
            if (sourceY < 0 || sourceY >= height) {
                continue;
            }
            for (int px = 0; px < width; px++) {
                const int sourceX = back(px, toX, fromX);
                if (sourceX >= 0 && sourceX < width) {
                    const uint32_t pixel = model[static_cast<size_t>(sourceY) * width + sourceX];
                    if (pixel & 0x80000000u) {
                        picture[static_cast<size_t>(py) * width + px] = pixel & 0xFFFFFF;
                    }
                }
            }
        }
    }

    std::vector<const char *> Backgrounds() {
        std::vector<const char *> names = g_d.stages;
        names.push_back("White");
        names.push_back("Black");
        return names;
    }
    const std::vector<const char *> &AnimationNames() {
        static std::vector<std::string> text;
        static std::vector<const char *> names;
        if (names.empty()) {
            text.push_back("Standing (you control it)");
            for (int list = 0; list < kListsMost; list++) {
                for (int n = 0; n < kAList; n++) {
                    text.push_back(std::string(1, static_cast<char>('A' + list)) + std::to_string(n));
                }
            }
            for (const std::string &one : text) {
                names.push_back(one.c_str());
            }
        }
        return names;
    }
} // namespace

bool CharacterViewer();

int ViewedCharacter() {
    return g.character;
}

void StartModel(Machine &machine) {
    g = ModelState();
    g.on = true;
    g.character = std::clamp(FeatureValue("model_character"), 0, Characters() - 1);
    g.background = std::clamp(FeatureValue("model_background"), 0, Black());
    BeginModel(machine, g.background < Stages() ? g.background : 0);
}

void EndModel() {
    if (g.on) {
        g = ModelState();
        SetSlowMotion(1);
        SetHud({});
    }
}

void ModelBeforeFrame(Machine &machine) {
    SetSlowMotion(kSlowBy[std::clamp(FeatureValue("model_speed"), 0, static_cast<int>(std::size(kSlowBy)) - 1)]);
    ShowAnimation(machine);
}

void ModelFrame(Machine &machine) {
    // (another character or background chosen in the menu: the viewer begins again with it)
    if (std::clamp(FeatureValue("model_character"), 0, Characters() - 1) != g.character ||
        std::clamp(FeatureValue("model_background"), 0, Black()) != g.background) {
        StartModel(machine);
        return;
    }
    Compose(machine);
    NameAnimation();
}

void RegisterModel(const Description &description) {
    g_d = description;
    if (description.characters.empty()) {
        return;
    }
    Game &game = TheGame();
    game.pages.push_back({kModel, "Alternative modes",
                          "One character by itself, on a stage or on white or black, at any size, moved with your controls or "
                          "going through any of its animations."});
    game.commands.push_back({"model_start", kModel, "Start",
                             "Shows the character chosen below by itself. Player 1's controls move it while it stands. Reset ends it.",
                             StartModel});
    game.commands.push_back({"model_reset", kModel, "Reset options",
                             "Puts every choice of this page back: the first character and stage, its own size, standing, full "
                             "speed, repeating at once, one animation only.",
                             ResetChoices});
    game.features.push_back({"model_character", kModel, "Character", "Who is shown. Changing it while viewing goes to the new one.", 0,
                             description.characters});
    game.features.push_back({"model_background", kModel, "Background",
                             "What is behind the character: one of the stages, or plain white or black. Changing it while viewing "
                             "goes to the new one.",
                             0, Backgrounds()});
    game.features.push_back({"model_size", kModel, "Size", "How large the character is drawn, from a quarter of its own size to 25 times.", 0,
                             {"1x", "1.5x", "2x", "3x", "4x", "6x", "8x", "12x", "16x", "25x", "0.25x", "0.5x", "0.75x"}});
    game.features.push_back({"model_animation", kModel, "Animation",
                             "What the character does: stands, moved with player 1's controls, or goes through one of its "
                             "animations over and over. They are named by the game's lists of them and a number (A3); the ones a "
                             "character has not got are passed over. While one is showing, left and right go to the one before "
                             "and the one after, and its name is shown.",
                             0, AnimationNames()});
    game.features.push_back({"model_speed", kModel, "Speed",
                             "How fast the viewer runs: slow motion for looking at an animation frame by frame.", 0,
                             {"Full", "1/2", "1/4", "1/8"}});
    game.features.push_back({"model_repeat", kModel, "Repeat after", "How long an animation's last frame is held before it starts again.",
                             0,
                             {"At once", "1 second", "2 seconds", "3 seconds", "4 seconds", "5 seconds", "6 seconds", "7 seconds",
                              "8 seconds", "9 seconds", "10 seconds"}});
    game.features.push_back({"model_all", kModel, "Play all",
                             "Goes through all of the character's animations, one after another, round from the last to the "
                             "first. Left and right still step through them.",
                             0});
}

} // namespace cps2recomp::beat
