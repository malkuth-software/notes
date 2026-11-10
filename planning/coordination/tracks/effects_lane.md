# Effects lane (w8)

Worker workspace D:/gui_forms_w8 (gui_forms/ clone of malkuth-software/gui_forms; template_harness/
at the harness.lock pin 850b52d7, cloned from D:/gui_forms_int/template_harness). Started 2026-11-03.

## Task (coordinator, 2026-11-03), each a small verified commit on branches from origin/main
1. Themes select effects BY NAME: theme-language `effect` property per part (EFFECT_PROGRAMS_DESIGN §7;
   CLOSURE_0.5 EP-3). Today `effect` is only a ProgressBar control property
   (src/controls/range_control/progress_bar/progress_bar.cpp:35); src/core/theme_language/ has none.
2. Effects take colour ROLES (primary/secondary), resolved from theme + light/dark scheme (EP-4).
3. EffectFrame::pointer {inside, pressed, position}, generic EffectInstance::note_pointer, engine-side
   EffectInputGrant (approved; design §8a(1)); existing motion checksums unchanged.
4. clip_part_shape(): Painter::clip_rounded_rect_corners (recording + replay + Skia raster; default
   falls back to the smallest radius); EffectInstance::paint takes the part's shape (EP-6).
5. Backdrop sampling: EffectDeclaration backdrop_margin + one metered canvas call drawing the blurred
   backplane into a rect (glass's read-only backplane view); hosting control passed to paint;
   non-ambient effects added to the window's backplane damage list (EP-5).
- C ABI worker (w5) had NOT started 3-5 (no branches in w5, checked 2026-11-03); its plan for 4/5 is in
  tracks/gui_forms/c_abi.md "Effects (coordinator, after T4's phase 7 audit)". Ask the coordinator
  before redoing anything forwarded.
- AVOID window.cpp, window_lifecycle.cpp (ask first). Theme compiler beyond the `effect` property: T4's
  area, coordinate through the coordinator.
- Every addition: clang plugin effect pass accepts it and is costed; tests (rounded-corner pixels,
  pass acceptance, budgets).

## Setup done
- env: D:/gui_forms_w8/env.sh (from w5's, paths to w8), toolchain_env.sh (GF_WORKSPACE=w8).
- Text stack copied from w7 (third_party/harfbuzz c7a7457b, freetype f4205da1, libunibreak).
- stx_auxlib: pre-seeded CMakeCache.txt in .build/native-windows-x64/{gui-forms,gui-forms-sanitize}
  with FETCHCONTENT_SOURCE_DIR_GUI_FORMS_STX_AUXLIB=D:/gui_forms_int/.build/native-windows-x64/
  gui-forms/_deps/gui_forms_stx_auxlib-src (08c8132d = stx_auxlib.lock); no download.
- dev/tools/check.sh 'regex' : ci_build plain (+ sanitize unless PLAIN_ONLY=1; GALLERY=1 adds
  --gallery), style, lifetime; logs in dev/logs/.
- Branch claude/effects-theme-property created at origin/main feacf6d4; NO code changes yet.
- The first plain build (check.sh 'effect|progress') was started then STOPPED by the reboot notice;
  it never completed: the tree is not yet proven to configure.

## File map (read so far)
- include/gui_forms/engine/effect/effect.hpp (EffectFrame, EffectDeclaration, EffectInputKind/Event)
- include/gui_forms/engine/effect_registry/effect_registry.hpp, src/core/engine/effect_registry/
  effect_registry.cpp (EffectInstance::paint_view: save, clip_rect(layer), translate, canvas, run;
  queue_input)
- include/gui_forms/engine/effect_canvas/effect_canvas.hpp, src/core/engine/effect_canvas/
- src/controls/range_control/progress_bar/progress_bar.cpp (effect props, on_pointer forwarding
  ~lines 110-126, sync_effects/resolved_effect ~260-320), progress_bar_effects.cpp,
  src/controls/built_in_effect_registry/
- src/core/theme_language/* (motion/theme_motion_lowering.cpp is the likely model for an `effect`
  lowering; not yet read).

## State (2026-11-03, after the reboot)
- Build fix: miniaudio FETCHCONTENT_SOURCE_DIR pre-seeded in both build trees (D:/gui_forms_int's
  _deps/gui_forms_miniaudio-src). Baseline 'effect|progress' 17/17 plain on feacf6d4.
- Item 1 (themes name effects) written on claude/effects-theme-property (uncommitted until the
  sanitize run passes): effect_property "effect" + effect_none_name; RuleDiagnosticKind::effect_invalid;
  Resolution::effect/effect_declared (last matching rule); compiler PropertyClass::effect, lower_effect
  (kebab name -> snake, none/initial = none, inherit refused, malformed = keyword_unknown refusal);
  engine effect_name_valid, EffectSelectionSource/Outcome, check_effect_selection (unknown, parent,
  frame budget 4M ops incl. outsets x scale, theme-only verified), note_effect_refused (counter
  EffectLoadDiagnostics::refused + one stderr line); ProgressBar::requested_effect (app > style >
  theme for part fill, activity while marquee via ThemeEngine::plan), effect_selection().
  Tests: effect_program_tests themes_select_effects; compiler refusals (array 36->40); conformance
  fixture effects.css (+ COVERAGE rows property.effect, property.effect-none). Docs regenerated
  (generate_library_docs.py); theme-language.md row. docs/reference/inventory.json NOT regenerated
  (tools/api_reference.py, not in CI).
- Decision taken: load checks run at the control (registry is in controls lib, theme compile in core);
  compile checks only the name's form. A compile-time registry check would need ThemeCompileInput
  to carry an EffectRegistry (T4's area) - an option to raise with the coordinator.
- Item 2 plan: effect-primary/effect-secondary theme properties, role() only; Resolution colours;
  EffectFrame colours via EffectInstance::set_colours; ProgressBar derives its appearance colours
  from the roles (alphas kept) unless the app set its own appearance; HC via high_contrast_colour.
- Lesson: in python heredocs never write "\n" for C escapes (use chr(92)); `timeout` kills builds.

## State (2026-11-04): items 1-5 committed and verified, reported for checkpoint 16
Branches, stacked (each from the one before, the first from origin/main feacf6d4):
- claude/effects-theme-property 608a81e7: item 1 (EP-3).
- claude/effects-colour-roles 6ad56357: item 2 (EP-4). effect-primary/effect-secondary are role()
  only; scheme via media rule copies; HC via high_contrast_colour; the app's own
  ProgressBarAnimationAppearance comes before the theme roles; roles tint the appearance colours
  at the appearance's alphas.
- claude/effects-pointer-record 7385e0d4: item 3. EffectPointer {inside, pressed, position} on
  EffectFrame only under grant plus declared input (zero otherwise: checksums unchanged);
  EffectInstance::set_input_grant / note_pointer; ProgressBar::on_pointer is one call.
- claude/effects-clip-part-shape c807e299: item 4 (EP-6). Painter::clip_rounded_rect_corners
  (DisplayOperation 26; recording, replay, backplane composite, inspection, Skia exact).
  DECISION: the base fallback and the Skia text clip use the LARGEST radius (never show more
  than the exact clip), not the smallest the plan said. EffectCanvas::clip_part_shape (6, 0);
  EffectInstance::paint takes PartShape; ProgressBar passes its track material's corners. The
  built-in progress effects are unchanged (still clip_rect(fill)), so Gallery pixels are too.
- claude/effects-backdrop 36ee0449: item 5 (EP-7). EffectDeclaration::backdrop +
  backdrop_margin ([0, 8] logical; registration outcome backdrop_out_of_range); box half
  floor(margin*scale/3), image margin 3h; EffectCanvas::draw_backdrop(rect, opacity) (10, 2),
  refused without a lent backdrop; EffectInstance::paint gains `const Control* host`;
  sample_backdrop: engine_paint_place (skips a part in the backplane plane), engine_backplane,
  blur via engine_primitives::BoxBlurWork owned by the instance, result cached by (generation,
  origin, extent, scale, half) and refilled in place only when use_count()==1; reduced
  transparency draws it unblurred. Non-ambient effects get an engine part record (key part =
  effect name) marked as a backplane reader. Selection cost adds 16 ops per grown pixel.
  window.hpp gains ONE line, `friend class EffectInstance;` (no window.cpp change).
Verified each: touched tests plain + sanitized, house style, lifetime, state-machine gate
(--changed-from origin/main). Gotcha: a switch over PointerAction needs a reasoned default (F1).

## RESUME POINTER (2026-11-08 coder; both items registered, nothing uncommitted)
Workspace ~/Developer/gui_forms_workers/w8/gui_forms, base origin/checkpoint/2026-11-07-1 (815f7ae9).
- claude/effects_lane-fd5b 71dcbf0f: FD-5b was already in (af173593 colour, e417548c font; closure row stale);
  adds form_generator_tests test_presentation_refused_registry_wide. Queued.
- claude/effects_lane-fd4-pages 118504bb: the seven FD-4 pages as .form (see queue entry). Laptop checks only:
  a schema/class checker and stub-header syntax checks live in the session scratchpad (not kept).
- Next: take returns from the integrator.

## (old) RESUME POINTER (2026-11-08; HANDOVER to a code-only coder, owner decision; tree clean)
Branch claude/fd-on-m3 in ~/Developer/gui_forms_workers/w8/gui_forms, head bc40b41f, on cp19 835c3599. Every commit is
verified (last rounds: ct19/ct20 gallery, panels, forms, docs, schema plain + sanitize; abi1 abi_|schema plain + sanitize;
state-machine gate clean). Commits: 99820100 child properties; abb4b354 panel settings as properties +
retained-layout-panels.form; b7bf2037 GridPanel/FlexPanel/BlockPanel style + item_style, responsive-layouts.form,
css-layout-panels.form; c49a754a scroller face back in code (pixel-identical capture); bc40b41f ABI conformance kit
expects 3 grid child properties.

DONE (cp20 order): child properties; panel settings as form properties; the three slot pages as .form
(retained-layout-panels, responsive-layouts, css-layout-panels).
REMAINS:
1. FD-5b (not started): .form refusal of colour and font, routed to form_presentation_property (study the existing
   E813 font refusal and the W201 literal-colour warning in the theme/form language; colours and fonts become theme
   classes in demo/themes/gallery/gallery.theme.css).
2. My seven FD-4 pages, none started: dock-and-anchor, docking-workspace, containers-and-focus, tabs-and-pages,
   anchored-overlays, commands-and-menus, live-surfaces (w7 owns the semantics-heavy ten).
3. Rebase onto main when the coordinator says cp19 has merged.

RULES IN FORCE
- Layout panel styles (coordinator, option 1): a text `style` property per GridPanel/FlexPanel/BlockPanel and an
  `item_style` child property, read by LayoutStyleReader and read back by describe_layout_style (helper
  src/controls/scrollable_control/container_control/layout_style_property.{hpp,cpp}). A panel admits only the fields it
  carries. Any reader error OR warning refuses (std::invalid_argument -> form_load_refused), never silently.
  Grid style refuses grid-template-areas: write spans as span blocks (`span name { anchor: r c; rows: n; columns: n; }`,
  anonymous `span { ... }` allowed). item_style refuses placement lines (use `area`/`cell`) and margin lengths (only
  `auto`; lengths are the child's own `margin`). BlockPanel item display is block or inline-block.
- Table: auto placement ignores spans; a child with an explicit span needs an explicit cell, else a load diagnostic.
  group_spec text "176 176 176 priority 2"; GridCell point x = row; track_specs text
  "<fixed|content|remaining> min pref max [weight W] [hidden] [priority N]"; TLP column/row styles "absolute 140",
  "percent 1", "auto".
- Page pattern (FD6.3): gui_forms_add_form(gui_forms_gallery_model demo/pages/X.form CLASS XDocument NAMESPACE
  gui_forms::gallery::pages), plus the form in the gui_forms_add_theme FORMS list; page class calls
  document_.initialize_component(*this) and mount_document(body) (move the root's children into the body; group boxes
  set_style(page_style())); classes name roles ("heading" "page-title", "muted" "body", "positive" "emphasis",
  "paper" "group-caption"); code keeps what a form cannot express (models, handlers, items lists, actuators).
- Any CSS change: update the sheet's sha256 in demo/themes/gallery/theme.json.
- Display digests: tests/gallery_pages/display_trace_tests.cpp; a changed digest needs a history comment and a pixel
  check (Gallery `--page <id> --size 1280x820 --capture`, before/after, diff). Themed sunken faces differ from direct
  strokes in their bottom corners (8 px, delta 11, reported as a possible theme defect), so keep such faces in code.
PITFALLS
- Header-comment code examples are code spans, never indented blocks (the docs generator joins lines; bare * breaks
  Sphinx). New public setters go under their property in docs/library/docstrings/*.rst; regenerate with
  python3 tools/generate_library_docs.py (docs-check fails otherwise).
- .form properties apply in document order: a TrackBar writes maximum before minimum.
- Enum values in .form use dashes; insets follow CSS order (1, 2 or 4 numbers); `bounds` is the authored bounds.
- New registered properties change manifests/control-schema.json (written by gui_forms_gallery_pages_tests with
  GUI_FORMS_CONTROL_SCHEMA_WRITE=1) and docs/controls/generated/schema.md; the ABI conformance kit
  (tests/abi/conformance/gf_conformance_schema.c) counts grid child properties.
- Enum property defaults: BindingValue{std::int64_t{n}}; nullable defaults need descriptor.nullable = true.
- Integrator build notes: rsync keeps laptop mtimes, so touch demo/pages/*.form before building, or a stale generated
  form header survives; sanitize runs exclude labels area_host|area_a11y|gallery_accessibility|foreground; launched-app
  tests run plain only.

## (old) RESUME POINTER (2026-11-07 evening; MILESTONE: three slot pages as .form committed)
- Branch claude/fd-on-m3 on cp19 (835c3599): 99820100 (child properties) + abb4b354 (panel settings, retained page)
  + b7bf2037 "Layout panel styles; Responsive Layouts and CSS Layout Panels as forms" + c49a754a (scroller face in code, ct20). Verified ct19 (21 tests plain +
  sanitize, style, lifetime, docs, schema, gate clean) after ct18 (schema written + copied back, all targets).
- Coordinator ruling (option 1): GridPanel/FlexPanel/BlockPanel `style` (text property) and `item_style` (child
  property), LayoutStyleReader + describe_layout_style round trip (helper
  src/controls/scrollable_control/container_control/layout_style_property.{hpp,cpp}). Refusals throw
  invalid_argument -> form_load_refused; reader warnings refuse too (never silent). Grid style REFUSES
  grid-template-areas (span blocks are the .form spelling; set_template_areas replaces every span, so mapping would
  erase span blocks); item_style refuses placement lines and margin lengths (only auto margins).
- responsive-layouts: tile faces are theme classes (specimen-navigation/-inspector/-activity), painted as part
  rasters, pixel-identical. The scroller's themed sunken face differed in 8 bottom-corner pixels (max delta 11), so
  its face stays set_background in code (c49a754a); 1280x820 capture before/after: 0 changed pixels.
  Digest 0x32350b9adc99d846. Capture tool: dev_w8/capture_diff.sh (plain tree, --page --capture) + png_diff.py
  (run locally: no PIL on the M4). Verified ct20.
  Master/detail roles, command bar items, overflow actuator, slider steps stay in code. css-layout-panels digest unchanged.
- verify.sh now touches demo/pages/*.form first (rsync mtimes left a stale generated form header in the sanitize tree).
  TrackBar in .form: write maximum before minimum (properties apply in document order).
- bc40b41f: ABI conformance kit expects 3 grid child properties, finds item_style by name (abi_|schema plain + sanitize green).
- NEXT: FD-5b (.form refusal: colour/font -> form_presentation_property), then the seven FD-4 pages. Rebase when told.

## (old) RESUME POINTER (2026-11-07; MILESTONE: panel settings + retained-layout-panels.form committed)
- Branch claude/fd-on-m3 on cp19 origin/checkpoint/2026-11-05-4 (835c3599): 99820100 (child properties, ct13 verified)
  + abb4b354 "Panel settings as form properties; Retained Layout Panels as a form". Verified ct15
  (dev_w8/round2.sh, regex.txt responsive_track|layout_panel|gallery_pages|display_trace|command_overflow|form_|
  layout_corpus|doc_examples): 19 tests plain + sanitize, style, lifetime, docs, schema (written + copied back), gate
  clean, all targets. Display trace hash for retained-layout-panels unchanged.
- Panel properties: TLP column_count/row_count/grow_style/cell_border_style/track_spacing/column_styles/row_styles
  (text collections "absolute 140", "percent 1", "auto"); Flow flow_direction/wrap_contents/item_spacing/
  main_alignment/cross_alignment; Responsive orientation/track_gap/track_specs ("<fixed|content|remaining> min pref
  max [weight W] [hidden] [priority N]"); Overflow group_gap. Helper header
  src/controls/scrollable_control/container_control/text_collection_property.hpp.
- Sanitize runs in scripts exclude labels area_host|area_a11y|gallery_accessibility|foreground (verify.sh has it).
  Header-comment code examples: code spans only. M4 lock queues behind other workers (ct15 waited ~25 min).
- OPEN QUESTION sent to the coordinator: css-layout-panels .form spelling of CSS settings: (a) one text `style`
  property per Grid/Flex panel + `item_style` child property, read by LayoutStyleReader (recommended); (b) one
  property per CSS longhand.
- NEXT: responsive-layouts.form (tracks with track_specs/child_track, shelf with group_spec, master-detail and
  scroller; timers/handlers stay in code), then css-layout-panels.form after the answer; then FD-5b; then FD-4 seven.
- After cp19 merges: rebase onto main when told.

## (old) RESUME POINTER (2026-11-05 late; PAUSED by the owner; nothing uncommitted)
- Branch claude/fd-on-m3 rebased onto main 9e669353 (#469 merged), no conflicts: a65130fb (= f02353fb, in cp19) +
  70cb6acb (= caa2cc83, child properties, for cp20). Programs configuration check on the plain tree
  (dev_w8/demoboard.sh: -DGUI_FORMS_BUILD_FILE_MANAGER_DEMOBOARD=ON, ninja -k 0, ctest -R demoboard): build passed,
  2/2 demoboard tests passed. Note: the plain tree now keeps that option ON in its cache.
- After cp19 merges: rebase once more (drop a65130fb if contained).
- NEXT (unchanged): panel settings as form properties (study 10 s6; define_bindable_property + enum descriptors as
  Panel border_style does): TLP column/row count, grow/cell-border style, track spacing, column/row styles as text;
  flow direction/wrap/spacing/alignments; responsive orientation/gap/track specs; overflow group gap; grid templates,
  gaps, alignments, auto flow; flex direction/wrap/alignments/gaps. Then the three slot pages, FD-5b, FD-4 seven.

## (old) RESUME POINTER (2026-11-05 night; MILESTONE: child properties committed)
- Branch claude/fd-on-m3 on main 4ab9a107: f02353fb (attached props + GridPanel spans) + caa2cc83
  "Child properties: table placement, track and group spec" (35ef66f6 squashed in). Verified: ct11 (21 tests plain +
  sanitize, docs, schema, all targets) with only form_model failing on the new property order; fixed; ct12 (form
  tests plain + sanitize + checks) green; schema rewritten (schema2) and copied back. Gate: only the 2 known F1s in
  tests/macos_message_dialog_key_tests.mm (not mine).
- m4ws SYNC LESSON: a later m4run syncs the laptop tree over the M4 with --delete, so copy M4-written files
  (control-schema.json) back IMMEDIATELY after the run that writes them, before any other run.
- cp20 order (coordinator, PLAN_CP20.md): finish child properties -> three slot pages as .form -> FD-5b (.form
  refusal: colour/font -> form_presentation_property) -> FD-4 layout/panel pages (send the coordinator my proposed
  list first; w7 takes semantics-heavy ones).
- Coordinator answers: (1) auto placement ignores spans in cp20; a child with an explicit span needs an explicit
  cell, else a load diagnostic (setter refusal -> form_load_refused). (2) group_spec is a child property whose
  .form value is text: "176 176 176 priority 2" ("" = none).
- FD-4 split (coordinator): mine after slot pages + FD-5b: dock-and-anchor, docking-workspace, containers-and-focus,
  tabs-and-pages, anchored-overlays, commands-and-menus, live-surfaces (+ css-layout-panels, responsive-layouts,
  retained-layout-panels). w7 takes the semantics-heavy ten. macOS launched-app tests: plain only from now on.
- Slot pages need panel properties (study 10 s6) that are NOT yet properties: TLP column/row count, grow/border style,
  styles as text collections; flow direction/wrap/spacing/alignment; grid templates; track specs; group gap.
  Panels use define_bindable_property (old style, like ScaledPanel design_size) until M-G.
- m4run LESSON: never pass a regex with | as an argument (the remote joins words; zsh pipes it). round2.sh reads
  dev_w8/regex.txt. ct9 lost an hour to that.
- NEXT: slot pages. First the panel properties of study 10 s6 (define_bindable_property, like ScaledPanel), then
  retained-layout-panels.form (flow cards + table), css-layout-panels.form, responsive-layouts.form; code keeps what
  forms cannot express. Then FD-5b, then FD-4 pages.

## (old) RESUME POINTER (2026-11-05, stopped for the app restart; WIP on top of the milestone)
- Branch claude/fd-on-m3: f02353fb (milestone, verified ct8 + schema regenerated) + WIP 35ef66f6 (HEAD, UNVERIFIED):
  ResponsiveTrackPanel child_track as a child property (ChildTrackCapability, view value std::optional<std::uint32_t>,
  nullable unsigned "child_track", none = automatic; set_child_track/child_track() removed, callers in
  responsive-layouts.cpp, demo/responsive_tracks_lab.cpp, doc_examples_tests, responsive_track_panel_tests migrated;
  new test_child_track_child_property; ResponsiveTrackPanel.rst documents it; generated docs regenerated locally),
  and the stale set_flow_break/flow_break entries dropped from docs/reference/contracts/flow_layout_panel_compat.json
  (coordinator request). Round ct9 (dev_w8/round2.sh: schema write, verify full, all targets) was ABANDONED.
- NEXT: rerun `m4run w8 -- sh dev_w8/round2.sh "responsive_track|doc_examples|layout_panel|gallery_pages|layout_direction" origin/main`
  (copy dev/tools/round2.sh + schema_write.sh into gui_forms/dev_w8 first; scp manifests/control-schema.json back;
  docs/reference/inventory.json may need regenerating if docs-check flags set_child_track). Then squash the WIP.
- Coordinator approved order: TableLayoutPanelCompat cell/spans onto GridPanel spans (10.9 s5), ResponsiveTrackPanel
  child_track (WIP done), CommandOverflowPanel group_spec, then the three slot pages as .form.
- OPEN, to ask the coordinator (options + recommendation) before coding:
  1. TLP spans for AUTO-PLACED children (layout_corpus_tests sets spans on children without a cell; 10.9 s5 anchors
     spans at the child's cell only). Options: (a) per-child column_span/row_span child values kept as the WinForms
     surface, placed children mirrored into the table's GridSpan list; (b) span writes on an automatic child refused;
     (c) pending extents per automatic child applied at auto-placement. Recommend (a)/(c). Also overlap: WinForms lets
     spans overlap, GridSpan rules refuse; divergence record needed.
  2. CommandOverflowGroupSpec is a struct (min/preferred/max + optional priority), no property kind. Options:
     (a) four child properties group_minimum/preferred/maximum + collapse_priority; (b) one text-kind spec
     "270 270 270 priority 2"; (c) typed view only, not in .form. Recommend (a).

## (old) RESUME POINTER (2026-11-05 11:20, on the Mac; rebased onto main 4ab9a107, schema regenerated)
- Branch claude/fd-on-m3 = f02353fb, ONE commit "Attached properties and GridPanel spans (studies 10, 10.9, 15.4)"
  on origin/main 4ab9a107 (cp18). ct8: 18/18 touched tests plain, all targets built, docs --check passed.
  f02353fb only adds the regenerated manifests/control-schema.json + docs/controls/generated/schema.md
  (grid_panel area/cell, flow_layout_panel_compat flow_break). The schema is written by gui_forms_gallery_pages_tests
  with GUI_FORMS_CONTROL_SCHEMA_WRITE=1 (dev_w8/schema_write.sh, ~15 min on the M4); add gallery_pages to test regexes.
- Demoboard checked: no set_flow_break/flow_break( calls on this branch or origin/checkpoint/2026-11-05-3.
- Coordinator: GridCell as a point (x = row) and `area` as a quoted string in .form stand for now.
- NEXT: the three slot pages as .form (retained-layout-panels, css-layout-panels, responsive-layouts). They still need
  per-child setters turned into child properties first: TableLayoutPanelCompat set_cell_position/set_column_span/
  set_row_span (10.9 section 5), ResponsiveTrackPanel set_child_track, CommandOverflowPanel set_group_spec;
  plus study 10.6 panel properties (column/row counts, styles as text collections), grid track lists, flex slots.

## (old) RESUME POINTER (2026-11-05 09:40, on the Mac; MILESTONE: attached props + GridPanel spans committed)
- Branch claude/fd-on-m3 = 9d2fa306 (one commit on cp18 dfa4f9b5 = origin/checkpoint/2026-11-05-2): attached
  properties (child tables, ChildPropertyView, flow_break) + GridPanel spans/placement + .form span blocks
  (E814). Backups of the older WIP: backup/claude/fd-on-m3 (27a0c7db base). Reported to the coordinator.
- Verified on the M4: 18 touched tests plain + ASan/UBSan (regex in dev/tools/round.sh call:
  grid_panel|form_|layout_panel|layout_direction|layout_corpus|property_capability|binding_tests|
  theme_conformance|theme_syntax|schema_export|retained-layout|css-layout), every target built (ninja -k 0),
  style, lifetime, docs --check, schema check/render --check. Gate: only tests/macos_message_dialog_key_tests.mm
  225/236 F1 (not in this diff). Windows/Linux: CI only.
- Scripts: dev/tools/verify.sh, round.sh, final.sh (copies in gui_forms/dev_w8/, git-excluded). m4ws runs the
  remote command as words joined by spaces: never `sh -c "a; b"` (it breaks); put sequences in a script.
  Do not edit the tree while an m4run waits for the lock (the sync happens when the lock is taken).
- Next: (1) the three slot pages (retained-layout-panels, css-layout-panels, responsive-layouts) as forms using
  flow-break / span / area / cell; (2) remaining FD6.3 pages; (3) TableLayoutPanelCompat mapping onto spans
  (10.9 section 5), if the coordinator wants it now. Rebase onto cp18's landed main when told.
- Deferred, in the report: auto-placement ignores spans; `area` is a quoted string in .form.

## (old) RESUME POINTER (2026-11-05, on the Mac; attached props + GridPanel spans WIP)
- Workspace now ~/Developer/gui_forms_workers/w8/gui_forms (laptop). Branch claude/fd-on-m3 checked out from
  backup/claude/fd-on-m3 (base 27a0c7db, inside cp18 dfa4f9b5; cp18 not landed yet: main = cp17 ebb9fbf4).
  backup/claude/attached-props is the older pre-cp18 WIP (superseded by fd-on-m3).
- Scripts: dev/tools/verify.sh (master copy) is copied to gui_forms/dev_w8/verify.sh (git-excluded via
  .git/info/exclude; only gui_forms/ and template_harness/ sync to the M4). Run:
  `cd w8 && ../bin/m4run w8 --label X -- sh dev_w8/verify.sh quick|full|checks "'regex'" [BASE]`;
  logs on the M4 in w8/dev/logs/verify-*.log (read with `m4run w8 --log ../dev/logs/verify-plain.log`).
  Use BASE 27a0c7db for the gate until cp18 lands. m4run moves to background after 180 s: run it with
  run_in_background and wait for the notification.
- Fixed: form_model_tests BindingValue{1U} ambiguity. Written (uncommitted, round ct3 running):
  * core include/gui_forms/layout/grid_span/grid_span.hpp + src/core/layout/grid_span/grid_span.cpp: GridCell
    (PropertyValueTraits: Point x=row y=column), GridSpan, GridSpanOutcome, maximum_grid_span_extent (=10000,
    static_assert vs maximum_grid_tracks), grid_span_extent_valid/cell_admitted/contains/interior/overlap,
    HoldsGridSpans<T> opt-in trait (GridPanel specializes it).
  * GridPanel: GridAreaCapability/GridCellCapability (area[*c], cell[*c]), GridPlacement, define_span,
    remove_span(name|anchor), set_spans, spans(), placement(child); template_areas regenerated from named spans
    within max(recorded extent, span extents); set_template_areas replaces all spans (throws on refusal);
    set_item_layout lines replace area/cell and vice versa; child_leaving erases records; cell has no default.
  * .form: `span [name] { anchor: r c; rows: n; columns: n; }` only inside holds_spans types
    (FormSchemaType::holds_spans/define_span via form_control_type + HoldsGridSpans); FormModel::spans;
    E814 form_span_conflict (overlap, extent, area naming no span, cell inside a span; repeated name = E806);
    loader defines spans before properties; generator emits define_span before placements (+grid_span.hpp,
    <stdexcept>); editor skips span blocks in find_object/direct_children(object_rule).
  * docs: GridPanel/FlowLayoutPanelCompat docstrings, manual.json stale flow_break rows removed,
    tools/api_docs_rst.py CELL_FIELD admits ChildPropertyView; generate_library_docs.py run locally (it also
    regenerates cp18 noise in Window.rst etc. because the base predates cp18's regenerate).
  * tests: grid_panel_tests test_spans_and_placement; form_model_tests test_spans; form_loader_tests
    test_spans; form_generator_tests test_spans.
- dev/tools/round.sh (copy in gui_forms/dev_w8/): build + control_schema render + verify full; then copy the
  rendered schema files back from the M4 (status in ../dev/logs/status.txt on the M4).
- Deferred (to report): auto-placement does not treat a span as one region (solver unchanged, as CSS ignores
  named areas); TableLayoutPanelCompat mapping (10.9 section 5); `area` takes a quoted string in .form
  (text kind rule), not the study's bare ident. Then: commit (squash WIPs), slot pages.

## (old) RESUME POINTER (2026-11-05, before the reboot; UNPARKED: attached properties and table spans)
- UNPARKED by the coordinator after W's M3: attached layout properties (study 10 + W's 15.4: child values keyed
  by RuntimeId, erased in child_leaving; borrow proxy with && write; hooks prepare_write(view, child, v&) and
  written(view, child); ChildPropertyEntry thunks on Component&; no connect_changed/origin), then GridPanel
  spans + placement (study 10.9, owner APPROVED Q1 (a) GridPanel, Q2 (i) area[*c]/cell[*c], Q3 (a) refuse
  removing a span in use, Q4 (a) justify_self/align_self are Control properties that W adds after G9 with my
  SelfAlignment {automatic,start,center,end,stretch} shape), then the .form `span` syntax + E814
  form_span_conflict, then the three slot pages and the remaining FD6.3 pages. Lands with cp18/cp19; restack
  after cp18 lands. Shared build slot only; foreground Gallery runs on private desktops only.
- Refusals: they are in cp18 (cp18d/trial has them as abd7ad79..af173593 with the fold's `classes` resolution
  in M2's constant Control table). My origin/main restacks (claude/form-font-refusal eba2dbc2,
  claude/form-colour-refusal ca1ed997) are superseded by cp18; delete when cp18 lands.
- WIP branch claude/fd-on-m3 (D:/gui_forms_w8/gui_forms, checked out) = w3 cp18d/trial 27a0c7db (fetched as
  refs/remotes/w3/cp18d) + ff4473de (model: FormSchemaType::child_properties, FormPropertyProvider,
  descriptor_position, E804 detail naming the parent's type; model tests) + 9410a279 (ChildPropertyEntry and
  templated helpers in property_table.hpp/.cpp; include/gui_forms/binding/child_property_view; Control::
  child_property_table/child_property_descriptors/child_property_value/set_child_property_value/
  reset_child_property/should_serialize_child_property/require_direct_child/child_leaving (called in
  remove_child and on_dispose); FlowLayoutPanelCompat FlowBreakCapability + child table, set_flow_break/
  flow_break(child) removed and callers rewritten to flow_break[*child]; form_property_descriptor; loader and
  generator write parent-provided properties via set_child_property_value (name-keyed until M-G);
  describe_form_type fills child_properties; schema export child_properties (only when non-empty) +
  control_schema.py snake/name-clash check + render line; tests: layout_panel_tests
  test_flow_break_child_property, form_generator_tests test_attached_properties). NEVER BUILT.
- Next: build `sh dev/tools/build_tests.sh 'form_|layout_panel|layout_direction|property_capability|binding_tests|theme_conformance'`
  (log dev/logs/ct1.log); fix; write_schema.sh (flow_layout_panel_compat gains child_properties), fg_gallery,
  verify.sh with base w3/cp18d; squash the two WIPs; report. W's M3 (cbe8202c) is NOT in this base (it
  conflicts with cp18's overlay rename; not needed here).
- Then GridPanel spans: GridSpan{name, GridCell anchor{row,column}, rows, columns} list as the single storage;
  style_.template_areas regenerated from the named spans (keep set_template_areas' dims as a recorded extent);
  placement -> GridItemStyle lines at layout (area: four area lines; cell: line r+1/c+1 with span R/C of an
  anchored span, else span 1; automatic). GridPanel callers: css-layout-panels.cpp:115, grid_panel_tests 251/285,
  layout_corpus_tests 998; item layout API set_item_layout/item_layout (GridItemLayout.placement) to fold into
  placement.

## (old) RESUME POINTER (2026-11-05, after the restart; forms/Gallery lane, PARKED after the refusals)
- OWNER (via coordinator, 2026-11-05): finish the font refusal, then colour + form_style_block, report them,
  then PARK the Gallery page conversions and the attached layout properties until W's M3 lands. Builds use only
  the shared slots (build_slot.py already keeps non-critical runs off slot 0).
- VERIFIED, reported: claude/form-font-refusal 0163f7d4 (font stage, squashed; base w1/t4-fonts 25e8d85a;
  cp17 has 25e8d85a but not c367a5ca, which sits below it here) and claude/form-colour-refusal 615df70e on it
  (colour stage: E813 also refuses colour-kind properties that are not content values; 810 renamed
  form_style_repeated -> form_style_block, every `style { }` in an object's block refused while the syntax
  still parses it; FormObject::style removed; model/loader test schemas mark back_color content_value to keep
  colour value coverage; sample/note dialogs and page descriptors drop colours; corpus forms use classes).
  Font: 14 tests plain, fg_gallery 4/4 + lock 2/2 (display traces unchanged), checkers clean. Colour: 12
  tests plain, checkers and gate clean (no demo change, so no fg run). Main tree has the colour branch checked out.
- Next when unparked: rebase both onto origin/main once cp17 lands (T4 under them); they fold after T4.
- PARKED until W's M3: more FD6.3 pages (next smallest: checked-collections, choices-and-sections,
  dates-and-calendar; they need CheckedListBox items with check states, CommandBar items, ListView columns as
  form properties) and attached layout properties (study section 10).

## (old) RESUME POINTER (2026-11-05, before the machine restart; forms/Gallery lane)
Scope (owner, 2026-11-05): W's form and Gallery work moves here (W is property capabilities only).
(a) FD6.3 Gallery pages: convert the rest smallest first, SKIP the slot pages (retained-layout-panels,
css-layout-panels, responsive-layouts); checksums may change with explained differences. (b) .form presentation
refusal E813 form_presentation_property -> project theme: fonts now (T4's fonts verified), colours together with
form_style_block (sibling error; the corpus forms settings_dialog and spacing_and_comments move their styling
into a theme); TypographySpecimen.font exempt (content_value); variant enums stay. (c) classes + G3 + Gallery
theme (done). Attached layout properties: design APPROVED by the owner, with one condition: generated code uses
the name-keyed set_child_property_value only until W's M1 capability tables exist, then moves to typed
child-property capabilities in the same M-G step that migrates the property tables. Design text is section 10
of desks/coordinator/PROPERTY_CAPABILITIES_STUDY.md. Implement after the convertible pages.
- VERIFIED, reported to main: claude/fd6-pages-3 14a2a56e on cp16 8b7b470a (3 commits: classes + G3
  Label/Panel/GroupBox faces + tests/theme_rule_faces_tests.cpp; content_value + TextBox multiline/read_only/
  placeholder_text, CheckBox three_state, RadioButton group_name; gui_forms_add_theme + demo/themes/gallery +
  page colours to classes + semantic-feedback/control-spectrum forms + 3 explained digest re-records).
  15 touched tests, fg_gallery, style/lifetime/docs/registry/schema/gate all clean.
- WIP (checked out in D:/gui_forms_w8/gui_forms): claude/form-font-refusal c68c265c = w1's claude/t4-fonts
  25e8d85a (fetched as refs/remotes/w1/t4-fonts) + the 3 fd6 commits cherry-picked (0b10d98d 94a2fcaa d6c79913)
  + WIP E813 font stage: DiagnosticCode form_presentation_property=813 (severity, name, COVERAGE row);
  form_model read_declaration refuses kind font unless content_value; the Gallery's 87 fonts -> 13 type classes
  (page-title body body-emphasis emphasis strong note note-emphasis note-strong tag group-caption control-title
  control-body card-title) in gallery.theme.css (sha in theme.json); generator tests moved to
  typography-specimen + test_presentation_fonts; sample_dialog heading font dropped (golden edited by hand).
  NOT YET VERIFIED: a build_tests run ('form_|theme_rule_faces|semantic_gallery|binding_tests|
  theme_conformance|theme_compiler', log D:/gui_forms_w8/dev/logs/ff1.log) was in flight at the restart.
- Next: read ff1.log or rerun it; fix; check 'group-box' is a valid form type in test_presentation_fonts;
  fg_gallery.sh, expecting display traces UNCHANGED (theme fonts must equal the old FontSpecs; a diff means
  checking T4 font resolution, e.g. letter-spacing 0.24px, 10.5px); verify.sh with base w1/t4-fonts; squash
  the WIP; report to main (folds after T4 in cp17).
- Then: E813 colour stage + form_style_block (back-color in form_model_tests:138, form_loader_tests:113,
  form_generator_tests page descriptors, form_application_type_tests:48, generated/note_dialog.form,
  sample_dialog.form, corpus forms -> a theme); more pages; attached layout properties.
- Tools: D:/gui_forms_w8/dev/tools (build_tests.sh, verify.sh, fg_gallery.sh, write_schema.sh, redigest.py,
  trace_now.sh, trace_cp16.sh with the cp16 worktree detached at 8b7b470a, fonts_to_classes.py).
- Lessons: background-color lies UNDER recipe gradients (use `background`); rule primitives paint via
  draw_part_raster; stock anatomies are needed for ::part names; colour roles are a FIXED vocabulary and a
  package :root palette re-values them for its own rules only; content_value is C++-only (no C ABI flag);
  the Bash tool turns a typed backslash-n into a newline, so write C escapes with Write/Edit.

## (old) RESUME POINTER (2026-11-04, lane paused by the owner; the cp16 head is verified)
- cp16 head: claude/fx-cp16 f61151aa on cp16/m0 c8439042 (S1.1, S1.2 and S2.0; T4's claude/t4-cp16
  90294c8c is on the same base). Six commits: items 1-5 plus "reach the surface root through
  attached_window" (EffectInstance is a friend of SurfaceRoot next to ThemeEngine; surface_root()
  is protected, and inside a sub-surface attached_window() is null, so no backdrop is lent there,
  as with glass). Clean restack. Targeted plain: 28/28 (effect_program, every effect_pass case,
  range_controls, engine_input, engine_glass, display_chunk, opaque_live_surface_raster, material,
  visual_inspection, theme_compiler, theme_conformance, theme_cascade), under build_slot.py.
- Where: worktree D:/gui_forms_w8/cp16/gui_forms (its own build D:/gui_forms_w8/cp16/.build,
  template_harness worktree at fb752e71 beside it, third_party freetype/harfbuzz/libunibreak copied,
  stx_auxlib and miniaudio sources from D:/gui_forms_int's _deps). Script: dev/tools/cp16_targeted.sh.
- Stale: the claude/effects-* branches in the main tree (S1.2 d1f5aa8e base; effects-backdrop
  8d27d451 has the same fix). claude/fx-cp16 supersedes them; delete them when the coordinator
  names them as contained. The cp16/m0 ref itself has since moved to bb709955 (same subject as
  c8439042, restacked); rebase again only if told.
- Next: nothing until the fold; then rebase onto origin/main and delete the contained branches.

## (old) RESUME POINTER (2026-11-03, before the Shadow reboot)
Re-run `PLAIN_ONLY=1 sh /d/gui_forms_w8/dev/tools/check.sh 'effect|progress'` (first build; in the
background, it is long), then read theme_language motion lowering + engine/theme_rules to design
item 1 (how a rule's per-part `effect` reaches ProgressBar::resolved_effect, with load checks and the
fallback rule diagnostic). Then items 2-5 in order, each its own branch/commit.

## M-S8 resume (2026-11-08, w8)
Branch claude/effects_lane-ms8 (base w6 families_b-b5 b06b3253), head 479f9b1f, all registered. Done: Splitter, SplitContainer (+values header), ContainerControl, SplitterPanel, Docking (DockSite, DockNavigator, DockWorkspace), LinkLabel, DrawingSurface, MetricsView, ExpandableSections.
Next: DropDownButton (width/edge as views: their setters adjust content_padding from the old value; mode cell set by the constructor), RasterCanvas, MasterDetailView, BindingNavigator; then pages and strips, ribbon, components (check which B9 already did). Skipped as methods only: CommandBar (set_items), DataGridViewRowHeights (set_height indexed).
Scratchpad helpers: rw.py (setter/getter rewrite), manual_rm.py, docs.sh, sweep.sh, syn.sh.

2026-11-08 resume (w8, M-S8): remainder list done on claude/effects_lane-ms8 (base b06b3253), head 8fece002, all queued. Commits after 4e12ae67: 56083b9a PageStrip, 4baf3943 TabControl, 8dde157d TabPage, 322dedfe PageView, 6e2a44a1 NavigationPane, e47b9db6 BackstageView+QuickAccessToolbar, 402ba1f1 Ribbon, eaf53a6e RibbonGallery, 0bc0bd4f RibbonFontControl+RibbonColorPicker, 5fe85678 ErrorProvider, 8fece002 HelpProvider+RibbonToolTip+ContextMenu. None compiled (laptop clang barred since the pause; no M4 check run). Pending: rebase onto w6's families_b base fix once it registers.

2026-11-08 FD-4 (w8): claude/effects_lane-fd4-gallery e90a7aee on 7e317d4e, queued. All 28 Gallery pages are .form now (the last seven carried from 118504bb onto M-S8, with FD-6 bind() for check boxes, Commands and the interval). Not compiled. Next: integrator returns.

2026-11-08 rule engine (w8): claude/effects_lane-rules on e90a7aee (head in the queue entry). Part C rules per APP_WINDOW_RULES_DESIGN §C/§D: gf::Rule aggregate, closed vocabulary (key/WinForms mouse/focus/interaction/command/value triggers; key_down, modifiers, hovering, pressed, focused, focus_within, enabled, visible, window_active, is_true, equals with not_ forms), program actions + builtin invoke/set/toggle/focus, RuleSet (consume, duplicates, tokens, stale on disposal), gf::on one-call spellings, GF_TABLE_RULES 18 (since 1.1) with conformance part, docs/user/rules.md. Written against W's claude/interaction-state names (Control::mouse_down() etc., SurfaceRoot::hovered_control/interaction_snapshot/dispatch_context_request); needs W merged first. Key route hook is one line in SurfaceRoot::route_key for the integrator (W's window.cpp untouched). Not compiled. Next: integrator returns; cp25 migration (examples, Gallery rules).
2026-11-08 rule engine rebased onto W's ef378c38; Q-C5 hooks added in raise_mouse_events and route_key (head 0da5baa1, queue entry). ABI regeneration with the integrator. Next: integrator returns; cp25 migration.
2026-11-08 cp25 C docs and tests on claude/effects_lane-rules @ 60c915a1 (mouse-events.md, mapping rows, examples/reference/mouse_events.cpp, mouse_event_order_tests).
2026-11-08 cp22: X-2 docs on claude/effects_lane-rules @ 64b55e13; X-3 template_harness claude/x3-reference-app @ 335f5b8; FD-7 draft notes claude/fd7-gate @ 3b4de40 (worktree w8/notes_fd7).
2026-11-08 Gallery panes (w8): claude/effects_lane-gallery-panes @ f9363a2c on 389ff85e: input-and-rules and theme-settings panes plus registration, queued with audit and dependencies. Not compiled.
2026-11-08 Gallery panes moved onto W's c64cdee2 page_builders: claude/effects_lane-gallery-panes-2 @ 4a4b1179 (show_window_theme). Supersedes gallery-panes f9363a2c.
2026-11-08 owner bugs on panes: claude/effects_lane-gallery-panes-3 @ 0373969c on 5b6f0235 (log history, clipped captions).
2026-11-08 run(unique_ptr) docs sweep: claude/effects_lane-run-unique @ 51f79f73 on batch10; APP_WINDOW_RULES_DESIGN edited in place.

- 2026-11-08 stx move (owner 11-05): claude/effects_lane-stx-move @ 7a1d0130 on batch10 06a78004; stx side local branch claude/stx-move-sync @ 1f5c6f54 in w8/stx (unpushed, must land first). Queued HIGH RISK; nothing built.
