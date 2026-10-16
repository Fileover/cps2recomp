#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# All release builds on a Mac with Docker: macOS here, Linux and Windows in the x86_64 build image
# (tools/release/Dockerfile). Writes build/release/ with the three downloads and SHA256SUMS.txt.
# Needs the translated code of a build of each game with its ROM set (make GAME=<id>, make GAME=<id> explore).
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
tools/audit.sh
docker build --platform linux/amd64 -q -t cps2recomp-build tools/release >/dev/null
tools/release/macos.sh
docker run --rm --platform linux/amd64 -e GAMES="${GAMES:-}" -v "$ROOT":/src -w /src cps2recomp-build tools/release/linux.sh
docker run --rm --platform linux/amd64 -e GAMES="${GAMES:-}" -v "$ROOT":/src -w /src cps2recomp-build tools/release/windows.sh
cd build/release
shasum -a 256 CPS2Recomp-macOS.zip CPS2Recomp-linux-x86_64.tar.gz CPS2Recomp-windows-x64.zip > SHA256SUMS.txt
cat SHA256SUMS.txt
echo "version $(cat "$ROOT/VERSION"): release files in $ROOT/build/release"
