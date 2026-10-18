#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# All release builds on a Mac with Docker: macOS here, Linux and Windows in the x86_64 build image
# (tools/release/Dockerfile). Writes build/release/ with the three downloads, SHA256SUMS.txt, and the launcher's
# updates (each system's update-<system>.txt and update-files/).
# Needs the translated code of a build of each game with its ROM set (make GAME=<id>, make GAME=<id> explore).
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
tools/audit.sh
docker build --platform linux/amd64 -q -t cps2recomp-build tools/release >/dev/null
rm -rf build/release/update-files build/release/update-*.txt  # (the launcher's updates: written again by each script)
tools/release/macos.sh
docker run --rm --platform linux/amd64 -e GAMES="${GAMES:-}" -v "$ROOT":/src -w /src cps2recomp-build tools/release/linux.sh
docker run --rm --platform linux/amd64 -e GAMES="${GAMES:-}" -v "$ROOT":/src -w /src cps2recomp-build tools/release/windows.sh
cd build/release
shasum -a 256 CPS2Recomp-macOS.zip CPS2Recomp-linux-x86_64.tar.gz CPS2Recomp-windows-x64.zip > SHA256SUMS.txt
cat SHA256SUMS.txt
# (every file the manifests name is there)
missing=$(sed -n 's/^file \([0-9a-f]*\) .*/\1/p' update-*.txt | sort -u | while read -r sum; do [ -f "update-files/$sum.gz" ] || echo "$sum"; done)
if [ -n "$missing" ]; then
    echo "error: files the update manifests name are not in update-files: $missing" >&2
    exit 1
fi
echo "update manifests: $(ls update-*.txt | tr '\n' ' '); $(ls update-files | wc -l | tr -d ' ') files"
echo "version $(cat "$ROOT/VERSION"): release files in $ROOT/build/release"
