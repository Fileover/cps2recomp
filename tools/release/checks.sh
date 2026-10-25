#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# A release's checks (docs/RELEASING.md, step 1): for every game (games/*, or GAMES="a b"), with its ROM set in
# rom/<id>/: its build, check, check-blocks, check-release over its ladder movie if it has one (40,000 frames;
# Puzzle Fighter's 66,000), check-rewrites and check-widescreen if it has them; then the repository's make check.
# One at a time (two builds of one game must never run together). Each step's output is in build/release-checks/;
# a line for each step, and at the end the ones that failed. About 45 minutes.
#   tools/release/checks.sh
#
# Every game's code must have been found first (work/<id>/entries.txt: make GAME=<id> explore, or
# tools/seed_entries.sh from a tree where it was): a tree without it translates from guesses, and check and
# check-blocks fail for that alone. The checks stop at once if a game has none.
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
GAMES="${GAMES:-$(cd games && for cfg in */game.cfg; do printf '%s ' "${cfg%/game.cfg}"; done)}"
LOGS=build/release-checks
mkdir -p "$LOGS"
failed=""
unexplored=""
for game in $GAMES; do
    [ -d "rom/$game" ] && [ ! -s "work/$game/entries.txt" ] && unexplored="$unexplored $game"
done
if [ -n "$unexplored" ]; then
    echo "error: no code found yet for:$unexplored (work/<id>/entries.txt). For each:" >&2
    echo "  make GAME=<id> explore, or tools/seed_entries.sh <a tree where it was explored>" >&2
    exit 1
fi
step() { # game, step's name, make's arguments
    game=$1 name=$2
    shift 2
    if make GAME="$game" "$@" > "$LOGS/$game.$name.log" 2>&1; then
        echo "$game $name: passed"
    else
        echo "$game $name: FAILED ($LOGS/$game.$name.log)"
        failed="$failed $game:$name"
    fi
}
for game in $GAMES; do
    if [ ! -d "rom/$game" ]; then
        echo "$game: no ROM set in rom/$game"
        failed="$failed $game:rom"
        continue
    fi
    step "$game" build
    step "$game" check check
    step "$game" blocks check-blocks
    if [ -f "games/$game/movies/ladder.txt" ]; then
        frames=40000
        [ "$game" = puzzle_fighter ] && frames=66000
        step "$game" release check-release FRAMES=$frames
    fi
    [ -f "games/$game/check_rewrites.sh" ] && step "$game" rewrites check-rewrites
    [ -f "games/$game/check_widescreen.sh" ] && step "$game" widescreen check-widescreen
done
if make check > "$LOGS/repository.log" 2>&1; then echo "make check: passed"; else echo "make check: FAILED"; failed="$failed check"; fi
if [ -n "$failed" ]; then
    echo "failed:$failed"
    exit 1
fi
echo "every check passed"
