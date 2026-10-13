# Contributing

Thanks for helping! cps2recomp is the toolkit the CPS-2 ports are built with; a change here reaches
every game that takes a copy of it.

## The rules that keep this project legal

These aren't negotiable; pull requests that break them are closed.

1. **Never add anything from a game.** No ROM or part of one, no extracted graphics, sounds or text,
   no settings memory, save states, RAM dumps, video or screenshots, and nothing generated from a
   game: generated C++ (`gen/`), entry lists (`entries.txt`), disassembly listings, input movies.
   The only "game" here is the stand-in in `example/`, which has no ROM set. The same goes for
   issues: attach logs, not game files.
2. **Never add a key**, a key file, key values, or a tool that writes key files; and never point to
   where ROM sets or keys can be found.
3. **Nothing from leaked material**: no leaked source, and no names taken from it.
4. **No emulator's code**, beyond what `THIRD_PARTY_NOTICES.md` lists (MAME's implementation of the
   program cipher). Reading an emulator to learn how the hardware behaves is fine; copying its code
   in is not.
5. **Only add code you have the right to contribute**, under the GPL-3.0. Code from other projects
   must have a compatible license and be credited in `THIRD_PARTY_NOTICES.md`.

## Where things go

Nothing here may be about one game: no addresses, names or special cases of a game's. What a game
needs that others do not goes in that game's `game.cfg` or its own sources (`games/<id>/src` in its
repository), through the hooks in `runtime/lift.h` and `runtime/game.h`.

`recomp-common/` is a copy of another repository and is never changed here: changes go to
[recomp-common](https://github.com/tmcleroy/recomp-common), then `tools/sync-common.sh`.

## Building and testing

`make check` here needs no game: it audits the files (`tools/audit.sh`), checks the copy of
recomp-common, builds the program with no translated code and the launcher for the stand-in game in
`example/`, starts both, and tries `tools/sync-toolkit.sh`. CI runs the same on macOS, Linux and
Windows.

A change to the translators, the machine, the picture or the sound needs a game as well, which only
someone with its ROM set can run. From the game's repository, against your clone of this one
(`make CR=../cps2recomp`, see [README.md](README.md)):

- `make CR=../cps2recomp check`: the interpreter and the translated code must give the same state
  after every frame, and two runs must be alike;
- `make CR=../cps2recomp check-rewrites`: every hand-written routine of the game against the code
  it replaces; `check-sound`: the sound driver against the game's sound program;
- with every feature off the game must run exactly as before: the same movie must give the same
  hashes (`--movie`, `--hash-log`).

Say in the pull request which game it was checked with. Nothing may add input lag or slow a game
down: measure when in doubt. Both the keyboard and gamepads must keep working.

## Code style

- Match the surrounding code: naming, comment density and idiom. Comments explain why.
- C++20; 4-space indents, braces on the same line, lines up to about 120 columns.
- Every source file starts with the license and copyright lines the others have; the audit fails
  on one without.
- Commit messages: a short summary line, then what changed and why.

## Code of conduct

Be kind and constructive. See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
