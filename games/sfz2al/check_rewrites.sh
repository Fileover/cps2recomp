#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy
#
# Checks Alpha 2's rewritten routines (games/sfa2/src), which this game builds too, against this game's own code:
# Alpha 2's check, its movies played on this game (games/sfa2/check_rewrites.sh). Run from the repository's top.
exec sh games/sfa2/check_rewrites.sh sfz2al
