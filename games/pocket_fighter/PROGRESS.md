# Super Gem Fighter Mini Mix: progress

The USA arcade release of 1997-09-04 (ROM set `sgemf`, the parent set), *Super Gem Fighter: Mini Mix*, called
*Pocket Fighter* in Japan, on Capcom's CPS-2 board. Started 2026-10-19. Its sources are its own (`src/`).

## The ROM set

`sgemf` was chosen because it is the set competitive play uses: FBNeo's (and so Fightcade's) set of that name, the
one the FBNeo training-mode scripts are written for. Every region's release (`pfghtj` Japan, `sgemfa` Asia,
`sgemfh` Hispanic) is the same revision, 970904; they differ in the words on the screen and the warning. All 18
files, the key included, match MAME's CRC32s (`src/mame/capcom/cps2.cpp`, `ROM_START( sgemf )`).

The graphics are two rows of sockets: four 4 MB files at 0, then four 1 MB files at 0x1000000 (as Alien vs.
Predator's). Three buttons a player (Punch, Kick, Special: MAME's `cps2_2p3b`).

## Where it stands (2026-10-19)

- Boots and plays on the toolkit's own machine: the warning, the QSound logo, the introduction, the title, the
  attract mode's stories and demonstrations, the character choice, versus fights, the whole one-player ladder to
  the ending, the staff roll, the score ranking and the attract mode again (the ladder movie, by frame 38,000).
  Seen in pictures only: nobody has played it in a window or listened to it yet.
- 68000 program: 80,465 instructions translated to C++ (4,302 entries, `make explore` twice: about 4.5 and 7
  minutes). No instruction falls back to the interpreter over runs `explore` never made: the ladder movie, two
  made-up ladders of 60,000 frames, versus, 30,000 frames of two-player play, 20,000 of attract, the test menu
  (checked 2026-10-19). The interpreter stays only as the fallback for a place no run has reached.
- Sound: driver 1.30, translated: 392 entries, 5,180 instructions (the ladder, versus and the sound test,
  `--sound-test`, seeds 1-12, recorded into the entries). `--stats` still counts one instruction of the sound
  driver with no translation in every run, as Vampire Savior's build does (the same driver): the toolkit's, not
  this game's.
- The features every game has: free play (Enhanced; the game's own), straight to the title, the board's settings
  (the Board page), cheats (no damage for each player, one-hit knockouts, infinite time), save states, run-ahead,
  arcade timing, a custom soundtrack (its songs named by number until found), the in-game menu and the launcher.
- Not yet: a training mode, the alternative modes, widescreen, custom colours (below, "Next").

## The game's memory and routines (found by experiment)

Variables are reached from A5 = FF8000, as in the Alpha games.

- **Where the game is** (`pocket.h`): words at FF8000, FF8004, FF8008, FF800C, each a step of the one before (2 in
  a game; under it 2 choosing, 4 the match; then the match's and the round's steps). Traced with `--watch`: the
  board's start goes 2, 4 (the warning, frames 95-276), 6, 8 (frame 459); the show then 2, 0C, 0E, 10, ..., 18 the
  introduction (frame 1210), whose step at FF800C counts up in twos from 2 to 5A; at **4C** (frame 4887) the title
  builds up from white. Then 1A and on (the parental advisory, the demonstrations). With a credit in, from that 2
  to 2 and 4 (frames 470-471): the title asking for Start. A game begins at **0079D8** (FF8000 to 2), which copies
  the difficulty to A5+13E (007A64). FF8002 counts down each frame.
- **Credits**: a byte at **FF80F6** (RAM dumps after 2, 4 and 6 coins: 1, 2, 3; a new board asks two coins a
  credit). The coins toward the next credit are at FF8143. Held at 2 with `--poke`, Start goes to the character
  choice.
- **Free play**: the game's own, one of the coin modes (A5+117): 00-08 one coin for one to nine credits, 09-10 two to
  nine coins a credit, 11 two coins to start and one to continue, **12 free play** (09 on a new board). The board's
  start reads the mode (001796) and for free play sets **A5+11C** and the coin words **A5+F2** and **A5+FA** to 0101
  (001814); the other modes take their coin words from a table (00228C). The game tests A5+11C wherever it counts
  credits. The feature (`coins.cpp`) sets what that start sets, as Super Puzzle Fighter II Turbo's does. As on the
  board, the attract mode runs its whole show in free play; Start lets a player in at any point.
- **The players**: records at FF8400 and FF8800 (0x400 long). Health a word at **+40**, full 0x90 (0082F4 at a
  round's start); below nothing a knockout (015CE6 puts FFFF in it and in the bar, +42).
- **Damage**: every hit and throw takes its damage off at **015D6E** (`sub.w d4,0x40(a1)`, the victim at A1; the
  routine 015D64 keeps the health before at A5-5780 and adds the damage to +1B0). It is the only writer of either
  player's health in a fight that `--watch` found over fights and made-up play, but for the round's start and the
  knockout. The cheats are a hook there (`cheats.cpp`): 117 hits on player 1 over a run with no damage, the health
  0x90 after each.
- **The round's clock**: a byte at **FF8189** (A5+189), seconds left, 99 at a round's start; a second off at
  **0083E6** (`subq.b #1,0x189(a5)`), with the picture's BCD copy at FF8188 taken down by `sbcd` beside it; the
  frames of the second at **FF818A**, counted down at **0083DC** (at nothing the second goes, and 008FBE starts
  them again). Infinite time gives the frame back before it is taken (`cheats.cpp`), so both copies stand still.
- **The board's settings**: read once as the board starts, at **001796** (called at 001088, returning to
  **00108E**), from the settings record at A5-7398 (FF0C68) into variables from A5+113 on. The test menu edits the
  variables themselves. The Game page's and two of the System page's, each stepped through in the test menu with
  the variable read after each step (the board's own, shown in blue, in brackets):

  | | record | A5+ | values |
  | --- | --- | --- | --- |
  | Coin mode | +00 | 117 | as above (09, two coins a credit) |
  | Continue | +01 | 118 | 0 off, 1 on (on) |
  | Monitor | +02 | 116 | (not on the Board page) |
  | Demo sound | +03 | 119 | 0 off, 1 on (on) |
  | Sound mode | +04 | 11A | (not on the Board page) |
  | Chute type | +05 | 11B | (not on the Board page) |
  | Difficulty | +10 | 120 | 0 easy, 1 normal, 2-6 hard 1-5, 7 expert (**hard 3**) |
  | Damage level | +11 | 121 | 0 low, 1 normal, 2 high, 3 maximum (normal) |
  | Timer speed | +12 | 160 | 0 slow, 1 normal, 2 fast, 3 fastest (slow) |
  | Game speed | +13 | 15B | 0-3 turbo 1-4, 4 free select (free select) |
  | Max round | +14 | 122 | 1, 2, 3: one, three, five rounds (three) |
  | Event | +15 | 125 | 0 off, 1 one match, which clears continue (0018B6) (off) |
  | Join-in | +16 | 124 | 0 off, 1 on (on) |

  The collection's launcher shares a feature between games by name and choices: Difficulty, Timer speed, Max round
  and Event mean what Street Fighter Alpha's do, Join-in what Super Puzzle Fighter II Turbo's does. Damage level has
  Puzzle Fighter's choices where it means something else, so it is this game's own (`OwnFeatures`, game.cpp); Game
  speed's choices are its own.
- **The settings memory**: its routines are at 001F22 (read), 001F86, 001FEA and 002108 (the line driver, 804040).
  The toolkit's shortcut (`runtime/board.h`, `AddSettingsMemory`) is not used yet: it only saves time at the start.
- **The sound program**: Capcom's sound driver **1.30** (`1.30 /CPS2 1996 /OCT.`, at the start of `pcf.01`), as
  Vampire Savior's; not the toolkit's 2.01b, so the Z80 runs translated. Its table of songs is at 0x9006, four bytes
  a song, the word at 0x9000 the count (0x7ED), as Vampire Savior's.

## Checked

All on 2026-10-19, on the branch that brought the game in:

- `make GAME=pocket_fighter check` (translated code against the interpreter, frame by frame; two runs alike): the
  ladder movie (40,000 frames, `free_play,no_damage,one_hit`), the versus movie (14,000, `free_play`), made-up play
  with every cheat and straight to the title on (40,000). All ok.
- `make GAME=pocket_fighter check-blocks`: the ladder movie (346,717,987 68000 blocks, 139,442,384 of the sound
  program's), versus (113,258,704 and 48,420,936), 20,000 frames of the test menu with made-up input (198,334,937 and
  79,271,831). No difference.
- `make GAME=pocket_fighter check-release` (40,000 frames): two runs of the build with no translated code alike;
  its pictures the translated build's but for 10 frames, ending on the same.
- **With every feature off the game runs as with no sources**: the interpreter (`--cpu interp`) gives the same
  per-frame hashes with `src/` and with an empty one, 6,000 frames of made-up play.
- Straight to the title: the per-frame hashes are the same with it on and off (4,900 frames).
- Free play: Start goes to the character choice with no coin, and the screens say FREE PLAY.
- Cheats: no damage, 117 hits on player 1 over a run, the health 0x90 after each; one-hit knockouts put Morrigan
  down and the next match came; infinite time held the clock at 99.
- The board's settings: each of the 32 choices puts the right value in its variable as the board starts; with
  Expert, the one-match event and continue chosen, continue is cleared, and the test switch puts the board's own
  values back (Hard 3, off, on); the settings memory file is the same with and without choices.
- Save states: a state saved at frame 3,000 of made-up play, loaded at 3,000 into a run that had played otherwise,
  with the same input after: frames 3,001-6,000 alike. Run-ahead 1 and 2: the state the same as without it, all
  6,000 frames of the versus movie (2 frames took three times the processor, as it should).
- `make check` (the whole repository: audit of 422 files, the build with no game, the launchers).
- The game's launcher, drawn with no window: the Game page (the set's name), the Features page (Enhanced, free play
  on, the cheats, the Board page's nine settings), the Controllers page (Punch, Kick, Special).

**Not checked by a person yet**: nobody has played it in a window, heard its sound or music, or tried it with a
gamepad; the custom soundtrack has not been tried with files; arcade timing has not been run over the ladder.

## The movies

Scripted (the generator is not kept): `ladder.txt` (with `free_play,no_damage,one_hit`: Start at 1200, then
forward and Punch every 40 frames) takes Ryu up the ladder; `versus.txt` (with `free_play`) is both players in,
player 1 forward and Punch, player 2 Kick now and then.

## Next

- A training mode, the alternative modes, widescreen and custom colours, as the other fighting games' ports have
  them: written for this game's own fight code (its hit tests, damage, the players' state machine, the gems), found
  as above. The FBNeo training-mode script names more of the player record (state +06, facing +0B, combo +144,
  stun +146/+17F/+19E/+19F, the gauge +194/+195, the special moves' levels +18A and +1A2, items +1F0): to be checked
  here before use.
- The music's names (`CPS2RECOMP_SOUNDTRACK_LOG` over every stage and screen; `--music-test`).
- The settings memory's shortcut (`AddSettingsMemory`), if its routines are Capcom's usual ones.
