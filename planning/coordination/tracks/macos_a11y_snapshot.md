# Track: macOS accessibility, reuse the semantic snapshot (Mac side)

Requested 2026-11-03 by w4 (hot paths, Windows side) through the owner;
item 4 of `handoffs/W4_MAC_HANDOFF_2026-11-03.md` in this repository.
Workspace w4 (`~/Developer/gui_forms_workers/w4/gui_forms`), local branch
`claude/macos-a11y-snapshot` off `origin/claude/page-cpu` 68059db0 (which
contains the Windows reference fix 82aa7dc6). Build and test only on the M4
through `bin/m4run w4` / `bin/m4ws w4`. Never push; the coordinator delivers.

The coordinator runs other M4 jobs from w3 at the same time (tests, the
vsync lab, the T5 probe). m4ws serialises them; activated GUI tests need the
M4 idle (`M4WS_WAIT_IDLE=180`).

## The defect

In `src/host/macos/application/macos_host.mm`, `accessibilityChildren`
builds `(*_model).semantic_snapshot()` and only then compares
`snapshot.generation` with `_accessibilityCacheGeneration`, so every
VoiceOver query builds a whole snapshot to find it unchanged.

## The fix

Read `(*_model).semantic_generation()` first and build the snapshot only
when it differs from the cached one. Every other caller of
`semantic_snapshot()` in that file gets the same treatment. If the semantic
generation alone misses changes (values and bounds can change without a
bump), use the key the Windows fix uses (82aa7dc6: content revision, display
generation, focus and activation). Decide by reading what bumps
`semantic_generation()` and what 82aa7dc6 keys on, not by trial: state in
the commit message why the chosen key is sufficient.

## Measure

`gui_forms_macos_accessibility_client_gallery_tests` (and the other macOS
accessibility tests), plain and `--sanitize`, before (at 68059db0) and after
the fix: ctest's own time per test; repeat each once more if the two runs
disagree by more than 10%. All macOS accessibility tests must pass after.
Then the house-style and lifetime checkers on the touched files.

## Hand back

Local commit on `claude/macos-a11y-snapshot`; report the commit, the
before/after times (plain and sanitized), the key chosen and why, and the
exact commands. Record milestones below.

## State

2026-11-03 (w4 worker):
- Baseline at 68059db0 measured (ctest times, two rounds each; log
  `w4/dev/logs/a11y-before-1103-161628.log`). Plain: tests 0.86/0.50 s,
  client 10.61/9.96 s, client_gallery 6.71/6.56 s. Sanitized: 2.51/1.53,
  13.02/11.90, 7.77/7.23. All 3 pass in each round.
- Fix written (uncommitted at this point): `MacAccessibilityTreeKey`
  {semantic generation, content revision, display generation} read before
  any snapshot; snapshot only when it moved; the key stored is re-read after
  the snapshot (layout inside it can move it). Why: arrange_dirty_recursive
  moves bounds with touch_paint only (content revision), no semantic bump;
  elements store node bounds. Focus/text/enabled/visibility/resize bump the
  semantic generation (invalidation sets in include/gui_forms/dirty.hpp).
  Pressed/captured/active/title/scale left out: the macOS snapshot does not
  read them (append_semantic_nodes reads only focus) and frames are
  converted to screen at read time.
- Tools: `w4/dev/tools/a11y_measure.sh` (plain+sanitize, N rounds, ctest
  time lines) and `a11y_verify.sh` (measure + style + lifetime + gate); m4ws
  syncs only gui_forms/, so copies live there as `.a11y_*.sh`, excluded via
  .git/info/exclude. Run: `M4WS_WAIT_IDLE=180 bin/m4run w4 -- sh .a11y_verify.sh`.
- DONE: committed 0bb7a572 on claude/macos-a11y-snapshot (local, not
  pushed). After times (two rounds): plain 0.58/0.48, 9.76/10.22,
  6.65/6.36 s; sanitized 2.09/1.38, 12.03/11.73, 7.44/7.13 s. No change
  beyond noise (tests dominated by launch and AX IPC). 3/3 pass in every
  round; style, lifetime, gate clean. Log
  `w4/dev/logs/a11y-after-1103-163358.log`. Next: nothing; await fold.
