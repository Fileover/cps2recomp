# Alien vs. Predator: progress

## How it is made (2026-10-18)

- **The ROM set** `avspu` (USA 940520), as MAME's current sets have it (with `avspu.key`). Older collections have
  four graphics ROMs (`avp.14m`, `16m`, `18m`, `20m`) from an earlier dump, and no key file; the CRCs in `game.cfg`
  are the redumped ones. The program is the one revision; the other regions differ only in their words.
- **Translated code**: the toolkit as for any CPS-2 game. `make explore` found the code (82,889 instructions; none
  left to the interpreter but 25 a guess took for code); the sound program, Capcom's QSound driver of November 1993,
  is translated too (2,153 instructions). `make check` passes.
- **Three players**: the toolkit's CPS-2 machine reads players 3 and 4 where the board has them (their sticks and
  buttons at 804010, Start and Coin on the system port), game.cfg says `players = 3`. The test menu's input test
  shows the third player's every line.

## What is known of the game (avsp.h has the names)

- A5 is FF8000. Credits: A5+18C (byte, at most 9).
- **Settings**: the settings memory's record at FF0EF0; 003C5E copies it into variables at the board's start
  (called at 000D1A) and when the test menu is left (0B3C64): coin 1CD (0-7, 8 two to start and one to continue,
  9 free play), continue 1BE, flip 1B2, demo sound 1AD, sound 1D2, player mode 1CE (0 three players with a chute
  each, 1 three on one chute, 2 two: the factory's), difficulty 1CF (0-7 for 1-8; 3), extend 1D0 (0 none, 1-3 only
  at 100,000/400,000/1,000,000, 4-7 first and every; 1), lives 1D1 (0-3 start and continue 1/2/3/5, 4-7 start
  1/2/3/5 and continue one more; 4). From the player mode it works out a third player (1C5) and a chute each (1C6),
  which the game reads. The Board page hooks the two returns (000D20, 0B3C6A).
- **The show** (A5+02, word): 2-C as the board starts, then 0, then the attract's steps, 10 the title (drawn at its
  own step, A5+04, 8).
- **Players**: three records of 0x100 from FF8380: +00 in play (1), +41 health (0x65 full; FF down), +45 the full
  health, +F1 lives after this one.
- **Rounds**: the scene, A5+1E8 (word, two a scene), cleared before a game and set to 2 by 0082DC as the players
  start after round 1's story (its rts, 008352, is hooked for "Start at round"); the round shown, A5+222, from the
  table at 00CCA0 (scenes 0-2 round 1, 3-4 round 2, 5 round 3, 6-7 round 4, 8 round 5, 9-13 round 6, 14 on round 7).
- **The clock**: minutes A5+224, BCD seconds A5+225, counted down at 0085A4 each time the frame count A5+221 (from
  0x3C) runs out.

## Not done yet

- A custom soundtrack (the song table and the music's names).
- Weapon cheats (the guns' heat, the Predators' cannon): not found yet.
- Nobody has played it by hand; the sound has not been listened to.
