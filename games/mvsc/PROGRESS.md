# Marvel vs. Capcom: progress

The European arcade release of 1998-01-23 (ROM set `mvsc`, the parent set), *Marvel Vs. Capcom: Clash of Super
Heroes*, on Capcom's CPS-2 board. Started 2026-10-28 (issue #135). The last of the Marvel games on the board, made from
Marvel Super Heroes vs. Street Fighter's program, on the Marvel games' shared engine (`fight/marvel.h`): each side is a
team of two characters, swapped in and out during the fight, with a special partner to call in.

## The ROM set

`mvsc` was chosen because it is the set competitive play uses: FBNeo's (and so Fightcade's) set of that name, the one
the FBNeo training-mode scripts are written for. Sets of the same date (Asia, Brazil, Hispanic, Japan, USA 980123) differ
in their region; older revisions exist (971222 USA, 980112). The owner's own copy (a merged `mvsc.zip` among their
MiSTer backups, on the Windows PC) has the same files as the Internet Archive's (item VintageArcade, `mvsc.7z`); all 21
files, the key included, match MAME's CRC32s (`src/mame/capcom/cps2.cpp`, `ROM_START( mvsc )`, the good dump of
`mvce.04a`).

The graphics are two rows of four 4 MB sockets. Six buttons a player (MAME's `cps2_2p6b`). Sound driver 1.30.

## Where it stands (2026-10-28)

- Boots and plays on the toolkit's own machine: the warning, the notices, the introduction, the title, the attract
  mode's fights, the character choice (two characters, the mode, the speed, then the special partner), versus fights,
  the whole one-player ladder to Onslaught and the ending, the staff roll, game over and the show again (the ladder
  movie: Onslaught by frame 15,000, game over by 22,500). Seen in pictures only: nobody has played it in a window or
  listened to it yet.
- 68000 program: translated to C++ (183,126 instructions from 1,513 entries: `tools/portmap.py` from Marvel Super Heroes
  vs. Street Fighter's, `make explore`, and the runs below with `--record-entries`). No instruction falls back to the
  interpreter over runs `explore` never made: the ladder movie (and in widescreen), made-up ladders, versus, two-player
  play, 20,000 frames of attract, the test menu, the sound test, and every mode (training, attract mode three ways, the
  stage and model viewers, the colour editor, survival, endless battle and its scroll, ring out, tag-team co-op and 2 vs
  2). Six places the modes' other machines reach (the camera's jump tables, and a return the ring out comes back to)
  were added by hand, as for Marvel Super Heroes vs. Street Fighter: those machines record no entries. The interpreter
  stays only as the fallback for a place no run has reached.
- Sound: driver 1.30, translated. `--stats` counts one instruction of the sound driver with no translation in every
  run, as the other games' builds do: the toolkit's, not this game's.
- The features every game has: free play (Enhanced; the game's own), straight to the title, the board's settings
  (the Board page), cheats (no damage for each side, one-hit knockouts, infinite time), save states, run-ahead,
  arcade timing, a custom soundtrack (its music named), the in-game menu and the launcher.
- Everything the other Marvel games have: a training mode on the toolkit's session (`fight/session.h`: hit boxes,
  frame data, its meter and log, input display, combo readout, health refills, the dummy; and the game's own gauge
  refill and partner's refill), the Alternative modes page (`fight/alternative.h`: attract mode three ways, the stage
  viewer), the fighting modes (`fight/battle.h`: survival, endless battle, ring out), the model viewer (`model/`),
  custom colours and the colour editor (`fight/colours.h`), widescreen and invisible walls, the zoom, the hidden
  characters with no code, and tag-team co-op and 2 vs 2 (`fight/tag.h`).
- Not added, as the user's rules say: the tag team battle (the game has its own tag team), and the CPU coach (the
  game's computer plays every mode itself).
- **On the Marvel games' shared engine** (`fight/marvel.h`), with nothing new in it: the game keeps its tag team as
  Marvel Super Heroes vs. Street Fighter does (records of their own, `playerAt`) and its camera the same way (a copy of
  its place, `cameraWorkX`/`Y`), so `src/engine.cpp` is a description of where this game keeps the same things; the
  frame copy is the toolkit's `marvel::CopyFrameBoxSets` (001DFC, the same instructions).
- No readable rewrites: what the features change is done in hooks that run the game's own code unless a feature is
  on, each a few lines.

## The game's memory and routines (found by experiment)

Variables sit below A5 = FF8000; the facts are in `src/mvsc.h`, each with how it was found. `tools/portmap.py --from`
Marvel Super Heroes vs. Street Fighter's work folder (with its guessed entries) `--to work/mvsc` placed 51,385 of
155,293 of that game's instructions here and named 9,587 places, which gave most of what is below a first guess; each
was then confirmed here. In short, Marvel Super Heroes vs. Street Fighter's variables 0x800 lower again (FF4800 there
is FF4000 here), and the players' records 0x20 further on from +0x250 (health +270, and so on) and 0x10 from +0xA0
(the action +B0, the push box +B4, held at the edge +B9):

- **Where the game is**: FF4000, a byte, 8 while a round is fought; FF4100 set once a game is played, FF4101 who is in;
  FF4104, the game's step (4 choosing, 8 a match), FF4106 the match's. The show runs as a task (its record at FF2666,
  step byte FF266A); the title's scene takes its first step at 0D1302 (frame 1,420 from switching on).
- **Credits** FF4499; **free play** the game's own coin mode 12 (FF44A5), which sets FF449B (007F36).
- **The board's settings**: the record at FFF648 is read into variables at the board's start (007E7A, called from
  000FAC; the hook at its return, 000FB2, puts the choices in). Each option was changed in the test menu, saved, and the
  variables read after the next start; the names read from pictures. The game has a tenth, Flickering (+17, FFF9B8);
  the event sets continue and join-in off.
- **Players**: four records of 0x400 from FF3000 (player 1's first character, player 2's, then their second ones); as in
  Marvel Super Heroes vs. Street Fighter a tag does not swap them, and the longs at FF40C8 and FF40D0 are each side's
  character on point (traced over the tag-team movies). +0C/+10 where a character stands, +4B facing, +52 the character,
  +6C the box table, +70-+7C the box numbers, +82/+83 attacks hit / can be hit, +85 hit-stop, +B0 the action, +B4 the
  push box (the FBNeo hitbox script's places for this set), +270 health (`sub.w d5,0x270(a6)` at 01B74E and 01BEA0),
  +272/+274 the gauge, +277 the colour choice, +27A what comes back while resting, +280 which pick it is, +281 1 while
  it rests, +284 the partner's record, +288 its palettes.
- **Characters**: the word +52, each number put in player 1's picks and seen in a fight with its name: 02 War Machine,
  04 Captain America, 06 Hulk, 08 Wolverine, 0A Gambit, 0C Venom, 0E Spider-Man, 10 Roll, 12 Ryu, 14 Captain Commando,
  16 Chun-Li, 18 Jin, 1A Zangief, 1C Strider Hiryu, 1E Morrigan, 20 Mega Man, 24 Red Venom, 26 Orange Hulk, 28 Gold War
  Machine, 2A Shadow Lady, 2C Lilith; 22 is Onslaught (player 2's picks in its fight) and 00 its second form, which put
  in the picks fight as Ryu (the game makes their fights its own way), so they are not in the model viewer; 2E on reset
  the board.
- **Choosing**: a 0x30 record a player from FF4180 (player 2's at FF41B0), +02 the side, +0C the character under the
  cursor; its first byte steps to 7 once the characters, the mode and the speed are chosen, then on for the special
  partner. A character is taken at 00BCDA (the hidden characters' hook). Picks: FF40B0 on, as before.
- **Stage**: FF4113 (0-9, named from pictures); taken at 00C2CA from a table (0E324C, or 0E326C) by a count; Onslaught's
  fights are on 9 and 10 (10, space, keeps no camera record, and 11 is none: not offered); the hook at 00C2F8 puts a
  mode's stage and characters in.
- **Camera**: as Marvel Super Heroes vs. Street Fighter's: one of three records by stage (the hitbox script's table),
  its step (0CB2A2 across, 0CB4E2 down) working on a copy (+76/+78), moved the way FF4408 and FF4409 say, kept between
  +68/+6A and +6C/+6E (0CDBB6, 0CDBDA) and put back (0CDBCE, 0CDBF2). The ends across are -64 to 320 on most stages,
  -64 to 304 in the bathhouse and 960 to 1344 in Onslaught's city; the floor is the camera's lowest place + 0xD8. A
  character is kept inside the picture at 013AD6 (the planes' table at 02563A).
- **Panel and objects** (from the object lists of rounds on every stage): colour sets 00-0B the fighters (player 1's
  first character 00, player 2's 03, their second characters 06 and 09: 013186 on), 0E the shadows, 0D, 13-15 and
  1E-1F the panel, 16-1D the stage's own pieces; the partners' marks on the panel are tiles 1CC80-1CCFF.
- **Colours**: a long a character at 1C7268, two sets of three palettes, 0x60 apart (Onslaught's two forms share one).
  PalMod's file is `mvc.06a`.
- **Animations**: groups of words of offsets from a list a character at 0E676C; frames of 0x10 bytes, copied into the
  record by 001DFC.
- **Music**: each character's theme is 0C and its number (word / 2); the screen before a match plays player 2's first
  character's, the round its second's (traced over the ladder with `CPS2RECOMP_SOUNDTRACK_LOG` and the characters'
  words). 0C is Onslaught's fight's. Others: 01 the opening, 02 player select, 04 before each stage, 05 the winner, 07
  the QSound logo, 0B before Onslaught, 2E after it, 20 the ending, 1D the staff roll, 0A game over, 2F the rankings.
- **The round's result**: FF4005. **The sound queue**: FF27D8, the next place at FF27D2 (0D4C7A). **The random
  number**: a long divs then the number kept at 001932; a hook at 001930 starts a block there.

Facts were confirmed with the help of the FBNeo training mode's scripts (peon2/fbneo-training-mode: `games/mvsc`,
`hitboxes/marvel-hitboxes.lua`), each found again here; no code was taken.

## The checks

(2026-10-28, all on the Mac.)

- `make GAME=mvsc check FRAMES=40000 MOVIE=games/mvsc/movies/ladder.txt ARGS="--features free_play,no_damage,one_hit"`:
  translated code does what the interpreter does over the whole ladder; two runs alike.
- `make GAME=mvsc check FRAMES=12000 MOVIE=games/mvsc/movies/versus.txt`: the same over versus.
- `make GAME=mvsc check-blocks MOVIE=games/mvsc/movies/ladder.txt FRAMES=40000`: every translated block checked against
  the interpreter, and the sound driver's (308,022,572 blocks and 140,046,696 of the sound driver's, no difference).
- `make GAME=mvsc check-release FRAMES=40000`: two runs of the build with no translated code alike; its pictures the
  translated build's but for 18 frames (each one frame early), and the same last picture.
- `make GAME=mvsc check-widescreen`: every one of the ten stages walked to both ends in widescreen, no unpainted
  columns; invisible walls leave the game's memory the board's at every 4,000 frames of the ladder; the ladder in
  widescreen with no instruction left to the interpreter.
- The tag-team movies (X-Men vs. Street Fighter's with the fight 400 frames later, as this game's choosing is longer):
  `make GAME=mvsc check FRAMES=6600 MOVIE=games/mvsc/movies/coop.txt ARGS="--features free_play,tag_team=1 --poke FF3274=03"`
  and `make GAME=mvsc check FRAMES=5600 MOVIE=games/mvsc/movies/two_vs_two.txt ARGS="--features free_play,one_hit,tag_team=2"`:
  translated as interpreted, two runs alike; the point character's long traced through the tags, and the label under
  the bars following it (pictures every 100 frames).
- `make GAME=mvsc check-rollback` and `check-online`: pass (3,000 frames, 940 rollbacks; the online match the same as
  both players' controls replayed). `make check` and `tools/audit.sh` pass.
- By hand: `--stats` over the runs above and every mode (no instruction to the interpreter; the ring out and the
  endless battle run a dozen times each, their choices at random); features off and no sources at all give the same
  `--hash-log` over 6,000 frames of versus (`--cpu interp`); a state saved at frame 3,000 and loaded into a run of the
  same input: frames 3,001-6,000 alike; run-ahead 1 and 2 give run-ahead 0's hash log over 8,000 frames of versus;
  "Straight to the title" on and off give the same hash log; every Board choice read back from its variable (the event
  setting continue and join-in off too), the test menu showing the board's own with choices in, and the settings memory
  the same with and without them; no damage (player 2's team's health stays 0x90 through versus), infinite time (the
  clock at 99), one-hit knockouts (the ladder); the hidden characters (Start held on Chun-Li: Shadow Lady); frame data
  over a training session (`frame_data.csv`); pictures of every mode, the zoom at 2x and on both, the special partner
  called (medium punch and kick), and the launcher's three pages.

## Not checked by a person

Nobody has played it in a window, used a gamepad with it, or listened to it: every check above is headless. The
custom soundtrack files, the dummy's record and replay, and the arcade timing have not been tried by hand. Roll's and
Zangief's themes are named by their numbers, not heard. Attract mode's "fights only" shows parts of the introduction
between the fights too: this game's show mixes them more than the earlier games'.

## Next

- Online play is the toolkit's; this game has not been tried over a network.
