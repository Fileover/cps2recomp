# Street Fighter Alpha 3: the model viewer and custom colours (notes to fold into PROGRESS.md and README.md)

## For PROGRESS.md

### Model viewer (`src/model.cpp`, the toolkit's `model/model.h`)

As Alpha 1's and 2's: a round of a two-player match reached unseen, player 2 put out of the way.

- **Animations**: the table at `0DD412`, a long by character (the character's number, `+102`), to three blocks of four
  longs: the A-ism's, the X-ism's (`+10`), the V-ism's (`+20`); 02FC4A puts a block's longs in the player's record as
  `+2B0`, `+2B8`, `+2B4`, `+2BC` when the character is set up (found as the only writer of those four). The viewer's
  lists A-D are the A-ism block's in the record's order (`+2B0`, `+2B4`, `+2B8` the reelings, `+2BC`). Each list is word
  offsets from the list to an animation (02A762). A list's length is not kept: it runs to the next of the character's
  lists or to the first animation frame after it (Ryu: A 76, B 50, C 49, D 70; Gen up to 133); `IsFrame` passes over
  what is not a frame.
- **Frames**: 0x14 bytes (02A7EA steps on by 0x14): `+00` a byte, the frames it is shown for (`FF` for good); `+01`
  flags (bit 7: the next frame is the long at `+14`, a jump; bit 6: the move's end, `st 0x33` at 02A7FC); `+04` the
  picture (a long); `+08` the number of a set of four box numbers (through the record's `+88`); `+09` the attack box;
  `+0A` (through `+C0`). 02A770 copies `+00`-`+03` to the record's `+32`-`+35`: the viewer shows a frame by putting it
  at `+1C` and `+32` at 7F each frame, so the game's own stepping never comes round.
- **The camera** leaves out a player whose `+263` is not zero (the players' extent for the camera, 01869C/018700):
  the viewer sets it for player 2. (`+67` is the throw's here, not the camera's.)
- **The panel's faces** (beside the bars) are objects in the players' own colour sets, tiles `16C00`-`177FF` (seen for
  Ryu, Ken, Cammy, M. Bison, Vega, Juni; no fighter's own picture uses that range: their tiles are elsewhere, e.g.
  Juni `23AD5`-`2E220`): `ThePanel` (modes.cpp) takes them by tile too, or player 1's face stayed in the viewer.
- Checked with no window: Ryu standing on his stage, Chun-Li's A10 on white, Cammy's A3 at 3x on black, Play all on
  Juni (A20, A34, A48, A68...) and on Gen through all four lists (A, B, C, D), 9,000 frames each of Ryu, Zangief and
  Karin's Play all with no instruction in the interpreter. The "Stage in front" choice and the Line row's animation
  search were not looked at closely.

### Custom colours and the colour editor (`src/colours.cpp`, the toolkit's `fight/colours.h`)

- **Where**: the table at `32C094`, a word by character of how far on from the table its colours are (0x3C0 apart:
  Ryu's at `32C0D4`), six sets there of five palettes of 16 colours (0xA0 a set). The set is
  `(ism + 1) * 2 + choice`: the ism `+132` (FF X-ism, 0 A-ism, 1 V-ism: seen with the byte held at each, the panel
  saying X-ISM, A-ISM, V-ISM; 2 resets the board), the colour choice `+128` (0 the ism chosen with a punch, 1 a kick).
  01F95C works it out as a round starts, keeps where it is at `+14C` and where its palettes go at `+150` (player 1
  `90C000`, player 2 `90C0A0`: colour sets 0-4 and 5-9, the record's `+0D`), and copies the five there. So the sets are
  named `XP XK AP AK VP VK`. Characters that share a set in the table (12, 13 Gen's stance, 14, 17) are not
  characters of this version; the editor and the files have the 28 fighters.
- **Effects**: single palettes from the tables at `3329D4` (030306, by character) and `34A7B4` (03037A), put back from
  `+14C` as they end (0303BE). The ranges a PalMod file may change are the sets (`32C0D4`-`3329D4`) and the first
  table's palettes (`332A14`-`3383B4`, up to the stages' table at `3383B4`, 01F994).
- **PalMod file**: `sz3.09c` (`300000`-`37FFFF`, words byte-swapped), refused unless its first `20000` bytes are the
  game's. Not tried with PalMod itself.
- Checked with no window: "Write the game's colours" wrote 28 files of six sets (Ryu's XP the grey X-ism gi, AP the
  white one); a `Ryu.txt` with the AP set's gi made red gave a red Ryu in a versus fight (A-ism, a punch); the editor
  driven by a movie on Sakura (its panel, rows stepped, a colour's level changed, Random colours with two presses) and
  its file saved by itself (`palettes/Sakura.txt`). Not used in a window by a person.
- Japanese names read too: `Nash.txt` for Charlie, `Gouki.txt` for Akuma (not Vega or M. Bison: in Japan those name
  other characters of this game).

## For README.md

Rows for the Alternative modes table:

| Model viewer | One character by itself, on a stage or on plain white or black, from a quarter of its size to 25 times. It stands and moves with your controls, or goes through any of its animations (named A0, B12, C3... as the game's own four lists number them; left and right step through them), over and over or all of them one after another (Play all), at full speed or in slow motion |
| Colour editor | Change a character's colours while you watch: [custom colours](#custom-colours) |

A section:

## Custom colours

Give any character colours of your own: change them in the game's colour editor, or bring files someone else made.

**The colour editor** is on the in-game menu's "Alternative modes" page. It shows the character with a panel at the
right, worked with player 1's controls: up and down choose a row, left and right change it.

- **Character, background, size, animation**: who you are dressing, on what, how large, standing or going through one
  of their animations.
- **Set**: which of the character's six colour sets, by how it is chosen: each ism (X, A, V) chosen with a punch (P) or
  a kick (K): XP, XK, AP, AK, VP, VK.
- **Line**: the character's own colours, or one of four lines for their effects.
- **Colour, Red, Green, Blue**: one of the line's 15 colours (it blinks on the character when you choose it), and its
  red, green and blue in the board's 16 levels each.
- **Random colours**: every colour of the set at random, at a second press on its row (LK, left or right, twice), so
  that one press by mistake loses nothing. HK puts back the colours from before the random ones, until the next change.
- **Save set** and **Load**: keep a set you like in the character's library and put it back later, into this set or
  any other. Left and right on Load try each saved one on the character; LP loads it, MK twice deletes it, and moving
  off the row without loading puts your colours back.
- LP copies a colour, MP pastes it, HP gives back the game's; the menu page puts back the game's colours for the whole
  set or the whole character.

Changes show at once and are saved as you go. **To play in them, choose the character's ism with the button of the
set you changed** (Sakura's A-ism with a punch, say), anywhere: arcade, versus, training.

**Files.** What you change goes in a `palettes` folder beside `settings.ini`, a text file a character (`Sakura.txt`)
holding only the sets that are not the game's. The folder is:

| System | Folder |
| --- | --- |
| Windows | `%APPDATA%\Street Fighter Alpha 3\palettes` (paste it into Explorer's address bar; it is the `AppData\Roaming` folder in your user folder) |
| macOS | `~/Library/Application Support/Street Fighter Alpha 3/palettes` |
| Linux, SteamOS | `~/.local/share/Street Fighter Alpha 3/palettes` |
| [Portable mode](../../README.md#portable-mode) | `user/Street Fighter Alpha 3/palettes` beside the launcher |

You can write or share them by hand: "Write the game's colours" (on the colour editor's page of the menu) puts every
character's own colours in `palettes/original` to start from, each set up to five lines of 16 colours (`#RRGGBB` or
`#RGB`; `-` keeps the game's). Saved sets are files too, one a set, in `palettes/library/<Character>/` (`Sakura 1.txt`,
`Sakura 2.txt`...): to share a design, share its file, and to use one someone gave you, put it in that folder. The
format is the same as the other games' here.

**PalMod.** A copy of your set's `sz3.09c` changed with [PalMod](https://github.com/Preppy/PalMod) can go in
`palettes` too (never the one in your ROM set): only the characters' colours are taken from it, so it cannot change
how the game plays, and text files go over it. This has not been tried with PalMod itself yet: if your colours do not
show, please [report it](#feedback-and-contributing).

"Load colour files again", on the same page, reads files you changed while the game runs. "Custom colours" on the
Display page switches all of it: on in Enhanced (it changes nothing until there are files), off in Pure.
