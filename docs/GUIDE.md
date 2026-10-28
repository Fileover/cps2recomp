# Player's guide

The [README](../README.md) has the main features and how to start. This is the rest: the launcher's pages, the
online details, the custom soundtrack, updates, portable mode and the terminal.

## The launcher's pages

Each game you have a ROM set for gets a page of its own in the sidebar: its Display switches (full screen,
widescreen), Play, and the features only it has. The **Features** page has what the games share (free play,
cheats, run-ahead, the picture, training mode's displays and the dummy, and the rest), set for every game at
once; a feature changed in a game's own menu is set for the others too when you leave the game. The
**Controllers** page sets the keys and buttons for all of them; its Layout for chooses a game, and Its own layout
gives that game keys and buttons apart from the others', under the game's own names for its buttons.

**Multi game** (the page after Controllers) plays several games one after another, for as long as you like. Its
**Endless battle** goes through every stage of a fighting game with the game playing both fighters, then on to the
next game's stages, and round again after the last: choose the games (every fighting game, or only some), their order
(the sidebar's, or shuffled), how the stages change (**Shuffle**, each fading into the next after the time you choose,
or **Scroll**, going by in order at the speed you choose), the **Screen** (every game full screen, every game in a
window, or as each game is set), the **Picture** (every game widescreen, every game 4:3, or as each game is set) and
the rest of the Display page every game has (**Invisible walls**, **Scaling**, **Picture shape**, **Window size**,
**Shader**, each also as each game is set) and the **Zoom** (**Both fighters**: in every game the camera follows the
two fighters, as close as they allow), then **Start**. These are for that run only: each game's own settings stay as they
were. In a window, each game opens where the one before it was and as large, wherever you moved it. Touch
the controls to take a fighter over. While it plays, **N** (or a gamepad's left shoulder) fades out to the next game
and **F** (or the left trigger) held fast-forwards at four times the speed; the page lists them under **While it
plays**. Quitting a game (its menu's Quit game, or closing its window) ends the run, as does **Stop after this
game**.

![The Multi game page: the endless battle through every stage of each game you choose](screenshots/launcher-multi.png)

The games are in your order: drag a game's row in the sidebar up or down to keep the ones you play most at the top
(with a gamepad or the keyboard, **Move up** and **Move down** on its page, under Play). The Games page lists them
the same way. Playing only one or two of the games? Switch on **Only the games you have** on the Games page, and the
others are left out of it and of the Features and Controllers pages (**Choose folder…** still finds them). On a small screen, or with large text, the launcher closes up to fit: a narrower sidebar whose list
scrolls, and each row's buttons under its words.

## Shaders

The picture through one of RetroArch's shaders, by
[librashader](https://github.com/SnowflakePowered/librashader): a curved CRT, CRT Lottes, CRT Royale, CRT Guest or
a light CRT; scanlines light, strong, as an OSSC draws them, or dark; or xBRZ and ScaleFX to smooth the pixels'
edges (the Display page's **Shader**). **Your own** takes any RetroArch slang preset (`.slangp`) you put in a
`shaders` folder beside the game's settings, with the files it names (a few need a newer OpenGL than macOS has; Linux and Windows have it).
Under Shader on the launcher's Features page, **In the game's menu** → **Choose** keeps the in-game menu's list to
your favourites. Zoomed in, the camera closes in on the shaded picture, scanlines and all, as on a monitor. The menu is drawn over the picture without the shader. A shader that cannot be made leaves the
picture as it was.

| | | |
| --- | --- | --- |
| ![No shader](screenshots/shaders/off.png)<br>Off | ![CRT (crt-geom-mini)](screenshots/shaders/crt-geom-mini.png)<br>CRT | ![CRT Royale](screenshots/shaders/crt-royale.png)<br>CRT Royale |
| ![CRT Lottes](screenshots/shaders/crt-lottes.png)<br>CRT Lottes | ![Scanlines, light](screenshots/shaders/scanline-fract.png)<br>Scanlines, light | ![xBRZ](screenshots/shaders/xbrz.png)<br>xBRZ |

## Online play in detail

Two players, each on their own machine (macOS, Linux, the Steam Deck or Windows, in any mix), each with the game and
its ROM set. On the game's page in the launcher, under Play, the **Online** card has **Host** and **Join**. The host
presses Host, and their game shows a **match code** (`K7Q-M2X`); the other player types it into Join. Nothing needs
setting up on either router. The host is player 1. The game's window says it is waiting until the other player comes
(Esc gives up). From a terminal:

```sh
cps2recomp-launcher --play sfa3 -- --host                  # the host: shows a code, waits for the other player
cps2recomp-launcher --play sfa3 -- --join K7Q-M2X          # the other player: the host's code (or address)
```

- **Join with a code** on the launcher's Online page: type a code a host gave you, for a match in any of the games,
  and press Join: the launcher starts the game the match is in.
- **The lobby** (opt-in): switch on **List my matches in the lobby** on the Online card and give **Your name**, and the
  matches you host are listed under **Open matches** on every other player's card for that game, until someone joins;
  they press Join. Matches not listed are joined only with their code.
- **The Online page** in the launcher's sidebar shows every game's open matches at once, each with Join, and hosts a
  match in the lobby in whichever of your games you choose. A match in a game whose ROM set you haven't added is shown
  greyed, with **Add…** to go and add it.
- **How the code works**: a small service of ours introduces the two games, telling each where the other can be
  reached, and they then connect straight to each other through their routers; none of the match goes through it
  ([online/README.md](../online/README.md)). A code lasts 30 minutes and is good for one match.
- **Joining by address** works too, without the service: on a home network, the host's own address (the Online card
  lists it); over the internet, the host's public address with UDP port 7845 forwarded to it on the router (`--host
  7900` and `--join ADDRESS:7900` for another port); or a VPN such as [Tailscale](https://tailscale.com). That is
  also the way for the rare two networks a code can't connect (some mobile networks).
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

## A custom soundtrack

Each game can play your own music files in place of the board's music: an arranged soundtrack, say. The sound
effects and voices stay the board's.

1. Switch on **Custom soundtrack** (the Features page, under Sound; it is on in Enhanced, off in Pure).
2. On a game's page, under Sound, **Choose…** the folder your music files are in (`.ogg`, `.flac`, `.mp3` or
   `.wav`). With none chosen, the game uses the `soundtrack` folder in its own settings folder
   (`~/Library/Application Support/<Game>/` on macOS, `~/.local/share/<Game>/` on Linux and Steam Deck,
   `%APPDATA%\<Game>\` on Windows, `user/<Game>/` beside the launcher in [portable mode](#portable-mode)), which
   it makes.

That is all it takes: **a folder of music plays as a playlist**, in place of all the game's music, one file after
another in their names' order (`2 ...` before `10 ...`), and from the first again after the last. Choose **On,
shuffled** for a random order.

**To replace particular songs** instead, name each file after the piece of music it replaces: `ryu.flac`,
`player_select.ogg`, `ken_ending.mp3`. The game writes `songs.txt` into the folder, with every piece of its music
by name and number and what it is (Street Fighter Alpha 2 has 59: each character's theme, each ending, the staff
rolls, the title and the screens between fights). Once one file is named for a song, the folder works this way: a
song you have no file for plays as the board has it, and other files are left alone.

For files with other names, loops and loudness, put `soundtrack.ini` in the folder:

```ini
volume = 80                  ; every song, in percent

[ryu]                        ; a song by its name (or its number: [01])
file = 05 Ryu's Theme.flac
loop = 0:12.5 - 2:47.25      ; the part that repeats: start - end (minutes:seconds, or seconds)
volume = 90
```

A song's file repeats whole unless it says otherwise: by `loop` in `soundtrack.ini`, `loop = no` to play it once, or the
loop tags many soundtrack packs have (`LOOPSTART`, with `LOOPEND` or `LOOPLENGTH`, in samples). The music the board
plays once (a fanfare, the versus screen, a game over) is played once. The music keeps the game's time: pausing,
the menu, slow motion, a save state and run-ahead leave it where the game is (a playlist plays on, and waits only
while the game is held). The game plays exactly the same with
or without it.

No music is included, and this project can't help you get any: use files you made from discs you own.

## Updates

When the launcher opens it asks GitHub for this project's releases (the Updates card, at the top of the Games
page, has a switch to stop that, and **Check now**). When there is a newer version it shows at the top of the Games page: **What's new**
opens its notes, and **Update** downloads the new version's download for your system (the same file as on the
releases page), checks it and every file that changed against the release's list, and only then
puts them all in place; if anything goes wrong nothing is changed. The launcher then starts again as the new
version. Your settings, save states and ROM sets are not touched. Quit the games first: Update waits until none is
running. A beta is offered only to a beta.

The launcher needs to be able to write to its own folder: on macOS, move `CPS-2 Recomp.app` to Applications (or
anywhere of your own) before opening it the first time, not run it from the downloaded zip's folder. A copy built
from source is told of a new version but not changed.

## Portable mode

To keep everything with the launcher (on a USB stick or an external drive you take from one computer to another),
put an empty file named `portable.txt` beside it:

| System | Where `portable.txt` goes | Where your things then are |
| --- | --- | --- |
| Windows, Linux, Steam Deck | in the `CPS2Recomp` folder, beside `cps2recomp-launcher` | `CPS2Recomp/user/` |
| macOS | in the folder `CPS-2 Recomp.app` is in (not inside the app) | `user/` in that folder |

Settings, save states, each board's settings memory, custom colours, the score attack records, a soundtrack in
the game's own folder and the logs then go in `user/`, a folder for the launcher and one for each game, laid out
as in your user folder. The launcher's Games page shows where they are under **Your files** (**Show** opens the
folder).

- **The first time**, if your user folder already has the games' things, the launcher asks whether to copy them
  to `user/`. Copying never takes anything from your user folder and never writes over a file.
- **To stop**, take `portable.txt` away: the launcher uses your user folder again, as it was when you started
  portable mode. Nothing is moved either way; to carry things back, copy the folders in `user/` to your user
  folder yourself (with the launcher closed).
- **Updates** keep `portable.txt` and `user/`: they change only the release's own files.
- **The folder has to be one you can write to.** If it is not (a read-only drive, or Windows' Program Files), the
  launcher says so and stops rather than keep your things somewhere else.
- ROM sets are still only read, wherever they are. A ROM folder chosen on a drive letter (Windows) or a mount
  point (`/Volumes/...`, `/media/...`) is remembered as that path: on another computer, choose it again if the
  drive is somewhere else. A Steam shortcut (**Add to Steam**) is for the computer it was added on.

## From a terminal

```sh
cps2recomp-launcher --play sfa2                 # straight into a game, as the launcher has it set
cps2recomp-launcher --play sfa2 -- --features widescreen,no_damage
                                                # with more arguments for the game, after the launcher's own
cps2recomp-launcher --page vampire_savior       # the launcher, at a game's page
sfa2 --rom ~/roms/sfa2u.zip --settings my.ini   # a game's own program (beside the launcher), by itself
sfa2 --soundtrack ~/music/sfa2-arranged          # a custom soundtrack's folder (with the feature on)
sfa2 --help                                     # what a game's program takes
```

The games' ids are `ssf2t`, `sfa`, `sfa2`, `sfz2al`, `sfa3`, `pocket_fighter`, `msh`, `xmvsf`, `mshvsf`, `mvsc`, `puzzle_fighter`, `vampire_savior`, `vsav2`, `vhunt2`, `vsav_definitive`, `avsp` and `dino`. `--features` may be given more than once, and
a later one wins.

## Coming from a game's own download

Earlier releases came one game at a time (Street Fighter Alpha 2 Recomp, Vampire Savior Recomp). This launcher
keeps each game's settings, save states and board memory in the same folder as before, and takes the ROM set you
chose in a game's own launcher, so everything carries on. A game's own release still works.
