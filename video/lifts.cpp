// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// Capcom's video library as the games share it, written once (docs/NATIVE_VIDEO.md): the routines that hand the
// board the picture, rewritten (docs/READABLE.md) so that they also hand it to the native interface (native.h).
//
// A game says in its game.cfg where its copy of each part is. A routine rewritten whole: the address, then a CRC-32 of
// the routine's instruction words with their operands left out (`video_palette_commit = 00123E:XXXXXXXX`); its
// operands (where the game keeps its variables, which of the board's addresses it writes) are read from the game's
// own copy, so that one rewrite serves every game with that version. A copy that is not that version is not
// rewritten (its own code runs) and is reported. Elsewhere, the instructions that write the video's registers
// (`video_writes = ADDRESS ...`): a hook at each says what it is about to write, and the instruction then runs as it
// was.
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
        // (the native interface is told each write first: the upload is made from the game's buffer and pages)
        c.nativeVideo->GameWrites(c, g_commit.pagesRegister, pages, false);
        c.Write(g_commit.pagesRegister, 1, pages);
        c.nativeVideo->GameWrites(c, g_commit.baseRegister, base, false);
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
    // --- The video's registers written elsewhere: every other family's commit, every game's start-up, the object
    // bank's swaps. Each is a `move.b` or `move.w` (from a data register, an address register's place, a variable or a
    // number; to a fixed address or an address register's place). The hook works out, from the CPU as the instruction
    // finds it, the value and the place, and tells the native interface when the place is a video register.
    struct Operand {
        unsigned mode = 0, reg = 0;
        uint32_t extension = 0; // (where its extension words start)
    };
    struct Move {
        bool byte = false;
        Operand from, to;
    };
    std::map<uint32_t, Move> g_moves;

    unsigned ExtensionWords(const Operand &o, bool byte) {
        switch (o.mode) {
        case 5: return 1; // d16(An)
        case 7:
            switch (o.reg) {
            case 0: return 1; // xxx.w
            case 1: return 2; // xxx.l
            case 2: return 1; // d16(PC)
            case 4: return 1; // #n (a byte's in the low half of a word)
            default: return 0;
            }
        default: return 0;
        }
    }
    bool Supported(const Operand &o, bool source) {
        if (o.mode == 6) {
            return false; // (an index: none of the games' writers so far)
        }
        if (o.mode == 7) {
            return o.reg == 0 || o.reg == 1 || (source && (o.reg == 2 || o.reg == 4));
        }
        return source || o.mode >= 2; // (a register is no destination here)
    }
    // Where an operand of mode 7 is (none of the CPU's registers needed)
    uint32_t FixedPlace(const Operand &o) {
        switch (o.reg) {
        case 0: return static_cast<uint32_t>(static_cast<int16_t>(OpcodeWord(o.extension)));
        case 1: return OpcodeLong(o.extension);
        case 2: return o.extension + static_cast<uint32_t>(static_cast<int16_t>(OpcodeWord(o.extension)));
        default: return 0;
        }
    }
    // Where an operand is in memory, as the CPU is before the instruction
    uint32_t Place(const M68K &c, const Operand &o, bool byte) {
        switch (o.mode) {
        case 2: case 3: return c.a[o.reg];
        case 4: return c.a[o.reg] - ((byte && o.reg != 7) ? 1 : 2);
        case 5: return c.a[o.reg] + static_cast<uint32_t>(static_cast<int16_t>(OpcodeWord(o.extension)));
        case 7: return FixedPlace(o);
        default: return 0;
        }
    }
    uint16_t Value(M68K &c, const Operand &o, bool byte) {
        if (o.mode == 0) {
            return static_cast<uint16_t>(byte ? c.d[o.reg] & 0xFF : c.d[o.reg] & 0xFFFF);
        }
        if (o.mode == 1) {
            return static_cast<uint16_t>(c.a[o.reg]);
        }
        if (o.mode == 7 && o.reg == 4) {
            const uint16_t n = OpcodeWord(o.extension);
            return byte ? n & 0xFF : n;
        }
        const uint32_t at = Place(c, o, byte) & 0xFFFFFF;
        return byte ? c.bus.Read8(at) : c.bus.Read16(at & ~1u);
    }

    bool VideoWrite(M68K &c) {
        const Move &move = g_moves[c.pc];
        const uint32_t to = Place(c, move.to, move.byte) & 0xFFFFFF;
        if (native_video::IsVideoRegister(to)) {
            c.nativeVideo->GameWrites(c, to, Value(c, move.from, move.byte), move.byte);
        }
        return false;
    }

    void AddWrites(const char *list) {
        for (const char *p = list; *p;) {
            char *end = nullptr;
            const uint32_t at = static_cast<uint32_t>(strtoul(p, &end, 16));
            if (end == p) {
                p++; // (spaces, and the "||" a key given twice is joined with)
                continue;
            }
            p = end;
            const uint16_t op = OpcodeWord(at);
            Move move;
            move.byte = (op >> 12) == 1;
            move.from = {static_cast<unsigned>((op >> 3) & 7), static_cast<unsigned>(op & 7), at + 2};
            move.to = {static_cast<unsigned>((op >> 6) & 7), static_cast<unsigned>((op >> 9) & 7), 0};
            move.to.extension = move.from.extension + 2 * ExtensionWords(move.from, move.byte);
            const bool fixed = move.to.mode == 7;
            if (((op >> 12) != 1 && (op >> 12) != 3) || !Supported(move.from, true) || !Supported(move.to, false) ||
                (fixed && !native_video::IsVideoRegister(FixedPlace(move.to)))) {
                fprintf(stderr, "video: game.cfg's video_writes: the instruction at %06X is not a move to the video's registers the "
                                "toolkit reads; not native\n", at);
                continue;
            }
            g_moves[at] = move;
            AddWatch(at, "VideoWrite", VideoWrite);
        }
    }
} // namespace

// (a game.cfg without one of the keys: none of that part)
#ifndef GAME_VIDEO_PALETTE_COMMIT
#define GAME_VIDEO_PALETTE_COMMIT ""
#endif
#ifndef GAME_VIDEO_WRITES
#define GAME_VIDEO_WRITES ""
#endif

void RegisterVideoLifts() {
    if (*GAME_VIDEO_PALETTE_COMMIT) {
        AddPaletteCommit(GAME_VIDEO_PALETTE_COMMIT);
    }
    AddWrites(GAME_VIDEO_WRITES);
}

} // namespace cps2recomp
