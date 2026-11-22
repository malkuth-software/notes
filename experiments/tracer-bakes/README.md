# Tracer bakes playground

This is decision 8 of `TRACER_THEME_BAKING_STUDY.md` (coordinator desk): see and measure tracer theme bakes
before any engine work. It runs the owner's own engines, not lookalikes:

- today's lamp baker, `lamp_keys`;
- conv-teapot's bake;
- bfft's photonic transport engine.

Each engine bakes four control-sized scenes. The output is linear light planes, with the time per bake at
1, 4 and all threads, and a local page shows the results side by side.

## Build and run (integrator, on the M4)

From this folder:

```sh
make            # build/bin/{tracer_bakes, teapot_bake, lamp_keys, regime_scene_native}
build/bin/tracer_bakes 160
```

`make run` does both.

`CXX` and `CC` default to `c++` and `cc`. Each engine is built with its own upstream flags (see `Makefile`).
The bfft build uses `-mcpu=native`, so build and run it on the same machine.

The driver runs every engine as its own process:

- with `TRACER_THREADS` set to 1, 4 and the hardware count;
- three times at each count;
- recording the median wall time of the whole process (scene build, bake and file writes);
- comparing each count's planes byte for byte with the single-thread planes.

It prints one line per scene and column. Engine output goes to `build/tracer_bakes/work/engines.log`. Keep the
M4 otherwise idle, because the timings are the deliverable.

The run is long. bfft is the slow one: each of its cells is two engine runs (the fixed-light pass and the lamp
pass) × 3 thread counts × 3 repeats. For a first look, run it at size 96.

Output in `build/tracer_bakes/`:

- `<scene>_<column>.f32`: int32 width and height, then the fixed-light radiance and the lamp radiance, each as
  RGB float32 with row 0 at the top. Values are radiance at unit white light, albedo included.
- `planes.js`: every cell as base64, with its timings, for the page. About 7 MB at size 160.
- `timings.json`: the timings alone.

## Viewing (owner, locally)

Copy `build/tracer_bakes/` from the M4 into this folder's `build/`. Then open `tracer_bakes.html` directly as a
local file; it needs no server.

The page has these controls, all applied at composite time, so nothing re-bakes:

- the fixed light's colour and power;
- the lamp's colour and power;
- exposure;
- normalise;
- a view selector: composite, the fixed plane alone, or the lamp plane alone.

The composite is

  rgb = colour_F × power_F × F + colour_L × power_L × L, then 1 − e^(−1.3·v)

The engines use different units and their own framings. "normalise" scales each plane by its own 99th
percentile, so that looks can be compared; turn it off to see absolute values.

## The scenes and what each engine is given

The four scenes: a keycap in a 0.12-deep well with an inset lens, a lamp lens in a bezel, a bar and a low strip
lying on a plate (contact shadows), and a deep white key well with a lamp in its floor (inter-reflection). They
are defined once as height fields in `scenes.hpp`.

The fixed light comes from the upper left, chart direction (−0.45, 0.55, 0.70). Where the coordinator's rule
applies ("adapt the scene to the engine, not the engine to the scene"), each engine receives the scenes in the
geometry it expects.

| Column | Engine | What it is given | What it cannot do here |
|---|---|---|---|
| `lamp_keys` | `engines/lamp_keys/lamp_keys.cpp`, the shipped baker's std-only source | Its own families: `keycap` for the keycap row, `indicator` for the lamp row. Its scenes are parameter sets of SDF lenses in a well, and it takes no other geometry. | The strip and open-well rows are empty, with a note. A run bakes the whole family (two styles, up and down: 4 bakes); the page shows the first style unpressed. |
| `teapot` | conv-teapot's `Renderer::bake_light`, `bake_open` and `gather_bounce` (`engines/conv_teapot/src`, unchanged) | The height field as the engine's own parametric surface: one `LinearSurface` over a lattice with one node per two texels. `teapot_bake.cpp` calls `build_mesh` and `bake_light`, then reads the maps back exactly as `shade_baked` combines them, with the scene's albedo. | No local emitter for diffuse light (it knows one sun and the room's sky), so its lamp plane is only the lens's own emission. Its sun colour, sky colours and bounce albedo (0.9) are the engine's own. |
| `bfft` | `regime_scene_native` (`engines/bfft/...`), whose area lights are integrated over exact source windows, with the retained diffuse field | The scenes in its own geometry (`tracer_scenes.hpp`): tiled rectangles (the engine lights each transport patch with one value), spheres, and rectangular area lights. The fixed light is a distant rectangle. The lamp is a visible emissive lens plus a hidden box of five area lights. | Rounded bevels and domes become boxes and spheres. Its field is facet-level, so the tiling sets how finely inter-reflection varies. |
| `playground` | Not an engine | This folder's own height-field adaptation of the source-window idea, plus a 32-direction gather (the first version of this prototype). | Kept only as a labelled extra, for comparison. |

## Changes made to the engine copies

These are the only changes, all to inputs and outputs:

- **bfft `regime_scene_native.cpp`:**
  - includes `worker_limit.hpp` and `tracer_scenes.hpp`;
  - adds a `tracer` scene mode, with `--tracer-scene` and `--tracer-pass`;
  - uses a straight-down camera for that mode;
  - writes the linear render for that mode, as it already did for `key`;
  - its five worker-pool sizes call `tracer_worker_count()` (TRACER_THREADS, else the hardware count) in place
    of `std::thread::hardware_concurrency()`.
- **`lamp_keys.cpp`:** its one worker-pool size reads TRACER_THREADS the same way.
- **conv-teapot:** no changes. Its thread count is `RenderOptions::threads`.

`PROVENANCE.md` lists every file's origin.

## What to look at

- **Contact shadows and crevices:**
  - `lamp_keys` has none outside its lens;
  - conv-teapot darkens by the open fraction;
  - bfft gets them from its field and exact light windows.
- **Lamp light on the surroundings:**
  - `lamp_keys` lights only its own lens, by design;
  - conv-teapot cannot light anything from a local lamp;
  - bfft lights the well walls and the bezel from the lamp's area lights, with exact penumbrae.
- **The well:** the floor round the lamp is lit only by the walls. That is inter-reflection, and bfft's field
  is what shows it.
- **Cost:** compare the 1-thread times against the 4-thread and all-thread times, per engine. The page shows
  whether each engine's output stays byte-identical across thread counts.

## Limits

- The four columns do not render identical geometry. Each engine is given the scene in its own terms, so the
  comparison is of looks and cost, not pixel by pixel.
- The times cover whole processes. They include scene and acceleration-structure builds, and for bfft its field
  compile. The engines also print their own internal timings to the log.
- Nothing here was compiled before it was committed. The integrator's first build is also the first test of
  the adapters and patches.

## Provenance

conv-teapot and everything in BFFT are MIT (owner, 2026-11-13). Only the teapot's and BFFT's own code is used.
Nothing comes from the confidential AirSpy DDC method or from studying it. `PROVENANCE.md` has the per-file
table.
