**What this changes and why**

**Checklist**
- [ ] Nothing from a game (files, generated code, entry lists, listings, dumps, save states, input movies, screenshots), no key and no emulator's code is added. `make audit` passes.
- [ ] Nothing here is about one game: what a game needs goes in that game's `game.cfg` or sources.
- [ ] `make check` passes here. If the translators, the machine, the picture or the sound changed: `make CR=../cps2recomp check` and `check-rewrites` pass in a game's repository (say which game).
- [ ] No added input lag or slowdown.
- [ ] Docs and `CHANGELOG.md` updated if behavior changed.
