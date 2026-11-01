# Hot paths: the analytic-fill fix rebased and measured (2026-10-31)

Follows `results/2026-10-29-hot-paths`. Host: the M4 Mac mini, shared (load average 8-10
during these runs). Branch `claude/aaa-clip-invariant` (gui_forms), rebased onto main 2fe87fd9
(SK2), heads af3361f7 (the fix) and the isa-corpus listing on top. Tools in `tools/`, logs in
`logs/`.

## Verification after the rebase

- stx_skia: candidate 138 of 138, baseline 93 of 93, candidate sanitized 138 of 138. Negative
  control (the previous `SkScan_AAAPath.cpp`): the band checks fail, 1265 cases.
- `compare_profiles.py`: after listing, 63 corpora, 20 intentional differences, 0 failures.
  The rebase brought two new movers, `isa-workload.raw` and `isa-workload-highp.raw` (SK2's
  isa workload): only their `paths` case moves (443 and 431 pixels, up to 38 levels). Its
  curves' control points reach past the 320x200 surface, so the surface's own rows cut them.
  Proof (`tools/isa_unclipped.*`): the same paths drawn into a 360x260 surface translated by
  (20, 30) give the 320x200 frame's interior exactly with the fix (0 pixels, default and
  forced raster-pipeline blitters) and 432 and 443 different pixels without it. Listed as
  intentional differences.
- GUI.Forms: the 16 Skia-backed pixel tests plain and sanitized; house style, lifetime rules,
  state-machine gate, stx_skia style, docs check all pass.

## The 54 workload hash moves

`tools/aaa_probe.sh` (the coordinator's probe, extended): every paint-workload case's frame
dumped before and after the fix, then each frame repainted whole, in seven uneven row bands,
and in seven uneven column bands.

- **Rows:** after the fix every one of the 69 cases equals its own whole repaint after the
  measured partial repaints and after the row-banded repaint. Before it, 33 cases differed
  from their row-banded repaint. Before/after differences are small (0 to 717 pixels per frame;
  `logs/aaa-round2.txt`); the page cases repaint whole, so theirs come from controls' own clips
  (viewports, tab pages) cutting a fill's rows, which now gives the unclipped coverage.
- **Columns (limit a): not fixed.** 48 cases before and 51 after differ from their
  column-banded repaint, up to 4626 pixels and 40 levels (instrument page). A primitive probe
  (`tools/column_cut.cpp`, `logs/aaa-columns.txt`, 160x60, cut at every column) shows every
  anti-aliased path seams at a column cut, identically before and after the fix: rounded fill
  137 of 159 cuts, up to 65 levels; rounded stroke 139, 66; cubic fill 150, 68; a 2 px line 147,
  35 (with the raster pipeline forced: 18, 23, 150, 147). A linear gradient does not seam.
  Cause: edges are still built against the clip's columns (SkEdgeClipper chops lines and curves
  at the clip's left and right, which changes the whole chopped edge), and SkRectClipBlitter
  turns a cut `blitAntiH2` into `blitAntiH`. GUI.Forms damage rectangles need not be full
  width, so this seam is on main today, larger than the row seam was.

## Cost of walking the path's own rows (limit b)

`tools/patch_workload.py` probe_tall: a rounded fill plus a 2 px rounded stroke, 800 wide and
H tall, centred on a 900x620 raster, painted under a 100-row damage band; best of 5 x 400
frames, ns per frame, two rounds.

| H | before | after |
| --- | --- | --- |
| 600 | 8,569 / 9,892 | 21,009 / 21,820 |
| 6,000 | 8,564 / 9,321 | 120,029 / 106,661 |
| 30,000 | 8,567 / 9,340 | 88,818 / 82,022 |
| 200,000 | 8,575 / 9,438 | 70,593 / 61,969 |

Before, the cost is the band's; after, it grows with the path's rows (about 9 ns per row for
the pair; beyond 32767 rows the walk is limited, and the largest shapes take another Skia
route). A shape barely taller than the window already costs 2.4 times. The Gallery workload is
flat (pages 93.81 to 94.09 ms, 2026-10-29) because its paths are short, but a DocumentView-like
tall background or outline under small damage would pay this on every repaint.

## Options sent to the coordinator

Cost (b):
1. **Exact skip-ahead, pixels as now (recommended).** Rows above the clip are walked without
   computing coverage (a no-blit instantiation of the same walkers), then in a second wave
   full-row stretches between events (edge ends, curve segments, intersections) are skipped
   in one step (x += n * dX is exact in fixed point). Proved against the full walk (edge state
   and pixels) on the corpora and the random paths. Cost back to about the band's for the second
   wave.
2. History-free edge positions (x from each segment's own start, never accumulated), so the walk
   can start at the clip's top exactly. Simpler and clip-invariant by construction, but every
   anti-aliased corpus and many GUI.Forms goldens move against the reference.
3. Accept the cost.

Columns (a):
1. **Extend the invariance to columns in stx_skia (recommended).** Build edges against the
   path's own columns, let the additive blitters cut columns, and cut the walkers' direct
   blits with a column-aware clip blitter that keeps each call's kind (a cut `blitAntiH2` or
   `blitAntiV2` becomes a `blitMask` of the same two pixels, which is what those blitters do
   internally; to be checked blitter by blitter).
2. GUI.Forms widens every damage rectangle to full rows. Simple, but it multiplies the painted
   area of small damage (a caret, a hovered button), against the CPU work in item 2.
3. Record it as a known limitation.

## Idle and animating CPU per Gallery page and lab (task 2, first pass)

Measured on the M4 with a headless real-time simulation (`.hp/cpu_sim.cpp`
on the gui_forms side, scratch): the window is polled at its own
`next_wake`, damage is taken, aligned outward and painted into a SkiaRaster
at scale 2, and presentation is acknowledged. This covers the core and the
raster; it leaves out CoreGraphics presentation. The real AppKit host could
not be measured: the M4's session is locked, so every window is occluded
and paints nothing (the vsync lab's producer still showed 12.3% CPU and 56
wake-ups a second there). Each stage settles 1.5 s and is measured 4 s; CPU
is process user+system time over wall time. Logs: `logs/cpu-sim-before.txt`
(origin/main 2fe87fd9), `logs/cpu-sim-inspector-after.txt` (with the fix).

Steady state before any fix, every stage above zero (all other pages and
labs measured 0.00% with no wake-ups; a page switch costs one full paint):

| Stage | CPU % | Paints/s | Damage (window fraction) | Cause |
| --- | --- | --- | --- | --- |
| Gallery: ranges-and-progress | 22.0 | 48.5 | 0.062 | Four animated ProgressBars as 16 ms active surfaces (194 ticks/s); `fill_linear_gradient_spread` dominates the samples (lowp gradient stages, about 14-25 ns per pixel) |
| Gallery: animation-laboratory | 20.6 | 49.0 | 0.315 | EasingPreview damages its whole face each frame; half the samples are HarfBuzz shaping and FreeType rasterizing of its static text, re-done because a rebuilt chunk drops its text caches |
| Layered material lab | 15.7 | 3.7 | 1.000 | Inspector view and window-spanning overlay as 250 ms active surfaces; the scheduler repaints an active surface every tick |
| Visual inspector lab | 14.1 | 4.0 | 1.000 | Same |
| VSync lab | 13.3 | 4.2 | 0.045 | 240 Hz producer thread renders whether or not the window is visible (paint itself 1.3%) |
| Seam proximity lab | 6.4 | 4.0 | 1.000 | Same as the inspector labs |
| Gallery: instrument | 3.6 | 24.7 | 0.072 | 30 Hz active surface reaching 25 Hz; sweep advances per paint, not per time |
| Gallery: timing-and-tooltips | 1.3 | 4.2 | 0.031 | 4 Hz UI timer by design, only while the page is shown |

Fixed (clear defect, gui_forms `claude/hot-paths-inspector-quiet` e1676a27):
the labs poll `refresh_if_changed()` from one UI timer; both inspector
controls repaint only when what they would draw changed, and skip occluded
windows. The view's pending-damage count is drawn but not compared, since it
always counts the view's own repaint (comparing it made every poll repaint).
After: layered material 0.11%, visual inspector 0.06%, seam proximity 0.10%,
0 paints a second, 4.2 polls a second.

Proposed fixes, ranked by measured cost, sent to the coordinator as options:
ProgressBar animation (damage only the moving fill, hold the stripe tile,
and the gradient kernel under section 8); a text-run cache that survives
chunk rebuilds (memory budget is an owner decision); EasingPreview damaging
only its marker tracks; pausing the vsync producer while not presented
(owner decision: the lab exists to stress the producer); a time-based
instrument sweep; and, as a framework option, an active surface whose
on_frame reports whether it changed.

## AAA skip-ahead and column invariance (stopped at checkpoint 7, partial)

gui_forms `claude/aaa-clip-invariant` cac5a320 (WIP, not for folding). Logs:
`logs/aaa-random-skip-vs-full.txt`, `logs/aaa-column-cut-after.txt`, `logs/aaa-round3.txt`,
`logs/aaa-round4.txt`.

- **Skip-ahead (decision 1): exact.** 1500 random paths x 2 blitters hash bit-identical with
  skip-ahead on and off (also under ASan/UBSan, 600 cases). Tall rounded fill + 2 px stroke
  under a 100-row band, ns/frame, before (origin/main) -> after: 600 rows 9.3-10.0k ->
  11.4-12.3k; 6000 rows 9.4-10.0k -> 11.3-12.2k (row fix alone: 107-120k); 30000 rows
  9.3-10.0k -> 11.7-12.6k. The 200000-row case read 1.7k after; it was taken before the
  over-32767 fallback and is unexplained, so it is not counted.
- **Columns (decision 2): partial.** Primitive probe clean at every column cut, both
  blitters (was 137-150 seamed cuts of 159, up to 68 levels). Random AA fills on a canvas: no
  row, column or region seams; strokes thinner than 1 px (hairlines, a separate scan
  converter) still seam, up to 5 levels. Workload: 69/69 frames equal their row-band repaint,
  48/69 their column-band repaint (21 still differ, up to 219 levels; cause not found).
  Open defects: stx_skia aa_scan_converter column-band random cases differ (large deltas,
  coverage blitter), and an ASan out-of-bounds read in blit_aaa_trapezoid_row from
  canvas_device's drawPoints.

## AAA column invariance: finished (2026-10-31 late)

Branch `claude/aaa-clip-invariant` 480dd3af (on main 6856347b). Three defects fixed after the
partial state above:
1. The ASan out-of-bounds read: the walk reached 32767 px, past the ±8191 that
   SkScan::AntiFillPath guarantees, and a steep edge's SkFixed x wrapped (tiled canvas
   corpus, ir 8190-8217 under a 0-8191 clip). The walk is now limited to ±8191.
2. The random-path column-band failures (all 93 were rectangles): `blitFatAntiRect` reused
   the top row's runs for the bottom row after a clip blitter had broken them in place.
3. Anti-aliased clips cut by column bands (17337 px, up to 240 levels, in a new probe kind;
   the workload's large deltas): the cut `blitAntiRect` was sent as blitV + blitRect, which
   the SkAAClip builder cannot take; it is now one blitAntiRect with cut edge alphas 0.

Workload column-band repaints: 61/69 frames identical (origin/main's stx_skia: 21/69). The 8
left are one-row strips of 1-4 levels from hairlines and 1-pixel lines (SkScan_Antihair),
identical on origin/main. Per-primitive probe (`logs/aaa-kinds-and-workload-columns-after.txt`):
every AAA kind clean in rows, columns and regions; AA rect fill, image and gradient rects (1
level), rrect stroke w1 (2) and hairlines (7) keep Skia's seams. Verification:
`logs/aaa-columns-verification.txt`.
