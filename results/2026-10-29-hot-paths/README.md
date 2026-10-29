# Hot paths: results and resume plan (2026-10-29)

Work stopped on the owner's direction (2026-10-29). This record covers the hot-paths track from
the stx_skia re-baseline through the start of banded paint: the measurements, what landed, what
is committed locally and how far it is verified, and how to resume. It follows the Linux
upstream-Skia profile in `results/2026-10-27-e5-linux-hot-paths`.

Hosts: the M4 Mac mini (4 performance and 6 efficiency cores, macOS arm64), shared with other
workers, so its load average was usually 5-7 during timings; linux-x64 runs came from CI.
Everything below marked "M4" was built and run only there.

## Baselines on stx_skia

M4, xctrace Time Profiler at 1 ms, main 39b5b77f (checkpoint 1). Summaries in `logs/`.

| Workload | Leading self time |
| --- | --- |
| paint (5095 samples) | rect_memset32 40.69 %; composite_text_coverage 16.90 % (26.26 % inclusive); prepared_clip_coverage 9.34 %; 2-stop gradient 2.83 %; aaa_fill_path 1.12 % (4.97 %) |
| text_engine (6688) | af_glyph_hints_reload 11.56 %; gray_render_line 10.33 %; TT_Load_Simple_Glyph 4.71 %; composite_text_coverage 4.25 %; af_latin_hints_apply 3.86 % (33.04 %) |
| document_view (8362, 100 runs in one process) | validate_utf8 30.84 %; publish 8.87 % (39.82 %) |

Linux x64 on stx_skia (CI run 40772244893, 39b5b77f): rect_memset32 29.41 %,
composite_text_coverage 24.29 % (34.10 %), prepared_clip_coverage 9.96 %, validate_utf8 29.98 %.

Rectangle fill volume (`logs/fill.txt`): 99.66 % of rect_memset32 comes through
SkiaRaster::fill_rect during display replay. Opaque fill area over the paint workload is 4.91
times the damaged device area: Panel::paint_panel 65.8 % (the root panel alone 1.00 times the
damage), the Window backplane 20.4 % (entirely under the root panel), GroupBox 7.7 %, Button
2.2 %.

## Landed in checkpoint 3 (main 8cde98df, #447)

- **Profiler fix** (`tools/profile_hot_paths.py`): profile flags go in
  `CMAKE_CXX_FLAGS_RELEASE`, so the toolchain's runtime flags in `CMAKE_CXX_FLAGS_INIT` (the
  macOS 14 libc++ runtime) survive on macOS.
- **F1, hint once per glyph** (`glyph_run_raster.cpp`): each distinct glyph is loaded and hinted
  once per run, then copied, transformed, translated per subpixel phase, emboldened and
  rendered per occurrence. Bit-identical (310 hash lines over 11 faces, 4 sizes, 4 syntheses,
  2 transforms, `tools/glyph_check.*`). Repeated glyphs 2.26-2.58 to 1.31-1.34 us; text_engine
  rasterize stage 2.39-2.67 to 2.23-2.25 ms per frame (M4).
- **Opaque cover hint:** a Panel with an opaque solid material reports its cover; the Window
  skips the backplane and the planes under the deepest covering control. 69 of 69 workload
  pixel hashes identical; damage 6.58 to 6.04 ms and pages 103.27 to 93.86 ms (sums of median
  frame times), wall 5.13 to 4.71 s (M4). Gallery trace digests were re-recorded (each page
  lost only the backplane's commands).

## F2, the CJK fallback hinter: designed, not started

Glyphs that no cmap entry reaches (GSUB ligatures, Indic conjuncts, Arabic forms) get the
autofitter's fallback style, HANI, which snaps x even in light mode. Decided design (option 1):
one shared helper, run lazily before each face's first autohinted load, reads the face's
`glyph-to-script-map`; the face's own style is the most common value among cmap-reached glyphs
other than the fallback value; where the two differ, fallback entries are rewritten to it. CJK
faces (own style is the fallback) and symbol faces whose cmap glyphs are mostly fallback-styled
(Noto Emoji) are left alone. Only values read back from the map are used. Sites:
`glyph_run_raster.cpp`, `prepared_raster.cpp`, `text_mask_fonts.cpp`/`text_mask_raster.cpp`,
`paint_probe.cpp`; `harfbuzz_font_engine.cpp` and `skia_executor.cpp` never autohint. The
acceptance condition: list every golden that moves and why (Latin ligatures, Indic, Arabic).
No branch exists.

## B1, banded paint: design approved

Design note (2026-10-28), from a prototype (`tools/band_proto.cpp`, `tools/band_check.sh`): the
frame is recorded, then replayed in device-row bands on the pool, each band with its own
SkiaRaster.

- **Speed (M4 under load, sums of medians):** pages at scale 2, serial 70.83 ms (record 2.77,
  replay 67.53) to 29.03 ms at 4 threads and 4 bands (2.44x), 25.80 ms at 6 threads and 12
  bands (2.74x). Scale 1: 24.20 to 12.20 ms (1.98x). 10 % damage: no gain at scale 1, 1.35x at
  scale 2, 1.87x at scale 3. `logs/band-summary.txt`.
- **Contention:** at most 3 of about 5,800 samples per worker wait on a Skia lock (SkPath
  semaphores); none on SkResourceCache, since Gallery shadows use the retained shadow memo.
  `logs/band-locks*.txt`.
- **Pixels:** text, rectangles, images, groups and shadows are identical banded; anti-aliased
  paths differ within a few rows of band edges. Cause: the analytic fill built edges against the
  clip and started its walk at the clip's top (`logs/band-bisect.txt`).
- **Defect found:** a partial repaint of an unchanged frame changes the same pixels along the
  damage edge today (tabs-and-pages scale 1: 12 pixels; dock-and-anchor scale 2: 12).

Owner decisions (2026-10-28): D1 a C++ `AtomicThreadPool::Batch` over the C pool's begin and
wait, the caller replaying one band; D2 make stx_skia's analytic fill clip-invariant; D3 fix the
damage-edge seam regardless. Plan: Batch with tests, a `detail::BandedReplayTarget` that
SkiaRaster implements behind window_presentation's single replay call, device-row bands over
the damage bounds, a serial memo pre-pass (its cost measured), per-band canvases, and a
ThreadSanitizer run. Banded output must equal serial output on every case. Not started.

## The analytic-fill fix: committed, verified on the M4, review pending

Branch `claude/aaa-clip-invariant` (gui_forms), head e23cadd1, on 8cde98df.

- **Change** (`stx_skia/skia/src/core/SkScan_AAAPath.cpp`): a non-inverse fill walks the path's
  own rows with the clip's columns; the run-based accumulators drop rows outside the clip; a
  row-only clip blitter cuts the rows the walkers blit directly and forwards each call in the
  same kind. A rectangle clip blitter is not enough: it turns `blitAntiH2` into `blitAntiH`,
  which the raster-pipeline and 32-bit blitters round differently, leaving pixels inside the clip
  a level off. Recorded in stx_skia's `docs/CONVERSION.md`.
- **Tests:** stx_skia's `aa_scan_converters.cpp` writes `aa-scan-clip-rows.raw` (clip-row-cut
  cases, whole fills against banded fills over the seam shapes and 1500 random paths, and four
  rounded shapes through a canvas cut at every row). GUI.Forms gains
  `skia_damage_edge_tests.cpp`: a scene at scales 1-3 repainted above and below every device
  row, with no byte allowed to change.
- **Verified (M4):** stx_skia candidate 137 of 137, baseline 92 of 92, candidate under the
  sanitizer 137 of 137; `aa-scan-converters.raw` still matches the reference; with the previous
  source the band checks fail (1265 failures). GUI.Forms: the 16 Skia-backed pixel tests plain
  and under the sanitizer; house style, lifetime rules, state-machine gate, stx_skia style and
  the docs check. No GUI.Forms golden moved. `logs/aaa-*.txt`.
- **Goldens that move:** the new `aa-scan-clip-rows.raw`, and six stx_skia corpora that draw an
  anti-aliased fill through a clip cutting its rows: region-clips (6.0 M of 35.3 M bytes),
  canvas-device (3009 of 280 K), android-clip (8), layers (8533), render (2344), mask-filter
  (27054). They are listed as intentional differences, which is the recommended policy; the
  alternative is splitting each corpus into stable and clip-row artifacts. Not rerun after
  listing them: `tools/compare_profiles.py`.
- **Workload:** frame time flat (damage 5.80 to 5.81 ms, pages 93.81 to 94.09 ms, within the
  M4's noise). Pixel hashes differ in 54 of 69 cases (all 9 damage cases and 45 page cases).
  Those cases repaint partial damage, so the moves are expected to be the corrected seam, but
  this is not yet confirmed case by case.
- **Pending:** review by the stx_skia worker; the coordinator's choice of corpus policy;
  confirming the 54 workload hash moves.

## FreeType findings (for the post-1.0 fork)

1. **FT_Glyph_To_Bitmap leaks** (2.14.2, pin f4205da1): the dummy slot's rendered bitmap
   (FT_GLYPH_OWN_BITMAP) is never freed after FT_Bitmap_Copy. The same path bounds glyphs only
   by 0x8000, since the 10-ems check needs `slot->face`. GUI.Forms does not call it.
2. **Autofitter script numbers contradict their documentation:** `fallback-script` and
   `default-script` are compared with the internal, alphabetically generated AF_Script enum
   (arab 1, latn 30, none 39, hani 59 in 2.14.2), not FT_AUTOHINTER_SCRIPT_* (LATIN 1 selects
   Arabic). `glyph-to-script-map` holds AF_Style indices with flag bits (0x4000 non-base, 0x8000
   digit). `tools/ft_probe.cpp` printed the values.

## Tools

The scratch tools lived untracked in the worker's `.hp/`; copies are in `tools/`. Paths in them
assume the worker layout (`gui_forms/` with `../.build/native-macos-arm64/` beside it) and run on
the M4 through the worker's sync wrapper.

| Tool | Purpose |
| --- | --- |
| `m4_profile.py`, `xt_summary.py`, `symbolize.sh`, `docview.sh` | xctrace Time Profiler on macOS: build with profile flags in `CMAKE_CXX_FLAGS_RELEASE`, record, symbolize with `nm` and a voted ASLR slide (xctrace's own symbols name inlined callees) |
| `fill_trace.patch`, `fill_run.sh`, `fill_summary.py` | Fill-volume attribution: each fill command carries its recording backtrace |
| `glyph_check.cpp`, `glyph_check.sh`, `f1_verify.sh` | F1 identity (hash lines against main) and per-glyph timing |
| `cover_check.sh`, `cover_compare.py`, `trace_compare.py` | Paint workload before (origin/main) and after: pixel hashes, display-trace removals, alternating timings |
| `band_proto.cpp`, `band_check.sh`, `band_summary.py`, `sample_locks.py` | Banded-replay prototype, pixel bisect, seam finder and lock sampling |
| `damage_probe.cpp` | The damage-edge scene one primitive at a time |
| `stx_check.sh`, `stx_full.sh`, `moved_corpora.sh` | stx_skia's analytic-fill corpus in both profiles; all profiles, comparison, sanitizer and negative control |
| `aaa_verify.sh` | GUI.Forms's Skia-backed tests plain and under the sanitizer, plus the checkers |

Pitfalls: passing `-DCMAKE_CXX_FLAGS` replaces the toolchain's initial flags and the build fails
(`from_chars` unavailable); `xctrace --all-processes` exports are too large to parse; Time
Profiler follows only the launched process.

## Resume

1. Rebase `claude/aaa-clip-invariant` onto main. Rerun `tools/stx_full.sh` (with the six
   corpora listed, comparison should pass) and `tools/aaa_verify.sh all`. Confirm the 54
   workload hash moves are seam corrections: rerun `tools/cover_check.sh`; for a sample of
   cases, diff the before and after frames and check that every changed pixel lies within a few
   rows of a damage edge under an anti-aliased path. Send the diff to the stx_skia worker
   through the coordinator, and get the corpus policy decided.
2. F2 on a new branch `claude/f2-fallback-style` from main: the helper and its four sites as
   designed; list every golden and hash that moves with its script (Latin ligatures, Indic,
   Arabic).
3. B1 on a new branch from main after step 1 lands: `AtomicThreadPool::Batch` and its tests
   (regenerate docs/api), BandedReplayTarget, the memo pre-pass (measured), per-band canvases, a
   ThreadSanitizer run, and banded-equals-serial over every workload case.

Remaining track items after B1: text compositing and `prepared_clip_coverage`, `validate_utf8`,
and the optional stx_conv resize on the pool (owner to confirm).
