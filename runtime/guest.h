// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
//
// A board of another game, run beside the game's and never seen as it is (game.cfg's guest_set): for a game whose
// sources show something of that game in their own (its stages behind their fighters, say). It is a whole machine of
// its own, with that game's ROM set: its 68000 runs on the interpreter (the translated code is the game's), with hooks
// of its own at that game's addresses, and its sound side as the board's. What the game's sources take of it (its
// picture, its sound) is theirs to put together; its state is theirs to keep with their own (online play runs frames
// again from a save state, and the guest must come back with them).
//
// The set is optional: a game with a guest_set plays without it, and its sources leave out what needs it.
#pragma once

#include "../machine/machine.h"
#include "../machine/romset.h"

#include <cstdint>
#include <memory>
#include <string>

namespace cps2recomp {

class Archive;

// The port loads the set (game.cfg's guest_ lines), from --rom3 or beside --rom (false, and `error` why, when it is not
// there)
bool LoadGuest(const std::string &path, const RomList &list, std::string &error);
// The guest's machine (nullptr: no set)
Machine *Guest();
// Another of the same (nullptr: no set), made when first asked for: for the game's sources to run ahead of what they
// need (a guest's board taken somewhere and kept, before the game is there), outside the game's state; it may be run on
// a thread of their own (it shares nothing with the guest but the ROM set and the hooks, which do not change once the
// game runs, and it is never freed)
Machine *GuestSpare();
// A hook in the guest's code (as a game's own: runtime/lift.h), at that game's address (before the game runs: the hooks
// are not changed after)
void AddGuestHook(uint32_t address, bool (*fn)(M68K &c));

} // namespace cps2recomp
