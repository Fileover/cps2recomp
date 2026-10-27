# Changelog

## Unreleased

- **Shaders** (macOS for now; #121): the Display page's **Shader** draws the picture through one of RetroArch's slang
  shader presets, by [librashader](https://github.com/SnowflakePowered/librashader) 0.12.1 with OpenGL: CRT
  (crt-geom-mini), CRT Lottes, CRT Royale, CRT Guest (advanced, fast), Scanlines, xBRZ (freescale) and ScaleFX, or Your own,
  the first `.slangp` in `shaders/` beside settings.ini. `--shader FILE.slangp` takes any preset. The picture keeps its
  shape and size; zoomed, the shader is given what the camera sees. The menu is shown without the shader. librashader
  needs the window's own OpenGL, which SDL's GPU API does not hand out, so with a shader the window is an OpenGL
  window; switching to or from Off makes the window again where it is. Without `librashader.dylib`, or with a preset
  it cannot make, the picture is drawn as before (and stderr says why). macOS's OpenGL stops at 4.1, so a preset whose
  shaders need more (crt-geom itself, crt-hyllian) cannot be made there: CRT is crt-geom-mini. The app carries librashader (built from
  source by `tools/librashader.sh`, which needs Rust) and the presets (copied by `tools/shaders.py` from a pinned
  slang-shaders commit, each file keeping its own licence).

## 0.4.0-beta.7

Two more games, Street Fighter Alpha 3 and Super Street Fighter II Turbo, with what the other fighting games have
(Super Turbo's tag team battle too): fourteen in all. A multi-game endless battle that hands off from one game to the
next. Zoom on one fighter or on both in every fighting game. The CPU now plays the tag style and the ring out itself
in the Vampire Savior games and Alpha 2 and 3, and every Alpha's and Vampire Savior's computer player is readable C++.
Speeds from 1/8 to 8x and Reset all options in the menu, your games in your own order on the launcher, a launcher that
fits a small window, portable mode, and a download for Linux ARM64.

- **Faster speeds, and finer ones**: the menu's Speed goes from 1/8 to 8x in 13 steps, each about 1.4 times the last
  (1/8, 1/6, 1/4, 1/3, 1/2, 3/4, Full, 1.5x, 2x, 3x, 4x, 6x, 8x; - and = step through them). Sped up, every frame's
  sound is played, faster by as much, as slowed it is played slower. `--stats` says the screen's ticks when they
  were not the game's frames.
- **Reset all options**, above Reset game (Reset renamed): every feature back to the launcher's default, the Enhanced
  preset, in settings.ini too, and the speed back to full. The game's own Test menu settings and the controls stay.
- **Street Fighter Alpha's computer player is readable C++** (`games/sfa/src/computer.cpp`), Alpha 2's interpreter
  with Alpha 1's own (one if, fewer waits and tests, one kind of jump, a think time by character), checked call by
  call against the game's code with nothing differing (about 910,000 calls). It plays the ring out as the game always
  has: Alpha 2's tactics for the modes made no difference here CPU against CPU, and the coach did worse.
  `tools/aiscripts.py sfa` prints Alpha 1's scripts (its tables found by `lea (pc)`, its skipping routines' `move.w
  (a0)+,d1` runs counted).
- **A stuck random number in the fighting modes** (survival, the endless battle, the ring out) of Alpha 1-3 and the
  Vampire Savior games: a match begun from the character choice was given a random number of the toolkit's, and 130
  of the 65,536 are ones the game's own routine keeps going round between two (or one), so every choice it made at
  random came out the same. In Alpha 1's ring out the computer then broke in on itself without end, and the game hung
  (1 test run in 36). Now the game's own number is stepped on so many times at random (`Description::randomNext`,
  `battle::CapcomRandomNext`).

- **Zoom** is gentler: the size eases more slowly and never faster than about 0.5x a second going in (1.2x coming
  out), and the camera keeping both fighters in the picture goes closer only when it could go 0.15x closer and eases
  its aim between them, so a fight's every step does not move it (it felt nauseating). A fighter can leave the picture
  for a moment on a quick jump away while it catches up.
- **Multi game**: the rest of the Display page every game has, for the whole run: **Invisible walls**, **Scaling**,
  **Picture shape** and **Window size**, each also "As each game is set" (the default), given after the game's own
  settings as Screen and Picture are, and kept nowhere. In a window, the next game opens where the one before was and
  as tall (the game ending for the next writes its window's place to a file in the temporary folder, which the next
  reads within the minute and removes); it opened where the system puts a new window, so a window moved stayed behind.
- **Zoom on both fighters** in every fighting game: the left stick's click held and the right stick's click (keyboard:
  Left Shift and W) keeps both in the picture, the camera as close as shows both whole (2x at most, the whole picture
  at least): tight when they are close, wide when they are apart, kept inside the stage. Again for the whole picture;
  a shoulder looks at one of them instead. How much room a fighter takes is the game's (`Game::focusLift`,
  `focusTall`; `modes::Focus(place, lift, tall)`): the Vampire Savior games 128 lines tall, Alpha 1-3 112, Super Turbo
  104, Super Gem Fighter 76, the Marvel engine 136. `--zoom 1:3` and `--keys TICK:z2` for tests.
- **Street Fighter Alpha 3's CPU plays the tag style and the ring out itself**, as Alpha 2's and the Vampire Savior
  games' do: its computer player is readable C++ now (`games/sfa3/src/computer.cpp`, Alpha 2's interpreter with Alpha
  3's own: the isms' super combos and custom combos, a throw instruction, cancels), checked call by call against the
  game's code with nothing differing (about 1.45 million calls), and it has the port's tactics for the modes. CPU
  against CPU it won 53% and 64% of ring outs against the game's own play (47% and 53% with nobody playing the mode;
  the coach 53% and 45%). In the tag style it tags only while the other is knocked down: in this game every tag
  costs the side that makes it (the coach's too), and that costs least. `tools/aiscripts.py sfa3` prints Alpha 3's
  scripts, its own instructions named, and `--check` says whether a script has what a skipping routine of its does not
  know inside an if.

- **Street Fighter Alpha 2's CPU plays the tag style and the ring out itself**, as the Vampire Savior games' does: its
  own scripts, run by its interpreter (`games/sfa2/src/computer.cpp`), have three tactics of the port's: out of a
  corner (a throw, a jump over you, a walk out), knocking you over your end (a throw or a sweep), and in the tag
  style backing off and tagging; its own guard on all the while, which the toolkit's coach (now Super Turbo's alone)
  cost it by taking its controls. CPU against CPU it won 64% and 59% of ring outs against the game's own play (53%
  and 47% with nobody playing the mode; the coach 56% and 59%). `ComputerModeOf` says no mode for a game whose
  computer does not play the modes itself. `tools/aiscripts.py` names Alpha 2's jumps' ways as its jump reads them
  (1 forward, 0 straight up, FF back), and the tests of the other's jump with them.

- **Zoom** in every fighting game: Street Fighter Alpha, Alpha 2, Alpha 3, Super Turbo, Super Gem Fighter, Marvel
  Super Heroes and X-Men vs. Street Fighter, as in the Vampire Savior games (the left stick's click and a shoulder;
  the d-pad to frame the fighter; Left Shift, Q, E and the arrows on the keyboard). The toolkit's `modes::Focus`
  (`fight/alternative.h`) gives a game its `Game::focus` from where its fighter stands: only while a round is fought
  and not in the model viewer, looking so many lines above the feet (Alpha 1-3 44, Super Turbo 40, Super Gem Fighter
  28, the Marvel engine 40, for the size of their fighters). Each game's place is its model viewer's with the wide
  picture's columns counted (`widescreen.cpp`); the Marvel engine's is in `fight/marvel.cpp`, from where it puts the
  training mode's boxes, a tag-team game's point characters being its first two records.
- **Zoom**: the camera keeps its fighter in the middle at every size, past the stage's ends too (black there); it
  was kept inside the picture, so near an end the fighter was off to the side, and it trailed a dash. It eases only
  when it changes fighter or size. With the left stick's click held, the d-pad or the stick moves it to frame the
  fighter as the player wants (kept until the zoom is off or looks at the other fighter; `Zoom::Pan`), and the keyboard has the zoom too: Left
  Shift held, Q and E for players 1 and 2, the arrows to move it. Vampire Savior's looks a little lower (44 lines
  up from the feet, not 56), so at 2x the feet and their shadow are in the picture.
- Street Fighter Alpha 2's computer player as readable C++ (`games/sfa2/src/computer.cpp`): the script interpreter
  (ComputerThinks) and the routines the fighters' code calls (Chance, BrokenOff, Reactions, AnswerNow), checked call
  by call against the game's code with nothing differing (about 1.9 million calls, every instruction and test the
  scripts have). The CPU plays as before.
- **Zoom** (Vampire Savior, Definitive, Vampire Savior 2, Vampire Hunter 2): a gamepad's left stick's click held
  and the left shoulder pressed zooms the camera in on player 1's fighter, the right shoulder on player 2's, a size up
  each press (1.25x, 1.5x, 1.75x, 2x, the model viewer's sizes), then back to the whole picture. The camera eases
  from size to size and follows the fighter; "ZOOM 1.5x  PLAYER 1" is said at the bottom for a few seconds, and
  outside a fight the picture goes back to its own size. It is the window's alone: the game, its save states,
  movies and screenshots are as they were. While the click is held the shoulders are not the game's (the right one
  is hard punch), and with a zoom to work the click alone opens the menu when it is let go, not as it goes down.
  The port draws part of the picture larger (the shader's `view`, `port/present.h` `Zoomed`) and the readout's
  panels over it at their own size; a game's sources say where to look with `Game::focus` (`runtime/game.h`;
  `port/zoom.h`). `--zoom SIZE:PLAYER` starts zoomed, for tests and `--present-test`.
- **Multi game**: the **Picture** of every game in a run (widescreen, 4:3, or as each game is set) and a **Window**
  beside Full screen, so a run looks the same from game to game. For that run only: each game's own switches are
  left as they were (`--features` and the games' new `--window` after the launcher's own).
- **Multi game** on macOS: the next game comes to the front. It opened behind the other windows, since macOS brings
  forward only a program started by the one in front; the game ending for the next one hands the front to the
  launcher first (`port/front_macos.mm`), and every game raises its window as it opens.
- `tools/aiscripts.py sfa2` prints Street Fighter Alpha 2's CPU scripts: the same interpreter as Vampire Savior's,
  its own instructions named from its code (a second if with else if, the super combo, the Alpha counter, answers
  picked by where the other's attack reaches), checked against the places the game's interpreter keeps in five runs
  (games/sfa2/PROGRESS.md, "The computer player: scripts").
- **Linux ARM64**: a download for 64-bit ARM Linux (aarch64: ARM handhelds, single-board computers, ARM
  Chromebooks' Linux), `CPS2Recomp-linux-arm64.tar.gz`, beside the x86_64 one, and its launcher updates itself from
  `update-linux-arm64.txt` (issue #99). CI builds and starts it on an ARM64 runner. (`tools/release/linux.sh` builds
  for the processor it runs on; `tools/release/all.sh` makes it in an ARM64 build image.)
- **The Vampire Savior games' CPU plays the tag style and the ring out itself** (Vampire Savior, Definitive, Vampire
  Savior 2, Vampire Hunter 2): its own scripts, run by its interpreter (`computer.cpp`), have three tactics of the
  port's, chosen in its own way as the fight calls for them: out of a corner (a throw that sends you behind, a jump over
  you, a walk out), knocking you over your end (a throw or a sweep), and in the tag style backing off and tagging. It
  keeps its own guard all the while, which the toolkit's coach (still Alpha 2's and Super Turbo's) cost it by taking
  its controls. CPU against CPU it won 64-66% of ring outs against the game's own play (the coach: no gain on the same
  runs). `battle::Description::computerPlaysModes` and `ComputerModeOf`, `ComputerBack`, `ComputerTagWanted`,
  `ComputerSkill`, `ComputerAsksTag` (`fight/battle.h`). `tools/aiscripts.py` reads a normal's button and strength as
  the game does (it had them a bit off: the throws are heavy and medium punches).
- Street Fighter Alpha 3's widescreen is on in the alternative modes' fights too (survival, the endless battle, the
  ring out, the tag team battle), as in Alpha and Alpha 2; only arcade, versus and training had it (issue #109).
  Alpha 2's README said its modes were 4:3; they were already wide.
- **Multi game**: in its endless battle, **N** (or a gamepad's left shoulder) fades out to the next game, and **F** (or
  the left trigger) held runs the game four times as fast, without sound ("FAST FORWARD x4" at the bottom); each only
  when the player has not given it to the game. The Multi game page lists them under While it plays, and in Now
  playing (recomp-common: `MultiButton`). (`port/frontend_sdl.cpp`, `Port::hurry`, `runtime/game.h` `AskNextGame`.)
- The ring out shows its score (Ring outs: N) at the top all the while it plays, as survival does, not only when it
  is over.
- **The Vampire Savior games' computer player is readable C++** (`games/vampire_savior/src/computer.cpp`): the
  interpreter of its scripts (choosing a plan, a tactic, a reaction; the instructions and the if's tests; each
  instruction looked at frame by frame until it is over) and fourteen routines the fighters' own code calls in it,
  about 1,900 instructions of the game's. Each is checked call by call against the game's own code, in Vampire
  Savior, Definitive, Vampire Savior 2 and Vampire Hunter 2 (the last two's code differs in four places, which their
  `family.h` names), over the computer's movies, the attract mode's fights and ladders at levels from lowest to
  highest. Nothing plays differently: it is the ground for a computer that plays the alternative modes itself.
- **Another game: Street Fighter Alpha 3** (`games/sfa3`; ROM set `sfa3u`, USA 980904, the last revision and the one
  tournaments and Fightcade play; issue #89). The game as the board plays it, translated whole (its 68000 program and
  its sound program, Capcom's driver 1.71) and checked, with everything Street Fighter Alpha 2's port has: a **training
  mode** (hit boxes and the throw range, frame data, its meter and log, input display, combo, stun and custom combo
  readouts, fixed damage, health refills, a dummy that records, replays, guards high or low as each attack needs, Alpha
  counters, techs throws, rises quickly, mashes out of stun or is played by the game's own computer player, start over,
  any of the 26 stages), the **alternative modes** (attract mode three ways, the stage viewer and its infinite stage,
  the model viewer, the colour editor, survival, endless battle, ring out, the **tag team battle** in the knock-out and
  the Marvel-style tag style, co-op and 2 vs 2), **widescreen** and invisible walls, **custom colours** (files, PalMod's
  sz3.09c, the editor, a set for each ism and button), the **hidden characters** with no code (Start held as you choose
  M. Bison, Cammy or Rose: Balrog, Juni, Juli), and the features every game has: free play (the game's own), straight
  to the title, the board's settings (the Board page: difficulty, damage, timer and game speed, the rounds for one and
  two players, event, join-in, continue, demo sound), cheats (no damage, full gauge and no stun for each player,
  one-hit knockouts, infinite time), save states, run-ahead, arcade timing and a custom soundtrack (its music named)
  ([its progress](games/sfa3/PROGRESS.md)).
- The custom soundtrack takes `musicBelow` for a sound program whose songs do not say which are music (driver 1.71's).
- Widescreen's holes (where no plane is painted) are filled from the stage alone, before the objects are drawn: a
  fighter or the panel beside one is no longer mirrored into it.
- **Multi game**: a new page of the launcher's, for modes that play several games one after another. Its first,
  **Multi-game endless battle**, goes through every stage of a fighting game with the game playing both fighters,
  then does the same in the next game, and round again: the player chooses which games (any of the eleven fighting
  games), their order (the sidebar's or shuffled), the stages' way (Shuffle: in turn at random, each fading into the
  next after 30 seconds to 5 minutes; Scroll: going by in order, slow, medium or fast) and full screen. A game is
  started with `--multi endless:...` and ends with exit code 3 when its last stage is done (`runtime/game.h`,
  `MultiGame`, `EndProgram`); quitting it stops the run. (recomp-common: `Collection::multi`.)
- **Another game: Super Street Fighter II Turbo** (`games/ssf2t`; ROM set `ssf2xjr1`, *Super Street Fighter II X:
  Grand Master Challenge*, Japan 940223, the revision tournaments and Fightcade play; issue #88). The game as the board
  plays it, translated whole (its 68000 program, the characters' code past the first megabyte too, and its sound
  program, Capcom's driver 1.04) and checked, with everything the other fighting games have: a **training mode** (hit
  boxes, frame data, its meter and log, input display, combo and stun readouts, health and gauge refills, a dummy that
  records, replays and guards, start over, any of the sixteen stages), the **alternative modes** (attract mode three
  ways, the stage viewer and its infinite stage, the model viewer, the colour editor, survival, endless battle, ring
  out), **widescreen** and invisible walls, **custom colours** (files, PalMod's sfxj.04a, the editor), the **hidden
  characters** with no code (the three punches at once as a character is chosen: Akuma; any two buttons: its old
  version), and the features every game has: free play (the game's own), straight to the title, the board's settings
  (the Board page: difficulty, game speed, continue, demo sound), cheats, save states, run-ahead, arcade timing and a
  custom soundtrack (its music named) ([its progress](games/ssf2t/PROGRESS.md)).
- **Super Street Fighter II Turbo: the tag team battle**, as Street Fighter Alpha 2 and Vampire Savior have it (the
  tag style and the knock-out style, teams of two or three, against the CPU, co-op or 2 vs 2), and the **CPU that
  plays the tag style and the ring out as a person would**: it tags its fighters in, and in a ring out walks its
  opponent to an end and knocks it over, keeping off its own. The input display's directions were mirrored (the
  game's facing byte was read the wrong way round); they are the player's own now.
- The fighting modes (`fight/battle.h`) take `handOver`, for a game whose computer and person work a move under way by
  bytes of their own (Super Street Fighter II Turbo: a fighter changes hands only standing or crouching), and
  `speedWords`, for speeds kept in words rather than longs.
- The fighting modes (`fight/battle.h`) take `ownMaster` for a game that keeps no one byte saying a player is its own
  master (Super Street Fighter II Turbo: several of its states, and guarding only before a hit lands).
- **Your games in your order**: drag a game's row in the launcher's sidebar up or down (or, with a gamepad or the
  keyboard, **Move up** and **Move down** on its page, under Play) to keep the ones you play most at the top. The
  Games page lists them in the same order; it is kept in `launcher.ini` (issue #91).
- **The launcher fits a small window** (issue #73): on a small screen, or with large text, it no longer opens
  bigger than the screen or leaves its rows' words under their buttons. A narrow window has a narrower sidebar
  (long names cut short with "…") and less room round the page, and a row's buttons go under its words when there
  is too little room beside them. The sidebar's list of games scrolls when it is longer than the window (twelve
  games already were at the usual size). The window can be made as small as 600×400.
- The launcher's Games page: the Updates card is at the top, above the games, and each game's **Play** is before its
  **Choose…**, so that a gamepad's ring coming into the row is on Play.
- **The CPU plays the tag style and the ring out as a person would** (Street Fighter Alpha 2 and the Vampire Savior
  games). The game's computer player knew nothing of either: it tagged only when its bar emptied, and fought a ring out
  for damage. Now a side the game plays is coached by the toolkit (`fight/battle.cpp`). In the tag style it tags when
  the one on is low and a partner has more (or has rested), best while the other fighter is down or far off, never just
  out of a hit and not straight after its last tag, and tags in the partner with the most health. In a ring out it
  walks the other towards an end and throws or sweeps it there (a throw held away sends the other on, towards its end),
  and keeps off its own end: it gets out from near it with a jump over the other or a throw that swaps sides, never into
  an attack coming (the game's own guard test; the game's play guards it), and the game's own walk does not take it back
  towards that end (a line in Alpha 2's Walking rewrite, a hook on Vampire Savior's walk). How soon it sees a chance and
  how often it takes one go by the difficulty. Additive in the toolkit (`battle::Description::canAct`, `difficultyNow`,
  `attackComing`; `RingOutMayWalkBack`); with neither mode on, every movie plays exactly as before. For tests:
  `CPS2RECOMP_BATTLE_CPU=1` (both sides the game's, the mode begun again when over), `CPS2RECOMP_COACH`,
  `CPS2RECOMP_COACH_PARTS` and `CPS2RECOMP_BATTLE_LOG`; the numbers are in each game's PROGRESS.
- **Portable mode** (issue #65): an empty `portable.txt` beside the launcher (on macOS, beside the app) keeps
  every game's settings, save states, board memory and logs in `user/` there instead of your user folder, so the
  collection can live on a drive you carry. The first time, the launcher offers to copy what your user folder
  has (copying only: nothing is moved or written over); taking `portable.txt` away goes back to your user folder
  as it was. Updates keep `portable.txt` and `user/`, and a folder that cannot be written to is said, not
  worked around. The Games page's new **Your files** card shows where everything is kept and opens the folder. A
  game's own program, opened by itself, finds the launcher's settings in the same place. (recomp-common
  `src/folders.h`.)
- The release's checks (`tools/release/checks.sh`) stop at once if a game's code has not been found yet (no
  `work/<id>/entries.txt`), where they ran every step before and failed `check` and `check-blocks` for that alone.
  `tools/seed_entries.sh <another tree>` gives a fresh tree the code another found, so that it needs no
  `make explore`.

## 0.4.0-beta.6

A twelfth game, Marvel Super Heroes, with everything the other fighting games have, on a part of the toolkit it shares
with X-Men vs. Street Fighter. A tag team battle for the Vampire Savior games and Street Fighter Alpha 2: teams of two
or three, tagged in Marvel-style or one knock-out at a time, against the computer, in co-op or 2 vs 2. And widescreen's
switch on the launcher's first page for every game that has widescreen.

- **Another game: Marvel Super Heroes** (`games/msh`; ROM set `msh`, *Marvel Super Heroes*, Europe 951024, the parent
  set and the one competitive play uses). The game as the board plays it, translated whole (its 68000 program, and its
  sound program, Capcom's driver 1.05) and checked, with everything the other fighting games have: a **training mode**
  (hit boxes, frame data, its meter and log, input display, combo readout, health and gauge refills, all six Infinity
  Gems at hand, a dummy that records, replays and guards, start over, any stage), the **alternative modes** (attract
  mode three ways, the stage viewer and its infinite stage, the model viewer, the colour editor, survival, endless
  battle, ring out), **widescreen** and invisible walls, **custom colours** (files, PalMod's msh.05, the editor), the
  **hidden characters** with no code (Start held as a character is chosen: a punch Dr. Doom, a kick Thanos), and the
  features every game has: free play (the game's own), straight to the title, the board's settings (the Board page:
  difficulty, damage, timer, rounds, mode, event, join-in, continue, demo sound), cheats, save states, run-ahead,
  arcade timing and a custom soundtrack (its music named) ([its progress](games/msh/PROGRESS.md)).
- **The Marvel games' engine** (`fight/marvel.h`): what X-Men vs. Street Fighter's and Marvel Super Heroes' sources
  would write alike (the training aids' view of the players and their boxes, the ring out's movements, widescreen and
  the walls, the model viewer) is now the toolkit's, from a description of each game. X-Men vs. Street Fighter runs on
  it exactly as before (the same hash logs over its ladder and every mode).
- The model viewer names a game's animation groups after Z as AA, AB... (Marvel Super Heroes has up to thirty).
- For tests: `CPS2RECOMP_BATTLE_SEED` gives survival, the endless battle and the ring out the same choices every run,
  and `CPS2RECOMP_WATCH_REPORTS` raises `--watch`'s limit of 2,000 reports; `battle::Description::resultNow` for a game
  that keeps a round's result its own way.
- **Tag team battle** (`fight/battle.h`; Vampire Savior, Vampire Savior 2, Vampire Hunter 2 and Vampire Savior
  Definitive): a mode on the Alternative modes page with two teams of two or three, chosen on its page. One fighter of
  each team at a time, all on one stage, each fighter on one health bar (a team of two has two between them, as one
  fighter has in a usual match; or as many as the board gives): the one knocked down is out and the next of that team
  comes on with a full bar, and the winner stays on with the health it has left (and a little back). Team 2 is the CPU
  (with a difficulty of its own on the page), or player 2. Fighters begin a match ready, with no entrances, and the same
  character twice or more in a match, on either team, is in a set of colours of its own.
  With "Tag team" on the same page each fighter is a person's own (co-op against the computer's team, or 2 vs 2 for
  four players): the tag-team router (`fight/tag.h`) follows whose fighter is on, and the Controllers page has four
  players. Off, every game is as it was.
- **The team battle's tag style** ("Style: Tag" on its page; the same four games): Marvel-style, each fighter with a
  bar of its own, tagged in with a gamepad's left trigger (next to HP and HK on the right) or Start whenever the
  fighter on can act: the one on calls the next one in (a voice of
  its own), jumps back and flies off the
  picture, and the next jumps in from that side with a jump attack that can hit (the game's own jump and attack); a
  bar emptied knocks its fighter down (the game's own knock-down, landing lying, the announcer's call as at a match's
  end), where it lies two seconds and cannot
  be hit (time for a setup), and the next jumps in; resting fighters get back the health that comes back by itself; the team's super gauges carry across a
  tag; the others' health listed under the bars; a time over decided by the teams' health in all. In co-op and 2 vs 2
  the partner tags in with their own left trigger. (Marvel vs. Capcom's HP+HK is a Dark Force here, as is every punch
  and kick of one strength; two punches or two kicks make the EX moves: the left trigger is free.) The game sets a fighter up anew itself (its own setting up of a player,
  mid-fight), after what the outgoing character made is given back; every pair of characters swapped back and forth
  under made-up play in Vampire Savior, Vampire Savior 2 and Vampire Hunter 2 (602 pairs, 3,010 swaps) with no fault.
- The team battle's **tag style is the default** in every game that has it (its page's Style row: Tag first, then
  Knock-out). A Style chosen before this reads the other way once.
- **Controls for each game** (issue #61; `port/bindings.h`, recomp-common's launcher): the collection's Controllers
  page has Layout for, every game or one of them, and Its own layout gives that game keys and buttons of its own,
  with its own names for its buttons (Attack, Jump, Shoot), starting as every game's are; switched off, it has every
  game's again. Every other game keeps the shared layout.
- **All three punches, all three kicks**: two more actions on the Controllers page for every fighting game (and the
  collection's shared layout), each pressing three buttons at once, for the moves and the supers that take them. No key
  or button has them until one is given (`ppp` and `kkk` in `settings.ini`).
- **Tag team battle in Street Fighter Alpha 2**, both styles, as the Vampire Savior games have it: teams of two or three
  on one stage, the knock-out style and the tag style (the left trigger or Start tags; the one on calls the next one in,
  where its character has a call of its own, jumps back and flies off, and the next jumps in with a jump attack; a bar
  emptied is the game's own knock-down, lying two seconds and not to be hit, with the announcer's call), co-op and 2 vs
  2, no entrances, a CPU difficulty of its own, every copy of a character in its own colours, the super combo gauge
  carried across a tag, one round deciding a tag match and a time over by the teams' health. "Hidden characters" holds
  for fighters tagged in. The game sets a fighter up anew itself (`games/sfa2/src/livetag.cpp`), after what the
  outgoing character made is given back and with its name under the bar put right; every pair of the eighteen
  characters (and to each hidden form) swapped back and forth under made-up play, 756 pairs and 5,292 swaps, with no
  fault.
- Fixed: **widescreen's switch on the launcher's first page** (under "Full screen") in Street Fighter Alpha, X-Men vs.
  Street Fighter, Super Gem Fighter Mini Mix and Marvel Super Heroes. Their widescreen was only on the in-game menu's
  Display page; every game with widescreen now has it in both places, as its page says
  ([#84](https://github.com/tmcleroy/cps2recomp/issues/84)).

## 0.4.0-beta.5

An eleventh game, X-Men vs. Street Fighter, the first tag-team game here, with everything the other fighting games
have: a full training mode, attract mode, the stage and model viewers, the colour editor, survival, endless battle,
ring out, widescreen, custom colours and the hidden character. It also has tag-team play for up to four people: co-op
against the computer, or 2 vs 2, each person playing a character of their own, from "Tag team" on the Alternative
modes page.

- **Another game: X-Men vs. Street Fighter** (`games/xmvsf`; ROM set `xmvsf`, *X-Men Vs. Street Fighter*, Europe
  961004, the parent set and the one competitive play uses), the first tag-team game here. The game as the board plays
  it, translated whole (its 68000 program, and its sound program, Capcom's driver 1.16b) and checked, with everything
  the other fighting games have: a **training mode** (hit boxes, frame data, its meter and log, input display, combo
  readout, health and gauge refills, a dummy that records, replays and guards, start over, any stage), the
  **alternative modes** (attract mode three ways, the stage viewer and its infinite stage, the model viewer, the colour
  editor, survival, endless battle, ring out), **widescreen** and invisible walls, **custom colours** (files, PalMod's
  xvs.05a, the editor), the **hidden character** with no code (Start held on Ryu: Akuma), and the features every game
  has: free play (the game's own), straight to the title, the board's settings (the Board page), cheats, save states,
  run-ahead, arcade timing and a custom soundtrack (its music named) ([its progress](games/xmvsf/PROGRESS.md)).
- **Tag-team co-op and 2 vs 2** (`fight/tag.h`, issue #69; X-Men vs. Street Fighter first): each character of a team
  played by a person of its own, from "Tag team" on the Alternative modes page. Co-op: players 1 and 2 against the
  computer; 2 vs 2: players 1 and 3 against 2 and 4. Each person chooses their own character, the controls follow the
  game's own point character (tags, knock-outs, team supers), the partner tags in with HP+HK, and a label under the
  bars says who is on point. No patched ROM. The game's Controllers page has four players. Off, the game is the same as
  before frame for frame.
- **The fighting modules for games built otherwise** (`fight/alternative.h`, `fight/session.h`, `fight/battle.h`): a
  game whose steps are not the Alpha games' four words says itself when a round is fought, the players choose and a
  game runs; a sound queue of another size; a camera whose words move with the stage, with ends that differ by stage;
  a camera word that is not the picture's left edge, and heights that grow downwards (the ring out); a computer byte
  that is 0 for the computer; a tag team's partner (MatchWanted::partners, and the partner told who plays it). All
  optional: the other games' scripted runs are the same as before.
- **Fix: run-ahead in a training session's character choice** (`fight/session.cpp`): run-ahead's frames no longer
  move on what player 1 held as it chose, so player 2's choice is the same with run-ahead as without.

## 0.4.0-beta.4

A tenth game, Super Gem Fighter Mini Mix (*Pocket Fighter* in Japan), with everything the other fighting games have:
a full training mode, attract mode, the stage and model viewers, survival, endless battle, ring out, widescreen, custom
colours and the hidden characters. Street Fighter Alpha gets all of that too. Alien vs. Predator and Cadillacs and Dinosaurs
gain the beat-'em-up modes: attract mode, score attack, and stage and character viewers. Both also get a custom
soundtrack of your own music, and Alien vs. Predator's "Straight to the title" now works with free play on.

- **Another game: Super Gem Fighter Mini Mix** (`games/pocket_fighter`; ROM set `sgemf`, *Super Gem Fighter: Mini
  Mix*, USA 970904, the parent set and the one competitive play uses; *Pocket Fighter* in Japan). The game as the
  board plays it, translated whole (its 68000 program, and its sound program, Capcom's driver 1.30) and checked,
  with everything the other fighting games have: a **training mode** (hit boxes, frame data, its meter and log, input
  display, combo and stun readouts, health and gauge refills, a dummy that records, replays and guards, start over, any
  stage), the **alternative modes** (attract mode, the stage viewer and its infinite stage, the model viewer, the colour
  editor, survival, endless battle, ring out), **widescreen** and invisible walls (the camera's place and keeping the
  players inside the picture rewritten as readable C++, checked call by call), **custom colours** (files, PalMod's
  pcf.07, the editor), the **hidden characters** with no code (Start held on Ryu: Akuma; on Ken: Dan), and the features
  every game has: free play (the game's own, one of the board's coin settings), straight to the title, the board's
  settings (the Board page), cheats, save states, run-ahead, arcade timing and a custom soundtrack (its music named)
  ([its progress](games/pocket_fighter/PROGRESS.md)).
- **Street Fighter Alpha: everything the other fighting games have** (`games/sfa`), on the toolkit's shared modules:
  a **training mode** (hit boxes, frame data, its meter and log, input display, combo and stun readouts, health and
  gauge refills, a dummy that records, replays and guards, start over, any stage), the **alternative modes** (attract
  mode three ways, the stage viewer and its infinite stage, the model viewer, the colour editor, survival, endless
  battle, ring out), **widescreen** and invisible walls (the camera's place and keeping the players inside the picture
  rewritten as readable C++, checked call by call), **custom colours** (files, PalMod's sfz.06, the editor), the
  **hidden characters** with no code (Start held on Ryu: Akuma; on Sagat: M. Bison; on Ken: Dan), and the music's
  names for the custom soundtrack ([its progress](games/sfa/PROGRESS.md)).
- **A panel that is a scroll screen** (`fight/battle.h`, `panelLayers`; `Video::markLayers`): the endless battle and
  the ring out lay a game's own bars and names over the stage another machine shows, where the panel is not objects
  (Street Fighter Alpha's). No other game's picture changes.
- **The Alternative modes page and the training session in the toolkit** (`fight/alternative.h`, `fight/session.h`):
  what every fighting game had as its own copy (the page, attract mode, the stage viewer, getting a machine to a
  round; the session, start over, the aids put together) is one module each that a game describes itself to. Vampire
  Savior (and the games built from its sources) and Street Fighter Alpha 2 are on them, every scripted run of their
  modes and training the same as before, picture for picture ([FIGHT.md](docs/FIGHT.md)).
- **Alien vs. Predator and Cadillacs and Dinosaurs: alternative modes**, at the head of the in-game menu as the
  fighting games have them:
  - **Attract mode**: the game's own show on a loop, with its sound on and no coin in: all of it, or only the story,
    only the demonstrations, or only the characters' files.
  - **Score attack**: one player and one credit from the first round, with no continue and none of the cheats, free
    play or "Start at round": the board's own game for the best score, which is kept (`score_attack.txt` beside
    `settings.ini`) and shown as it begins and ends.
  - **Stage viewer**: each stage with nobody on it (no players, enemies or panel), only the stage and its music,
    scrolled with the stick or by itself, slowly to fast, or every stage in turn. Alien vs. Predator's are its rounds'
    scenes; Cadillacs and Dinosaurs' are the parts of its episodes a game can be continued from, the camera going up
    and down where the episode does (the climb up the building, the shaft in the basement).
  - **Model viewer**: each of the four characters by itself, on a stage or on plain white or black, at a quarter
    of its size to 25 times, moved with player 1's controls or going through any of its animations (by the game's
    lists of them), in slow motion if you like, one or all of them in turn.
- **A custom soundtrack for both**: their song tables found (`games/avsp/src/music.cpp`, `games/dino/src/music.cpp`),
  and the songs heard in attract mode, each round or episode and a continue named in `songs.txt`.
- **The beat-'em-up modes in the toolkit** (`beat/beat.h`, `beat/modes.cpp`, `beat/model.cpp`): one copy for every
  beat-'em-up, which says how its game gets to a stage, where its camera and its characters' animations are, and
  which of its objects are a character's. The CPS-1 board's picture can now mark which pixels are objects'
  (`Video::markObjects`, as the CPS-2's could).
- **One model viewer for every game** (`model/model.h`): Street Fighter Alpha 2's, the Vampire Savior games' and the
  beat-'em-ups' model viewer were three copies of the same thing; now each game says only how it gets to a
  character alone and where its animations are. The fighting games' viewers are as they were, frame for frame.
- **The model viewer's "Play all" crashed the beat-'em-ups**: on Alien vs. Predator a list's last entries are not
  always animations (Kurosawa's crashed it), and on Cadillacs and Dinosaurs each list's end mark was taken for one, as
  was Jack's A29, which is not one; they are passed over now.
- **The beat-'em-ups' model viewer** (it was their "Character viewer": it has the fighting games' name now):
  - Starting it a second time, or choosing another character or background while it was on, began it again every
    frame, the game going round its start, title and choice of character until it was quit: beginning it ended the
    viewer it had just begun (`beat/model.cpp`).
  - The character is in the middle of the picture, as in the fighting games: it was where the game had put it, at
    the edge or out of the picture on many stages, and at the larger sizes, which are made about where it stands,
    off to one side or out of the picture altogether (that place was 64 pixels out on Alien vs. Predator and read
    from the wrong word on Cadillacs and Dinosaurs). It stays in the middle until moved with player 1's controls,
    and the camera stays where it was as the viewer began: on Cadillacs and Dinosaurs it went on following the
    character, scrolling the stage, and the character slid from side to side as it did.
  - Cadillacs and Dinosaurs: the episodes' walk in at their start no longer walks the character on the spot a while
    after the viewer has begun, and things the enemies throw, which can be in a character's colours, are no longer
    drawn as part of it unless they touch it.
  - It is unseen until the character is there and standing (on Alien vs. Predator, until the camera has stopped
    going up or down after it): the stages that bring the players in late, or with an
    entrance (thrown in, dropped from above), showed an empty stage or the character lying down for several seconds.
  - Cadillacs and Dinosaurs: the character is out of harm (unseen things falling knocked it down over and over on some
    stages); the second and third parts of Jack's Garage and the second of the Swamp Forest no longer leave it in the
    episode's entrance for good; the Swamp Forest's second part has the stage viewer's camera, the game's own showing
    its rows out of place.
  - Alien vs. Predator: the character's body no longer drops out of every fourth picture or so on the busier scenes
    (the game, slowed down, shows its objects a frame later).
- **Alien vs. Predator's "Straight to the title"** ran only the board's first second and a half unseen when free play
  was on: the show's step is at FF8000, not FF8002 (`games/avsp/src/avsp.h`, kMode).

## 0.4.0-beta.3

- **Another game: Street Fighter Alpha** (`games/sfa`; ROM set `sfa`, *Street Fighter Alpha: Warriors' Dreams*,
  Europe 950727). Its 68000 program and its sound program (driver 1.05A) translated whole, checked block by block
  over the ladder and versus movies; with free play, straight to the title, the board's settings (Board page), cheats
  (no damage for each player, one-hit knockouts, infinite time), save states, run-ahead, arcade timing and a custom
  soundtrack (its songs named by number for now). Its sources are its own: Alpha 2 is not made from its program (the
  routines Alpha 2's sources hook are mostly not in this game), so no training mode, alternative modes, widescreen or
  custom colours yet.
- **macOS: SDL 3.4.16 or later** for a build from source (CMake asks for it; docs/BUILDING.md). SDL 3.4.14 (Homebrew's
  until recently) leaves every game's window black while the game runs and sounds: its Metal GPU device takes a
  frame's fence still being worked on for one signalled, so the swapchain's pictures after the first few are all
  skipped (SDL 3.4.16: "Fixed the SDL_QueryGPUFence() return value on Apple platforms"). Release builds compile 3.4.16
  themselves and were not affected. Linux and Windows are unchanged.
- **A release has only its three downloads**, their checksums and three small update lists (it had a file for
  every file of every download too, 46 more). The launcher's **Update** now downloads the system's download
  itself, checks it, and puts in place what changed (recomp-common 91b4e51; `update-<system>.txt` names the
  archive, `tools/release/lib.sh`). **A launcher of 0.4.0-beta.2 can't update itself to this version** (it reads
  only the old lists and says the list of files could not be read): download it by hand once, and later versions
  come through Update again.
- **Each release's page says how to install it** on macOS, Windows, Linux and the Steam Deck and Steam Machine,
  above its changes (`tools/release/notes.sh`, used by the release workflow).

## 0.4.0-beta.2

- **Vampire Savior Definitive** no longer sometimes draws a screen of wrong tiles: the scroll screens looked up
  whether a tile was the extra set's at the graphics' offset instead of the map's place, reading past the end of
  that list (`video/video.cpp`, DrawScroll; the CPS-1 games' tile offset had taken the map base's name).

- **Ring out, in Street Fighter Alpha 2 and the Vampire Savior games**: a fighter knocked over the end of the stage
  is thrown back with the game's own knock-down animation (and in Street Fighter Alpha 2 its cry), and falls
  through the floor and out of the picture (before, its picture slid down); it is out as soon as it is past where
  the board would have stopped it, while still in sight. The winner jumps off, and at the top of its jump goes on
  in the next match as it was (place, picture, speeds): the camera follows it down past the old stage to the next
  one's floor, the winner in front of both, and it lands as from any jump. (Vampire Savior's way down faded
  through black; it is the same as Street Fighter Alpha 2's now.) The winner no longer stands about waiting for the
  next match: each match and stage after the first is begun from a machine kept at the character choice (about 500
  frames to its round, not 2,000 from switching on), and made ready twice as fast while the one out falls. In Street
  Fighter Alpha 2 the player's colours stay the ones chosen from match to match (they went back to the first set). Each
  match begun from the character choice is given a random number of its own, so the computer's choices do not repeat
  from one match to the next.
- **The fighting modes in the toolkit** (`fight/battle.h`): survival, the endless battle and the ring out, one
  copy for every fighting game, which describes its memory and makes the ring out's two movements at its players'
  step (`games/sfa2/src/player.cpp`, PlayerFighting; `games/vampire_savior/src/battle.cpp`, RingOutStep from the
  player step hook). The camera may be held beyond a stage's ends (`CameraWanted::beyondEnds`).

- **Updates in the launcher**: it checks this repository's releases when it opens and offers a newer version at the
  top of the Games page; **Update** downloads only the files that changed (a new game: its program and the
  launcher), checks each against the release's SHA-256, puts them all in place or none, and starts the new launcher
  (recomp-common 1ea2766, `launcher/src/update.h`). The release scripts write what it reads: each system's
  `update-<system>.txt` and every file gzipped under its SHA-256 (`tools/release/lib.sh`, `update_files`;
  docs/RELEASING.md). The first release with them is the first a launcher can update to: a copy older than this
  feature is updated by hand once.

- **Vampire Savior Definitive** (beta; `games/vsav_definitive`; ROM sets `vsavj` and `vsav2`): Vampire Savior with
  Donovan, Phobos and Pyron taken from Vampire Savior 2 (its code, pictures and sounds), all eighteen characters
  by Vampire Savior's rules. At the character choice the three have places of their own, a row under the grid
  with their faces (and the Definitive page of the in-game menu);
  the training mode, the modes and the model viewer have them; the arcade goes to its end with them. The launcher
  gives it the `vsav2` set chosen on Vampire Savior 2's page (recomp-common 568e39f, `Launch::file_of`; `--rom2`).
  How: an extra ROM set mapped from A00000 (`game.cfg`'s `extra_set`), objects and scroll tiles drawn from its
  graphics, a second sound side with its sound program and samples (`machine/sound_board.h`), and `tools/graft.py`
  with the game's `graft.cfg` (tables by character, `extend`, `mask`, `divert`, `ram`, `patch`). Not yet: Donovan's
  stage and Vampire Savior 2's music.
- Hooks are numbered in 16 bits (from the 256th on they went to the wrong hooks).
- **Two beat-'em-ups for three players: Alien vs. Predator and Cadillacs and Dinosaurs** (`games/avsp`, ROM set
  `avspu`, USA 940520, a CPS-2 game; `games/dino`, ROM set `dino`, World 930201, a CPS-1 game with QSound). Each has
  three players at once, free play, a round or stage to start at, the board's settings on the Board page (Alien vs.
  Predator's Players setting lets the third join: on in Enhanced), infinite health, lives and time, straight to the
  title, and everything the port gives every game. Both translated whole and checked (`make check`).
- **A CPS-1 board with QSound** ("CPS-1.5") in the toolkit: `board = cps1` and `cps_b` in a game.cfg; its memory map,
  inputs for three or four players, 12 MHz, the objects in video RAM placed among the scroll screens by the layer
  control (with the layer below them keeping its high pens in front), the settings memory as 128 bytes, no program
  cipher, and the sound CPU's Kabuki cipher deciphered with a key worked out from the player's own ROM in about a
  second (`machine/kabuki.*`; no key is in the repository). The Z80 reads opcodes through `Z80Bus::ReadOpcode`.
- **Up to four players**: game.cfg's `players`, the CPS-2's third and fourth players' wiring, movies with a column
  for each player who presses something, run-ahead and the spare machines keeping every player. recomp-common's
  bindings take up to four players; the Controllers page has a button for each.
- **A game's own names for its buttons** (game.cfg's `buttons`, "Attack | Jump | Shoot"): its launcher shows them and
  only them, laid out on a gamepad as a beat-'em-up's are; in the collection every game shares one table, a fighting
  game's six with the other games' names beside them.
- **Rapid fire** (`rapid_fire`, Game page, every game): a held button pressed for two frames and let go for two, for
  the first button or all of them; the first press is not held back.
- `--dump-ram` may be given more than once. The launcher's words fit a game with no key file or training mode.

- **Arcade timing** (the `arcade_timing` feature, Game page, off unless chosen): the 68000 runs as on the board,
  its acknowledge a clock late for every access outside 500000-7FFFFF and its bus taken by the CPS-A's DMA (the row
  scroll, the scroll layers' tile rows, the palette), so a game slows down where and as much as on the board
  (`docs/TIMING.md`). Measured with jtcps2 simulated as a reference: without it the port's 68000 did about 1.35 times
  the board's work in a frame; with it, Street Fighter Alpha 2's work RAM is the reference's at 5,639 of 5,671 frames
  through a whole fight (the rest are registers kept aside, caught at a different point at the frame's end). With
  it on, the game's own code runs in place of the readable rewrites, but where training mode, the sources' own
  modes, widescreen or a cheat need them (`Game::rewritesNeeded`). With it off nothing changes. Save states are
  version 7 (5 and 6 still load). `CPS2RECOMP_BUS_LOG` logs each frame's waits and DMA.

- **Two more games: Vampire Savior 2 and Vampire Hunter 2** (`games/vsav2`, `games/vhunt2`; ROM sets `vsav2`, Japan
  970913, and `vhunt2`, Japan 970929). Capcom made both from Vampire Savior's program: Vampire Savior 2 brings back
  Donovan, Phobos (Huitzil) and Pyron, Vampire Hunter 2 is Night Warriors' cast on Vampire Savior's engine, and
  both have a thirteenth stage (Donovan's). They build Vampire Savior's sources, so each has everything its port
  has: the training mode and the dummy, widescreen and invisible walls, survival, the endless battle, ring out, the
  stage and model viewers, attract mode, the hidden characters (each game's own: Oboro Bishamon as the board gives
  him, Shadow, Dark Gallon in Vampire Hunter 2), custom colours and the colour editor (PalMod's file in Vampire Savior
  2 only), a custom soundtrack with each game's music named, the board's settings, save states and run-ahead. Every
  rewrite is checked against each game's own code (`make GAME=<id> check-rewrites`), and widescreen as Vampire
  Savior's is (`check-widescreen`).
- **A game made from another's program builds that game's sources** ([docs/TOOLKIT.md](docs/TOOLKIT.md)): a
  `game.cfg`'s `sources`, places named `At()` and `AtA5()` and listed in the game's `family.h`, `tools/portmap.py`
  (where one program's code and variables are in another's) and `tools/places.py` (that list); a hook or rewrite at
  place 0 (`kNoPlace`) is not made. Vampire Savior is as it was, frame for frame.

- **A custom soundtrack's folder can be any folder of music**: with no file in it named for a song, its files play
  as a playlist in place of all the game's music, one after another in their names' order ("2 ..." before
  "10 ..."), from the first again after the last; the feature's new choice "On, shuffled" plays them in a random
  order, a new one each time round. Files named for songs work as before.

- **A custom soundtrack**, in every game: your own music files (Ogg Vorbis, FLAC, MP3 or WAV; an arranged soundtrack,
  say) in place of the board's music, song by song, with the board's sound effects and voices. The "Custom
  soundtrack" feature (Sound; on in Enhanced) and a folder on each game's page (recomp-common 6c526f4), or
  `--soundtrack DIR`. Files are named after the songs, which each game names in the `songs.txt` it writes there
  (Street Fighter Alpha 2's 59, Vampire Savior's 80, Super Puzzle Fighter II Turbo's 24); `soundtrack.ini` names
  other files, their loops and loudness. The music the game asks for is caught where it leaves its requests for the
  sound CPU (`machine/soundtrack.h`), which is then told to stop its music, and played in the game's time: save
  states (now version 6; version 5's still load), run-ahead and spare machines keep it where the game is. With the
  feature off, or no files, nothing changes. Decoders: stb_vorbis, dr_flac, dr_mp3, dr_wav (`third_party/`,
  public domain).
- `CPS2RECOMP_SOUNDTRACK_LOG` lists each piece of music the game asks for, with the frame; `--music-test FRAMES`
  plays each on the board in turn and says how long it lasts; `--soundtrack-dump FILE` writes what the soundtrack
  plays (headless).

- The games can run at once, each in its own window (one of each). A shared feature one game's menu changes while
  another runs is set in that one when it ends. (recomp-common 5e82073)

- `make collection-app` makes the collection's app bundle for your own machine (`build/collection/CPS-2 Recomp.app`:
  the launcher and every built game, with Homebrew's SDL 3 inside), as `make GAME=<id> app` does for one game.

## 0.4.0-beta.1

CPS-2 Recomp: the toolkit and its games in one repository, and one launcher for all of them.

- **The games are here**: Street Fighter Alpha 2 (`games/sfa2`), Super Puzzle Fighter II Turbo
  (`games/puzzle_fighter`) and Vampire Savior (`games/vampire_savior`), each with its README, docs, movies and
  checks. Each was its own repository with a copy of the toolkit; their histories until now are in their own
  changelogs (`games/<id>/CHANGELOG.md`). A toolkit change now reaches every game at once, and is checked with
  every game. `tools/sync-toolkit.sh` and the toolkit copies are gone.
- **One launcher for every game** (recomp-common's collection launcher; `make collection`): a Games page that
  finds each game's ROM set in a folder you choose (or one dropped on the window), a Features page with what the
  games share, set for all of them at once (a feature changed in one game's own menu is set for the others when
  it ends), the Controllers page for all of them, and a page for each game whose ROM set you have, with its
  Display switches, Play, and the features only it has. Each game keeps its own settings folder, as its own
  launcher did, and the ROM set chosen there is taken over.
- From a terminal: `cps2recomp-launcher --play ID` (straight into a game), `--play ID -- ARGS` (more arguments
  for the game, after the launcher's own), `--page ID`. A game's program still runs by itself, with all its
  arguments; `--features` may now be given more than once (later ones win). A game's program opened by itself
  starts the collection's launcher at its page when there is no launcher of its own beside it.
- The release is the collection: `CPS2Recomp-macOS.zip`, `CPS2Recomp-linux-x86_64.tar.gz`,
  `CPS2Recomp-windows-x64.zip`, each with the launcher and every game (`tools/release/`). CI builds and starts all
  of it, with no translated code, on all three systems.
- Build from the top: `make GAME=<id>` and its targets as before, `make games`, `make collection`,
  `make collection-app`; ROM sets in `rom/<id>/`.
- Settings: `kFeatureOwn` (`OwnFeatures`), a game's feature that the collection never sets for another game (the
  survival and ring out records, the animations' numbers).

## 0.3.0 (2026-10-15)

- recomp-common 0.5.0: in the in-game menu, Right on a page's row goes into that page; the
  launcher's Features page has a search box (Ctrl+F or Cmd+F; Esc empties it, a second Esc leaves it;
  a gamepad's A goes in and B leaves with what was typed kept).
- The in-game menu's first page has a "Test menu" row: it presses the board's test switch (as F2
  does), which opens the game's own operator menu, where its settings are (difficulty, rounds and the rest).
- Colour editor: random colours (the Random row: LK, left or right) only at a second press within a second and
  a half, and HK puts back the set from before them until the next change. The Line row goes to the
  animation that best shows the line when the game says which (`Description::animationForLine`).
  "Load colour files again" and "Write the game's colours" are on the editor's page, not Display.
- The game's program started with no arguments (a double click on it, not on the launcher) starts
  the launcher beside it instead: straight into the game (`--play`) once a ROM set has been chosen.
  A start that fails says why in a message box too, unless headless. On Windows the game is a
  windowed program (no console window beside it).
- recomp-common 394d613: the menu leaves room for a page's longest description (it ran over the hint),
  a value too long for its row is drawn small, and the panel is opaque.
- Display options for every game (the port's features, on the Display page of the launcher and the
  in-game menu): Scaling (Sharp, as before; Whole pixels; Smooth), Picture shape (4:3, as before;
  Square pixels; Fill) and Window size (3x, as before; 2x to 6x). A window takes the new shape or
  size at once. `FitPicture` and `Presenter::SetScaling` (present.h); `--scale` still goes before the
  setting, and `--present-size WxH` draws `--present-test`'s picture in a window that size.
- `SetSlowMotion`: a game's sources can ask for the game to run slowly (a frame in every so many
  pictures); the slower of that and the menu's speed is used.
- The check of a rewrite compares video RAM too for a rewrite that names `kChecksVideo` (a routine
  that writes a scroll screen's map or the row scroll table).
- The code for three lines of 0.2.0's list, which was released without it: `kFeatureFirstPage`,
  `Video::wide.rows` and `Video::objectsTaken`; and the board's own picture drawn by a form of the
  renderer that knows its width.
- `fight/`: what a fighting game's training mode has in common (frame data, its meter and a log of
  every move, the input display in numpad notation, the combo readout, a dummy that guards, records
  and replays, boxes on the picture); `docs/FIGHT.md`. `runtime/spare.*`: machines kept as save
  states and run beside the port's, for a game's modes. `SettingsPath()`.
- The dummy at random: what it does while guarding, the escape from a throw and the way up from a
  knock-down; "Answers now and then" (one time in two, with any slot that has a recording).
- Run-ahead for every game (the port's `run_ahead` feature: off, 1 or 2 frames): a press is seen a
  frame or two sooner and the game runs exactly as without it. Not while stopped, stepped or slowed,
  nor when the game says not (`Game::noRunAhead`); `RunningAhead()` for a game's sources. Listed in
  the launcher (`RegisterPortFeatures`). The 68000's wait is kept in a save state (state version 5).
- `Video::wide.fillHoles`, a game's choice: pixels of the added columns that nothing was drawn at
  take the line's own picture, mirrored at the nearest drawn pixel.
- A first-page switch changed in the in-game menu is changed in `launcher.ini` too; the window takes
  the widest picture's shape again when it changes.
- The launcher's Game page says how to open the in-game menu while playing.
- `tools/at-once.sh`: a game's checks run a few at a time, their outputs in order.
- Training aids work only in a training session: `kFeatureTraining` (a game's sources mark theirs with
  `TrainingAids`; the dummy's are marked by `RegisterDummy`) and `Game::inTraining`. Outside a session
  `FeatureValue` gives 0 for them, while the menu shows the player's setting (`FeatureSetting`) and
  `settings.ini` keeps it.
- Custom colours and a colour editor for a fighting game's characters (`fight/colours.h`): the
  `custom_colours` feature (text files a character and a PalMod-changed copy of the ROM file in
  `palettes/` beside settings.ini, written over the ROM image's colours), "Write the game's colours"
  and "Load colour files again", and the editor (Alternative modes → Colour editor) in the game's
  model viewer, with a library of saved sets. A game describes where its colours are and supplies its
  viewer's hooks (`colours::Description`). From Vampire Savior's, with the same files and folders.

## 0.2.0 (2026-10-13)

- A wider picture for a game's sources to ask for (`Video::wide`): up to 64 more columns on each side
  of the board's 384 (512 is the whole raster: 16:9), each scroll screen and each object drawn moved
  sideways as the sources say, and a scroll screen's tiles outside the board's own columns taken
  from the sources where the game only keeps its map up for its own picture. The window, the menu,
  the readout, screenshots and `--present-test` take the picture at the width it has; the window
  starts in the shape `Game::widestPicture` gives. With nothing asked for, the picture and the
  state hashes are what they were.
- A feature's switch can be on the launcher's first page, under "Full screen", instead of on its
  Features page: the feature has `kFeatureFirstPage`, and the game's `game.cfg` names it in
  `display` (name | title | what it says). The launcher says at every start whether it is on.
- `Video::wide.rows`: the scroll screen of 16-pixel tiles moved by its own amount on each row of the
  row scroll table.
- `Video::objectsTaken`: the sources may change the object list the picture is made from, when
  the board has taken it.
- `Machine::ObjectsToCome`: the object list the next picture will be made from.
- `CPS2RECOMP_WIDE=n`: n more columns on each side whatever the sources ask, to see what a game has
  beyond its picture's edges.
- recomp-common 0.4.0: the launcher works with a gamepad, or the arrow keys, alone, and has a
  "Muted" switch (the game is started with `--mute`).
- The in-game menu's buttons are the same whatever the game's are set to: the arrows, the d-pad and
  the left stick; Space, Enter and a gamepad's A choose; Backspace and B go back (leave a page,
  close the menu from the first). A "Sound" row switches the sound off and on; `--mute` starts with
  it off (the sound device is opened all the same, so that the row can switch it on).

## 0.1.0

The first public version: the toolkit as its first game, Vampire Savior Recomp, uses it.

- The translators of 68000 and Z80 code to C++ (`tools/recomp.py`, `tools/recompz80.py`), and
  finding a game's code by made-up play (`make explore`).
- The machine: the 68000 with an interpreter for code that has no translation, the bus, interrupts
  and timing, the settings memory, the ROM set and its cipher; the picture; the Z80 and the QSound
  chip; Capcom's sound driver (version 2.01b) as ordinary code, used in place of the Z80 where a
  game has that version.
- The picture on SDL's GPU API (Metal, Vulkan, Direct3D 12), with SDL's renderer where there is no
  device.
- Routines of a game replaced by C++, readable rewrites and their checker; features and settings;
  save states; the in-game menu and the launcher the recomp ports share (recomp-common).
- A game's repository takes a copy with `tools/sync-toolkit.sh`, checked by `make check-toolkit`;
  `make CR=../cps2recomp` builds a game against a clone.
- `make check` here, with no game: the audit, the stand-in game in `example/`, the sync script.
