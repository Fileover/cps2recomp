# Court

A ball game for the fighting games, on the in-game menu's **Alternative modes** page (first in the Vampire Savior
games: Vampire Savior, Vampire Savior Definitive, Vampire Savior 2 and Vampire Hunter 2, which share its sources). The stage is a court cut in two by a line down the middle of the picture. Each fighter keeps
to its own half, and the two of them hit a big ball that flies over the whole court. Get the ball past the other
fighter and off their end of the court for a point. First to 3, 5 or 10 wins.

## Playing

- **Hitting the ball.** Any attack that touches the ball hits it. Harder attacks send it faster: light, medium, hard,
  then special moves, then moves made with a gauge (ES and EX moves, supers). Where the attack meets the ball decides
  the angle: strike it below its middle and it goes up, above its middle and it goes down (a spike), squarely and it
  flies straight. It always goes toward the other half. Fireballs and other thrown attacks hit it too, a little softer.
- **Spin.** A ball struck above its middle spins forward and kicks on when it bounces. Struck below, it spins back and
  checks up.
- **Three in a row.** Three hits by the same fighter in a row (each more than a moment apart, and not too long) make a
  **power shot**: much faster, glowing orange.
- **Knock-downs.** The ball bounces off a fighter softly. A hot ball (a power shot or a super) or one that is simply
  fast enough, hit by the other fighter, **knocks down** whoever it hits.
- **The line.** The line only separates the fighters: the ball flies over it. A fighter only hits the ball over their
  own half, never reaching across for it; but fireballs and other thrown attacks hit it anywhere.
- **The court** is the whole stage: in Vampire Savior 768 columns, twice the board's picture, from 256 to 1024 (the
  camera held in the middle), and the fighters start in the middles of their halves. A gentle ball dropping on a fighter (a serve on a
  head) rolls off towards the line, never off the back to its own end.
- **Points.** The ball off an end of the court is a point for the other fighter; a ball lying still stays in play
  until someone strikes it. The next serve drops on the half of whoever lost the point.
- **Nobody is caught.** The fighters can be neither hit nor grabbed while the court runs: a command grab that reaches
  across the court (Anakaris's coffin) catches nothing.
- **The gauge** fills as it always does when attacks are thrown, so supers come round.
- Nobody loses health and the clock stands still.

The menu's **Court** page has Start and these choices:

| Row | Choices |
| --- | --- |
| First to | 3, 5 or 10 points |
| Player 2 side | The CPU, or player 2 |
| CPU level | Easy, medium or hard |
| Camera | **Follow the play** (the default: as close as shows both fighters and the ball, to 1.3x a 16:9 picture; out to the whole stage, black above it, when they are apart) or **Whole court** (the whole stage always) |
| Fighter 1, Fighter 2 | Someone at random, or a character |
| Stage | Random (of the game's own), or any stage: the game's own, then the other games' (guest stages, each needing that game's ROM set) |

## How it is made

`fight/court.{h,cpp}` holds the game itself, the same for every fighting game: the ball's flight and bounces, what an
attack does to it, the rules and the score, the CPU's side, the ball and the line drawn, the score shown, and the
menu's page. It runs as one of the battle modes' kinds (`fight/battle.cpp`, `Kind::Court`). The battle starts the
match (the fighters and stage from the page's rows, nobody played by the game's own computer, no clock, no
knock-out), holds the camera in the middle of the stage, and calls the court before and after each frame.

- **The court** is the picture: its left and right edges in the stage (wide or not) are the ends, and the line is
  their middle. The ceiling is the picture's top, below the game's panel (`pictureTop`).
- **Units** are pixels and frames (a sixtieth of a second), floats in the court's own state. Heights are the game's
  (y upwards; `floor` is a fighter's standing height).
- **Contacts** come once a frame from the game's sources (`Description::shapes`): every attack box that is out (the
  fighters' and what they threw, each with its power and an id that stays the same while the same attack is out) and
  the fighters' bodies (the boxes they can be hit in). The strongest attack touching the ball strikes it, once per
  attack (`id`). Bodies bounce it softly and never let it rest on them. Attack boxes reach `kStrikeSlack` pixels
  further for the ball than for a fighter, and the CPU's reach further still by level (`kComputerSlack`): a big ball is
  met generously, and the CPU's play is not exact.
- **Drawn** as round boxes over the picture (`HudBox::round`, `port/hud.cpp`), a frame behind like the game's sprites
  (`shown`). The overlay is zoomed with the picture, the zoom by distance too: `Port::Windowed` draws the boxes on the
  panel-less picture it zooms.
- **The court's width**: the game's wide picture, forced while the court runs and as wide as the stage (vsav's
  `widescreen.cpp`: `court::On()` counts as the feature on and played in, its side `Video::kMaxSide`, 192 columns, where
  the widescreen feature's is `Video::kWideSide`, 64; the camera's ends both put in the middle). The renderer draws up to
  192 columns a side (`kMaxSide`: its line buffers and the GPU's textures sized to match); the stage's far plane that
  is drawn as the camera moves comes from the stage's plan (vsav's `SideTile`), as for widescreen.
- **The picture shown** (`Game::framedColumns`, `framedFollows`; `Port::Framed`): a picture wider than the window's
  shape is looked at through the court's own camera into a 16:9 frame at three times a 512-column picture's size, made
  on the CPU (the GPU's own picture is not used meanwhile), the panel and the readout laid over it where a 512-column
  picture has them. The camera eases to as close as shows both fighters (`Game::focus`) and the ball
  (`Game::focusAlso`), at most 1.3x, and out to the whole picture, its floor at the frame's bottom and black above. The
  menu is drawn over the whole picture in that frame. Screenshots (`--shot-every`) are the whole 768-column picture;
  `--present-every` is the frame. and `Game::zoomWanted` (the court asks for the zoom by distance while it runs, unless its page says Whole
  court; the player's own zoom comes back after).
- **Save states**: the court's state is one POD (`court::State`, in the battle's).
- **The CPU's side** (`Computer`): where the ball will come down on its half (a copy of the ball flown on, no
  fighters), standing a little behind that spot. It tries each button strength, with that strength's startup frames
  and reach, against where the ball will be when the attack comes out. It jumps only for a ball over its own half,
  crouches for a ball on the floor, and now and then (medium and hard) does a quarter-circle-forward special. It plays
  from the court's own controls, never the game's computer, which knows nothing of a ball.

### Tuning

The feel is in the constants at the top of `fight/court.cpp`: `kRadius` 15, `kGravity` 0.13, `kSpeeds` (4.4, 5.7, 7.0,
8.0, 10.5 by power), `kKnockDownSpeed` 9.6, the bounce and grip factors, `kStreak*`, `kRally*` (the ball gets a little
faster each time it crosses the line in a rally), `kFreezes` and `kBallFreezes` (how long the attacker and the ball
are held when it is struck: the hit's weight), `kDeadBall`.

## Adding it to another game

A game needs the battle modes first (`battle::Register`, [FIGHT.md](FIGHT.md#the-fighting-modes)). Then:

0. **The wide picture forced while the court runs, as wide as the stage** (the game's widescreen code: `court::On()` as
   the feature on, its side `Video::kMaxSide` or what the stage needs, the camera's ends both in the middle; then
   `Game::framedColumns` shows it in a 16:9 frame).
1. **`games/<id>/src/court.cpp`** with a `RegisterCourt()` filling `court::Description` and calling `court::Register`.
   Call it before `RegisterBattle()` in the game's `game.cpp`: the battle makes the court's page only for a game that
   described one. Vampire Savior's (`games/vampire_savior/src/court.cpp`) is the model.
   - `shapes`: each player record's and thrown record's attack box now (the same read as the game's own hit test, or
     the training hit boxes', `boxes.cpp`), with its side, its power and an id (record address and the attack's number
     in its entry); and both fighters' hurt boxes as `kBody`.
   - **Power** from the attack's own data. In Vampire Savior, byte `+0x12` of the attack's entry is 1, 2 or 3 for a
     button's light, medium or hard attack and higher for a special move (each its own value: Demitri's fireball 5, its
     ES version 9, his uppercut 7, Lilith's rising blade 0F). So a **super's** power is told by a gauge spent: the
     player's stock byte going down marks the move under way as a super until the fighter can act again. Find the
     byte by logging entries as attacks come out (the `VSAV_COURT_LOG=1` log in vsav's file) while a movie or the CPU
     presses each button and a few specials.
   - `place`: a fighter put at x (its record's place), for the start of a game.
   - `fighterX`, `fighterY`, `halfWidth`, `canAct`, `floor`, `pictureFloor`, `pictureTop`, `wideStage`: as the
     training session's description has them.
   - `freeze`: the attacker's hitstop byte. `playSound` and `hitSounds`: the game's own hit sounds. Find them by
     tracing the sound queue (`--trace` of the ring) while hits land in training: Vampire Savior's are 0100, 0101, 0102
     for light, medium, hard.
   - A game that builds another's sources through a family (vsav2 and vhunt2 build vsav's) needs each new `At(...)`
     place in its `family.h` table (`tools/places.py`, or by hand: FF8135 was added so, between two neighbours that
     stayed put); the build stops on a place a table lacks.
   - `everyFrame`: anything to keep set while the court runs. Vampire Savior's "first attack made" byte (FF8135),
     else the game says FIRST ATTACK when the ball knocks someone down; and each fighter's "cannot be hit" counter
     (+147) held up, which the game's grab test (029406) checks, or Anakaris's coffin catches the other fighter across
     the court (`--watch` on the victim's control byte found the routine: 029482 writes the caught state).
2. **Keep the fighters apart.** Where the game pushes the two players apart (vsav's `PushApart` rewrite), while
   `court::On()`: put each one's x through `court::KeepToHalf(side, x, halfWidth)` and push nothing.
3. **Keep their attacks off each other.** In the game's hit tests (vsav's `PlayersAgainstPlayers` and
   `ThrownAgainstPlayers` rewrites), test nothing between the players while `court::On()`. Throws need no change: the
   two never stand close enough.
4. **The knock-down.** At the player's step (vsav: the `PlayerStep` hook, `CourtStep`), when
   `court::KnockDownWanted(side, away)`: put the fighter in the state a hit that knocks down leaves (vsav: state
   `02020000`, reaction byte `+54` = 4, `+117` = 1 so it lands lying) and let the game's own reeling run from there.
   The game throws it the way its (here missing) attacker faced, so turn its speed across to `away` while it flies.
5. Build, and check with the test switches below: rallies, a knock-down (`CPS2RECOMP_COURT_SHOT`), the zoom
   (`--present-every`).

A game whose players are not on a fixed 384-column picture (a tag game's partner records, a game whose camera's x is
not the picture's left edge) needs those cases in its `shapes` and in the battle's camera facts. The toolkit does
not care what the fighters are, only where their boxes are.

### Test switches

| Variable | What it does |
| --- | --- |
| `CPS2RECOMP_COURT_CPU=1` | Player 1's side played by the court's CPU too: a whole game with no input |
| `CPS2RECOMP_COURT_LOG=1` | The ball, the fighters and the score every 6 frames, and each press, hit and bounce (with the machine's frame, the `--shot-every` pictures' numbers) |
| `CPS2RECOMP_COURT_BOXES=1` | The boxes the ball was tested against, drawn (attacks red, bodies blue), and a line per frame |
| `CPS2RECOMP_COURT_SHOT=X,Y,VX,VY` | The first serve sent off from there at that speed, hot, as if player 1 had struck it (a knock-down test: `600,110,10,0.5`) |
| `CPS2RECOMP_BATTLE_SEED=N` | The battle's choices at random the same each run |

For example, a whole CPU-against-CPU game at the hard level, seen every 6 frames:

```
CPS2RECOMP_COURT_CPU=1 CPS2RECOMP_BATTLE_SEED=7 build/vampire_savior/vampire_savior --rom rom/vampire_savior \
  --headless --settings /tmp/court/settings.ini --keys "5:fcourt_level=2,10:ccourt_start" --frames 6000 \
  --shot-every 6:/tmp/court/f
```

## Next

- **Guest stages' ends**: a guest stage narrower than 768 columns leaves black at the picture's edges (its art ends
  there); the court's ends could stop where the art does.
- **Tall stages**: the Marvel games' stages (as guests) with the ball going up past the picture's top and the camera
  following, past the board's picture vertically (the renderer drawing more of the stage than the game asks for).
- **The panel**: hide the health bars and clock, keep the gauge (`court::Description::hidden`, which the battle puts
  in as `Video::hideObject` while the court runs). In Vampire Savior palette 0 is the bars' frames top and bottom
  alike and palette 2 the clock's digits, so it takes tile numbers: about 13E00-13E98 for the bars and 13C90-13CCA for
  the clock against 13C50-13C70, 13CD0-13CF0 and 13DE6 for the gauge, checked on every character's panel.
- **Power-ups** (more power, other physics) and **combos with properties** (three specials in a row giving the ball a
  property of its own), a tip-off or face-off serve.
- **The CPU**: smarter positioning (a lob over a fighter near the line, waiting for a ball to drop), specials of each
  character's own.
- More games: Alpha 2 next (its collision is rewritten), then the Marvel games.
