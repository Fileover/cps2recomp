# Super Street Fighter II Turbo: progress

The ROM set is `ssf2xjr1`, *Super Street Fighter II X: Grand Master Challenge (Japan 940223)*, a clone of `ssf2t`:
the revision Super Turbo is played on in tournaments and on Fightcade (issue #88: "whichever one the competitive
community prefers"). Every file's CRC32 was checked against MAME's `cps2.cpp`, the key included. The sound driver is
Capcom's 1.04 (`sfx.01`), run translated.

## Where it stands

- **Native**: the 68000 program translated by `tools/recomp.py` to `code_end` 240000 (the characters' own routines are
  past the first megabyte: the tables at 100000 name them, from 10FF12 to 2156E2; with 100000 3,605 instructions of a
  6,000-frame fight ran in the interpreter). The Z80 program translated. No emulator is linked.
- **Rewritten as readable C++** (`src/widescreen.cpp`, checked by `check_rewrites.sh`): the camera's place across
  (0CC4E4) and up (0CC630), and keeping a player inside the picture (0043F4).
- **Features**: free play (the game's own), straight to the title, the Board page, cheats, the custom soundtrack's
  names, the training session and its aids, the Alternative modes page (attract mode, the stage viewer, the model
  viewer, survival, endless battle, ring out), custom colours and the colour editor, widescreen and invisible walls,
  the hidden characters (Akuma and the old versions) with no code.

## What was found, and how

All in `src/ssf2t.h` with the routine or the experiment each came from. A5 is FF8000. FBNeo's training script for
this set (peon2/fbneo-training-mode, `games/ssf2t`) named some of them first; each was looked at again here.

- **Reads relative to the PC see the deciphered program**: `program.bin` holds the deciphered program and
  `program.bin.data` the ROM as it is. A table read with `move.w table(pc,dn)` is in the first; one reached through
  an address register (`lea table(pc),a0` then `(a0,dn)`) in the second. Both kinds are mixed, even in one routine
  (a character's animation list is the first kind, its frames the second).
- **Steps**: FF8000 is 2 in a game (the board's start steps 2 to 10 first, then its show from 0); FF8004 is 0 while
  choosing and 4 in the match; FF8008 is the match's: 2-4 versus, 6-8 a round's introduction, 0A fought, 0C-10 its
  end. Traced with `--watch FF8000-FF800F` through attract mode, a one-player game and a two-player one.
- **The show**: FF8000 2 the introduction and the title (FF8004 16 on: the title from white), 4 to 0C the notices, 0E
  a demonstration fight (a game the computer plays for both), 10 to 16 the rankings; frame 805 its first step.
- **Credits** FF82E9 (dumps after 1, 2, 3 coins; 002418), **players in** FF82F0.
- **Free play**: the coin mode (FF8342) is read at the board's start (00823A; the record at A5-6578, copied at
  00831C); mode 12, which the test menu never offers, sets FF82DF, tested wherever credits are counted, and the coin
  words FF82AE/FF82B6 come from the table at 008536 (0101 for mode 12). The title says FREE PLAY with it.
- **The board's settings**: the test menu's System Configuration, each option stepped with Right and the variable read
  after each press: game speed FF83A9 (0-3 turbo 1-4, 4 free select; turbo 2), difficulty FF82D3 (0-7: 1-8; 4), demo
  sound FF82DD (off), continue FF82DA (on), start mode FF8344, monitor flip, sound. The hook is 008FFC, where the
  boot code's call of 00823A returns; a game begins at 00D69C.
- **Clock** FF8DCE, BCD (99 at 00D780), with the frames of each second at FF8DCF: 40 a second, taken at 00FE74.
- **Players**: records at FF844E and FF884E, 0x400 apart (cleared at 003616). Health +2A (0x90 full) and before the
  last hit +2C. +03 who is in charge (00 standing or walking, 02 crouching, 04 jumping, 06 getting up, 08 guarding, 0A
  a normal or a throw, 0C a special, 0E reeling, 14 down), from a trace of a scripted fight against a player who stood,
  guarded, was hit and thrown. +06 and +0A place (longs, 16 bits of fraction; the floor 0x28), +12 facing left, +1A
  the frame, +34 the box tables, +3C/+3E speeds and +40/+42 their pulls (words, 002F50), +47 hit stop, +49 guarded,
  +181 in the air, +18B attacking, +1C2 half width, +284 chosen, +2B4 super gauge (0x30), +381 side, +38E a person,
  +391 the character, +39E the opponent, +3B1 the colours, +3B6 old, +3BD Akuma.
- **Damage**: 0BE646 (a player's attack: D6 off A2) and 0BEBA6 (a thrown thing's); throws D4 off A3 at 00490A and
  0049B0, or put by at +78 (004A58) and taken as the throw ends. Found as the writers of both healths over made-up play
  with no damage; with the five hooks no writer is left over three games of 30,000 frames each, for both players.
- **Hits**: the tests at 0BE518 (player 1's attack, player 2's, then thrown things at 0BEAC8); the box test 0BF5AE takes a
  box's offsets from the frame's numbers and the record's tables (as FBNeo's hit box script has them); a hit and a
  guarded one meet at 0BE678 (0BEBDA for thrown things), +49 set when guarded (0BEF32-0BEF58).
- **The player's step**: 0782D4, the dispatch on +03 of the fight step (0782A0), for the person's and the computer's.
- **Stun**: +5E built up, +5C its timer; stunned at the table at 07F1C6 by +2B1 (30, 34, 35).
- **Stage**: FF8C4E, the number of whose stage (0-15, in the characters' order; seen with its low byte held at each);
  the choosing writes it (0039A6), the match's first step is 00D714 and the stage is set up at 0CCC5C.
- **Sound queue**: FF0A7E (A5-7582), sixteen bytes a request, the next place at FF0A7A; FF05 stops the music.
- **Camera**: record FF8ECE, across FF8ED4 and up FF8ED8, copies FF8FEA-FF8FF0; ends A5+C54/C56 across, C58/C5A up;
  0x100 to 0x280 across on every stage. Players stop at 0x128 and 0x3D8.
- **The round's result** FF8DD2: 1 player 1, 2 player 2, FF neither (00FC88).
- **Objects by colour set**: hiding each set in turn on every stage: player 1 01-05, player 2 10-14, shadows 0A, hit
  sparks 0E, stage pieces 0B and 1A-1F, the panel the rest (07-08 faces, 0C bars, 0D score and clock, 19 gauges). The
  stage is all scroll screens and pieces; the panel is all objects.
- **Colours**: the table at 0BFAF2 by character (16 Akuma), a list of sets by +3B1, five palettes a set; copied at
  0BFA78 to palettes 1-5 (player 1) and 10-14 (player 2). LP-HK give sets 0-5 (seen choosing with each).
- **Animations**: three tables at 100000/100088/100110 name each character's routine (16 Akuma, 17-20 the old ones,
  21 the computer's Akuma); its list is 0x34 into it; frames are 0x18 bytes (002ED0 sets one, 002EE2 steps it).
- **Hidden characters**: Akuma's code ends at 00E942 (the three punches), the old version's at 015788; the choosing
  is 00E9B8 (a button at 00EA5C, made final at 00E93A).
- **Music**: the table at 0x9006 of `sfx.01`, 0x1FF entries, 68 of them music. Named from the soundtrack log over a
  fight on each stage, the ladder, a two-player game and attract mode.
- **Random number** FF82D0 (a shift register: 002E96).

## Checks

To be filled in with the numbers of `make GAME=ssf2t check`, `check-blocks`, `check-release`, `check-rewrites`,
`check-widescreen` and the hand-run checks (docs/VERIFYING.md).

## Not yet

- The other characters' endings in the music's names, and continue and game over.
- Nobody has played it by hand, heard its sound, or seen it in a window.
