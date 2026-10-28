# Online play

Two players, each on their own machine, play a match with **rollback**: your own controls answer at once, whatever
the distance. Any mix of macOS, Linux, the Steam Deck and Windows can play each other.

## Starting a match

Each player needs the game and its ROM set. On the game's page in the launcher, under Play, the **Online** card has
**Host** and **Join**. The host
presses Host, and their game shows a **match code** (`K7Q-M2X`); the other player types it into Join. Nothing needs
setting up on either router. The host is player 1. The game's window says it is waiting until the other player comes
(Esc gives up). From a terminal:

```sh
cps2recomp-launcher --play sfa3 -- --host                  # the host: shows a code, waits for the other player
cps2recomp-launcher --play sfa3 -- --join K7Q-M2X          # the other player: the host's code (or address)
```

## The Online page and the lobby

- **Join with a code** on the launcher's Online page: type a code a host gave you, for a match in any of the games,
  and press Join: the launcher starts the game the match is in.
- **The lobby** (opt-in): switch on **List my matches in the lobby** on the Online card and give **Your name**, and the
  matches you host are listed under **Open matches** on every other player's card for that game, until someone joins;
  they press Join. Matches not listed are joined only with their code.
- **The Online page** in the launcher's sidebar shows every game's open matches at once, each with Join, and hosts a
  match in the lobby in whichever of your games you choose. A match in a game whose ROM set you haven't added is shown
  greyed, with **Add…** to go and add it.

## How it connects

- **How the code works**: a small service of ours introduces the two games, telling each where the other can be
  reached, and they then connect straight to each other through their routers; none of the match goes through it
  ([online/README.md](../online/README.md)). A code lasts 30 minutes and is good for one match.
- **Joining by address** works too, without the service: on a home network, the host's own address (the Online card
  lists it); over the internet, the host's public address with UDP port 7845 forwarded to it on the router (`--host
  7900` and `--join ADDRESS:7900` for another port); or a VPN such as [Tailscale](https://tailscale.com). That is
  also the way for the rare two networks a code can't connect (some mobile networks).

## In a match

- **Each player presses Start to come in**, as at the arcade: the first to press plays the computer until the other
  presses theirs ("Here comes a new challenger").
- **Each player plays with their own player 1 controls**, and hears and sees the game as their own settings have it
  (the Display page, the shader, the zoom, the custom soundtrack). The match takes the host's board settings
  (difficulty, timer, rounds...) and the host's features that change play (free play, arcade timing,
  widescreen, hidden characters...), for the match only: the other player's own are left as they are.
- **Rollback**: each side runs the game with both players' controls, its own player's held back 2 frames (as a
  board's game might be; `--net-delay N` on the host, 0 to 8), and the other player's guessed until they come. When
  a guess was wrong, the game goes back to that frame and runs on again with the real ones, unseen and unheard, in
  the time of one frame. Up to 8 frames are guessed; past that the game waits for the other side.
- **The two games are checked against each other**: every second each side sends the other a fingerprint of the
  game; if they ever differ, the match stops and says so (a desync, which should not happen).
- **The ping** is said at the bottom of the screen all through a match ("PING 42 ms"), and the input delay too if you
  like ("DELAY 2"): each is a switch, **Ping** (on) and **Input delay**, on the Online page under In a match (and on the
  Features page).
  "WAITING FOR THE OTHER PLAYER" shows if their game goes quiet for a moment, and **PRESS START TO JOIN** until you
  have.
- **Coming back after a drop**: if the other player's game closes or their connection drops, the host's game stays
  open and waits for them with the same code; a joining player whose connection dropped is taken back to the host by
  itself. The two games then start over together from the title. (The host closing their game ends the match.)
- **Win streaks**: in the fighting games, a player who wins two or more matches in a row against the other has it
  said under their life bar ("STREAK 3"), counted from when both have pressed Start, until the streak ends.
- **Leaving a match**: Esc, or the gamepad's menu button, asks **Leave the match?** over the game (which goes on);
  Confirm leaves, Back stays.
- **Off during a match**: the cheats (on both sides, for a fair match), the in-game menu, save states, pause, the speed, run-ahead, the training mode and the
  other modes (endless battle, tag team, the viewers). A match ends when either player closes their game.
