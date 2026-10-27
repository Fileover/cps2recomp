#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# The Display page's shaders (port/shading.h), made once into build/deps/librashader/ and kept there:
#   librashader.dylib   librashader's C API with its OpenGL runtime only, built from its source (Rust: rustup's
#                       stable cargo, with the aarch64-apple-darwin and x86_64-apple-darwin targets), Apple silicon
#                       and Intel in one
#   shaders/            the presets of RetroArch's slang-shaders the Display page names, with the files they use
#                       (tools/shaders.py)
# Each source is checked against its checksum. macOS only for now. Prints the folder.
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEPS="$ROOT/build/deps"
OUT="$DEPS/librashader"
LIBRASHADER_VERSION=0.12.1
LIBRASHADER_SHA256=dfa1098ac5bda745bf0e0fc4797b0af441baaaae4b3c7f2982503d468cec9548
SLANG_SHADERS_COMMIT=2b0530bac0e8083d9d643c194929dc0a6b69bd35
SLANG_SHADERS_SHA256=97427ee7a2b61257f2419314684ae5694ae68b645bae34d820c05254bb14551c
sha256() { if command -v sha256sum >/dev/null; then sha256sum "$@"; else shasum -a 256 "$@"; fi; }
# (a source archive, checked, unpacked into $DEPS: $1 the file, $2 its URL, $3 its checksum)
fetch() {
    if [ ! -f "$DEPS/$1" ]; then
        curl -fsSL -o "$DEPS/$1.part" "$2"
        mv "$DEPS/$1.part" "$DEPS/$1"
    fi
    echo "$3  $DEPS/$1" | sha256 -c - >/dev/null
}
mkdir -p "$OUT" "$DEPS"
PATH="/opt/homebrew/opt/rustup/bin:$HOME/.cargo/bin:$PATH"

SOURCE="$DEPS/librashader-librashader-v$LIBRASHADER_VERSION"
if [ ! -f "$OUT/librashader.dylib" ] || [ ! -f "$OUT/version-$LIBRASHADER_VERSION" ]; then
    fetch "librashader-$LIBRASHADER_VERSION.tar.gz" \
        "https://github.com/SnowflakePowered/librashader/archive/refs/tags/librashader-v$LIBRASHADER_VERSION.tar.gz" "$LIBRASHADER_SHA256"
    [ -d "$SOURCE" ] || tar -xzf "$DEPS/librashader-$LIBRASHADER_VERSION.tar.gz" -C "$DEPS"
    if ! command -v cargo >/dev/null; then
        echo "error: librashader is built with Rust: brew install rustup && rustup default stable" >&2
        exit 1
    fi
    for target in aarch64-apple-darwin x86_64-apple-darwin; do
        rustup target add "$target" >/dev/null 2>&1 || true
        (cd "$SOURCE" && MACOSX_DEPLOYMENT_TARGET=13.0 cargo build --quiet --release --locked -p librashader-capi --lib \
            --no-default-features --features runtime-opengl --target "$target")
    done
    lipo -create -output "$OUT/librashader.dylib" \
        "$SOURCE/target/aarch64-apple-darwin/release/liblibrashader_capi.dylib" "$SOURCE/target/x86_64-apple-darwin/release/liblibrashader_capi.dylib"
    install_name_tool -id "@rpath/librashader.dylib" "$OUT/librashader.dylib"
    strip -x "$OUT/librashader.dylib"
    cp "$SOURCE/LICENSE.md" "$OUT/librashader-LICENSE.md"
    cp "$SOURCE/LICENSE-GPL.md" "$OUT/librashader-LICENSE-GPL.md"
    rm -f "$OUT"/version-* && touch "$OUT/version-$LIBRASHADER_VERSION"
fi

if [ ! -d "$OUT/shaders" ] || [ "$ROOT/port/main.cpp" -nt "$OUT/shaders" ] || [ "$ROOT/tools/shaders.py" -nt "$OUT/shaders" ]; then
    fetch "slang-shaders-$SLANG_SHADERS_COMMIT.tar.gz" \
        "https://github.com/libretro/slang-shaders/archive/$SLANG_SHADERS_COMMIT.tar.gz" "$SLANG_SHADERS_SHA256"
    [ -d "$DEPS/slang-shaders-$SLANG_SHADERS_COMMIT" ] || tar -xzf "$DEPS/slang-shaders-$SLANG_SHADERS_COMMIT.tar.gz" -C "$DEPS"
    python3 "$ROOT/tools/shaders.py" "$DEPS/slang-shaders-$SLANG_SHADERS_COMMIT" "$OUT/shaders" >&2
fi
echo "$OUT"
