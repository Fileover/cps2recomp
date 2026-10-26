#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
"""Prints the computer player's scripts as text: the Vampire Savior games' and Super Gem Fighter's computer is an
interpreter running scripts from the ROM, a set for each character (games/vampire_savior/PROGRESS.md, "The
computer player"). Read from a build's work/<id>/ (program.lst, program.bin, program.bin.data), run from the
repository's top.

  aiscripts.py [GAME]                     every character's scripts (GAME: vampire_savior)
  aiscripts.py GAME --character NAME|N    one character's
  aiscripts.py GAME --check               decode everything and say what did not fit; prints nothing else
  aiscripts.py GAME --against FILE        check a run's --watch of a computer's script pointer (+220, a long)
                                          against the instructions found: every place the game's interpreter
                                          kept there must be one of theirs, and every step from one to the next
                                          one the scripts allow (CPS2RECOMP_WATCH_REPORTS=N for more than the
                                          first 2,000 writes)

The instructions' lengths are the game's own: its routine that skips over instructions (an "if" whose test
fails) has a table of them, read here from the program. Where the instructions are the ones this was written
from (Vampire Savior's), they are named; an instruction of another kind is printed as its words.

Each character has four banks of scripts: 0 its plans (one chosen by the game), 1 its tactics (one at random
from 32 slots for its plan, in the row of its tier: 0, 20 or 40 by the difficulty), 2 its reactions to an attack or a
projectile (likewise at random), 3 more reactions (one chosen by the game). A script is a word (what happens
when it is broken off) and instructions, two bytes each (an operation and a kind of it) and words after, to
the end (FF). Distances are the gap between the two fighters (+130); a move is a word: a normal (bit 0
clear: stance, punch or kick, strength, direction, variant) or a special (bit 0 set: its number, strength).
"""
import argparse
import os
import re
import struct
import sys

ROOT = os.getcwd() if os.path.isdir("games") else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


class Program:
    def __init__(self, game):
        work = os.path.join(ROOT, "work", game)
        self.code = open(os.path.join(work, "program.bin"), "rb").read()      # as the 68000 fetches (and PC-relative)
        self.data = open(os.path.join(work, "program.bin.data"), "rb").read()  # as it reads data
        self.lines = []
        for line in open(os.path.join(work, "program.lst")).read().split("\n"):
            m = re.match(r"^[> ]([0-9A-F]{6})  (.*)$", line)
            if m:
                self.lines.append((int(m.group(1), 16), m.group(2)))

    def cw(self, a):  # a signed word of the program's code space
        return struct.unpack(">h", self.code[a:a + 2])[0]

    def cuw(self, a):
        return struct.unpack(">H", self.code[a:a + 2])[0]

    def dw(self, a):  # a word of its data
        return struct.unpack(">H", self.data[a:a + 2])[0]

    def dsw(self, a):  # a signed word of its data (offsets: lea 0(a0,d1.w))
        return struct.unpack(">h", self.data[a:a + 2])[0]

    def dl(self, a):
        return struct.unpack(">I", self.data[a:a + 4])[0]


# --- the instructions' lengths: the game's skipping routine ---

def skip_lengths(p):
    """{(op, kind): operand words} and {test: operand words} for "if", from the routine that skips over
    instructions: move.b (a0)+,d0 / bmi / move.w T(pc,d0.w),d1 / jmp T(pc,d1.w) (the operation's table)."""
    table = None
    for i, (a, ins) in enumerate(p.lines):
        if ins == "move.b (a0)+,d0" and i + 2 < len(p.lines) and p.lines[i + 1][1].startswith("bmi ") and \
                re.match(r"move\.w \$[0-9A-F]+\(pc,d0\.w\),d1", p.lines[i + 2][1]):
            table = int(re.search(r"\$([0-9A-F]+)", p.lines[i + 2][1]).group(1), 16)
            break
    if table is None:
        sys.exit("aiscripts: the routine that skips over instructions is not in this program's listing")

    def length_at(a):  # a routine that steps over operands: lea N(a0),a0; rts or rts
        if p.cuw(a) == 0x41E8 and p.cuw(a + 4) == 0x4E75:
            return p.cuw(a + 2) // 2
        if p.cuw(a) == 0x4E75:
            return 0
        return None

    def sub_table(t, end):  # word offsets from t to routines, up to `end` (where the code goes on)
        out = []
        a = t
        while a < end:
            n = length_at(t + p.cw(a))
            if n is None:
                break
            out.append(n)
            a += 2
        return out

    lengths, tests = {}, {}
    handlers = []
    for op in range(0, 0x40, 2):
        h = table + p.cw(table + op)
        if not (table < h < table + 0x400) or p.cuw(h) != 0x1018:  # move.b (a0)+,d0: the kind
            break
        handlers.append((op, h))
    starts = sorted(h for _, h in handlers)
    for op, h in handlers:
        later = [s for s in starts if s > h]
        if p.cuw(h + 2) == 0x6100:  # bsr: every kind the same
            n = length_at(h + 4 + p.cw(h + 4))
            lengths[(op, None)] = n
        elif p.cuw(h + 2) == 0x6000:  # bra back: no operands
            lengths[(op, None)] = 0
        elif p.cuw(h + 2) == 0x323B and p.cuw(h + 6) == 0x4EBB:  # a table of kinds, jsr
            t = h + 4 + p.cuw(h + 4)
            for k, n in enumerate(sub_table(t, later[0] if later else t + 0x40)):
                lengths[(op, 2 * k)] = n
        elif p.cuw(h + 2) == 0x323B and p.cuw(h + 6) == 0x4EFB:  # "if": its kinds by a jmp table
            t = h + 4 + p.cuw(h + 4)
            kinds = [t + p.cw(t + 2 * k) for k in range(3)]
            lengths[(op, 0)] = lengths[(op, 2)] = lengths[(op, 4)] = 0  # (the test's own word: below)
            ifs = kinds[0]  # addq.b #1,nesting / move.w (a0)+,d0 / move.w T(pc,d0.w),d1 / jsr
            tt = ifs + 8 + p.cuw(ifs + 8)
            for k, n in enumerate(sub_table(tt, kinds[1])):
                tests[2 * k] = n
            lengths["if"] = op
    return lengths, tests


# --- the banks of scripts: the routines that start one ---

def bank_tables(p):
    """The four tables of the characters' scripts (by +382), from the routines that start a script of each bank:
    movea.l #TABLE,a0 ... move.l a0,0x210(a6) (0x214, 0x218, 0x21C)."""
    banks = {}
    for i, (a, ins) in enumerate(p.lines):
        m = re.match(r"move\.l a0,0x(21[048C])\(a6\)$", ins)
        if not m:
            continue
        for back in range(1, 24):
            mm = re.match(r"movea\.l #\$([0-9A-F]+),a0$", p.lines[i - back][1])
            if mm:
                bank = (int(m.group(1), 16) - 0x210) // 4
                banks.setdefault(bank, int(mm.group(1), 16))
                break
            if p.lines[i - back][1].startswith("rts"):
                break
    if sorted(banks) != [0, 1, 2, 3]:
        sys.exit("aiscripts: the banks' tables were not all found (%s)" % banks)
    return banks


def character_names(game):
    """The characters' names by number: a family's roster ({0xNN, "Name"} in family.h), or a game's list
    (kCharacterNames[...] = {"Name", ...} in its src/*.h)"""
    names = {}
    folder = os.path.join(ROOT, "games", game)
    family = os.path.join(folder, "family.h")
    if os.path.exists(family):
        for m in re.finditer(r'\{0x([0-9A-Fa-f]+), "([^"]+)"', open(family).read()):
            names.setdefault(int(m.group(1), 16), m.group(2))
    src = os.path.join(folder, "src")
    for f in sorted(os.listdir(src)) if not names and os.path.isdir(src) else []:
        m = re.search(r"kCharacterNames\[[^\]]*\] = \{([^}]*)\}", open(os.path.join(src, f)).read())
        if m:
            for n, name in enumerate(re.findall(r'"([^"]*)"', m.group(1))):
                names[n] = name
    return names


# --- naming (Vampire Savior's instructions) ---

STRENGTH = {0: "light", 2: "medium", 4: "heavy", 6: "strength 6"}


def move(w):
    if w & 1:
        text = "special %d %s" % ((w & 0x1E) >> 1, STRENGTH.get((w >> 4) & 6, "?"))
        variant = (w >> 8) & 0xF
    else:
        # (as the game unpacks it, 02CE44: the stance, punch or kick, the strength, in twos from bit 1)
        stance = {0: "standing", 2: "crouching", 4: "jumping", 6: "stance 6"}[w & 6]
        button = {0: "punch", 2: "kick"}.get((w >> 2) & 6, "button %d" % ((w >> 2) & 6))
        text = "%s %s %s" % (stance, STRENGTH[(w >> 4) & 6], button)
        held = (w >> 8) & 0xF
        if held:
            text += " holding %X" % held
        variant = (w >> 12) & 0xF
    if variant:
        text += ", variant %d" % variant
    return text


def way(w):  # a jump's or dash's word: the way (high byte: 0 forward, 1 back, FF up) and its kind (low)
    return {0: "forward", 1: "back", 0xFF: "straight up"}.get(w >> 8, "way %02X" % (w >> 8))


def with_move(w, m, frames):
    """A jump's or dash's move, by its kind (the way word's low byte, a byte offset into a table: 0 and 2 start
    no timer, 4 starts the move after so many frames; bit 0 is Felicia's alone)"""
    kind = w & 0xFF
    if kind & 0xFE == 4:
        return ": %s after %d frames" % (move(m), frames)
    return "" if not m and not frames else " (kind %d: %s, %d)" % (kind, move(m), frames)


TESTS = {
    0x00: "gap <= {0}", 0x02: "gap >= {0}", 0x04: "one time in 2", 0x06: "a gauge stock",
    0x08: "the other in the air", 0x0A: "the other crouching", 0x0C: "the other lying down",
    0x0E: "the other's projectile out", 0x10: "the other's projectile out ({0})",
    0x12: "the other's projectile out ({0})", 0x14: "an attack coming",
    0x16: "the other jumping back", 0x18: "the other jumping forward", 0x1A: "the other jumping straight up",
    0x1C: "the other open (on the floor, in a move)", 0x1E: "level <= {0}", 0x20: "level >= {0}",
    0x22: "no projectile of mine out", 0x24: "the other's health below {0}", 0x26: "my health above the other's",
    0x28: "the other in the air, falling", 0x2A: "the other in the air, rising", 0x2C: "the other below {0} up",
    0x2E: "the other above {0} up", 0x30: "the other's projectile out", 0x32: "none of the other's projectiles out",
    0x34: "my health below {0}", 0x36: "three times in 4", 0x38: "the other is character {0}",
    0x3A: "I am character {0}", 0x3C: "+39 set", 0x3E: "one time in 16 by the bits {0:04X}",
    0x40: "the other's projectile {0} out", 0x42: "gauge stocks >= {0}",
}


def name(op, kind, args, names):
    a = args + [0] * 5
    n = {
        (0x00, 0x00): "stand %d frames" % a[0],
        (0x00, 0x02): "crouch",
        (0x00, 0x04): "stand up",
        (0x00, 0x06): "wait while the other is in the air",
        (0x00, 0x08): "wait while the other is in the air, above %d" % a[0],
        (0x00, 0x0A): "wait while my projectile %d is out" % a[0],
        (0x00, 0x0C): "(nothing)",
        (0x00, 0x0E): "wait while the other's projectile is out (%d)" % a[0],
        (0x00, 0x10): "think (a time by level and character)",
        (0x00, 0x12): "wait while my projectile is out",
        (0x00, 0x14): "wait until the other is open",
        (0x00, 0x16): "think (a time by level and character), not broken off",
        (0x00, 0x18): "wait a frame",
        (0x00, 0x1A): "wait %d frames, not broken off" % a[0],
        (0x00, 0x1C): "wait until gap <= %d, at most %d frames" % (a[0], a[1]),
        (0x00, 0x1E): "wait until gap >= %d, at most %d frames" % (a[0], a[1]),
        (0x00, 0x20): "wait until the other is open (not thrown)",
        (0x00, 0x22): "wait (+23F, with a gauge stock)",
        (0x00, 0x24): "stand",
        (0x02, 0x00): "walk forward until gap <= %d" % a[0],
        (0x02, 0x02): "walk back until gap >= %d" % a[0],
        (0x02, 0x04): "walk to gap %d" % a[0],
        (0x02, 0x06): "walk forward %d frames" % a[0],
        (0x02, 0x08): "walk back %d frames" % a[0],
        (0x02, 0x0A): "walk forward until gap <= %d, at most %d frames" % (a[0], a[1]),
        (0x02, 0x0C): "walk back until gap >= %d, at most %d frames" % (a[0], a[1]),
        (0x02, 0x0E): "walk to gap %d, at most %d frames" % (a[0], a[1]),
        (0x04, 0x00): "jump %s%s" % (way(a[1]), with_move(a[1], a[0], a[2])),
        (0x04, 0x02): "jump %s (%04X), then jump %s%s" % (way(a[0]), a[1], way(a[3]), with_move(a[3], a[2], a[4])),
        (0x04, 0x04): "jump %s%s (+23C %d)" % (way(a[1]), with_move(a[1], a[0], a[2]), a[3]),
        (0x04, 0x06): "jump %s, the other way%s" % (way(a[1]), with_move(a[1], a[0], a[2])),
        (0x04, 0x08): "jump %s into special 5%s" % (way(a[1]), with_move(a[1], a[0], a[2])),
        (0x04, 0x0A): "jump %s into chain %d%s" % (way(a[1]), a[0], with_move(a[1], 0, a[2]) and " (%d frames)" % a[2]),
        (0x06, 0x00): "attack: %s" % move(a[0]),
        (0x06, 0x02): "attack, held: %s" % move(a[0]),
        (0x08, 0x00): "special move: %s" % move(a[0]),
        (0x0A, 0x00): "gauge move (one stock): %s" % move(a[0]),
        (0x0E, 0x00): "go to a tactic (bank 1), then back here",
        (0x0E, 0x02): "go to a tactic (bank 1) by the table 0BC780's chance, then back here",
        (0x10, 0x00): "guard if an attack is coming (by the character's chance)",
        (0x10, 0x02): "guard if an attack is coming, then: %s" % move(a[0]),
        (0x12, 0x00): "above level %d: let the reactions %04X break in" % (a[0], a[1]),
        (0x12, 0x02): "above level %d: keep the reactions %04X out" % (a[0], a[1]),
        (0x12, 0x04): "can be broken off",
        (0x12, 0x06): "cannot be broken off",
        (0x14, 0x00): "gauge move, kind 12 (one stock): %s" % move(a[0]),
        (0x16, 0x00): "dash %s%s" % (way(a[1]), with_move(a[1], a[0], a[2])),
        (0x18, 0x00): "chain %d" % a[0],
        (0x1A, 0x00): "(stop)",
    }.get((op, kind))
    if n is None:
        n = "op %02X kind %02X %s" % (op, kind, " ".join("%04X" % x for x in args))
    return n


ENDS = {0x00: "end: a tactic (bank 1) at random", 0x02: "end: back to the plan", 0x04: "end: a tactic, or back to the plan, by the table 0BB480's chance"}


# --- reading the scripts ---

class Decoder:
    def __init__(self, p, lengths, tests, names):
        self.p, self.lengths, self.tests, self.names = p, lengths, tests, names
        self.ifop = lengths.get("if")
        self.problems = []
        self.boundaries = set()
        self.ends = set()
        self.printed = {}  # a script's place: what it was first printed as
        self.following = {}  # an instruction's place: the next one's
        self.firsts = set()  # scripts' first instructions
        self.plans = set()  # the plans' instructions (a plan's place is kept as it runs, and gone back to)
        self.in_plan = False

    def instruction_length(self, op, kind):
        if (op, kind) in self.lengths:
            return self.lengths[(op, kind)]
        return self.lengths.get((op, None))

    def script(self, at, limit, label):
        """The script at `at` (its first word, then its instructions), as lines; `limit`: where the bank's data
        ends (a script that runs on past it is a problem)."""
        p = self.p
        self.firsts.add(at + 2)
        if at in self.printed:
            return ["    the same as %s" % self.printed[at]], at
        self.printed[at] = label
        lines = ["    header %04X" % p.dw(at)]
        a = at + 2
        self.firsts.add(a)
        depth = 0
        previous = None
        while a < limit:
            self.boundaries.add(a)
            if previous is not None:
                self.following[previous] = a
            previous = a
            if self.in_plan:
                self.plans.add(a)
            op, kind = p.data[a], p.data[a + 1]
            if op == 0xFF:
                lines.append("    %06X  [FF %02X] %s%s" % (a, kind, "  " * depth, ENDS.get(kind, "end, kind %02X" % kind)))
                self.ends.add(a)
                if depth:
                    self.problems.append("%s: ends inside an if (%06X)" % (label, a))
                return lines, a + 2
            if op == self.ifop:
                if kind == 0:
                    test = p.dw(a + 2)
                    n = self.tests.get(test)
                    if n is None:
                        self.problems.append("%s: an if of an unknown test %02X (%06X)" % (label, test, a))
                        return lines, a
                    args = [p.dw(a + 4 + 2 * i) for i in range(n)]
                    text = TESTS.get(test, "test %02X" % test).format(*args) if n else TESTS.get(test, "test %02X" % test)
                    if test in (0x38, 0x3A) and args:
                        text = text.replace("character %d" % args[0], self.names.get(args[0], "character %d" % args[0]))
                    lines.append("    %06X  [%02X %02X] %sif %s" % (a, op, test, "  " * depth, text))
                    depth += 1
                    a += 4 + 2 * n
                    continue
                if kind == 2:
                    lines.append("    %06X  [%02X 02] %selse" % (a, op, "  " * max(depth - 1, 0)))
                    a += 2
                    continue
                if kind == 4:
                    depth = max(depth - 1, 0)
                    lines.append("    %06X  [%02X 04] %sendif" % (a, op, "  " * depth))
                    a += 2
                    continue
            n = self.instruction_length(op, kind)
            if n is None:
                self.problems.append("%s: an instruction not in the game's table, %02X %02X (%06X)" % (label, op, kind, a))
                return lines, a
            args = [p.dw(a + 2 + 2 * i) for i in range(n)]
            lines.append("    %06X  [%02X %02X] %s%s" % (a, op, kind, "  " * depth, name(op, kind, args, self.names)))
            a += 2 + 2 * n
        self.problems.append("%s: runs past its bank's data (%06X)" % (label, a))
        return lines, a


def offsets(p, base):
    """A table of signed word offsets from `base` (to scripts before it too), which runs up to where the nearest
    of those after it points"""
    out, end = [], None
    while end is None or 2 * len(out) < end:
        o = p.dsw(base + 2 * len(out))
        if o == 0 or abs(o) >= 0x8000:
            break
        if o > 0:
            end = o if end is None else min(end, o)
        out.append(base + o)
        if len(out) > 256:
            break
    return out


def character(p, d, banks, who, out):
    name = d.names.get(who, "character %02X" % who)
    end = len(p.data)
    tables = [p.dl(banks[b] + 4 * who) for b in range(4)]
    if not all(0x400 <= t < end for t in tables):
        return False
    out.append("== %s (%02X)" % (name, who))
    # bank 0, the plans: by +205 (a byte offset into the word offsets)
    d.in_plan = True
    for k, s in enumerate(offsets(p, tables[0])):
        out.append("  plan %d" % k)
        lines, _ = d.script(s, end, "%s plan %d" % (name, k))
        out.extend(lines)
    d.in_plan = False
    # banks 1 and 2, the tactics (by the plan, +205) and the reactions: for each, 32 slots a tier (three rows), then
    # the scripts' offsets at +60
    for bank, what in ((1, "tactic"), (2, "reaction")):
        for g, sub in enumerate(offsets(p, tables[bank])):
            slots = list(p.data[sub:sub + 0x60])
            for k in range(max(slots) + 1):
                s = sub + p.dsw(sub + 0x60 + 2 * k)
                rows = "/".join("%d" % slots[r * 32:(r + 1) * 32].count(k) for r in range(3))
                out.append("  %s %d.%d (slots of 32 by tier 0/20/40: %s)" % (what, g, k, rows))
                lines, _ = d.script(s, end, "%s %s %d.%d" % (name, what, g, k))
                out.extend(lines)
    # bank 3: by +22B
    for k, s in enumerate(offsets(p, tables[3])):
        out.append("  reaction (by +22B) %d" % k)
        lines, _ = d.script(s, end, "%s reaction-b %d" % (name, k))
        out.extend(lines)
    return True


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("game", nargs="?", default="vampire_savior")
    ap.add_argument("--character")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--against")
    args = ap.parse_args()
    p = Program(args.game)
    lengths, tests = skip_lengths(p)
    banks = bank_tables(p)
    names = character_names(args.game)
    d = Decoder(p, lengths, tests, names)
    who = None
    if args.character:
        if re.match(r"^(0x)?[0-9A-Fa-f]+$", args.character):
            who = int(args.character, 16 if args.character.startswith("0x") else 10)
        else:
            found = [n for n, s in names.items() if s.lower() == args.character.lower()]
            if not found:
                sys.exit("aiscripts: no character %r (%s)" % (args.character, ", ".join(sorted(names.values()))))
            who = found[0]
    out = []
    seen = set()
    characters = min(32, (banks[1] - banks[0]) // 4)  # (the tables one after the other, a long a character)
    for n in range(characters):
        if who is not None and n != who:
            continue
        table = p.dl(banks[0] + 4 * n)
        if table in seen and who is None:
            continue  # (a character with another's scripts)
        seen.add(table)
        character(p, d, banks, n, out)
    if args.against:
        # The interpreter keeps in +220 the next instruction's place (after one is done) or the place just after
        # an instruction's first word (as it starts one); after an end, just past it. From one kept place to the
        # next it goes on to the following instruction, forward in the same script (an if or else skipping), to
        # the start of a script, or back into the plan where it was (the plan's place is kept as it runs).
        kept = places = 0
        wrong = {}
        moves = {"stayed": 0, "to the next instruction": 0, "forward in the script": 0, "to a script's start": 0,
                 "back to the plan": 0, "elsewhere": 0}
        returns = d.plans
        previous = None
        for line in open(args.against):
            m = re.match(r"watch: ([0-9A-F]{6}) writes ([0-9A-F]{8}) to ([0-9A-F]{6})", line)
            if not m or int(m.group(3), 16) & 0x3FF != 0x220:
                continue
            value = int(m.group(2), 16)
            if not 0x400 <= value < len(p.data):
                previous = None
                continue  # (cleared)
            kept += 1
            here = value if value in d.boundaries else value - 2
            if here in d.boundaries or here in d.ends:
                places += 1
            else:
                wrong[value] = m.group(1)
            if previous is not None:
                # (a place kept can be an instruction's or the word after its first: either reading will do)
                candidates = [c for c in (value, value - 2) if c in d.boundaries]
                if value == previous:
                    moves["stayed"] += 1
                elif any(c == d.following.get(b) for c in candidates for b in previous_here):
                    moves["to the next instruction"] += 1
                elif any(c in d.firsts for c in candidates):
                    moves["to a script's start"] += 1
                elif any(c in returns for c in candidates):
                    moves["back to the plan"] += 1
                else:
                    ahead = set()
                    for b in previous_here:
                        x, steps = b, 0
                        while x is not None and steps < 256:
                            ahead.add(x)
                            x, steps = d.following.get(x), steps + 1
                    moves["forward in the script" if any(c in ahead for c in candidates) else "elsewhere"] += 1
            previous, previous_here = value, [c for c in (value, value - 2) if c in d.boundaries]
        print("aiscripts: %d places the game kept, %d of them the instructions' found here; from one to the next: %s" %
              (kept, places, ", ".join("%s %d" % (k, v) for k, v in moves.items())))
        for value, at in sorted(wrong.items())[:20]:
            print("  %06X (kept at %s) is not" % (value, at))
        return 0 if kept and not wrong and not moves["elsewhere"] else 1
    if args.check:
        ins = len(d.boundaries)
        print("aiscripts: %s: %d instructions in %d characters' scripts; banks at %s; %d kinds of instruction, %d tests" %
              (args.game, ins, len(seen), " ".join("%06X" % banks[b] for b in range(4)),
               len([k for k in lengths if k != "if"]), len(tests)))
        for problem in d.problems:
            print("  " + problem)
        return 1 if d.problems else 0
    print("\n".join(out))
    for problem in d.problems:
        print("aiscripts: " + problem, file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
