# Night Warriors: Darkstalkers' Revenge: progress

Issue #215. The set is `nwarr`, Night Warriors: Darkstalkers' Revenge (Europe 950316), MAME's parent and the set FBNeo's
training mode (and so Fightcade) plays. Every file's CRC32 checked against MAME's `cps2.cpp`. Sound driver 1.05A (runs
translated). Six buttons.

## Not a Vampire Savior game

`tools/portmap.py` lines up no more than 1.4% of any other game's code with this one: Vampire Savior 0.3%, Vampire
Hunter 2 0.4%, Super Turbo 1.4%, Alpha 1.1%, Marvel Super Heroes 0.7%, X-Men: Children of the Atom 0.0%. So Night
Warriors has sources of its own (`src/`), not Vampire Savior's through a `family.h`. Darkstalkers (1994) is the game
that should share this program.

## Where things are (each found here; nwarr.h has the names)

| What | Where | How found |
| --- | --- | --- |
| A5 | FF8000 | `move.w 0x0(a5),d0` steps the words |
| Step words | FF8000/04/08/0C | `--watch FF8000-FF8005` over boot and attract (FF8006 is a countdown that fills a watch) |
| Board start | FF8000 2 to 10 (01F88C on), frames 72-441, then 0 | watch |
| Show loop | FF8000 2-1A (020BC0 on); 6 with FF8004 = 14 is the title (frames 2801-3304); 8 on the fights shown | watch, pictures |
| A game begins | 0215F4 gives the game's own routine 02527C (Start pressed, in 021406, which takes the credit at 021454) | watch on credits, listing |
| Credits | FF81B2 (byte; the coin record at A5+1AC, 004D26 adds, 021454 takes) | dumps after 1, 2, 3 coins |
| Free play | none: the coin modes are 1 coin 1-9 credits, 2-9 coins a credit, 2 coins start 1 continue (18) | test menu stepped |
| Round clock | FF8E09 BCD (99 at 00876C, a second off at 0087AC); frames of the second FF8E15 (008798), reload FF8E17 | watch through a fight |
| Players | FF8388 and FF8888, 0x500 apart; health word +42 (0x90 full, 00668A) | watch; FBNeo script's FF83CB/FF88CB |
| Damage | 0C1384 `sub.w d4,0x42(a4)` (the only health writer in a fight) | watch |
| Settings copied | 01FC34 copies the record at A5-6D12 into the variables, returns 01FCDC | watch on the difficulty variable at boot |
| Song table | sound ROM 0x9006, 0x400 songs (as Marvel Super Heroes) | the word at 0x9000 |

The board's settings (each variable found by saving the test menu with one option changed and diffing memory after the
next start; names read from the test menu): difficulty A5+1F1 (0-7, 4 Medium), damage 263 (0-8, 5 Medium), time
count speed 233 (0-7, 4 Normal), turbo 212 (off, on, free select), max round 237 (1, 3, 5), mode select 203, join in
20A, event 209 (the test menu also sets join in off), continue 1EB, demo sound 1F0. The test menu edits its own copy
at FF256B (coin FF2586, difficulty FF258C).

The choice screens: speed (Normal, Turbo), the character, then mode (Normal, Automatic guard).

Health is taken off in 35 places, not one (`lst.py -r 'sub.w d4,0x42(aN)|subq.w #1,0x42(aN)'`): 0C1384 (most hits,
in the hit routine 0C1302/06/0A: A6 the attacker, A4 the one hit, A0 the attack), throws' and characters' own moves'
(A4), the thrown player's (A6), and a slow drain of damage still to come (02C184, the word at +188 a point at a
time; 06811A, 0681B4 and 0DA878 are subq's too). `cheats.cpp` hooks them all.

`code_end` is 120000: the game has code up to 0x11FFFF (it calls 108D74), and with 100000 more than a million
instructions a ladder ran in the interpreter.

### The player record (for training mode; RAM dumps of player 1 standing, crouching, walking, jumping, attacking, and
of player 2 hit and guarding)

| Offset | What |
| --- | --- |
| +02 | character (Hsien-Ko 0D, Victor 03) |
| +03 | 02 in a fight |
| +04 | state: 00 standing or walking, 02 crouching, 08 jumping, 0A guarding (02BAB2), 0C reeling; the step 02A220 dispatches on it (table at 02A252: 00-18) |
| +05 | 02 while attacking |
| +06 | 02, 04 while reeling |
| +08 | walking: 00 forward, 01 back, 02 not |
| +09 | facing: 00 right, 01 left |
| +10, +14 | X and Y words (Y 0x28 on the floor, upwards) |
| +1C | the animation frame (long, in the ROM) |
| +30, +34 | speeds across and up (longs) |
| +4B | hit stop (both players, 0B as a hit lands) |
| +10A | FF while attacking |

The fight step: 02A200 runs 02A220 for each player (A6 = FF8388, then FF8888).

### For the modes

| What | Where | How found |
| --- | --- | --- |
| A game's steps | FF8000 2 (the game's routine 02527C); FF8004 0C choosing, 04 the match; FF8008 06-08 a round's introduction, 0A fought, 0C over, 0E-10 the next round, 12 the match won, 00-04 the next opponent | watch through a one-hit ladder |
| Players in | FF81F9: bit 0 player 1, bit 1 player 2 | dumps before and after each Start |
| Sound queue | ring of 16-byte requests at FF0AA0 (mask 0x3F0), written up to the word at FF8026, sent from FF8028 by 008058 | listing (writes to 618000) |
| Camera | object at FF8F08: X (the picture's left edge in the stage) FF8F18, 0x100-0x280; Y FF8F1C; scroll 2 = X - 0xC0 (0FF6A8) | walking both players to each end |
| Stage | ends 0x118 and 0x3E8 for the players; they start at 0x228 and 0x2D8 | same |
| Stage number? | FF8D89 follows the opponent in most fights (not confirmed) | dumps up the ladder |
| Controls | FF808E player 1, FF8092 player 2 (A5+90/94: last frame's): low byte stick (bits 0-3) and punches (4-6), high byte kicks (0-2); read at 004942 | dumps with known buttons held |

## Done

- `free_play` (credits kept at 2), `skip_intro`, cheats (`no_damage`, `no_damage_2`, `one_hit`, `infinite_time`), the
  Board page (ten settings), the song table.
- Movies: `ladder.txt` (free_play, no_damage, one_hit: the ending at about frame 26,000, the title again by 27,000)
  and `versus.txt` (two players, then on against the computer).

Checked: free play and straight to the title (title at frame ~500 with free play, ~2800 without); each cheat over a
fight (player 1 or 2 stays at 0x90; one hit knocks out; the clock never ticks).

## Next

The checks (section 6 of the add-a-game steps), then everything the other fighting games have: training mode,
alternative modes, widescreen, camera zoom, colours, music names.
