# Street Fighter II: Champion Edition Recomp

> Part of **[CPS-2 Recomp](../../README.md)**: one download and one launcher for every Capcom arcade game this
> project has ported. This page is about Street Fighter II: Champion Edition; the [main page](../../README.md) says how
> to download and play.

An unofficial native PC port of the arcade game *Street Fighter II′: Champion Edition* (Capcom, 1992, CPS-1 board)
for macOS, Linux (Steam Deck included) and Windows, made by static recompilation.

> **Initial classic CPS-1 port, with sound.** The game's picture, controls and arcade music and effects run on the
> toolkit's shared classic CPS-1 board. Board options and cheats are declared in `game.cfg`; there is no training
> mode yet. The YM2151 uses [ymfm](https://github.com/aaronsgiles/ymfm), the BSD-3-Clause library listed in
> [THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md). The OKI MSM6295 is implemented in `sound/okim6295.cpp`.

**No game files are included.** You need your own copy of the game: the arcade ROM set `sf2ce` (World, 920513), from
a board you own. This project cannot help you get one.

This project is not affiliated with, endorsed by, or sponsored by Capcom. *Street Fighter* is a trademark of Capcom
Co., Ltd.

## What you need

- The ROM set `sf2ce` (*Street Fighter II′: Champion Edition*, World 920513): `sf2ce.zip` or a folder with its files.
  A merged zip with its clones in it works too: the launcher finds each file by checksum. Other regions and revisions
  are refused.
- One of: macOS 13 or later; Linux x86_64 or SteamOS; Windows 10 or 11, 64-bit.

## Building from source

With your own ROM set in `rom/sf2ce/` (ignored by Git), CMake, Ninja, SDL 3 and Python 3, from the repository's top:

```sh
make GAME=sf2ce            # twice the first time
make GAME=sf2ce explore    # once: finds the game's code by made-up play and translates it
make GAME=sf2ce run
```

## Progress

**Where it stands.** The board is `cps1-classic` (machine/romset.h): a CPS-1 with no QSound. Three things the classic
board needed and Cadillacs and Dinosaurs had not:

- **The graphics mapper.** The PAL S9263B keeps the three scroll screens' tiles in its third bank (`gfx_map` in
  game.cfg, `GfxRange`); without it the backgrounds and the text were other tiles.
- **The CPS-B-21's multiplier** (800140 and 800142 multiplied, the product at 800144 and 800146): the game looks up
  the jump at the start of a fight with it, and jumped to nowhere and restarted without it.
- **The kicks** at 800176: player 1's in bits 0 to 2, player 2's in bits 4 to 6, light to hard. Each kick checked
  from the start of a round: a light, a medium and Ryu's roundhouse.

**The sound** ( `machine/cps1_sound.cpp`). The board's Z80 runs its program at 3.579545 MHz on its own map
(ROM, a bank of it at 8000, RAM at D000, the YM2151 at F000, the OKI at F002, the bank at F004, the OKI's pin 7 at
F006), and reads the 68000's requests from the two latches at 800181 and 800189 (F008 and F00A). Its interrupt is the
YM2151's line (a level, `Z80::SetInterruptLine`), about 250 times a second from the chip's timers, which set the
music's tempo. The YM2151 is ymfm's; the OKI MSM6295 (`sound/okim6295.cpp`) was written here from its data sheet:
four voices of 4-bit ADPCM from the sample ROMs' table. The two are mixed to one speaker, as on the board, at the
port's output rate, and kept in save states (version 9; older ones load, their sound starting afresh). DIP switch C6
(the demonstrations' sound) is on, as the boards left the factory, so the attract mode plays its music.

**The DIP switches** are as the boards left the factory (`dip` in game.cfg, `FF FC 9F`): difficulty 3 (normal), continues allowed, the demonstrations' sound. Before, the port read them all off: the easiest difficulty and no continues.

**The Board page** (the in-game menu's, F1 or Esc, and the launcher's): the DIP switches by name, from game.cfg's `board_option_N` lines: difficulty (0 to 7), free play, continue, demo sound and two coins to start, each "As the board is set" unless chosen. The board reads them all the time, so a change applies at once, as the game next uses it.

**The Cheats page** (the menu's and the launcher's), from game.cfg's `cheat_N` lines: writes to the work RAM before
every frame while a cheat is on, while a round is fought. Infinite energy for each player, infinite time and one-hit
knockouts (player 2). The bosses can be chosen in this game, so there is no cheat for them. Each can be switched any
time; off, the game goes on as it was.

**Next.** The DIP switches by name (free play, difficulty), and the features the other fighting games have.

## Reproducible checks

`movies/smoke_play.txt` supplies 40,000 frames of input from reset. It exercises coin/start and gameplay;
it is a smoke route, not a completed game. Ghouls finishes its board tests later than Street Fighter II;
its first coin/start during those tests is ignored and the next pair starts play.

```sh
make GAME=sf2ce check FRAMES=40000 MOVIE=games/sf2ce/movies/smoke_play.txt
make GAME=sf2ce check-rollback FRAMES=12000
make GAME=sf2ce check-online FRAMES=6000
```

The online check uses two local peers and simulated packet delay/loss; it does not test Internet play.
The classic sound program runs on the Z80. `check-sound`'s QSound high-level-driver comparison is not applicable.
Windows is the tested platform for this contribution; macOS/Linux builds still need verification.
