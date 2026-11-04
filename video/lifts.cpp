// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// Capcom's video library as the games share it, written once (docs/NATIVE_VIDEO.md): the routines that hand the
// board the picture, rewritten (docs/READABLE.md) so that they also hand it to the native interface (native.h).
//
// A game says in its game.cfg where its copy of each routine is, and the version: the address, then a CRC-32 of the
// routine's instruction words with their operands left out (`video_palette_commit = 00123E:XXXXXXXX`). The
// routine's operands (where the game keeps its variables, which of the board's addresses it writes) are read from
// the game's own copy, so that one rewrite serves every game with that version. A copy that is not that version
// is not rewritten (its own code runs) and is reported.
#include "native.h"

#include "../runtime/game.h"
#include "../runtime/lift.h"
#include "game_info.h"

#include <zlib.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

namespace cps2recomp {

namespace {
    inline uint16_t OpcodeWord(uint32_t address) {
        const uint8_t *p = ProgramOpcodes(address);
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }
    inline uint32_t OpcodeLong(uint32_t address) {
        return (static_cast<uint32_t>(OpcodeWord(address)) << 16) | OpcodeWord(address + 2);
    }
    inline uint32_t Variable(const M68K &c, int16_t offset) {
        return (c.a[5] + offset) & 0xFFFFFF;
    }

    // `address:crc` from game.cfg; false if the routine there is not that version (the CRC of the instruction words
    // at `words`, offsets from the address)
    bool Find(const char *key, const char *value, std::initializer_list<int> words, uint32_t &address) {
        char *end = nullptr;
        address = static_cast<uint32_t>(strtoul(value, &end, 16));
        const uint32_t want = *end == ':' ? static_cast<uint32_t>(strtoul(end + 1, nullptr, 16)) : 0;
        uint8_t shape[32];
        size_t size = 0;
        for (int at : words) {
            const uint16_t word = OpcodeWord(address + static_cast<uint32_t>(at));
            shape[size++] = static_cast<uint8_t>(word >> 8);
            shape[size++] = static_cast<uint8_t>(word);
        }
        const uint32_t got = static_cast<uint32_t>(crc32(0, shape, static_cast<uInt>(size)));
        if (got != want) {
            fprintf(stderr, "video: game.cfg's %s: the routine at %06X is not the version the toolkit has (its CRC %08X, not %08X); "
                            "the game's own code runs\n", key, address, got, want);
            return false;
        }
        return true;
    }

    // Where a palette's buffer is, from the word the game gives the chip
    inline uint32_t BufferAt(uint16_t base) {
        return 0x900000 + ((static_cast<uint32_t>(base) << 8) & 0x3FC00);
    }

    // --- The palette's commit, once a frame in the vertical blank's interrupt (Alpha's kernel: Street Fighter Alpha
    // 1-3, Zero 2 Alpha, the Vampire Savior family, Pocket Fighter, Puzzle Fighter). Its three instructions:
    //
    //   move.w pages(a5),<CPS-B's palette control>   the pages the chip is to take
    //   move.w base(a5),<CPS-A's palette base>       where from: writing it makes the chip take them
    //   move.w next(a5),base(a5)                     next frame's buffer, as the game asked for it this frame
    //   rts
    struct PaletteCommit {
        int16_t pages, base, next;              // the variables (from A5)
        uint32_t pagesRegister, baseRegister;   // the board's addresses the game's copy writes
    } g_commit;

    void CommitPalette(M68K &c) {
        const uint16_t pages = Ram16(Variable(c, g_commit.pages));
        const uint16_t base = Ram16(Variable(c, g_commit.base));
        // The native interface is handed the upload as the game sees it: its buffer, the pages it asks for
        c.nativeVideo->gamePages = static_cast<uint8_t>(pages);
        c.nativeVideo->Palette(native_video::GamePalette(c.nativeVideo->videoRam, BufferAt(base), c.nativeVideo->gamePages));
        c.Write(g_commit.pagesRegister, 1, pages);
        c.Write(g_commit.baseRegister, 1, base);
        const uint16_t next = Ram16(Variable(c, g_commit.next));
        SetRam16(Variable(c, g_commit.base), next);
        // (the flags of the last move)
        c.flagN = next & 0x8000;
        c.flagZ = next == 0;
        c.flagV = c.flagC = false;
    }

    void AddPaletteCommit(const char *value) {
        uint32_t at;
        if (!Find("video_palette_commit", value, {0, 8, 16, 22}, at)) {
            return;
        }
        g_commit.pages = static_cast<int16_t>(OpcodeWord(at + 2));
        g_commit.pagesRegister = OpcodeLong(at + 4);
        g_commit.base = static_cast<int16_t>(OpcodeWord(at + 10));
        g_commit.baseRegister = OpcodeLong(at + 12);
        g_commit.next = static_cast<int16_t>(OpcodeWord(at + 18));
        if (static_cast<int16_t>(OpcodeWord(at + 20)) != g_commit.base) {
            fprintf(stderr, "video: the palette's commit at %06X does not move the next buffer into the one it gave\n", at);
            return;
        }
        // (move.w d16(An),xxx.l twice: 24 cycles each; move.w d16(An),d16(An): 20; Return adds the rts)
        AddRewrite(at, "CommitPalette", CommitPalette, 0, 68);
    }
    // --- The palette written elsewhere: every other family's commit (inline in its vertical blank's routine), and every
    // game's start-up. Each is one instruction, `move.w <a variable from A5, or a number>,<the chip's register>`; a hook
    // there hands the interface what the instruction gives the board, and the game's instruction then runs as it was.
    // game.cfg names them (addresses, spaces between): video_palette_pages, where the game sets the pages the chip is
    // to take (CPS-B's palette control); video_palette_upload, where it gives the buffer (CPS-A's palette base: the
    // chip takes the palette then).
    struct Operand {
        bool variable = false;
        int16_t offset = 0; // (a variable: from A5)
        uint16_t value = 0; // (a number)
        uint16_t Of(const M68K &c) const {
            return variable ? Ram16(Variable(c, offset)) : value;
        }
    };
    std::map<uint32_t, Operand> g_pagesAt, g_uploadsAt;

    // The instruction at `at`, if it is `move.w d16(A5),xxx.l` or `move.w #n,xxx.l` to the register at `chip` (either
    // of the video chips' two places, 8001xx and 8041xx)
    bool ReadMove(const char *key, uint32_t at, uint32_t chip, Operand &operand) {
        const uint16_t op = OpcodeWord(at);
        uint32_t to = 0;
        if (op == 0x33ED) { // move.w d16(A5),xxx.l
            operand.variable = true;
            operand.offset = static_cast<int16_t>(OpcodeWord(at + 2));
            to = OpcodeLong(at + 4);
        } else if (op == 0x33FC) { // move.w #n,xxx.l
            operand.value = OpcodeWord(at + 2);
            to = OpcodeLong(at + 4);
        }
        if ((to & 0xFFBFFF) != chip) {
            fprintf(stderr, "video: game.cfg's %s: the instruction at %06X is not a move.w of a variable or a number to %06X; "
                            "not native\n", key, at, chip);
            return false;
        }
        return true;
    }

    bool PagesSet(M68K &c) {
        c.nativeVideo->gamePages = static_cast<uint8_t>(g_pagesAt[c.pc].Of(c));
        return false;
    }
    bool PaletteGiven(M68K &c) {
        const uint16_t base = g_uploadsAt[c.pc].Of(c);
        c.nativeVideo->Palette(native_video::GamePalette(c.nativeVideo->videoRam, BufferAt(base), c.nativeVideo->gamePages));
        return false;
    }

    void AddMoves(const char *key, const char *list, uint32_t chip, std::map<uint32_t, Operand> &into, HookFn fn, const char *name) {
        for (const char *p = list; *p;) {
            char *end = nullptr;
            const uint32_t at = static_cast<uint32_t>(strtoul(p, &end, 16));
            if (end == p) {
                p++; // (spaces, and the "||" a key given twice is joined with)
                continue;
            }
            p = end;
            Operand operand;
            if (ReadMove(key, at, chip, operand)) {
                into[at] = operand;
                AddHook(at, name, fn);
            }
        }
    }
} // namespace

// (a game.cfg without one of the keys: none of that part)
#ifndef GAME_VIDEO_PALETTE_COMMIT
#define GAME_VIDEO_PALETTE_COMMIT ""
#endif
#ifndef GAME_VIDEO_PALETTE_PAGES
#define GAME_VIDEO_PALETTE_PAGES ""
#endif
#ifndef GAME_VIDEO_PALETTE_UPLOAD
#define GAME_VIDEO_PALETTE_UPLOAD ""
#endif

void RegisterVideoLifts() {
    if (*GAME_VIDEO_PALETTE_COMMIT) {
        AddPaletteCommit(GAME_VIDEO_PALETTE_COMMIT);
    }
    AddMoves("video_palette_pages", GAME_VIDEO_PALETTE_PAGES, 0x800170, g_pagesAt, PagesSet, "PalettePages");
    AddMoves("video_palette_upload", GAME_VIDEO_PALETTE_UPLOAD, 0x80010A, g_uploadsAt, PaletteGiven, "PaletteUpload");
}

} // namespace cps2recomp
