// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior Definitive: the first Darkstalkers' stages. Darkstalkers (dstlk) is not made from Vampire Savior's
// program (its engine is Night Warriors' own: games/nwarr), so its stages are not taken in as the characters are.
// They are shown as that game shows them: its board runs beside this one (runtime/guest.h, game.cfg's guest_set),
// a two-player match on the stage, its fighters left standing and never seen, its clock held; its camera is put where
// this game's is every frame. The picture is that board's stage, with this game's fighters and panel over it (drawn
// alone, marked: Video::markObjects); the music is that stage's own, in place of this game's stage tune (the
// soundtrack's Take), with this game's fades. Both games' stages are the same size (the camera's ends 256 and 640,
// the floor 0x28 up), so the camera needs no more than its place.
//
// Which: a stage chosen by name (training's, the stage viewer's: the twelve after Vampire Savior's), or with the
// "Darkstalkers' stages" feature, a match's stage is the opponent's own in Darkstalkers, where it has one (player 2's
// in a two-player match; the computer's against it): twelve of the eighteen have.
//
// Darkstalkers' facts here were found for its port (games/dstlk/family.h; games/nwarr/src/nwarr.h, where Night
// Warriors' places are): A5 = FF8000; the game's steps FF8000, FF8004, FF8008; credits FF81D0; who is in FF81F9; the
// stage FF9388, a two-player match's set at 0059C6 from A5+13B0; the clock FF9409; the camera an object at FF9508.
#include "definitive.h"
#include "guest.h"
#include "modes.h"
#include "session.h"
#include "state.h"
#include "vsav.h"
#include "widescreen.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

namespace cps2recomp {

bool BattleMode(); // (fight/battle.h)

namespace {
    using namespace vsav;

    // This game's match step (kMatchStep) while its versus screen is up: from the match's start to its first round's
    // (a two-player match's steps, logged: 0A, then 0 and 2 as the round is set up, 4 its round)
    constexpr uint16_t kMatchVersus = 0x0A;

    // --- Darkstalkers' (the guest's) memory ---
    constexpr uint32_t kDsMode = 0xFF8000, kDsStep = 0xFF8004, kDsStep2 = 0xFF8008;
    constexpr uint32_t kDsCredits = 0xFF81D0, kDsPlayersIn = 0xFF81F9;
    constexpr uint32_t kDsVersusStage = 0xFF93B0; // (a byte, the stage's number + 1: 0059C6 makes it the stage)
    constexpr uint32_t kDsClock = 0xFF9409;
    constexpr uint32_t kDsCamera = 0xFF9508, kDsCameraX = 0x10, kDsCameraY = 0x14;
    constexpr uint32_t kDsAcrossStep = 0x34, kDsUpStep = 0x36, kDsAcrossWay = 0x2C, kDsUpWay = 0x2D;
    // Its code: a two-player match's stage set (0059C6), the camera's step across and up (0EDAF4, 0EDC1A: called from
    // its frame, 0EDA94)
    constexpr uint32_t kDsVersusStageSet = 0x0059C6, kDsCameraAcross = 0x0EDAF4, kDsCameraUp = 0x0EDC1A;
    // A game (FF8000 2), the match (FF8004 4), a round fought (FF8008 0A); the character choice (FF8004 0)
    enum : uint16_t { kDsGame = 2, kDsChoosing = 0, kDsMatch = 4, kDsFought = 0x0A };
    // Objects by colour set: the panel's, the players' and their shadows' and sparks' are below 14, the stage's from it
    constexpr unsigned kDsStageSets = 0x14;

    constexpr int kDsStages = kMoreStages; // (family.h names them)
    // A character's (this game's number) stage there, 1 the first (0: none)
    int DsStageOf(uint8_t character) {
        switch (character) {
        case 0x01: return 1;  // Demitri
        case 0x02: return 2;  // Gallon (Jon Talbain)
        case 0x12: return 2;  // ... Dark Gallon
        case 0x03: return 3;  // Victor
        case 0x04: return 4;  // Zabel (Lord Raptor)
        case 0x05: return 5;  // Morrigan
        case 0x06: return 6;  // Anakaris
        case 0x07: return 7;  // Felicia
        case 0x08: return 8;  // Bishamon
        case 0x18: return 8;  // ... Oboro Bishamon
        case 0x09: return 9;  // Aulbath (Rikuo)
        case 0x0A: return 10; // Sasquatch
        case 0x10: return 11; // Phobos (Huitzil)
        case 0x11: return 12; // Pyron
        default: return 0;
        }
    }

    struct State {
        int8_t stage = 0;  // the Darkstalkers stage shown, 1 the first (0: this game's own); its board is in its round
        int8_t follow = 0; // its camera goes where this game's is (while its stage is shown)
        int16_t cameraX = 0, cameraY = 0; // where its camera is to be (this game's, as its last frame left it)
    } g;
    std::vector<uint8_t> g_guestState; // (the guest's machine, for a save state: kept so a peer without the set can read past it)
    size_t g_soundBefore = 0;

    uint8_t *GuestByte(Machine &m, uint32_t address) {
        return &m.ram[address & 0xFFFF];
    }
    uint8_t G8(Machine &m, uint32_t address) {
        return *GuestByte(m, address);
    }
    uint16_t G16(Machine &m, uint32_t address) {
        const uint8_t *p = GuestByte(m, address);
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }
    void SetG8(Machine &m, uint32_t address, uint8_t value) {
        *GuestByte(m, address) = value;
    }
    uint16_t C16(M68K &c, uint32_t address) {
        const uint8_t *p = &c.ram[address & 0xFFFF];
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }
    void SetC16(M68K &c, uint32_t address, uint16_t value) {
        uint8_t *p = &c.ram[address & 0xFFFF];
        p[0] = static_cast<uint8_t>(value >> 8);
        p[1] = static_cast<uint8_t>(value);
    }
    void SetC8(M68K &c, uint32_t address, uint8_t value) {
        c.ram[address & 0xFFFF] = value;
    }
    void ReturnGuest(M68K &c) {
        const uint8_t *p = &c.ram[c.a[7] & 0xFFFF];
        c.pc = static_cast<uint32_t>((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]) & 0xFFFFFF;
        c.a[7] += 4;
    }

    // --- each stage's round, made ready on a thread of its own ---

    // Each stage's round is made once, on a spare board of the guest (runtime/guest.h, GuestSpare) and a thread of its
    // own, from the program's start: its board played with made-up controls (credits, both Starts, a punch at each
    // screen that waits for one) into a two-player match on the stage, to the round. That is the same board, frame for
    // frame, every time, so a match on a stage always starts from it, however long ago it was made: the same in a run
    // played straight, its frames run again (online play's rollback), a save state loaded or the other player's game.
    // The thread starts with the game and goes through the stages in turn (about 500 of the guest's frames each, from
    // its character choice, kept on the way), a stage wanted first; a match that wants one not ready yet waits for it.
    struct Ready {
        std::mutex lock;
        std::condition_variable done;
        std::vector<uint8_t> round[kDsStages]; // (each written once, by the thread; read once `made` says so)
        std::atomic<bool> made[kDsStages] = {};
        std::atomic<int> wanted{0};
        bool started = false;
    };
    Ready &TheReady() {
        static Ready &ready = *new Ready; // (never freed: the thread may run until the program is gone)
        return ready;
    }
    Machine *g_ahead = nullptr; // (the thread's board)
    int8_t g_aheadStage = 0;    // (the stage it is taking to its round: 0 none yet)
    bool Ahead(M68K &c) {
        return g_ahead && &c == &g_ahead->cpu;
    }

    // The guest's controls on its way to a round: credits kept, each player's Start until both are in, then a punch
    // now and then (the character choice, anything else that waits for a button)
    void Controls(Machine &m, uint8_t &pressing) {
        memset(m.players, 0, sizeof(m.players));
        SetG8(m, kDsCredits, 2);
        const bool down = (pressing & 7) < 3;
        pressing++;
        if (!down || (G16(m, kDsMode) == kDsGame && G16(m, kDsStep) == kDsMatch)) {
            return;
        }
        if (G8(m, kDsPlayersIn) != 3) {
            m.players[(pressing >> 3) & 1] = kStart;
            return;
        }
        m.players[(pressing >> 3) & 1] = kPunch1;
    }
    bool RoundOn(Machine &m) {
        return G16(m, kDsMode) == kDsGame && G16(m, kDsStep) == kDsMatch && G16(m, kDsStep2) == kDsFought;
    }
    bool Choosing(Machine &m) {
        return G16(m, kDsMode) == kDsGame && G16(m, kDsStep) == kDsChoosing && G8(m, kDsPlayersIn) == 3;
    }
    std::vector<uint8_t> Saved(Machine &m) {
        std::vector<uint8_t> data;
        Archive ar(data, true);
        m.Serialize(ar);
        return data;
    }
    void Load(Machine &m, std::vector<uint8_t> data) {
        Archive ar(data, false);
        m.Serialize(ar);
    }

    void MakeRounds() {
        Ready &ready = TheReady();
        Machine &m = *g_ahead;
        m.drawing = false;
        // To the character choice, kept
        uint8_t pressing = 0;
        constexpr int kMostFrames = 20000; // (never more than about 1,000: a board that does not get there is given up)
        m.Reset();
        for (int n = 0; n < kMostFrames && !Choosing(m); n++) {
            Controls(m, pressing);
            m.RunFrame();
            m.audio.clear();
        }
        const std::vector<uint8_t> choosing = Saved(m);
        const uint8_t choosingPressing = pressing;
        for (int left = kDsStages; left > 0; left--) {
            // (the stage wanted first, else the next not made)
            int stage = ready.wanted.load();
            if (stage < 1 || stage > kDsStages || ready.made[stage - 1]) {
                stage = 0;
                for (int s = 1; s <= kDsStages && !stage; s++) {
                    stage = ready.made[s - 1] ? 0 : s;
                }
            }
            if (!stage) {
                break;
            }
            Load(m, choosing);
            pressing = choosingPressing;
            g_aheadStage = static_cast<int8_t>(stage);
            for (int n = 0; n < kMostFrames && !RoundOn(m); n++) {
                Controls(m, pressing);
                m.RunFrame();
                m.audio.clear();
            }
            memset(m.players, 0, sizeof(m.players));
            {
                std::lock_guard<std::mutex> hold(ready.lock);
                ready.round[stage - 1] = Saved(m);
                ready.made[stage - 1] = true;
            }
            ready.done.notify_all();
        }
    }
    void StartMaking() {
        Ready &ready = TheReady();
        if (ready.started || !Guest()) {
            return;
        }
        ready.started = true;
        g_ahead = GuestSpare();
        std::thread(MakeRounds).detach();
    }
    // Stage `stage`'s round, into the guest's board (waiting for it, if it is not made yet)
    void ToRound(Machine &guest, int stage) {
        Ready &ready = TheReady();
        StartMaking();
        ready.wanted = stage;
        std::unique_lock<std::mutex> hold(ready.lock);
        ready.done.wait(hold, [&] { return ready.made[stage - 1].load(); });
        Load(guest, ready.round[stage - 1]);
    }

    // --- the guest's hooks ---

    // 0059C6: a two-player match's stage, from A5+13B0 (the stage + 1): the one wanted
    bool VersusStageSet(M68K &c) {
        if (Ahead(c) && g_aheadStage > 0) {
            SetC8(c, kDsVersusStage, static_cast<uint8_t>(g_aheadStage));
        }
        return false;
    }
    // 0EDAF4, 0EDC1A: the camera (A6) steps across, and up or down, after the players; while its stage is shown, it goes
    // where this game's is, by the step there (the scroll screens behind move by the step: 0x34, 0x36). The game paints
    // the stage's new columns and rows as its camera comes to them, a few pixels a frame: so it goes no faster than
    // kMostStep, and catches up a camera put somewhere else at once (training's start over) in a few frames.
    constexpr int kMostStep = 8;
    int16_t StepTo(int16_t was, int16_t to) {
        return static_cast<int16_t>(std::clamp(to - was, -kMostStep, kMostStep));
    }
    bool CameraAcross(M68K &c) {
        if (Ahead(c) || !g.follow) {
            return false;
        }
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        const uint16_t was = C16(c, camera + kDsCameraX);
        const int16_t step = StepTo(static_cast<int16_t>(was), g.cameraX);
        SetC16(c, camera + kDsCameraX, static_cast<uint16_t>(was + step));
        SetC16(c, camera + kDsAcrossStep, static_cast<uint16_t>(step));
        SetC8(c, camera + kDsAcrossWay, step >= 0 ? 4 : 0);
        ReturnGuest(c);
        return true;
    }
    bool CameraUp(M68K &c) {
        if (Ahead(c) || !g.follow) {
            return false;
        }
        const uint32_t camera = c.a[6] & 0xFFFFFF;
        const uint16_t was = C16(c, camera + kDsCameraY);
        const int16_t step = StepTo(static_cast<int16_t>(was), g.cameraY);
        SetC16(c, camera + kDsCameraY, static_cast<uint16_t>(was + step));
        SetC16(c, camera + kDsUpStep, static_cast<uint16_t>(step));
        if (step) {
            SetC8(c, camera + kDsUpWay, step > 0 ? 0x0C : 0x08);
        }
        ReturnGuest(c);
        return true;
    }

    // --- which stage ---

    // The Darkstalkers stage this game's match is to be on now (0: its own)
    int StageWanted() {
        if (!Guest() || BattleMode()) {
            return 0;
        }
        // (a stage chosen by name: training's, the viewer's; after this game's twelve)
        const int wanted = session::WantedStage();
        if (wanted > kStages && wanted <= kStages + kDsStages) {
            return wanted - kStages;
        }
        if (wanted >= 1 && wanted <= kStages) {
            return 0;
        }
        if (!FeatureOn("dstlk_stages") || Ram16(kMode) != 2 || Ram16(kGameStep) != kGameMatch) {
            return 0;
        }
        // The opponent's: the computer's, or player 2's
        const int against = Ram8(Player(0) + kComputer) ? 0 : 1;
        return DsStageOf(Ram8(Player(against) + kCharacter));
    }

    void (*g_before)(Machine &) = nullptr;
    void (*g_frame)(Machine &) = nullptr;
    void (*g_state)(Archive &) = nullptr;

    // The guest's stage is shown: in a match of this game's (not its versus screen), or the stage viewer's
    bool Shown() {
        return g.stage > 0 &&
               (StageViewer() || (Ram16(kMode) == 2 && Ram16(kGameStep) == kGameMatch && Ram16(kMatchStep) != kMatchVersus));
    }

    void BeforeFrame(Machine &machine) {
        if (g_before) {
            g_before(machine);
        }
        g_soundBefore = machine.audio.size();
        // This game's stage music is the guest's while a Darkstalkers stage is chosen (from before the round, so that
        // this game's never starts)
        std::vector<bool> taken;
        if (g.stage > 0) {
            for (const uint16_t tune : family::kStageTunes) {
                if (taken.size() <= tune) {
                    taken.resize(tune + 1u);
                }
                taken[tune] = true;
            }
        }
        machine.soundtrack.Take(std::move(taken));
        if (!Shown()) {
            return;
        }
        // This game draws its fighters and its panel alone, marked, to be laid over the guest's stage (the stage
        // viewer: nothing of its own)
        if (StageViewer()) {
            machine.video.layerMask = 0;
        } else {
            machine.video.layerMask = 1;
            machine.video.hideObject = [](uint32_t tile, uint16_t attributes) { return !NotTheStage(tile, attributes); };
            machine.video.markObjects = true;
        }
    }

    // The guest's frame, after this game's: its picture under this game's marked objects, its sound with this game's
    void Frame(Machine &machine) {
        if (g_frame) {
            g_frame(machine);
        }
        Machine *guest = Guest();
        if (!guest) {
            return;
        }
        // (with the option on, the stages are made ready from the start)
        if (FeatureOn("dstlk_stages")) {
            StartMaking();
        }
        const int want = StageWanted();
        if (want != g.stage) {
            g = State();
            g.stage = static_cast<int8_t>(want);
            if (want) {
                ToRound(*guest, want);
            }
        }
        if (g.stage == 0) {
            return;
        }
        // Its camera where this game's is, while its stage is shown; its clock held
        const bool shown = Shown();
        g.follow = shown;
        g.cameraX = static_cast<int16_t>(std::clamp<int>(Ram16(kCameraX), kCameraLeft, kCameraRight));
        g.cameraY = static_cast<int16_t>(Ram16(kCameraY));
        SetG8(*guest, kDsClock, 0x99);
        memset(guest->players, 0, sizeof(guest->players));
        guest->drawing = machine.drawing;
        guest->video.hideObject = [](uint32_t, uint16_t attributes) { return (attributes & 0x1F) < kDsStageSets; };
        guest->video.wide.side = machine.video.wide.side;
        guest->RunFrame();

        // The picture: the guest's, under this game's marked pixels
        if (shown && machine.drawing && guest->PictureWidth() == machine.PictureWidth()) {
            uint32_t *picture = machine.Picture();
            const uint32_t *stage = guest->Picture();
            const size_t pixels = static_cast<size_t>(machine.PictureWidth()) * Video::kHeight;
            for (size_t i = 0; i < pixels; i++) {
                picture[i] = picture[i] & 0xFF000000 ? picture[i] & 0xFFFFFF : stage[i];
            }
        }
        // The sound: the guest's music, as loud as this game's would be (its fades), while this game's stage tune is
        // what it plays
        const Soundtrack::State &music = machine.soundtrack.state;
        const bool playing = shown && music.song >= 0 && machine.soundtrack.Taken(static_cast<uint16_t>(music.song));
        const int64_t gain = playing ? static_cast<int64_t>(music.level) * music.allLevel / Soundtrack::kFull : 0;
        const size_t from = std::min(g_soundBefore, machine.audio.size());
        const size_t made = std::min(machine.audio.size() - from, guest->audio.size());
        for (size_t i = 0; i < made && gain; i++) {
            const int64_t sample = machine.audio[from + i] + guest->audio[i] * gain / Soundtrack::kFull;
            machine.audio[from + i] = static_cast<int16_t>(std::clamp<int64_t>(sample, -32768, 32767));
        }
        guest->audio.clear();
    }

    void StagesState(Archive &ar) {
        if (g_state) {
            g_state(ar);
        }
        if (ar.version != 0 && ar.version < 9) { // (a save state from before has none)
            return;
        }
        ar.Mark(0x44535447);
        ar.Pod(g);
        Machine *guest = Guest();
        if (ar.Saving()) {
            g_guestState.clear();
            if (guest && g.stage > 0) {
                Archive inner(g_guestState, true);
                guest->Serialize(inner);
            }
            ar.Vec(g_guestState);
        } else {
            ar.Vec(g_guestState);
            if (!guest) {
                g = State(); // (no set here: this game's own stages)
            } else if (g.stage > 0 && !g_guestState.empty()) {
                Archive inner(g_guestState, false);
                guest->Serialize(inner);
            } else if (g.stage > 0) {
                ToRound(*guest, g.stage); // (a peer's state without its board: from its round)
            }
        }
    }
} // namespace

namespace definitive {

void RegisterStages() {
    if (Guest()) {
        AddGuestHook(kDsVersusStageSet, VersusStageSet);
        AddGuestHook(kDsCameraAcross, CameraAcross);
        AddGuestHook(kDsCameraUp, CameraUp);
    }
    Game &game = TheGame();
    // (Display: it changes nothing of the game's play; online, each player sees the stage of their own choice)
    game.features.push_back({"dstlk_stages", "Display", "Darkstalkers' stages",
                             "A match is fought on the opponent's own stage in the first Darkstalkers, where it has one (twelve "
                             "of the eighteen have): its picture and its music, as that game shows them. Needs Darkstalkers' ROM "
                             "set (dstlk) beside this game's, or chosen on its page in the launcher. Training's and the stage "
                             "viewer's lists have them too.",
                             0});
    g_before = game.beforeFrame;
    game.beforeFrame = BeforeFrame;
    g_frame = game.frame;
    game.frame = Frame;
    g_state = game.state;
    game.state = StagesState;
}

} // namespace definitive

} // namespace cps2recomp
