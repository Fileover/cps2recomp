# The picture at the game's level

Today the 68000's code is translated ahead of time, and the sound is native for games with QSound driver 2.01b
(`sound/driver.cpp`, checked write for write against the Z80 with `--sound check`). The picture is not native yet:
`video/video.cpp` draws from the board's object memory, its CPS-A and CPS-B registers, and palette memory. That is a
model of the board's video chips, and a port should not ship one.

The plan: the games' drawing code hands a native interface each frame's objects, scroll layers, layer order and
palettes. A native renderer on SDL_GPU draws from that interface. The board model stays only as the reference, and
two checks run beside it every frame:

- **(a) the interface:** the records the rewritten game code hands over equal the records decoded from what the board
  model was given (`--video check`);
- **(b) the pixels:** a CPU rasterisation of those records equals `video.cpp`'s picture.

This page has the inventory the work starts from, the interface, the checks and the order of work. The first slice,
the palette's upload, is done: [below](#the-first-slice-the-palettes-upload).

All of it is written once, in the toolkit. The interface, the renderer, the decoder and the checks are in `video/`.
Capcom's video library, which the games share, is rewritten once in `video/lifts.cpp`. A game's `game.cfg` gives only where its copy of each part is. For a
routine it adds a CRC-32 of the routine's instruction words with their operands left out, which says the copy is the
version the toolkit has; a single instruction is checked to be the move the toolkit expects. Per-game code is only for a game whose drawing really differs; the games that need it are named
below.

## Finding out (`CPS2RECOMP_VIDEO_CENSUS`)

`CPS2RECOMP_VIDEO_CENSUS=FILE` with `--cpu blocks` (the interpreter, doing what translated code does) writes, at the
end of a run, every instruction that read or wrote the video hardware. Each line has the instruction's address, the
part reached, `W` or `R`, how often, the addresses it reached, and the interrupt masks it ran under. Mask 1 is the
main loop, 4 is the vertical blank's interrupt (level 2), 10 is the raster interrupt's (level 4), and 80 is start-up.

The parts are `O` object memory, `V` video RAM, `A` CPS-A, `B` CPS-B, `R` the object registers at 400000, and `K`
the object bank at 8040E0.

The census's output is the game's, so it stays outside the repository, like `work/`. Grouping the lines by routine
takes a few lines of Python over `work/<id>/program.lst`.

## The inventory

### Where the video hardware is written, in every game

The games run the same frame in the same order. Their main loop draws: it writes the next frame's object list into
the object bank not on screen, writes the scroll screens' maps and the row-scroll table in video RAM, and writes
colours into palette buffers in video RAM. It also sets *shadow* words in work RAM: scroll positions, layer
control, the palette buffer and its pages. Then the vertical blank's interrupt hands the frame to the board in one
place: the object bank swapped, the shadows copied to the CPS-A and CPS-B registers and the object registers, and
the palette's base written last (writing it makes the chip take the palette). Nothing in video memory is read back
as anything but the game's own data: the games read video RAM only to fade and change colours they wrote, and never
read object memory after start-up.

So the boundary is clear. **Video RAM (maps, row scroll, palette buffers) and object memory are the game's data
structures**, kept where the 68000 has them, as work RAM is. **What goes native is the board:** the registers'
meanings, the latching at the vertical blank, and the drawing. The game-side records are made where the game hands
the frame over, from the shadows the game keeps. They are not made from the registers.

### The families (static comparison of all 22 games, census of the representatives)

Each game's interrupt handlers (the vectors at 68 and 70) and the routines they call were compared by shape, with
operands left out. The library comes in these versions.

The **census** column says which game was run (attract mode and its ladder movie); the other games of a family
were only compared by shape.

| Family | Games | Frame's commit (vertical blank) | Palette's upload | Object list | Mid-frame changes | Census |
| --- | --- | --- | --- | --- | --- | --- |
| **Alpha** | sfa, sfa2, sfz2al, sfa3, pocket_fighter, puzzle_fighter; vampire_savior, vsav2, vhunt2, vsav_definitive | shadows at A5+40.. to the registers; scrolls (sfa2 0014CC); layer control and object registers (sfa2 0013CE) | **one routine, the same version in all ten** (sfa2 00123E, vsav 001762; CRC 770DFAA6) | main loop, the bank not shown; a list builder and its piece writers | none seen (the Vampire games set the raster counters and have a handler, 001372, not reached in these runs) | sfa2, vampire_savior |
| **Marvel** | xmcota, msh, xmvsf, mshvsf, mvsc | one long routine (msh 000584, 000800, 00088E) | inline: msh and xmcota a constant buffer on the frames a counter says; mshvsf, mvsc, xmvsf a variable | main loop; the pieces' tables read from video RAM | **yes**: msh's raster interrupt (0007B6) changes scrolls 2-3 and the layer control | msh |
| **SF2** | ssf2t, hsf2 | 001A6C, 001AD4 | inline: a variable, or the constant 9240 on some screens | **main loop and interrupt**, a copy in video RAM, read back, the bank swapped from the main loop too | none seen | ssf2t |
| **Darkstalkers** | dstlk, nwarr | 0007D4, 0041C4, 0043A2 | the Alpha shape without the next buffer (00089E: two instructions) | main loop; the bank swapped from the main loop too (0017F4, 001834); a piece writer reads video RAM | none | dstlk |
| **AvsP** | avsp | 000E0E, 001214, 0012EC | inline (000EC4) | main loop | none | avsp (attract only) |
| **Giga Wing** | gigawing | 000A7C, 000AD8, 0038AE, 00393C | inline (000B30), the pages after it | main loop | none seen (it sets the raster counters) | gigawing |
| **CPS-1** | dino, punisher | another board: objects in video RAM, latched by the CPS-A's base (`LatchObjectsCps1`), no object bank | a constant (9140) | | | not run |

Code of a game's own (or a family's) will be needed for: **SSF2T and HSF2**'s object list (two writers, read back),
**Darkstalkers'** bank swapped from the main loop, **Marvel**'s mid-frame changes, and the **CPS-1** games (another
board). The rest fits a library rewritten once with each game's addresses from `game.cfg`.

### Census: Street Fighter Alpha 2 (attract mode 4,000 frames, the ladder 4,800)

- **CPS-A, CPS-B, object registers, object bank:** written only in the vertical blank's interrupt (mask 4): the
  frame's commit at 001150 and its callees 00123E (palette), 0013CE (layers, object registers), 0014CC (scrolls).
  The start-up routines 00091A, 000968, 0009C0 and 000B48 set them once.
- **Object memory:** written only from the main loop, only the bank not on screen. 0127A4 starts the frame's list
  (two entries cleared, 0x380 entries at most) and calls the drawers by priority group. A dozen routines
  (0129B6-012FFE) put an object's pieces in the list, one entry each; 018820 writes the list's end.
- **Scroll maps:** about 15 routines in the main loop, from 0115FC to 06468E (stage tiles as the camera moves, text,
  the panel).
- **Row scroll:** 928000-92BFFF, written at start-up and by 0121CC and 0121F4.
- **Palette buffers:** about 40 routines (colours put, fades read and written) in 90C000-90CFFF and the other
  buffers the game switches between (92A0.., 9280.., 91C0..).
- **Raster interrupt:** not used.

### Census: Marvel Super Heroes (attract mode 4,000 frames, the ladder 6,000)

- **Registers:** the frame's commit in the vertical blank (000584, 000800, 00088E, 000742), as in Alpha but one long
  routine. The object bank is swapped as the interrupt starts (00053E).
- **The raster interrupt is used:** 0007B6 writes scroll 2 and 3's registers and the layer control part way down the
  picture (381 writes over the run: some screens only). The main loop sets the raster counters (CPS-B 4E-52) for it.
- **Object memory:** from the main loop, the bank not on screen. Some thirty piece writers from 019B02 to 01A5F6.
  They **read their pieces' tables out of video RAM** (924000-925FFF, 928000-92A0xx): the game keeps data there that
  the board never shows.
- **Palette:** the upload is only on frames when a counter says so (00066E): a constant buffer (9140), the pages
  always 3F.

### Census: Super Street Fighter II Turbo (attract mode 4,000 frames, the ladder 6,000)

The exception:

- **Object memory is written from the main loop and in the vertical blank's interrupt.** About thirty main-loop
  routines write each piece twice, to object memory and to a copy in video RAM (910000-910644). The interrupt
  (001D5C, 001D8E) copies that copy into the object memory's top part (708FB0-7095F4).
- **Object memory is read back** by the main loop (0C81E2, 0C83A4), and the object bank is swapped from the main loop
  too (0C682A, 0C69C2), as well as from the interrupt.
- **Palette:** a variable buffer, or the constant 9240 on some screens (001C5C, 001C6C).
- So its object list needs code of its own (HSF2's is the same program grown).

<!-- census: other families -->

### What each fighting game's stage code needs (for stages taken from one game into another)

The toolkit already shows other games' stages behind a game's fighters (`fight/guest_stages.h`; Vampire Savior
Definitive shows Darkstalkers' and Alpha 2's). It does that by running the guest's board beside the game's, drawing
the guest's whole picture, and laying the game's marked fighters and panel over it. With the interface, the guest's
code still runs on its own board (its CPU and RAM), but its picture is not drawn. Its stage's records (scroll
layers, row scroll, palette pages, stage objects) go into the game's frame as layers of their own.

What a stage needs. Two games are already described as guests (`games/<id>/guest.h`); the rest is to be found, one
game at a time, the same way: the census of a fight gives which scroll screens' maps and which palette buffers the
stage's code writes (by the CPS-A's bases at the time).

| | Scroll layers | Palettes | Stage objects | Camera | Music |
| --- | --- | --- | --- | --- | --- |
| Alpha 2 (`games/sfa2/guest.h`) | all three drawn today (`layers` 0xF); which carry the stage: to find | the scroll pages, and the object page's sets 27 and up | objects in colour sets 27 and up that are not the panel's tiles | asks where the players are (`Extents`, 01BFE0, 01C1D0); ends 256-640 | the stage's tune by number |
| Darkstalkers (`games/dstlk/guest.h`; Night Warriors at its own places) | all three today; to find | the scroll pages, the object page's sets 14 and up | colour sets 14 and up | an object stepped across and up (`Steps`, 0EDAF4, 0EDC1A); ends 256-640 | by number |
| Every other fighting game (sfa, sfz2al, sfa3, the Vampire games, the Marvel games, ssf2t, hsf2, pocket_fighter, xmcota) | to find | to find | its port's `notTheStage` where it has one | its port's camera facts (the widescreen and zoom features found them) | its port's `stageTunes` |

What this asks of the interface: **a frame holds layers from more than one producer** (the game and a guest). Each
producer has its own palettes, its own tile graphics, its own priorities, and its own scroll and row scroll. The
frame's layer order says how one producer's layers go between the other's: the guest's stage behind the game's
fighters, shadows and panel.

## The interface (`video/native.h`)

Made for a GPU to draw from: flat arrays of fixed size, uploaded as buffers. There is nothing to walk and no pointers
into the 68000's memory.

```cpp
struct Producer {                  // the game, or a guest whose stage is shown
    Graphics *tiles;               // its graphics ROMs as an indexed texture (once, at load)
    uint16_t palette[6][512];      // after this frame's uploads (the colour words; the shader makes RGB)
    struct Layer {                 // scroll 1-3
        uint8_t tileSize;          // 8, 16, 32
        bool on;
        uint16_t scrollX, scrollY;
        const uint16_t *map;       // 64 x 64 entries of (tile, attributes): a GPU buffer, re-sent when written
        bool rowScroll; uint16_t rowOffset; const uint16_t *rows; // 1,024 words (scroll 2)
    } layers[3];
    struct Object { int16_t x, y; uint32_t tile; uint16_t attributes; uint8_t level; } objects[1024];
    int objectCount;
    uint16_t layerControl;         // order and enables
    uint16_t priorityMasks[4];     // which pens of a scroll screen are over the objects of each level
};
struct Band { int firstLine; /* the registers above that changed from this line */ };
struct Frame {
    Producer producers[2];         // [0] the game; [1] a guest stage, or none
    struct Slot { uint8_t producer, layer; } order[8]; // back to front; a guest's layers between the game's
    Band bands[8]; int bandCount;  // raster effects: what changed part way down
    int side;                      // widescreen columns each side (Video::wide)
};
```

On the GPU, each scroll layer and the object list are instanced quads, one per tile, with the tile number, place,
flips and colour set in the instance. The fragment shader reads the indexed tile texture and the palette (a
512 x 12 texture: two producers' six pages). Priority between objects and scroll pens (the CPS-2's masks) is a
per-pixel decision, so each layer writes its pen's priority group to a stencil or depth value, and the objects test
against it. Row scroll is a per-line offset buffer for the scroll 2 pass, and bands are a scissor per run of lines.

The **CPU rasteriser** draws the same `Frame`, line by line, for check (b). It is the reference against
`video.cpp`. It does not draw what the player sees.

Done so far: `PaletteUpload` (one upload: the pages taken, the buffer, the colours).

## The checks

- **(a)** `--video check`. Each lift hands the interface its record, then makes the board writes the game's code made.
  The board model decodes the same record from its registers and memories at the moment it acts on them
  (`video.cpp`'s reading of them). The two must be equal. A record only the board made comes from game code not
  native yet, and is counted with the first frame it came in. Any of those, a record only the game's side gave, or one
  that differs, is a failure (exit 6). Each machine has its own interface (`Machine::nativeVideo`, reached by the
  lifts through the CPU): the spare and guest boards run the same code, and only the port's own machine is checked. This does not run with `--check-rewrites`, which runs the original as well and so
  makes every upload twice.
- **(b)** (to come) the CPU rasteriser's picture against `video.cpp`'s, every line, with `--video check`.
- The rewrites among the lifts are checked against the game's code by `--check-rewrites` and `make check-rewrites`.
  The hooks among them only read, but they are places translated code stops, which could move where an interrupt is
  taken. So for both, a run's frame hashes (`--hash-log`) must be the same as a build's without them.
- The game's side keeps a little state of its own (the pages last asked for). It is not yet in the save state, so a
  check run is to start from the program's start, not from a loaded state or a rollback.
- **Board timing:** with `arcade_timing`, rewrites run only when a game's features need them, and then without the
  bus's waits (`runtime/recomp.cpp`), as for every rewrite. A video lift that is to run always must count the waits
  of the instructions it replaces. That comes before the native renderer is the default.

## The first slice: the palette's upload

The upload is what each family hands the board at one or two instructions. Two kinds of lift cover them all.

**The Alpha library's commit routine**, in the vertical blank's interrupt, is rewritten once (`CommitPalette`):

```
move.w pages(a5),<CPS-B palette control>   the pages the chip is to take
move.w base(a5),<CPS-A palette base>       where from; writing it makes the chip take them
move.w next(a5),base(a5)                   next frame's buffer, as the game asked this frame
rts
```

Its operands (the three variables, the two registers) are read from the game's own copy, which `game.cfg` names with
its version: `video_palette_commit = 00123E:770DFAA6`. At start-up the toolkit checks the CRC.

**Everywhere else** the upload is one instruction inside a bigger routine (the other families' commits, every game's
start-up). Each is `move.w <a variable from A5, or a number>,<the register>`. A hook there hands the interface what the
instruction gives, from the game's variable or number, and the game's instruction then runs as it was. `game.cfg`
lists them:

```
video_palette_pages = 0009D0 000BB4      where the game sets the pages
video_palette_upload = 0009D8 000B7E     where it gives the buffer
```

At start-up the toolkit reads each instruction and takes it only if it is that move to that register. When the frame's
commit is rewritten whole (step 3 below), these hooks become part of it.

`build.mk` gives every address to the translator (`video_hooks.txt`).

The two kinds sit in eight games so far, one or two of each family's version. The other games of a family have the
same code and get the same lines once they have been run. Results, with `--video check` over attract mode (4,000
frames) and the ladder movie (6,000; Giga Wing's with `free_play,invincible`, the rest `free_play,no_damage,one_hit`),
and the ladder's frame hashes against a build without the lifts:

| Game | Lifts | Attract mode: uploads the same | The ladder: uploads the same | Frame hashes |
| --- | --- | --- | --- | --- |
| sfa2 | commit routine + start-up hooks | 3,941 | 5,939 | the same, 6,000 |
| vampire_savior | commit routine + start-up hooks | 3,941 | 5,941 | the same, 6,000 |
| dstlk | hooks (two-instruction commit) | 3,957 | 5,957 | the same, 6,000 |
| msh | hooks (constant buffer, on demand) | 1,516 | 3,060 | the same, 6,000 |
| mshvsf | hooks (variable buffer) | 3,926 | 5,926 | the same, 6,000 |
| ssf2t | hooks (variable or constant) | 3,968 | 5,968 | the same, 6,000 |
| avsp | hooks | 3,934 | (no ladder movie) | attract mode the same, 4,000 |
| gigawing | hooks | 3,734 | 5,826 | the same, 6,000 |

In every run none differed, none came only from the game's side, and none was made by code that is not native (the
start-up's uploads included). The builds without the lifts were this branch's before them (sfa2, vampire_savior,
dstlk, msh, ssf2t) and the `widescreen-stage-ends` branch's (avsp, gigawing, mshvsf), whose sfa2 gives this branch's
hashes, all 6,000 frames.

Before the hooks, the commit routine alone (sfa2, Vampire Savior): `--check-rewrites` over the ladders, 4,738 and
5,939 calls, 0 differed, the cycles exact (68 a call). The frame hashes with and without it
(`CPS2RECOMP_SKIP_REWRITES=CommitPalette`) were the same, all 4,800 and 6,000.

## Order of work

Sizes: S a session, M a few, L a week or more.

1. **Palettes**: done for the eight games above; the same lines for the other twelve CPS-2 games, each run once
   (S together); the CPS-1 games' palette control (another register, S).
2. **The object list:** the drawers' entries as `Object`s (the Alpha library's list builder and its piece writers,
   the bank swap). Check (a) against the list latched at the vertical blank. M for Alpha, M for Marvel; SF2's copy
   in the interrupt, S.
3. **Registers from shadows:** scrolls, layer control, priority masks, object offsets (sfa2's 0013CE is already a
   rewrite, `FrameRegisters`, in the game's sources; it moves to the toolkit). M.
4. **Raster bands:** Marvel (seen); the Vampire games and Giga Wing set the counters but changed nothing in these
   runs. S-M.
5. **The CPU rasteriser of `Frame` and check (b)** against `video.cpp` on every game's movies. M.
6. **The SDL_GPU renderer** drawing `Frame`, with the widescreen columns and the existing presenters and shaders
   after it. L.
7. **The native renderer by default**, `video.cpp` kept only for `--video board` and the checks. The bus waits in the
   lifts (above) come first. S.
8. **Stages between games** through two producers: Vampire Savior Definitive's guests first, then every fighting
   game's stages in every other's (a guest's facts are already written once per game). L.
