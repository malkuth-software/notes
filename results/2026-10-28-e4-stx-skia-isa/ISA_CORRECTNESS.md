# Each SIMD path against the scalar build, then its speed — 2026-10-28

Phase E, item E4: correctness first, speed second. Method and tools: gui_forms
`stx_skia/docs/ISA_PROFILES.md`. Paths below are relative to gui_forms' `stx_skia/`.

**Outcome.**
- **No defect found.** NEON, SSE2, SSE4.2 and AVX2 render the twelve-case workload within one
  or two levels per channel of the scalar build. The only larger difference has a known,
  deterministic cause: NEON and AVX2 fuse multiply-add, and that moves nearest-sampling texel
  boundaries.
- **Shipped arm64 and x86-64 differ in one visible way.** At high precision, nearest-scaled
  images differ in 560 of 64,000 pixels between NEON (fused) and SSE2 (not fused); at the
  default precision they are identical.
  - The owner accepted this on 2026-10-28: nearest-scaled drawing serves only the gallery, an
    example and tests.
  - No test or golden depends on a nearest-scaled cell boundary.
  - Removing the FMA differences is a post-1.0 item; `DIFFERENCE_TRACE.md` lists them all.
- **Speed on the M4:** NEON is 2–10× the scalar build on blended and sampled drawing, but only
  1.3–1.4× on opaque fills and clears, which run near store bandwidth. On strokes NEON is
  slower than scalar.

## Hosts and builds

- **arm64:** the M4 mini, Homebrew Clang 22.1.8, Release.
- **x86-64:** the same machine under Rosetta 2. The x86-64 objects are cross-compiled
  (`-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_SYSTEM_NAME=Darwin
  -DCMAKE_SYSTEM_PROCESSOR=x86_64`). Rosetta ran the AVX2 build: its 138 tests pass and its
  pixels carry the fused-multiply-add signature, though `hw.optional.avx2_0` reads 0 there.
  Rosetta times nothing that matters, so x86-64 speed comes from CI.
- **Profiles:** `candidate` (NEON or SSE2, as shipped), `candidate-scalar` for each
  architecture, and on x86-64 `candidate-x86-64-v2` (SSE4.2) and `candidate-x86-64-v3` (AVX2).
  Every profile passes its 138 tests.
- **The reference:** the reference profile (the pin, `c20afa3c`) and the candidate write
  identical workload files on the M4, so the workload is part of the candidate/reference
  comparison as well.

## Correctness

Each SIMD profile is compared with its own architecture's scalar profile. Pixels are BGRA,
premultiplied, 320×200 per case. "Default" is the library's choice (low precision wherever a
pipeline allows); "highp" forces the float stages.

**Largest channel difference, default precision** (pixels that differ, of 64,000, in brackets):

| Case | NEON | SSE2 | SSE4.2 | AVX2 |
|---|---|---|---|---|
| clear | 0 | 0 | 0 | 0 |
| opaque_fills | 0 | 0 | 0 | 0 |
| translucent_fills | 1 (35,508) | 1 (35,508) | 1 (35,508) | 1 (35,508) |
| rounded_rects | 1 (14,495) | 2 (15,421) | 2 (15,421) | 2 (15,421) |
| strokes | 0 | 1 (2,567) | 1 (2,567) | 1 (2,567) |
| gradients | 1 (10,266) | 1 (10,266) | 1 (10,266) | 1 (10,266) |
| image_copies | 1 (20,427) | 1 (21,169) | 1 (21,169) | 1 (21,169) |
| scaled_linear | 1 (17,462) | 1 (17,462) | 1 (17,462) | 1 (17,462) |
| scaled_nearest | 0 | 0 | 0 | 0 |
| glyph_masks | 1 (3,286) | 2 (3,842) | 2 (3,842) | 2 (3,842) |
| clipped | 1 (9,921) | 2 (10,224) | 2 (10,224) | 2 (10,224) |
| paths | 2 (696) | 2 (1,369) | 2 (1,369) | 2 (1,369) |

These are the low-precision stages' rounding against float stages: Skia's design, the same on
every SIMD path, and within two levels.

**High precision.** Every case is identical to scalar except:

| Case | NEON | SSE2 | SSE4.2 | AVX2 |
|---|---|---|---|---|
| gradients | 0 | 1 (1 pixel) | 1 (1 pixel) | 0 |
| scaled_linear | 1 (739) | 1 (235) | 1 (235) | 1 (739) |
| scaled_nearest | **255 (560)** | 0 | 0 | **255 (560)** |
| clipped | 0 | 1 (1 pixel) | 1 (1 pixel) | 0 |

**The fused-multiply-add finding.** NEON's `mad` is `vfmaq_f32` and AVX2's is
`_mm256_fmadd_ps`; SSE2, SSE4.2 and the scalar stages round the product first. The 560 pixels
are the same on NEON and AVX2, and absent on SSE2 and SSE4.2, so the cause is the fused
operation. A sample coordinate that lands exactly on a texel boundary goes to the neighbouring
texel, and with this image's red ramp wrapping at 255 the channel difference reaches 255. Low
precision computes those coordinates without the fused operation, so the default precision is
identical to scalar there. Nothing is wrong on either side; a texel boundary is a tie. It is a
cross-architecture difference GUI.Forms should know about for goldens of nearest-scaled
images.

**Between the shipped paths.** SSE2 against NEON, directly: high precision differs only in
those three cases (gradients 1 pixel, scaled_linear 525 pixels by 1, scaled_nearest 560
pixels). The low-precision stages differ by up to two levels, because NEON and SSE2 round their
8-bit blends differently.

**The other corpus files.** Twenty-two of the 59 other corpus files differ from the scalar
profile on at least one SIMD path (`tests/isa_bounds.txt` lists them). Each is traced to its
cause in `DIFFERENCE_TRACE.md`, which also holds the owner's list of every FMA-caused difference.
All of them are intended: Skia's low precision, rounding ties, SIMD approximations, NaN signs,
hand-written blit rows and fused multiply-add.

**Across architectures, scalar against scalar.** The arm64 and x86-64 scalar builds differ in ten
corpus files. With no SIMD and no contraction in either, the differences come from the platforms:
- NaN encoding;
- Apple's libm returning different last bits;
- `malloc_size` and `max_align_t`;
- one upstream wrong value: black converted to an HLG colour by the scalar colour path.

`DIFFERENCE_TRACE.md` traces each of these to the test line that records it.

## Speed on the M4

`measure_isa.py`, seven trials in rotating order, median frame time of a 1280×800 frame.
Ratios above one mean the scalar build is slower.

| Case | NEON default ms | Scalar ms | Scalar/NEON | NEON highp ms | Highp/default |
|---|---|---|---|---|---|
| clear | 0.056 | 0.078 | 1.40 | 0.053 | 0.95 |
| opaque_fills | 0.120 | 0.159 | 1.32 | 0.116 | 0.97 |
| translucent_fills | 0.781 | 7.591 | 9.71 | 1.891 | 2.42 |
| rounded_rects | 0.445 | 2.063 | 4.64 | 0.769 | 1.73 |
| strokes | 0.185 | 0.170 | **0.92** | 0.209 | 1.13 |
| gradients | 1.522 | 5.504 | 3.62 | 1.475 | 0.97 |
| image_copies | 0.664 | 6.355 | 9.57 | 1.591 | 2.40 |
| scaled_linear | 2.416 | 10.817 | 4.48 | 3.524 | 1.46 |
| scaled_nearest | 0.861 | 5.926 | 6.88 | 1.729 | 2.01 |
| glyph_masks | 5.268 | 10.912 | 2.07 | 5.741 | 1.09 |
| clipped | 1.765 | 6.282 | 3.56 | 1.740 | 0.99 |
| paths | 0.436 | 1.214 | 2.79 | 0.533 | 1.22 |

**What this says.**
- **Fills are bandwidth.** A full 1280×800 clear takes 56 µs with NEON (about 73 GB/s) and 78 µs
  scalar. SIMD gives 1.3–1.4×, which agrees with FILL_KERNEL: the kernel is near store bandwidth,
  and the lever for `rect_memset32` is pixels filled, not fill speed.
- **Low precision earns its keep** on blending and sampling (1.5–2.4× over high precision on
  NEON), but not on gradients (0.97) or clipped drawing (0.99).
- **Strokes are slower with NEON than scalar** (0.92, outside the 6% noise floor). The case is
  hairlines and thin antialiased strokes: many one- and two-pixel spans, where pipeline set-up
  costs more than the vector work saves. It is the same short-span weakness FILL_KERNEL found in
  narrow rows, and the first concrete lead for a kernel change.
- **Glyph masks** cost about 0.84 µs per 12×16 glyph, mostly per-draw set-up. GUI.Forms
  composites its text coverage itself, so this is not its text path.

x86-64 timings, and the same table for SSE2 against AVX2, come from CI: the stx_skia profiles
job runs `tools/run_recorded_workloads.py` on Linux x64, Linux arm64 and Windows x64 and uploads
the results.

## Reproducing

```sh
cd stx_skia
sh tools/build_native.sh candidate
sh tools/build_native.sh candidate-scalar
python3 -B tools/compare_isa.py build/candidate-scalar build/candidate
python3 -B tools/measure_isa.py build/candidate build/candidate-scalar
```

On arm64 macOS, the x86-64 profiles build with `STX_SKIA_CMAKE_ARGS="-DCMAKE_OSX_ARCHITECTURES=x86_64
-DCMAKE_SYSTEM_NAME=Darwin -DCMAKE_SYSTEM_PROCESSOR=x86_64"` and a separate `STX_SKIA_BUILD_ROOT`.
