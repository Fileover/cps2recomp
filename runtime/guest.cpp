// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// The guest board (guest.h).
#include "guest.h"

#include <map>
#include <vector>

namespace cps2recomp {

namespace {
    // (never freed: a spare may run on a thread of the game's sources' own until the program is gone)
    RomSet &g_roms = *new RomSet;
    std::unique_ptr<Machine> g_machine;
    Machine *g_spare = nullptr;
    std::vector<uint8_t> &g_hooked = *new std::vector<uint8_t>; // a byte a word of the guest's ROM: a hook there
    std::map<uint32_t, bool (*)(M68K &c)> &g_hooks = *new std::map<uint32_t, bool (*)(M68K &c)>;

    bool RunGuestHook(M68K &c, uint32_t address) {
        const auto found = g_hooks.find(address);
        if (found == g_hooks.end()) {
            return false;
        }
        c.pc = address;
        return found->second(c);
    }

    std::unique_ptr<Machine> MakeMachine() {
        auto made = std::make_unique<Machine>(g_roms);
        Machine &m = *made;
        m.cpuMode = M68K::Mode::Interpreter;
        // (the sound program as ordinary code where there is one for it, as the port's own runs; else the Z80's own code)
        m.soundSide = m.soundDriver ? Machine::SoundSide::Driver : Machine::SoundSide::Z80;
        m.cpu.ownHooked = g_hooked.data();
        m.cpu.ownHookedSize = static_cast<uint32_t>(g_roms.program.size());
        m.cpu.ownHook = RunGuestHook;
        m.Reset();
        return made;
    }
} // namespace

bool LoadGuest(const std::string &path, const RomList &list, std::string &error) {
    if (!LoadRomSet(path, list, g_roms, error) && !LoadRomSet(path + ".zip", list, g_roms, error)) {
        return false;
    }
    g_hooked.assign(g_roms.program.size() / 2 + 1, 0);
    for (const auto &[address, fn] : g_hooks) {
        if (address / 2 < g_hooked.size()) {
            g_hooked[address / 2] = 1;
        }
    }
    g_machine = MakeMachine();
    return true;
}

Machine *Guest() {
    return g_machine.get();
}

Machine *GuestSpare() {
    if (g_machine && !g_spare) {
        g_spare = MakeMachine().release(); // (never freed: as above)
    }
    return g_spare;
}

void AddGuestHook(uint32_t address, bool (*fn)(M68K &c)) {
    g_hooks[address] = fn;
    if (address / 2 < g_hooked.size()) {
        g_hooked[address / 2] = 1;
    }
}

} // namespace cps2recomp
