# Every SIMD and cross-architecture difference, traced — 2026-10-28

E4 tracing. `ISA_CORRECTNESS.md` found 22 corpus files that differ between a SIMD path and the
scalar build, and 10 that differ between the arm64 and x86-64 scalar builds. This note traces
each one to its cause and says whether it is intended. Paths are relative to gui_forms'
`stx_skia/`.

**Outcome.**
- **No stx_skia defect.** Every difference comes from one of ten mechanisms (below). All of
  them are Skia's own design or the platform's, and the reference pin behaves the same.
- **Fused multiply-add is a cause; contraction never is.** NEON and AVX2 fuse the raster
  pipeline's `mad`. Building the shipped profiles with `-ffp-contract=off` changed no byte of any
  corpus, on either architecture.
- **One upstream wrong value found and corrected.** Converting black to an HLG Rec. 2020 colour
  through the scalar colour-space path gave 16.5 on arm64 and −15.2 on x86-64 instead of 0.
  GUI.Forms never uses an HLG destination. The coordinator chose to correct it (option 2 below).
  Since then, black stays black and both architectures agree.

## How the causes were separated

Each step is a temporary build on the M4. The sources were patched in the M4's copy only and
restored afterwards, and none of these builds is committed.

| Build | What changes | What the comparison isolates |
|---|---|---|
| shipped profiles with `-ffp-contract=off` | the compiler may not contract `a*b+c` | contraction: no byte changed anywhere |
| NEON and AVX2 with unfused `mad`/`nmad` (`vfmaq_f32`, `vfmsq_f32`, `_mm256_fmadd_ps`, `_mm256_fnmadd_ps` replaced by a multiply and an add) | fused multiply-add in the raster pipeline | **fused** vs **unfused**: everything FMA causes |
| the unfused builds with high precision forced (`gForceHighPrecisionRasterPipeline = true`), plus SSE2 with high precision forced | the 16-bit low-precision stages | **default** vs **forced high precision**: everything low precision causes |
| scalar with the hand-written SSE code restored, then with the raster pipeline's SSE2 stages, then with `skvx` SIMD | one SIMD layer at a time | which layer a difference lives in |
| SSE2 with `-fno-vectorize -fno-slp-vectorize` | compiler auto-vectorization | auto-vectorization: only `matrix-transforms`, NaN signs |

What is left after FMA and low precision are removed is the residual. Two tests set the
precision flag themselves (`raster_pipeline_corpus.cpp` and the workload), so their
low-precision halves stay in the residual.

The corpus helper `tests/differential_corpus.h` was instrumented the same way to log which test
source line appended each byte. Eight corpora use it, and each of their differences was mapped to
the line that wrote it. A probe compared Apple's libm on arm64 and x86-64 over 100,000 inputs per
function.

## The ten mechanisms

1. **Fused multiply-add.** NEON's `mad` is `vfmaq_f32` and AVX2's is `_mm256_fmadd_ps`. SSE2,
   SSE4.2 and scalar round the product first. Only high-precision float stages use it: matrix
   and coordinate stages, filter weights, gradients, transfer functions. Low-precision stages do
   not, so the default rendering of the workload is unaffected.
2. **Low precision.** The SIMD paths run 8- and 16-bit stages where a pipeline allows. The scalar
   build has none (Skia does not build them for scalar), so it always runs float stages. This
   is Skia's speed-for-precision trade.
   - The difference is usually one or two levels.
   - Low precision ignores the decal tile's inclusive edge. Upstream's high-precision `decal_x`
     uses `(0 < x < w) | x == edge`, while low precision uses `0 <= x < w`. A nearest sample
     exactly on the edge of a decal image is therefore inside on one and outside on the other.
     This explains the large `image-sampling` differences: up to 159 in a channel, all on
     decal-tiled cases.
3. **Rounding ties.**
   - Scalar `round(v)` is `uint32_t(v + 0.5f)`, which rounds halves up.
   - NEON (`vcvtnq_u32_f32`) and SSE/AVX (`_mm_cvtps_epi32`) round halves to even.
   - A value exactly halfway between two levels therefore differs by one. This is common to
     every SIMD path and is upstream code.
4. **SIMD approximations.** On every SIMD path, `rcp_precise` is a hardware estimate refined by
   one Newton step; scalar divides exactly. NEON, SSE4.1+ and AVX2 use the bare estimate for
   `rcp_fast`. Stages that divide (unpremultiply, divide-based blend modes) differ in the low bits
   as a result. This is attributed by elimination and code reading, not by a separate build.
5. **Hardware half-float conversion.**
   - NEON (`vcvt_f16_f32`) and AVX2 (`_mm256_cvtps_ph`) round to nearest.
   - The software `to_half` used by SSE2 and scalar truncates.
   - F16 stores therefore differ by one F16 unit. NEON and AVX2 differ from scalar in the same
     164,560 bytes of `conversions` once FMA is removed.
6. **NaN encoding.**
   - x86's default NaN is negative (`0xffc00000`, `0xfff8…`); ARM's is positive (`0x7fc00000`,
     `0x7ff8…`).
   - Vector and scalar code also pick different operands when a NaN propagates.
   - Only the sign bit differs, which is the byte difference of 128 in the tables below.
   - No test reads a NaN's sign.
7. **Hand-written blit rows.** `SkBlitRow`/`SkBlitMask` have SSE2 and NEON versions of the
   legacy 32-bit blitters. Restoring only the SSE code to the scalar build reproduces 96 bytes of
   `blitters`.
8. **Platform libm.** Apple's libm returns different last bits on arm64 and x86-64 (probe,
   100,000 inputs each):
   - `sin` differs in 19,023 results (at most 2 ulp), `cos` 17,588, `atan2` 4,624 (at most 3
     ulp), `acos` 640, `exp` 266, `pow` 129, `cbrt` 51;
   - `sinf` 1,330, `cosf` 1,278, `powf` 595 (1 float ulp each);
   - `log` and `atan2f` 0.

   Geometry that calls these functions then differs. Near a tie, a root count or subdivision
   can change.
9. **Allocator and alignment.** `malloc_size` returns different usable sizes on the two
   platforms. `alignof(max_align_t)` is 16 on x86-64 macOS and 8 on arm64.
10. **An upstream wrong value through a NaN.** See `color-plans` below.

## The fused-multiply-add list

Owner, 2026-10-28: fixing cross-architecture FMA differences is a post-1.0 item. This is the
complete list it starts from. Each entry gives the path, what differs, and the size:
- **NEON** is the unfused NEON build against the shipped (fused) NEON build.
- **AVX2** is the same comparison on x86-64.
- **Pixels** are counts out of 64,000 in the workload.
- **Bytes** are differing corpus bytes. A corpus that hashes its pixels (`blitters`,
  `canvas-device`) counts 8 bytes per case whose pixels changed.

The shipped x86-64 build is SSE2 (no runtime AVX2 dispatch: stx_skia has no Haswell stage
table, and skcms is built with `SKCMS_DISABLE_HSW`), and SSE2 never fuses. So every entry is also
a difference between shipped arm64 and shipped x86-64. An x86-64 build at x86-64-v3 (AVX2) would
match NEON on all of them.

Contraction is the cause of nothing: `-ffp-contract=off` builds of every shipped profile are
byte-identical to the profiles themselves.

| # | Path | What differs | NEON | AVX2 |
|---|---|---|---|---|
| 1 | **Nearest-scaled image drawing, high precision** (workload `scaled_nearest`, highp): the sample coordinate `mad` lands on a texel boundary, and nearest sampling takes the neighbour texel | pixels, up to 255 in a channel | 560 px | 560 px |
| 2 | Bilinear-scaled image drawing, high precision (workload `scaled_linear`, highp): filter weights | pixels, 1 level | 525 px | 525 px |
| 3 | Gradient, high precision (workload `gradients`, highp) | pixels, 1 level | 1 px | 1 px |
| 4 | Drawing under an antialiased clip, high precision (workload `clipped`, highp) | pixels, 1 level | 1 px | 1 px |
| 5 | Image sampling, all filters and tile modes, high-precision cases (`image-sampling`) | pixels, up to 141 (texel choice) | 18,058 B | 18,061 B |
| 6 | Image sampling, corrected mip cases (`image-sampling-corrected`) | pixels, 1 level | 744 B | 747 B |
| 7 | Image subset drawing (`image-subsets`) | pixels, up to 2 | 562 B | 562 B |
| 8 | Gradient geometry and stops (`gradient-geometry`) | pixels, 1 level | 2,550 B | 2,550 B |
| 9 | Patch (Coons) drawing (`patch-utils`) | pixels, 1 level | 32 B | 32 B |
| 10 | Shader coordinate mapping (`shader-coordinates`) | pixels, 1 level | 113 B | 113 B |
| 11 | Shader blending (`shader-blends`) | pixels, 1 level | 26 B | 26 B |
| 12 | Mixed scenes (`render`) | pixels, 1 level | 1,964 B | 1,976 B |
| 13 | Canvas drawing (`canvas-device`, hashed) | pixel hashes | 940 B (about 118 cases) | 972 B (about 122 cases) |
| 14 | Blitters (`blitters`, hashed) | one case's pixel hash | 8 B | 8 B |
| 15 | Raster-pipeline programs (`raster-pipeline`): conversion and float-store cases | pixels and floats, up to 64 per byte | 817 B | 826 B |
| 16 | Pixel conversion between formats (`conversions`) | pixels, up to 208 per byte in packed formats | 1,139 B | 1,139 B |
| 17 | Colour-space conversion, raster path (`color-plans`): transfer functions and gamut matrices | floats, last bits (e.g. 0.8919754 vs 0.8919792) | 4,506 B | 4,498 B |
| 18 | Shader colour working spaces (`shader-working-space`) | floats, last bits | 119 B | 119 B |
| 19 | Colour-filter composition (`color-composition`) | floats, last bits | 243 B | 243 B |
| 20 | Colour matrices and tables (`color-adjustments`) | floats, last bits | 154 B | 154 B |

In the workload's **default** precision, FMA changes no pixel. The default precision is
low precision wherever a pipeline allows, which is the common 8-bit sRGB case. So entries 1–4
appear only where a pipeline runs at high precision: a colour-space transform, perspective, a
float format, or a stage with no low-precision form.

## Known and accepted: nearest-scaled images (entry 1)

Owner decision, 2026-10-28: the arm64/x86-64 difference in nearest-scaled drawing is **known and
accepted**.
- **Users.** Nearest-scaled drawing is used only by GUI.Forms' gallery, the `paint_contract`
  example and tests. No product uses `RasterCanvas` or `InterpolationMode::nearest`.
- **Revisit trigger.** It is revisited if a product builds a pixel editor on `RasterCanvas`.
- **The fix itself** is the post-1.0 FMA item above.

**No test or golden depends on a nearest-scaled cell boundary.** Every use of nearest sampling in
gui_forms was checked:
- `tests/canvas_raster_tests.cpp` and `tests/windows_canvas_raster_tests.cpp`:
  - They compare `RasterCanvas`'s tiled painting with one direct draw, both made on the same
    machine. Nothing is compared against a stored image.
  - Tolerance: one level per channel.
  - They sample nearest at zooms 1/64, 0.1, 0.25, 1, 1.25 and 3.1, at scales 1, 1.5 and 2, from
    view origin (470.3, 480.7).
  - Computed exactly, the closest any pixel centre comes to a texel boundary is 0.0011 texel
    (zoom 3.1, scale 1.5, near x = 476). That is about 35 single-precision ulps there.
  - Every other configuration keeps at least 0.0065 texel.
  - Neither path can move a sample across a boundary, so no change was needed.
- `tests/drawing_skia_tests.cpp`: draws a 2×2 image into 4×4 pixels with nearest sampling, an
  integer zoom of 2. Its samples fall at a quarter of a texel from each boundary.
- `tests/canvas_tests.cpp`: zoom 2 (integer), and checks only the recorded sampling mode.
- `examples/paint_contract`: zooms 2 and 4 (integer). Its checks are damage rectangles, encoded
  bytes and coordinate mapping, not rendered pixels.
- `tests/gallery_interaction_tests.cpp`: counts the gallery's image resources and bytes; it
  checks no rendered pixel.
- Golden files: none in gui_forms holds a nearest-scaled image.

## The 22 files that differ between SIMD and scalar

The FMA and low-precision columns give the bytes those mechanisms cause. Each pair is NEON then
SSE2 for low precision, and NEON then AVX2 for FMA. The residual is what is left against scalar
with both removed.

| File | Records | FMA (NEON/AVX2) | Low precision (NEON/SSE2) | Residual (NEON/SSE2) and its cause | Intended |
|---|---|---|---|---|---|
| gradient-geometry | gradient pixels | 2,550/2,550 | 689,300/689,316 | 448,360/448,249, ≤1: rounding ties (3) | yes, Skia |
| matrix-transforms | matrix floats | 0 | 0 | 61/47, NaN signs only (6). On x86-64 these 47 bytes come from compiler auto-vectorization (they vanish with `-fno-vectorize`). Line `append_matrix` | yes (NaN payload is not meaningful) |
| curve-geometry | curve floats | 0 | 0 | AVX2 only: 164, NaN signs of doubles in the cubic solver corpus (lines 592, 594) | yes |
| patch-utils | patch pixels | 32/32 | 0 | 138/138, ≤1: rounding ties | yes |
| shader-coordinates | pixels | 113/113 | 2/2 | 4/4, ≤1: rounding ties | yes |
| shader-working-space | floats | 119/119 | 0 | 0 | FMA list |
| color-plans | conversion floats | 4,506/4,498 | 0 | 257/532, all in the blend colour-filter section: SIMD reciprocal (4) in divide-based blend modes | yes |
| color-composition | floats | 243/243 | 0 | 0 | FMA list |
| color-adjustments | floats | 154/154 | 0 | 0 | FMA list |
| raster-pipeline | programs' pixels and floats | 817/826 | in the residual: the test forces both precisions itself | 11,851/337. (a) Its own low-precision conversion half: NEON's low-precision integer rounding differs from scalar more than SSE2's does (2). (b) The `extend` case: low-precision `srcover_rgba_8888` leaves registers in 0–255 and the following `store_8888` writes them as they are, while high precision clamps them to 255. That makes 96 pixels differ on every SIMD path; the test appends a store after a blit on purpose (2). (c) 8–9 bytes of float stores: SIMD approximations (4) | yes |
| blitters | pixel hashes | 8/8 | 12,938/14,114 | 8,927/1,214: hand-written NEON/SSE2 blit rows (7), plus rounding ties | yes |
| canvas-device | pixel hashes | 940/972 | 11,493/13,144 | 4,607/1,895: blit rows (7), rounding ties, NaN signs | yes |
| android-clip | pixel hashes | 0 | 8/8 | 0 | yes |
| image-subsets | pixels | 562/562 | 17,090/67,097 | 265/265, ≤1: rounding ties | yes |
| image-sampling | pixels | 18,058/18,061 | 25,336/76,656 (up to 159: the decal edge, 2) | 1,761/1,765, ≤1: rounding ties | yes |
| image-sampling-corrected | pixels | 744/747 | 0 | 0/3, ≤1 | yes |
| shader-blends | pixels | 26/26 | 521,876/811,857 | 116/1,349, ≤1: rounding ties | yes |
| layers | pixels | 0 | 174,251/543,943 | 53,836/52, ≤1. NEON: SIMD reciprocal (4); SSE2: ties | yes |
| render | pixels | 1,964/1,976 | 205,784/258,853 | 1,606/1,606, ≤1: rounding ties | yes |
| conversions | pixels in every format | 1,139/1,139 | 4,204/4,204 | 160,356/8,425. NEON: hardware F16 rounding (5); AVX2 shows the same 164,560 bytes. Both: rounding ties in 10-bit packed formats, where one level moves a byte by 4 or 16 | yes |
| pixmap | scaled and copied pixels | 0 | 0 | 942/942: ties in 10-bit packed formats (the bytes differ by 4 or 16), plus 22 bytes of NaN signs | yes |
| mask-filter | blurred masks | 0 | 2,063/2,063 | 1,038/1,038, ≤1: rounding ties | yes |

**The workload's own residual.** Once FMA is removed, NEON and SSE2 high precision differ from
scalar only in:
- `scaled_linear`: 235 pixels by one level (ties in the filter's 8-bit store);
- `gradients` and `clipped`: one pixel each.

Default precision is low precision against float stages throughout: at most two levels.

## The 10 files that differ between arm64 and x86-64 scalar builds

Neither build has SIMD, auto-vectorization or contraction. After the HLG correction,
`color-plans` and `color-plans-corrected` are identical on both architectures, which leaves nine
files. The byte counts in this note were measured before that split. Source lines are from the
instrumented corpus helper.

| File | Bytes | Cause | Where | Intended |
|---|---|---|---|---|
| numeric-core | 4 | NaN encoding (6): `0x7fc00000`/`0x7ff8…` vs `0xffc00000`/`0xfff8…` | `numeric_core.cpp` lines 81, 83 | yes |
| geometry-primitives | 600 | NaN encoding | point and rect results of non-finite inputs | yes |
| contour-measure | 278 | NaN encoding | `contour_measure.cpp` lines 109–133 | yes |
| matrix-transforms | 13,175 | 13,166 bytes NaN encoding. 9 bytes are signed zeros (+0 vs −0) in mapped rectangles (lines 77–80), inferred to be the sign of an underflowed product of a libm-derived term (8) | `append_matrix`, `append_rect` | yes |
| path-construction | 763 | 601 bytes NaN encoding. 160 bytes are 1-ulp floats from arcs and conics (`sinf`, `cosf`, `tan`; libm, 8). 12 bytes of SVG text print those floats with different digits (lines 451, 470) | lines 71–79, 102, 361–403, 451, 470 | yes |
| curve-geometry | 6.3 of 11.9 MB; sizes 11,923,596 vs 11,923,544 | NaN encoding first (line 113, offset 48,311), then 1-ulp floats (line 125). At offset 4,273,680, `SkChopCubicAtMaxCurvature` returns 3 pieces on one and 2 on the other: a libm last-bit difference in the cubic solver (`acos`, `cos`, `cbrt`) crosses a root-count tie. From there the files diverge in content and length | lines 325–343, and the solver corpus lines 448–676 | yes |
| mask-cache | 4 | one double's last bit in a Gaussian filter factor (`SkGaussFilter`, `exp`; libm, 8) | `mask_cache.cpp` line 50 | yes |
| tarray-core-capacity | 24,976 | `TArray` takes the allocator's usable size as capacity (`malloc_size`): 160 vs 148 or 240 (9) | capacity records | yes |
| block-list-core | 3.6 of 6.1 MB; sizes differ | block layout follows `alignof(max_align_t)`, 16 vs 8 (9) | lines 69–89 onward | yes |
| color-plans | 945 | **wrong value upstream** (10), now corrected: see below | scalar conversion path, HLG Rec. 2020 destinations | **no**; corrected |

**`color-plans`: black into an HLG destination.**
- **Where.** `SkColorSpaceXformSteps::apply(float*)` is the scalar path. It is used for single
  colours (`SkColor4f`), not for pixels. For an HLG destination it applies the inverse OOTF.
- **How the value goes wrong:**
  1. The OOTF factor is `std::pow(Y, 1/1.2 − 1)`. For black, `Y = 0`, so the factor is `+inf`.
  2. `0 × inf` is NaN.
  3. The HLG encoding then takes skcms' bit-trick `log2f_` of that NaN. That function
     reinterprets the float's bits as an integer.
  4. The probe shows the NaN is `0x7fc00000` on arm64 and `0xffc00000` on x86-64, so the
     "logarithm" is 16.49897 on arm64 and −15.23413 on x86-64. Black should encode to 0.
  5. A premultiplied destination multiplies by alpha 0 and stores +0 or −0.
- **Affected cases.** All 63 differing plan groups have the HLG Rec. 2020 destination, and every
  differing sample is black or has alpha 0.
- **Not affected.** The raster-pipeline path for the same plans is correct and identical on both
  architectures, because its `approx_powf` returns 0 for 0.
- **Upstream.** The code is identical in the reference pin c20afa3c.
- **Undefined behaviour.** None: the bits are copied with `memcpy`, so this is a wrong value.
- **GUI.Forms exposure:** none. It never creates an HLG colour space.

The options put to the coordinator were:
1. record it as an upstream defect and leave the code;
2. return 0 from the OOTF step for `Y = 0`;
3. report it upstream as well.

**Decision (coordinator, 2026-10-28): option 2**, under the owner's rule that matching the
reference is not correctness. There is no upstream report unless the owner asks for one.

The fix is on gui_forms `claude/stx-skia-e4`:
- The scalar OOTF now matches the raster pipeline's `ootf` stage. Zero luminance gives 0, and a
  negative luminance keeps its sign; `std::pow` alone gave NaN there too.
- HLG-destination plans are recorded in `color-plans-corrected.raw`, an intentional difference.
- The candidate checks that black stays black.
- stx_skia `docs/CONVERSION.md` has the record, under "HLG black".

## Between the shipped paths

For arm64 NEON against x86-64 SSE2, as shipped, the differences fall into four groups:
- **FMA:** the 20 entries above.
- **Low precision:** NEON and SSE2 round their 8- and 16-bit blends differently, by up to two
  levels: `rounded_rects`, `strokes`, `image_copies`, `glyph_masks`, `clipped` and `paths` in
  the workload, plus the low-precision bytes of the corpora.
- **SIMD approximations:** reciprocal and F16 conversion (4, 5).
- **The platform:** the ten files above (6, 8, 9, 10).

Removing FMA and low precision from both leaves only the approximation and platform groups. An
AVX2 x86-64 build would match NEON on every FMA entry.
