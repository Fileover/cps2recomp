# Giga Wing: progress

## How it is made (2026-11-03)

- **The ROM set** `gigawing` (USA 990222), the parent set, from the Internet Archive's VintageArcade item (none of the
  user's machines had it). Every file's CRC32 is MAME's (`cps2.cpp`, `ROM_START( gigawing )`), the key's too. One row of
  graphics (four 4 MB files at 0), two buttons (`cps2_2p2b`), Capcom's sound driver 1.40 (translated, not rewritten).
- **The picture is horizontal.** MAME has the game `ROT0`: a vertical shooter on a monitor standing as usual, its
  picture 384 x 224 like every other CPS-2 game's. Nothing is turned.
- **Other sets**: `scripts/other_sets_survey.py` (the add-game skill's) found `gigawingh` and `gigawingb` alike but for
  the header, and `gigawinga` but for the header and five words of data in its own third file (10432E, 11102E, 111052,
  11119C, 112ECE): all three are `other_set`. `gigawingj` (990223) is another revision, and the Phoenix sets are not
  taken. `online_words` has the header's words, those five, and the bytes the boot's ROM check adds up (000234: every
  4,095th byte of each 512 KB, as on the chips, where the keys differ).
- **Translated code**: `make explore` twice (free play, then free play and invincibility so that made-up play goes up
  the stages): 4,657 entries, about 88,000 instructions, 126 left to the interpreter (4 of them data); the sound program
  all translated (3,356 instructions).

## What is known of the game (gigawing.h has the names)

The game is Takumi's: its code is not Capcom's usual. A5 is FF8000 (000130), nearly every variable below it; its code
is strewn with `cmpi.l #$19721027,d1`, which does nothing. Its work is 83 tasks (entries of 16 bytes at A5-162A, their
routines in the table at 000634, the loop at 000584).

- **Credits**: the byte at A5-5F6B (up to 9; a coin adds at 001826, Start takes at 057BBE).
- **Free play**: the board's own. The settings routine (000D92) sets the byte at A5-5F69 when the coin mode (A5-5F5F,
  the record's first byte) is 0x12, the last; Start then takes no credit and the screens say FREE PLAY.
- **Settings**: the record at A5-5F4C (FF20B4): +00 coin mode, +01 continue (0 on), +02 monitor flip (0x80), +03 demo
  sound (1 on), +04 sound (0 QSound, 1 mono), +05 chute type, +10 difficulty (0-7, shown 1-8; 1), +11 ship stock (2-5;
  3), +12 force bomb stock (0-4; 2). 000D92 copies them into A5-5F5F, -5DBB, -5DBA, -5DBD, -5DBC, -5DB9, -5DBE, -5DB8,
  -5DB7 as the board starts and as the test menu is left. Found by stepping each option of the test menu with Right
  and Left. The game reads the ships at a game's start (0085F4) and the bombs at each ship's start (009236).
- **The mode** (the word at A5-0D82, task 82's routine 0072B8 jumps on it): 0C and 10 the logos, 11 the story, 05 the
  ranking, 04 the title, 12 a demonstration, 0B the choice of fighter, 0E the pilot's story, 0D the lesson (Start skips
  both), 06 a stage played, 07 continue and game over, 08 the ranking after a game. Before the task loop runs (the
  warning) it is 0. With credits or free play the board goes from its warning straight to the title.
- **Players in**: the byte at A5-0FA4, bit 0 player 1 and bit 1 player 2 (057C20).
- **Stage**: the word at A5-0CC0, 0-6; 008484 puts it at 0 as a game begins (the port's start at stage hooks the
  next instruction), 0915DA counts it on. Stage 6 is a gauntlet of the earlier bosses; stage 7 the Medallion.
- **Ships**: words at A5-0D24 (player 1, then 2), set at a game's start, one taken at a loss (008F7E); **force bombs**
  at A5-0D20 (one taken at 04AE98; 023764 counts no further than 8). **Fighter**: words at A5-0CF8 (0 Sinnosuke and
  the Raijin, 1 Ruby and the Carmine, 2 Isha and the Porchka, 3 Stuck and the Widerstand, 4 an unfinished fifth: a grey
  ship with a broad laser, no pilot on the choice of fighter and broken panel icons; the Dreamcast port finished it as
  Stranger). The choice of fighter (040B6A) steps its cursor (+02 of the player's object) with Down and Up and keeps it
  below 4 (`andi.b #3`, 040B92). **Power**: words at A5-0CF4, 0-3.
- **Scoring**: the multiplier (BCD long a player, A5-0D1C on, at most 49,999,999) and the medal chain (BCD long a
  player, A5-0D10 on): a gold medal picked up (02369C) adds its value to the chain and the chain to the multiplier;
  02368C puts the chain back to 0. **Scores**: twelve BCD digits a player from A5-0F32 and A5-0F2A.
- **The ships**: objects of 0xC0 at FFADEA and FFAEAA: +00 flags (bit 15 in use, bit 12 hit), +0A across (the
  picture's column), +0E up from the picture's foot, +16-+1C the hit box from the middle (3 each way: 6 x 6), +58 and
  +74 pointers to the player's ships and bombs. Two tests hit them: 023956 the enemies' shots (a list from A5-10E0,
  each a point at +06, +0A; 029DE8 walks it and calls 02398C, which sets the hit bit), and 023A4C the enemies' bodies
  (0x30 records of 0x80 from A5-0A36, their boxes at +16-+1C round +0A, +0E; 023AAE sets the hit bit). So a shot
  is a point against the ship's box, and a body a box against the ship's middle; a shot's point is its tip.
- **The boss's clock**: the word at A5+57FC, seconds left, counted down every 63 frames by 0593F6 while bit 3 of
  A5-0D84 is set (the boss is being fought); at 0 the boss leaves. A boss that leaves on stage 4 ends the game.
- **Sound**: the song table is at 0x9006 after a word of their number (0xAF), as Capcom's. Music found by playing
  each stage (music.cpp names them).
- **Slowdown**: with Arcade timing on, the ladder movie loses about 670 of 28,000 frames over stages 1 to 3 to the
  board's slowdown (up to one in twenty in the busiest stretches); with it off, none.

## What the port adds

Free play (the game's own flag, put back from the coin mode when switched off), straight to the title (run unseen until
the title or a game), start at stage (008488), the cheats (invincibility by answering "missed" at 02398C and skipping
023AAE; infinite bombs by skipping the subtraction at 04AE98; infinite ships kept at two while a stage is played), the
Board page (choices put in the variables before every frame, the board's own while the test switch is held), the
music's names, and the shoot-'em-up modes of the toolkit's `shmup/` (practice and its aids, score attack, caravan,
boss rush, stage viewer, attract mode).

## Checks

(filled in below as they are run)

## Not checked by a person

Nobody has played it by hand: not the controls' feel, not the modes in a window (headless runs only), not the sound
(never listened to), not online play between two machines (`check-online` plays two copies on one).
