# Street Fighter Alpha 3: music, stage tunes, attract mode, hidden characters (notes for PROGRESS.md)

## Music (`src/music.cpp`)

- Driver 1.71's table of songs at 0x9006 (four bytes a song) does not tell music from effects (both begin with a 0x20
  header), and songs 19-1C (Zangief's and Gen's themes among them) have no entry there at all, yet the board plays them
  (heard in fights: the audio dump's level with player 1 poked to Zangief is as loud as with Dhalsim; `--music-test`
  plays all of 01-2F). So the toolkit's `musicBelow` now counts every song below it as music, with an entry or not
  (`machine/soundtrack.cpp`, FindMusic: one line changed in the main branch's new code).
- A fight's tune is a character's theme, not the stage's: 003A30 takes player 1's or player 2's record by A5+130 and
  looks the character up in the table at 003ABA (09-1A for characters 00-11, 1E Balrog, 1F Cammy, 21-26 E. Honda to
  Karin, 27 Juli and Juni), with its own cases before (003A54 on): Sodom of one kind (+132 bit 7) 1D, the final boss
  (+15D) 20, the tenth fight against Ryu 28, the ninth on the final stage 29, the ninth on Akuma's stage 1B. The other
  tunes are asked for by kind from the table at 003AF8 (2D, 2F, 08, 02, 04, 05, 03, 06, 07, 01, 2B, 2C, 29).
- Placed with CPS2RECOMP_SOUNDTRACK_LOG and pictures where each starts: the ladder to its end with E. Honda and with Ryu
  (02 player select, 08 the character's introduction before each match, 04 the winner's quote, 2B a rival's words, 29
  Ryu against Ken on the final stage, 2C M. Bison's words, 20 the final fight, 2D the ending and staff roll, 06 game
  over, 07 rankings), a lost game (05 continue), a challenger joining (03), attract mode (2F the notices, 01 the
  introduction, the demonstration fights' themes, 07). `--music-test 3600`: 01 (26.8 s), 03 (2.7 s), 06 (3.0 s) and 2F
  (4.0 s) end of themselves; 00 is silent; the rest play on.
- Not placed: 1C, 2A, 2E (they play on the board; where the game asks for them was not seen), and 28 and 1B were not
  heard in a game (named from the code).

## Stage tunes (`sfa3.h`, kStageTunes)

The game has no tune of a stage's own: each stage's is its character's theme (stage n is character n's for n < 12; M.
Bison's garden 13, Cammy's 1F, E. Honda's 21 ... Karin's 26), and the final stage's the final fight's (20). The stage
viewer's "the stage's own" leaves the tune to the game, which plays the unseen fighters' theme, so a hook at 003AB2
(`modes.cpp`, FightTune: the tune just taken from the table) gives the stage's in the stage viewer only. Checked: the
viewer on Ryu's stage 09, the final stage 20, Cammy's 1F; its Music row (Cammy's on Ryu's stage: FF05, then 1F).
kTuneChoose 02; kTuneVersus 08 (the character's introduction is this game's versus screen).

## Attract mode (`modes.cpp`)

- The loop's counts: FF0E76 (A5-718A) stepped by 2 at the end of each half of the loop (0043EC: after 1A and at the
  loop's end; its bit 1 sends the loop back to 0C), and FF0E77 (A5-7189) with it, 0 to 0A, which picks a page of the
  show (025886). Found as the only bytes stepping alike between RAM dumps before each demonstration fight over three
  loops, then `--watch`. The fights-only show steps both where it leaves a half out (HalfDone), so they go as on the
  board (traced: 02/02, 04/04, ... 0C/00, 0E/02 at each fight).
- The demonstration fights' fighters are drawn at random (004B3A: 004BA4 the random number into a table, never the same
  as the last, kept at A5-7188), so the fights-only show's fights are not the board's in the same order (it never could
  be: the random number has moved on differently). Seen: seven fights in a row, each other fighters on another stage.
- Checked with `--keys 60:cattract_start` and `attract_show` 0, 1, 2 over 24,000 frames (the steps traced, pictures every
  1,000): everything is the board's loop (0C ... 32, then 0C); the intro only goes 10, 12, 10 ...; the fights only 14,
  16, 2A, 2C, 14 ... (each fight 1,200 frames).

## Hidden characters (`src/choosing.cpp`)

- The character choice's grid is 8 x 8 at 01E724 (pc-relative, so the deciphered program), the cursor's column and row
  at the player's +10 and +14 (01E6BE; 01E700 writes the cell's character to +102). 25 faces and four cells (A0, 62, 63,
  A1) that choose at random: bit 5 of the cell (01E6EE; the character then drawn from the table at 01E8C4 by the random
  number, 01E898), bits 6 and 7 the players who may not use one. Balrog (15), Juli (1E) and Juni (1F) are on no cell.
- The board gives them by its "time release": A5+EE, from the settings memory (0019DE: the record's +75), stepped by
  the board's play time and games played (001604 on). From level 1 the random cells' table has them in its second half
  (01E8B2), and holding the cursor still on the middle cell for a second sets +3E (01E944); levels 2 and 4 open more
  (01E7B0: +132 set to FF, a hidden kind of ism; 01E78E). Not followed further.
- The feature (Game page, `hidden_characters`): Start held as a player chooses M. Bison gives Balrog, Cammy Juni, Rose
  Juli, changed at 01E764 where the game takes the cursor's character (not on a random cell). Checked: each of the
  three chosen so by player 1 (cursor walked from Ryu), fighting with its own name plate and face on its stage; with
  the feature off the same movies give M. Bison, Cammy and Rose.

## README lines

- Soundtrack bullet: **A custom soundtrack**: your own music in place of the board's, with the board's sound effects and
  voices: a folder of music plays as a playlist (in order or shuffled), or files named for the game's songs replace
  those songs. The `songs.txt` the game writes in the soundtrack folder names its music: each character's theme, the
  ladder's last fights and M. Bison's, player select, the characters' introductions, the winner's, continue, game over,
  the ending and the rankings. [How →](../../README.md#a-custom-soundtrack)
- **Hidden characters** (Game page): Balrog, Juni and Juli, whom the board gives only by its time release, with no code:
  hold Start as you choose M. Bison for Balrog, Cammy for Juni, or Rose for Juli.

## Not done here

- Song 00 counts as music under `musicBelow` (it is silent; the game asks for it as the board starts).
- Nothing listened to by a person; no window.
