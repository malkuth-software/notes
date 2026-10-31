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
