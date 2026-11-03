# Street Fighter II: Hyper Fighting Recomp

> Part of **[CPS-2 Recomp](../../README.md)**: one download and one launcher for every Capcom arcade game this
> project has ported. This page is about Street Fighter II: Hyper Fighting; the [main page](../../README.md) says how
> to download and play.

An unofficial native PC port of the arcade game *Street Fighter II′: Hyper Fighting* (Capcom, 1992, CPS-1 board;
*Street Fighter II′ Turbo* in Japan) for macOS, Linux (Steam Deck included) and Windows, made by static
recompilation.

> **Initial classic CPS-1 port, with sound.** The game's picture, controls and arcade music and effects run on the
> toolkit's shared classic CPS-1 board. Board options and cheats are declared in `game.cfg`; there is no training
> mode yet. The YM2151 uses [ymfm](https://github.com/aaronsgiles/ymfm), the BSD-3-Clause library listed in
> [THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md). The OKI MSM6295 is implemented in `sound/okim6295.cpp`.

**No game files are included.** You need your own copy of the game: the arcade ROM set `sf2hf` (World, 921209), from
a board you own. This project cannot help you get one.

This project is not affiliated with, endorsed by, or sponsored by Capcom. *Street Fighter* is a trademark of Capcom
Co., Ltd.

## What you need

- The ROM set `sf2hf` (*Street Fighter II′: Hyper Fighting*, World 921209): `sf2hf.zip` or a folder with its files
  (a merged zip works too: files are found by checksum). Other regions and revisions are refused.
- One of: macOS 13 or later; Linux x86_64 or SteamOS; Windows 10 or 11, 64-bit.

## Building from source

With your own ROM set in `rom/sf2hf/` (ignored by Git), from the repository's top:

```sh
make GAME=sf2hf            # twice the first time
make GAME=sf2hf explore    # once: finds the game's code by made-up play and translates it
make GAME=sf2hf run
```

A build of `sf2ce` first lets `tools/portmap.py` move its entry points to this program before exploring
(`--from work/sf2ce --to work/sf2hf --limit 180000 --entries ...`, added to `work/sf2hf/entries.txt`).

## Progress

**Where it stands.** Uses Champion Edition's classic board, S9263B graphics mapper, CPS-B-21 multiplier,
extra kick inputs and sound hardware. Game-specific ROM lists, mapper ranges, DIPs and cheats stay in `game.cfg`.

**The DIP switches** are as the boards left the factory (`dip` in game.cfg, `FF FC 9F`): difficulty 3 (normal), continues allowed, the demonstrations' sound. Before, the port read them all off: the easiest difficulty and no continues.

**The Board page** (the in-game menu's, F1 or Esc, and the launcher's): the DIP switches by name, from game.cfg's `board_option_N` lines: difficulty (0 to 7), free play, continue, demo sound and two coins to start, each "As the board is set" unless chosen. The board reads them all the time, so a change applies at once, as the game next uses it.

**The Cheats page** (the menu's and the launcher's), from game.cfg's `cheat_N` lines: writes to the work RAM before
every frame while a cheat is on, while a round is fought. Infinite energy for each player, infinite time and one-hit
knockouts (player 2). The bosses can be chosen in this game, so there is no cheat for them. Each can be switched any
time; off, the game goes on as it was.

## Reproducible checks

`movies/smoke_play.txt` supplies 40,000 frames of input from reset. It exercises coin/start and gameplay;
it is a smoke route, not a completed game. Ghouls finishes its board tests later than Street Fighter II;
its first coin/start during those tests is ignored and the next pair starts play.

```sh
make GAME=sf2hf check FRAMES=40000 MOVIE=games/sf2hf/movies/smoke_play.txt
make GAME=sf2hf check-rollback FRAMES=12000
make GAME=sf2hf check-online FRAMES=6000
```

The online check uses two local peers and simulated packet delay/loss; it does not test Internet play.
The classic sound program runs on the Z80. `check-sound`'s QSound high-level-driver comparison is not applicable.
Windows is the tested platform for this contribution; macOS/Linux builds still need verification.
