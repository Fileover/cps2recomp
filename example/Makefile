# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
# A stand-in for a game's repository, with no game: what a game's Makefile looks like, and what this
# repository's CI builds (the program with no translated code, and the launcher). A real game's
# repository has a copy of the toolkit in cps2recomp/ and says CR ?= cps2recomp; it does not set NO_ROM.
GAME ?= example
CR ?= ..
NO_ROM = 1
include $(CR)/build.mk
