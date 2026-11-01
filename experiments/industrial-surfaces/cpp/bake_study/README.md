# Bake study: the same look for less time and memory

Subject: the dialog's two lamp keys (`cpp/lamp_keys.cpp`, family `dialog`). Each
key is baked up and down, 240 × 105 px, as linear light planes (room, each lamp
at unit power, coverage). The question: can a much cheaper bake look virtually
the same?

Machine: Apple A18 Pro laptop, 6 cores, under heavy unrelated load (load
average 7–14). Times are therefore **CPU seconds** for all four bakes (median
of three, `/usr/bin/time` user + sys). Wall times in the logs are unreliable.

## Measure

`compare.cpp` composites a bake as `demo.html` does: tone 1 − e^(−1.3 v),
over the body colour 397, sRGB 8-bit. It does this in four states: off; lit;
lit and pressed; lit and half pressed. It then reports CIEDE2000 per pixel:
the maximum, the 99.9th percentile, the mean, and the share of pixels above 1
(a just-noticeable difference) and above 2.

**The bar.** The approved reference takes 2 × 2 samples per pixel. Against a
converged bake (6 × 6, with hits snapped; `build/study/ref_g6`), it already
differs on 3.7 % (CANCEL) and 2.8 % (OK) of pixels by more than 1. "Virtually
identical" here means: **no further from the converged image than the
reference is.**

## Found: an empty-chord fault in the reference

The outside march stops anywhere within 2e-4 of the surface. The inside ray
then starts 4e-4 behind that point, and the inside march also stops within
2e-4. When the first stop falls at the far end of its tolerance, the inside
chord is empty. That is the streaking and the ragged rim in the approved bakes.
`--snap` moves each hit onto the surface (two Newton steps), and the streaks
are gone. Every later result is measured against snapped bakes. See
`build/study/approved_vs_fast_{ok,cancel}.png`: rows are approved, fast,
converged.

## Time

| bake | switches | CPU s | speed-up | > 1 ΔE00 vs converged (CANCEL / OK) |
|---|---|---|---|---|
| reference | – | 3.21 | 1× | 3.7 % / 2.8 % |
| snapped | `--snap` | 3.34 | 1× | same |
| **fast** | `--snap --convex --bbox --chord 0.02 --analytic --poly` | **1.00** | **3.2×** | 3.7 % / 3.1 % |
| fast + adaptive | + `--adaptive 0.5 --turn 0.999` | 0.89 | 3.6× | 3.7 % / 3.2 % |

The switches, each checked on its own against the snapped reference:

- `--convex`: a ray leaving the lens sees the room at once. The lens is
  convex and alone in a control-only scene. Exact.
- `--bbox`: camera rays start at the lens's bounding box, and rays that miss it
  stop. Worst ΔE00 2.4 on 0.2 % of pixels.
- `--poly` (from the distance field's structure): the smooth lens lies inside
  the polyhedron of its planes, because a smooth maximum is never below the
  maximum. A ray's crossing of that polyhedron is found exactly. The inside
  march starts 0.02 before its exit, which removes most of the slab-crawling
  steps. Worst ΔE00 0.7.
- `--analytic`: the normal is the distance field's exact gradient through the
  smooth maxima, one evaluation instead of six. Worst ΔE00 0.7.
- `--chord 0.02`: a larger step through the smooth medium. Worst ΔE00 0.9.
- `--adaptive T --turn C`: one sample per pixel. A pixel gets the full 2 × 2
  where its neighbours' hits differ, the legend is near, its neighbours' light
  differs by more than T, or the form's normal turns (cosine below C). Without
  the turn test, a highlight thinner than a pixel on the rounded front edge is
  missed.

### Second round: the photonic engine's own structure

The engine is a light-transport field solver. Light lives on a small set of
directed source regions; source integrals are exact over intervals cut by
occluder silhouettes, with Gaussian quadrature only inside them; inter-surface
bounces are solved once as a residual Neumann march with an energy-budget
frontier; and the camera only finds its first hit and evaluates a compiled
program of that surface's incoming regions. It does not replay paths per
pixel. (CONV is only an optional camera reconstruction; the first round's
interval port measured that peripheral piece.)

**Retaining the light inside the key** (the engine's central move): measured
first, then not built. A camera sample takes **0.81 legs** inside the keys on
average (about one per covered sample). CANCEL's white plastic stops the light
within one chord, and OK's rays end on the frosted back. There is no path
replay for a retained field to remove.

What a pixel does cost is geometry (the first hit, the exit, normals) and the
chord's source integral. Two of the engine's ideas fit that work:

- `--flat`: exact geometry where it is flat. Where one plane leads every other
  by 2.5 × the rounding, no smooth maximum in the chain adds anything, so the
  smooth surface is exactly that plane. The polyhedron's crossing there is the
  exact hit or exit, taken with no march and no snap. Rounded regions keep the
  march.
- `--gauss N`: exact transmittance, quadrature only on the source, as the
  engine integrates inside exactly bounded intervals. The medium's extinction
  is uniform here (hidden coil, no divider), so with u = 1 − e^(−σt),
  ∫ e^(−σt) S dt = (1/σ) ∫ S du, and N-point Gauss–Legendre takes the smooth
  source alone.
- `--fast-relief` (angle-addition recurrences for the grain's 14 waves): no
  gain; cosine is cheap on this core.

CPU seconds, re-measured together (the machine's load differed from the first
round's table):

| bake | CPU s | speed-up | > 1 ΔE00 vs converged (CANCEL / OK) |
|---|---|---|---|
| reference (unsnapped) | 2.49 | 1× | 17.0 % / 13.8 % (empty-chord streaks) |
| fast | 0.86 | 2.9× | 3.7 % / 3.1 % |
| fast + `--flat` | 0.74 | 3.4× | 3.7 % / 3.1 % |
| **fast + `--flat --gauss 4`** | **0.56** | **4.4×** | 3.9 % / 3.1 % |
| + `--adaptive 0.5 --turn 0.999` | 0.57 | 4.4× | 3.9 % / 3.2 % |

The snapped reference (the honest bar) scores 3.7 % / 2.8 %, so the 4.4× bake
is at the reference's quality.

## The painted body (glint kernel)

`body_study.cpp` paints demo.html's body: 1000 × 620 at scale 2, ГОСТ 397
below the title strip with film 0.55 and specular 1.6. It paints it three
ways and compares bytes with the C++ port of the JavaScript kernel
(`paint_glints`, which matched the JavaScript byte for byte).

On the speck path, the kernel draws every pixel's density from about six
jittered speck points in reach. Each point gives a Poisson facet count in the
four half-vector bins around the pixel's half-vector. A point's count in a bin,
its hue and its jitter do not depend on the pixel asking, but the original
recomputes them for every pixel in reach, about nine times each.
`paint_glints_retained` (in `surface_kernels.cpp`) paints in 32 × 32 tiles.
Within a tile it computes each (point, bin) once on first use, each cell's
jitter once, and each bin's λ and e^(−λ) once, in caller-supplied
`GlintScratch`. Pixels combine them in the original order.

| body | wall ms | speed-up | pixels differing |
|---|---|---|---|
| `paint_glints`, one thread | 1540 | 1× | – |
| `paint_glints_retained`, one thread | 511 | 3.0× | 0 |
| `paint_glints_retained`, 6 threads (bands of rows) | 142 | 10.9× | 0 |

The output is byte-identical: all 620,000 pixels match. The JavaScript
version of the same retention (`glint_fast.js`) is byte-identical too.
(`tools/body_study.mjs`'s Node timings run inside a `vm` sandbox, about 40×
slower than the browser, and are not comparable.) The demo now loads the C++
body's bytes (`build/demo_body.rgba`) instead of painting it in JavaScript on
every load. In GUI.Forms the retained kernel paints straight into the
framebuffer, damage rectangle by damage rectangle.

## Memory

`pack.cpp` stores a bake compactly and decodes it into the standard format, so
`compare` sees exactly what a runtime would. Figures are for both keys, up and
down; the original is 3.43 MB of float32.

| store | bytes | of original | worst ΔE00 | 99.9th |
|---|---|---|---|---|
| one grey channel, lamps merged | 1.21 MB | 35 % | 0 | 0 |
| + covered pixels only | 0.94 MB | 27 % | 0 | 0 |
| + binary16 | 0.47 MB | 14 % | 0.88 | 0.50 |
| + 8-bit log (instead of binary16) | 0.24 MB | 7 % | 1.25 | 0.69 |

The planes are grey to float precision because the materials are untinted and
the lamps are baked white; the colour is applied at composite. CANCEL's two
lamps always share a colour and power in the dialog, so storing their sum is
exact.

## Negative results (kept)

- **Light planes at half resolution** (bilinear): worst ΔE00 45. The legend and
  the edges live in the light.
- **Looser march tolerance** (`--eps 1e-3`): broken image, and slower.
- **Down state as a warp of the up state** (a rigid press of 0.04 seen nearly
  from above): 4 % of pixels > 1, worst 35, at the sides, where foreshortening
  changes.
- **Split bake, with the transmitted light at reduced resolution**
  (`--split --t-scale N`): the surface (reflection, ink, coverage) is baked at
  full resolution, the light from inside reduced, and the two joined at
  runtime. At full resolution the split is faithful: it matches the unsplit
  bake. Reduced, it fails, for two reasons. On OK the top's grain refracts the
  lamp's light at pixel scale. On both keys the rims put sharp features in the
  transmitted light.
- **The grain to first order** (`--t-linear`: the light through a flat top,
  plus its change per unit tilt in x and y, times the grain's own slope). Exact
  enough on CANCEL. On OK, 4.2 % of pixels > 1 against the reference's 2.8 %,
  with outliers to 61 where the grain tips rays across total internal
  reflection.

## From the photonic engine

Read in `bfft/experiments/photonic_transport_field`: `README.md` and
`CHILD_VOLUME_RESPONSE.md`; `render_adaptive` in
`native/regime_scene_native.cpp`.

- **Child-volume response.** The engine caches a lens's unit response per
  boundary fiber and mode. Power is not in the key, so intensity changes cost
  nothing. Our per-lamp unit-power planes are the same idea; that is why
  blinking, fading and colour are free at composite.
- **Certified scanline intervals** (`render_adaptive`), ported as
  `--intervals`. A visible run is one interval: five exact controls are
  interpolated with bfft's CONV (`conv_native.c`, linked unchanged), and twelve
  exact witnesses must carry the same optical path signature (the faces each
  leg crosses, and total internal reflection) and the same 8-bit display
  values. Otherwise the interval is split. On the keys' smooth transport (flat
  top, no legend) it evaluated 48.6 % of pixels exactly and interpolated the
  rest: about a 2× cut in transport evaluations. It is not more, because rims,
  chamfer reflections and the lamp's edges keep breaking intervals. Net CPU
  0.85 s against 0.91 s without it. The grain cannot pass a witness, so this
  route needs the grain handled apart, which the first-order model does not do
  well enough on clear acrylic.
- Not yet tried: the engine's analytic visible-edge field, which takes coverage
  from projected contours instead of supersampling the edges.

## Run

```sh
clang++ -O2 -std=c++20 cpp/bake_study/compare.cpp -o build/bake_compare
clang++ -O2 -std=c++20 cpp/bake_study/pack.cpp -o build/bake_pack
clang -O3 -std=c11 -pthread -c bfft/standalone_conv_resize_demo/native/conv_native.c -o build/conv_native.o
clang++ -O3 -std=c++20 -pthread cpp/bake_study/lean_keys.cpp build/conv_native.o -o build/lean_keys
build/lean_keys build/study/ref_g6 240 dialog --snap --grid 6         # the converged bake
REF=build/study/ref_g6 sh tools/bake_study.sh fast --snap --convex --bbox --chord 0.02 --analytic --poly
```
