# Darkstalkers: The Night Warriors: progress

Issue #215. The set is `dstlk`, Darkstalkers: The Night Warriors (Europe 940705), MAME's parent and the set FBNeo's
training mode plays. Every file's CRC32 checked against MAME's `cps2.cpp`. Sound driver 1.05A.

## Made from Night Warriors' program

`tools/portmap.py` from Night Warriors places 48% of its translated instructions here (674 anchors), so Darkstalkers
builds Night Warriors' sources (`games/nwarr/src`, game.cfg's `sources`), as Vampire Hunter 2 builds Vampire
Savior's: those sources name their places as `nwarr::At(0x...)`, and `family.h` here says where each is in this game
(`tools/places.py`, then by hand). Facts that differ between the two are in each game's `family.h`.

## How it differs from Night Warriors (found here)

| What | Night Warriors | Darkstalkers | How found |
| --- | --- | --- | --- |
| Player records | FF8388, 0x500 apart | FF8388, 0x400 apart | the second searched for in a fight's dump |
| Record's end | choice +4A1, colours +4A6, person +49F | +3A1, +3A6, +39F (0x100 less) | 005F98 (the copy of the choice), 005DE2 |
| Edge byte (keep-in) | +6F | +6A | 006204 |
| Health writers | 35 | 21, 4 of them with no counterpart | `lst.py -r` |
| The show | 2 QSound, 4 moon, 6 intro and title, 0E-14 fights, 16 ranking | 2 intro and title (step 08 on), 4-0A QSound, 0C the map, 0E a fight, 10 ranking | watch, pictures every 200 frames |
| Choosing | FF8004 0C (speed, character, guard mode screens) | FF8004 0 (speed on the select screen) | watch |
| Thrown things | three a player | two a player (places.py: FFAA2E, FFAAEE player 2's; FFABAE, FFAC6E player 1's) | places.py |

The same: the board's start (2 to 10, then 0 at frame 447), FF8000 2 in a game, FF8004 04 the match, FF8008 0A
while a round is fought, health +42 (0x90), X +10, Y +14, the frame +1C, the box tables +5C, half width +1C0.
