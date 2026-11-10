# State-machine hunter fixes track (worker w3)

Workspace: `D:\gui_forms_w3` (checkout `/d/gui_forms_w3/gui_forms`). House style 1 and 9.
Read `D:\gui_forms_workers\BRIEF.md` first. Moved here from `tracks/stage_a.md` on 2026-10-22.

## RESUME HERE (2026-10-28, before the machine move; supersedes the queue notes below)

Everything that matters is pushed; nothing exists only on the old machine except build trees.

PAUSED 2026-10-28 (final pause, machine move). Verified at the pause: no dirty worktree and no branch absent from
the remote in gui_forms, template_harness (and worktrees th_lb, th_scratch) or notes. The coordinator owns the
harness #10 merge (merge-when-green watcher). #435 (hosts + stx E2 batches) MERGED as ca67b70f after the pause
notice: all four E2 areas are on main. On resume: confirm harness #10 merged, then the open queue below.

**E2 repair queue (coordinator item 3), gui_forms** (decisions in `tools/loop_branch_decisions.json`, sorted by path,
function, condition; every area edits it, so area PRs stack):
- core #433 MERGED (102 -> 85 invariant sites, all decided). controls #434 MERGED (151 -> 135, all decided).
- hosts #435, branch `claude/e2-hosts-hoists`, base main, head 9b272720, auto-merge ON, CI running at the move.
  #437 (stx_*, `claude/e2-stx-hoists` 5c93b154) was MERGED INTO the hosts branch, so #435 now carries hosts + stx.
  - hosts batch (5025c7a6): host/render/audio/demo 14 -> 10 decided; DIB painter gradient/stroke loops split by mode,
    pinned by `expected_gradient_hash` in tests/windows_content_pixels_tests.cpp (recorded from the old code).
  - stx batch (5c93b154): 51 -> 42 decided (23 false_shape, 19 keep). Verified on its own base 5025c7a6: png + audio
    ctest (8) and the sanitizer over them; png_writer identity hash unchanged; stx_vorbis's own 7 tests (standalone
    build, plain and STX_VORBIS_SANITIZERS=ON) pass. Not re-run on 9b272720 (a server-side merge of main).
  - Counts exclude any path with a tests/tools/fuzz segment: 318 at the start (the coordinator's 327 excludes only
    top-level dirs). After all four areas: 272 remain, every one decided (240 keep, 32 false_shape;
    core 80+5, controls 134+1, hosts 7+3, stx 19+23);
    the decisions file holds 218 entries (one entry can cover several sites).
  - Next if #435 fails: read the failing job's log, fix on `claude/e2-hosts-hoists`, push; auto-merge stays on.
- Working files (first-run queue `e2_queue.json`, report, per-area dumps, edit and decision scripts, before/after
  reruns, vorbis_tests.sh): template_harness branch `claude/e2-repair-scratch` (56e94ba), `scratch/e2_gui_forms/`,
  with a README giving the rerun command. Not for merging.

**template_harness** (no branch protection: merge by hand with `gh pr merge N -R malkuth-software/template_harness
--merge` only when EVERY check is green; never --auto):
- #9 MERGED (8db4b47) after all 22 checks green: lockstep drops core.autocrlf; adopting.md CRLF/cache/Skia
  pitfalls; ci_build_steps.py:222; gui_forms.toml drops the Windows skia=false override.
- #10 `claude/gui-forms-loop-decisions` head 56457b1 (origin/main with #9 merged in; it merged without conflict,
  CHANGELOG keeps both sets of bullets; test_project*.py 56 pass): gui_forms.toml `[loop_branches] decisions =
  "tools/loop_branch_decisions.json"`. CI restarted on 56457b1. Next: merge by hand when EVERY check is green.
- `claude/loop-branches` (315d21c): the superseded E2 v1 WIP on the old JSON reader, pushed only to preserve it.
  E2 v2 merged as harness #7. Safe to delete after the owner agrees.

**notes**: #1, #3, #4 MERGED. Nothing open.

**Open queue (after the E2 stack):**
1. Codemod improvement (coordinator item 4, later): a dereference of a non-const optional field that is only read,
   e.g. `(*r.image).pixels == x`, is a hand site today; the codemod should rewrite it as a read-only access.
2. The E2 keep and false_shape decisions: one file, gui_forms `tools/loop_branch_decisions.json`, maintained with the
   repairs; harness #10 wires it into projects/gui_forms.toml. New invariant sites from later code get a hoist or a
   recorded decision with a reason; a hoisted const bool does not lower the count, only removing the branch does.
3. Then: report counts to the coordinator and stand by for the 0.5 leg.
- api_reference.py's stx_zlib include problem belongs to the cleanup worker, not this track.

## History: core routing and Windows (w3, 2026-10-22)

State: A-core, A-W2 (UIA provider), A-W3 (UIA Gallery client) and #211 merged.
Since then: the state-machine hunter's findings (#232) fixed in four draft PRs,
each verified locally on touched tests + sanitizer + both checkers; CI pending:
- #243 core/C ABI/render/examples, head 1f1b5dcd (on main b1e37c1f)
- #246 controls/demo, head 24f6cfbc; #247 hosts, head 27f37948;
  #248 tests/stx_vorbis, head 0bc6c348 (all on main 7ce8f0b2)
Hunter on core alone after #243: core F1 3 (all `#if` false positives), F2 0.
Next step: merge the four heads locally (detached), rerun the hunter
(`scratchpad/tools/hunt.sh final`) and report F1/F2 counts; propose hunter
tuning on #232 (drop preprocessor lines in body_statements; accept a fallback
that runs to the function's end, masked_text_box); public-API C3 decisions for
the owner: ErrorProviderChange::has_error, ImageAttributesSnapshot::
has_color_matrix, HostClipboardImageResult::has_image, stx_vorbis
OggPacket/OggStreamEnd::has_granule.

## Owner decisions (relayed by the coordinator, 2026-10-22)

1. TextMaskLimit: give the text-mask cache's entry count its own specific name instead of
   reporting it as `metadata_bytes`.
2. Public `has_*` fields (ErrorProviderChange::has_error, ImageAttributesSnapshot::has_color_matrix,
   HostClipboardImageResult::has_image, OggPacket/OggStreamEnd::has_granule): reshape only where
   the flag can disagree with the value beside it; keep a flag that is the only record of its
   fact. The PR says which changed and why.
Both go in their own PR, with docs updated, after the restack/conflict/hunter steps.

## State (2026-10-22, after restack/conflict/hunter steps)

- Cause of #246-#248 failing everywhere: not a dependency. CI checks out the PR head SHA but
  runs main's workflow, which calls `tools/audit_link_map.py` and `tools/ci_sanitizer_seed.py`
  (added on main after 7ce8f0b2). All builds and 171 tests passed on Linux. Fixed by
  `gh pr update-branch` on #246, #247, #248; no restack needed, each builds alone on main.
- #243 merged with main (b458bb1b): replay `begin_group`/`end_group` added in the compliant form;
  the PNG colour reshape moved to `src/core/image/color_conversion.{hpp,cpp}` (`ColorEncoding`).
  Plus 31c65d11/325c6ada: `apply_operation` and `corner_radii` (new on main) fixed. Head 325c6ada.
- #248 head 37453290: paint_group_tests and part_composite_scene.hpp switches (new on main) fixed.
- #250 (`claude/state-machine-hunter-preprocessor`, on main): hunter drops preprocessor lines in
  body_statements; unit test added.
- Hunter over main + #243 + #246..#248 + #250 hunter: F1 3, F2 0. The 3 are the listed false
  positives in controls (fallback computed then returned at function end: masked_text_box,
  button_base, container_layout_utilities); left unedited.
- Tool gap (for the build worker): `ci_build.py --sanitize --tests <regex>` leaves some matching
  tests unbuilt (Not Run: png_malformed_sweep, stx_png_core, engine_expression/input/motion/relief,
  paint_group). Worked around by building those targets in the sanitizer tree by hand.
- Windows "Expected LLVM 22.1.8; got clang 23.1.3" CI failures are a toolchain-pin issue the build
  worker is fixing (coordinator, 2026-10-22).

## Open PRs (2026-10-22, end of this pass; all drafts, the coordinator merges)

| PR | Branch | Head | Base |
|---|---|---|---|
| #243 | `claude/state-machine-fixes-core` | 325c6ada | main |
| #246 | `claude/state-machine-fixes-controls` | update-branch merge | main |
| #247 | `claude/state-machine-fixes-hosts` | update-branch merge | main |
| #248 | `claude/state-machine-fixes-tests` | 37453290 | main |
| #250 | `claude/state-machine-hunter-preprocessor` | (tools only) | main |
| #258 | `claude/state-machine-owner-decisions` | 87b4471b | #243 |

#258 does both owner decisions: `TextMaskLimit::cache_entries`; has_* reshapes (all four listed
records changed: ErrorProviderChange::has_error removed; ImageAttributesSnapshot::color_matrix,
HostClipboardImageResult::image, OggPacket::granule/page_granule, OggStreamEnd::granule optional).
Verified: 13 touched tests plain + ASan/UBSan; text-mask tests in `.build/textmask`
(GUI_FORMS_BUILD_TEXT_MASKS=ON, plain only); stx_vorbis standalone `.build/stx_vorbis_{OFF,ON}`
7/7 each (`sh /d/gui_forms_w3/extra.sh`). Worktrees: wt_controls, wt_hosts, wt_tests, wt_hunter.

## Next step

Wait for the coordinator: fix CI failures on #243/#246/#247/#248/#250/#258 when woken (Windows
LLVM-pin failures are the build worker's). When #243 merges, retarget #258 to main.
Optional hunter tuning still open: accept a fallback computed and then returned at the function's
end (the 3 listed controls false positives); not done, not asked.

## Standby: Window::take_damage() leaves paint_dirty_ stale (2026-10-22)

Confirmed by test (fails on main): after replace_png (damage on every plane, no dirty control), taking
all damage left needs_frame() true and wake requests armed. #266 (`claude/window-damage-paint-dirty`,
on main, head 535f132f): paint_dirty_ removed; `Window::paint_pending()` derives from
dirty_after_render_, root/popup paint-dirty subtree flags and plane damage. 25 paint/frame/host tests
plain + ASan/UBSan. macOS #249 "frame pending" is needs_frame(); consistent with this bug, but
confirmation needs #249's CI on top of #266. Possible second route (unconfirmed): a paint-dirty control
whose damage bounds are empty keeps needs_frame true with nothing for the host to draw.
2026-10-22 later: main (c123b6e9, with E7r #261) merged into #266; E7r's idle keyframe bake now reads
paint_pending() (head 17784ac4). 40 window/scheduler/engine/paint tests plain + ASan/UBSan pass.
#249's macOS dump: "frame pending yes ... paint lease dirty_queued content 259 rendered 259
presented 259": everything rendered and presented, yet the stored flag said dirty, which is the stale
paint_dirty_ this PR removes.

## Standby: paint lease stuck dirty_queued with nothing owed (2026-10-22)

#249 on macOS, after #266 merged: "dirty_queued, content = rendered = presented = 259". Cause: owed
work included the root's paint-dirty subtree flag; a control no paint reaches (outside the client
area, or clipped by a viewport) keeps its mark forever. #275 (`claude/paint-lease-owed`, on main,
head c3fb6ff1): paint_pending() = dirty_after_render_ || plane damage; paint_lease_state_ removed,
paint_lease_state() derives it (no hand-assigned states). Test an_unreachable_dirty_control_owes_no_frame
fails on main. 43 tests plain + ASan/UBSan, 26 wider plain. Windows/Linux hosts never read lease
state or needs_frame. Next: #249's macOS CI on top of #275 confirms.
Standing check (coordinator, 2026-10-22): when a change removes or renames a member, run
`python tools/generate_library_docs.py` then `--check` (manual.json method explanations go stale).
#275 docs fix pushed.

## Hunt 2 and the gate: DONE, both PRs ready with auto-merge (w3 successor, 2026-10-25)

| PR | Branch | Head | Base | Content |
|---|---|---|---|---|
| #311 | `claude/state-machine-hunt-2` | 31770544 | main | C++ fixes only (main 24fb355d merged in) |
| #314 | `claude/state-machine-gate` | 1622e0f3 | main, stacked on #311 | checker + tests + workflow + results + docs/dev/tools.md |

- Split decision: #311 lost its tools/workflow changes (commit 31770544 restores main's), and #314 re-adds them
  in a new commit on top of #311. Do NOT branch the gate from a commit before the removal, because a merge
  would revert the tools.
- Rerun on the merged Windows tree (gate checker): F1 0, F2 0, C1 10, C2 31, C3 2. Before: F1 14 (7 real,
  7 false positives), F2 2. The 10-21 baseline: F1 52, F2 9. Main's untuned checker on #311 shows 7 F1, all
  false positives.
- Verified for #311: 18 touched tests plain and under ASan/UBSan (regex in D:\gui_forms_w3\dev\hunt3\regex.txt);
  house-style 0, lifetime 0, library docs check PASS.
- Verified for #314: tools/test_state_machines.py 28 tests (2 new: changed_sources in a scratch git repo,
  and a no-source run that passes without a tree). A hosts-off, Skia-off tree (D:\gui_forms_w3\.build\gate,
  configured from the main checkout because fetched deps live there): full 93 s, one file 12 s, none 6 s.
- Gate check name is `state-machine-gate` (reported to the coordinator). The coordinator adds it to required
  checks after its first green run on main. Daily full-tree run: the same workflow, cron '41 3 * * *'.
- Not gated, by design: host adapters and Skia (F1/F2 are compiler-only, the Linux tree compiles no host).
  Possible follow-up: enable the Linux host in the gate tree (needs X11 dev packages).
- C3 for the owner: MonthCalendar::today_set_ (private), doc example Reminder::has_date. The owner approved both
  (2026-10-25). Done in #315 (`claude/state-machine-c3`, head 001a08ad, base main, stacked on #311, auto-merge on):
  MonthCalendar keeps system_today_ plus std::optional set_today_; the Reminder example holds an
  std::optional date. Docs regenerated. 3 tests plain and under ASan/UBSan; checkers 0; C3 is now 0.
- Records: results/2026-10-25-state-machine-hunt-2/ (in #314). Raw outputs are in D:\gui_forms_w3\dev\hunt3\.
- Next step: when woken on a CI failure, read the log. The Linux configure and install in the gate are
  unproven until #314's first run. Worktree D:\gui_forms_w3\wt_gate.

## Hunt 2 and the gate (2026-10-23, WIP, interrupted by shutdown; superseded by the section above)

Branch claude/state-machine-hunt-2 (draft PR, head ec782bc9, NOT verified; no auto-merge).
Hunter over main 89635628 (Windows tree): F1 11 + 3 in newly scanned stx/fuzz dirs, F2 2;
after fixes + tuning the full Windows run is F1 0, F2 0 (C1 10, C2 31, C3 2).
Fixed files: material_primitive, procedural_pattern, relief_primitives, semantic_snapshot,
windows_text_automation, cng_pixels, backplane_composite, fuzz/inflate_protocol_fuzz, stx_cng/tests/common.hpp.
Hunter: tuning (fallback at block/function end; nested-loop break; fallthrough), stx/fuzz dirs,
--changed-from/--list-changed/--gate; 26 unit tests pass. Workflow .github/workflows/state-machines.yml,
job name `state-machine-gate` (Linux, Skia and hosts off).
Owner approved the gate as a required check, new code only; coordinator adds it after first green main run.
Left: touched tests plain + ASan/UBSan, house-style + lifetime checkers, first CI run (full tree since
checker changed; Linux Skia-off configure untested), timings (one-file PR, no-source PR), daily full-tree
command to the build worker (ae67c70cef46d7057): `python3 -B gui_forms/tools/check_state_machines.py
--root gui_forms --build-dir <tree> --jobs 4 --gate --json state-machines.json`, results dir
results/2026-10-23-state-machine-hunt-2/ (raw outputs in D:\gui_forms_w3\dev\hunt2\), C3 list for owner
(MonthCalendar::today_set_, doc example Reminder::has_date). PR body: D:\gui_forms_w3\dev\pr_bodies\hunt2.md.

## Standby: close stage DP (2026-10-25, in progress)

Task (coordinator, from the owner): record the DP gate on macOS and X11 from a CI run, then mark DP complete in
planning/stages/DP-damage-preserving-painting/README.md and planning/stages/README.md. One PR, auto-merge.
If a host genuinely fails the gate, stop and report; do not paper over it.
- #320 (`claude/dp-gate-records`, from main d6111581, head 1374fa59, DRAFT, no auto-merge yet): adds the
  X11 `damage_rectangles_gate` in tests/linux_host_tests.cpp (raster plus XGetImage screen diff) and the new
  tests/macos_damage_rectangles_tests.mm (bundle fixture; occluded cacheDisplay readback). Both print
  `dp-gate|host=...` lines. Neither compiles on Windows, so CI is their first build.
- Next, when woken after CI: download the gui-forms-evidence-<platform> artifacts (LastTest.log) and take the
  dp-gate lines. Write results/2026-10-25-dp-gate-macos and -x11 (run id, painted area against the sum and the
  bounding box, rectangles/pixels presented, untouched pixels). Update both briefs, then mark ready plus auto-merge.
- Known risk: AppKit's getRectsBeingDrawn on layer-backed views may hand over the union. The macOS test would
  then fail on painted area or rectangle count. That is a genuine finding to report, not to loosen.
- 2026-10-25, after run 40234395220: X11 passes on x64 and arm64 (painted 2400, box 99900, 2 rects and 2400 px
  copied, 0 changed outside in the raster and on screen); recorded in results/2026-10-25-dp-gate-x11. macOS failed:
  painted 120000, presents 1, rects 1, pixels 99900, changed inside 2400 and outside 0.
  - Cause 1, a test artifact (fixed in 65f3b43e): the occluded cacheDisplay readback gives drawRect: the whole view,
    and the host keeps it as _pendingDamage while the model declines to paint, so the next visible frame painted
    everything. The test now drains it with one whole-window frame before the baseline, and prints native_dirty_area,
    cg clip bounds, wantsLayer, layer_backed and layerContentsRedrawPolicy.
  - Cause 2, likely genuine: AppKit handed drawRect: ONE rectangle, exactly the 370x270 union of the two
    setNeedsDisplayInRect calls (the host calls setNeedsDisplayInRect per rectangle; layer-backed CALayer keeps one
    dirty rect). The host then paints the drawn rects (they are added to _pendingDamage), so painted will be 99900.
  - Expect the rerun to show painted 99900 and rects 1. Then report owner options: (A) the adapter paints only model
    damage when the raster is reusable (paint = sum; the copy stays AppKit's union), recommended; (B) present
    through CALayer tiles or sublayers so the copy is the set (large); (C) macOS without partial present cannot attach
    under contract 2, so not viable. No ready or auto-merge until macOS passes or the owner decides.
  - Side finding: an occluded exposure leaves whole-view pending damage on macOS, which makes the next visible frame a
    full repaint (a small adapter inefficiency, not fixed).
- 2026-10-25, rerun 40235599394 at 65f3b43e confirmed cause 2 (painted 99900, native_dirty 99900, layer_backed 1,
  wantsLayer 0). The owner said "study what we did for playsuite"; the coordinator directed option A as the
  6e9a8171 live-surface rule. Implemented in bbd25b4d (pushed, CI pending):
  - macos_host.mm drawRect: on a reusable raster, AppKit's drawn rects become copy-only except inside live-surface
    destinations (new detail::append_within_live_destinations in live_surface_damage.hpp, unit-tested in
    tests/opaque_live_surface_tests.cpp; plain and sanitizer passed on Windows).
  - tests/macos_damage_rectangles_tests.mm rewritten: screencapture -R grab (posix_spawn/waitpid) before and after,
    a permission/visibility check, painted == sum, raster and screen outside 0, inside == expected; presented
    rects/pixels only reported (rects >= 1, pixels >= expected), since AppKit's copy stays the union.
  - games repo was read (exception ended with the study): tools/screen_check.py and tools/measure_frames.py cited in
    the #320 body; nothing copied. Local copies in D:/gui_forms_workers/games_study/.
- Next, when woken: if macOS is green, write results/2026-10-25-dp-gate-macos (run id, painted vs sum and box, the
  copy as AppKit's union over unchanged pixels, NOT the rectangle set, raster and screen untouched counts), update the
  macOS row of PAINT_PIPELINE_AVAILABILITY.md the same way, mark DP complete in its brief and stages README, then ready
  plus auto-merge. Tell the coordinator the "receive the rectangle set" clause may need the owner's acceptance on
  macOS. If screencapture lacks Screen Recording permission on the runner, report it (do not drop the screen check
  silently).
- Side finding (not on this path, own PR if wanted): notifyOcclusion:NO adds whole-view pending damage and
  setNeedsDisplay:YES, so every unocclusion is a full repaint; it is deliberate ("AppKit may consume a display request
  while our model is still hidden"). With bbd25b4d the setNeedsDisplay part is a copy, but the added full-bounds
  _pendingDamage still repaints everything.
- 2026-10-25, run 40237617801 at bbd25b4d: macOS gate test timed out (10.5 s, no dp-gate line). Cause: the rewrite
  dropped first_frame's root invalidate + collectDamage, so the drain waited for a presented revision that a repaint of
  unchanged content never produces. Restored in bd88d8e1; stages now print `dp-gate-stage|<name>|presented_revision=`,
  captures print `dp-gate-capture|<name>|region=`, and a timeout names its stage.
  - Host fix in the same commit: a pure copy drawRect: (no model, live or live-exposure damage) would reach
    begin_prepared_frame with empty damage, which clears the candidate and repaints the whole raster. It now keeps
    AppKit's rects in that case (the pre-bbd25b4d behaviour). Lease and receipt advance as before when model damage is
    painted (model damage is always inside AppKit's rects: collectDamage requests display for each rectangle).
  - Owner still to answer: the macOS rectangle-set clause, and the occlusion side-finding PR.
- 2026-10-25, run 40239056547 at bd88d8e1: green everywhere (macOS 217/217, gate test 1.84 s). dp-gate macOS: painted
  2400, box 99900, 1 present, 1 rect / 99900 px copied (AppKit's union), raster and screen 2400 inside / 0 outside.
  8c5b66a4 adds results/2026-10-25-dp-gate-macos, the PAINT_PIPELINE_AVAILABILITY macOS row (union over unchanged
  pixels, not the set) and a 2026-10-25 status paragraph in the DP brief (stage open for the owner).
- Next, on the owner's answer: if the clause is accepted for macOS, mark DP complete in its brief and in
  planning/stages/README.md, push, then `gh pr ready 320` and `gh pr merge 320 --auto --merge`. If not, bring options
  (for example CALayer per-rectangle presentation). Occlusion side-finding PR only if the owner asks for it.
- 2026-10-25, owner "accept. accept": DP decision 6 recorded (macOS copy accepted as recorded), DP marked complete in
  its brief, planning/stages/README.md and the roadmap row (57aa733f). #320 ready + auto-merge; branch updated
  server-side (dc95f540). DP DONE once #320 merges.
- Occlusion side finding: #337 (`claude/macos-unocclusion-damage`, head ffdc0111, DRAFT), STACKED on #320 because it
  edits #320's drawRect: path; retarget to main after #320 merges, then ready + auto-merge when green.
  - notifyOcclusion:NO no longer adds full bounds; occluded draws with the front held and no live frame take AppKit's
    rects as copy requests; a draw with nothing owed and no live frame is copy-only (no prepared frame, no begin_frame,
    no receipt). Test stages hidden/revealed/bare_reveal in macos_damage_rectangles_tests (block 1200 painted on
    reveal; 0 paint passes on a bare reveal; occluded_dirty asserted). Copy-only draws record no screen-copy time
    (as before for pixels: a rejected same-revision receipt never recorded them).
- NEXT: paint-time pixel snapping (owner L-Q8), own PR from main. Notes: D:/gui_forms_workers/tracks/l_layout.md and
  results/2026-10-25-l1-layout-contract/README.md (#333). Golden-image updates listed in the PR body.
- #337 HOLD (owner: the old white-window bug, bdf3d413). 8771c42a: detail::copy_only_draw / front_held in
  src/host/macos/application/front_reuse.hpp (epoch via _frontSurfaceEpoch set on committed receipt; extent; clean
  lease; no lost live region; nothing pending); unit-tested in opaque_live_surface_tests (plain+sanitizer pass).
  Copy-only still copies (CGContextDrawImage whenever _rasterHasContent). setNeedsDisplay:YES rearm kept. Test adds
  resize-while-hidden (painted >= view area) and real miniaturize/deminiaturize x2 (with/without hidden change) with
  screencapture equality outside the block and a not-white check; `dp-reveal|...` lines carry native_draws/painted/
  passes = the evidence. Counters doc note added. When CI reports: if green, tell the coordinator "copy-only holds";
  if the restore comes back wrong, revert the reveal to full-bounds damage (main behaviour) and close the side finding
  as intended. Do not mark ready until the coordinator says.
- #320 MERGED (DP complete). #337 retargeted to main by the coordinator; DO NOT push to its branch until the coordinator
  sends the macOS dp-reveal result.
- Pixel snapping (L-Q8): #340 (`claude/paint-edge-snapping`, from main, head 9d9eb6f0, DRAFT; rounded strokes snapped too, scene has rounded fill+border). snap_device_edge in
  device_damage.hpp; Skia/CG snapped_local_rect / snapped_user_rect (axis-aligned only; fills, gradients, stroke ring
  even-odd, rounded-rect bounds); Dib edge_rect/device_area/fill_area, stroke bands tile (fixed corner double-blend),
  GDI clip rounds. Scene test tests/support/edge_snapping_scene.hpp in skia_edge_snapping_tests,
  coregraphics_raster_edge_snapping_tests, windows_canvas_raster_tests. Golden moved: windows_shape_reference box kinds.
  Local: 19 windows_ tests pass; 6 DIB tests pass under sanitizer. Skia/CG first compile in CI: on failures, update the
  listed Skia goldens (L1 record list) and add each to the #340 body's golden list with the reason.
- #337: run 40247570489 green but the real restore repaints fully (activation dirties the tree), so copy-only was not
  exercised on a real restore. Coordinator approved a real-occlusion case: f8dfc1aa adds cover/covered/uncovered
  stages (non-activating NSPanel; "not exercised" if never occluded / key changes / no AppKit draw). Evidence in the #337
  body. NEXT: on the run, read dp-reveal|occlusion lines; if green with paint_passes=0 on the unchanged pass, ready +
  auto-merge; if "not exercised", report to the coordinator and do not weaken.
- 2026-10-25 later: #337 evidence held at run 40250561098 (occlusion unchanged: 1 draw, key 1/1, painted 0, outside 0,
  red 235; changed: 2 draws, painted 1200, outside 0). Lines added to the #337 body; READY + auto-merge queued.
- #340 head 09ba7bcb (still DRAFT): Skia rounded fill/stroke now drawn in device space (snapped_device_rect,
  device_radii, resetMatrix): the float round-trip put AA edges a hair inside a pixel (scale 1.5, px 11,42). CG: the
  snapped gradient clip has AA off; the scene's gradient check allows a dither of 2 (gradient_dither) and prints the
  largest ink difference (Quartz dither inferred: pixel 26,14 differed mid-row). fail_values prints the pixels. Next:
  read the CG "largest ink difference" lines from CI; when green, ready + auto-merge. A local Skia build needs a
  download (not approved).
- #340 head c09e0426: gradient_coverage splits the gradient check (coordinator): coverage exact (inked inside the
  rounded box, background in a 2 px band), interior within gradient_dither=2, edge pixels within the interior's
  byte range. Stroke/shared/rounded checks exact. If CI prints "largest interior difference 0" for CoreGraphics,
  remove the tolerance. No Skia download (coordinator): CI is the Skia check.

## PAUSED 2026-10-25
1. **Now:** branch `claude/paint-edge-snapping` (#340), head c09e0426, pushed. The working tree in D:\gui_forms_w3\gui_forms is clean, with nothing uncommitted. I was waiting for CI on c09e0426.
2. **My open PRs and branches:**
   - #340 `claude/paint-edge-snapping` c09e0426: DRAFT, CI pending. Has the Skia device-space rounded fix, the CG gradient clip with AA off, and the gradient_coverage split.
   - #337 `claude/macos-unocclusion-damage` d66a921e (main synced server-side): READY with auto-merge queued; evidence in the PR description.
   - #311 `claude/state-machine-hunt-2`, #314 `claude/state-machine-gate`, #315 `claude/state-machine-c3`: ready, open (auto-merge set earlier; the coordinator merges).
   - #320 merged.
3. **Next, in order:**
   1. Read #340's CI result. Check the Skia and CoreGraphics scene tests, and read the CG "largest interior difference" lines from LastTest.log in the macOS evidence artifact.
   2. If CG shows 0, remove gradient_dither (then it was not dither). If it is greater than 0, keep it and record the value in the PR description.
   3. Fix any remaining painter failure in the painter, and update the golden list one at a time, with reasons.
   4. When green: `gh pr ready 340`, then `gh pr merge 340 --auto --merge`.
   5. Watch #337 land.
4. **Waiting:**
   - On the coordinator: #340's CI result (the coordinator wakes me) and the resume instruction.
   - On the owner: nothing open on my side. Skia download not approved; CI is the Skia check.

## RESUMED 2026-10-25 (after the reboot)
- Tree clean at c09e0426. #340 CI run 40253050874 (c09e0426) is QUEUED. The 09ba7bcb run 40252321661 was cancelled. Nothing local needed rerunning: the only background work was CI.
- Next: the steps in "PAUSED 2026-10-25" item 3, once the coordinator reports run 40253050874.
- #340 head fb3ccd33 (DRAFT, CI pending). Fixes after run 40253050874:
  - Skia: a tie at 42.5 rounded down through the float translation. SkiaRaster now keeps offset/saved_offsets in
    double; SnapTransform uses them when the matrix matches (else falls back to float).
  - CG: snapped_device_box rounds y from the top row (y-flip tie); aliased_clip_rect sits a quarter pixel inside
    (a round trip put the clip a hair outside at 1.25, so Quartz took the next row).
  - Dither per painter: CG quartz_gradient_dither = 1 (measured), Skia and DIB 0.
  - Next: read CI; when green, ready + auto-merge.
- #340 GREEN at fb3ccd33 (run 40258464746). Dither: CG 1 at every scale; Skia (macOS, Linux) and DIB 0. Recorded in
  the PR body. READY + auto-merge queued.
- NOW: macOS AX regression (metrics_view value one paint behind). Branch claude/macos-ax-value-refresh from main
  08828475 (uncommitted). Cause: MetricsView's semantic_descriptor read live window metrics, which change on every paint
  without advancing semantic_generation, while every host caches by generation. Fix: MetricsView publishes the text it
  last painted (painted_metrics_), and a paint that changes it invalidates Dirty::semantics (no paint owed).
  Docstring and generated rst updated. New assertions in basic_controls_tests. CI does run the client test (the
  runner is trusted: SDKLastTest shows metrics_view matched, Test Passed), so the bench difference is timing.
- NEXT after that: normalise the volatile values in the Gallery display trace (live control count, RuntimeIds), based
  on main after L3's one-time re-record.
- #350 `claude/macos-ax-value-refresh` 1c7a6d9e, DRAFT: the MetricsView painted-value fix. Waiting on the owner's M4
  run (coordinator routes it) and CI. Likely exposure: 2dd53556 (DockSite always publishes a live region, so the
  host republishes early) or c2cf549b (text ranges, same); bench bisect suggested in the PR body.
- NEXT: Gallery display-trace normalisation (live control count, RuntimeIds), from main after L3's re-record lands.

## Stage SK, universal Skia (owner decision 2026-10-26), my priority
- The brief PR is open (claude/sk-universal-skia-brief): planning/stages/SK-universal-skia/README.md plus an index row.
  Phases: SK1 build (Skia mandatory, Windows CI builds a native CLANG64 RGBA Skia, gate: steady-state wall time
  near 8.3 min); SK2 RGBA everywhere plus a per-host present conversion kernel (measured); SK3 the Windows host on
  SkiaRaster (DibPainter deleted, DP guarantees kept); SK4 CoreGraphicsRaster deleted; SK5 goldens re-recorded once,
  tolerances removed.
- Open: SK-Q1 public BGRA (recommend keeping it only at the GDI compatibility endpoint); SK-Q2 local Skia on D:
  trees (recommend the CI archive for every tree, a local build only for the tree that changes the Skia build).
  No downloads until approved.
- Swap-ready: the pinned fetch/build stays the only seam; use only the stx_skia REQUIREMENTS "Required" list.
- Next: SK1 from main. The Gallery trace normalisation waits on L3. #350 (AX) waits on the M4. #340 auto-merge queued.

## SK update (2026-10-26): BGRA correction and SK1a
- OWNER CORRECTION: "one pixel format everywhere, BGRA8 premultiplied sRGB" (replaces RGBA). Every present becomes a
  plain copy (X11 reorder goes; macOS CGImage premultiplied-first little-endian); Skia BGRA on all four platforms;
  native_live_surface_pixel_format() BGRA everywhere; present time per frame measured before/after on all 3 hosts.
  SK-Q1 settled: RGBA only as input, converted once at entry (PNG decode, PixelFormat::rgba32 / GD_PIXEL_RGBA32,
  LiveSurfacePixelFormat::rgba32); removed: FramebufferChannelOrder, engine RGBA8 storage. HostImage (straight RGBA8
  clipboard/notify-icon interchange) left unchanged as not-a-surface (flagged to the coordinator).
- #355 brief: correction pushed at 4aa57581 (worktree D:\gui_forms_w3\dev\wt_sk_brief, local branch sk-brief-local),
  PR body updated (dev/pr_bodies/sk_brief.md).
- SK-Q2 answered: CI's cached archive, digest-checked, no local Skia fetch/build. Before the FIRST local download, tell
  the coordinator the size and source (`skia_archive.py describe --platform windows-x64`, metadata only).
- SK1 split: SK1a = #356 `claude/sk1-skia-mandatory` c7196cad DRAFT, CI pending. tools/skia_archive.py
  (identity/pack/probe/publish/describe/fetch), tools/skia_archive_release.json (empty pins),
  tools/skia_archive_probe.cpp (asserts kN32 == BGRA, reads 0,0,255,255), tools/test_skia_archive.py (16 tests, in
  native.yml), native CLANG64 profile (args: cc clang, cxx clang++, ar llvm-ar, -fms-extensions, B0 G8 R16 A24 shifts;
  script: cygpath -m, gn.exe/ninja.exe, ccache cc_wrapper). Workflow: Windows step build+pack+probe before the link-map
  audit, artifact gui-forms-skia-windows-x64-<identity>, main-only publish step (until pinned; falls back to artifact).
  GUI.Forms still SKIA=OFF on Windows; ci_build.py untouched (local trees must never fetch Skia).
- Main risk: Skia's gcc_like GN toolchain on a Windows host (cmd.exe /c stamps, python3 ar.py). Read the
  skia-archive step log first when CI reports.
- SK1b next (after SK1a merges and a main push publishes): pin (+ packaging/pinned_cache_revisions.json), ci_build
  Windows fetches the pinned archive (CI and local), CMake WIN32 defines the 4 shifts + static_assert, Skia mandatory
  (FATAL_ERROR on OFF, remove OFF paths and TARGET guards), release.yml / state-machines.yml / prove_compiler_cache.py,
  install.md, Windows job times before/after (gate ~8.3 min steady state). SK2 adds the shifts to mac/linux profiles.
- SK1 baseline (before), last 40 successful native.yml runs, Windows build job: main push (promoted) 1:02-1:52; PR
  reuse/docs 3:38-5:05; PR code builds 8:15-24:35 (toolkit step 5:43-15:29). Data: D:\gui_forms_w3\dev\sk_windows_times.txt.
  Recorded in #356's body. Watch the 40-min timeout on #356's first (cold Skia) run.
- #356 now at da0b34af (coordinator: own job for the cold build). native.yml: skia-windows-x64-plan (ubuntu, `plan`:
  build only if identity neither pinned nor in an unexpired artifact) -> skia-windows-x64 (windows-2022, 60 min,
  toolchain restore, fetch+build+pack+probe, prints "Skia fetch N s, cold build N s", uploads the artifact) ->
  native-windows-x64 (needs all three; runs on success or skipped). native-platform fetches with --artifact-fallback
  into .build/native-windows-x64/skia-archive for the link-map audit. TODO when CI reports: put the cold build time
  (that log line + the job time) into #356's body ("Cold build time (first run): pending").

## HostImage and the CNG 1.0 intake (2026-10-26)

- HostImage (owner 2026-10-26): becomes BGRA8 premultiplied sRGB; hosts convert
  to the OS format only at the hand-over (Windows clipboard bitmap; PNG on
  macOS/Linux) and back on read. Put in SK2 with exact premultiplied clipboard
  round-trip tests per host. Brief updated on #355 at 73eae35d (and PR body).
- CNG 1.0 intake: queued after SK1a/#356, before SK2. Upstream
  falseywinchnet/stx_cng 9fde861451367796fa11da8686e47d164b6c8042, cloned (data
  only) at D:\gui_forms_w3\dev\cng_upstream. Upstream zstd tarball has the
  pinned SHA (eb33e51f...). Upstream read in full (codec, decoder, encoder,
  collection, tests, FORMAT.md, COLLECTIONS.md).
  Plan: rewrite stx_cng/ to 1.0 in house style (noexcept explicit results,
  reusable owners with static zstd contexts, std::thread fork-join with a
  serial fallback when a worker can't start, strict single zstd frame for the
  tables), collections under D3 (span members, caller output budget), tests
  (conformance vectors byte-identical, roundtrip, malformed, limits,
  collections, fuzz), FORMAT.md 1.0, PROVENANCE with SHA-256s; seam decodes
  BGRA straight into caller pixels (resource contract already says discard
  on failure); registry admission stays strict (all copies intact).
  Decision points to raise with the PR: D2 extent vs 1.0's 2^31 samples
  (recommend: package conforms, registry limits stay); zstd level 19 needs
  the BTULTRA strategies (enable; keep excluding the rest); threads option.
  No conflict with AGENTS.md codec rules or D3 found.
- 2026-10-26 progress: worktree moved to D:\gui_forms_w3\dev\ws_cng\gui_forms
  (workspace layout so ci_build.py works; text stack copied from the w3
  checkout), branch claude/cng-1.0-intake off main 3cc4d156, uncommitted.
  Done: library port (conformance byte-identical at 1 and 4 threads), tests
  core/malformed/limits/collection/conformance/fuzz harness (plain and
  STX_CNG_SANITIZE pass; GUI.Forms sanitize tree passes resource_pixels +
  stx_cng), installed consumer, seam (direct BGRA decode, in-place widen /
  premultiply / swap), resource_pixels fixture (BGR(A) swap, padded rows),
  fuzz-smoke -pthread, FORMAT.md verbatim, COLLECTIONS.md derived (Access
  replaced, D3), PROVENANCE (blob SHA-256s), README. Measurement corpus rebuilt
  at dev/cng_measure/corpus (248 of 257 by SHA; photos via System.Drawing
  from C:\Windows\Web, crops from the converted PNG, offsets rounded up;
  8 screenshots + 1 ppm not on disk); bench.cpp/summarize.py there.
  Next: finish bench, storage probe, HARDENING.md, results dir, commit, PR.
- 2026-10-26 DONE: PR #359 (claude/cng-1.0-intake, head 846ca324), ready,
  auto-merge (merge) set; CI pending. HARDENING.md rewritten for 1.0;
  results/2026-10-26-cng-1.0-intake (README, bench.tsv, inputs.tsv,
  storage.txt, scripts). Numbers: 232 images, CNG 0.764 x PNG6 bytes, 0.804 x
  PNG9, 0.912 x CNG 0.1; decode sum PNG 7.46 s, CNG 5.55 s (1 t), 2.05 s (4 t);
  icons slower (~8 us fixed per file) and 4 threads hurt multi-segment icons;
  encoder storage up to ~7.7 B/sample (209.8 MB on 3840x2400), decoder 552 KB
  (1 t) / 614 KB (4 t) / 1.85 MB (64 t). State-machine checker fix in
  tests/common.hpp (pattern switch returns per case, default with reason).
  Verified: standalone rel + san ctest 6/6, GUI.Forms sanitize (earlier, before
  the common.hpp switch rewrite; stx_cng tests reran standalone after it),
  house-style/lifetime/state-machines/docs checks clean.
  Raised with coordinator: D2 extent vs 2^31 (recommend package conforms,
  registry limits stay); BTULTRA enabled for level 19; seam 1 thread; seam
  failure = discard; strict admission rejects one damaged copy;
  planning/coordination/tracks/stx_cng.md still describes CSP v2.
  Next: #356 cold Skia build time, then SK1b; SK2 (HostImage + clipboard tests).
- 2026-10-26 OWNER: "then lets just use png. i'll work over another format."
  Coordinator switched off auto-merge on #359; it stays open pending the
  owner's close decision. DO NOT push to #359. SK2's BGRA work is PNG-only at
  the image seam (PNG decodes once into BGRA resources); no CNG-specific work.
  Now: #356 cold-build time, then SK1b, then SK2.
- 2026-10-26 #356 cold run (40274634795) failed in skia-windows-x64 after 74 s:
  "Skia PNG-only source patch does not apply cleanly" (Windows checkout gives
  CRLF patches; MSYS2 git checks Skia out LF). Fix fc0fc0d7: third_party/.gitattributes
  `*.patch text eol=lf`. CI pending; the cold build time is still to be recorded in
  #356's body from the "Skia fetch N s, cold build N s" line of the next run.
  SK1b waits for SK1a to merge and a main push to publish; SK2 after.
- 2026-10-26 CNG removal PR CANCELLED (owner: "nah, lets just not use it").
  Keep stx_cng and the seam's CNG path exactly as on main; AGENTS.md untouched.
  Do not use CNG for any asset or new work. The removal worktree/branch
  (claude/remove-cng, never pushed, no commits) was deleted. #359 stays as the
  coordinator left it (auto-merge off, do not push).
  Order: #356 cold-build time (fix fc0fc0d7 pushed, CI pending) -> SK1b -> SK2 (PNG only).
- 2026-10-26 CNG removal ON again (owner final: "we can remove it entirely. we have
  our own png and zlib after all"). Branch claude/remove-cng at 511b23be, pushed,
  NO PR yet: open it (base main, body D:\gui_forms_w3\dev\pr_bodies\remove_cng.md)
  only after SK1a (#356) merges, and before SK2. If main moves first, merge main
  locally only on conflict. Worktree D:\gui_forms_w3\dev\ws_rmcng\gui_forms (text
  stack moved there from ws_cng; ws_cng is obsolete). Removed stx_cng + CI +
  zstd pin, ImageResourceEncoding::cng, invalid_image_header/data, cng_pixels,
  registry CNG admission, CNG tests/docs/licences, AGENTS.md CNG lines
  (root-approved); planning direction lines (FORWARD_PROGRAMME, SK/T2/T3 briefs)
  updated, coordination track records left. 2026-10-22 record moved to
  results/2026-10-22-stx-cng-intake. Verified: sanitizer tree 7/7 image/registry
  tests, state-machine checker (no findings in changed files), house-style,
  lifetime, tool tests, docs check.
  LESSON: never `git add` in a shell that sourced toolchain_env.sh: MSYS2's git
  there has no core.autocrlf and stages CRLF (it staged 4,000 files once; reset).
- 2026-10-26 #356: cold build passed (run 40288102635: fetch 14 s, cold build
  203 s, 541 steps, job 4:44, artifact 3.2 MB); recorded in the body. Marked
  ready with auto-merge (merge); remaining CI pending.

## PAUSED before SK1b (2026-10-26)

Owner: "lets pause before skia." No SK1b, SK2 or further SK work, and no Skia
archive download, until the coordinator resumes it.

State:
- SK1a (#356) merged (2d33baac on main; the .gitattributes fix fc0fc0d7 is in it).
- CNG removal: PR #361 (claude/remove-cng, head 511b23be, base main), opened,
  auto-merge (merge) set, CI pending. Not Skia work; the coordinator merges.
- Main push run 40293459248 (at 2d33baac) was still queued when the pause came;
  it should publish the Windows Skia archive into release build-2d33baac...
  Not checked; my waiter was stopped. Nothing downloaded locally.
- SK1b worktree D:\gui_forms_w3\dev\ws_sk1b\gui_forms, branch
  claude/sk1b-skia-pinned off 2d33baac: clean, no edits, never pushed.

Next steps on resume (in order):
1. Confirm run 40293459248 (or a later main push) published the
   skia-windows-x64 archive: `gh release view build-<sha> --json assets`.
2. Pin it in tools/skia_archive_release.json:
   platforms.windows-x64 = {tag, identity, archive_sha256}. Take the identity
   from `skia_archive.py identity --platform windows-x64` and archive_sha256
   from the release manifest (no archive download). Add the revision to
   packaging/pinned_cache_revisions.json.
3. Run `skia_archive.py describe --platform windows-x64`, then tell the
   coordinator the size and source BEFORE the first local download.
4. SK1b edits:
   - ci_build.py: Windows fetches the pinned archive (`fetch --into`, outside
     the repo) and configures with -DGUI_FORMS_SKIA_PREBUILT=ON plus the
     archive paths.
   - CMake WIN32: the four shift defines (B0 G8 R16 A24) plus a static_assert
     that kN32 == BGRA.
   - Skia mandatory: FATAL_ERROR naming third_party/fetch_skia_cpu.sh; drop the
     OFF paths and the `if(TARGET gui_forms_skia)` guards.
   - Flip Windows SKIA=OFF to ON in ci_build.py:484, release.yml:285,
     state-machines.yml:98, prove_compiler_cache.py:65,
     run_sanitizer_suite.sh:50 and run_facade_surface_smoke.sh:45.
   - Update the native-platform.yml fetch-step comment (it says SK1b is
     pending). Update docs/user/install.md.
   - The Windows host links Skia but keeps DibPainter until SK3.
   - Gates: the link-map audit (--forbid-foreign) passes; Windows job times
     before/after (baseline in dev/sk_windows_times.txt; gate about 8.3 min
     steady state).
5. SK2 (PNG only): BGRA8 premultiplied everywhere, HostImage plus per-host
   clipboard round-trip tests, and the shifts in the mac/linux profiles.

## PAUSED for the night (2026-10-26)

SK stays paused (see "PAUSED before SK1b"). Task in progress: the org-move PR
("Org move: repository references and merge-queue triggers", base main;
repository now malkuth-software/gui_forms).

- Worktree D:\gui_forms_w3\dev\ws_org\gui_forms, branch claude/org-move off
  origin/main ca60b8ee. NOTHING committed or pushed. No local builds or runs
  were in flight.
- Uncommitted, applied, not yet verified:
  - repository references in README.md, docs/building/reusing-ci-caches.md,
    docs/conf.py, docs/index.rst, docs/release/index.md,
    packaging/programs.json, planning/coordination/BRIEF.md,
    planning/stages/README.md, tools/cache_manifest.py,
    tools/skia_archive.py and tools/windows_toolchain.py;
  - workflows, via dev/edit_scripts/org_workflows.py:
    - native.yml: merge_group trigger; Sanitizers CHANGED_FROM falls back to
      merge_group.base_sha; no apt-cache save on merge_group;
    - native-platform.yml: no apt or object cache save on merge_group;
    - release.yml: no apt or object cache save on merge_group; NARROW
      includes merge_group;
    - state-machines.yml: merge_group trigger; BASE falls back to
      merge_group.base_sha.
- Written, NOT yet run: D:\gui_forms_w3\dev\edit_scripts\org_tools.py.
  Run it with python -B org_tools.py <worktree>. It changes:
  - ci_tree_receipt.py and ci_reuse.py: trust merge_group runs;
  - ci_sanitizer_seed.py: a merge group seeds from the queued PR's head
    branch, parsed from refs/heads/gh-readonly-queue/<base>/pr-N-<sha>;
  - release_tests.py: `merged` carries the merge-group run whose head_sha is
    main's commit first, then the existing PR path;
  - tests for all of the above in test_release_tests, test_ci_tree_receipt,
    test_ci_reuse and test_ci_sanitizer_seed.
- Findings so far:
  - The repository string is not an identity input anywhere. The Skia
    identity hashes only third_party and toolchain inputs; the Windows
    toolchain identity is spec_sha256; cache_manifest's field is
    informational. So there is no cold rebuild.
  - Nothing compares a stored repository: the run checks compare the API's
    full_name with GITHUB_REPOSITORY, and GitHub reports the current name.
  - planning/coordination/README.md has no gui_forms references; its only
    hit is falseywinchnet/EpistemicGPT.
  - Left alone as historical records: BUILD_SIZE_PLAN_2026-10-23.md,
    coordination/tracks/build.md, stx_vorbis/docs, results/ and
    experiments/.
  - Publishing (publish_cache, the Skia publish, the release publish via
    publication_version, cache-prune) is gated on event == push on main,
    so it does not fire on merge_group.
- Next steps:
  1. Run org_tools.py.
  2. Run the tool tests: test_ci_tree_receipt, test_ci_reuse,
     test_ci_sanitizer_seed, test_release_tests, test_publish_cache,
     test_skia_archive, test_windows_toolchain, test_ci_promote and
     test_publication_version.
  3. Run generate_library_docs --check and house style over tools.
  4. In a fresh shell (never after sourcing toolchain_env.sh): git add,
     check `git diff --cached --stat`, then commit and push.
  5. Open the PR on auto-merge and report.

## RESUMED 2026-10-27: org-move PR open
- #368 `claude/org-move`, head f18f6950, rebased onto main 2fe26d8d; ready, auto-merge (merge) set, CI pending.
  Once it merges, the coordinator turns on the merge queue. Worktree D:\gui_forms_w3\dev\ws_org\gui_forms; body
  dev/pr_bodies/org_move.md.
  Verified: 10 tool test suites plus test_house_style, house style and lifetime over tools, docs check.
- LESSON: in Git Bash, use `git cat-file -p <blob>` to check committed line endings. `git show HEAD:path` and a
  grep loop both misled.
- SK stays paused (see "PAUSED before SK1b").
- Next, from the notes:
  1. #350 (macOS AX MetricsView painted value) is waiting on the owner's M4 run.
  2. Gallery display-trace normalisation (live control count, RuntimeIds) waits on L3's re-record landing on main.
  3. SK1b when the coordinator lifts the pause.

## Consumer cache pins (2026-10-27)
- Branch `claude/pin-consumer-caches` f2e38aa2 (worktree D:\gui_forms_w3\dev\ws_pins\gui_forms), PUSHED with NO PR:
  the coordinator wants the PR opened after #368 merges. It adds SwiftEdit 4e126d6eb3ab… (ci/dependencies.json) and
  File Manager c90eacf366… (dependencies.lock.json gui_forms, plus the submodule) to
  packaging/pinned_cache_revisions.json. test_prune_cache_releases and test_windows_toolchain pass.
- Both build-<rev> releases are ALREADY GONE (gh release view: not found). Republishing is not viable:
  - native.yml at 4e126d6 publishes only when ref == main, and a dispatch on main builds main's head;
  - so a republish would be a hand-made release from a side-branch run;
  - and the ccache archive is not reproducible (run_id is in the manifest; the objects differ), so the bytes would
    not match SwiftEdit's pinned cache_sha256 (File Manager pins sha256 too).

  Either way the consumer must change its lock, so moving the pin to a current revision that has a release costs
  the same. Recommendation: they stay uncached until the consumers move their pins; pin the new revisions here
  when they do.
- The stx_rtf2md pin waits on RT3.
- NEXT: when #368 merges, open the PR from claude/pin-consumer-caches (base main) on auto-merge.

## Docs publish after CI (2026-10-27)
- Branch `claude/docs-after-ci` b97162eb (worktree D:\gui_forms_w3\dev\ws_rtd\gui_forms), PUSHED, NO PR: open it
  after #368 merges (coordinator), on auto-merge.
- Contents:
  - .github/workflows/docs-publish.yml: workflow_run on both "GUI.Forms source and compiler cache" and
    "GUI.Forms programs and release", completed, branches [main, 'v*'];
    - job `latest`: native succeeded, push, main;
    - job `stable`: release succeeded, push, v* tag;
    - permissions contents: read;
  - tools/readthedocs_build.py: POST API v3, the token only in the header, logs status and build id, clear failure
    when the secret is missing; tools/test_readthedocs_build.py (6 tests, in native.yml house-style);
  - docs/release/index.md "Documentation site"; docs/README.md line.
- To confirm when it first runs:
  1. That the workflow_run branches filter matches a tag run's head_branch (the tag name).
  2. That RTD moves `stable` to the newest tag when the `latest` build syncs tags. Releases from main pushes create
     tags through the API, which starts no workflow.
- When #368 merges, open both: claude/pin-consumer-caches and claude/docs-after-ci (each base main, auto-merge).
- 2026-10-27: owner has set READTHEDOCS_TOKEN and switched RTD auto builds off. After docs-after-ci merges, the first
  successful main push run is the live test: read the docs-publish `latest` job log and report the build id.

## Merge queue live (2026-10-27)
- #368 merged as b90038d9. Merge queue ON (ruleset 25902268, ALLGREEN, 5 building, 90 min, MERGE).
- Opened #372 (claude/docs-after-ci 3a681796) and #373 (claude/pin-consumer-caches 064cf396), both with auto-merge,
  which enters the queue.
- Watching the first merge_group runs: background script D:\gui_forms_w3\dev\watch_merge_group.sh writes
  D:\gui_forms_w3\dev\merge_group_watch.txt (job names and conclusions). Confirm every required check reports on
  merge_group and nothing publishes (no build-<sha> release or v* tag from a queue branch; cache-prune and the
  publish jobs skipped). If a required check never reports, tell the coordinator AT ONCE.
- LESSON: a stray `git checkout origin/main --` detached ws_pins; always check the branch before rebasing.

## LAND AND FREEZE (2026-10-27, D:\gui_forms_workers\LAND_AND_FREEZE.md)
- In queue, with auto-merge: #372 docs-after-ci, #373 pin-consumer-caches (position 4; a queued branch refuses
  pushes), #383 textbox-migration-note (0.2 Consumer migration section in RELEASE_NOTES.md).
- #384 claude/pin-file-manager-2fe26d8 (44cc6c2f) is STACKED on #373: retarget it to main when #373 merges, then
  set auto-merge.
- PARKED (no PR, deferred until after the branch cleanup):
  - claude/consumer-contract 2a8e6ada: consumer-tooling.md and test_consumer_tooling.py; verified, 6 tests OK.
  - ci_reuse/ci_promote parameterization: not started.
- #350 is in the merge queue (the coordinator marked it ready; no longer excepted). SK paused.
- Watching the first merge-group native run 40375996854 (pr-358): house-style, docs and skia plan succeeded on
  merge_group; state-machine-gate succeeded on 3 groups. The run is slow because of the runner backlog; the queue
  timeout is 90 min (group created 18:14Z).
- Run 40375996854 (pr-358 group): Sanitizers succeeded on merge_group (reported after the interim handback).
- First merge group (pr-358, run 40375996854):
  - house-style, docs, Sanitizers, the 4 native builds and state-machine-gate all succeeded on merge_group;
  - every publish step was skipped; #358 merged as ccfd73d1;
  - BUT engine-gate reported SKIPPED (counted as passing).
- DEFECT (mine, SK1a #356): the job-level if on skia-windows-x64 made every steady-state run skip it. That skipped
  programs-windows-x64 and publish (implicit success()) and engine-gate (its 'skipped' guard). Since 2d33baac: no
  Windows programs tests, no vX.Y.Z release, no engine gate (data: the last 40 successful runs).
  - Fix: #386 claude/fix-windows-programs-skip 18a38b9d, auto-merge set. The Skia job always runs (an echo step on
    Ubuntu when there is nothing to build); native-windows-x64 requires its success; programs-windows-x64 and
    publish name their needs.
  - Check on #386's own run that engine-gate and programs-windows-x64 run.
- Open: #372, #373 (queue), #383, #386 (auto-merge); #384 stacked on #373.
- 2026-10-27: #372 MERGED. #350 in the queue (position 6); it lands with the landing.
- 2026-10-27: the coordinator's script retargets #384 to main and queues it when #373 merges. I'm IDLE until woken for a failure on one of my PRs or the first docs-publish build id.

## QUEUED for after the checkpoint (owner-approved; first item; do NOT start before the freeze completes)
State-machine gate hardening (found by L: #387's log shows 8 findings and still passes). Separate small PRs:
1. Make the gate fail on findings: pipefail plus propagating the checker's exit status in state-machines.yml (it
   pipes through `tee`). Audit every `| tee` step in every workflow for the same hole.
2. A guard so no required check passes by being skipped: each required job fails explicitly when a job it depends on
   was skipped unexpectedly (engine-gate, fixed by #386, was the other instance).
3. Clear the findings already on main (defaults with stated reasons, or justified exemptions) so the gate starts at
   zero, including tests/windows_compositor_tests.cpp:321 (F1, Windows-only, not compiled in the Linux gate tree).
   Answer: should the gate also scan Windows-only files?
Report the plan when the coordinator wakes me after the freeze.
- UPDATE: DROP item 1; #396 (codex/house-style-state-machine-gate, the owner's outside Codex session) replaces it:
  pipefail, a regression test for the pipeline exit status, and a full-tree check whenever the gate workflow changes.
  Do NOT push to #396. Items 2 (skipped-dependency guard) and 3 (clear the existing findings) stay. If #396's
  full-tree run fails on existing findings, item 3 blocks #396 and comes first after the freeze.

## FROZEN (2026-10-27): checkpoint `checkpoint-2026-10-27` on main 14164ee5
- Landed from this track: #350, #356 (SK1a), #361, #368 (org move and merge-queue triggers), #372 (docs after
  CI), #373 and #384 (consumer cache pins including 2fe26d8d), #383 (TextBox migration note), #386 (fix for the
  CI jobs skipped behind the Skia job).
- Parked: claude/consumer-contract 2a8e6ada (consumer-tooling.md and its test) -> plan phase D, together with
  the ci_reuse/ci_promote parameterisation (not started).
- SK stays PAUSED before SK1b (see that section). Gate items 2 and 3 (above) wait for plan phase E.
- Still owed: the first docs-publish Read the Docs build id, when the coordinator wakes me.

## Phase B (PLAN_2026-10-27.md): the status docs PR
- #403 claude/roadmap-to-0.5 42ece8b9, auto-merge (MERGE) set; worktree D:\gui_forms_w3\dev\ws_roadmap.
  Updates docs/dev/roadmap.md, the stage index and "Where to pick up" in planning/stages/README.md, the state
  paragraph and F status in planning/FORWARD_PROGRAMME.md; adds planning/ROADMAP_TO_0.5.md (indexed in
  planning/README.md). Ran generate_library_docs.py --check (PASS) and house style (0); Sphinx is not installed
  locally, so the docs CI job is the -W check.
- Not done (outside my files): the claude.ai stage board artifact linked from AGENTS.md still shows the old
  statuses; the coordinator owns it.
- Next: idle until phase D/E or a failure on #403.

## Phase C4 (owner-approved 2026-10-27): move commentary to malkuth-software/notes
- Branch claude/move-commentary-to-notes, worktree D:\gui_forms_w3\dev\ws_notes, commit 707bc9e6 (off main 41214d74).
  Workspace checkout /d/gui_forms_w3/gui_forms is DETACHED at that commit for build.sh (it was on
  claude/sk1-skia-mandatory, clean; return it there afterwards).
- Verified first: the notes gui_forms branch holds every file of the five folders at main 41214d74 (one CSV differs
  only in CRLF).
- Kept (a): engine-gate template -> tools/engine_gate_record.template.md; PNG benchmark baseline ->
  tests/image/png_benchmark_baseline_decoder.cpp.txt; ADRs inlined into docs/dev/adr/. font-substitution.json evidence
  strings kept (compiled in; now relative to the notes root, header comment says so).
- Convention: code comments cite notes:<path> (defined in AGENTS.md "Forward programme" and docs/dev/evidence.md).
- Pre-existing local failure, also on main: test_state_machines test_workflow_keeps_checker_failure_through_tee
  (49 != 1) on Windows.
- Next: build.sh all (running), sanitizer over touched tests, push, PR, auto-merge, report.
- C4 DONE: #404 claude/move-commentary-to-notes 707bc9e6, auto-merge MERGE set. Full build OK; whole suite 266/267
  (windows_compositor_tests failed once under -j 3 full-suite load, passed alone and in 3 parallel window-test runs;
  it reads real screen pixels, so another test's window may cover the sample point: reported to the coordinator,
  proposed fix a RESOURCE_LOCK for screen-reading tests). Sanitizer over touched tests 35/35. Workspace checkout
  returned to claude/sk1-skia-mandatory. Idle until woken.
- #404 MERGED (502d1e52). Owner accepted all four recommendations.
- Follow-up #405 claude/adr-links-to-notes 911ae6db, auto-merge MERGE: docs/dev/adr removed, decisions.md links the
  notes decisions/ records (verified identical to the checkpoint originals), old adr pages forward to dev/decisions
  via MOVED_PAGES, PROVING_SLICE.md removed (identical copy at notes gui_forms root).
- QUEUED for phase E (do NOT start before): (1) flaky gui_forms_windows_compositor_tests under full-suite load:
  RESOURCE_LOCK or RUN_SERIAL on the screen-reading tests; (2) test_workflow_keeps_checker_failure_through_tee
  returns 49 locally: this machine's bash or a real defect? Also gate items 2 and 3 (above).
- IDLE until woken (still owed: the first docs-publish RTD build id).
- Standby task DONE (2026-10-27): read-only audit of unpushed local branches in w1-w4 ->
  D:/gui_forms_workers/BRANCH_AUDIT_2026-10-27.md. 37 branches: 36 LANDED, 1 SET ASIDE (cng-1.0-intake), none needs the
  owner. Nothing deleted. Idle.

## Release SDK (owner-approved 2026-10-27): #406
Branch claude/release-sdk, head 6eddddc2, off main e13a02a4; worktree D:\gui_forms_w3\dev\ws_sdk.
Auto-merge set (merge queue); CI pending. Not waiting.
- New tools/package_sdk.py and test_package_sdk.py (19 tests).
- release.yml: a "Package the SDK" step in the programs job, plus the test in the prepare job.
- publish_tested_release.py: expects, checks and sums one SDK per receipt platform, and adds an SDK table to the notes.
- Docs: install.md "The release SDK archives", plus one line each in download.md and RELEASE_NOTES.
- Assets: gui-forms-sdk-<v>-{macos-arm64,linux-x64,linux-arm64}.tar.gz and gui-forms-sdk-<v>-windows-x64.zip.
- Top directory gui-forms-sdk-<v>-<platform>/. It adds cmake/llvm22.cmake, and on macOS llvm-runtime/.
- Windows size: 33.8 MB zip (88.5 MB unpacked), measured from the int tree 57eb3bd4. I deleted the install_manifest.txt the measurement left there.
- Decision in force: the SDK comes from release.yml's programs job, not the native jobs, because promoted main runs skip the build.
- Note: /d/gui_forms_w3's own build tree is out of sync with its checkout (configured from a later main; the checkout is sk1). Reconfigure before using it.
Next: idle. Still owed: the first RTD build id when woken. Phase D and E queues as above.

## Source SDK (owner redirect 2026-10-27): branch claude/source-sdk
Worktree D:\gui_forms_w3\dev\ws_src, off main ab67c008 (#406 merge). Commit 3cebe5d8. Not pushed yet at the time of writing.
- One asset: gui-forms-sdk-<v>-source.tar.gz, top directory gui-forms-sdk-<v>/.
- package_sdk.py is rewritten (git archive plus vendored pins plus SDK_MANIFEST.json). The binary path is removed.
- Vendored: third_party/harfbuzz (test/ and perf/ excluded), third_party/freetype, third_party/libunibreak, and third_party/miniaudio (Audio.cmake now prefers it).
- tools/build_sdk.py: --target (host only), --prefix, --jobs, --work.
- ci_build.py refactor: prepare_host, host_options and toolkit_options. test_build_sdk checks for drift against build_toolkit.
- release.yml: a "Package the source SDK" step on the linux-x64 leg only.
- Size: 56.5 MB local (5,155 files; release tree 47 MB of it).
- Skia source is not vendored: 176 MB in 12,278 files, plus 11 MB for its FreeType.
- Offline Windows build test: unpacked in D:\gui_forms_w3\dev\sdk_offline, with dead proxies and GIT_ALLOW_PROTOCOL=file.
- PR #407 is pushed (head 3cebe5d8), with auto-merge set; CI pending. Not waiting on it.
- Offline Windows build from the unpacked archive: configure, all 541 build steps and install all passed (rc 0). No .git and no _deps/ were created.
- Next: idle. Still owed: the first RTD build id. Open for the owner: whether to vendor the Skia source (176 MB unpacked).

## PAUSED for the night (2026-10-27)
- #406 and #407 are merged; v0.2.1308 carries the source SDK. No background work is running.
- Still owed: the first docs-publish Read the Docs build id. Not yet obtained; fetch it when woken.
- Reconfigure first: the w3 build tree (D:\gui_forms_w3\.build\native-windows-x64\gui-forms) was configured from a later main than the checkout. The checkout /d/gui_forms_w3/gui_forms is on claude/sk1-skia-mandatory at fc0fc0d7. Reconfigure, or delete and rebuild, before using it.
- Worktrees: D:\gui_forms_w3\dev\ws_sdk (claude/release-sdk, merged) and D:\gui_forms_w3\dev\ws_src (claude/source-sdk, merged). Both can be removed.
- Offline test tree: D:\gui_forms_w3\dev\sdk_offline (unpacked archive, build and prefix). Disposable.
- Open for the owner: whether to vendor the Skia source (176 MB unpacked, plus 11 MB for its FreeType).
- Phase E queue is unchanged.

## SDK features (2026-10-27): PR #412, branch claude/sdk-features, head 786c509d (auto-merge; CI pending)
Worktree D:\gui_forms_w3\dev\ws_feat (template_harness clone at D:\gui_forms_w3\dev\template_harness, pinned bb287401).
- TextMasks and AudioLoopTransport now install and export as components.
- build_sdk.py --features default|all; all adds TEXT_MASKS and AUDIO_LOOP_TRANSPORT.
- verify_install.py --feature-components builds tests/installed_feature_consumer.
- Local proof: feat_prefix and feat_build under D:\gui_forms_w3\dev (both disposable).
- Pre-existing issues noted in the PR:
  - paint_contract on Windows doesn't copy the Threading DLL.
  - test_ci_build's build-record test fails in a worktree not named gui_forms.
Next: idle. RTD build id still owed.

## Install fixes and the RTD id (2026-10-27)
- PR #414, branch claude/install-fixes, auto-merge:
  - paint_contract stages $<TARGET_RUNTIME_DLLS>;
  - test_ci_build's build-record test patches load_rules.
  Worktree D:\gui_forms_w3\dev\ws_fix.
- #412 is accepted by the owner and waiting on auto-merge.
- RTD build id (owed, now delivered): the first successful docs-publish run, 40404324971 (2026-10-26 21:26Z), started Read the Docs build 35425981. The latest at the time of writing is run 40567496674, build 35451955.
- template_harness clone at D:\gui_forms_w3\dev\template_harness, now at the 4437b47 pin.
Next: idle.

## SDK text stack export (2026-10-27)
- #415 claude/sdk-text-stack fb52fe02, auto-merge set, CI pending. Worktree: D:\gui_forms_w3\dev\ws_feat (switched from the merged claude/sdk-features).
- GUI_FORMS_INSTALL_TEXT_STACK (default OFF; in build_sdk FEATURE_SETS["all"]) -> cmake/TextStackExport.cmake installs libfreetype.a/libharfbuzz.a,
  include/freetype2 + include/harfbuzz, export GUIFormsTextStackTargets (GUIForms::FreeType, GUIForms::HarfBuzz -> FreeType).
- Decision in force: SKIP_INSTALL_ALL set around both add_subdirectory calls (else harfbuzzConfig export errors: freetype in two export sets);
  so upstream hb-features.h is not generated or installed (nothing includes it).
- Config components gain FreeType, HarfBuzz; verify_install FEATURE_COMPONENTS too; consumer test text_stack.cpp shapes "GUI.Forms" with Arimo.
- Local proof: text_prefix + feat_build under D:\gui_forms_w3\dev (disposable); logs text_build.log, text_verify.log.
- Next: idle until woken.

## House-style importance order (2026-10-27)
- notes main c95b3da (direct commit; notes has no CI): HOUSE_STYLE.md = variant A (most important first) + an order line under the title.
- gui_forms #416 claude/house-style-order 17746eac, auto-merge, CI pending: AGENTS.md summary reordered; 47 "house style section N/§N" refs renumbered
  (old->new 1->3 2->9 3->8 4->6 5->1 6->4 7->7 8->5 9->2 10->11 11->10). Script: D:\gui_forms_w3\dev\hs_scan\renumber.py <root> [--write].
- template_harness #3 merged at once (8819bb8a; no branch protection, so --auto merged it immediately). gui_forms' harness.lock still pins 523c2a7.
- Not changed (reported to the coordinator): 46 refs on the notes project branches (gui_forms, file_manager) in hs_scan/notes_hits.txt; EpistemicGPT copy.
- planning/PROGRAMMING_HOUSE_STYLE.md is gone from gui_forms (707bc9e6). EpistemicGPT's programming-house-style.md = old notes text minus the returned-storage addendum bullet.
- Owner decisions applied (2026-10-27): notes gui_forms fdf1d8e (living docs renumbered, 7 dated records got the pre-2026-10-27 note line,
  stages README table renumbered + points at HOUSE_STYLE.md on main), notes file_manager ca6345c, notes main 0ac2782 (README: canonical).
  gui_forms #417 claude/house-style-canonical (stacked on #416), auto-merge: AGENTS.md + docs/dev/style.md name notes main canonical.
- template_harness has no branch protection: merge there only with `gh pr merge --merge` after green, never --auto.

## House-style rewording study (2026-10-27, research only)
- Deliverable: D:\gui_forms_workers\HOUSE_STYLE_REWORDING_STUDY.md (six misses from the A/B review: intent with sources, NASA analogue, why the wording failed, proposed wording and section, open questions).
- Nothing changed in any repository. Read-only notes worktree D:\gui_forms_w3\dev\hs_scan\ng (notes gui_forms @fdf1d8e) removed after the study.
- Reported to the coordinator: DockLayout next_id wrap defect (dock_layout_persistence.cpp:400-430, dock_layout_edits.cpp:157-160); wake_ready/set_dispatch_wake_handler don't state their locking; C3 could match a status enum beside an optional.
- NASA PDFs (JPL D-60411, NASA-GB-8719.13) were cached incidentally by the fetch tool under the session's tool-results folder; not opened.
- Next: idle until woken.

## Study follow-ups (owner decisions 2026-10-27)
- gui_forms #419 claude/dock-identity-limit d5a27958, auto-merge, CI pending: dock_identity_limit 0xFFFFFFFE; restore rejects next_id > limit
  and IDs >= limit; new_node_id/add_stack/attach_beside return DockRefusal (identities_exhausted); invariants hold next_id in 1..limit;
  test_identity_limit. Ran dock tests (6) release + ASan/UBSan (ci_build.py --sanitize --tests dock). Branch lives in /d/gui_forms_w3/gui_forms.
  Pinned template_harness worktree now at /d/gui_forms_w3/template_harness (523c2a7; ci_build needs it beside the checkout).
- gui_forms #418 claude/wake-lock-contract 74f8c20b, auto-merge: wake_ready + set_dispatch_wake_handler lock contracts (worktree dev/ws_wake).
- notes PR #1 claude/house-style-rewording b28bc18 (DO NOT MERGE; owner reviews). Six wordings + addendum bullet; body lists copies.
  EpistemicGPT patch: D:/gui_forms_workers/house_style_rewording.patch (against 900285f0, verified to apply). Nothing pushed there.
  Stale notes:gui_forms planning/PROGRAMMING_HOUSE_STYLE.md NOT deleted: living docs still cite it (coordination BRIEF/README, COMPILER_CACHE, stage READMEs).
- C3 status-beside-payload prototype (scratchpad c3_proto.py, c3_factories.py; not a PR): S1 (status + std::optional, plain data records)
  7 findings, 6 real, 1 false (stb_vorbis C struct); DockDragOutcome real but the governing member is `end`, not `result`.
  S2 (status + any payload, *Result/*Outcome, outside tests) 44 records, ~31 status-gated; only 1 of 51 uses named factories.
- Next: Phase E (D:/gui_forms_workers/PHASE_E_DESIGN.md): E1 #401 integrity checker rebase+triage+guard classes; E2 loop-branch classifier; stop after E2.

## Phase E (2026-10-27)
- E1 done: gui_forms #420 claude/code-integrity-checker eff4901f (auto-merge, CI pending), supersedes #401 (force-push to codex/ branch was
  denied by the permission system; coordinator to close #401). Worktree dev/ws_integrity.
  Guards: checked_arithmetic, dominating_validation, named_failure; `trap` reserved (owner + stx_skia list pending; coordinator asking).
  reviews.json (17 decisions) + ignored scopes src/render/skia/, src/abi/drawing_skia_c_api.cpp. Triage table in code_integrity_checker/reports/README.md:
  defect 5 (4 Skia-ignored; GUI.Forms: windows_host.cpp:3915 trace %.*s), guarded 24, false shape 16, trap? 13. Gate: 0 raised.
- C3 status-beside-payload: HOLD (owner scope decision).
- Low priority after E2: notes PR repointing living docs off notes:gui_forms planning/PROGRAMMING_HOUSE_STYLE.md, then delete it.
- Next: E2 loop-branch classifier in template_harness (projects/<product>.toml config; clang -ast-dump=json over compile_commands.json);
  report first-run counts on gui_forms before opening a PR; stop after E2.

## Decisions in force (2026-10-27, later)
- Trap class dropped (Skia's automatic traps were not real violations). E1 has no trap guard and no trap? marks; each candidate is
  triaged on its merits; a non-violation is false_shape with a one-line reason. No work on Skia at all (vendored Skia, headers,
  preserved pre-fix headers): Skia paths are excluded from the integrity checker's scope and from the triage.
- C3: narrow form now (status member beside std::optional, plain data records) in template_harness check_state_machines.py with a test;
  stb_vorbis C-compat struct excluded as reviewed. Broad form and migrating the 44 Result/Outcome records wait for a later owner decision.
- House form for results and states (owner): a std::variant inside (each alternative carries its own data), named factories to build,
  named queries to read; std::holds_alternative and std::get_if allowed; std::visit with a lambda stays out. notes PR #1 a3636ee carries
  it (still DO NOT MERGE); EpistemicGPT patch regenerated (base 900285f0 == EGPT main 75bea98 for that file; verified to apply;
  generator scratchpad egpt_apply.py). The 6 C3 gui_forms fixes use this form.
- E1 updated: #420 claude/code-integrity-checker 0cbfda7d (auto-merge, CI pending). Trap class gone; reviews.json excluded_scopes (Skia) not read;
  39 non-Skia triaged: 1 defect (windows_host.cpp:3915 trace %.*s, StableId has no length limit; not fixed), 23 guarded, 15 false shape.
  Run: 989 files (11 excluded), 31 candidates: 0 raised, 14 guarded, 17 reviewed. 34 checker tests. Generators: scratchpad ci_reviews.py,
  ci_triage_table.py. Next: C3 harness PR, then the 6 gui_forms records (std::variant form), then E2.
- C3 harness: template_harness #4 claude/c3-status-beside-optional 7c7deef (merge with `gh pr merge --merge` after green; never --auto).
  Finds the 5 narrow records; state_machines.reviewed_shapes (new, project file) keeps ResolvedTextLayout.
  DockDragOutcome governing member = `end` (coordinator agreed).
- BROAD-FORM QUEUE (owner decision pending): the 44 Result/Outcome records (S2: status beside any payload), plus ResolvedTextLayout
  (design_metrics follows the measuring engine, not the status grade). tests/text_block_tests.cpp gives an estimated layout design
  metrics, a state production never builds; the broad decision should settle that test too.
- C3 gui_forms PR: branch claude/c3-status-variants in /d/gui_forms_w3/gui_forms (the build checkout), 5 records in the std::variant form.
- C3 records: owner wants them done PROCEDURALLY (encapsulate by codemod over clang AST ranges, make fields private, then switch the
  inside to std::variant in one place). Manual conversion stopped before any edit landed (branch claude/c3-status-variants has no
  commits; unapplied hand script scratchpad c3_docview.py). STUDY first, report before building: sites per record and form, mechanical
  share, tooling, effort units; plus E6 (exception surface): throw/try retired, functions that could become noexcept; house form needs
  noexcept moves and get_if/holds_alternative reads only. Study tools (scratchpad): ast_reader.py (streams -ast-dump=json, parses only
  first-party top-level decls), c3_study.py, c3_study_report.py, nothrow_probe.cpp. E2 waits for the study (shared AST reader).
- C3 STUDY REPORTED (SendMessage to main): narrow five 165/194 sites mechanical (85%; 93% without DockDragOutcome, whose 17 writes in
  end_drag + window-move drop need a hand restructure); broad samples 130/141 (92%). E6: 0 throws, 2 try (bad_alloc boundaries, kept),
  0 optional::value(); no new noexcept beyond factories/queries; all alternatives nothrow-movable (PartRaster via shared_ptr).
  Reader: streaming -ast-dump=json, 615 MB/unit, 48 units 1727 s at 4 threads (GIL); process pool + include prefilter next.
  libclang.dll + clang.cindex exist in the pinned clang64 as an alternative. Next: E2 (builds the reader in template_harness) unless redirected.

## Reader and codemod order (coordinator 2026-10-27, latest)
- Order: (1) shared AST reader in template_harness, (2) codemod + narrow five gui_forms PR (+ E6 note in PHASE_E_DESIGN.md: no
  exception gain on these eight records, as measured), (3) rest of E2: report gui_forms first-run counts to main BEFORE the PR, then stop.
- harness #5 claude/bootstrap-linux-display MERGED (xvfb-run for Linux Full bootstrap / Bootstrap again).
- harness #4 claude/c3-status-beside-optional head cfbe9acd after update-branch; merge myself with `gh pr merge 4 --merge` on all green.
- harness #6 claude/ast-reader 384609f (worktree D:\gui_forms_w3\dev\th_reader): harness/clang_ast.py (+ _json, _libclang, test_clang_ast.py,
  23 tests, parity). python3-clang-22 installed ONLY in harness-state-machines.yml (owner approval, CI only); apt-archives cache restore/save
  added there as in harness-native.yml; "~++" debs not kept (existing policy). Self-test only when harness_path == product_path.
  Merge myself with `gh pr merge 6 --merge` on all green (no branch protection; never --auto).
- libclang on Windows needs -ccc-install-dir <compiler bin> and -resource-dir (done inside parse_arguments).
- The old E2 run (bvqu2ne2z, old buggy splitter) was stopped; its outputs are void. E2 (dev/template_harness claude/loop-branches,
  uncommitted) must be rebased on #6 and ported to clang_ast.Node roles.
- Next: codemod (reader -> site classifier -> factory/query generator -> binary byte-range rewriter refusing overlaps -> privatise),
  in template_harness or gui_forms tools? -> lives beside the reader in template_harness (shared); run on branch claude/c3-status-variants.

## PAUSED for reboot (2026-10-27): see D:\gui_forms_workers\STANDBY_HANDOFF.md
- #6 reader open (384609f, CI nearly green); #4 cfbe9acd waiting on 4 file_manager bootstrap jobs; codemod WIP 31f8fa4 on
  claude/record-codemod (intermittent libclang whole-run test failure open); E2 uncommitted in dev/template_harness.

## RESUMED after reboot (2026-10-27, evening)
- harness #4 MERGED by main (cfbe9acd); gui_forms #401 closed; product branch protection live on all five products.
- harness #6 claude/ast-reader head a562359 (dev/th_reader): reader-test gate (step id `reader`: changed=true on schedule/dispatch
  or when the reader files/tests/this workflow changed; installs bindings and runs test_clang_ast under HARNESS_AST_READER=libclang,
  independent of C++ scope). 1f0ac98's step was broken YAML (joined continuations, a printf "\n" turned into a newline at column 0:
  GitHub rejected the workflow, so nothing ran); repaired in a562359. LESSON: doubled backslashes in Bash tool input can arrive single;
  write scripts with the Write tool. Next: confirm from the gate's log (python3-clang-22 installed; 24 tests under libclang, none
  skipped), then `gh pr merge 6 --merge` on all green.
- Codemod whole-run defect ROOT-CAUSED and fixed in #6 (4a27793): libclang CXFile handles are per TU and freed addresses are reused;
  the process-wide name cache named one unit's nodes after another's. Cache now per Converter; regression test alternates two units.
- claude/record-codemod (dev/th_codemod) head 16f2dbc, on a562359: 9fc969b WIP codemod; cdcd7fd codemod tests in the reader step;
  16f2dbc only C-family units from the compdb (a full `ninja -t compdb` lists custom commands; their shell was run as a compiler for
  -print-resource-dir, failing the first gui_forms run after 1186 s and making two empty dirs in the w3 build tree, removed).
- gui_forms report rerun in scratchpad cm/ (gf_report.md, gf_run.json, gf_run.log "rc=").
- E2 ported to clang_ast.Node: dev/template_harness claude/loop-branches-v2 commit 12d97e5 (local, on 4a27793). Class tests pass on
  both backends. First gui_forms run in scratchpad e2/ (e2_report.md, e2_queue.json, e2_run.log). Report counts to main BEFORE PR.
- #6 head 3d7d816: Windows compile commands now split by CommandLineToArgvW rules (non-POSIX shlex + quote strip had turned
  -DGUI_FORMS_VERSION_STRING=\"0.2.0\" into a broken define; first E2 run failed in gallery_controls.cpp after 646 s). Codemod
  (f1daa9a) and E2 (128716a) rebased on it; both gui_forms runs restarted (scratchpad cm/ and e2/).
- Coordinator 2026-10-27: the untracked stx_cng/ in D:/gui_forms_w3/gui_forms was removed by main (a cached zstd tarball only); the rule to leave it alone is retired.
- Coordinator: E2 counts received; E2 PR approved. Stride tests stay edge (repair unroll/split). The 327 non-test invariants are the
  first repair queue AFTER E2 lands; coordinator schedules it (do not start). main swept 68 merged branches org-wide.
- harness #7 E2 (claude/loop-branches-v2, 991177b), base claude/ast-reader. After #6 merges: retarget to main, then merge on green
  (template_harness: no branch protection, so --auto would merge at once; merge by hand after all checks are green).
- #6 confirmed on 3d7d816: gate job 122583736714 "AST reader test: true", python3-clang-22 22.1.8 installed, 26 tests under libclang,
  none skipped. Merging once Bootstrap proof completes green.
- harness #6 MERGED (dd57170) after all 22 checks green on 3d7d816. #7 (E2) retargeted to main + update-branch (2fe2a8f); merge by
  hand on green. harness #8 codemod opened (claude/record-codemod 6c9893c; WIP commit reworded; fixes: compdb units, functional
  cast type, factory local name, CRLF edits).
- gui_forms claude/c3-status-variants (uncommitted, D:\gui_forms_w3\gui_forms): codemod --apply (15 files, 177 mechanical) + hand
  edits (dock_site_drag end_drag/end_window_move; HostClipboardImageResult producers core/headless/windows/macos/linux; decoders take
  generation; release_image() &&; tests/example/benchmark; docs/api regenerated). build.sh 6 tests pass; sanitizer running
  (c3_sanitize.out). Next: commit encapsulation, then variant switch, then gui_forms PR with E6 note.
- api_reference.py on main lacks -Istx_zlib/include (inventory.json cannot regenerate): reported to main, not mine.
- gui_forms #426 claude/c3-status-variants (a6a24faa encapsulation, 6f70cf46 variant switch), auto-merge on; E6 note in body.
  9 tests + sanitizer clean. harness #7 and #8 waiting on bootstrap jobs; both touch the workflow's reader-test step (expect a
  conflict for whichever merges second; resolve locally). Merge each by hand on green.
- harness #7 E2 MERGED (fe5f31e; CI ran its class tests under json and libclang). #8 merged main locally to resolve the workflow /
  example_app.toml conflict (both lists kept): head 0d52909, waiting on checks; merge by hand on green.
- harness #8 codemod MERGED (848477a; 14 codemod tests ran under libclang in CI). gui_forms #426 MERGED (auto). Handoff queue done.
- Open for the coordinator: the 327 non-test invariants (E2 first repair queue; scheduled by main); api_reference.py stx_zlib include;
  codemod improvement: a deref of a non-const optional field that is only read (`(*r.image).pixels == x`) is a hand site today.
- Notes PR #1 (house-style rewordings) MERGED as e16fdb49 (owner approved).

## Queue after reboot (coordinator 2026-10-27, latest)

Order: 1 notes repoint, 2 harness CRLF/Skia PR, 3 E2 repair queue by area, 4 (later) codemod optional-field read.
- 1 DONE (open for main to merge; notes has no CI): notes #3 (base gui_forms, claude/repoint-house-style): 10 living docs
  repointed to `HOUSE_STYLE.md` on notes main (relative links use the blob URL), STATE_MACHINE_AUDIT R6 renumbered (§3, §2),
  planning/PROGRAMMING_HOUSE_STYLE.md deleted; dated records left as history. notes #4 (base file_manager,
  claude/repoint-house-style-fm): the 1.0 consolidation survey's one living reference. Worktrees D:\gui_forms_w3\dev\notes_rp, notes_fm.
- 2 harness #9 (claude/crlf-skia-cleanup, D:\gui_forms_w3\dev\template_harness): lockstep drops core.autocrlf=true; adopting.md
  CRLF/cache/Skia-identity pitfalls rewritten (the fetch_sdk CRLF rewrite it described never existed on main); ci_build_steps:222;
  gui_forms.toml drops the Windows skia=false override (books.toml kept: its own pin). Merge by hand on green.
- 3 E2 repair: scratchpad e2/area.py counts invariants by area, excluding any path with a tests/tools/fuzz segment: 318
  (controls 151, core 102, stx_png 24, stx_vorbis 15, stx_zlib 9, host-windows 8, stx_conv 3, demo 2, render 2, audio 1,
  host-linux 1). The coordinator's 327 counts only top-level tests/tools/fuzz as excluded. One PR per area: core, controls,
  hosts, stx_*. Keep decisions go in a decisions JSON wired through `[loop_branches] decisions` in the harness's
  projects/gui_forms.toml (or --decisions locally).
- 3 progress: decisions file gui_forms tools/loop_branch_decisions.json ({about, decisions:[path,function,condition,decision,reason]}).
  Scratchpad e2/: ctx.py <area> dumps sites; <area>_decisions.py maps site numbers to (decision, reason); build_decisions.py
  writes them; per-area rerun: check_loop_branches.py --paths <dirs> --decisions <file>.
  - core #433 MERGED: 102 -> 85, all decided (80 keep, 5 false_shape); 17 hoisted; tests: negative drop shadow, 32-bit DIB no alpha.
  - controls #434 (claude/e2-controls-hoists, retargeted to main, auto-merge on): 151 -> 135 decided (134 keep, 1 false_shape).
  - hosts #435 (claude/e2-hosts-hoists, stacked on #434; retarget + auto-merge once #434 merges): host/render/audio/demo
    14 -> 10 decided; DIB painter gradient/stroke splits pinned by a scene hash in windows_content_pixels_tests.
  - stx_* committed locally: claude/e2-stx-hoists 5c93b154 on hosts (5025c7a6), NOT pushed (would be 3 deep). 51 -> 42, all
    decided (23 false_shape: if constexpr, Ogg page state; 19 keep). Hoisted: PNG encoder gather_row by alpha mode and
    filter_row per filter with the first-pixel edge peeled; TextScan Latin-1 is one NUL find; vorbis transform index.
    png + audio tests (8) pass, sanitizer clean; png_writer identity hash unchanged. Push + PR (base claude/e2-hosts-hoists)
    once #434 merges; retarget to main after #435 merges. The shared decisions JSON forces stacking.
  - harness #10 (claude/gui-forms-loop-decisions, worktree D:\gui_forms_w3\dev\th_lb): gui_forms.toml [loop_branches]
    decisions = tools/loop_branch_decisions.json + CHANGELOG. Merge by hand on green; #9 also adds Changed bullets (expect a
    CHANGELOG conflict for the second to merge).
- harness #9: python-tests failed on test_project_names (expected the Windows skia=false override); fixed, pushed; 412 harness
  tests pass locally. Merge by hand on green.
- 2026-10-28 stack: #434 MERGED; notes #3/#4 MERGED. #435 retargeted to main, updated (7b299324), auto-merge on.
  #437 stx (claude/e2-stx-hoists 5c93b154, base claude/e2-hosts-hoists) opened; auto-merge refused while stacked.
  stx_vorbis standalone tests (7) pass plain and with STX_VORBIS_SANITIZERS=ON (trees .build/native-windows-x64/
  stx-vorbis-plain|sanitize; script scratchpad e2/vorbis_tests.sh); results commented on #437.
  Next: when #435 merges: gh pr edit 437 --base main, gh pr update-branch 437, auto-merge. Harness #9 then #10: merge by
  hand on all-green; the second needs its CHANGELOG "### Changed" conflict resolved (keep both bullets). Then report
  counts and stand by for the 0.5 leg.

## Queue (coordinator 2026-10-28; laptop workspace ~/Developer/gui_forms_workers/w3, build only via `m4ws w3 ...`)
Order: 1 CodeQL alerts, 2 macOS dialog-key flake, 3 Windows compositor flake check, 4 E3 then E5 static half.
- 1 DONE, pushed: gui_forms #443 claude/codeql-false-shapes 7de0e7e9 (no auto-merge; coordinator merges). Owner decision: patch
  false positives into equivalent forms, never dismiss. deflate.cpp cost sums widen to uint64 (alerts 4,5,6 + fixed distance sum);
  packet.cpp:188 size_t{row}*step (3); c_adapter.cpp arena = subspan of the aligned buffer (10). Triage table appended to
  code_integrity_checker/reports/README.md. Ran on M4: deflate|png_writer plain + --sanitize 4/4; oracle records_sha256 a3353d2f...
  identical before/after (511 records, block types 153/244/114); standalone stx_vorbis plain + sanitize 7/7 (script dev/q1_vorbis.sh,
  run as `m4ws w3 bash -s < dev/q1_vorbis.sh`). Confirm "Analyze (c-cpp)" shows 3,4,5,6,10 fixed when woken.
- 3 DONE (report only): since #409 merged (2026-10-27T07:12) 74 "GUI.Forms source and compiler cache" runs, 402 Windows jobs:
  0 failures (8 cancelled, 131 skipped); 68 job logs show both gui_forms_windows_compositor_tests and _probe Passed, 52 did not
  select them. Data: w3/dev/win_jobs.tsv, comp_hits.tsv.
- 2 IN PROGRESS: PR #438 macOS job 123011178509: first probe (ok/ok/return) active=1 alert_key=1 visible=0 ordered_out=1 FAIL;
  the other 15 passed. Cause (hypothesis to reproduce under load on the M4): the window server reports the first alert's
  occlusion state asynchronously and 250 ms (first_key_delay) is not always enough in a fresh process. Planned fix: before
  sampling facts and posting the key, poll every 20 ms (bounded, about 3 s) until the modal window is key and occlusion-visible;
  at the deadline sample anyway so a window that never shows still fails. Branch claude/dialog-key-wait (no commits yet).
- 4 not started.
- OWNER RULE (2026-10-28): workers no longer push or open PRs. Commit on local branches in w3/gui_forms only; pass on the M4
  (touched tests plain + --sanitize, checkers); report branch/head/what ran; coordinator folds local branches into a checkpoint.
  Do not push updates to already-pushed branches (#443). Rebase when the coordinator says a checkpoint landed.
- #443 reviewed by the coordinator (merges on green). #409 confirmed fixed.
- 2 DONE (local, not pushed): claude/dialog-key-wait cfdbd9bf (on main 31d974a0). Reproduced: first_key_delay cut to 5 ms gives
  CI's exact signature (active=1 alert_key=1 visible=0 ordered_out=1) on 5/5 runs; CPU load alone (12 busy loops, 15 runs) did not.
  Fix: wait_for_alert polls 20 ms x 150 until modal window key + occlusion visible, then post_probe_key; probe lines print
  show_polls. With the fix at 5 ms: 5/5 pass (1-2 polls); at 250 ms under load: 20/20 pass; --sanitize pass. Script dev/dk_load.sh
  (`m4ws w3 bash -s N < dev/dk_load.sh`, LOAD=k busy loops).
  M4 CONTENTION: one sanitizer run failed 2 probes with active=0 alert_key=0 level=0 while the integration tree's suite ran
  gui_forms_macos_accessibility_fixture_app at the same time (activation stolen across trees). Not a test defect; reported.
- Next: 4 E3 (const/restrict candidates) then E5 static half, in template_harness (own branch worktree w3/th_dev).
- m4ws now holds one M4 lock across workspaces (coordinator); GUI tests no longer collide. dialog-key-wait cfdbd9bf joins checkpoint 1.
- 4 E3 DONE (local, not pushed): template_harness worktree ~/Developer/gui_forms_workers/w3/th_dev, branch claude/e3-const-restrict
  d6796fe (on harness main 5256019). harness/check_const_restrict.py + test_const_restrict.py; clang_ast Node.parameters (per
  call argument parameter type, both backends; json member calls from the method's declaration); project.py
  [const_restrict] decisions (decisions_path helper shared with loop_branches); workflow reader step runs the new tests; CHANGELOG,
  adopting.md. Const is deep (writes through pointer members count); value casts read; .* / ->* escape; restrict needs an
  output written inside a loop.
  Run on M4: dev/th_m4.sh syncs th_dev to the M4's w3/th_dev and runs via m4ws. Tests: SDKROOT=$(xcrun --show-sdk-path)
  HARNESS_AST_READER=libclang python3 -m unittest test_clang_ast test_loop_branches test_codemod_records test_const_restrict: OK.
  (Without SDKROOT the codemod whole-run tests fail on the M4 on harness main too: libc++ "unresolved using declaration".)
  test_project's workspace test fails on macOS on harness main too (/private/var vs /var).
  gui_forms run (HARNESS_PROJECT=projects/gui_forms.toml, --build-dir w3/.build/native-macos-arm64/gui-forms, 649 units,
  176 s): const 160 open / 89 signature_bound (73 open outside tests/); restrict 1 open (stb_vorbis_open_pushdata, not a
  kernel) / 3 bound; restrict_span 26 open (stx_png 10, core 5, stx_zlib 4, stx_vorbis 3, controls 2, render 1, fuzz 1).
  Queue: M4 w3/e3_queue.json, copy w3/dev/e3_queue.json, report dev/e3_report.md. 4 open const sites are under
  src/render/skia (out of scope by the Skia rule; no exclusion list yet).
  Sample review (8 random non-test const): all plausible read-only. Registry::retain (writes through slot->record) was a false
  candidate before deep const; fixed.
  Open decision sent to coordinator: where restrict starts (gui_forms private span kernels box_blur/image_variant with
  GUI_FORMS_RESTRICT locals vs stx_* with per-package macros vs wait for profiles).
- Next: E5 static half (copies) on the same reader, unless redirected.
- Coordinator 2026-10-28: restrict = option 1 (GUI.Forms private kernels first: box_across, box_down, resample_one_line; each
  with written overlap rule, debug overlap assertion, before/after timing); stx_* only after. M4 env.sh now exports SDKROOT.
- HARNESS DEFECT (to fix, separate): template_harness test_project.ProjectFileTests.test_the_workspace_and_default_project_file
  fails on macOS (on harness main too): project_path() resolves /private/var/... while the test expects the tempdir's /var/...
  (macOS /var -> /private/var symlink). Fix: compare resolved paths (or resolve the tempdir root in the test).
- Order now: E5 static half (harness, local branch), then restrict changes (gui_forms, local branch); report at each milestone.

## 2026-10-28: checkpoint 1 refold (state-machine gate on macOS tree)

- claude/dialog-key-wait (local, w3 gui_forms): 10bba656 key_down_event now
  returns from each case with an Escape fallback after the switch; 087e6c03
  gives stated defaults to appkit_host_services.mm (icon, buttons, sound cue),
  macos_compositor_tests.mm (gate.step) and macos_exposure_tests.mm (stage).
  Head 087e6c03.
- Ran on the M4: check_state_machines --gate over the whole macOS tree (host-macos
  and tests: 0 F1/F2); macos compositor/exposure/message_dialog_key/host_close/
  clipboard_image + host_protocol tests plain and --sanitize (6/6 each);
  check_house_style on the four files (0).
- Remaining whole-tree F1 (not in the six, render/skia, stx_skia track's area):
  skia_executor.cpp:182 element.verb, :375 brush.hatch_style, :980 command.kind.
- Next: commit E5 (claude/e5-copies in th_dev), then restrict kernels.

## 2026-10-28: E5 static half committed

- template_harness claude/e5-copies (local, w3/th_dev, on claude/e3-const-restrict
  d6796fe): 56918bd check_copies.py + test_copies.py, [copies] decisions,
  json reader member-call writes. GUI.Forms run: 1439 candidates (see commit
  message; queue in w3/dev/e5_queue.json, e5_report.md).
- Ran on the M4: test_copies, test_const_restrict, test_clang_ast, test_project
  (json; only the known /private/var test_project failure) and the first three
  under HARNESS_AST_READER=libclang (38 OK).
- Next: restrict kernels in gui_forms on a local branch (GUI_FORMS_RESTRICT,
  box_across/box_down, resample_one_line; overlap rule, debug assertion, timing).

## 2026-10-28: restrict kernels (local branch claude/restrict-kernels on origin/main 39b5b77f)

- claude/dialog-key-wait deleted (content on main via #445). E3/E5 branches unchanged.
- Finding: Clang ignores __restrict on local pointers (the prescribed "local
  pointers from spans" form gave byte-identical code, no timing change). The
  branch puts GUI_FORMS_RESTRICT on parameters of internal-linkage kernels
  instead, called from span wrappers that assert the overlap rule.
- f4576938: kernel/restrict.hpp (macro, caution, bytes_disjoint/spans_disjoint,
  require_disjoint: abort unless NDEBUG; verified abort in -O0, skipped -DNDEBUG),
  box_down -> down_rows restricted; engine_material_tests kernel_overlap_rule.
  box_down -37 to -38% (1024^2 RGBA 2694 -> 1671 us).
- 8f47ffc1 (droppable): box_across -> across_rows, resample_one_line copy ->
  copy_line restricted; within noise (-1% / +2%).
- Bench: w3/dev/rbench (bench_box.cpp, bench_variant.cpp, build.sh, run.sh,
  asm.sh); M4 binaries in w3/.build/restrict_bench; logs w3/dev/rb_run*.log.
- Ran: engine material/glass/light + overlay_shadow plain at both commits,
  sanitized at head; state-machine gate --changed-from origin/main clean;
  check_house_style clean.
- Pending coordinator decision: keep or drop 8f47ffc1; then stx_* restrict.

## 2026-10-28: restrict applied uniformly (owner correction)

- Rule in force: restrict wherever the no-overlap promise can be shown (written
  rule + assertion active without NDEBUG and under ASan + touched tests under
  --sanitize); timing is information only. Public headers out until reviewed.
- gui_forms claude/restrict-kernels (local, on origin/main 39b5b77f), head 0c054037:
  b93ede50 box_down (+ sanitizer-active assertion), 90eab82d box_across +
  resample copy (kept per owner), 76f3d592 header moved to
  src/core/types/restrict (gui_forms::detail, require_disjoint_writes) +
  fill_grapheme_arrays, Impl::fill_bounded_runs, distribute_remaining,
  0c054037 stx_png (STX_PNG_RESTRICT) and stx_vorbis (STX_VORBIS_RESTRICT).
- tools/const_restrict_decisions.json: 30 decisions (16 apply, 14 keep with reasons).
- template_harness claude/e5-copies 13b25bb wires [const_restrict] decisions.
- Ran: touched GUI.Forms tests (engine material/glass/light, overlay_shadow,
  grapheme x2, bounded_shape x2, harfbuzz_font_engine, responsive_track_panel,
  png x7 incl. stx_png_core, audio x2) plain + --sanitize; standalone stx_vorbis
  plain + sanitized (q1_vorbis.sh); gate --changed-from clean; house style clean.
- Not done: neon/sse2 butterflies (not in the E3 queue); no vorbis timing.

## 2026-10-28: SIMD butterflies, vorbis timing, harness test fix

- gui_forms claude/restrict-kernels head c7c65416: NEON/SSE2/AVX2 butterflies
  restricted via internal *_pass + require_butterfly_disjoint. Standalone vorbis
  plain+sanitized pass (NEON); x86_64 -fsyntax-only for SSE2/AVX2 paths.
  Timing (info): inverse_mdct 2048 scalar -49%, NEON -55%; 256 unchanged; couple -5%.
- template_harness claude/e5-copies head bfeed3f: test_project compares resolved
  paths (passes on the M4 with TMPDIR /private/var/folders, /tmp, /var/tmp).
  The separate harness defect recorded earlier is fixed.
- Next: coordinator folds template_harness checkpoint, then gui_forms.

## 2026-10-28: restrict macro defect found and fixed (3defa29f)

- macOS <sys/cdefs.h> #defines __restrict to nothing in C++: every restrict use on
  the macOS build was a no-op until 3defa29f (macros now __restrict__ on Clang/GCC,
  __restrict on MSVC). Found because the E3 checker (bfeed3f harness) still
  listed the restricted kernels; the AST confirmed plain pointers.
- Earlier timings (b93ede50 -38%, vorbis -49/-55%) were NOT restrict effects
  (restructuring / core scheduling noise). Real-restrict timing is in 3defa29f's
  message: box_down about -60%; others within noise; mdct 2048 bimodal (P/E cores?).
- The "Clang ignores local restrict" claim was based on the no-op build; removed.
- Waiting: template_harness #11 (bfeed3f) merge -> then harness.lock bump commit on
  restrict-kernels (lock.py bump --to <merge sha>), move w3/template_harness to the pin,
  rerun const_restrict checker (expect only decided entries), state gate, tests.
- Pre-run with bfeed3f harness + --decisions: 14 decided + 3 decided restrict,
  6 open = the restricted kernels themselves (should clear with 3defa29f).

## 2026-10-28: restrict-kernels rebased, messages corrected; harness wording branch

- gui_forms claude/restrict-kernels rebased onto origin/main f8326690 (checkpoint 2),
  head be9ae56e (69bd0c49, 25514d3d, d0547daa, b4ee0403, f639e133, be9ae56e). Messages
  no longer carry no-op-restrict timings; be9ae56e records real-restrict timing and that
  neutral/slower uses stay (uniform rule). Same patch as before (range-diff: messages only).
  Verified on M4: 19 touched tests plain + --sanitize, standalone vorbis plain + sanitized,
  state gate --changed-from clean.
- template_harness claude/e3-restrict-wording a7ac855 on bfeed3f (#11 head, not merged yet;
  rebase onto main after #11 merges): check_const_restrict help + CHANGELOG.
- Still waiting: #11 merge sha -> harness.lock bump commit on restrict-kernels.

## 2026-10-28: lock bump and bfft macro form (ready for checkpoint 3)

- gui_forms claude/restrict-kernels head 47fcc0a9: 9386326f harness.lock -> 53af66c
  (#11 merge), 47fcc0a9 macros follow bfft's form exactly (MSVC proper __restrict,
  GCC/Clang incl. clang-cl __restrict__). w3/template_harness checked out at 53af66c.
- M4 at 47fcc0a9: AST carries __restrict; const_restrict with --decisions: 14 decided,
  0 open; 19 touched tests plain + --sanitize; standalone vorbis plain + sanitized;
  state gate --changed-from clean.
- template_harness claude/e3-restrict-wording 8c016cd on origin/main 53af66c (help names
  bfft's form; CHANGELOG).
- Standby after this report.

## 2026-10-28: Read the Docs root links (owner task)

- gui_forms claude/rtd-root-links (local, on origin/main f8326690) head 4e5ae9f7: README.md
  122 links + tools/control_registry.py SITE_URL drop /en/latest/. All 122 changed links
  curl 200 (no redirects). Other readthedocs mentions are API URLs or numpydoc (unchanged).
- Ran: test_readthedocs_build, test_control_registry, control_registry check and
  emit-cpp --check, test_generate_library_docs, generate_library_docs --check: all pass.

## 2026-10-28: translation system design note (owner design task, no code)

- Researched Plan Paint (languages/README.md, src/localization.{hpp,cpp},
  localization_platform.cpp, forms/dialog.cpp english_help + settings selector,
  ribbon.cpp "Help! English!" button) and GUI.Forms (Control accessible_name/description,
  layout_direction inherit, SheenBidi in harfbuzz engine, StableId, control_registry,
  for-windows-forms-developers.md "Designer/.resx -> Code", macOS host owns the app menu).
- Note sent to coordinator, with the coordinator's two added owner requirements
  (user-side template export without source; untranslatable recovery path). Key proposals:
  "…"_tr literal operator template registers every key in the binary; Window side table
  re-resolves localized properties on Application::set_language; library LanguageSettings
  piece + library-owned menu items + startup flag/env + safe start; checker validates packs
  against the binary's exported key set. Open owner decisions listed in the note.
- No branch. w3/gui_forms back on claude/rtd-root-links. Standby.

## 2026-10-28: macOS Quit menu names the program (checkpoint 4, with rtd-root-links)

- gui_forms claude/macos-quit-name (local, on origin/main f8326690) head 37c563fa: install_application_menu
  moves to src/host/macos/application/application_menu.{hpp,mm}; Quit title = CFBundleDisplayName ->
  CFBundleName -> process name. New gui_forms_macos_application_menu_tests (temporary bundles + live
  installed menu of the unbundled test process).
- M4: macos_(application_menu|exposure|clipboard_image)_tests plain 3/3 and --sanitize 3/3; house style
  0; lifetime rules on touched files 0; state gate --changed-from origin/main clean.
- Translation note revision (user-override mode, owner answers on decisions 3-5) sent with this report;
  decisions 1 and 2 still open with the owner.
- Coordinator 2026-10-28: translation revision saved on the desk but its keying is superseded. The owner
  points at CSL locales (identifier keys, ASCII with dashes: stable IDs and message names, never English
  text; access-key field instead of '&' in text; CSL forms and single/multiple for variants and plurals).
  Waiting for the confirmed details before revising. Standby.

## 2026-10-29: checkpoint 3 landed; rebases; translation note rewrite (CSL basis)

- Checkpoint 3 on main 8cde98df (#447). Deleted claude/restrict-kernels. Rebased onto 8cde98df:
  claude/rtd-root-links 031457e9, claude/macos-quit-name 4d0864a9 (menu test plain + --sanitize 1/1 after
  rebase). Both for checkpoint 4. w3 clone on claude/macos-quit-name.
- Translation note rewritten on the owner's confirmed basis: identifier keys (ASCII lowercase/digits/dash),
  control path + property, developer message names, CSL locale structure as JSON (info, terms, forms,
  single/multiple + CLDR extras, conventions), access-key field, items= placeholder lists (exact match or
  entry missing; checker error), locales/ folders, single-file locales, user-override trio, opt-in
  follow_system_locale pattern (same pattern for theme high-contrast/light-dark), skeleton export, Plan
  Paint converter. StableId migration measured: 1,634 distinct literals, 1,437 violate (1,357 dots,
  92 underscores), '.'/'_' -> '-' fixes all with 3 collisions. Open owner questions listed in the note.
- Standby after sending.
- 2026-10-29 owner answers on rewrite 2: no StableId renaming (dots are hierarchy; locale `controls` and
  skeleton nest by segment then child); per-segment ASCII-dash validation; 92 underscore IDs are an owner
  question (not planned). Dynamic plain-string text translated only initially (alternative: unaddressable);
  source guard dropped. Accepted: single/zero may omit {count}; '&File' compat split; ID-less items
  unaddressable. Measured: with dots as separators only the 92 underscore literals fail. Revised sections sent.
- 2026-10-29 owner: locale files are CSL-style XML (`locales/locales-<tag>.xml`). Found GUI.Forms already has
  the bounded XML reader/writer (include/gui_forms/xml.hpp, DK decision 43; fuzz/xml_reader_fuzz.cpp,
  round-trip) matching the owner's reader rules; it refuses mixed content, so a control with child controls
  carries its text in a `text` attribute (owner decision raised). Revised format, skeleton and reader
  sections sent. Standby.

## 2026-10-29: StableId dashes + enforcement (owner; checkpoint 4 topic) — done, b3c50fd5

- Branch claude/stable-id-dashes (local, from origin/main 8cde98df), head b3c50fd5. Verified on the M4:
  97 touched/fixture-user/gallery tests plain and --sanitize 100%; client gallery a11y x5 pass (one earlier
  failure was during another M4 user; not reproduced); generate_library_docs --check PASS; house style +
  lifetime 0 on 57 files; state gate --changed-from origin/main exit 0.
- PageSet refuses page ids that break the rule (PageEditStatus::invalid_id; PageView panels and PageStrip
  list rows carry the page id).
- docs/reference/inventory.json NOT committed: origin/main's was already stale (unrelated ColourRole etc.;
  regenerated diff 9.4k lines). Coordinator should regenerate it at checkpoint with LLVM 22.
- StableId: valid_stable_id (constexpr; '.'-separated [a-z0-9]+(-[a-z0-9]+)*); StableIdLiteral consteval
  ctor (bad literal = compile error via non-constexpr detail::stable_id_literal_is_not_...); checked
  template ctor for std::string / std::string_view (throws invalid_argument); bare const char* refused at
  compile time. tests/stable_id_compile_failures.py (ctest gui_forms_stable_id_compile_failures) +
  core_tests test_stable_id_rule. Docs: StableId docstring, docs/user/controls.md.
- Renamed: 88 fixture IDs (+ their item prefixes; StockControlEntry gains stable_id, manifest_id derived
  '-'->'_'; macOS accessibility tests attribute by stable_id, report by manifest_id), doc examples,
  demoboard, docking_vocabulary side-tab, input.maximum-length. Library-built IDs that would throw:
  data_grid/list_view/tree_list_view scroll bars, label-editor, font_dialog captions/lists, auto-complete,
  ribbon group-popup/application-menu, dock drag-layer/position-menu + position item names
  (dock-as-document, auto-hide), navigation pane menu items, task dialog expanded-text, property grid
  row/category ids (id_segments: css_spelling + non-alnum -> '-'), ribbon tests' hosted item ids.
  const char* StableId sites wrapped in std::string (15 tests, layered_material_lab, linux_dialogs).
- Left alone (not StableIds, semantic-only virtual node ids with '_' suffixes): page_strip scroll_back,
  calendar previous_month, ribbon tab_row/command_plane/scroll_left, ribbon_gallery scroll_up/more_button,
  list_view default_group, data_grid row_header. Report as follow-up.
- Item IDs composed into StableIds are now checked when set (else they threw only when a popup opened):
  ContextMenu identity + every menu item ID (rows are <menu>.popup.row.<item>); every ToolStrip item ID
  (overflow rows carry it); QuickAccessToolbar command IDs (customize rows); Ribbon hosted (non-authored)
  command IDs. Renamed for it: binding navigator items (move-first..., separator-*, add-new), demoboard
  menu items (open-with, move-copy, new-folder, ...), demoboard helper-built ids (fm.ribbon.move-copy,
  fm.ribbon.view-mode, fm.selection.object-name, fm.selection.preview-disclosure), command_route
  strip-save, ribbon_scaling text-color/word-wrap (icon_menu prefixes menu item IDs). Test in
  menu_controls_tests (test_item_ids_follow_the_stable_id_segment_rule). Docstrings ContextMenu,
  ToolStrip, QuickAccessToolbar, Ribbon. Command IDs elsewhere are not StableIds and are unchanged.
- Docs regenerated (generate_library_docs --check PASS). API inventory (docs/reference/inventory.json)
  must be regenerated on the M4 with LLVM 22 (tools/api_reference.py --build <tree>).
- Plan Paint (for its track): atlas.cpp ternary-of-literal StableIds (6) and pattern_canvas const char*
  ids[i] will not compile; wrap in std::string.
- Owner decision A recorded: control text is always a `text` attribute.
- Queue after this: clang plugin + dogfood (owner assignment), then locale note plugin section.

## 2026-10-29: semantic node IDs follow the dash rule (owner decision; on top of stable-id-dashes, checkpoint 5)

- Branch claude/semantic-id-dashes from claude/stable-id-dashes b3c50fd5 (local).
- Window::append_semantic_nodes checks every virtual child ID and controlled_panel (require_valid_virtual_ids,
  std::invalid_argument naming the control); semantic_tests test_virtual_node_ids_follow_the_segment_rule.
- Renamed: page_strip scroll-back/scroll-forward/list-button, calendar + month calendar previous-month/next-month,
  ribbon tab-row/command-plane/scroll-left/scroll-right, ribbon_gallery scroll-up/scroll-down/more-button,
  list_view default-group, data_grid row-header; tests and docstrings that name them.
- App-supplied IDs that become node IDs checked at set time: ListView items/columns/groups, TreeListView
  nodes/columns, TreeView/ObjectView/CorrespondenceView/BreadcrumbTrail (shared validate_identity_text), Ribbon
  tabs/sets/groups/commands/recent items, RibbonGallery categories/items, MenuStrip items, BackstageView items,
  PropertyList groups/rows, DataGridView unbound/virtual row IDs and column names (one segment), BindingSource
  record IDs. Not checked at set time: window-mode grid row IDs from a row window source (fetched during
  layout; caught by the snapshot check).
- Fixed in passing (defects in b3c50fd5 outside its test selection): responsive_tracks_lab "responsive.scale.100%",
  BindingNavigator docstring command names. Gallery: project file ids tree-view.cpp/readme.md, style item IDs via
  style_id() ("style.heading-1"); ribbon tests' underscore ids.
- Head 4d4ec350 (on b3c50fd5). Verified: 125 tests plain and --sanitize pass, generate_library_docs --check
  PASS, house style + lifetime 0 on 31 files, state gate --changed-from origin/main exit 0. Not yet reported.
- Tooling: m4ws's lock-wait loop eats stdin, so `m4ws w3 bash -s < script` runs nothing. Copy the script
  with scp to Developer/Projects/gui_forms_workers/w3/<x>.sh and run `m4ws w3 bash ../<x>.sh`.

## 2026-10-29: localizable-literal Clang plugin + dogfood (owner assignment)

- Branch claude/localizable-literal-plugin from origin/main 8cde98df (local, in progress).
- Plugin tools/localizable_plugin (own CMake project, Clang 22 package, -fno-rtti, -undefined
  dynamic_lookup on Apple), registered "gui_forms_localizable" (no dashes: the driver splits
  -fplugin-arg-<name>-<arg> at the first dash). Arg root=<dir>.
- Mark: GUI_FORMS_LOCALIZABLE(property) = [[clang::annotate("gui_forms.localizable:<property>")]]
  (include/gui_forms/text/localizable/localizable_property.hpp) on setter params and item fields.
- Table: one ELF-note-shaped blob per TU (u32 LE; name "GUIForms\0", type 0x47464c01; desc = count,
  then property,name,file,line,column,function,text), 4-aligned, section __DATA,__gui_forms_loc /
  .note.gui_forms.loc / .gfloc$m. Variant name "v-" + 12 hex of FNV-1a-64 of the bytes. File relative to
  root (normalized, real-path fallback). Dedupe key (file,line,col,property,function).
- Runtime: gui_forms::localizable_literals() (dyld getsectiondata per image / dl_iterate_phdr PT_NOTE /
  COFF main image only), parse_localizable_tables(). Dump: tools/dump_localizable_literals.py (llvm-objcopy).
- CMake: cmake/LocalizablePlugin.cmake (option GUI_FORMS_LOCALIZABLE_PLUGIN ON, ExternalProject,
  gui_forms_compile_with_localizable_plugin(), install to lib/gui_forms); package option
  GUI_FORMS_LOCALIZABLE_LITERALS (ON) adds -fplugin to GUIForms::Core for Clang 22 consumers.
  Gallery model compiled with it. Test gui_forms_localizable_plugin_tests (+ static/shared/plain units),
  gui_forms_localizable_dump_tests.
- House style: no lambdas/auto/-> in tools or tests, so lambda coverage is a scratch consumer check only.
- api_docs_parse.py strips GUI_FORMS_LOCALIZABLE(...) from signatures. User doc docs/user/localizable-literals.md.

## 2026-10-29: folded down (owner direction) — RESUME POINTER

- Done and verified: claude/stable-id-dashes b3c50fd5 (reported, checkpoint 5) and
  claude/semantic-id-dashes 4d4ec350 on top of it (125 tests plain + --sanitize, docs --check PASS, house
  style + lifetime 0 on 31 files, state gate --changed-from origin/main exit 0). Nothing else pending there.
- PARKED, not to be resumed without a new assignment: the clang plugin. It had been largely built when the
  fold-down arrived; kept as local WIP commit claude/localizable-literal-plugin c88712f6 (from origin/main
  8cde98df, unverified). State there: builds on the M4; the dogfood test finds every expected record
  (incl. unexecuted branches, same literal twice, concatenation, raw/u8/escapes/UTF-8, constexpr char* and
  string_view constants, templates once, marked own helpers, designated/positional aggregates, field
  assignment, no-plugin TU absent, static lib and shared lib found; lambdas and generic lambdas found when
  present, but the committed test drops them for house style). Remaining: fix the reader test's
  "a table cut at 85" expectation (an unpadded complete table legitimately parses), sanitizer run, state
  gate, docs --check (generate_library_docs already regenerated; api_docs_parse strips the mark), installed-
  package consumer check (script dev/loc_pkg.sh + dev/consumer/), CI needs (LLVM 22 Clang CMake package on
  Linux; Windows/MSYS2 plugin loading untested; COFF reader scans the main image only).
  Owner scope for the plugin as assigned: literals passed to localizable setters (text, tool-tip, title,
  placeholder, accessible name/description, header, items' text) into a dedicated section table (name,
  property, call site, bytes), no rewriting; dump command; -fplugin via the CMake package, on by default with
  an off option; edge cases as listed above plus TU without plugin and static vs shared.
- NOT STARTED: docs/reference/inventory.json regeneration with LLVM 22
  (tools/api_reference.py --clang /opt/homebrew/opt/llvm@22/bin/clang++ --build <M4 tree>), its own small
  branch from origin/main; commit message must say main's copy was already stale (a 9.4k-line diff on main).
- Locale note (coordinator keeps desks/coordinator/TRANSLATION_DESIGN_2026-10-28.md): gaps reported to the
  coordinator 2026-10-29 (decision A, runtime-text decision, dash-rule answers, plugin section, items property).

## 2026-10-31: resumed (standby); inventory done; plugin rebased

- Main 2fe87fd9. stable-id-dashes and semantic-id-dashes landed squashed as 9819ab78 (content checked);
  local branch claude/stable-id-dashes deleted (semantic-id-dashes no longer existed locally).
- Assignment 1 DONE: claude/inventory-llvm22 b233a13a (from 2fe87fd9): inventory.json regenerated on the M4
  (LLVM 22, tree ../.build/native-macos-arm64/gui-forms built by ci_build --gallery); api_reference --check
  PASS, generate_library_docs --check PASS; stale SK2 enumerators gone. Note: api_reference writes the file
  on the M4 side; scp it back before the next m4ws sync overwrites it.
- Assignment 2 IN PROGRESS: c88712f6 cherry-picked onto 2fe87fd9 as claude/localizable-literal-plugin
  (conflicts: text_box.hpp read_only now out-of-line; text.hpp includes markdown/ instead of rich_text/;
  docs/api/library.text.rst taken from main, to regenerate). Next: remaining list in the RESUME POINTER.

## 2026-10-31: both assignments DONE and reported — RESUME POINTER

- claude/inventory-llvm22 b233a13a (on 2fe87fd9): inventory regenerated (see above).
- claude/localizable-literal-plugin 19897965, stacked on b233a13a (fold the top branch). Verified on the M4:
  localizable plugin + dump tests plain; those + core + 6 Gallery tests (9) --sanitize; installed-package consumer
  (dev/loc_pkg.sh, dev/consumer; on: 4 records incl. lambdas, off: 0); api_reference --check, docs --check,
  control_schema check, house style + lifetime 0, state gate no F1/F2.
- Fixed while verifying: plugin sub-build inherited sanitizer env flags (now -D...FLAGS= empty); plugin project
  enables C (FindLibEdit); table is no_sanitize(address,hwaddress) (ASan redzone inside section -> overflow);
  reader cut test bound.
- Open for the coordinator (CI, not changed): LLVM 22 Clang CMake package on Linux runners; Windows/MSYS2 plugin
  loading; COFF reader main image only. Follow-up candidates: MetricsView/EasingPreview set_title and
  TypographySpecimen set_text/set_caption are unmarked (specimen controls; outside the assigned property list).
- M4 helper scripts: w3/loc_verify.sh, w3/loc_v2.sh (copies in dev/). Nothing pending.

### 2026-10-31: owner on the unmarked specimen setters
- MetricsView/EasingPreview set_title and TypographySpecimen set_text/set_caption stay unmarked now. Owner: "specimens are examples, so if its not correct to our standard, it will need a touchup pass during that part of our staged work" -> mark them (or otherwise bring them to standard) in the benchmark/demo touch-up stage of the 0.5 consumer refactor (notes ROADMAP_TO_0.5.md, Addendum 2026-10-31).

## 2026-10-31: probe direct launch (coordinator assignment) — STOPPED at the activation difference

- Branch claude/probe-direct-launch from 2fe87fd9. Direct exec of the probe bundle's Contents/MacOS executable:
  activation "none", key_window/app_active false in all 18 cases, probe fails (2 runs). Under open: "activate",
  all true. Launcher change reverted; attempt kept as w3/dev/probe_direct_launch_attempt.patch.
- Committed only the compositor_tests grant line: head (see git log), 1 file. Probe/tests pass plain, tests
  --sanitize, style/lifetime 0, gate no F1/F2. M4 now reports screen_capture_access true and
  accessibility_trusted true for ssh-started processes.
- Awaiting the coordinator's choice of approach (options in the report).

## 2026-10-31 (late): CI fix for the plugin; probe Screen Recording preflight — both DONE, reported — RESUME POINTER

- claude/localizable-literal-plugin head 00882371 (on 19897965, on claude/inventory-llvm22 b233a13a). Checkpoint
  2026-11-01-1 CI (run 41260194301) failed: Linux ClangTargets.cmake names missing libclangBasic.a; MSYS2 C compiler
  derived without .exe; macOS-15 plugin minos 15.0 (audit_macos_minimum). Fix in cmake/LocalizablePlugin.cmake:
  configure-time probe (fresh plugin_dir/build configure + build + compile probe.cpp with -fplugin, check "GUIForms"
  and the literal in the object); ExternalProject only when usable, reusing that dir; else one STATUS line, details
  in <tree>/localizable_plugin/probe.log. Success cached by GUI_FORMS_LOCALIZABLE_PLUGIN_PROBED = generator|args;
  failures retried. Passes main CMAKE_C_COMPILER and CMAKE_OSX_DEPLOYMENT_TARGET. Verified M4
  (dev/tools/loc_probe_verify.sh): on plain 2/2, --sanitize 3/3, minos 14.0 both trees, missing and broken package
  skip cleanly (tests 1/1), restore 2/2, checkers 0, gate clean. Linux/Windows runners unproven (CI only).
- claude/probe-direct-launch head 579ab3f3 (on efcd11bd): compositor probe reads CGPreflightScreenCaptureAccess once;
  without it no CGWindowListCreateImage, cases print "image_reads":"not exercised: ...", run succeeds;
  --no-image-reads forces the path. M4 under open: screen_capture_access false (18/18 skipped, all active);
  direct ssh run (granted): 5 composite reads. dev/tools/probe_capture_verify.sh.
- dev/tools: loc_probe_verify.sh, probe_capture_verify.sh (scp to M4 w3/ and run `m4run w3 -- bash ../<x>.sh`; m4ws
  syncs only gui_forms/ and template_harness/). In scripts pass ci_build --tests without the extra inner quotes.

## 2026-11-01: plugin rebased onto e1262237 with tooltip marks; phase wound down — RESUME POINTER

- claude/localizable-literal-plugin head fd758179 (e70c16d6 plugin, fd758179 probe) on origin/main e1262237.
  Conflicts with the W tooltip build resolved (tool_tip.hpp include + comment, control.hpp, text_box.hpp,
  tool_strip.hpp). New marks GUI_FORMS_LOCALIZABLE(tool_tip): Control::set_tool_tip_text (covers TabPage header
  tooltips and field hints, both read from it), ToolStripItem::tool_tip_text; ToolTip::set_tool_tip kept.
  docs/user/localizable-literals.md table updated. Not marked: RibbonToolTipContent title/description/
  disabled_reason (pre-existing, follow-up candidate); SemanticNode::tool_tip (library output).
- Verified M4 (dev/tools/loc_probe_verify.sh, --gallery set 'localizable|gallery|doc_examples|binding|tool_tip'):
  plain 14/14, --sanitize 14/14 no reports, minos 14.0, missing/broken package skip + 1/1, restore 2/2, house
  style/lifetime 0, gate clean, generate_library_docs --check PASS, control_schema check OK.
- api_reference --check FAILS, also on main: origin/main's docs/reference/inventory.json predates the tooltip build
  (no set_tool_tip_text), and docs/reference/contracts/tool_tip.json names the removed
  ToolTip::remove_tool_tip(const Control&) (render raises "Stale contract"). Not fixed here; for the coordinator.
- Phase wound down by the owner (2026-11-01). Nothing in progress. probe branch claude/probe-direct-launch 579ab3f3
  reported earlier.

## 2026-11-01 (Windows host): checkpoint 9 blockers + tool_tip contract — superseded by the RESUME POINTER below

- Host is now Windows x86-64 (D:/gui_forms_w3; env.sh/build.sh/toolchain_env.sh beside the checkout; sanitizer
  /d/malkuth/projects/tools/sanitize.sh; Python C:/Users/Shadow/AppData/Local/Programs/Python/Python314/python.exe).
  Scripts: dev/cp9/plain.sh (ci_build --gallery --tests RX), dev/cp9/verify.sh (style, lifetime, plain, state gate, sanitize).
- Task 1 branch claude/sm-checkpoint9-fixes from origin/checkpoint/2026-11-01-4 (b94c9a0a).
  - UIA: reproduced locally (easing_preview RangeValue expected 0, observed 0.15). Cause: ADR-016 AnimationTimer
    waits for a paint holding no request; Window::cancel_frame_requests (the clients' page freeze) cannot end it,
    so the first paint of the shown page starts the hidden easing preview's timer. Fix: Window counts
    cancellations (private, friend AnimationTimer); the timer records window lifetime token + count when it
    schedules; note_painted/started treat a moved count (or a ticking request ended without stop) as stopped.
    No public line or comment shifts (inventory unaffected). Test added in tests/animation_tests.cpp.
  - macOS SDK audit: template_harness local branch claude/sm-audit-compiler-plugins 850b52d (on origin/main
    5e3012b): --compiler-plugin NAME --compiler PATH; gui_forms 4ad1d6e0 bumps harness.lock to it, workflow passes
    clang++, shim test drops it from POSITIONAL_TOOLS. Not runnable here (macOS only).
- Task 2 (tool_tip contract + inventory regen) not started.

## 2026-11-01 (Windows host): tasks 1-3 DONE and reported — RESUME POINTER

- Task 1: gui_forms claude/sm-checkpoint9-fixes head 7a6ba09c (on origin/checkpoint/2026-11-01-4 b94c9a0a):
  4ad1d6e0 macOS SDK audit (harness.lock -> template_harness 850b52d, local branch
  claude/sm-audit-compiler-plugins on harness origin/main 5e3012b, NOT pushed; coordinator lands it and re-pins
  to the merge), 7a6ba09c AnimationTimer cancellation fix. Verified: 14 tests (animation, motion_clock, gallery*,
  doc_examples, progress_bar, split_container, error_glyph, visual_inspector, UIA+MSAA gallery) plain 14/14 and
  sanitized 14/14 (UIA 2131 s under ASan), style/lifetime 0, gate no F1/F2; harness tests 22 OK, shims 8 OK.
  macOS audit not runnable here. Linux: no SDK runtime audit; Windows: check_windows_sdk_imports would refuse a
  plugin .dll importing libc++.dll (plugin skipped on both runners today).
- Task 2: claude/sm-tool-tip-contract 1a6904bc (same base): tool_tip.json re-keyed/rewritten (reviewed false),
  inventory regenerated on Windows LLVM 22 (no platform spellings). api_reference --check PASS, docs --check PASS;
  also PASS with task 1's headers.
- Task 3 (coordinator queue item 3): claude/sm-ribbon-tooltip-localizable dab26da9 stacked on task 2 (both touch
  inventory.json; fold the top). 5 tests plain+sanitized, checks pass.
- Notes: this clone has core.autocrlf=true, so generate_library_docs --check reports CRLF-only "content differs"
  until the generator rewrites the files (no content change). w1 ran its own UIA tests concurrently.
- Follow-ups: Command::set_description / set_availability_reason unmarked; Windows plugin audit rule.

## 2026-11-02: cache safety + checkpoint 10 (Windows integration workspace)

- Cache safety (step 0, part 1): harness claude/sm-cache-safety 4b8c1d7 (harness-native.yml object save under
  success()); gui_forms claude/sm-cache-safety 3271a5f3 (native-platform/release saves under success() and after
  the tests; native.yml cache-prune waits for no failed need; tools/test_workflow_cache_saves.py, 4 tests, 3/4 fail
  on old main = the dry case).
- Checkpoint 10 = checkpoint/2026-11-02-1, head b1c74333 in D:/gui_forms_int, verify PASS (304/304 plain, static
  checks 0 failing). NOT pushed (coordinator pushes). 6 topics: tooltip+ribbon marks, cache safety, wake counters,
  conv hoist, w-dgv-colours 810f8d31 (T4 phase 1 + DGV), t4-accent-contrast.
- Dropped: w5 claude/c-abi-c3 2065dc68 — fold conflict in tests/gallery_pages/schema_export_tests.cpp (c-abi-c3
  turns the hard-coded `values` list into a loop over part_value_names; w-dgv-colours appends "authored_color" to
  that list). Owner of the fix: w5 rebases on the checkpoint and adds authored_color to part_value_names.
- Regeneration: library docs and display-trace digests unchanged on the fold; inventory.json regenerated
  (LLVM 22) but cannot fold as a topic (3-way squash conflicts with the topics' textual edits), and
  api_reference --check is not a CI gate. Delivered as w3 claude/sm-cp10-regen 147a1f6a = one commit on
  b1c74333, for the coordinator to push onto the checkpoint branch.
- D:/gui_forms_int: build.sh `all` now also runs checks.sh (CI static checks + code integrity + state gate
  --changed-from origin/main; logs dev/checks/). template_harness's own suite is skipped on this host
  (5 Windows path-form failures in bootstrap/lock tests; HARNESS_TESTS=1 runs it). Working tree was CRLF on
  2449 eol=lf files (stale checkout) and broke gui_forms_markdown_tests; renormalised.
- Next: the shared number formatter queue item (F0..F12, N, P, C, E, G; stx_auxlib first; w_controls.md cases).

## 2026-11-02: checkpoint 11 (D:/gui_forms_int, on main e22ea9c9)

- checkpoint/2026-11-02-2, head 560f3767, verify PASS (318/318 plain, bubble oracle skipped as before; static
  checks 0 failing). NOT pushed. Topics: C ABI C1-C4 (w5 c-abi-c4 7e46a767); DGV wrap/sizing/order (w2
  w-dgv-order 03431f83); -ffp-contract=off (w4 fp-contract-off 3bb9906d); T4 phases 2-3 + phase 4 batch 1
  (w1 t4-themes 3cfae07c). All five requested branches were on e22ea9c9.
- Dropped: w4 claude/stx-conv-dispatch 5463baaa: state gate F1 at stx_conv/src/lane_dispatch.cpp:83 (switch
  (variant) has no default) and C1 at :88 (case v128 `break;` without a reason). With it folded the pin worked:
  fresh configure fetched stx_auxlib 6cc942b; variants scalar;v128;avx2; all stx_conv tests passed, incl. the
  forced scalar/vector128 entries and a by-hand STX_SIMD_LEVEL=avx2 run.
- Inventory regeneration: w3 claude/sm-cp11-regen 19f06427 = one commit on 560f3767 (docs and digests unchanged).
- Next: the number formatter queue item.

## 2026-11-02: shared number formatter (queue item after checkpoint 10)

- gui_forms claude/sm-number-format 938bbb63 (on main e22ea9c9): include/gui_forms/binding/number_format
  (parse_number_format: F N P C E G, precision 0..99; append_formatted_number for double/int64/uint64;
  plain_number_text = the reverse for typed text). Binding format_string and DataGridView cell format use it
  (both replaced their F0..F12 code); Binding source update and DGV commit parse formatted text back.
- Decisions in force (documented in the header): invariant culture; doubles via std::to_chars (exact binary
  value, ties to even = .NET Core 3.0+); P rounds at p+2 then moves the point (exact); sign follows the sign
  bit ("-0.00"); integers exact, ties away from zero when a precision drops digits; G default threshold 15
  for doubles, every digit for integers; NaN/Infinity/-Infinity for every format; C invariant "(¤n)".
- stx_auxlib gaps (for its owner): text.hpp has the to_chars layer only: no group separators, no
  percent/currency patterns, no .NET G switching, no three-digit exponent, no parse of grouped/percent/currency
  text; GUI.Forms links only stx_auxlib's simd.cpp (and the pin is not on main yet).
- Verified: number_format (new), binding, data_grid_view (+order) tests plain and ASan/UBSan; house style,
  lifetime, library docs regenerated + --check, schema, registry, state gate --changed-from origin/main clean.
- w3 clone: renormalised 2478 stale CRLF files (eol=lf) before editing.

## 2026-11-02: D:/gui_forms_int verify hardened

- checks.sh now derives its commands from the checked-out workflows (D:/gui_forms_int/ci_commands.py WORKFLOW
  JOB prints a job's `run:` commands; no YAML library; fails loudly on a missing job): native.yml house-style
  (26 commands) + code-integrity.yml code-integrity, then the state gate and the docs job's
  `sphinx-build -W --keep-going -b html docs docs/_build/html` when Sphinx is importable (DOCS_PY, else
  .venv-docs, else the system Python). Harness self-tests still skipped on this host (HARNESS_TESTS=1).
- Proof: on the checkpoint 11 fold it fails house-style stock_anatomies --check exactly as CI did.
- Sphinx is NOT installed (not installed by me): docs/requirements.txt = sphinx 9.1.0, myst-parser 5.1.0,
  pydata-sphinx-theme 0.23.0, sphinx-design 0.7.0, sphinx-copybutton 0.5.2; 35 packages with dependencies,
  ~27 MiB of wheels from PyPI (largest babel 9.7 MiB, pydata theme 5.9 MiB, sphinx 3.7 MiB). Suggested:
  a venv at D:/gui_forms_int/.venv-docs.
- 2026-11-02: owner-approved Sphinx install done: D:/gui_forms_int/.venv-docs (pip install -r
  gui_forms/docs/requirements.txt; sphinx 9.1.0 etc.; nothing system-wide). checks.sh's docs step uses it (-q).
  Sphinx -W on checkpoint 11 head 1a9a89cb: PASS. Control on the pre-fix fold head 560f3767: FAIL, 61 warnings
  (library.c-abi.rst "Error when parsing function declaration"), i.e. the step catches CI's docs failure.
  D:/gui_forms_int/gui_forms is left detached at 1a9a89cb.

## 2026-11-02: text-mask CI step investigation (logs only)

- macOS "Test development text-mask ownership": same tree reconfigured in place (no second tree) with
  HARFBUZZ_TEXT/TEXT_MASKS/PREPARED_TEXT/AUDIO_LOOP_TRANSPORT. PREPARED_TEXT adds PUBLIC define
  GUI_FORMS_PREPARED_TEXT (Painter vtable), so ~399 objects recompile (7.8 min). Tests 32 s, serial (two
  application_text_configure tests 14.5 s + 8.5 s). Cache design right, but the PR restored a day-old
  scheduled cache (main pushes are promoted, never save) after 63 public headers changed: misses. Warm: 29 s.
- Branch claude/sm-ci-devtext-parallel b1c57cda: ctest --parallel 3 in both text-mask steps (~20 s).
- Owner options open: make draw_prepared_text unconditional / capability-based (cold ~2 min); cache warming
  after merge.

## 2026-11-02: win32metadata study

- Written: desks/coordinator/WIN32_METADATA_STUDY.md (no downloads). Inventory tool
  D:/gui_forms_w3/dev/tools/win32_inventory.py, data D:/gui_forms_w3/dev/win32_inventory.json.
- Product: 196 functions / 10 DLLs, 19 COM interfaces (117 methods), ~35 structs, 12 macro helpers; binaries
  import 153 UCRT functions. Recommendation: option 3 (generated headers + .def import libs, stx_win32, after
  0.5, staged); CRT/startup/libc++ stay MinGW's.
- Next: whatever the coordinator assigns.

## 2026-11-02: prepared text unconditional (owner option 1 of the text-mask investigation)

- gui_forms claude/sm-prepared-text-unconditional a5834350 (on main 17f94617; stacks 9aed870c = the
  parallel-ctest CI change, rebased b1c57cda). Core/public headers no longer test GUI_FORMS_PREPARED_TEXT:
  Painter always declares draw_prepared_text; DisplayCommand, RecordingPainter, replay, visual inspection always
  carry the prepared command. The definition moved from gui_forms_core PUBLIC to gui_forms_skia PUBLIC (every
  skia_raster.hpp includer links it) plus Application as before; the option now only adds sources.
- Measured (Windows tree, Skia on): toggling the four dev options recompiles 34 objects, all option-specific
  (text masks, prepared impl, 2 skia, 2 application, 2 core prepared_window, tests); no Core/Controls object.
  Cold step build 77 s (-j4, ccache off) + ctest 48 s (--parallel 3), 16/16 pass. macOS adds the 7 host
  objects: estimate ~1.5-2 min cold vs 8.5 min.
- Verified: default-config full build, house style, lifetime, library docs (controls.diagnostics.rst regen),
  schema, registry, workflow tests, state gate, 24 display/inspection/raster tests plain, 9 sanitized.
- Sanitizer note: sanitize.sh needs USERPROFILE exported (ccache refuses without it) when run from a subshell.

## 2026-11-02: checkpoint 12 (D:/gui_forms_int, on main 17f94617)

- Fold head 7402d018 (checkpoint.py fold record), five topics in the coordinator's order: C ABI C5 w5
  c-abi-c5 f2107b9f (contains T4 t4-control-colours a74ac740); T4 phase 4 w1 t4-themes 883963e5; W w2
  w-dgv-tooltip-fallback f202eaaa (contains w-dgv-row-heights, w-dgv-batch1-rest, w-winforms-divergences);
  number formatter w3 sm-number-format 8212b78d (incl. I04 fix: buffer end = first + chars_capacity);
  prepared text unconditional w3 sm-prepared-text-unconditional a5834350 (incl. parallel ctest).
- Regeneration commit on the fold head: w3 claude/sm-cp12-regen 28ef6dca (schema.md, inventory.json,
  abi_event_catalog.hpp, stock_anatomies.inc, 3 display-trace digests). A regen cannot be a fold topic (its
  3-way squash conflicts with the topics' own edits of the same generated files), so checkpoint.py verify on
  7402d018 fails by construction (stale stock_anatomies / catalog / digests); `sh build.sh all` (verify's
  command) was run on 28ef6dca: build OK, 324/324 (bubble oracle skipped), CHECKS ran=32 failed=0 incl. Sphinx.
- abi_conformance's plain failure (C5's open question) = stale abi_event_catalog.hpp; passes once regenerated.
- Sanitized (ASan+UBSan) at 28ef6dca: the 25 tests matching abi|c_api|shadow_frame|binding, all pass.
- Digest note: states-and-diagnostics, values-and-spinners, typography-and-inspection change only in the
  combination (T4 recorded on main; others leave them); cause unconfirmed, for T4/C5.
- Not folded (not rebased onto 17f94617 at fold time): w4 hot-paths-gradient-axis 9c630e47,
  hot-paths-mingw-stage-abi 95b9e179, stx-conv-dispatch 166bc808 (contains the stx_auxlib pin).
- Env pitfalls: sourcing env.sh in a script loses git identity (commit failed); ccache needs USERPROFILE
  exported for build.sh/sanitize.sh run from background shells.

## RESUME POINTER (2026-11-02, end of night)

- Checkpoint 12 fold 7402d018 + regen 28ef6dca verified locally; reported to the coordinator for push.
  D:/gui_forms_int/gui_forms is detached at 28ef6dca.
- Queued for tomorrow: the control superset study (desks/coordinator/CONTROL_SUPERSET_STUDY.md; tdesktop
  ui/ clean-room inventory, GPL-3.0: behaviour only; secondary: WPF, Avalonia, Qt, GTK4/libadwaita,
  Modern.Forms, ImGui/ImPlot, DevExpress/Telerik/Syncfusion catalogues; cross-reference
  planning/CONTROL_COMPLETENESS_MATRIX.md, EXHIBITS_PLAN.md, TRAJECTORY_TO_1.0.md).
- Scheduled later: stx_win32 after 0.5 (WIN32_METADATA_STUDY.md option 3).

## 2026-11-03: harness regenerate, after-merge #462, control superset study

- template_harness claude/sm-checkpoint-regen 4bbee91 (local in D:/gui_forms_w3/template_harness, unpushed):
  `checkpoint.py regenerate <clone>:<branch>` records a regeneration branch built on the folded head
  (no merges), fast-forwards the tree, moves the fold record's head/tree, names it in the fold log (PR body),
  drops any verification; verify/push then cover fold+regen exactly; after-merge reports it. 4 new tests;
  CheckpointTests 16/16 (run from checkpoint/; from the repo root the module name collides with the package).
  Docs: workflow.md, README, contract.md, CHANGELOG.
- after-merge #462: contained = t4-themes (moved to 855b0f0e, still contained), w-dgv-tooltip-fallback,
  sm-number-format, sm-prepared-text-unconditional; c-abi-c5 reported "not yet" only because main carries the
  regenerated abi_event_catalog.hpp (merge-tree differs in that one generated file). Deleted my contained w3
  branches (number-format, prepared-text, cp12-regen, ci-devtext-parallel).
- Study written: desks/coordinator/CONTROL_SUPERSET_STUDY.md (clean-room: names/behaviour from file listings
  only; tdesktop GPL-3.0, lib_ui via desktop-app/legal).

## RESUME POINTER (2026-11-03)

- Waiting on the coordinator: harness branch 4bbee91 review/push; owner decisions 1-6 in the superset study.
- D:/gui_forms_int/gui_forms detached at 28ef6dca (checkpoint 12 regen); D:/gui_forms_w3/gui_forms detached at
  origin/main.

## 2026-11-03: checkpoint 13 fold (in verify)

- Fold on 855b0f0e, 7 topics: stx-conv-dispatch (w4, contains stx-auxlib-pin), hot-paths-gradient-axis,
  hot-paths-mingw-stage-abi, c-abi-c6 fcb37541 (w5), stx-skia-sk3 2331780a (w4, added by the coordinator),
  T4 pinned at 4635aef3 via w3 claude/t4-themes-cp13 (w1 moved on), w3 claude/sm-sanitizers-manual b21a560a
  (owner ruling: the Sanitizers job is workflow_dispatch-only, input run_sanitizers). Fold head 5932f017;
  regen w3 claude/sm-cp13-regen 278a290e (inventory.json). Scripts: D:/gui_forms_w3/dev/tools/cp13c.sh
  (fold+regen+verify), cp13v.sh (verify only); logs D:/gui_forms_int/dev/cp13c/.
- DROPPED: w2 claude/w-message-box-gaps 715fb281 — docs_sphinx fails on docs/library/docstrings/Window.rst:198
  (`HelpProvider`s, an unterminated RST role); and the sanitized run of the 6-topic fold with it had a
  heap-use-after-free in gui_forms_windows_accessibility_uia_gallery_tests (ClientThread calls
  Window::invoke after run_windows destroyed the window; the topic changed windows_host.cpp).
- Int workspace fixes: build.sh runs sanitize.sh with Git Bash's sh (MSYS2's sh drops USERPROFILE and ccache
  refuses to run); build.sh all now runs the whole sanitized suite (rc 200+); SK3 needs
  GUI_FORMS_ENABLE_SKIA=ON (int build.sh and cache switched). Regen commits: git identity must be passed
  with -c (env.sh puts MSYS2's git first).
- Foreground lock (coordinator, mandatory): wrap fold verifies and Gallery/UIA runs in
  python -B D:/malkuth/projects/tools/foreground_lock.py -- <command>.

## RESUME POINTER (2026-11-03, checkpoint 13)

- If the cp13c verify (D:/gui_forms_int/dev/cp13c/verify.log, all.log, sanitize.log) is green: report heads
  to the coordinator. Then the remote split (REMOTE_SPLIT_DESIGN.md): step 0 remainder, step 2 programs
  split prep, step 1 stx; each its own topic, with job-min saved.
- Flag: stx-conv.yml and stx-vorbis.yml still run remote sanitizer steps.

## RESUME POINTER (2026-11-03, stopped for the reboot)

- Checkpoint 13 verify INTERRUPTED (reboot), not passed. Fold head 5932f01704e4 on 855b0f0e (7 topics:
  stx-conv-dispatch, hot-paths-gradient-axis, hot-paths-mingw-stage-abi, c-abi-c6 fcb37541, stx-skia-sk3
  2331780a, T4 pin 4635aef3 (w3 claude/t4-themes-cp13), sm-sanitizers-manual b21a560a); regen w3
  claude/sm-cp13-regen 278a290e172f. MessageBox gaps dropped (see above). D:/gui_forms_int/gui_forms detached
  at 278a290e.
- Results so far (logs D:/gui_forms_int/dev/cp13c/all.log, D:/gui_forms_int/ctest.log, sanitize.log):
  build ok; plain 485/485; checks 35/35 (docs_sphinx ok); sanitized 320/320 passed, interrupted at #321
  gui_forms_windows_accessibility_uia_gallery_tests (known sanitizer timeout per the coordinator; on the
  6-topic fold with MessageBox gaps it failed with a heap-use-after-free instead).
- To resume: under the foreground lock, rerun `sh D:/gui_forms_w3/dev/tools/cp13v.sh` (verify only; the
  sanitize tree is built, so it is tests only), then report heads. Plain c_abi lines in cp13v.sh need env.sh
  sourced for ctest. No uncommitted work in w3 clones; no step-0 branch started yet.

## RESUME POINTER (2026-11-03, after the reboot): checkpoint 13 VERIFIED, reported

- D:/gui_forms_int/template_harness moved to origin/main 244ed63 (contains PR #15 and the 850b52d pin).
- `checkpoint.py regenerate D:/gui_forms_w3/gui_forms:claude/sm-cp13-regen` recorded regen 278a290e172f on fold
  5932f01704e4 (base 855b0f0e); `checkpoint.py verify` under the foreground lock: PASS (verification.json head
  278a290e172f, tree 43adadc8eb03). Build ok; plain 485/485 (1 skipped: stx_conv_stream_bubble_oracle); checks
  35/35; sanitized 485/485 (same skip); #321 uia_gallery sanitized passed in 1905 s (TIMEOUT 3600), #320 msaa
  gallery 526 s. Log D:/gui_forms_int/dev/cp13d/verify.log.
- Waiting for the coordinator's `checkpoint.py push`. Then: the CI reduction from desks/coordinator/
  REMOTE_SPLIT_DESIGN.md (step 0 rest, step 2 programs-split prep, step 1 stx), each its own w3 topic.

## 2026-11-03: CI reduction step 0 DONE (local, stacked in w3) — RESUME POINTER

- Checkpoint 13 pushed by the coordinator as gui_forms#463 (checkpoint/2026-11-03-1). When it merges: run
  `checkpoint.py after-merge` (HARNESS_* as above) and list contained branches, incl. side branches
  claude/stx-skia-sk4, claude/effect-programs, claude/effect-pass and old checkpoint/* branches.
- Step 0 stack in D:/gui_forms_w3/gui_forms on main a5fdb2e8 (#463 merged; after-merge run, w3 contained branches deleted) (fold top: claude/ci-cold-build):
  - claude/ci-programs-receipt 8c4b3a87: PR/merge-group programs jobs archive their NEW ccache files
    (listing diff) as gui-programs-objects-<platform> (7 d); main finds the tested run (release_tests.py merged,
    moved before the build) and adds them. Measured basis: run 41473847301 linux-x64 build 230 s, ~200 s misses.
  - claude/ci-nightly-on-change 5a468b2a: house-style step decides on schedule (skip if last successful
    scheduled run has the same head_sha, except Sunday); docs (now needs house-style) and native-* follow.
  - claude/ci-stx-paths 6192e267: harness.lock dropped from the 4 stx workflows' path filters.
  - claude/ci-deps-dedup 4262e099: gui-deps-v4-<runner.os>-hash; miniaudio cached at third_party/miniaudio.
  - claude/ci-cold-build 7b02f7c1: native.yml dispatch input cold_build (no object restore/seed, programs
    skipped); the coordinator dispatches the measurement.
- Checks: tools/test_workflow_*.py, test_release_tests.py, harness test_ci_cache.py; all workflows parse
  (dev/tools/yamlcheck.py, run with D:/gui_forms_int/.venv-docs python, which has PyYAML).
- Pitfall: in this Bash tool a backslash-newline inside a quoted heredoc is dropped; use Edit for such lines.
- Open for the coordinator: stx-conv/stx-vorbis/stx-skia still run sanitizer steps remotely (decision).
- Next: step 2 (programs split prep), then step 1 (stx), each its own stack.

## Queued after the CI reduction: the 0.5 reference touch-up (planning; executes after W's
## property-capabilities migration lands)

Owner framing: 0.5 = internal candidate sealing ABI, API (property capabilities) and languages (form,
theme), incl. threaded sub-surfaces and theming. The Gallery and demoboards are the FIRST customer and the
REFERENCE; PlaySuite, Plan Paint, SwiftEdit, the benchmark demos and the File Manager frontend convert after.
Scope (to confirm against the landed code before executing):
1. Roadmap (notes planning/ROADMAP_TO_0.5.md): rewrite "0.5 ABI candidate" around the seal (ABI listing,
   property capabilities, form + theme languages, threaded sub-surfaces, theming) with state/left/gates;
   move the converters from "Addendum 2026-10-31 ... concludes 0.5" to a post-0.5 "conversion" section
   (dogfooders, order kept); resolve the Addendum's three "unknown" relations where the owner has ruled;
   the stage boards (planning/stages/*/README.md state lines, FORWARD_PROGRAMME.md, the published board) to
   the same picture.
2. Documentation: the getting-started and library pages that show properties, forms and themes rewritten
   to the property-capability API and the form/theme languages; one "conversion guide" page for the post-0.5
   workers (old call -> new capability, theme sheet instead of direct paint, ABI from C/C#);
   generate_library_docs/docs_sphinx/control_schema checks.
3. template_harness example_app (today: greeter.cpp/main.cpp, gui_forms.lock, fetch_sdk.py): make it the
   minimal reference application: a window built from a form description, controls bound through property
   capabilities, one theme sheet, a command, a threaded sub-surface (if sealed), consumed from the installed
   SDK through the lock; its tests (headless model + one host smoke); docs/workflow.md "start a product"
   section pointing at it.
- Fold verifies from cp14 (coordinator 2026-11-03): run the ctest `foreground` label set on a private desktop via
  D:/malkuth/projects/tools/run_on_desktop.py (no lock); hold foreground_lock.py only for the `input_desktop` set
  (UIA, MSAA, compositor probe), once hot paths' labels land.
- QUEUED (owner-approved, after hot paths' input_desktop/foreground labels land): change-based selection of the
  input_desktop set (UIA, MSAA, compositor probe) in the local fold verify (checkpoint.py verify / int build.sh,
  checks.sh). Runs when the fold touches src/host/windows accessibility (UIA/MSAA), the semantic schema (roles,
  states, patterns, semantic snapshot types), compositor/presentation code, or the tests themselves; or on an
  explicit --input-desktop switch (seals, releases, occasionally). One checked path-map file; the verify log says
  which set ran or was skipped and why; never silent. Also list (do not move) input-desktop checks that only test
  semantics we own, as candidates for headless tests against the semantic snapshot. Local verify only.

## RESUME POINTER (2026-11-03, step 2 in progress)

- Step 0 stack + claude/ci-stx-sanitizers-manual cb1a8f25 (stx sanitizer steps dispatch-only) accepted for
  cp14 (top of the stack is now cb1a8f25). Remind the coordinator to dispatch cold_build once cp14 merges.
- Step 2: plan, inventory and decisions in tracks/gui_forms/programs_split_prep.md. P1 done:
  claude/split-gallery-model-option 8000c443 on cb1a8f25. Waiting on decisions 1 (SDK component vs
  add_subdirectory) and 2 (filter-repo download) before P2. Check script dev/tools/gm_check.sh (Windows
  ci_build native config + regex; logs dev/split2/).
- Then step 1 (stx repository prep).

## RESUME POINTER (2026-11-03, step 2 P2 done locally; stx coverage done)

- Owner rulings for step 2: (1a) installed SDK + GalleryModel component; (2a) git filter-repo; (3) launching
  tests move; (4) name gui_forms_programs. git-filter-repo 2.47.0 installed with `pip install git-filter-repo`
  (Windows Python 3.14, PyPI); wheel sha256 2cd04929b9024e83e65db571cbe36aec65ead0cb5f9ec5abe42158654af5ad83
  (taken by a separate `pip download` of the same wheel, then deleted).
- stx coverage (coordinator item f) DONE, not yet reported when written: projects tools/stx_sanitize.sh,
  verify_selection.json, verify_select.py (18d8118, pushed); D:/gui_forms_int/build.sh `all` runs the selected
  sets after SANITIZE (EXTRA_SETS=all runs all); gui_forms claude/stx-skia-sanitize-windows 61831856 (MinGW has
  no LSan: detect_leaks=0). Gaps: stx_conv oracle absent (fetch needs approval: falseywinchnet/plan-paint
  3edc4520), stx_vorbis fuzz stages need macOS/Linux (M4 handoff); int stx_skia fails until 61831856 is folded.
- gui_forms claude/gallery-model-component 2909df8b (on 61831856): GUI_FORMS_INSTALL_GALLERY_MODEL, export
  without link, build_sdk --features all. Windows SDK built at D:/gui_forms_w3/dev/split2/sdk (dev/tools/sdk_all.sh).
- Programs repo D:/gui_forms_w3/programs_split/gui_forms_programs, branch main, head 67092c89 (see git
  log): CMakeLists against the SDK (models compile-only against Controls; programs link model + Application;
  per-platform target names as packaging/programs.json), fetch_gui_forms.py + gui_forms.lock (commit pin,
  placeholder zeros until 2909df8b lands on main), AGENTS.md, README, LICENSE, packaging via
  GUI_FORMS_SOURCE_DIR. Windows: dev/tools/programs_build.sh 25/25; dev/tools/programs_smoke.sh on a private
  desktop (run_on_desktop.py; coordinator rule: never the owner's desktop): Gallery, Correspondence, demoboard
  pass --smoke-test; labs have no smoke option (launch, stay up, killed at 30 s).
- Open decisions sent to the coordinator: release split (SDK source release stays in gui_forms; programs repo
  publishes programs only, publish_tested_release drops package_sdk); layered_material_raster_tests (macOS,
  private Skia/CG) stays in gui_forms with demo/layered_material_lab.* kept as its fixture.
- Next: workflows (after the release decision), M4 handoff, removal branch (held), owner asks list, step 1.

## RESUME POINTER (2026-11-03, cp14 fold plan sent; programs split parked)

- Programs split (owner rulings 1a/2a/3 in force): programs repo D:/gui_forms_w3/programs_split/gui_forms_programs
  main 974bcfb4 (workflows; bundle sha256 054fa328...); gui_forms held removal branch claude/split-remove-programs
  7f2d3b58 (fold only after the programs repo's first green run); M4 handoff tracks/gui_forms/
  m4_handoff_programs_split.md; owner asks tracks/gui_forms/programs_repo_owner_asks.md; step 1 prep
  tracks/gui_forms/stx_split_prep.md (filter-repo needs --prune-degenerate never, then
  dev/tools/check_filtered_history.sh).
- stx_conv oracle RETIRED (owner ruling 3, 2026-11-03): claude/stx-conv-retire-oracle 6ba25983 (on 2909df8b)
  removes STX_CONV_ORACLE, the Plan Paint differential and float-oracle tests, fetch/prepare scripts; projects
  tools/stx_sanitize.sh adjusted (8bdea51). Verify selection does not depend on them.
- cp14: candidates fetched into w3 as refs/cp14/<w>/<branch>. All worker branches sit on 855b0f0e (main's first
  parent); cp13 squashed parts of them, so each is rebased dropping its cp13-folded prefix (w4 from 2331780a,
  w5 from fcb37541, w1 from 4635aef3). dev/tools/cp14_stack.sh builds the stacked topics in worktree
  D:/gui_forms_w3/cp14_trial (branches cp14/w3,w4,strip,fd6,msgbox,w5,w1; each onto the previous; trace digests
  take the topic side, CMakeLists/native.yml union). Clean through w5; T4 (w1) conflicts with the effect
  programs in range_control rendering/progress_bar (27ac7a78) — needs w1. Plan sent to the coordinator; fold
  waits for the forwarded MessageBox head, then checkpoint.py fold <w3 clone>:cp14/<topic> ..., regenerate
  (trace digests, docs), verify with the new labels.
- After cp14 merges: pin gui_forms.lock in the programs repo; remind the coordinator to dispatch cold_build.
- Queued: input_desktop selection (after labels land); the 0.5 touch-up (after properties land).

## RESUME POINTER (2026-11-03, cp14 folded; regenerate + re-verify pending)

- cp14 fold: D:/gui_forms_int/gui_forms checkpoint-fold bb284e6c on a5fdb2e8, THREE topics (squash folds conflict on
  adjacent hunks, so stacks are grouped): (1) w3 step 0 + GalleryModel + 6ba25983 + w4 (w3 clone cp14/w4);
  (2) FD1-FD6.1 (cp14/fd6only); (3) strip 83695883 + MessageBox 2716d8ee + w5 C ABI/effects/llvm-prune/SweepScope
  (cp14/t3w5). T4 -> cp15 after w1 rebases onto the folded tree.
- First verify: only gallery_display_trace failed (table from w5's old base); sanitize broke when its shell was
  stopped (0xC0000142, not a defect); stx_conv 10/10. Never kill the integration verify; wait for it.
- Regen topic claude/cp14-regen 929e2592 (w3 clone): 517a02f7 traces recorded on the fold (vs main only W's three
  MessageBox pages + ranges-and-progress differ); 929e2592 CI nightly decision moved to tools/ci_nightly_due.sh,
  house-style output renamed due (checks.sh executes run-block lines).
- Next: after the old verify exits: checkpoint.py regenerate D:/gui_forms_w3/gui_forms:claude/cp14-regen, then
  SKIP_FOLD=1 sh dev/tools/cp14v.sh; confirm launchers (foreground -> run_on_desktop, input_desktop -> lock); report.
- cp15 queue: T4 (w1 rebased), FD6.2 09034d31, Wine smoke fix claude/wine-smoke-collection-id ee3660ae.
- Programs split: the programs repo (main 95a9e60a incl. the Wine fix) is a REHEARSAL. Re-extract the programs history
  with the filter-repo script after FD6.2 lands in cp15 (coordinator 2026-11-03), re-apply the SDK commits, then the
  owner asks. After cp14 merges: remind cold_build. Pin the lock after the re-extraction's base merges.

## RESUME POINTER (2026-11-03, paused at cp14: verify FAILED on 6 static checks; owner pause for the day)

- Integration tree D:/gui_forms_int/gui_forms checkpoint-fold 929e2592 (fold bb284e6c + regen 517a02f7, 929e2592),
  base a5fdb2e8. Verify: BUILD 0, suite 516/516 (launchers checked in CTestTestfile: 3 foreground tests via
  run_on_desktop.py, 5 input_desktop via foreground_lock.py, 730 s), SANITIZE 0, stx_conv 10/10, stx_skia 138/138,
  stx_vorbis skipped by selection. CHECKS 6 of 37 failed (logs D:/gui_forms_int/dev/checks/):
  1-2 tools/form_compiler/form_compiler_catalog.hpp stale (FD5 emit-forms x SweepScope): regenerate.
  3 src/core/theme/stock_anatomies/stock_anatomies.inc stale: regenerate.
  4 paint census: progress_bar_effects.cpp 6 direct calls (baseline 0), range_control_rendering.cpp fell 25->12:
    w5's effect programs; needs `update` plus an exemption/decision (w5 or coordinator), not a mechanical regen.
  5 code integrity raised I05: src/core/engine/effect_registry/effect_registry.cpp:107
    static_cast<int>(name.size()) without a visible bound (w5).
  6 Sphinx: docs/api/library.geometry-and-appearance.rst:6270 `|x|` read as a substitution, from the
    effect_fraction docstring (w5): escape in the docstring, regenerate docs.
- Next on resume: regen items 1-3 (+docs) in a second regen commit on claude/cp14-regen; items 4-6 from w5 (or the
  coordinator's ruling), fold them as a regen/fix topic; re-verify with SKIP_FOLD=1 dev/tools/cp14v.sh; report.
- After cp14 merges: remind the coordinator to dispatch cold_build. cp15: T4 (w1 rebased), FD6.2 09034d31, Wine fix
  ee3660ae. Programs split: re-extract the programs history after FD6.2 lands (filter-repo --prune-degenerate never +
  dev/tools/check_filtered_history.sh), re-apply the SDK commits, then pin gui_forms_programs' gui_forms.lock to
  that main; the current programs repo (95a9e60a) is a rehearsal. Then step 1 (stx extraction, stx_split_prep.md).

## RESUME POINTER (2026-11-03, resumed after shutdown: cp14 regen 2 done; waiting on w5 fixes)

- claude/cp14-regen now 221d2480 (w3 clone): + form_compiler_catalog.hpp and stock_anatomies.inc regenerated
  (SweepScope; anatomy tables renumber from stock_13). generate_library_docs --check already passed; docs regen
  again after w5's docstring fix.
- Plan for w5's fixes (census classifier + baseline, effect_registry.cpp:107 bound, effect_fraction `|x|`):
  append them to cp14/t3w5 in the w3 clone (they belong to that topic), refold all three topics (cp14v.sh fold
  command), rebase claude/cp14-regen's 4 commits onto the new fold head, regen docs if needed, then
  checkpoint.py regenerate D:/gui_forms_w3/gui_forms:claude/cp14-regen, then SKIP_FOLD=1 sh dev/tools/cp14v.sh.

## RESUME POINTER (2026-11-03, cp14 verify GREEN; waiting for the coordinator's push)

- w5's fix 76cd6a7b cherry-picked onto cp14/t3w5 as 30ad0e61 (census manifest conflicted on totals; recomputed with
  paint_census.py update on the topic tree: only w5's intended entries changed). Refolded (dev/tools/cp14_refold.sh):
  fold head cd652af4 (topics 008d8064, 4bde249d, cd652af4); claude/cp14-regen rebased to 22fbaedd (0addd596 traces,
  3f8903c3 nightly script, 22fbaedd catalog + anatomies); checkpoint.py regenerate recorded it.
- Verify on 22fbaedd: BUILD 0, suite 516/516, CHECKS 37/37, SANITIZE 0 (516/516), stx_conv 10/10, stx_skia 138/138,
  stx_vorbis skipped by selection. Logs D:/gui_forms_int/dev/cp14/verify.log.
- After the push merges: pin gui_forms_programs' lock (after the re-extraction's base), remind cold_build; then the queue.

## RESUME POINTER (2026-11-03, #464 gate fix sent; step 1 skeleton started)

- #464 (checkpoint/2026-11-03-1) state-machine-gate failed in CI: generate_headers now builds
  gui_forms_forms/note_dialog_form.hpp (FD5 gui_forms_add_form test), which needs the form compiler compiled and linked;
  output was hidden. Fix: gui_forms claude/cp14-gate-fix e8d5ceb1 (worktree D:/gui_forms_w3/gate_wt, on 22fbaedd):
  option GUI_FORMS_BUILD_GENERATED_FORM_TESTS (ON; gate OFF), harness.lock -> template_harness
  claude/sm-gate-ninja-output 4138d2b (on harness main 244ed63; prints ninja stdout/stderr on failure). Both need
  pushing by the coordinator (harness first). Fresh-tree gate on Windows: dev/tools/gate_fresh.sh rc 0, no F1/F2.
  The Linux root cause of the ninja failure is not seen (the old tree passed on Windows); the new output will show it if
  it recurs.
- Step 1 (stx repo): skeleton in D:/gui_forms_w3/dev/stx_skeleton (root CMakeLists adding the five packages with
  STX_BUILD_TESTS/STX_WITH_VORBIS, LICENSE, .gitattributes, .gitignore, harness.lock, harness-checkout action with
  product-path .). Not yet built. FINDING: stx-conv.yml needs tools/resampler_face_study (self-contained) — add it to the
  extraction paths. Next: extraction script from main after #464, apply skeleton, build root on Windows, README/AGENTS.md.
- After #464 merges: checkpoint.py after-merge; list claude/stx-skia-sk4, claude/effect-programs, claude/effect-pass for
  deletion; prep cp15 (T4 rebased, FD6.2 09034d31 + fd6-pages 998e6b8b, Wine ee3660ae, page-cpu 68059db0,
  stx-skia-avx2 0032b0b6).
- SPLIT (coordinator): #464 gets gui_forms claude/cp14-gate-option f221ba88 (on 22fbaedd; option + state-machines.yml only,
  harness.lock unchanged). Fresh-tree gate with the pinned harness 850b52d (worktree D:/gui_forms_w3/harness_850,
  dev/tools/gate_fresh_850.sh): rc 0, no F1/F2. Harness 4138d2b goes up separately (owner PR); lock bump in a later
  checkpoint. e8d5ceb1 (claude/cp14-gate-fix) is superseded.
- Harness bootstrap repetition fix (owner allergy, 2026-11-03): template_harness claude/sm-gate-ninja-output now
  4138d2b + 3924fba (PR #16 carries both; owner merges). Plan-level input filter (BOOTSTRAP_INPUTS), per-tree receipts
  (bootstrap-receipt-<project>-<platform>-<tree>, lookup on push to main), warm ws/.ccache Actions cache for projects
  without a published cache (file_manager), cold_build dispatch input. Harness tests 442 OK (29 skipped).
  Caveat: PR-scoped caches are invisible to main and other PRs; main only saves when a main run actually builds.
- Follow-up (file_manager smoke runs GUI.Forms' whole suite): gui_forms claude/ci-build-install-only 03a31134 (on 22fbaedd,
  worktree gate_wt): ci_build.py --install-only (build, install, notice/import checks, reference examples; no toolkit
  ctest); test_ci_build 13/13. file_manager patch D:/gui_forms_w3/dev/fm_smoke/build_native_smoke.patch against
  file_manager main e4850281 (tools/build_native.py blob 64bd9bae): smoke passes --install-only and its manifest says
  so. It lands only after file_manager's gui_forms submodule pin (now 8e288666) includes 03a31134. Via the coordinator.
- Next: step 1 (stx skeleton build), after-merge + cp15 prep once #464 merges.

## RESUME POINTER (2026-11-03, #464 merged = main feacf6d4; cp15 trial stack; step 1 rehearsal)

- #464 merged (main feacf6d4; c0a82da6 on top was the real libstdc++ cause: StorageRelease at namespace scope).
  after-merge: claude/cp14-regen contained. cp14/w4, cp14/fd6only, cp14/t3w5 show "not yet" only because they sit on
  the old base (merge-tree conflicts with later main edits); every commit is in main by subject. Side branches
  claude/stx-skia-sk4 (origin 1b00acdc), claude/effect-programs (6f7d1b5b), claude/effect-pass (24f966f1): all their
  commits are in the cp14 topics (by subject) -> deletable. Local w3 deletable: claude/cp14-regen, cp14/* topics,
  claude/cp14-gate-fix (superseded), claude/cp14-gate-option (in main).
- cp15 trial (dev/tools/cp15_stack.sh, worktree D:/gui_forms_w3/cp15_trial, branches cp15/<topic> in w3 clone):
  install (claude/ci-build-install-only) e55744db 1; wine (ee3660ae) 618503ba 1; w4 (page-cpu + stx-skia-avx2, from
  3613ed5b) 967c4156 4 commits; fd6 (W fd6-pages 998e6b8b from 1febc847) STOPS at 171c59f5 (D11 framework fixes) in
  src/controls/button_base/button/button.cpp vs main's 193a570c (T4 phase 4): W must rebase fd6-pages onto feacf6d4.
  T4 waits for w1's rebase. Rule added: the Wine smoke script takes the wine side + FD6.2's "form-document" wording.
  Harness lock bump (4138d2b+3924fba) after template_harness #16 merges.
- Step 1: dev/tools/stx_extract.sh <ref> <out>: filter-repo (5 packages + tools/resampler_face_study + 3 workflows,
  --prune-degenerate never), check_filtered_history, skeleton commit (dev/stx_skeleton: root CMakeLists, README,
  AGENTS.md, LICENSE, .gitattributes, .gitignore, harness.lock, harness-checkout action). Rehearsal from 22fbaedd:
  D:/gui_forms_w3/programs_split/stx_rehearsal main f21ab8ad, 727/727 commits identical; root build on Windows
  (dev/tools/stx_root_build.sh) configure/build/27 tests/install OK. stx_conv_stream_bubble_oracle still registered
  (skipped) after the oracle retirement: small follow-up. Waiting on the owner: create the stx repository; decisions
  1-3 of stx_split_prep.md (unruled as far as recorded).

## RESUME POINTER (2026-11-03, stx extracted from main feacf6d4; ready to push; removal branch held)

- **stx local repo** `D:/gui_forms_w3/programs_split/stx`, branch main, head **24a6a8ee** (728 filtered commits +
  the root commit). `dev/tools/stx_extract.sh feacf6d4 <out>`; check_filtered_history.sh: 728/728 trees identical,
  0 mismatches. Root commit adds (dev/stx_skeleton): root CMakeLists (5 packages; STX_BUILD_TESTS, STX_WITH_VORBIS,
  STX_WITH_AUXLIB_SIMD), stx_auxlib.lock + cmake/StxAuxlib.cmake (gui_forms' copy, so stx_conv keeps avx2 lane
  selection and installs stx_auxlib_simd), README, AGENTS.md, LICENSE, .gitattributes, harness.lock, harness-checkout
  action, and the three workflows at the root: no path filters, push main + pull_request + merge_group, job names
  `packages|conv|vorbis (<runner>)`, cancel-in-progress only on PRs.
- Windows root build (dev/tools/stx_root_build.sh): configure/build OK, 33/33 tests, install OK (6 packages incl.
  stx_auxlib_simd); lanes scalar;v128;avx2.
- Owner asks: `tracks/gui_forms/stx_repo_owner_asks.md` (settings, 12 required checks).
- **Held removal branch** `claude/split-remove-stx` **84a26dd6** (worktree D:/gui_forms_w3/stx_rm_wt, from origin/main feacf6d4;
  Windows against the local stx: configure, full build, 8 stx-facing tests incl. audio/stx_vorbis, install with all
  stx packages + 12 stx licences; test_ci_build 11 OK; house style 0; dev/tools/stx_rm_build.sh):
  stx.lock (24a6a8ee) + cmake/Stx.cmake (FetchContent of the pinned commit or FETCHCONTENT_SOURCE_DIR_GUI_FORMS_STX
  override, or GUI_FORMS_STX_PREFIX; configure-time build+install into <build>/stx-prefix with this build's
  compilers/flags, stamped; find_package; SDK install copies the prefix whole); stx dirs, face study, stx workflows,
  stx_auxlib.lock and cmake/StxAuxlib.cmake removed; fuzzers and the rtf2md corpus test read GUI_FORMS_STX_SOURCE_DIR;
  fuzz-smoke checks out stx at the lock. Open: codec dev tools (tools/benchmark_*, validate_*, review_deflate_memory)
  still compile stx paths at ROOT (decision sent: move them to stx).
- Branch cleanup: 44 local w3 branches contained in main deleted (dev/branch_audit.txt); kept cp14/w1, cp15/*,
  worktree branches.
- cold_build run 41758939023: success; queued 02:10Z, jobs from 02:31Z; builds linux-x64 22.7 min, linux-arm64
  26.6, windows-x64 41.7, macos-arm64 45.3; run total 67 min.
- cp15: waiting for W (fd6-pages) and T4 (w1); add 3ecf8fbc (oracle registration) to the fold.

## RESUME POINTER (2026-11-03 late, reboot stop; nothing uncommitted, no runs of mine)

- stx main 24a6a8ee PUSHED; first CI green (12 jobs; names confirmed in stx_repo_owner_asks.md).
- Held gui_forms `claude/split-remove-stx` 84a26dd6 (worktree D:/gui_forms_w3/stx_rm_wt): verified on Windows (see above).
- gui_forms `claude/stx-conv-oracle-registration` 3ecf8fbc (gate_wt): for cp15 and to carry into stx.
- NOT STARTED (owner ruling): stx PR branch off stx main 24a6a8ee in D:/gui_forms_w3/programs_split/stx moving
  tools/benchmark_deflate|inflate|png|match_index.py, validate_inflate|png.py, review_deflate_memory.py, the codec
  oracle manifests (tests/compression/oracle_sha256.json etc.) and codec-differential.yml into stx, plus 3ecf8fbc's
  stx_conv part (dev/tools/bubble_oracle_patch.py applies it); then the matching removal amended onto
  claude/split-remove-stx (and drop those files there). Then cp15 when W and T4 report.

## RESUME POINTER (2026-11-04, after the reboot: stx codec PR branch ready; cp15 waits on W and T4)

- stx `claude/codec-dev-tools` **24cc718f** (D:/gui_forms_w3/programs_split/stx, on stx main 24a6a8ee; the coordinator
  pushes): 2bce47cf bubble-oracle retirement (= 3ecf8fbc's stx_conv files exactly), 24cc718f codec scripts in tools/,
  manifests in stx_zlib/tests + stx_png/tests, harness sources in stx names (STX_ZLIB_INFLATE_ORACLE, STX_PNG_ORACLE,
  STX_*_FUZZ_STANDALONE, stx_png_oracle), tools/codec_bench (historical GUI.Forms baselines + seam header copies),
  tools/CODEC_EXPERIMENTS.md, codec-differential.yml, 4 new package tests. benchmark_inflate.py needs --gui-forms or
  --baseline. Scripts: dev/tools/codec_move.py, codec_move_docs.py (from gui_forms feacf6d4); check
  dev/tools/codec_stx_check.sh (bash): root 36/36, sanitized zlib/png 7/7, packages alone, house style 0.
  Oracle scripts not run (need pinned zlib/libpng).
- Held gui_forms `claude/split-remove-stx` now **df3cfd59** (84a26dd6 + codec removal via dev/tools/codec_remove.py;
  worktree stx_rm_wt). Before folding: stx.lock -> the stx main commit that contains 24cc718f (fuzz-smoke reads
  stx/stx_zlib/fuzz/...). Windows against local stx 24cc718f: build all, 7 stx-facing tests, install; test_ci_build 11 OK.
- Harness lock bump: gui_forms `claude/harness-lock-fb752e7` ea6ae6cf (worktree D:/gui_forms_w3/hlock_wt, on feacf6d4);
  #16 merged as fb752e7.
- cp15 candidates: install e55744db, wine 618503ba, w4 967c4156, hlock ea6ae6cf, 3ecf8fbc (oracle registration),
  W fd6-pages + T4 t4-themes (heads from the coordinator). Then checkpoint.py fold/regenerate/verify; then programs
  re-extract (FD6.2), owner asks, held removal branches.

## RESUME POINTER (2026-11-04, cp15 folded; verify running)

- cp15 fold in D:/gui_forms_int/gui_forms: head 746901f52326 on feacf6d4, 4 topics from the w3 stack (dev/tools/cp15_stack.sh,
  order install, w4, wine, fd6, oreg, hlock, w7; fold dev/tools/cp15f.sh): cp15/w4 (install-only + page-cpu/avx2),
  cp15/fd6 (wine + W fd6-pages f33059cb incl. FD6.3; W's clone has since moved to fc7d3b6e, not taken), cp15/hlock
  (3ecf8fbc + harness fb752e7), cp15/w7 (19730f2e headless semantic tests). T4 -> cp16 (coordinator ok).
- Regen claude/cp15-regen cd8c1c98 (worktree D:/gui_forms_w3/cp15_regen_wt): paint census; typography_specimen 6->7 by
  coordinator ruling (line boxes = diagnostic inspection content), gallery_controls 45->0.
- int template_harness moved to fb752e7. projects tools/sanitize.sh has SKIP_LABELS (5a4d89b); cp15v.sh sets
  SKIP_LABELS=input_desktop (coordinator: input_desktop plain only, by label).
- Verify: sh dev/tools/cp15v.sh (SKIP_REGEN=1 to only verify); output D:/gui_forms_int/dev/cp15/cp15v.out, verify.log.
- cp16 trial (T4 5ebbc402 onto cp15/w7): first stop ecc39cad conflicts only on track_bar.cpp includes (union) and display_trace_tests.cpp (topic side, regen); worktree D:/gui_forms_w3/cp16_trial branch cp16/t4.
- 2026-11-04 BUILD RULES (owner): I am the batch builder. Every heavy run goes through
  `python -B D:/malkuth/projects/tools/build_slot.py -- <command>` (max 2 machine-wide, re-entrant); the fold verify takes one
  slot for its whole run. Batch failures go back to the owning topic; the rest of the batch stays.
- cp15 first verify: all gates passed but sanitize.sh was rewritten mid-run (coordinator 33248d6) -> rerun with
  SKIP_REGEN=1 VERIFY_SKIP_SETS=input_desktop (set passed 5/5 sanitized on head 2d6e3dcc; note in the fold log). Tell the
  coordinator when a fold verify starts and ends (they hold tools/*.sh edits meanwhile).
- cp16 trial (dev/tools/cp16_stack.sh, cp16_conflicts.sh): order fd6b c1484f8b, w7b d04b56b2, s1 (S1.2 d1f5aa8e + S2.0 f5a780d7),
  m0 35c3a970, fx (w8 rebasing onto S1.2), T4 (w1 rebasing); hot paths if in time. Fold after cp15 merges.

## RESUME POINTER (2026-11-04, owner pause: #465 profiles fix committed; cp16 not started)

- #465 (cp15, checkpoint/2026-11-04-1 cd8c1c98) failed the stx_skia profiles job (run 41823704720). Fix: local commit
  **882ef1c2** on branch claude/cp15-ci-fix (w3 clone, worktree D:/gui_forms_w3/cp15_fix), on cd8c1c98. The coordinator pushes it to
  the PR; I never push. (1) simd.cpp is added only when STX_SKIA_BASELINE is off (the reference has none). (2) SkOpts_ml3.cpp
  no longer pre-includes src/core/SkUtils.h: its 256-bit load/store/bit-cast helpers must sit in the AVX target region
  (SysV ABI; Windows never diagnoses). Checked with dev/tools/cp15_profiles.sh (the job's steps through Git's sh; MSYS2 sh
  drops PROCESSOR_ARCHITECTURE, and the configure then fails with processor "") against the reference D:/gui_forms_w5/stx_skia
  (c20afa3c, read-only): candidate 139, baseline 93, scalar 138, v3 138, compares OK. dev/tools/ml3_sysv.sh is a cross-target
  SysV compile of the ml3 units (180 ABI errors before, 0 after). Only CI can prove arm64 Linux.
- cp16 (fold only after #465 lands; checkpoint.py fold/regenerate/verify under build_slot; tell the coordinator when the
  verify starts and ends). Order: fd6b c1484f8b, w7b d04b56b2, S1.2+S2.0 (d1f5aa8e + f5a780d7), hot paths 94dbbb2a, m0 35c3a970,
  fx **f61151aa** (claude/fx-cp16, D:/gui_forms_w8/cp16/gui_forms; 6 commits; EffectInstance is a friend of SurfaceRoot), T4
  **2644e5c2** (claude/t4-cp16, D:/gui_forms_w1/gui_forms; replaces t4-themes and t4-variants). fx and T4 sit on the old m0
  c8439042, so rebase them onto the current stack (the trial's m0 is bb709955). Then a regen commit: docs/reference/contracts/window.json
  still lists Window::root (moved to SurfaceRoot), so move the entry to the SurfaceRoot contract; regenerate inventory.json and the
  display-trace digests. Trial: dev/tools/cp16_stack.sh (fx and t4 entries now point at refs/cp16/w8/fx-cp16 and
  refs/cp16/w1/t4-cp16, upstream c8439042; not yet rerun with them).
- cp17 candidates: S2.1 ff059e69 (w5) together with the assert sweep 7aa159bd (remove the two live_surface pending entries);
  M0.1 b8d17f0d (w2); T4 fonts after its rebase; held claude/split-remove-stx df3cfd59 (stx PR #1 merged: move stx.lock to the
  stx main commit containing 24cc718f once cp15 lands).
- After that: checkpoint.py after-merge; programs re-extract (FD6.2 in); gui_forms_programs owner asks; split-remove-programs.
- int build.sh: SKIP_LABELS=input_desktop set permanently (the verify_select input_desktop set runs it sanitized when touched).

## RESUME POINTER (2026-11-07: cp20/trial 200a0f24 on main 00c78e6c; 3 open items)

- cp19 merged (main 00c78e6c). cp20/trial 200a0f24: S6, S7 part 1, S5, glass, fd child props, 2c2f251a (rst
  code spans), c404fc9d inventory regen (older), T4 (a) 40ba3bc1 + 2863db55, P-5 c2501119, 200a0f24 ABI event
  catalog regen (P-5's data_grid_view_types.hpp).
- Full round on 4b8db724 (before 200a0f24): all targets; programs + demoboard 2/2; plain 511/512 (event catalog,
  fixed by 200a0f24); sanitized 484/486 (event catalog; sub_surface_tests "both pointer moves reach the
  sub-surface's control", tests/sub_surface_tests.cpp:1590, intermittent: passed alone; w5 to root-cause);
  checks: registry (DataGridViewCells missing, w5), api_reference stale (regen at the end), harness shims.
- Focused T4 (a) round: theme|relocate|gallery|doc_examples|binding 48/48 plain, 46/46 sanitized.
- Waiting for: w5 registry fix + S7 part 2 (on P-5) + the sub_surface race; MonthCalendar; FD-4; T4 (b).
- At the end: .fix_contracts.py + inventory regen (.cp20_round.sh does it), digests if pages moved, full
  .cp19_verify.sh, report.
- Sanitized runs in my scripts carry --skip-labels 'area_host|area_a11y|gallery_accessibility|foreground'.

## Previous RESUME POINTER (2026-11-05 reboot: cp18 head b431adc7 handed over for push with auto-merge; verify incomplete)

FIRST ON RESUME: fix whatever the CI or a local run finds on cp18. Then run the full verify:
`GUI_FORMS_BUILD_PRIORITY=critical` in build_slot.py's env; fetch the head into /d/gui_forms_int/gui_forms, then
`sh /d/gui_forms_int/build.sh all`, with input_desktop.

Fold state:
- cp18d/trial = cp18/final = b431adc7, in D:/gui_forms_w3/cp16_trial, 36 commits on origin/main ebb9fbf4.
- Order:
  - W through M2d 84ec483f;
  - C ABI S2.2..S3.2, then 338eef16 (anatomy);
  - T5, T4, FD, the overlay rename (594cadb2, re-resolved with dev/tools/rename_blocks.py), the leftover renames;
  - 87f72562 (live-surfaces M2 spelling);
  - e4e53032 (the lab keeps its overlay);
  - 417e9a77 (575b580e, the semantic node, adapted to .read());
  - 27a0c7db (owner rulings: OverlayAttachment owns the overlay root; ~Control -> SurfaceRoot::forget_destroyed_subtree
    backstop; ~SurfaceRoot forgets what is still attached; core_tests cases test_window_owns_a_shown_overlay and
    test_destroyed_attached_control_is_forgotten);
  - b431adc7 (regeneration).

Verify state:
- On b431adc7: every generator --check passes; house-style spelling is clean on the touched files.
- On 87f72562 + regen: the full build, and 39/41 touched tests (semantic_gallery and compositor_lab fixed since).
- Packaging tests and stx_skia profiles passed on cp18b.
- NOT run on b431adc7: whether 417e9a77/27a0c7db build at all (a background run of core_tests, compositor_lab,
  overlay, semantic_gallery etc., plain and sanitized, was in flight at the reboot; its output is lost), the full
  suite, the sanitized suite, input_desktop, the state gate, docs/Sphinx.
- The reported W change: ~Control releases its table slot via forget_destroyed_subtree.

stx move (parked): see the stx section below.
- stx PR #2 (a162112) and PR #3 (e35e312) are open.
- The gui_forms consume branch claude/consume-stx-skia is at 89bb0ac7 in D:/gui_forms_w3/skia_consume_wt. Phase 2
  (md4c/sheenbidi/unicode/libunibreak/fonts from stx) is not done.
- After cp18 lands: re-pin stx.lock to the #2/#3 merge, rebase consume onto main, drop "stx_skia/" from
  verify_selection.json.

cp19 candidates: none folded beyond cp18; ask the coordinator.

### (older) cp18d details

- cp18d/trial supersedes cp18b and cp18c. Fold order:
  - W through M2d 84ec483f;
  - C ABI S2.2..S3.2, then 338eef16 (anatomy; f262c95b's extra rst comes from regen);
  - T5, T4, FD, the overlay rename, the leftover renames;
  - 87f72562 (live-surfaces `accessible_name =`);
  - e4e53032 (the compositor lab kept its overlay alive: UAF through M0's control table; ownership decision sent
    to the coordinator).
- The uncommitted regen in int is current except for the next C ABI commit: API docs, inventory, contract keys,
  control-schema.json, schema.md, digests (4 pages explained in the comment).
- Open: semantic_gallery_tests fails (SubSurfaceHost publishes no node; its group refuses focus) until the C ABI fix arrives.
- Then: fold that commit, re-run cp18_regen.sh + cp18_regen2.sh (re-apply the digests, then control_schema render),
  commit the regen in int and fetch it into cp16_trial, run the full verify (critical slot), send the head.

### (older) cp18b details

- cp18b/trial in D:/gui_forms_w3/cp16_trial, on origin/main ebb9fbf4. Order:
  - W = claude/m2-control-family b47cf3b6, the whole branch (coordinator: it replaces eb4d7777);
  - C ABI S2.2..S3.2;
  - hot paths (T5; window.cpp mark_dirty conflicts resolved to ours, i.e. M0's settle_dirty);
  - T4;
  - effects (FD/G3 `classes` moved into M2's property table after cursor; count 23);
  - the overlay rename 594cadb2 (conflict blocks resolved to ours with the renames re-applied:
    dev/tools/rename_blocks.py);
  - c146bd75 (leftover renames, and the CMake theme FORMS list);
  - 1d75c539 (provisional SubSurfaceHost registry record, anatomy_pending stopgap,
    schema test records the host).
- The old cp18/trial (without M2) is kept for reference.
- Uncommitted regen sits in D:/gui_forms_int/gui_forms (int is at 1d75c539):
  - API docs and inventory (api_reference with --clang D:/a/_temp/msys64/clang64/bin/clang++.exe, run IN THE INT TREE,
    since paths are filtered by ROOT);
  - contract keys (dev/tools/fix_stale_contracts.py);
  - catalogs;
  - the display-trace digests (only FD6.3's 3 pages differ from the rename branch; M2 moved none).
  - Scripts: dev/tools/cp18_regen.sh, cp18_regen2.sh, cp18_regen3.sh.
- Blocker: SubSurfaceHost has no T1 anatomy, so the schema writer fails. Coordinator option 1: the C ABI worker sends one
  commit on ee26abc4 (anatomy, stock_anatomies, registry record, live-surfaces page). Cherry-pick it, then drop
  my anatomy_pending stopgap / provisional record where it replaces them. Re-run the regen (registry, catalogs,
  anatomies, schema write via GUI_FORMS_CONTROL_SCHEMA_WRITE=1 on gui_forms_gallery_pages_tests, live-surfaces digest),
  commit "cp18 fold: regenerate", then the full verify with GUI_FORMS_BUILD_PRIORITY=critical in build_slot's env
  (input_desktop included), and send the head to main.
- Already green on this tree:
  - the packaging unit tests (programs, sdk);
  - stx_skia profiles ALL-OK (dev/tools/cp18_side.sh; reference c20afa3c exported from w5's clone into
    dev/stx_skia_ref_c20afa3c).
- The stx move stays parked (next section).

## Previous RESUME POINTER (2026-11-05, stx move: stx PRs #2 and #3 open; gui_forms consume branch building)

- cp17 = #467 (coordinator pushed). My #467 fix: cp17/trial **7f5d4648** (packaging notices read $STX/<pkg>/ from
  the build's stx checkout; package_programs.stx_source_directory; tests 19/19). Sent.
- **Owner job (2026-11-05): everything stx_ and every third-party source into malkuth-software/stx.**
  stx clone D:/gui_forms_w3/skia_move/stx (core.autocrlf false):
  - **stx PR #2** claude/stx-skia-move **a162112**: af044fd stx_skia history merged in (filter-repo of gui_forms
    1ff3daa7 over stx_skia/, stx-skia.yml, tools/check_stx_skia_style.py+test; 62 commits, 0 mismatches;
    dev/tools/skia_extract.sh), 18b488d root build (STX_WITH_SKIA, STX_SKIA_SWIZZLE_ONLY now installs the swizzle
    package), stx-skia.yml on every change + style/lifetime + root-install consumer, tools/harness_link.py, docs;
    a162112 third_party/freetype (whole) + harfbuzz (minus test/ perf/, owner option 2) + intake notes (FTL)
    (dev/tools/text_stack_import.sh, text_stack_check.sh: every kept blob = pin). Windows check
    (dev/tools/skia_stx_check.sh): root 175/175, install 7 packages, consumers, swizzle-only, style, lifetime OK.
  - **stx PR #3** (stacked on #2) claude/stx-third-party-rest **e35e312**: libunibreak (pin, whole), md4c, sheenbidi,
    unicode (from gui_forms 7f5d4648), Noto CJK JP + Emoji + OFL (sha256 ok), NOTO_FALLBACK_FONTS_INTAKE.md
    (dev/tools/third_party_rest_import.sh).
- **gui_forms consume branch** claude/consume-stx-skia (worktree D:/gui_forms_w3/skia_consume_wt, on cp17/trial
  7f5d4648), UNCOMMITTED at this point: stx_skia/, stx-skia.yml, check_stx_skia_style*, fetch_text_stack.sh and the
  HarfBuzz intake record removed; Stx.cmake finds stx_skia (swizzle-only when Skia off), forwards
  CMAKE_<LANG>_FLAGS_<TYPE>, text stack from GUI_FORMS_STX_SOURCE_DIR/third_party; GUIFormsConfig find_dependency
  (stx_skia); licences from the prefix/stx source; packaging $STX for Skia/stx_skia/HarfBuzz/FreeType (required);
  package_sdk vendors stx at third_party/stx (toolkit_configure passes it as FETCHCONTENT_SOURCE_DIR_GUI_FORMS_STX);
  workflows drop fetch_text_stack and the FT/HB cache paths; code-integrity reads include src (14 stx_skia reviews
  dropped); docs. stx.lock = a162112 (re-pin to the #2 merge commit). Patches: dev/tools/consume_*_patch.py.
  Build/test: dev/tools/consume_build.sh (output dev/consume_build.out).
- projects tools (mine to commit): stx_sanitize.sh reads the package from the stx checkout at stx.lock when the
  gui_forms tree no longer has it; verify_selection.json stx_skia set paths += stx.lock (drop "stx_skia/" when the
  consume branch folds: verify_select refuses a missing prefix).
- Next: consume build result; commit; phase 2 on the consume branch after #3 (md4c/sheenbidi/unicode/libunibreak/
  fonts from the stx checkout; remove fetch_linebreak.sh, fetch_showcase_fallback_fonts.sh and the dirs); report.
- Open question to the coordinator: the other third-party fonts in assets/fonts (Carlito, Cousine, Arimo, Tinos,
  Selawik, Noto Arabic/Hebrew/Devanagari/Bengali/Gurmukhi) and code_integrity/const_restrict/loop_branch decisions
  for stx paths.

## Previous RESUME POINTER (2026-11-05, cp17 VERIFIED and reported; waiting for the coordinator's push)

- cp17/trial head **1ff3daa7** in D:/gui_forms_w3/cp16_trial on origin/main edfad055: 2c1cbcd7 + 1ff3daa7 (compositor_lab_tests'
  3 switches get failing default arms with reasons, gate F1; engine_recipe_field_tests names the 5 font recipe fields = T4
  ad9fb505's test hunk). Nothing uncommitted.
- Verify (int tree at 1ff3daa7, GUI_FORMS_BUILD_PRIORITY=critical on every build_slot call, a coordinator rule): build rc 0,
  plain 501/501, sanitized 496/496, checks 39/39, stx_skia sanitized 139/139, input_desktop sanitized 5/5
  (dev/cp17_full_verify.out, dev/cp17_extra_sets.out). build.sh `all` exited 250 only because projects
  tools/verify_selection.json still maps the deleted stx_conv/ and stx_vorbis/; the selected sets ran via
  dev/tools/cp17_extra_sets.sh. Reported to the coordinator (who owns the json) with a recommendation to drop the two sets.
- after-merge (HARNESS_PROJECT=D:/gui_forms_int/template_harness/projects/gui_forms.toml, run in D:/gui_forms_int) reports the
  cp15 record only: claude/cp15-regen is contained. The auto-mode classifier refused `branch -D`, so that is left to the coordinator.
- Next: when cp17 lands, rebase/drop cp17/trial; then cp18 (T5 backdrop, S2.2 + S2.3, W/T4 items) and the X-1/X-2 docs
  touch-up with the programs re-extract.

## Previous RESUME POINTER (2026-11-05, restart: cp16 merged; cp17 folded, verify stopped with 2 failures to fix)

- cp16 done: #466 merged (main edfad055 = tree of 892ea575). My #466 fixes, all in main: 3a2a4a08 (effect test theme declares
  its dark variant, T4 variants), 758b85ab (invalidation test follows hot paths' root-only resize; abi_event_catalog.py
  PUBLISHED_AS SurfaceRoot->Window), 83db00a7 (CorrespondenceView dangling string_view, the cross-platform display-trace
  digest cause), 892ea575 (PropertyList dangling view + paint check; -Werror=dangling-assignment-gsl/-gsl on Clang
  first-party targets; seam lab reads the engine's band). Open for T4: a 1 px panel band's edge stroke overpaints the hairline.
- **cp17 fold:** branch cp17/trial in D:/gui_forms_w3/cp16_trial, head **2c1cbcd7** on origin/main edfad055 (14 commits),
  nothing uncommitted. Contents in order: S2.1 (w5 e537614f), assert sweep (w7 8147e03b), split-remove-stx (84a26dd6,
  df3cfd59; the 4 stx_conv conflicts resolved as deletions, those files equal stx main), PENDING emptied + stx.lock ->
  stx main 58b88201 (contains 24cc718f), C-10 (.github/workflows/effect-pass.yml, path-filtered PRs + dispatch; doc note),
  T4 163759ce, fdc9d240 (c367a5ca dropped), 25e8d85a, W-5 e88b147a, T5 C8 0968c952, M0.2 7293d367, T4 8a005c9b,
  the unregistered-test guard (tools/check_test_registration.py + its unit test, in native.yml house-style; 330 programs,
  0 findings; 5 by-hand programs listed in SHARED). All picks applied without textual conflicts. NOT in cp17: T5 backdrop, S2.2+.
- **Verify state** (int tree D:/gui_forms_int detached at 91158ffc = same tree as 2c1cbcd7; script
  dev/tools/cp17_full_verify.sh, output dev/cp17_full_verify.out): build rc 0; plain suite 500/501 -
  **gui_forms_engine_recipe_field_tests FAILS** (not triaged; ctest.log has no output: rerun with --output-on-failure;
  suspects T4 163759ce/fdc9d240/25e8d85a/8a005c9b vs C8 or M0.2); checks 38/39 - **state_gate FAILS**: 3 F1 (switch without
  default) in tests/compositor_lab_tests.cpp:152,185,244 (T5 C8 0968c952). Sanitized run and the input_desktop set were
  stopped for the restart (not run). Paint census / digests / inventory passed in the plain suite.
- Next: fix those two on cp17/trial (C8's switches: compliant default with reason, or ask w4), triage recipe_field; rerun the
  full verify (sh dev/tools/cp17_full_verify.sh under build_slot; input_desktop set must run: W-5 touched Windows
  accessibility); then checkpoint.py after-merge (template_harness/checkpoint) and report the head; the coordinator pushes cp17.
- cp18 candidates: T5 backdrop (w4), S2.2 + S2.3 (w5), anything W/T4 bring after cp17; X-1/X-2 docs touch-up with the
  programs re-extract (standby, after cp17).

## Previous RESUME POINTER (2026-11-04, machine shutdown; cp16 force-folded and pushed by the coordinator)

- #465 = checkpoint/2026-11-04-1 at 882ef1c2 (my profiles fix). CI at shutdown: 24 pass, 7 skipping, 1 pending (native macos-15 build).
- #466 = checkpoint/2026-11-04-2, force-folded by the coordinator (order and hand resolutions in the #466 body and in STATE.md
  "FORCE FOLD"). Its working branch is cp16/fx in D:/gui_forms_w3/cp16_trial; the coordinator pushed head **8b7b470a**:
  3c573fb0 (paint census `update`; text_box stays at 1 by coordinator ruling, T4's next item in t4_themes.md; effect_registry
  I05 bound), then 8b7b470a (95 stale pending Window contracts -> docs/reference/contracts/surface_root.json; inventory.json and
  docs/api regenerated; display-trace digests already matched). CI on 8b7b470a at shutdown: 2 pass, 6 pending, 1 skipping.
- My local checks on 8b7b470a: the int tree is detached at cp16-fx = 8b7b470a (built, build rc 0). build.sh 'effect|display_trace|paint_census'
  ran 18 tests: 17 pass, **gui_forms_effect_program_tests FAILS** (log D:/gui_forms_int/ctest.log). Check fx f61151aa's own head
  and the I05 edit in 3c573fb0 first; it is not yet triaged. checks.sh passed every check it reported (house style, stx_skia style, lifetime rules, code integrity --gate); the
  run's output is in D:/gui_forms_w3/dev/cp16_checks.out.
  Nothing is uncommitted on cp16/fx.
- Next: triage gui_forms_effect_program_tests on 8b7b470a; watch #465 (macos native) and #466 (the native builds, docs and state-machine gate). Put fixes on cp16/fx, or for a
  #465 fix on checkpoint/2026-11-04-1, then restack; build under build_slot; send the SHAs; the coordinator pushes. When green, stop.
- Then (unchanged): checkpoint.py after-merge; cp17 (S2.1 ff059e69 + assert sweep 7aa159bd with the two live_surface pending
  entries removed; M0.1 b8d17f0d; T4 fonts; split-remove-stx df3cfd59 with stx.lock re-pinned); programs re-extract.
- Local pitfall: after env.sh, MSYS2's sh/env drop PROCESSOR_ARCHITECTURE (and USERPROFILE). Run build_native.sh and
  cmake via Git's sh (dev/tools/cp15_profiles.sh does this).

## 2026-11-07: cp20 fold — RESUME POINTER
- cp20/trial in w3 on main 00c78e6c: P-5, w5 S6/S7.1/S7.2 stack, S5, glass, w8, T4 (a),
  FD-4 checked-collections + nine pages, P-4 Panel, B3 MonthCalendar, B3 TokenBox (e5350a95),
  then 36b07a17 "cp20 fold: regenerate" (inventory, ABI event catalog, dropped 4 stale Panel contracts).
- Full verify (.cp19_verify.sh 00c78e6c) started on 36b07a17.
- Coordinator: owner approved T4 (b) with muted text #5b6b79; w1 is amending. Fold T4 (b) when the new
  head arrives (above the regenerate commit, or redo the regenerate after it), then re-verify.
- Still pending: w5 pairing commit on top of the S7.2 stack; any further FD-4 pages.
- 2026-11-07 later: full verify on 36b07a17: all green except gui_forms_abi_conformance (GridPanel now 3
  child properties after w8 f71731a4; w8 to fix the test) and test_harness_shims (known, recorded).
  Trial reset to e5350a95, then w5 1907ef38 folded as 8c163161. Regenerate 36b07a17 kept as an object:
  re-pick or redo it last. Waiting for: w8 conformance fix, T4 (b) amended head, optional w5 unit test.
- w8 bc40b41f conformance fix folded as 98397089. Waiting for: T4 (b) amended head, optional w5 unit test.
- Trial head 955341b8 (cp20/trial-c): + w5 32a41a62, w7 2b91d783/19f736e5/242de2f5. Focused round green except stale ABI catalog (regenerate fixes) and test_harness_shims. T4 (b) 768ccf93 conflicts in 15 files (still on 2863db55; pages now forms): aborted, w1 to rebase onto cp20/trial. Then: fold, regenerate, full verify, report head.
- Owner: T4 (b) moves to cp21. cp20 VERIFIED at ee92be8b (regenerate on 955341b8): plain 512/512, GUI 13/13, sanitized 486/486, programs ok, checks 39/40 (test_harness_shims known). Reported to coordinator for push. Next: rebase when told; T4 (b) for cp21 needs w1's rebase onto the new main.

## 2026-11-07: ROLE CHANGE — integrator (owner decision) — RESUME POINTER
- w3 is now the integrator: read desks/coordinator/INTEGRATION_QUEUE.md and BRIEF.md "Coders and the integrator".
  Batch unrelated queued entries onto one branch on the current base (after cp20 #471 merges: origin/main),
  build+test once on the M4 (verify script .cp19_verify.sh BASE; focused .cp20_round.sh via a wrapper script,
  since m4run's ssh loses quoting of '|' patterns), fix minor breakage as separate commits attributed to the
  entry, return substantive problems to the coordinator with failing output, update each entry's status, report
  batch heads. Regenerate (.cp19_regen.sh + scp of inventory/contracts/schema; abi_event_catalog.py locally) last.
- m4ws runs two commands at once; GUI lock only for plain ctest/ci_build or M4WS_WAIT_IDLE. Scripts now use --jobs 5.
- Queue empty as of this entry. cp21 carries T4 (b) once w1 rebases it onto the new main.
- batch1 = integration/batch1 @ 47265a32 on cp20 71337875: w8 fd5b+fd4-pages, w6 codemod/ListView/TaskDialog/GroupBox(+TextChanged); fixes fafb9529, d65d0780, 47265a32. Focused plain+san green; reported. Next: batch2 = w6 a61537c5 TextBox, f8ddddb2 MaskedTextBox, 14363f52 (stacked).
- batch2 @ 972abe4b accepted. batch3 = integration/batch3 @ 2a3a3f21 (w6 b5 to 32283610 incl. W collections as d832def1), fixes 8a9efe3c 2c80f52e c98705ee e19ad973 e2df46c2; green. integration/line-03ac @ 66db7be8 = whole line rebased onto cp20 03ac8982 (clean, unbuilt). Next batch: w6 beb8c366 (B7), 8c2dd64e (B8/B9), 21ec9761+7e70c10e (W-4), on line-03ac.
- 2026-11-08 batches (integrator line, branches in w3): batch1 47265a32, batch2 972abe4b (accepted), batch3 2a3a3f21,
  line-03ac 66db7be8 (batch1-3 on cp20 03ac8982 = main d722217c), batch4 2c834e25 (7e70c10e returned, fixed by w6 1d4f60bc),
  batch5 b6cab937 (w7 roles, Part A b8027bbd, FD-6 7d7fe052; codemod binding_direction key), batch6 in progress on top
  (w5 abi-application 54fec269+da3c769e, w6 Toast..M-S2a; Label forwarders rewritten and deleted 4e7a48ac).
- Rules: never run clang/any compile on the laptop (Neo hosts Claude); M4 only. Do NOT take claude/splash-macos (owner hold).
  M-S entries: delete forwarders, run .rewrite.sh <configs> on the M4 (build log -> ../dev/rewrite.log), pull ../dev/rewrite.tar.
- Scripts: .batch_k0.sh (keep-going build, unique errors), .batch_round.sh BASE REGEX, .batchN.sh wrappers, .rewrite.sh.
- Owner rule 'code first, verify once': per batch one plain build + one test run (sanitizer only at checkpoint verify; .batch_round.sh runs it only with BATCH_SANITIZE=1); fix clear substantive breakage myself; return only design problems; report <=5 lines per batch.

## 2026-11-08 integrator: cp21 candidate
- integration/batch6 = cp21/candidate at 0c6dd2f7 (base d722217c). Checkpoint verify (run-1108-074030): plain 518/519 and sanitized 492/493, the one failure display_controls (fixed in 0c6dd2f7, then that test passed plain and sanitized); programs build and tests pass; checks 39/40.
- Failing check test_harness_shims (audit_macos_minimum given --help) fails in every run today, base included; it is outside batch6.
- Next: coordinator push approval; T4 (b) after w1 rebases; further queue batches on the cp21 line.

## 2026-11-08 integrator: cp22 candidate — RESUME POINTER
cp21 (#472) is to be closed in favour of cp22; cp21's CI failures are folded into cp22. Every batch is built on the M4 (m4run w3).
- **batch7**, integration/batch7 @ e5703399 on 0c6dd2f7: w6 M-S sweep 82b9fbd1/b7c73951/9b947eec/f16f8fd9, w8 ms8 + fd4-gallery.
- **batch8** @ f4ec4f19: interaction-state, rules, window-is-form, rule migration (adapted), splash, r8, p5, subsurface-followups. Reported as the A/B/K head.
- **batch9** @ 4d020751: w6 d28c726b..0125d0c7 (0125d0c7 replaces w7's collection_gallery; root CMake builds all six reference forms and the theme), T4 0c6ff9a2..d03bc2ed. Fix: ComboBox on_selected_index_changed raises its event (P-8 left it empty). Six T4 test failures were returned to w1: theme_contrast, gui_forms_theme and familiar_themes (W201 on the floors' literal text colours, a design question), theme_conformance (missing .expected), overlay_shadow, seam_proximity_lab.
- **batch10**, integration/batch10 (current): W's claude/gallery-stack 5b6f0235 (replaces my copy of gallery-init/panes), w5 17dc8945 + dd6aa7a0, w5 259d06ab + fd6b3088 (abi_tsan), w8 0373969c.
  - cp21 CI fixes: Windows UIA tests use cells; present_profile_workload has using declarations; MaterialColourFilter holds its matrix through a raw owning address, so SurfaceMaterial is standard layout under libstdc++ (verified by static_assert in the Linux VM).
  - GalleryTree's manual.json entry is dropped; digests now 35 (input-and-rules, theme-settings, spinners-and-rings added).
- Still to come: W's scroll/double-click fixes and T4's batch9 fixes and inspection page when they register. Then the Linux VM ci_build (`ssh m4mini '~/bin/gfvm sync w3'`, then the command in gfvm's header), docs (sphinx -W), engine gate, re-pinned digests and schema.md, creation-gate baselines (Q-K1 (a)), and the checkpoint verify (plain plus one sanitizer run).
- The ready report must list template_harness d423a57 (pinned by harness.lock, on no remote) as a push the owner approves first.
- Lesson: never pass a `bash -c '...'` string through m4run (it ran a bare python3 that hung on stdin); put the commands in a dot script.
- **Docs toolchain** (owner-approved 2026-11-08): the M4 venv ~/Developer/Projects/gui_forms_workers/docs-venv, built from the pinned docs/requirements.txt (sphinx 9.1.0). To run the docs job: `bash .docs_job.sh` via m4run, which calls `docs-venv/bin/sphinx-build -W --keep-going -b html docs ../dev/docs_html`. The same note is in env.sh's comments. Docs is green at c4496ef7.
- **batch11** (integration/batch11 on W's 8d4a3d75) is the cp22 candidate. The M4 plain suite is green apart from test_harness_shims, which fails at every base. Linux VM ci_build: 480/483.
  - splash_window: a warm show allocates 4 on Linux; returned to w5.
  - linux_accessibility and linux_accessibility_gallery: the VM lacks gsettings-desktop-schemas, so the at-spi bus launcher aborts. This is environmental.
- **cp22 head handed for push (owner: push and fix forward): integration/batch12 @ 79f3398a.**
  - batch12 = batch11 + w8's stx move 7a1d0130, w5 00475c31 and d20dcfcb/e0cfecd2, w6 dark-grounds f9e30b65..1fa1996e, W's e86819ad commits (d0fb796d, 80b1d61a) and T4 2d8b20e7/52c7637f/19030b44.
  - Fixes: Panel accepts the editor visual role (dark grounds broke every form compile); digests are now 36.
  - It compiles on the M4 and the generated files are regenerated. No suite was run at the head (owner directive).
  - Pins: stx.lock 1f5c6f54ef474c3abfc2db2aa4331aaf1ba807f6 (w8's local stx claude/stx-move-sync; must be pushed first); harness.lock d423a57c1a49c691d0328205cec29b3c45f80db4 (template_harness claude/families_b-c10; must be pushed first).
  - Local build note: an existing tree switching its stx source needs <tree>/stx-build and stx-prefix removed, or the stx configure fails with "does not match the source". My trees point at ~/Developer/Projects/gui_forms_workers/stx-move-src through FETCHCONTENT_SOURCE_DIR_GUI_FORMS_STX.
  - Open: rerun macos_damage_rectangles alone with M4WS_WAIT_IDLE=60; the Linux VM pass on batch12.

## 2026-11-08: cp22 landing on Windows (w3, D:/gui_forms_w3) — RESUME POINTER
- cp22 = PR #473 (checkpoint/2026-11-08-cp22). house-style failed: the harness pin d423a57c (template_harness
  claude/families_b-c10, NOT on main) added llvm-22-dev/libclang-22-dev to gui_forms.toml without the reference
  tests. Fix: template_harness claude/c10-toolkit-references 1833da37 (test_project_names, test_file_manager_names,
  file_manager.toml native profile; harness self-tests 442 OK on Windows) + gui_forms claude/cp22-harness-bump
  1416c6d3 (harness.lock). Pushed by coordinator; house-style, code-integrity, state-machine-gate and effect-pass now pass.
- docs failed (Sphinx warnings): popup_lift.hpp "APP_WINDOW_RULES_" wrap, window.hpp popup_window_model(*slot).
  Fix claude/cp22-docs-fix c6ca6d56 (on 1416c6d3); coordinator queues it after the native jobs.
- Local Windows verify: dev/tools/cp22_verify.sh in /d/gui_forms_int (int clone's 2026-11-05 leftovers saved to
  dev/int_leftover_2026-11-05.patch and dev/int_leftover/). Output dev/cp22_verify.out. Pitfall: call build.sh with
  Git's sh, never after sourcing env.sh (MSYS2 sh drops USERPROFILE -> ccache refuses).
- NEXT (owner decisions relayed 2026-11-08): after cp22 merges, cp23 = default_theme() -> gui-forms (fold verify
  fallout, re-record Windows digests/expectations with difference counts, list arm64/macOS goldens for a post-0.5
  Mac handoff or leave tolerant). T4 phase-6 step 1 (claude/t4-phase6 21641022: tests pin professional, Gallery on
  gui_forms_theme) is NOT in cp22 and not on GitHub (Mac w1 only). Then batch cp23 coder entries from the queue.
- 2026-11-08 later: native failures fixed by claude/cp22-test-fixes d896d01a (pushed by coordinator): 66f8e760 Panel accepts notice
  + theme_variant modules get a field; ed271a09 windows_canvas_raster set_scale; d896d01a Linux AT-SPI client roles + toast opener.
  Local Windows on d896d01a: the 10 affected tests plain+sanitized green, checks 38/38 (incl. Sphinx), stx_skia sanitized 139/139.
  cp22 head (pre-fix) full verify: plain 519/524, sanitized 514/519 (same 5, no ASan/UBSan reports), input_desktop 5/5.
  Int clone leftovers moved to D:/gui_forms_w3/dev/int_leftover*; stale stx-sanitize/stx_skia tree renamed .stale-intree.
  CI on d896d01a: house-style, code-integrity, docs pass; natives pending at handback. Next: cp23 default-theme switch.
- 2026-11-08/09: 0.5 fold on #473 = fold/cp22-plus in worktree D:/gui_forms_w3/fold23 (tree D:/gui_forms_w3/.build/fold23,
  scripts dev/tools/fold23_build.sh [tests], dev/tools/fold23_checks.sh). My commits: 2d0f5200 (dock_layout_tests parens),
  ed9b51b5 W111 fixture, ba4e3926 host_types AppKit token, 7848e020 animation test waits for reveal glide, 10113030 re-records
  (schema/anatomies hot+pressed; 36 digests, 643/36499 trace lines), 08cce673 scroll tests (paint + solid root; range 903).
  Plain suite clean on Windows; no sanitizer run (owner). test_harness_shims fails only because fold23 is a worktree.
  Scroll try-build cancelled (claude/scroll-on-cp22 2b900552, tree .build/scroll-gallery kept). cp23 default-theme switch next.
- 2026-11-09: cp22 MERGED (c9b11f0ab). Late cp22-plus fixes: a64e0b860 (Linux a11y fixture reset hides toasts),
  5ce8fce40 (installed form compiler INSTALL_RPATH $ORIGIN/../lib, @loader_path on macOS).
- 2026-11-09: cp24 fold = fold/cp24, checked out in worktree D:/gui_forms_w3/fold23 (same tree .build/fold23).
  Part 1 (theme-default-switch 0c485b11) + fixes b7cef9a53 + origin/main merge = d5c8ef2ad, clean (525/525, checks
  except worktree-only shims). Engine: per-window UnkeptPartAssembly table (8) + HC paint slot; overlay material
  optional -> 0 paint allocations per tab light/dark. Re-records: fade hash 0xca644bce5d05fa2e; 36 digests
  (11707/36499 lines). Parts 2-4 merged (window-minimum-size, theme-host-fixes [anatomies.inc regenerated],
  drag-engine 850bf2186) = 5b3560cc9, build/suite/regen/traces running (dev/fold24_p24.out). Part 5 pending:
  abi-keys-numeric w1 1f428f325, abi-modal-windows w7 68df67d1e (keys first, priority). Scripts: dev/tools/
  fold24_regen.sh, fold24_traces.sh <name> <baseline>, apply_digests.py. Mac list so far: engine_motion fade hash,
  Gallery digests (macOS/arm64 goldens re-record on the Mac).
- 2026-11-09: cp25 = fold/cp25 @ ffbdc2a77, pushed as PR #475 (checkpoint/2026-11-09-cp25). Contents: keys,
  styled runs, focus stack, origin/main, host-file-gaps (moved early), mac-handoff-probes b6433c51. Fixes:
  44a852f02 (tests match keys; Windows host native_phase_ ready before the init drain, which the C ABI modal
  part hung on; controls.focus keyboard only when a host presents the window), ffbdc2a77 (digests, regen, census,
  UTC termination log, state-gate defaults, integrity guards). Mac list: Gallery digests states-and-diagnostics
  and values-and-spinners (158/40565 lines). Paint census: text_box_decorations.cpp is a named content-paint
  exemption (owner ruling 2026-11-09).
  cp26 order, in progress on fold/cp25 (same worktree): data-grid d79df4b29 (+attached_window fix in
  abi_create_with.cpp), swapin-gaps-base 9c1f6d4d0 + swapin-gaps-compat-abi1 a8bde9221
  (D:/malkuth/probes/swapin_lib/gui_forms), ribbon-gaps 94a8934e9, look-integration (w1's rebased head),
  theme-host-fixes 6f4e9d3a + -2 29b6aa6d (w4), value-bar 1bfa4a94, drawing-primitives 32cd5cbd, plots d05561e4.
  Foreground tests: run_on_desktop.py; drawing_windows_probe and windows_compositor need foreground_lock.py.
