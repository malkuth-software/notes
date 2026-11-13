# Tracer bakes playground

Decision 8 of `TRACER_THEME_BAKING_STUDY.md` (coordinator desk): measure and look at tracer-style theme bakes
before any engine work. Four control-sized scenes are each lit by three methods. The output is linear light
planes plus the time per bake at 1, 4 and all threads, and a local page shows the results side by side.
Prototype style follows the theme lab brief ("Prototyping effects"): one C++20 standard-library generator and
one HTML page, with no framework, no network and nothing outside this folder.

## Build and run (integrator, on the M4)

From this folder:

```sh
mkdir -p build/tracer_bakes
c++ -std=c++20 -O2 -ffp-contract=off -pthread -o build/tracer_bakes_gen tracer_bakes.cpp
build/tracer_bakes_gen build/tracer_bakes 160
```

The second argument is the chart side in texels: even, 32 to 512, default 160. The run prints one line per
scene and method, with median milliseconds at 1 thread, 4 threads and all threads, and either `identical` or
`DIFFERENT ACROSS THREAD COUNTS`. The exit status is 0 when every bake is byte-identical across thread
counts, 3 when one is not, 1 on a write failure, and 2 on bad arguments.

Keep the M4 otherwise idle while it runs: the timings are the point. The planes do not depend on timing.

Output in `build/tracer_bakes/`:

- `<scene>_albedo.f32`: int32 width, height, 3, then RGB float32, interleaved.
- `<scene>_<method>.f32`: int32 width, height, 2, then the fixed-light plane and the lamp plane, float32
  (the same layout family as GUI.Forms light-plane bakes).
- `planes.js`: every plane as base64, plus the timings, for the page.
- `timings.json`: the timings alone.

At 160 texels, `planes.js` is about 5 MB.

## Viewing (owner, locally)

Copy `build/tracer_bakes/` from the M4 into this folder's `build/`. Then open `tracer_bakes.html` directly as
a local file. It loads `build/tracer_bakes/planes.js` with a script tag, so it works from `file://` without
a server.

The page has these controls:

- the fixed light's colour and power;
- the lamp's colour and power;
- exposure;
- a view selector: composite, the fixed-light plane alone, the lamp plane alone, or the difference from the
  window method.

All of them act at composite time:

  rgb = albedo × (fixed colour × power × F + lamp colour × power × L), then 1 − e^(−1.3·v)

so nothing is re-baked when they change.

## Scenes

Each scene is a height field over a unit chart, seen from the front. The one fixed light comes from the
upper left. It is a disc of angular radius 0.06 rad at unit irradiance, with sky ambient 0.22.

| Scene | Contents | Lamp |
|---|---|---|
| `keycap` | Ivory keycap with a dished top and rounded bevel, in a dark 0.12-deep well | Inset lens on the cap |
| `lamp` | Domed lens in a seat, ringed by a raised bezel | Sphere filling the dome |
| `contact` | Bar lying on the plate (contact crease), plus a 0.012-high strip for the 1–2 px contact-shadow case | Slot lamp at the bar's end |
| `well` | Deep (0.30) white-enamel well | Lamp in the floor. The walls are lit directly; the floor around it only by light returned from the walls. |

## Methods

### `relief`: today's approach

- Lambert shading n·l.
- A drop shadow offset towards the light by the relief primitive's contact-shadow rule,
  k = max(1, ⌊min(s, 2) + 0.5⌋) px.
- Ambient from an open sky, (0.5 + 0.5 n_z).
- An SDF-style lamp glow, fading over a fixed width beyond the lamp, with no occlusion.
- No bounce.

This stands in for the relief primitives and the SDF lamp falloff in `lamp_keys.cpp`. The full lamp baker
also models lens interiors, which this prototype does not; see "Limits".

### `teapot`: conv-teapot's light

Re-derived from the paper and `render.hpp` (`bake_open`, `gather_bounce`):

- **Point visibility** of each source's centre: hard shadows.
- **Open fraction** on texels with 4× the area of the fine ones. Each casts 32 cosine-weighted directions on
  a Fibonacci spiral, with r = √((k + 0.5)/32), angle = 2.39996·k plus a per-texel hash rotation.
- **Quarter-first:** every fourth direction is cast first. If those eight agree, they stand for all 32.
- **Transfer gather:** six Jacobi passes of

    B = B0 + (1/32) Σ_blocked w·ρ_land·B_land

  then a 3×3 smooth and a bilinear upsample. ρ is the luminance of the landing texel's albedo.

The bounce is grey, so tinting it by the receiver's albedo is the approximation discussed in the study (§6).

### `window`: bfft source windows

As `teapot`, except that each source is integrated exactly over its visible part:

1. The source (the fixed light's disc, or the lamp sphere seen as a disc of angular radius asin(r/d)) is cut
   into columns by lateral angle.
2. The horizon along each column's azimuth decides exactly how much of that column is visible.
3. Where the class of a column changes (blocked, partial or full), the boundary is located by bisection.
4. Fully visible runs are integrated in closed form, and only partial runs by 4-point Gauss–Legendre.

This keeps bfft's rule that quadrature measures inside intervals but never decides where a shadow starts.
It is a new height-field adaptation of that idea, not bfft's polygon and sphere window code.

## Parameters a theme author would set

- **Geometry:** relief shape, well depth, bezel, lens.
- **Lamp:** position and size.
- **Bake strategy:** relief, occlusion, bounce, windows.
- **Bounce passes:** six here.

Colour and power are composite-time values and never trigger a bake.

## Cost and properties

Per texel:

| Method | Work per texel |
|---|---|
| `relief` | One shading evaluation |
| `teapot` | One march per source, plus 8–32 marches per coarse texel and 6 gathers of at most 32 landings |
| `window` | As `teapot`, but each source costs 9 horizon marches plus 10 per class change |

The generator measures the whole cost. Scene construction (heights and normals) is not timed.

- **Baked once:** both planes are baked once per geometry and lamp layout, and reused for every colour and
  power.
- **No seams:** the bake always covers the whole chart, and painting samples linear planes, so any rectangle
  redraws without seams.
- **Determinism:** there is no FMA contraction, and every sum runs in a fixed order. Each thread writes only
  its own rows, and reads only data finished before its pass starts. The planes are therefore byte-identical
  for any thread count on one host, and the generator checks this on every run. Identical bytes across hosts
  are not claimed, because sin, cos, asin and atan come from libm. The engine port would use tables, as the
  finish kernels do.

## Limits

- Height fields only. There are no undercuts, no parametric surfaces and no Newton solves; a front-facing
  chart does not need them.
- Lenses are emitters with a white diffuser albedo. Refraction, the participating-child law and lens caustics
  are not modelled. Caustics come later, with the bfft beam compiler, after this and the source windows.
- The lamp is a sphere of unit radiance. The parts of the lamp sphere that sit inside the height field are
  occluded by it, as the physics says.
- At most 1/1024 of a column run is left out at each located class change. Near the zenith, columns
  degenerate, and the centre ray is used instead.

The generator is about 990 lines: three methods side by side, more than the brief's few hundred for a
single effect.

## Provenance

All code here is new. It re-derives methods from two sources, both MIT (owner, 2026-11-13):

- conv-teapot's paper and `src/render.hpp`: the open-fraction bake and the transfer gather;
- bfft `photonic_transport_field` (falseywinchnet/bfft at 6e64386): source-window partition and the
  retained-field formulation.

Nothing from the confidential AirSpy DDC method, nor anything derived from studying it, is used. In
particular, none of conv-teapot's speed-ups from that study are used (such as its correspondence-transport
acceptance rules).

The scene shapes, materials and the relief contact-shadow rule follow GUI.Forms' own relief primitives and
the theme lab's industrial-surfaces example.
