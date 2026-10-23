// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Vampire Savior: a spike for tagging in during a fight (not a feature yet). VSAV_SWAP=FRAME:PLAYER:CHARACTER[:QUIET],...
// (hex character) has the player's record given the character at that frame and its step put back to 0, so that the
// game's own setting up of a player (0222C8) makes it that character; QUIET sets the player's bit at A5+102 first,
// which skips the entrance.
#include "machine.h"
#include "vsav.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cps2recomp {

namespace {
    // The pools the game takes objects from (016C4E on: each a free list from A5, pushed to on release, 017028 on):
    // where the objects are, how many and how big, the tag at +8 a free one has, the list's pointer and count
    struct Pool {
        uint32_t first;
        int count;
        uint32_t size;
        uint8_t tag;
        int32_t list, left;
        uint32_t extra; // a byte set on release (+3C FF, +0D 1), or 0
        uint8_t extraValue;
    };
    constexpr Pool kPools[] = {
        {0xFF9400, 32, 0x100, 0x04, 0x7966, 0x79BE, 0x3C, 0xFF}, {0xFFB400, 8, 0x80, 0x08, 0x796A, 0x79BF, 0, 0},
        {0xFFB800, 32, 0x80, 0x0C, 0x796E, 0x79C0, 0x3C, 0xFF}, {0xFFC800, 24, 0x80, 0x10, 0x7972, 0x79C1, 0x0D, 1},
        {0xFFD400, 32, 0x80, 0x14, 0x7976, 0x79C2, 0, 0},       {0xFFE400, 16, 0x40, 0x18, 0x797A, 0x79C3, 0, 0},
        {0xFFE800, 6, 0x200, 0x1C, 0x797E, 0x79C4, 0, 0},
    };
    constexpr uint32_t kOwner = 0x30; // word: the record of whoever made the object
    constexpr uint32_t kLinks = 0x28; // four words of a player's record: objects of its own (01ADF6)

    // Every object `owner` made (and everything they made), given back to its pool as the game gives one back
    int ReleaseOwned(uint32_t owner) {
        int released = 0;
        for (const Pool &pool : kPools) {
            for (int n = 0; n < pool.count; n++) {
                const uint32_t object = pool.first + static_cast<uint32_t>(n) * pool.size;
                if (!Ram8(object + vsav::kActive) || Ram16(object + kOwner) != (owner & 0xFFFF) || object == owner) {
                    continue; // (free already, or not the owner's)
                }
                for (uint32_t at = 0; at < 0x10; at += 4) {
                    SetRam32(object + at, 0);
                }
                SetRam8(object + 0x08, pool.tag);
                if (pool.extra) {
                    SetRam8(object + pool.extra, pool.extraValue);
                }
                const uint32_t list = Ram32(0xFF8000 + static_cast<uint32_t>(pool.list)) - 2;
                SetRam16(list, static_cast<uint16_t>(object));
                SetRam32(0xFF8000 + static_cast<uint32_t>(pool.list), list);
                SetRam8(0xFF8000 + static_cast<uint32_t>(pool.left), static_cast<uint8_t>(Ram8(0xFF8000 + static_cast<uint32_t>(pool.left)) + 1));
                released += 1 + ReleaseOwned(object);
            }
        }
        return released;
    }
} // namespace

void LiveTagBeforeFrame(Machine &machine) {
    using namespace vsav;
    static const char *swap = getenv("VSAV_SWAP");
    static int frame = 0;
    if (!swap) {
        return;
    }
    frame++;
    // (the frame after: the player set up again by then, its set's palettes put in as at a round's start, 02ADAC)
    static int colour = -1;
    if (colour >= 0) {
        // (as 02ADAC does: the set's first palette, and its other three unless the character's bit says not, into the
        // buffer the board shows from at the player's slot, each colour keeping the brightness the buffer had)
        M68K &c = machine.cpu;
        const uint32_t record = Player(colour);
        const int lines = (kFirstLineOnly >> (Ram8(record + kCharacter) & 31)) & 1 ? 1 : kPalettesInSet;
        uint32_t from = Ram32(record + kColourSet);
        uint32_t to = Ram32(kPaletteBuffer) + static_cast<uint32_t>(Ram8(record + kPaletteSlot)) * kColoursInPalette * 2;
        for (int n = 0; n < lines * kColoursInPalette; n++, from += 2, to += 2) {
            const uint16_t bright = static_cast<uint16_t>(c.Read(to, 1) & 0xF000);
            c.Write(to, 1, static_cast<uint16_t>(Mem16(c, from) | bright));
        }
        colour = -1;
    }
    // (several, comma between; each done at its frame or, with VSAV_SWAP_FREE, as soon after as the player can act)
    static const bool free = getenv("VSAV_SWAP_FREE") != nullptr;
    static int done = 0; // (the entries done, a bit each)
    int at = 0, player = 0, character = 0, quiet = 0;
    bool now = false;
    int index = 0;
    for (const char *entry = swap; entry; entry = strchr(entry, ',') ? strchr(entry, ',') + 1 : nullptr, index++) {
        quiet = 0;
        if (sscanf(entry, "%d:%d:%x:%d", &at, &player, &character, &quiet) < 3 || (done >> index) & 1) {
            continue;
        }
        if (free ? frame >= at && CanAct(player) && !Ram8(Player(player) + kHitStop) && Ram16(kGameStep) == kGameMatch : frame == at) {
            done |= 1 << index;
            now = true;
            break;
        }
    }
    if (!now) {
        return;
    }
    const uint32_t record = Player(player);
    const uint16_t health = Ram16(record + kHealth), ceiling = Ram16(record + kHealthCeiling);
    fprintf(stderr, "swap at %d: player %d %02X -> %02X (health %04X/%04X, state %08X)\n", frame, player, Ram8(record + kCharacter),
            character, health, ceiling, Ram32(record + kState));
    const int released = ReleaseOwned(record);
    for (uint32_t at = kLinks; at < kLinks + 8; at += 2) {
        SetRam16(record + at, 0);
    }
    fprintf(stderr, "  released %d objects\n", released);
    SetRam8(record + kCharacter, static_cast<uint8_t>(character));
    SetRam8(record + kCharacterAgain, static_cast<uint8_t>(character));
    if (quiet) {
        SetRam8(0xFF8102, static_cast<uint8_t>(Ram8(0xFF8102) | (1u << Ram8(record + 0x381))));
    }
    // (the colours: the set of the character's sets that the player chose, kColourSet above what setting up clears)
    const uint8_t *p = Program(kColourSets + 4u * static_cast<uint32_t>(character));
    const uint32_t sets = (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | (static_cast<uint32_t>(p[2]) << 8) | p[3];
    SetRam32(record + kColourSet, sets + static_cast<uint32_t>(Ram8(record + kColours)) * kColourSetSize);
    SetRam32(record + kState, 0);
    colour = player;
}

} // namespace cps2recomp
