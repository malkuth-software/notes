# Track: semantic tests (worker w7)

Owner-approved 2026-11-03. Move the semantics GUI.Forms owns out of the
input_desktop tests (UIA provider, UIA text, UIA Gallery, MSAA Gallery,
compositor probe: real desktop, foreground lock) into headless tests against
Window::semantic_snapshot, keeping only the Windows adapter's mapping there.
No coverage lost (table below); runtime measured before and after (plain,
one batched run each).

## RESUME POINTER

DONE 2026-11-07 (cp20 FD-4, base origin/checkpoint/2026-11-05-4 835c3599):
- FD-4 branch claude/fd4-semantic-pages head 09f05e66 (aebae84b checked-collections; 09f05e66 the other nine
  pages + theme classes .small/.body-strong/.fine). Plain --gallery gallery_|doc_examples|semantic_gallery|
  form_language 10/10; sanitized gallery_|form_language 7/7 (launched-app labels skipped); libdocs --check,
  style, lifetime, gate --changed-from origin/main clean. Traces: only values-and-spinners re-recorded (property
  grid shows the declared Bounds). Next: wait for the coordinator (rebase S5 onto w5's S6/S7 when told).
- S5 claude/subsurface-s5 186aa3c1 unchanged.
- DONE 2026-11-07 (cp20 gaps, on 09f05e66, same branch): 2b91d783 .form string escapes (\n \r \t \" \' \\ \u{H},
  E815 form_escape_invalid; read_form_string/quote_form_string; timing two-line texts in the form); 19f736e5
  states page DiagnosticsCard (a metrics-view, already catalogued: my miss) in the form; 242de2f5 FlagsValueEditor
  StableId ctor + enumeration_name/none_name/flag_names/checked_flags properties, value-editors flags row in the
  form, C ABI create now makes it (conformance refused list updated, schema sample from stable id). Round
  (dev/tools/flags_round.sh): plain 45/45, sanitized 43/43, schema/libdocs/compat/style/lifetime/gate clean.
  Lesson: ci_build --tests narrows the build too; build with the full regex before a plain ctest.
- NOW (primary): desks/coordinator/APP_WINDOW_RULES_DESIGN.md finalised to the owner's rulings (2026-11-07)
  as the build plan (uncommitted; the coordinator commits). Q-A1 hybrid (owned Application, current while
  alive, config down the ownership chain); creation-cost contract + benchmark gate (Part K); window kinds
  Window/Popup/Splash; Splash before 0.5 (Part S, from SPLASH_NOTES.md: present-only window, one subtree two
  presentations, warm standby, per-host table). 28.75 U, cp21-cp25, SEAL items marked in section E; open
  Q-S1, Q-S2, Q-K1. Next: cp21 starts with the K gate baseline, then A, then FD-6 binding items 1-3, when
  the coordinator says build.
- DONE 2026-11-08 (cp20 blocker #471): claude/cp20-uia-stitch e95ad0af on origin/checkpoint/2026-11-07-1
  (worktree w7/cp20fix): UIA gallery test waits for started SubSurfaceHosts' semantics before building the
  page expectation (shown unfrozen, poll from the client thread, then frozen expectation without reselecting).
  Windows-only, not compiled here; style/lifetime clean.
- DONE 2026-11-08 cp21 FD-6 bindings: claude/fd6-bindings 7d7fe052 on origin/main d722217c (one commit):
  bind(...) in the slot, direction per property (binding_direction) + one_way, update trigger, convert
  reserved (E999); bind at creation (bind_form_value/bind_form_field/bind(Command&), Control-owned
  PropertyLink); loader FormSources + E819; E816 on- refused (808 retired), E817/E818; Gallery handlers in
  connect_handlers; golden regenerated. Verify (dev/tools/fd6_verify.sh): plain 45/45, sanitized 43/43,
  libdocs/schema/compat/style/lifetime/gate clean. Round script with regeneration: dev/tools/fd6_round.sh
  (docs come back with rsync from m4mini .../w7/gui_forms/docs/ - the laptop tree is authoritative).
- DONE 2026-11-08 W-4 roles: claude/w4-switch-search-roles 85c7e94e on d722217c: SemanticRole toggle_switch
  (keyword) and search_box, semantic_role_fallback (check_box/text_box), UIA Button+Toggle / Edit "search
  box", MSAA fallbacks, AX AXCheckBox+AXSwitch / AXTextField+AXSearchField, ATK toggle button / entry;
  oracles, semantic_tests, schema test; manifest/schema page/API regenerated. dev/tools/w4_round.sh: plain
  32/32, sanitized 29/29, checks and gate clean. cp20-uia-stitch deleted (merged).
- DONE 2026-11-08 Part B step 1, root slot (B.2a): claude/root-slot f4e86fc1 on claude/app-create b8027bbd
  (fetched from w2's clone as refs/remotes/w2/app-create). SurfaceRoot chrome_insets/set_chrome_insets/
  root_slot; flush measures/arranges the root in the slot; resize marks via slot extents; damage, overlays,
  UIA fragment root stay client. tests/window_root_slot_tests.cpp. dev/tools/slot_round.sh: plain 46/46,
  sanitized 40/40; libdocs/compat/style/lifetime clean; gate's one F1 is app-create's abi_windows.cpp:131.
- DONE 2026-11-08 sub-surface follow-ups queued UNVERIFIED: claude/subsurface-followups e692512c on origin/main
- DONE 2026-11-08 four follow-ups queued UNVERIFIED on claude/subsurface-followups a6de2562 (base e692512c): 177ca2a3 stopped/pairing_missed by posted work (coordinator ruling: never inside the present path); a6473ad6 grid region publishes no semantics (set_publishes_semantics); 7024a1d9 DataGridViewCellsTextProvider on the region's thread (decision 3(a) second half; touches W's cell_layout/resolve_cell, noted for M-R in the queue); a6de2562 benchmark M2 input path. Checkers, control_schema check, abi catalog check pass locally; no M4 runs (owner: nothing waits on builds). Next: whatever the coordinator assigns; fix-ups only if the integrator's build reports breakage.
  d722217c (S3-S7 already on main in cp20): waterfall lab on LiveSurfacePacer; macOS AX Gallery fixture waits for
  sub-surfaces to publish before freezing (S5 intermittent). OPEN, needs a coordinator decision: subsurface_host
  interactions stopped/pairing_missed (pairing is decided in Window's present path, window_presentation.cpp ~915;
  raising a control event there is reentrant: needs a deferred notification), virtual-mode provider on the
  sub-surface thread, grid's "Sub-surface" pane in the semantic tree, M2 input path.
- DONE 2026-11-08 Part K lazy registries + COW overrides queued UNVERIFIED: claude/window-is-form c2b46eb4 (on
  da3ecafb). WindowOverrides record (surface_root.hpp), lazy key tables (window_lifecycle.cpp), Linux AT-SPI lazy,
  control(id) wraps find(). Tests in application_instance_tests (test_window_overrides). NEXT: coordinator's call.
- DONE 2026-11-08 Part B (outside the Gallery) queued UNVERIFIED: claude/window-is-form da3ecafb = load_window
  (src/core/form_language/form_loader/form_window_loader.*), SurfaceRoot::control(id) -> ControlLookup
  (src/core/surface_root/control_lookup/), collection_gallery rewritten as .form + GalleryModel, first-window docs
  section, form_loader_tests. Owner rule "code first, verify once": no M4 runs; integrator builds. Gallery and the 29
  pages are w8's (my FD-4 share dropped). NEXT: whatever the coordinator assigns; fix returns from the integrator.
- DONE 2026-11-08 Part B step 2: claude/window-is-form 9359fcb3 (f4e86fc1 root-slot, a3756eaa FD-6 cherry-pick,
  956ec890 creation benchmark handlers moved to program code: app-create's form named on-clicked, E816 after FD-6).
  wif2: plain 12/12, sanitized 12/12 (form_|window_root_slot|doc_examples), libdocs/style/lifetime clean, gate's one
  F1 is abi_windows.cpp:131 (fixed in W's fdde9f5b). NEXT: runtime load_window/control(id), then Gallery migration.
- (detail) Part B step 2 on claude/window-is-form (root-slot + FD-6 cherry-pick):
  (gui_forms/.tmp_wif.sh = dev/tools/wif_round.sh; m4run scripts must live inside gui_forms/). FormGeneratorOptions
  derived/model_type/model_header; compiler --derived/--model/--model-header; gui_forms_add_form DERIVED/MODEL/
  MODEL_HEADER. Generator refactor: write_objects/write_values with ObjectAccess (through_made | as_members),
  object_reference "(*x)" / object_argument "*x" / "*this" for a derived control root; document shape byte-equal
  (golden). Derived binds without MODEL -> E812. Test: tests/form_language/form_derived_class_tests.cpp + fixtures
  generated/sample_model.hpp, derived_panel.form, plain_panel.form (CMake under GUI_FORMS_BUILD_GENERATED_FORM_TESTS).
- (plan as given) Part B step 2 (generated class): branch from root-slot + cherry-pick FD-6 7d7fe052. Plan: no Window
  constructor change - a generated `class X final : public gui_forms::Window` builds its objects in a static
  make_objects(), delegates to a private ctor that passes the root to Window(Application&, root, size) or
  Window(root, size); bindings in the body. user-control roots: UserControl subclass with
  initialize_tree_after_construction (make_control needs a live owner for add_child) and a static make(Model&).
  Opt-in via gui_forms_add_form MODEL/DERIVED until the Gallery migrates.
- Sanitized calls in my scripts carry --skip-labels 'area_host|area_a11y|gallery_accessibility|foreground'.

PAUSED 2026-11-05 (owner pause, cp20 work):
- S5 DONE: claude/subsurface-s5 186aa3c1 on w5 claude/subsurface-mac 5639ab72. Plain --gallery 13/14 + sanitized 4/4 +
  docs/style/lifetime/gate clean; macOS a11y in the plain tree: accessibility + client pass; the Gallery client failed
  once (ranges-and-progress never settled in 10 s, motion clock armed) then passed 2/2 on S5 and 1/1 on the base without
  it: unexplained intermittent, recorded in the commit and reported. Pre-restack copy claude/subsurface-s5-cp18 0ac16e05.
- FD-4 (split confirmed: my 10 pages tree-and-object-views, text-and-input, states-and-diagnostics, timing-and-tooltips,
  dialogs-and-host-services, checked-collections, choices-and-sections, dates-and-calendar, values-and-spinners,
  value-editors; w8 has the layout/panel pages + 3 slot pages). Branch claude/fd4-semantic-pages from origin/main
  9e669353, WIP 794dd765: checked-collections.form + page over CheckedCollectionsDocument + CMake (add_form and the
  add_theme FORMS list) + theme class .note-medium (10px/600). UNBUILT (round stopped by the pause).
  Pattern: copy ranges-and-progress (mount_document moves the root's children into the body, groups get page_style in
  code); label(size,weight,colour) -> classes: shell_blue_dark=heading, muted=muted, green=positive, ink=ink,
  accent=accent-text; 22/700 page-title, 12/400 body, 12/600 body-emphasis, 11/600 emphasis, 11/700 strong,
  10/400 note, 10/600 note-medium (new), 10/700 note-strong, 9/700 tag; group() -> "paper" "group-caption".
  Code-built labels (card/section content) keep code styling.
- Next: run `bin/m4run w7 --label fd4 -- sh .tmp_fd4.sh` (gui_forms/.tmp_fd4.sh, master w7/dev/tools/fd4_round.sh:
  plain --gallery 'gallery_|doc_examples|semantic_gallery|form_language'); fix, check display trace (re-record with notes
  if it differs), amend the WIP into a real commit; then choices-and-sections (read already: ChoiceGroup items, sections,
  cards stay in code).

S5 VERIFIED + AMENDED 2026-11-05 (cp20): claude/subsurface-s5 26d842f0 on w5 5639ab72. Plain --gallery 13/14, sanitized
4/4, docs regenerated, style/lifetime/gate clean. macOS a11y client tests (plain + gallery) fail environmentally: launched
app cannot load dylibs from /Volumes/Storage (.build symlink after the drive move; "file system sandbox blocked open()",
crash report on m4mini ~/Library/Logs/DiagnosticReports *_fixture_app-2026-11-05-185727.ips) - reported to the coordinator.
Next: FD-4 Gallery pages to .form, my share proposed (awaiting the coordinator's split with w8): tree-and-object-views,
text-and-input, states-and-diagnostics, timing-and-tooltips, dialogs-and-host-services, checked-collections,
choices-and-sections, dates-and-calendar, values-and-spinners, value-editors.

STOPPED 2026-11-05 (app restart): S5 head claude/subsurface-s5 e460a5ad on w5 5639ab72; tree clean, nothing uncommitted.
The plain round (gui_forms/.tmp_s5_all.sh: plain --gallery 'sub_surface|semantic|gallery|doc_examples', macOS a11y, docs
regen, gates) was started under M4WS_WAIT_IDLE=180 and abandoned before any result; the remote side may have been cut
mid-run. No sanitizer (coordinator: cp19 fold runs it; M4 space). Next: rerun
`M4WS_WAIT_IDLE=180 bin/m4run w7 --label s5-all -- sh .tmp_s5_all.sh` and in the same step rsync docs/ and
tests/doc_examples_tests.cpp back from m4mini; commit regenerated docs into the S5 commit (amend) with the counts; report.

RESTACKED 2026-11-05: claude/subsurface-s5 = e460a5ad on w5's claude/subsurface-mac 5639ab72 (S-stack on main 4ab9a107).
Conflicts (all keep-both: request channel vs semantics listener, stats, docs) resolved with w7/dev/tools/keep_both.py;
generated docs (docs/api/controls.drawing.rst, generated/gui_forms.SubSurfaceHost.rst, reference/inventory.json) taken from
the base and NOT yet regenerated. Pre-restack copy: claude/subsurface-s5-cp18 0ac16e05. NOT BUILT: the M4 disk is full;
when the coordinator says space is free: plain+sanitized sub_surface|semantic|gallery|doc_examples (--gallery), macOS a11y
(M4WS_WAIT_IDLE=180), .tmp_s5_regen.sh + rsync docs back in the same step, gates, then amend the commit message's counts.

S5 DONE 2026-11-05 (reported): claude/subsurface-s5 head 0ac16e05, one commit on cp18 origin/checkpoint/2026-11-05-2
dfa4f9b5 (local only). Verified on the M4: plain --gallery 14/14 (sub_surface|semantic|gallery|doc_examples), sanitized
4/4, macOS accessibility 3/3 (M4WS_WAIT_IDLE=180), style, lifetime, libdocs/api --check clean; gate F1 x2 in
tests/macos_message_dialog_key_tests.mm come with cp18 (not S5). M8: action round trip 120.8 us, stitched snapshot read 4.8 us.
Mac workflow notes: m4ws syncs only gui_forms/ and template_harness/ (put check scripts at gui_forms/.tmp_*.sh, excluded via
.git/info/exclude; masters in w7/dev/tools/); generated files must be rsynced back from
m4mini:Developer/Projects/gui_forms_workers/w7/gui_forms/ in the SAME step (the next m4ws sync overwrites them).
Next: rebase onto main when cp18 lands, then onto w5's rebased S3.3-S4.6 when the coordinator says; open follow-ups:
text layout for sub-surface nodes; UIA events from the sub-surface thread (design said so; snapshot model raises them on
the window thread) - coordinator/owner call.

2026-11-05 (Mac resume): W-5 is ALREADY ON MAIN (cp17, 4d502fee; backup e88b147a rebased away as applied;
CLOSURE_0.5.md still lists it as unowned: stale). Started SS-5 (S5 accessibility) on branch
claude/subsurface-s5 in ~/Developer/gui_forms_workers/w7/gui_forms, based on origin/checkpoint/2026-11-05-2
(cp18 dfa4f9b5, on auto-merge; it holds S2.1-S3.2 + host anatomy/semantic node). w5's S3.3/S4/S4.6
(claude/subsurface-s4 356a9da4, on cp16) are NOT under it (cherry-picking them onto cp18 conflicts in 8 files;
left to w5): when w5 rebases them onto main, rebase S5 onto their head (expect small conflicts in
sub_surface.cpp/.hpp, sub_surface_channel.hpp, sub_surface_host.*).
Design (decision 5a): the sub-surface thread publishes an immutable SemanticSnapshot whenever its
semantic_generation moves (end of loop turn), posts one coalesced notice to the parent (SubSurfaceSemanticsListener,
set_semantics_listener, take_semantics); SubSurfaceHost publishes the snapshot as its virtual children translated
into the window (stable id "<host>.<inner>", runtime id virtual_semantic_runtime_id("<host>#<inner runtime id>"),
bounds offset, focused only while keyboard_inside_); actions -> on_semantic_child_action -> posted as
SubSurfaceInputKind::semantic_action with the inner runtime id (stale refused there; statistics
semantic_snapshots_published/actions_performed/refused); focus action also focuses the host. No host adapter changes:
all three adapters read Window::semantic_snapshot and route virtual children by (control runtime id, stable id).
Limitations to report: no text layout (text ranges) for sub-surface nodes; UIA events raised by the adapter on the
window thread from the stitched snapshot (design said from the sub-surface thread).
Tests: tests/sub_surface_tests.cpp thread_publishes_semantics, host_stitches_semantics (prints an m8 line).
Script: w7/dev/tools/s5_check.sh '<regex>' [plain|sanitize|gates|all] (via bin/m4run w7 -- sh ../dev/tools/...).
Next: plain + sanitized sub_surface, gallery/semantic tests (live-surfaces page now has stitched nodes), macOS a11y
client (M4WS_WAIT_IDLE=180), docs (docstrings/API pages), gates; then report.

Previous pointer (2026-11-05, before the Mac resume):
OVERLAY RENAME: DONE 2026-11-05, reported to the coordinator for cp18. Branch claude/overlay-rename head 594cadb2
(one commit on origin/main edfad055, local only). Verified: build clean; 72 related tests plain (incl. display trace,
abi audit, schema, gallery pages); UIA/MSAA gallery under foreground_lock pass; 22 touched tests sanitized pass;
CI generated-file checks (dev/tools/ov_checks.sh), docs, house style, lifetime, gate --changed-from origin/main clean.
Details of what changed (kept for the record):
Post-restart work (uncommitted on top of fcaa0e89 until the tests pass, then one fixup commit):
- Build clean (dev/tools/ov_build.sh, ov_regen.sh under build_slot).
- Reverted over-renames: registry.hpp run_window/begin_invoke/request_close locals and messages for native popup
  windows (GF_WINDOW_RUN_POPUP) back to popup; restored the dated fm0 2026-08-10 manifest and the 0.5 ABI baseline.
- ABI: baseline untouched; the two breaks (GF_TABLE_POPUPS, gf_popups_table removed) recorded in
  manifests/abi/audit_policy.json breaks (report mode, owner C-1). Regenerated gf_abi.json, probe, C#/C++ bindings,
  schema (WRITE=1), schema.md: identical to the textual rename. Regenerated: event catalog, docs/api, inventory.json.
- Display trace: before = D:/gui_forms_int cp17 binary (same table, passes), after = w7; dev/tools/ov_trace_diff.py
  shows wording/geometry-only changes (sidebar entries on every page, renamed page texts); table re-recorded with a note.
- Prose: overlay vs overlay paint plane disambiguated (painting-and-damage.md, window_presentation.cpp comment,
  core_tests OverlayPlaneControl + message, manual.json attach_overlay, registry overlay_form_plane_overrides).
Next: rebase onto origin/main when cp18 is folded if asked; nothing pending.

State: 2026-11-03, DONE, reported to the coordinator. Branch
claude/headless-semantic-tests head 19730f2e (one clean commit on origin/main
feacf6d4, local only). Verified: semantic_fixture and semantic_gallery plain and
sanitized; docs, house style, lifetime and gate (--changed-from origin/main) clean;
the four thinned input_desktop tests are built, and they pass plain under the lock.
Measured (plain, one batched ctest -L input_desktop, logs dev/logs/input_desktop-{before,after}.log):
| test | before s | after s |
|---|---|---|
| compositor probe | 18.24 | 18.56 (rerun alone: the batched run waited 675 s on w2's lock) |
| uia_provider | 4.51 | 3.10 |
| uia_text | 2.63 | 2.41 |
| msaa_gallery | 20.44 | 12.62 |
| uia_gallery | 39.12 | 34.92 |
| total | 84.98 | 71.61 |
The headless pair costs 0.6 + 8.5 s, with no lock, in parallel.
Step 2 (coordinator, 2026-11-03): profile the two Gallery clients, propose cuts, no changes yet.
Profiled with a temporary probe (dev/tools/timing_probe.py, reverted; summary dev/tools/timing_summary.py;
logs dev/logs/probe-{uia,msaa}.err). No sleeps or fixed waits in either client.
- uia_gallery, 32.6 s internal: page walks 19.8 s (32 pages, 3202 elements, about 6 ms per element of
  cross-thread Current* and walker round-trips); 19 windows about 7.7 s (model build 2.4, create to
  host_ready 4.0, close 1.2); 58 actions 3.5 s.
- msaa_gallery, 19.0 s: 19 windows 13.6 s (build 3.9, create 4.7, close 4.9); walks 1.7 s; 56 actions 3.2 s.
Approved 1, 1b (reruns rare) and 2; DONE as d04b56b2 on top of 19730f2e (checkpoint 16; local only).
Both clients: two windows (compare, then act in the same window; an unseen change reruns alone in a
fresh window; 4 reruns each, all containers-and-focus dock nodes lost after an earlier action).
UIA: pages from one cache request (raw subtree, base + pattern-availability + pattern-state properties);
live: tree-and-object-views, TextPattern ranges, parent links, ControllerFor targets, stale element.
Verified: both clients plain; the sanitized tree ran the whole suite (ci_build dropped the -R regex),
518/518 passed, rc=2 unexplained; docs, style, lifetime and gate clean.
input_desktop batched under the lock (log dev/logs/input_desktop-after2.log, machine loaded: the
unchanged tests were slower too): probe 20.28, provider 6.67, text 3.17, msaa 11.65, uia 17.58,
total 59.40 s (71.61 before this step, 84.98 at the start). Unloaded solo runs: msaa 6.8-7.0, uia 15.2.
Loose end (coordinator, 2026-11-04), diagnosed and reported, no change made: the sanitized run's rc=2 is
sh's own syntax error, not ci_build's. verify-sanitize.log shows "sanitize.sh: line 25: syntax error near
unexpected token `fi'". sh reads a script while it runs it; tools/sanitize.sh in the shared projects clone was
rewritten at 22:44:21, 5 min after my run began at 22:39; when ci_build returned at 00:07, sh went on reading
at its old byte offset in the new file. The dropped -R filter: ci_build got no --tests (an unfiltered
cmake --build and ctest); with today's file a dry run (PYTHON=echo) passes the pattern correctly, so the
version read at 22:39 differed. Most likely the same rewrite, not proven. Proposed: wrap sanitize.sh's body
in main() { ...; } plus main "$@"; exit, so sh parses it before running; and have ci_build print its
arguments first. ci_build.py itself has no bug.
Lane 2 (coordinator, 2026-11-04): no bare assert() in tests. Branch claude/no-bare-assert-in-tests from
origin/main feacf6d4, head 7aa159bd (local only). Sweep: only tests/live_surface_tests.cpp (68, 64 with calls)
and tests/windows_live_surface_endpoint_tests.cpp (16, 15 with calls), left to the C ABI lane's S2.1, plus
stx_skia/tests/discardable_pool.cpp (2 precondition guards, no side effects), fixed with require_use()
(aborts in every build). Project pattern: each test file has its own require(); no shared macro; no #undef
NDEBUG anywhere; invalidation_damage_tests' #ifndef NDEBUG checks both build modes on purpose (kept).
Guard: tools/check_test_asserts.py (+ tools/test_check_test_asserts.py), ctests gui_forms_test_assert_check
and _unit; the two live_surface files are listed as PENDING, and a clean listed file fails as stale.
Verified Release (-O3 -DNDEBUG): flipped require_use aborts (0xc0000409), and an injected assert fails the
check; reverted. Plain and sanitized 3/3; style, lifetime, docs, gate clean. Nothing foreground touched.
2026-11-05 restart: lane 2 rebased onto cp16 (origin/checkpoint/2026-11-04-2, 8b7b470a), head 8147e03b (local only;
was 7aa159bd). On cp16: guard 606 files, 0 findings, 2 pending; plain 3/3 (assert_check, _unit, discardable_pool);
style clean; gate nothing to check. The two live_surface PENDING entries go when the C ABI worker's S2.1 folds with it.
Next: idle; rebase onto the standby's cp16 fixes when told; no further track items.
Decision taken: the Gallery clients' actions are re-keyed by (role, pattern call or default kind,
placement) and the per-type effects moved to semantic_gallery_tests (the recommended option).

## Workspace and commands

- Workspace D:/gui_forms_w7: clone gui_forms/ (origin GitHub), template_harness/
  at harness.lock 850b52d7 (fetched from D:/gui_forms_w3/template_harness),
  env.sh + toolchain_env.sh (copies of w4's, paths w7; shared ccache
  D:/gui_forms_main/.ccache; foreground and input_desktop launchers).
- Branch claude/headless-semantic-tests from origin/main feacf6d4 (cp13 merge).
- Build/test locally on Shadow (Windows): `sh /d/gui_forms_w7/dev/tools/verify_win.sh '<regex>'`
  (plain + sanitized + docs + style + lifetime + gate; copy of w4's).
- `dev/tools/build_first.sh`: first configure (ci_build --gallery) + the input_desktop targets.
- `dev/tools/measure_input_desktop.sh <label>`: ctest -L input_desktop once (each test takes
  the foreground lock through its TEST_LAUNCHER); log dev/logs/input_desktop-<label>.log.

## Files

- tests/semantic_fixture_tests.cpp (new, gui_forms_semantic_fixture_tests): the UIA provider and
  text fixtures, headless.
- tests/semantic_gallery_tests.cpp (new, gui_forms_semantic_gallery_tests): Gallery registry
  coverage, identities, positions, tab-to-panel, default actions by both clients' rules.
- The input_desktop tests: tests/windows_accessibility_{uia_provider,uia_text,uia_gallery,msaa_gallery}_tests.cpp,
  tests/windows_compositor_probe.cpp.

## Decisions in force
- Build rules (owner, 2026-11-04): build only what you touch and run those tests plain; no full or
  sanitized suite of your own (the batch fold verify covers it); wrap heavy runs in
  `python -B D:/malkuth/projects/tools/build_slot.py -- <command>` (at most 2 at once). dev/tools/verify_win.sh
  predates this: use quick.sh (under build_slot) plus the style/lifetime/gate lines instead.

- Do not touch window.cpp, window_lifecycle.cpp, the host accessibility provider or controls.
- A check half is (a) when it reads only the snapshot/model, (b) when it reads what MSAA/UIA return.
  A UIA/MSAA call followed by a model read is (b) (routing reaches the model) and stays.

## Before/after assertion table

(a) = semantic half (snapshot/model only); (b) = adapter half (what MSAA/UIA return,
or a UIA/MSAA call whose effect is then read in the model). "stays" = still in the
input_desktop test; "+ headless" = its (a) half is also checked in the named headless test.

UIA provider (51 expects before, 50 after; semantic_fixture_tests mirrors the fixture):
| check (expect message) | before | after |
|---|---|---|
| 1-3 window control type; button/check box found by AutomationId | provider | stays (b) + headless: fixture 1-2 |
| 4-5 button control type and name; FrameworkId | provider | stays (b) + headless: fixture 3 |
| 6 distinct runtime IDs ending in the provider's parts | provider | stays (b) + headless: fixture 4 |
| 7-10 tree reuse count; renamed button described again; restored name | provider | stays (b) + headless: fixture 5-6 |
| 11-15 Invoke offered, no Toggle on plain button, Invoke clicks | provider | stays (b) + headless: fixture 7-9 |
| 16-20 Toggle offered, control type, starts off, Toggle checks | provider | stays (b) + headless: fixture 10-13 |
| 21-24 Value offered, control type, SetValue, read back | provider | stays (b) + headless: fixture 14-16 |
| 25-28 RangeValue range, SetValue 7, past maximum refused | provider | stays (b) + headless: fixture 17-20 |
| 29-37 list items, SelectionItem Select, IsSelected, Selection | provider | stays (b) + headless: fixture 21-26 |
| 38-41 ExpandCollapse offered, collapsed, Expand, Collapse | provider | stays (b) + headless: fixture 27-32 |
| 42-43 doomed button found, offers Invoke | provider | stays (b) + headless: fixture 33 |
| 44 "the button left the window" (pure model) | provider | REMOVED there; headless: fixture 34 (+ 35-37 identity refused) |
| 45-46 removed element answers UIA_E_ELEMENTNOTAVAILABLE (name, Invoke) | provider | stays (b) |
| 47-51 second process window, provider of the second window, stale kept element | provider | stays (b) |

UIA text (38 before, 37 after):
| check | before | after |
|---|---|---|
| 1-5 found, TextPattern/TextPattern2, password box has none, single selection | text | stays (b) + headless: fixture 38-40 |
| 6-11 document range, GetText maxLength, FindText, IsReadOnly, enclosing element, degenerate move | text | stays (b) + headless: fixture 41-43 |
| 12-18 grapheme, word and document units, MoveEndpointByUnit | text | stays (b) + headless: fixture 44-46 |
| 19-23 multiline document, line, paragraph, one rectangle per line | text | stays (b) + headless: fixture 47-52 |
| 24-29 GetSelection, caret, Select, follows Select, one selection | text | stays (b) + headless: fixture 53-55 |
| 30-32 character rectangle, caret rectangle, RangeFromPoint | text | stays (b) + headless: fixture 47 |
| 33-36 event handlers; TextSelectionChanged; TextChanged | text | stays (b) + headless: fixture 56-57 (new generation) |
| 37 "the text box left the window" (pure model) | text | REMOVED there; headless: fixture 58-59 |
| 38 removed range answers UIA_E_ELEMENTNOTAVAILABLE | text | stays (b) + headless: fixture 60 (stale_identity) |

UIA Gallery / MSAA Gallery:
| check | before | after |
|---|---|---|
| per element: control type/role, name, AutomationId, help/description, value, states, patterns and pattern states, Level, AccessKey, IsDialog, LiveSetting, bounds/location, child count/order, parent | both clients | stays (b) |
| runtime IDs unique (UIA) | uia_gallery | stays (b) + headless: semantic_gallery (nonzero, unique (scope, node)) |
| position in set / set size | uia_gallery | stays (b) + headless: semantic_gallery (published together, within the set) |
| tab-to-panel ControllerFor | uia_gallery | stays (b) + headless: semantic_gallery (controlled panel is a tab panel node) |
| registry coverage (every visual registry control publishes its own node; presentational list exact) | both clients | MOVED: semantic_gallery registry coverage (same lists) |
| default action per (control type, role), effect observed in the model | both clients | per-type effect MOVED: semantic_gallery, by each client's rules (MSAA and UIA), plus undo; the clients now act once per (role, call, placement: own node / item / unscoped item), the adapter's routing key |
| stale element after page replaced (UIA_E_ELEMENTNOTAVAILABLE); disconnected object CO_E_OBJNOTCONNECTED (MSAA) | both clients | stays (b) |

Compositor probe: no (a) checks (uses nothing of GUI.Forms); unchanged.


## 2026-11-08: Spinner and ProgressRing

Done: claude/spinner-progress-ring @ deff18ce on 4a4b1179, queued unverified. Next: wait for the coordinator's checkpoint verify; fix any breakage it reports.

## 2026-11-08: toast live region and FD-6 Q2/Q3

Done: claude/toast-live-region-on-namespace @ 6bfcc3bc on batch10 06a78004, queued unverified. Decisions in force: semantic_announcement_due is the single host announcement rule; announces_on_appearance marks live regions spoken when new (toast only). Next: fix whatever the checkpoint verify reports.
