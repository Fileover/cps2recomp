# The toolkit

cps2recomp is the toolkit the games in this repository are made with: it makes native PC ports of Capcom CPS-2
arcade games by static recompilation. A game's 68000 program is translated to C++ ahead of time and compiled
into the port. There it runs on a small machine written for this toolkit: the video, the QSound chip, the sound
CPU (with its driver translated too, or rewritten as ordinary code), the board's registers and its settings
memory. No emulator is part of a port, and no game data is in this repository.

This page is for people making or working on ports. To play, see the [main page](../README.md).

## What a port gets from it

- **Translation ahead of time.** `tools/recomp.py` turns the game's 68000 program into C++, and
  `tools/recompz80.py` does the same for the sound CPU's. The code is found by reading the program
  and by made-up play (`tools/explore.sh`). Code it has not found yet runs in an interpreter, and is
  counted, so that the count can be driven to zero.
- **A machine of its own, not an emulator.** The 68000, the bus, interrupts and timing, the settings
  EEPROM, the ROM set and its cipher (`machine/`), the picture (`video/`), and the Z80 and QSound
  (`sound/`). Capcom's sound driver, version 2.01b, is also there as ordinary C++ (`sound/driver.cpp`),
  used in place of the Z80 for games that have that version.
- **A native picture.** SDL's GPU API (Metal, Vulkan or Direct3D 12), with SDL's renderer where
  there is no GPU device, and a wider picture (up to 16:9) for games whose sources ask for it. The
  Display page lets players choose how it fits the window: sharp, whole pixels or smooth; 4:3, square
  pixels or the whole screen; and the window's size.
- **Readable rewrites, with proof.** A game's routines can be rewritten by hand as ordinary C++. A
  checker runs the original and the rewrite on every call and compares memory, registers, timing and,
  when asked, video RAM ([docs/READABLE.md](READABLE.md)).
- **A training mode's parts.** `fight/` has what fighting games share: frame data and a frame meter,
  a log of every move, the input display in numpad notation, the combo readout, a dummy that stands,
  crouches, jumps, guards, records and replays, and hit boxes drawn through the game's camera. A
  game's sources only say, from its own memory, what the players are doing
  ([docs/FIGHT.md](FIGHT.md)).
- **Players' own colours.** Text files a character, or a ROM file changed with PalMod, recolour the
  characters, and a colour editor in the model viewer makes them; a game only says where its colours
  are (`fight/colours.h`).
- **Features players expect.** Save states, run-ahead (a press seen a frame or two sooner, with the
  game running exactly as without it), speed, pause and frame advance, slow motion, cheats and
  options a game declares once, and the in-game menu and launcher the recomp ports share
  ([recomp-common](https://github.com/tmcleroy/recomp-common); [docs/FEATURES.md](FEATURES.md)).
  Every change to how a game plays is opt-in: with all features off, the game runs as the board.
- **Checks at every step.** Translated code is checked against the interpreter, the sound driver's
  every chip write against the Z80's, and runs of the same input movie must give the same per-frame
  hashes ([docs/VERIFYING.md](VERIFYING.md)).
- **macOS, Linux (Steam Deck included) and Windows** through SDL 3. CI builds and starts the toolkit
  on all three.

## Trying it with no game

`make check` needs no game. It audits the files (`tools/audit.sh`: nothing from a game, no keys, no
personal paths), checks the copy of recomp-common, builds the program with no translated code and
the launcher for the stand-in game in `example/`, and starts both.

```sh
git clone https://github.com/tmcleroy/cps2recomp
cd cps2recomp
make check
```

You need a C++20 compiler (Clang, GCC or MinGW-w64), CMake, Ninja, Python 3, zlib and SDL 3. On
macOS that is `brew install cmake ninja sdl3`. [CONTRIBUTING.md](../CONTRIBUTING.md) has the rest, and
`.github/workflows/ci.yml` shows exactly what each system installs.

## How a game uses it

A game is a folder in `games/`:

```
games/<id>/     game.cfg (the ROM set's files and checksums, and what the build, the launcher and the checks
                need to know), src/ (names, features, rewrites), movies/, check_rewrites.sh, PROGRESS.md,
                README.md and docs/ (its page for players, what it has, how it works, what is next)
```

Everything is built from the top with `GAME=<id>` (`make GAME=<id>`, `make GAME=<id> explore`,
`make GAME=<id> check`, `check-blocks`, `check-rewrites`, `check-release`, `launcher`, `app`: `build.mk` has
them), and `make games` and `make collection` build every game and the collection's launcher
([BUILDING.md](BUILDING.md)). `example/` is a game with no game in it, which CI builds; any game in `games/` is a
complete one. Start a new game from either: a `game.cfg`, an empty `src/`, `make GAME=<id>` twice and
`make GAME=<id> explore`, then the game's own names and features. The collection's launcher picks up every game
in `games/` (`tools/gamecfg.py --collection` writes its table from their `game.cfg` files).

The player's ROM set is only read. The program is deciphered with the set's own key file, as the
board does when it starts; no key is in this repository.

## Working on the toolkit

A toolkit change is checked with every game: build each (`make GAME=<id>`) and run its checks
(`make GAME=<id> check`, `check-rewrites`; [VERIFYING.md](VERIFYING.md)). Anything about one game belongs in
that game's `game.cfg` or `src/`, not in the toolkit. A feature more than one game has should have the same
name and choices in each: the collection's launcher then shows it once, for all of them.

## Repository layout

| Path | What |
| --- | --- |
| `tools/` | The translators (`recomp.py`, `m68k.py`; `recompz80.py`, `z80.py`), `gamecfg.py`, the checks (`check.sh`, `check_sound.sh`, `check_release.sh`, `at-once.sh`), finding a game's code by made-up play (`explore.sh`), reading the listing (`lst.py`, `tracechanges.py`, `ramfind.py`), `gen_shaders.py`, `sync-common.sh`, `audit.sh`, `collection.sh`, and the release builds (`release/`) |
| `runtime/` | What translated code is written in terms of; hooks and readable rewrites (`lift.h`); features and settings (`game.h`); save states (`state.h`); spare machines a game's modes run beside the port's (`spare.h`) |
| `machine/` | The 68000, bus, interrupts, timing, settings EEPROM, ROM set and cipher |
| `video/`, `sound/` | The picture; the Z80 and QSound; Capcom's sound driver (version 2.01b) as ordinary code (`driver.cpp`) |
| `port/` | The frontend (SDL 3), putting the picture on the screen (`present.*`; shaders in `shaders/`, compiled into `shaders.inc` by `tools/gen_shaders.py`), the in-game menu adapter, the HUD, control bindings |
| `fight/` | What a fighting game's training mode has in common, and the players' own colours ([docs/FIGHT.md](FIGHT.md)) |
| `recomp-common/` | A copy of [recomp-common](https://github.com/tmcleroy/recomp-common) (the in-game menu, the settings rules, the launcher), made by `tools/sync-common.sh`; never changed here |
| `launcher/` | What this toolkit tells the shared launcher (`recomp-common/launcher`; SDL 3 + Dear ImGui): `src/description.cpp` has the words for a ROM set, the check of the player's, the game's arguments and the buttons that can be chosen, for one game's launcher or the collection's |
| `games/` | The games ([the main page](../README.md) has the list) |
| `example/` | A stand-in game with no game: what `make check` and CI build |
| `docs/` | `BUILDING.md`, `RELEASING.md`, `VERIFYING.md`, `READABLE.md`, `FEATURES.md`, `FIGHT.md`, this page; `screenshots/` (the READMEs' pictures) |
