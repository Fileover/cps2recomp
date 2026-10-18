# Cadillacs and Dinosaurs: progress

## How it is made (2026-10-18)

- **The ROM set** `dino` (World 930201, the parent set). No key file; the PAL dumps in the set are not read.
- **The board**: a CPS-1 with QSound (`board = cps1` in game.cfg; the toolkit's machine, video and ROM set have it:
  [docs/TOOLKIT.md](../../docs/TOOLKIT.md)). Its CPS-B-21 is set up as MAME's table of the chips has it for this game
  (QS2): layer control at 0A, priority masks 0C 0E 00 02, palette control 04, layer enable bits 16 (the game never
  uses them); its graphics PAL (CD63B) maps tiles to the ROMs in order.
- **The sound program** (`cd_q.5k`): its first 32 KB are enciphered (Kabuki). `machine/kabuki.cpp` works the key out
  from the program's first instructions (it is Capcom's QSound driver, "version 1.00 1992 / JUNE", which starts
  with a clear of its memory before the CPS-2's start) and its interrupt routine, and checks the candidates by
  following their code. The Z80 reads opcodes and other bytes from the two deciphered images.
- **Translated code**: `make explore` found the code (90,535 instructions; 170 left to the interpreter, 17 of them
  data); the sound program is all translated (2,210 instructions). `make check` passes.

## What is known of the game (dino.h has the names)

- A5 is FF8000. Coin chutes: three records of 16 bytes from A5+47A, credits at +3.
- **Settings**: the game reads three bytes laid out as the older CPS-1 boards' DIP switches (A5+777A-777C), packed by
  0AC86E from the settings the test menu shows (A5+775A on, kept at 776D-776F) as the board starts, and copied again
  by 0A86D8 when the test menu is left. B bits 0-2 difficulty (0-7, 3), B 3-4 difficulty 2 (0-3, 1), B 5-6 extend
  (0-3, 2), C 0-1 lives (0-3 for 1-4, 1), C bit 5 demo sound, C bit 6 continue.
- **The game's step** A5+4D0 (4 a game beginning, 0C playing, 08 between stages, 1C a stage's end) and the **stage**
  A5+4D9 (0-7): 00ABE8 steps it on at a stage's end; 00004CEE clears it with the game's other variables as a game
  begins (called at 00A9EC; 00A9F2 is hooked for "Start at stage").
- **The clock**: minutes A5+4E8, BCD seconds A5+4E9, counted down at 0103C0 each time the frame count A5+4EA (from
  0x32) runs out.
- **Players**: three records of 0x180 from FFB274 (3274(a5), 33F4(a5), 3574(a5)): +00 in play (1), +6D health (0x64
  full), +A3 lives after this one.
- **The show**: A5+35 goes 10, 30, 40, 50 as the board starts, 50 as the title is drawn (about frame 312).

## Not done yet

- A custom soundtrack (this driver's song table and command layout differ from the CPS-2's).
- Weapon cheats (the guns' ammunition): not found yet.
- Nobody has played it by hand; the sound has not been listened to (it plays: the chip is written and the output is
  loud in play, quiet in the attract with demo sound off).
