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
