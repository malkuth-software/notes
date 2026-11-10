# T4: the default theme (light and dark) and the Windows-familiar themes

Worker track for stage T4 (first stage of the arc to 0.5). Coordinator owns
merges; this worker commits on local branches only and never pushes
gui_forms. Only this file is pushed (projects main).

## Map

- Workspace: D:/gui_forms_w1 (gui_forms/, template_harness/ at the
  harness.lock pin 523c2a7, notes/ = sparse notes clone, plan-paint/).
- Build: `sh /d/gui_forms_w1/build.sh '<regex>'` (Windows, Skia ON since
  2026-11-01, tree .build/native-windows-x64/gui-forms; this tree has no
  Gallery tests, the sanitizer tree has them). Sanitizer:
  `sh /d/malkuth/projects/tools/sanitize.sh /d/gui_forms_w1 '<regex>'`.
- Python: C:/Users/Shadow/AppData/Local/Programs/Python/Python314/python.exe.
- One-step verify: `sh /d/gui_forms_w1/dev_t4/verify.sh '<regex>' [nosan]`
  (build+tests, sanitizer, style, lifetime, census, census tests,
  state-machine gate vs $T4_BASE, default origin/checkpoint/2026-11-01-4).
  Logs dev_t4/logs/. Reference copies of planning docs: dev_t4/ref/.
- Key code: theme records `src/core/theme/theme/theme.cpp`
  (windows_professional_theme_definition, default_theme, HC derivation),
  types `include/gui_forms/theme/types/theme_types.hpp`, paint path
  `include/gui_forms/theme/part_painting/part_painting.hpp`, engine built-in
  rules `src/core/engine/theme_rules/built_in_rules.cpp`, colour roles
  `src/core/engine/theme_rules/colour_roles.cpp`, compiler API
  `include/gui_forms/theme/theme_compiler/theme_compiler.hpp`.
- Planning: notes gui_forms branch planning/stages/T4-default-theme/
  (README, RENDERING_REFERENCE, PLAN.md, STYLE_MAPPING.md).

## State
- 2026-11-04 cp16: claude/t4-cp16 2644e5c2 (on the standby's trial cp16/m0 c8439042) replaces t4-themes and t4-variants;
  variants theme state lives on SurfaceRoot. Touched tests plain 68/68, foreground set plain 12/12, digests re-recorded.
  Fonts: claude/t4-fonts 658e5aa5 (WIP, on the old t4-variants 0c9c37cc) is built; theme_font_tests pass, the
  conformance not-admitted fixture needs updating; rebase it onto t4-cp16 after the fold (control.cpp uses
  surface_root_ now, not window_). Landing phase: nothing new until cp16 folds.

- 2026-11-01: started. gui_forms branch `claude/t4-themes` from
  origin/checkpoint/2026-11-01-4 (T3 complete). Notes branch
  `claude/t4-plan` (local, D:/gui_forms_w1/notes) holds PLAN.md and
  STYLE_MAPPING.md.
- Phase 1 DONE (2026-11-01): gui_forms `claude/t4-themes` = 03efd251
  (paint census tool + baseline manifest + unit tests; contrast report
  src/core/theme/contrast/theme_contrast.{hpp,cpp} + tests/theme_contrast_tests.cpp).
  Verified: theme_contrast|part_painting 2/2 plain and sanitized, style,
  lifetime, census, state-machine gate clean.
- Notes `claude/t4-plan` (local): a49e76e PLAN.md + STYLE_MAPPING.md,
  46a366a results/2026-11-01-t4-phase1 (census 47 stock files / 400 calls;
  windows-professional contrast 1688 rows, 466 fail; accent button hot etc.
  white on near-white).
- 2026-11-01: owner approved T4-1..T4-4 (coordinator). Done since:
  - `claude/t4-themes` 5395e20e: paint census in the house-style CI job
    (native.yml), exemptions for EasingPreview, TypographySpecimen,
    MetricsView, PictureBox (T4-4).
  - `claude/t4-accent-contrast` f797e60c (from the checkpoint, separate
    small branch): windows-professional accent states meet 4.5:1
    (fill_accent_states in theme.cpp, theme_tests check, 11 Gallery
    display-trace digests re-recorded: only gradient first stops changed).
    Verified theme_tests + gallery pages/trace/interaction sanitized,
    style, lifetime, gate.
  - Notes `claude/t4-plan` a22a9b2: spec §8.5 authored colours, §12.14
    T4-1..T4-4; plan and mapping marked accepted (coordinator folds it).
  - Found: plain tree had Skia OFF by mistake; fixed (build.sh flag).
  - UIA Gallery easing-preview RangeValue mismatch seen once (42-min
    sanitizer run); main 7a6ba09c (cancel_frame_requests stops timers
    waiting for a paint) addresses that cause.
- T4-1 DONE: `claude/t4-themes` 45f3203d before rebase (authored
  background/foreground on PartPaint/PartGeometry; schema value
  `authored_color`; manifest, schema.md, docs/api regenerated;
  docs/reference/inventory.json NOT regenerated: api_reference.py fails on
  a stale ToolTip contract already on main, and it is not in CI).
  Verified 57 plain / 61 sanitized (part_painting, engine_*, theme_*,
  gallery pages/trace/interaction, doc_examples, schema), style, lifetime,
  census, gate, control_schema render --check, generate_library_docs --check.
- Rebased onto origin/main 3e0c8d5a (checkpoint 2026-11-01-4 merged):
  `claude/t4-themes` (3 commits) and `claude/t4-accent-contrast` (1).
  Main changed no paint code since the old base, so the accent digests
  stand. Set T4_BASE=origin/main for verify.sh from now on.
- Phase 2 DONE (2026-11-01): `claude/t4-themes` ee1da5c3 (adds commits only;
  W stacks DataGridView batch 2 on this branch, so never rewrite it).
  `gui_forms_specimen_sheet` (tests/specimen/specimen_sheet.cpp, CMake
  target under `if(TARGET gui_forms_gallery_model)`, ctest
  gui_forms_specimen_sheet_tests runs --smoke). Plain tree now has
  GUI_FORMS_BUILD_GALLERY=ON. Full run: from the build tree,
  `./gui_forms_specimen_sheet.exe --out ../../../dev_t4/specimen_full
  --repeat` (~9 min; single run ~4.5 min; source env.sh first, no
  `timeout` wrapper: it breaks ccache). Gate: 64 controls x 7 states x 6
  configs = 2688 cells (2106 applicable), 0 differ across two captures,
  2162 contrast runs under threshold, classified A-E. Notes `claude/t4-plan`
  95554fb results/2026-11-01-t4-phase2 (README, manifest, contrast, groups,
  sheets.sha256). Tools: dev_t4/tools/phase2_contrast.py,
  phase2_classes.py. Verified: specimen test plain + sanitized, style,
  lifetime, census, gate (T4_BASE=origin/main).
- Plan Paint pin moved to 5e28a5e (coordinator, option 1; history reset
  2026-10-27): notes `claude/t4-plan` 4044f49 (PLAN.md note, README,
  RENDERING_REFERENCE, DESIGN_LANGUAGE_SPECIFICATION, stages/README).
- Phase 3 light DONE (2026-11-01): `claude/t4-themes` 143a1cbf.
  `gui_forms_theme()`/`gui_forms_theme_build()` in
  src/core/theme/gui_forms_theme/ (floor = Plan Paint recipes in C++, light
  sheet raw string compiled at first use via
  detail::compile_built_in_theme in theme_compiler.cpp);
  `stock_control_anatomies()` (tools/stock_anatomies.py, --check in CI);
  contrast report: house ring measured against the face, selection
  boundary gated when selected/hot/pressed (windows-professional now 1680
  rows / 464 fail). Test gui_forms_gui_forms_theme_tests: 0 diagnostics,
  8711 ops (bound 64/byte), ~2 ms, round trip, 840 light rows pass.
  Specimen `--theme gui-forms --light [--repeat]`. Verified plain +
  sanitized (theme/contrast/part_painting/specimen set), style, lifetime,
  census, gate. Record: notes `claude/t4-plan` 697e8de
  results/2026-11-01-t4-phase3-light (README, reports, manifest, light-1x
  sheets for the owner's first look).
- Owner's first look at the light sheet (2026-11-01, via coordinator): "it
  looks good". Recorded in notes `claude/t4-plan` f502277 (phase 3 README).
- Checkpoint 10 merged (main e22ea9c9, contains through 7f8b0b76).
  `claude/t4-themes` rebased with `git rebase --onto origin/main 7f8b0b76`
  (a plain rebase replays the folded commits and conflicts); old head kept
  as `claude/t4-themes-pre-cp10`. W must restack on the new hashes.
- Phase 4 batch 1 (2026-11-01), on `claude/t4-themes` after the rebase:
  670e82d1 phase 2, d5eb4935 phase 3, b2c39ba4 PageStrip/ToolStrip
  PartKeys, bf601706 light sheet tab stack/command plane as valid
  materials, 3cfae07c MenuStrip face + item plates via paint_part (census
  380 -> 378). Lesson: a material takes one border, or per-edge borders
  only square; an invalid assembled material is refused
  (engine_primitive_refusals) and falls back to the recipe silently; probe
  with dev_t4/scratch/plan_probe.cpp (temporary CMake target; restore
  CMakeLists from dev_t4/scratch/CMakeLists.bak). Tab stack and menu strip
  command plane now draw in the light sheet; windows-professional display
  traces unchanged; light contrast groups unchanged (disabled tab on
  #becfe4). Verified before rebase: 46 tests plain, sanitized set passed
  (a11y msaa gallery sanitized takes ~730 s; run alone with ctest
  --timeout 1800), style, lifetime, census, gate.
- Open: ToolStrip face rule has no Gallery ToolStrip specimen to verify;
  MenuStrip label/foot line/focus ring still direct (engine label and
  focus geometry differ, would change windows-professional pixels); dotted
  focus ring primitive; tab stack vs Plan Paint pixel comparison.

- Phase 4 batch 2 (2026-11-02), `claude/t4-themes`: 18e7bbcd tab stack
  matched to Plan Paint (probe dev_t4/scratch/tab_compare.cpp via
  dev_t4/tools/probe.sh: largest channel difference 112 at the two right
  corners only, <=16 elsewhere; gradient ends fading to `transparent` grey
  the engine material by up to 37 while paint_surface_material does not:
  suspected premultiplication defect, unreported in code); 27c285b3 dotted
  focus ring (ControlVisualRecipe::dotted_focus, `outline-style: dotted`,
  gui-forms floor dotted). Plain verified 15 tests; sanitizer and checkers
  for 27c285b3 NOT yet run.
- Checkpoint 11 fix: `claude/t4-cp11-anatomies` f208eeaa (from
  origin/checkpoint/2026-11-02-2): stock_anatomies.inc regenerated.
- Hot cues finding: specimen "hot" = pointer to the centre; most first
  instances are Gallery page_style() overrides (ButtonBase paints relief
  with no hover) or the centre hits a child; ComboBox tracks no hover and
  its anatomy has no hot state. Needs a coordinator decision (options in
  the report).

- Batch 3 (2026-11-02), `claude/t4-themes` after 27c285b3 (verified
  plain + sanitized + checkers + gate): 9cd96d42 material primitive
  interpolates gradient stops premultiplied (test fails on old kernel;
  gloss_2 and button_pressed_1 hashes re-recorded, rounding only);
  e1446b65 base Control hover tracking (Control::pointer_over_,
  Window::set_pointer_over/forget_hover; hot in base_anatomy_states;
  repaint only when the anatomy declares hot; window leave clears hover;
  test_hovered_control_is_hot in invalidation_damage_tests); 7b53e291
  specimen sheet hovers a point hitting the control itself (hot now
  differs from rest in 1 of 64 cells: the real gap).
- Owner/coordinator decisions: hot states options 1 + 3 approved
  (2026-11-02); T4 owns the theme engine now (T3 worker gone).

- Re-recorded goldens of 9cd96d42, old kernel (27c285b3) against new,
  raw premultiplied bytes: button_pressed_1 73 of 3549 pixels differ, max
  channel 1; gloss_2 146 of 14196, max channel 1; the other 22 reference
  rasters identical. Rounding only (coordinator asked, 2026-11-02).
- Known difference (coordinator: fine as is): the canvas brush path
  (src/render/skia/executor/skia_executor.cpp, SkGradient::Interpolation{}
  default) interpolates gradients in straight alpha; the material
  primitive and SkiaRaster painter interpolate premultiplied.

- 2342419e (claude/t4-themes): ButtonBase option 1. Overridden buttons
  paint face and cues through paint_part, the override's face/text as
  authored colours; paints_override_frame() keeps the relief only with
  per-control recipes or a connection. All 31 Gallery digests re-recorded
  (button frames only; six disabled labels to theme disabled ink); notes
  above the digest table. Verified plain, sanitized, checkers, gate, docs.
- 840f94b3 (claude/t4-control-colours, from main e22ea9c9; coordinator
  item, owner-approved): Control::set_back_color/set_fore_color (+ clear,
  ambient_back_color/ambient_fore_color). T4-1 rules; own background fills
  bounds before content (Window paint step) and grounds; ambient
  inheritance as WinForms; an inherited background is not repainted and
  skips wells (editors, lists, check boxes keep the theme's well). Recorded
  divergence: a check box's own background fills its box. Reach: paint_part
  only; direct painters read the ambient lookups. Tests in
  part_painting_tests and basic_controls_tests. Verified; reported ready
  for the C ABI worker.

- 2026-11-02 evening, after checkpoint 11 (main 17f94617): both branches
  rebased (`git rebase --onto origin/main 3cfae07c claude/t4-themes`;
  checkpoint 11 folded phases 2-3 + 3cfae07c as 560f3767).
  claude/t4-control-colours = a74ac740 (re-verified on main: plain,
  sanitized, checkers, gate, docs, anatomies). claude/t4-themes:
  fd360ddf..d2294eb4 rebased batch-2 commits, then 4fd6762a check box and
  radio button wells/marks/focus through paint_part (override = authored
  ground + ink; LinkLabel disabled keeps theme ink; census 378 -> 356),
  883963e5 ComboBox through paint_part + declares hot (schema rewritten;
  anatomy test hot on hover; census 356 -> 349). Each batch: Gallery trace
  diffs listed above the digest table; verified plain + sanitized,
  style, lifetime, census, gate, docs, stock anatomies.
- CheckBox BackColor divergence goes to W's manifests/winforms-divergences.json
  (coordinator told W).

### RESUME POINTER (2026-11-08 cp21; supersedes everything below)
- 2026-11-08 latest+10: ENGINE GAPS CLOSED. claude/t4-inspection 19030b44 on 52c7637f: keyline rings, pattern() + procedural checker, part text colours (Theme::resolve with part), named spin parts (NumericUpDown/DomainUpDown via paint_spin_parts in on_paint_overlay), ThemeGeometryTokens::scroll_arrows (macos off); specimen write_page settles idle patterns. Registered. Images w1/dev/inspection/*-before-after.png (r0 vs r12), gaps/*-gaps-2x.png (r10 vs r12). Notes claude/t4-gate (gaps table). dev_t4/v_all.sh + checks.sh = verify set. Dark grounds belong to w6. NEXT: coordinator findings; owner approvals.
- 2026-11-08 latest+9: FINAL ROUND DONE (coordinator: one head, no per-theme heads). claude/t4-inspection 52c7637f on 2d8b20e7: compiled parts sheets for classic/xp/windows7/macos (fonts 11/11/12/13 px body face stand-ins; see notes claude/t4-gate 6e59d88 "Final visual round"). Registered. Loop tools: gui_forms/dev_t4/inspect.sh <round>, dev_t4/t4_round.sh (verify), dev/tools/pull.sh <round>, dev/tools/before_after.py r0 <round>. Images w1/dev/inspection/*-before-after.png (r0 vs r10). Known gaps: xp group box colours, spin buttons unkeyed, classic default drops inner line, style().paper dark defect (reported). NEXT: coordinator findings; owner approvals.
- 2026-11-08 latest+8: coordinator chose option 2. claude/t4-inspection 2d8b20e7 on W gallery-stack 8d4a3d75: classic = Windows 2000 (registered). Loop: dev_t4/inspect.sh <round> [sheets] (M4: builds, renders controls-inspection per theme + sheets into build/inspect/<round>; scp to w1/dev/inspection/). verify.sh "<regex>" plain+sanitize. Images: w1/dev/inspection/classic-{before,after,before-after}.png. Notes claude/t4-gate 23aec02 (classic table). NEXT: xp-style (Luna), then windows7-style, then macos-style (AppKit reference: w1/dev/tools/appkit_reference.mm, build on M4); register a head per theme and tell the coordinator. Open: Gallery page surfaces paint the Gallery's own ground (W's shell), check/radio labels ignore theme font-family.
- 2026-11-08 latest+7: branch claude/t4-inspection = w2 gallery-stack 5b6f0235 + merge of claude/t4-batch9-fixes (clean), no page code yet. BLOCKER asked of coordinator: no API pins hover/pressed/focus on a control (pointer_over_ is window-set, ButtonBase hovered_/pressed_ private, TextBox/ComboBox final), so a static grid of real controls in hover/pressed needs a decision: (1) core inspection pin, (2) page shows static states + offline harness injects real input per cell window for hover/pressed [recommended], (3) subclass non-final types only.
- 2026-11-08 latest+6: batch9 fixes DONE: claude/t4-batch9-fixes 5b99547a on integration/batch9 4d020751 (six tests green plain+sanitized; W201 option 2 via ThemeCompileInput::colour_authority; notices take card frames; macos dark pressed accent #0a64e0; seam test reads part-raster box). Registered. NEXT: controls-inspection Gallery page on W's claude/gallery-stack 5b6f0235 (fetched as refs/remotes/w2/gallery-stack; trial merge conflicts only in docs/reference/inventory.json), then the M4 visual loop toward authentic Win2000/XP/7/macOS, images in w1/dev/inspection/.
- 2026-11-08 latest+5 (cp22 plan): d03bc2ed T4-P provenance per field (RuleProvenance on ThemeRule/CompiledRule, place_rules in compile_closure, CompiledThemeRules::resolve_provenance, Theme::provenance, test_provenance). T4-W/T4-4 already on main (5fc40cbd, 8bf1ba09; census 0 stock). T4-6 prepared not switched: dev_t4/t6_refset.sh + t6_index.py + t6_switch.patch (copies in w1 dev/tools); owner approval pending. Queued unverified. NEXT: integrator findings; run t6_refset.sh when allowed and show the owner index.html; switch only on approval.
- 2026-11-08 latest+4: 35c2f531 macos-style (Puppertino 2.0), light + dark via new ThemeDarkVariant::authored; notes claude/t4-gate 4 commits (latest: Puppertino table). Queued unverified. NEXT: integrator findings, gate numbers (t4_gate.sh covers macos-style), owner approvals.
- 2026-11-08 latest+3: 35c202a5 owner corrections: contrast verdict exempt for disabled text (all themes, specimen, dark test); exceptions removed; gui-forms disabled ink #627997 restored; classic/xp/w7 rewritten from 98.css/XP.css/7.css (per-control tables in notes claude/t4-gate 4df2e5c). Queued unverified. NEXT: fifth familiar theme macos-style from Puppertino (coordinator, queued after this pass), then integrator findings, gate numbers, approvals.
- Item 2 DONE: claude/t4-one-rasteriser 073fdb3c "One rasteriser" on claude/t4-theme-base-api 4556709c, both rebased onto origin/main d722217c. Pre-rebase verify: wide regex plain 195/195, sanitized 188/188, goldens round (theme|capture|golden|digest|control|material|render|paint|visual) plain 70/70, style/lifetime/census/gate clean. No golden or digest moved. canvas_raster fix: painted rectangles AND plane damage grow to device pixels (device_aligned in window_presentation.cpp); backplane baked once per size/scale (SurfaceRoot::paint_backplane_material); the test now calls window.set_scale(scale) like a host (a painter at a scale the window was not told is a host error). Exceptions: lone solid square fill stays fill_rect; over-budget bake falls back to layers.
- Owner rule (2026-11-08): never run clang on the laptop (Neo), not even -fsyntax-only; every compile goes through m4ws/m4run.
- Selector step 1 DONE: claude/t4-selectors-1 9b36f39e on claude/t4-one-rasteriser 0c6ff9a2 (amended: its API docs had been stale; the mirror's git status hides changes since rsync keeps mtimes, so api_verify now refreshes the index first and docs are compared after scp back). Specificity = CSS 3 columns; every language rule at theme_sheet; base_sheet/important_base_sheet levels via ThemeCompileInput::base_origin_end. Verified: 90 plain / 88 sanitized (theme|core_tests|abi|schema|gallery|specimen|capture|golden|digest|layered|semantic|cascade|font), style, lifetime, census, gate, docs.
- OWNER RULE "Code first, verify once" (BRIEF, 2026-11-08): write selector steps 2, 3, 4 and the CSS theme item (PLAN_CP20 line 45: the gui-forms theme compiled in from a CSS sheet, not C++ values) together, no verification between steps; at the end ONE plain M4 run of theme tests + goldens (no sanitizer, no wide regex), one queue entry, a report of five lines or fewer.
- Selector step 2 WRITTEN (uncommitted, on claude/t4-selectors-2 from 9b36f39e, not yet built cleanly): RuleExclusion + RuleSelector::family/excluded (theme_rules.hpp), PartFacts::family filled in Theme::resolve(control, role) and ThemeEngine::plan (part.family); exclusion_matches; rule_specificity counts family and the max exclusion; language: GroupKind {is, where, negation}, open_group/close_group, read_family (names from control_visual_role_name), add_exclusion, refused list without not/where; font props in :family rules warn; motion refines compares family/excluded; tests: test_step_two (selector), families_and_exclusions (cascade), conformance fixture selectors-functional.css (+COVERAGE rows; its .expected must be generated by GUI_FORMS_THEME_CONFORMANCE_WRITE=1 on the M4 and fetched by scp), E303 fixture moved to :has. One earlier run: only the E303 fixture failed (fixed since).
- Owner chose option 1 (2026-11-08): new theme-language properties muted-color, glyph-color (recipe colours, text position), default-outline: <width> <colour>, keylines: <top> <right> <bottom> <left> (colours, one unit inside the border = MaterialKeyline); port every family of gui_forms_theme.cpp exactly as :family blocks; shrink the C++ floor to structure only.
- Step 4 WRITTEN (uncommitted, unbuilt): engine (RuleAncestor etc., ancestors_match DP, AncestorBloom), ancestor_facts.hpp attach at all control facts sites, Control::set_classes -> restyle_descendants when reads_ancestors(), attach_subtree refreshes fonts; language push_ancestor (top level only, '>' child, whitespace descendant, ancestor compounds type/#id/.class/*), specificity sums compounds; tests test_combinators (selector), ancestor_compounds (cascade), fixture rows selector.descendant/.child, E302 fixture now '+'; docs.
- 2026-11-08 latest+2: b32769d4 owner's familiar-theme review: period values classic disabled #808080, xp disabled #aca899, w7 default frame #3399ff restored as owner-accepted exceptions (familiar_themes_tests accepted_exceptions); visual references 7.css (w7) and XP.css (xp, classic/98), never vendored. Notes claude/t4-gate has 2 commits. Queued unverified.
- 2026-11-08 latest+1: fcf81b15 notice/badge :family blocks (needs W-4 vocabulary b52e6d75), 36de6177 T4 phase 7 (FamiliarTheme classic/modern-flat/xp-style/windows7-style floor sheets, light only; detail::evaluate_floor_sheet shared; specimen --theme <id>; familiar_themes_tests). Phase 8 draft record: notes branch claude/t4-gate (w1 notes clone, results/2026-11-08-t4-gate/README.md), measurements pending dev_t4/t4_gate.sh after the integrator's verify_once.sh. All unverified, queued. NEXT: fix the integrator's findings, fill the gate record from its run, owner approvals; then item (D).
- 2026-11-08 latest: coordinator stopped the verify (owner: nothing waits on builds). Committed 685e79f1 (floor sheet) on 6fac1984 (steps 2+4), branch claude/t4-selectors-2, registered queued (unverified) in INTEGRATION_QUEUE; the integrator builds. NEXT: fix whatever the integrator's build reports; then item (D). W-4 fold needs notice/badge :family blocks in floor.css.
- 2026-11-08 later: steps 2 and 4 COMMITTED unverified as 6fac1984 on claude/t4-selectors-2 (a checkpoint commit, amended after the verify). Step 3 + CSS item WRITTEN (uncommitted on the same branch): RecipeField muted_text, glyph, default_outline_width, default_outline_colour (enum end, names, colour positions, validity, lengths, apply_recipe_fields, forced nulling); lowering properties keylines (material, drawn top/left/right/bottom), muted-color, glyph-color, default-outline; floor sheet `gui-forms/floor.css` inline in gui_forms_theme.cpp (gui_forms_floor_sheet()); detail::fill_family_recipes (src/core/theme/theme/floor_sheet.hpp, impl theme.cpp) evaluates it per family x state x selected onto recipe_structure() (dotted 1-unit ring, empty material); floor evaluated once (static), GuiFormsThemeBuild::floor_compile_operations; reference test floor_matches_reference in tests/gui_forms_theme_tests.cpp (old builders verbatim, canonical form: alpha-0 solids and downward gradients as endpoints or 180deg compare equal); fixture recipe-colours.css + 4 COVERAGE rows; docs table. notice/badge families do not exist on this base (no ControlVisualRole), so W-4 fold is not applicable here. Single verify: dev_t4/verify_once.sh via m4run (sel1.sh + p6_docs.sh); then scp back conformance .expected (incl. selectors-functional, recipe-colours), manifests/control-schema.json, docs/.
- Step 3 + CSS item DESIGN (decided by me, within option 1): the floor's values move to a CSS sheet of :family blocks (floor.css: `:family(button) {...}`, `:family(button):hover`, `:active` pressed, `:state(pending)`, `:invalid`, `:disabled`, `:state(deactivated)`, `:state(selected)`; states per visual_context_states in theme.cpp). At theme build, gui_forms_theme evaluates that sheet into the ThemeDefinition's ControlRoleRecipes (for each family x surface state x selected: resolve recipe+material fields with facts.family/states and apply onto a structure-only recipe), so dark derivation (derive_dark_recipes), forced high contrast and ResolvedControlRecipe's floor/authored split stay exactly as today; the runtime sheet stays light.css (parts). The C++ floor keeps structure only (windows-professional base, radii? no: radii in CSS too). Proof test: the old C++ family builders move into a test as the reference set; every family/state/selected recipe must be equal (ControlVisualRecipe ==). New properties: recipe fields muted_text (muted-color), glyph (glyph-color), default_ring + default_width (default-outline: <width> <colour>); material field keylines (keylines: <top> <right> <bottom> <left>, colours; 1 unit wide, 1 unit inside the border = MaterialKeyline{edge, colour, 1, 1}).

- Base: origin/checkpoint/2026-11-07-1 (71337875). Order (coordinator 2026-11-08):
  (A) theme-base API on claude/t4-theme-base-api: (i) rename ThemeCompileBase -> ThemeBase;
  (ii) compile_theme_packages(span<const ThemePackage>, library, images, options) = base, then
  packages in order, one cascade, one result (plus ApplicationResources::compile_theme_packages
  so hints apply); (iii) default_compiled_theme() -> shared_ptr<const Theme>, function-local
  static, never recompiled (process default when no application; stays windows_professional until
  the owner switches default_theme()). W owns ApplicationConfig{ApplicationThemeConfig theme{
  ThemeBase base{gui_forms}; vector<ThemePackage> packages}; ...} and Application::create.
  (B) item 2 on claude/t4-one-rasteriser: paint_surface_material through the material primitive
  (one rasteriser), golden moves explained. (C) item 1: owner ruled option 1 = add
  `:family(<role>)` pseudo-class (any control/part of that ControlVisualRole family, combinable
  with state pseudo-classes); gui-forms floor written as family rule blocks in the compiled-in
  base sheet, C++ keeps only a structural fallback; grammar/docs, unknown-family diagnostic,
  explicit specificity (state where :family sits vs type selectors), ABI/schema impact, pixel
  identity family by family vs the reference set. (D) rebase (b) onto the latest integrator batch
  head (w3 integration/batch*), redo colour moves in .form files, keep owner colours.
- 2026-11-08 owner ruling on the selector audit (desks/coordinator/THEME_SELECTORS_AUDIT.md):
  OPTION 2. After item 2, each its own commit: (1) CSS specificity (ids; classes+pseudo-classes;
  types+pseudo-elements) and the base sheet as a lower user-agent-like origin (re-record moved
  goldens, explain each); (2) :family, :not, :where; (3) gui-forms floor as family blocks, proven
  family by family; (4) descendant + child combinators (ancestor Bloom filter; subtree
  re-resolution on ancestor key change or reparent). Sibling combinators, :nth-*, :has stay
  refused; attribute selectors out by design.
- Item 2 design (drafting in worktree w1/rast on claude/t4-one-rasteriser): Painter::
  draw_surface_material virtual (base = detail::paint_material_layers, the old vector body);
  paint_surface_material checks + finish then dispatches. RecordingPainter(text_metrics,
  DeviceScale) bakes via detail::bake_material_raster (src/core/engine/primitives/material/
  material_raster.*; slicing via detail::assemble_slices moved to part_table/slice_assembly.*)
  and records draw_part_raster at bounds - outset (outset = max(8, visual reach)); reuse of the
  previous chunk's k-th material raster (DisplayCommand::surface_material + second = box size).
  Layers kept for image/procedural fills and a lone solid square fill; stripped of corner_radii,
  filters, mask, reflection (paint_surface_material never drew them). TODO: SkiaRaster override
  (direct painting), recorder construction sites (window.cpp control chunks, snapshot,
  presentation) pass the scale, goldens/digests, verify.

### RESUME POINTER (2026-11-07 cp20, rebase onto cp19; supersedes everything below)

- Task (cp20): base on cp19 = integration checkpoint/2026-11-05-4 f58b26f6 (fetched as
  refs/remotes/integration/cp19 in w1/gui_forms), verify, docs, refset, light compare, dark
  muted text, final (b), report light drift as options. No default_theme() switch.
- cp19 already had a Gallery project theme (demo/themes/gallery via gui_forms_add_theme,
  compiled onto professional; .form colours/fonts refused E813 and moved to classes:
  heading/muted/positive/ink/accent-text/paper + type classes). So (b) was REDONE:
  old branch kept as claude/t4-gallery-colours-pre-cp19 (920ba5fa).
- New claude/t4-gallery-colours on cp19: ec784c1e (variants stage B), aa7660e6 (a) API;
  (b) uncommitted = old step 1 (21641022: specimen --pages, professional_theme.hpp) +
  sheet work: select_gallery_theme sets options.base = gui_forms; my separate
  demo/gallery_theme.* embedding dropped; grounds merged into
  demo/themes/gallery/gallery.theme.css as classes with `background:` shorthand
  (ID selectors must name form objects, so C++ controls got classes: gallery-sidebar,
  gallery-status, gallery-specimen(-light), gallery-tint-mist...); label helper maps
  muted/shell_blue_dark/accent/green to cp19 classes muted/heading/accent-text/positive;
  Gallery :root palette keeps only success-on-paper (base defines the others; overriding
  them would recolour stock controls). Engine fix darken_recipe_fields kept. Scripts:
  dev/tools/take_head_hunks.py, p6c_edit.py. Digests not yet re-recorded.
- 2026-11-07: coordinator asked to rebase onto origin/checkpoint/2026-11-05-4 = 2bce69ad
  (f58b26f6 + one Windows test commit): branch now 40ba3bc1 (variants B), 2863db55 (a).
  (a) VERIFIED (dev/logs/a-verify-*: 3/3 plain + sanitized, style, lifetime, census, gate,
  api_reference, library docs, schema) and reported to main via SendMessage.
- (b) uncommitted, adds: SplitterPanel draws a rule-selected face (splitter_panel.{hpp,cpp},
  test_splitter_panel_rule_face in tests/theme_rule_faces_tests.cpp); .gallery-rule border:none.
  c1: wide 15 tests ok except trace; c1v verify passed (plain 15/15, sanitize 13/13 with the
  --skip-labels exclusion now in t4_verify.sh). Refset c1 counts light 16 / dark 368 / HC 312.
  Light vs phase 6 (dev/p6/light_compare_c1.tsv, names mapped via dev/p6/refset-p6-names):
  10.6-49% changed; constant 8.7% = sidebar #e2e8ee plates -> #dce5ed; containers' panes
  (fixed in c2 by SplitterPanel); 1px rules #a6b3bf -> #cdd1d6 (border:none in c2, check).
  c2 digests recorded; c2v verify running. Tool: dev/tools/pair_census.py OLD NEW.
- 2026-11-07 later: (b) committed 63ca2cd4; colour audit answered. Owner: restore only the
  muted role (#5b6b79) -> sheet edited (uncommitted), round + verify pending; then the sunken-face
  corner task (probe dev_t4/probe/well_probe.cpp, temp CMake target: restore CMakeLists from
  dev/CMakeLists.probe.bak before any commit); then cp21 lead (below).
- 768ccf93 = muted fix (committed, verified). Owner: (b) moves to cp21; rebase onto main after
  cp20 merges, together with the CSS authoring. Parked rebase onto cp20 trial 955341b8:
  claude/t4-gallery-colours-cp20-wip bda841f7 (page code transforms done incl. empty-loop removal;
  sheet taken from trial = must reapply my header/:root(muted+success)/ground classes and merge
  trial's specimen-* classes; digests + trace note to redo). w8 kept the responsive scroller's
  set_background on purpose (4f61485d) because of the raster corner difference.
- Sunken-face task DONE (analysis; no engine change, options to coordinator). Probe
  (dev/tools/well_probe.cpp + well_probe_run.sh; temp CMake target from dev/CMakeLists.probe.txt;
  NOTE the first three probe runs were invalid: raster.resize was missing): sunken Panel, authored
  background (direct Skia stroke, theme.cpp paint_surface_material: rrect inset w/2, radius r-w/2)
  vs rule face (material primitive raster). Integral bounds, professional base, scale 1: 16 px
  differ, all FOUR corners symmetric, max 11 (not bottom-only in isolation). Geometry matches
  (outer radius r, inner r-w); the coverage estimators differ: raster = linear SDF (0.5-d),
  Skia = its stroke AA. Exact area (r=2,w=1): corner .315 / edge-adjacent .913 / diagonal .215;
  raster .375/.917/.208, Skia .333/.842/.242 -> raster closer (abs error per corner .075 vs .187).
  Fractional y or height: raster snaps box edges to pixels, direct antialiases: up to 90-107 levels.
  gui-forms base: 338 px at scale 1 (inset shadow also differs).
- HOLD (owner via coordinator, 2026-11-07): after the sunken-face task, do NOT start the cp21
  CSS authoring or the (b) rebase until the application-first configuration study's theme part is
  ruled on (semantic-tests worker writes it). Report and stop when the sunken-face task is done.
- NOW: sunken-face corner task (professional base: authored stroke vs rule raster differ at
  bottom corners). Probe: dev_t4/probe/well_probe.cpp (PROBE_GUI_FORMS=1 selects gui-forms base;
  on gui-forms both paths were identical).
- cp21 LEAD (owner 2026-11-07; start only after cp20 is pushed): author gui-forms as CSS. Plan:
  (1) inventory what gui_forms_theme.cpp holds beyond the light sheet raw string (floor recipes
  in C++: plate(), fill_well_family, add_well_shade, per-family tables, dark derivation params);
  (2) decide which floor values the theme language can express today (materials, borders,
  shadows, radii, per-state faces via :hover/:pressed/:disabled, selected) and list the gaps;
  (3) move the light sheet to src/core/theme/gui_forms_theme/gui_forms.theme.css embedded with
  sha256 via the same embed path as gui_forms_add_theme (core target, no form check);
  (4) move floor values family by family into CSS rules, keeping a minimal C++ floor only for
  what the language cannot express; (5) after each family, prove pixel identity: specimen
  reference set (sheets + pages, light/dark/HC, scales 1 and 2) byte-identical vs the pre-move
  set, display traces unchanged, contrast counts unchanged; (6) document the layering
  (library sheet < program sheet, compiled in the same way) in docs/user/theme-language.md.
- NEXT: c2v result; fetch refset to dev/p6/refset-c2; png_compare; pair_census on values and
  containers; p6_index.py; commit (b) (final message); report with options.

### RESUME POINTER (2026-11-05 Mac, owner pause; older)

- claude/t4-gallery-colours = 920ba5fa on claude/t4-phase6 21641022:
  ace44471 (a) PUBLIC API ADDITION ThemeCompileBase + ThemeCompileOptions::base,
  test each way, docs/user, docs/api rst, inventory (ONLY theme_compiler.hpp entries
  spliced in: the full regeneration also carried unrelated pre-existing drift --
  Control, LiveSurface*, Presentation*, Metrics -- left out on purpose).
  920ba5fa WIP (b): Gallery sheet, digests (b3), darken_recipe_fields with literal
  text -> ink / light_ink (OKLab L >= 0.7), literal focus -> cue (built in b4).
- Round b4 (with the ink fix): traces identical to b3 (31 pages, 4578 lines vs
  phase 6); wide plain tests 15/15 pass.
- b3 results (before ink fix): verify plain/sanitize 14/15 each, only the
  accessibility client (external drive, per coordinator: launched-app tests run
  plain only now; sanitize trees cannot launch apps). Refset counts light 18 /
  dark 398 / HC 312 (phase 6: 18 / 959 / 568). Light vs phase 6
  (dev/p6/light_compare.tsv): changed share 9.9-50.6% per page, max delta mostly
  39 (137 on typography-and-inspection), mean 0.26-3.97.
- Light regressions vs phase 6 (to report with options): GroupBox paints its ground
  only with a background/style override, so group grounds lose #ffffff (largest
  share); navigation plates gone; 1px rule panels lighter (#a6b3bf -> #cdd1d6).
- docs: api_reference.py --build FAILS on main too: "Stale contract:
  gui_forms::Control::layout_transaction_state" (contract says noexcept, header
  does not); not mine -- report. library docs and schema checks PASS.
- p6bv4 verify DONE (dev/logs/p6bv4-1105-191101.log): plain 15/15; sanitize 14/15
  (only accessibility client: app launch blocked on sanitize trees, environmental);
  style, lifetime, census PASS; gate 2 F1 = pre-existing macos_message_dialog_key_tests.mm.
  Refset written to gui_forms/build/p6/refset on the M4 (not yet fetched).
- NEXT (exact): rsync build/p6/refset to dev/p6/refset; png_compare.py
  refset-p6 vs refset 'page-light-*.png' > dev/p6/light_compare.tsv; p6_index.py;
  look at dark css-layout muted text; amend 920ba5fa to a final (b) message (drop
  WIP); report head, counts, light measurement, light regressions with options,
  stale contract. default_theme() switch only after owner approval.

### RESUME POINTER (2026-11-05 Mac, app restart stop; older)

- claude/t4-gallery-colours = 2b206cee WIP (all of (a) + (b), one commit) on
  claude/t4-phase6 21641022. Includes since round b2: digests from build/p6/b2
  (still need re-recording: b3 changed the backplane), demo/gallery.form
  gallery.backplane is now `panel` (MaterialPanel ignored the sheet rule and drew
  #ffffff), darken_recipe_fields engine fix (built in b2: only the trace test failed).
- Round b2: 31 pages differ from phase 6 in 4578 lines; 14/15 tests passed (trace only).
  Round b3 was abandoned mid-queue (app restart).
- NEXT (exact): sh dev_t4/p6b.sh b3 via m4run (copy dev/tools/*.sh into
  gui_forms/dev_t4/ first); rsync build/p6/b3 to dev/p6; python3 dev/tools/digests.py
  dev/p6/b3/digests.txt gui_forms/tests/gallery_pages/display_trace_tests.cpp and fix
  the line counts in the trace note; dev_t4/p6b_verify.sh (plain + sanitized,
  checkers, gate, refset); rsync refset to dev/p6/refset; python3
  dev/tools/png_compare.py dev/p6/refset-p6/pages dev/p6/refset/pages
  'page-light-*.png' (coordinator wants max/mean delta and changed-pixel share per
  light page); regenerate index.html with per-scheme counts; regenerate docs/api
  (api_reference.py --build) and run docs/schema checks; split the WIP into (a) API
  [theme_compiler.hpp/.cpp, theme_package_compile_tests.cpp, docs] and (b) the rest;
  report head.

### RESUME POINTER (2026-11-05 Mac, gallery colours; older)

- Coordinator chose option 1: branch claude/t4-gallery-colours (on claude/t4-phase6
  21641022), UNCOMMITTED work in w1/gui_forms, to land as two commits:
  (a) PUBLIC API ADDITION: ThemeCompileBase {windows_professional, gui_forms} +
      ThemeCompileOptions::base (theme_compiler.hpp/.cpp: the gui-forms light sheet
      as a first VerifiedThemePack in `order`, floor gui_forms_theme_definition());
      test_base_theme in tests/theme_language/theme_package_compile_tests.cpp; docs
      paragraph in docs/user/theme-language.md. docs/api + inventory still to
      regenerate (api_reference.py --build ../.build/native-macos-arm64/gui-forms),
      generate_library_docs --check, control_schema check.
  (b) demo/gallery_theme.{css,hpp,cpp} + demo/gallery_theme_sheet.hpp.in (CMake
      file(READ)/file(SHA256)/configure_file into gui_forms_forms/); make_gallery sets
      gallery_theme(); TV-2 --no-dark moved the 41 .form colours; C++ grounds and
      label() colours -> classes/IDs in the sheet (dev/tools/p6b_edit.py did the bulk);
      page_style() removed except DiagnosticsCard; navigation selection is the class
      gallery-navigation-selected; backplane wash -> #gallery\.backplane rule;
      tools/relocate_colours.py marker says "no dark rules (--no-dark)" under --no-dark;
      test_gallery_theme_sheet (admits exactly 51 W201 = text colours, HC replaces by
      position). Engine fix in src/core/theme/theme/theme.cpp: darken_recipe_fields
      (rule-set text/outline/shadow colours darken by dark_parameter_colour under a
      derived dark variant; before, rule text colours stayed light-terms in dark).
- Why --no-dark (not dark seeds): the gui-forms base derives dark by rule and darkens
  every rule colour at paint, so seeded dark values would be darkened twice.
- Measured round b1 (before the engine fix and backplane): traces vs phase 6 differ in
  4576 lines (876 ink #1c2732->#1b2733, 762 nav plates now transparent, panels via
  draw_part_raster); refset findings light 18 / dark 734 / HC 578; dark still showed
  dark-on-dark headings (fixed since by darken_recipe_fields, UNBUILT) and the basics
  backplane light (moved, UNBUILT).
- BLOCKER (2026-11-05): M4 disk full (240 MB free of 228 GB; workers' .build trees
  82 GB). The sanitizer build failed at ranlib with ENOSPC. Reported to the coordinator.
- Next: round b2 (sh dev_t4/p6b.sh b2; compare against new), digests.py from
  build/p6/b2/digests.txt, update the trace note numbers, verify plain + sanitized
  (dev_t4/p6b_verify.sh), refset (dev/p6/refset; phase-6 copy kept in dev/p6/refset-p6),
  regenerate index.html with counts, split into commits (a) and (b), report.
- Open notes: basics relief styles (gallery_basic_style, the BasicControlStyle demo)
  stay authored light faces in dark; DiagnosticsCard keeps its own style; value-editors
  preview shows a colour value (content). Gate finding outside my change:
  tests/macos_message_dialog_key_tests.mm:225,236 F1 (pre-existing on main).

### RESUME POINTER (2026-11-05 Mac, older)

- Workspace now ~/Developer/gui_forms_workers/w1/gui_forms (Mac; builds on the M4 via
  bin/m4run w1). Branches restored from backup/<branch>: claude/t4-phase6, t4-relocate,
  t4-backdrop, t4-lowering (local, same heads as the backups except phase6).
- PHASE 6 STEP 1 DONE: claude/t4-phase6 = 21641022 (one commit, WIP squashed) on
  claude/t4-relocate 8209a921 on origin/main ebb9fbf4. Gallery sets gui_forms_theme();
  test_support::professional_theme(); specimen --theme windows-professional and new
  --pages; 31 digests re-recorded (all pages differ, 15799 of 29716 lines, no text moved).
  Verified: 'gallery|specimen|doc_examples' 11/11 plain + sanitized, style, lifetime 0,
  census, gate. Nothing else needed pinning (Gallery-wide 17-test set passed).
- Reference set (local): ~/Developer/gui_forms_workers/w1/dev/p6/refset/index.html
  (sheets light/dark/HC x 1x/2x, 93 page PNGs). Pixel contrast findings: light 18, dark
  959, HC 568. Dark Gallery looks mostly light: the Gallery's authored light grounds
  (page_style/form colours) stay light while theme text turns light. Decision sent to the
  coordinator: relocate the Gallery's colours (TV-2 over the Gallery) before the owner
  reviews dark, or review light now.
- Tools (dev/tools, copied into gui_forms/dev_t4/ which is git-excluded because m4ws syncs
  only gui_forms/): p6_round.sh <name> (traces to build/p6/<name> on the M4, diff counts
  vs base), p6_new.sh, p6_classify.py (run in dev/p6 after rsync from
  m4mini:Developer/Projects/gui_forms_workers/w1/gui_forms/build/p6/), digests.py
  <digests.txt> <display_trace_tests.cpp>, t4_verify.sh 'REGEX' [nosan] (quote the regex
  as "'a|b'"), p6_refset.sh. Build tree on the M4: w1/.build/native-macos-arm64/gui-forms.
- Waiting: cp18 to land (then restack t4-backdrop 58319f26, t4-lowering eb9f80c9,
  t4-relocate, t4-phase6 onto it). Step 2 (default_theme switch) only after owner approval.
- Windows-only tests: unverified here (no Windows host); CI or a later Windows host.

### RESUME POINTER (2026-11-05, after the restart; older)

- REBOOT STOP (2026-11-05): clone D:/gui_forms_w1/gui_forms on claude/t4-phase6 =
  1cdcc5ed WIP (phase 6 step 1 edits, UNBUILT) on claude/t4-relocate 8209a921 (TV-2,
  checked: 7 unit tests, filter built, style, registration). NEXT on resume: base
  traces from 8209a921 (build it in a detached worktree, or check out its
  demo/gallery.cpp and tests/specimen/specimen_sheet.cpp, run p6_round.sh's base half,
  then restore HEAD; never stash), then the new traces with p6_round.sh (drop the
  `switch` argument: the edits are already committed), the difference count, fallout
  fixes (pin professional where a test measures its look), digests.py, squash the WIP,
  then the reference set (specimen_sheet --out <local dir> light/dark/HC + Gallery page
  PNGs, a local HTML index opened in Brave) and send the coordinator the location.
  claude/t4-backdrop 58319f26 and claude/t4-lowering eb9f80c9 wait for cp18 (restack
  after it lands).
- UNPARKED 2026-11-05 (M3 landed). Builds: shared slot only; NO sanitized builds.
- TV-2 DONE: claude/t4-relocate 8209a921 on origin/main ebb9fbf4 (cp17):
  tools/relocate_colours.py + tools/test_relocate_colours.py (7 tests, wired in
  native.yml) + tools/dark_colour.cpp (gui_forms_dark_colour filter) + docs. Dry run
  over demo/pages/*.form moves 41 colours (dev_t4/logs/tv2_diff.txt). C++ trees later.
- PHASE 6 (owner chose two steps, 2026-11-05): step 1 = tests relying on the
  professional look pin tests/support/professional_theme.hpp
  (test_support::professional_theme()); Gallery + examples switch to gui-forms
  (make_gallery set_theme(gui_forms_theme())); Gallery traces re-recorded with a
  difference count (dev_t4/tools/p6_round.sh, counts in dev_t4/logs/p6_counts.txt);
  owner reference set light/dark/HC as LOCAL PNGs + a LOCAL HTML page (opened in
  Brave, never published); send the coordinator its location. Step 2 after owner
  approval: default_theme() switches; fallout via the standby's fold verify; arm64
  goldens to the Mac handoff. Specimen sheet: default now the Gallery's own
  (gui-forms), `--theme windows-professional` for the old look.
- Follow-up idea: ThemeCompileOptions base floor (gui-forms + its light sheet) so a
  project theme (e.g. a Gallery sheet with the relocated colours) builds on gui-forms.
- claude/t4-backdrop = 58319f26 on origin/main edfad055 (VERIFIED plain: 80/80 over
  'theme_|conformance|display_chunk|panel|engine_|part_painting|text_box|selector|
  seam_proximity|split', style/lifetime 0, census ok 0 stock, gate rc 0, docs checks pass).
  The WIPs are squashed: a5aacec3 splitter 2px rule; 58319f26 popup = native-only wording
  + regenerated backdrop.expected, API pages, inventory. Pre-squash backup
  claude/t4-backdrop-pre-squash = 7c98bf7e. 498ee9e5..8bdce8be duplicate cp17's census,
  well and fonts (expected to drop on the next rebase); ad9fb505 carries the
  engine_recipe_field_tests font-field fix the standby may need for cp17.
- claude/t4-lowering = eb9f80c9 (VERIFIED plain: 49/49 theme_/conformance/backdrop/panel/display_chunk + compositor_lab, compositor_backdrop, engine_backdrop, windows_compositor; style/lifetime 0, census ok, gate rc 0, docs regenerated and checked) = claude/t4-backdrop + T5's 0968c952 7b0c5ce8 f5e7abb1 398b130c
  (cherry-picked; the coordinator folds them with claude/t5-backdrop) + the lowering
  commit (patch kept at D:/gui_forms_w1/dev_t4/lowering.patch): CompiledThemeRules::
  resolve_backdrop, Theme::window_backdrop_chain, SurfaceRoot lower_theme_backdrop_chain
  (constructor, show_theme before the implied request, set_presentation_settings,
  Window::set_scale; application_backdrop_chain_ makes the app's chain hold). `popup`
  rules compiled and held for the native popup seam (w4 claude/t5-native-popups, WIP).
  Tests: theme_cascade surface_rules, theme_variant test_window_backdrop.
- Next after unpark: lower `popup` rules once the native popup seam lands; then phases 6-8,
  then TV-2.

### RESUME POINTER (2026-11-04, before the restart; older)

- Clone D:/gui_forms_w1/gui_forms, branch claude/t4-backdrop, rebased onto origin/main
  edfad055 (c367a5ca dropped; main's 83db00a7 covers it and fixes invalidation_damage).
  Stack: 498ee9e5 census exemptions, eb1b45dc frameless well, b023ecba + 8bdce8be fonts,
  ad9fb505 backdrop parse side + declared clarity (was 1e547745, verified pre-rebase:
  77/77 plain+sanitized, checkers, gate, docs; recorded for cp18), 965eab23 WIP splitter
  (band.frame = none below 2 px), 7c98bf7e WIP popup wording. The rebased stack is NOT
  yet re-verified.
- Next: (1) regenerate tests/theme_language/conformance/backdrop.expected
  (GUI_FORMS_THEME_CONFORMANCE_WRITE=1; the fixture now says popup.alert), regenerate
  docs (generate_library_docs.py; api_reference.py --build
  D:/gui_forms_w1/.build/native-windows-x64/gui-forms after `. ../env.sh`), run
  verify.sh with T4_BASE=origin/main over
  'theme_|conformance|display_chunk|panel|engine_|part_painting|text_box|selector|seam_proximity|split',
  then squash the WIPs into two clean commits (splitter; popup wording) and report.
  (2) Lowering commit: base on claude/t5-backdrop f5e7abb1 (hot paths' rework, in
  D:/gui_forms_w4/gui_forms; NOT 7b0c5ce8), cherry-pick ad9fb505 under it. `window`
  rules -> Window::set_backdrop_chain (role tints resolved per scheme/HC). `popup`
  rules: parse and hold; lower only once hot paths' native popup seam lands.
  (3) phases 6-8; (4) TV-2 relocate_colours.py.
- Owner rulings in force: blur() = ask the compositor for its blurred backing (system
  material), never sample other windows; NO frost operation (grammar stays blur,
  saturate, tint). "Popup" = native owned alert-like window only (TaskDialog/FontDialog
  panels); menus/drop-downs/tooltips are OVERLAYS (open_popup -> open_overlay rename by
  the semantic-tests worker for cp18), our own pixels, ordinary control rules, no backdrop.

### RESUME POINTER (2026-11-03, older)

- LATEST (2026-11-04, after the reboot): round on 5ebbc402 rerun
  (dev_t4/logs/round.txt). Variants (i)-(vii) being written in a separate
  worktree `D:/gui_forms_w1/wt_variants`, branch `claude/t4-variants` from
  5ebbc402 (uncommitted while the round ran, since the round builds
  `gui_forms/`). Design as built: ColourSchemeSet/SystemColourHints/
  ThemeFallback in presentation_types.hpp; ThemeRuleSheet::colour_schemes
  (from `:root { color-scheme }`), ThemeDefinition::colour_schemes, Theme::
  supports; Window::theme_fallback/application_theme, scheme_theme +
  show_theme; ApplicationResources::set_automatic_colour_scheme,
  system_colour_hints, W403 once per (theme, scheme); Control::
  application_colours_suspended (paint_part, paint_back_color, authored
  surface), W404 inline_colour in set_back/fore_color; W402 at package
  compile (not the bare language compiler, so language tests stay clean);
  env(system-accent|system-highlight, fallback) in the value reader,
  hints at compile time; CompositorState::dark_scheme + system_colours,
  Windows host reads AppsUseLightTheme, DWM AccentColor, COLOR_HIGHLIGHT;
  4 divergences in manifests/winforms-divergences.json; docs
  theme-language.md "Colour schemes". New test tests/theme_variant_tests.cpp.
  NEXT: copy into gui_forms on claude/t4-variants, build, fix the dark-window
  tests that now fall back (add `color-scheme: light dark`), verify, commit.

- LATEST (2026-11-03, cp15 rebase): `claude/t4-themes` rebased onto
  origin/main feacf6d4 (checkpoint 14). Head 5ebbc402 (digests re-recorded,
  all 31 pages). Effect programs won in progress_bar.cpp and
  range_control_rendering.*; a1895a65 (legacy progress census exemption)
  dropped; census stock 4 files 15 calls. Pre-rebase backup
  `claude/t4-themes-pre-cp15` = f54a9ca9. Verification round:
  NOT YET VERIFIED after the rebase: the round was stopped for the
  2026-11-03 reboot. Only the Gallery display trace ran (sanitize tree, rc 0,
  digests re-recorded). NEXT: rerun
  `T4_BASE=origin/main sh dev_t4/tools/round.sh 'theme|high_contrast|colour_role|engine_backdrop|progress|range_control|part_painting|dark_variant|contrast|specimen|paint_census|effect'`
  (log dev_t4/logs/round.txt), then send the coordinator head 5ebbc402 (or
  the fixed head), then implement design (i)-(vii). Notes `claude/t4-plan` 6d1a8a4 waits for the
  coordinator to push.
- Owner APPROVED the D1/D2 design note: implement (i)-(vii) next (see the
  design note). Phase 7 capability audit is written (below the plan) and
  the gap list sent to the coordinator (glass recipe material, effect
  backdrop sampling, effect clip_part_shape).
- Checkpoint 12 (main 855b0f0e) folded both branches (cf2ad98c T4,
  7ae52c98 C5 with back/fore colours); `claude/t4-themes` was reset to
  origin/main (old head 883963e5 contained) and now carries 4635aef3:
  button content, LinkLabel, ComboBox text and DropDownButton's marks as
  parts; paint_button_frame/paint_button_text/paints_override_frame
  removed; button_base and combo_box files at zero stock calls; census
  331. All 31 digests re-recorded (labels in part clips at the same
  origins; 16 disabled button labels #76808a -> #848f99, the part path's
  disabled ink). Verified plain, sanitized, checkers, gate, docs.
- Checkpoint 12's three regenerated digests (states-and-diagnostics,
  values-and-spinners, typography-and-inspection): C5 reflected new
  Control properties (Appearance: BackColor, ForeColor, BorderStyle,
  Cursor...) into the PropertyGrid on values-and-spinners, adding rows;
  the other two pages print live control counts (1335 -> 1364) and
  generation counters. Intended; no visual change elsewhere.
- 27ac7a78 range controls through paint_part (ScrollBar, TrackBar,
  ProgressBar track/fill; paint_sunken/paint_thumb removed; census 303).
  Progress effects (stripes, luminance, laser in range_control_rendering,
  13 calls) still direct: decision asked (engine primitives with motion
  phase vs content exemption).
- f12d96da OWNER RULING 2026-11-02 (supersedes keeping the part path's
  muted ink): disabled labels/captions/content use the theme's own
  disabled text (recipe.text resolved in the disabled state). Correction:
  my 4635aef3 report swapped the hexes; main had #848f99 (theme disabled
  text), the part path's muted ink was #76808a; the ruling restores
  #848f99.
- OWNER RULING 2026-11-02 (progress effects): option 2 as the interim.
  The 13 stripes/pulse/laser calls are legacy, to be replaced; the census
  exempts range_control/range_control_rendering.cpp with that reason (the
  progress bar's only exemption). Replacement = registered effects incl.
  bounded stateful effect programs (a later stage); do NOT port them into
  the engine.
- 7373e44a lists batch 1 (exemption split into a1895a65, its own commit so the effect-programs fold can drop it): ListBox, CheckedListBox, TreeView, PropertyList
  via parts; new paint_text_part_at (text part placed by baseline);
  exemption above; census 303 -> 259; six digests re-recorded.
- bc98e45b lists batch 2 (census 259 -> 201): ObjectView,
  CorrespondenceView and the PLAIN BreadcrumbTrail via parts; object
  glyphs = framing well (authored ground) + content_color fills.
  Left direct, pending a vocabulary decision asked of the coordinator:
  muted/secondary text (ObjectView secondary text, CorrespondenceView
  secondary/tags/detail/"MATCHED EXCERPT", calendar day names and hint),
  search-match highlight (collection_control_utilities
  draw_emphasized_text), CorrespondenceView status rail and state word
  colours, and the RAISED breadcrumb's pointed segments (no pointed
  shape in the engine; hit testing uses the noses).
  PartPaint::measured_text added (a cache: paint_text_part_at and
  ObjectView's details cache pass their measurement so warm details
  paints stay measurement-free, collection_controls_tests).
- Foreground lock (coordinator, mandatory): dev_t4/verify.sh runs
  Gallery/UIA/window tests under projects/tools/foreground_lock.py after
  building outside the lock; trace_diff.sh runs the Gallery exe under it.
- REBOOT STOP 2026-11-02 (night). Branch claude/t4-themes, head
  d53d5785 = "WIP:" commit on top of bc98e45b (verified). Owner accepted
  all four vocabulary questions (text tone admitted; match highlights as
  selection parts; correspondence rail/word via accent indices 0 pinned,
  1 stale, unavailable for offline; pointed-face shape for the raised
  breadcrumb incl. hit testing). Notes branch claude/t4-plan 3ff1d6c
  records them in the design language (local commit, unpushed).
- WIP d53d5785 (UNVERIFIED: last sanitize-tree build looked clean, but no
  trace diff, tests, schema or docs yet): ContextMenu/MenuStrip parts;
  PartPaint::tone + part_value_names "text_tone"; secondary text in
  ObjectView, CorrespondenceView, menu shortcuts; paint_emphasized_text
  (selection-part highlights); TextBox/RichTextBox; Panel (sunken -> editor
  well, raised -> button face, line -> keylines), GroupBox, Card,
  SplitterPanel, TableLayoutPanelCompat, ScrollableControl axes,
  SplitterGrip. Census 96 stock calls in 13 files.
- NEXT on resume: sh dev_t4/tools/trace_diff.sh (no lock: display traces
  are windowless; it saves logs/wip.patch before stashing), review diffs,
  re-record digests with a note, regenerate control schema
  (GUI_FORMS_CONTROL_SCHEMA_WRITE=1; schema.md/inventory via their
  generators), docs check, verify.sh with regex covering menu|text_box|
  rich_text|panel|group_box|card|split|table_layout|scrollable|object_view|
  correspondence|collection_controls|part_painting|gallery_(pages|
  interaction|display) (gallery tests run under the foreground lock),
  then split the WIP into proper commits. Then calendar/date picker
  (tone), basic_control_rendering, visual_inspector, value editors,
  error_glyph, connected_controls, cut_off_text, label, text_block, the
  raised breadcrumb (pointed face), MaterialPanel (ask: exempt as authored
  material?). Then phases 5-8 and T5.
- RESUMED 2026-11-02 (after reboot): the WIP was split into four commits
  on bc98e45b (WIP kept as local branch claude/t4-wip d53d5785; src/include
  identical): 394a0ed6 text tone + match highlights + correspondence rail
  (schema/docs/inventory regenerated), 25dde846 menus, 43d87f84 text
  boxes, 138a3f3c containers (all 31 digests). Each commit re-recorded its
  own digests with a note (dev_t4/tools/step.sh + digests.py; docs.sh
  regenerates schema/docs). Base traces of bc98e45b in logs/trace_base.
  Verify via `T4_BASE=origin/main sh dev_t4/verify.sh '<regex>'`.
- OPEN (flag to coordinator): a Panel with an authored background and
  BorderStyle::none now shows the panel material's hairline border (T4-1:
  authored background keeps the material's border), as theme panels
  already did; the Gallery's layout panels gain hairlines. Alternative:
  BorderStyle::none suppresses the material border for all panels
  (needs a borderless-face vocabulary bit).
- Foreground tests now run on a private desktop, no lock:
  `python -B D:/malkuth/projects/tools/run_on_desktop.py -- sh
  dev_t4/tools/fg.sh` (build the sanitize tree OUTSIDE first: ccache
  needs USERPROFILE, which fg.sh exports). Only UIA/MSAA/compositor probe
  need foreground_lock.py, batched. Through ec8c6487: gallery
  interaction/pages/display-trace, policy, catalog, windows pixel/
  snapshot/composite/raster/damage, focus and floating tests 13/13 plain
  and sanitized. UIA/MSAA gallery tests: ran under the lock at 15c08d53, 2/2 plain and
  sanitized (dev_t4/tools/acc.sh); nothing pending.
- Committed since (2026-11-02/03, on 138a3f3c): 3bdc9eb5 date picker +
  calendar popup; 4ee670a4 paint tests read the theme's parts; bea98549
  focus cues through the engine (rings honour corner_radius); e8da0740
  Label/TextBlock lines as label parts, CutOffText reveal always parts;
  70b9bf20 ErrorGlyph, Color/Flags value editors, inspector overlay
  outlines; ec8c6487 paint_theme_cues kept only for override recipes;
  then the raised breadcrumb (pointed face, PartPaint::point,
  pointed_face_contains used by hit testing; part_value_names
  "pointed_face"; tree-and-object-views re-recorded). Census 15 calls in
  4 files: visual_inspector_view 9, connected_controls 3,
  basic_control_rendering 2 (override recipes), material_panel 1.
- OPEN (flag to coordinator): (b) VisualInspectorView's dark diagnostic
  panel: port or exempt as a diagnostic instrument; (c) connected
  controls' public paint_connected_surface_material and the override
  recipe cues cannot go through the engine (authored recipes): exempt?;
  (d) MaterialPanel: exempt as authored material?
- REBASE PENDING (coordinator 2026-11-03): claude/t4-themes goes into
  checkpoint 15, not 14. After checkpoint 14 merges (coordinator says
  when): rebase onto origin/main, let the C ABI worker's effect programs
  win in progress_bar.cpp and range_control_rendering.cpp/.hpp (they
  delete the legacy progress effects 27ac7a78 rewrote), drop the legacy
  progress-effects census exemption (a1895a65), re-verify.
- Phase 5 in progress: bec6c306 = 5a, the dark colour rule. 5b/5c
  uncommitted: ControlVisualContext::dark, ControlRoleRecipes::dark and
  dark_selected, ThemeDefinition::dark_variant (none or derived),
  derive_dark_recipes in dark_variant.cpp, Theme::resolve, and
  Control::visual_context plus the backplane contexts reading the
  colour scheme. gui-forms sets derived.
- OWNER 2026-11-03: (a) Panel hairline kept; (b)-(d) exempted with
  reasons. f8759bee closes phase 4: stock 0 files, 0 calls; notes
  claude/t4-plan 021d53e results/2026-11-03-t4-phase4.
- T5 moved to the hot-paths worker (coordinator 2026-11-03). Only phase
  7's Windows 7-style (glass) theme depends on T5 (glass and backdrop
  work).
- PAUSED 2026-11-03 (coordinator). claude/t4-themes head 4ae44509:
  - 15c08d53 is the raised breadcrumb.
  - bec6c306 is 5a, the dark colour rule.
  - f8759bee is the phase 4 gate: 0 stock calls, with the four
    exemptions the owner agreed recorded and reasoned in the census.
  - 4ae44509 is a WIP commit for 5b/5c: the dark records, Theme::resolve,
    Control::visual_context plus the backplane reading the colour scheme,
    the engine darkening the sheet's materials and colour parameters by
    rule, and the contrast report's dark presentation (gui-forms dark is
    gated at 0 failures, 840 pass).
- RESUMED 2026-11-03 (after the shutdown). Backdrop failure FOUND and
  FIXED: bisect 855b0f0e..f8759bee (base passes) names 138a3f3c (panels
  through the engine). The fixture's root Panel face became an engine part
  raster; the backplane fallback (composite_backplane_chunk) composites
  part rasters and skips plain fills, so the opaque root face now covered
  the window material's base. No product change (with a framebuffer the
  whole plane replays, opaque before too); the test root is now a control
  that draws nothing (89b97744). Hashes unchanged.
- WIP split (old WIP kept as local branch claude/t4-wip-5bc 4ae44509):
  89b97744 backdrop test fix; then "5b/5c the dark records"; then "5c the
  engine darkens the sheet by rule" (new test sheet_darkened: ToolStrip
  probe, 14 authored materials darker; fails with the engine change
  reverted). WIP verified before the split: 37/37 touched plain and
  sanitized, fg2 13/13 plain and sanitized, style, lifetime, census, gate.
- Phase 5 committed (2026-11-03), claude/t4-themes head d3a0d57e:
  89b97744 backdrop test fix, cf26c57e 5b/5c dark records, 5f5d6d73 5c
  engine darkens the sheet, 6d17e2ea 5d HC role table (high_contrast_roles
  in theme.cpp; derived muted_text = text, focus_ring = palette focus;
  the 7:1 gate in gui_forms_theme_tests), d3a0d57e 5e containers
  (window/panel/card) never invert in HC (derive_high_contrast_recipes
  now takes the family; docs/inventory regenerated). Each verified: 40
  touched tests plain + sanitized, fg2 13/13 plain + sanitized, style,
  lifetime, census, gate, schema/docs checks.
- Phase 5 record: notes claude/t4-plan 6d1a8a4 results/2026-11-03-t4-phase5
  (recipe contrast light/dark/HC each 840 pass 0 fail; specimen
  --theme gui-forms --repeat 2688 cells, 0 differ; pixel runs under
  threshold light 11, dark 488, HC 198 at 1x, classified: all dark ones
  are Gallery-authored light-term colours; HC = exempt instruments 120,
  gallery_controls.cpp literal grounds ~45, same-colour runs ~33 OPEN).
  Notes sparse checkout: commit with `git add --sparse`.
- ASKED (coordinator, 2026-11-03): D1 authored colours under a derived
  dark variant (1 keep as authored + Gallery converts; 2 darken by rule
  with a per-control scheme-fixed opt-out, rec.; 3 ink chosen against an
  authored ground); D2 5f host palette (1 optional HostContrastPalette in
  PresentationSettings under presentation_preferences, HC records derived
  per palette and cached in Theme, rec.; 2 paint-time mapping only).
- Tools: dev_t4/tools/round.sh '<regex>' = verify.sh + docs.sh + fg2 on a
  private desktop (one step); T4_BASE defaults to 855b0f0e there.
- verify.sh base: T4_BASE=855b0f0e until the cp15 rebase.
- NEXT: on D1/D2 answers implement them; trace the HC same-colour runs;
  owner look at the dark/HC sheets; then phase 6 (default_theme ->
  gui-forms), 7, 8.
- CHECKPOINT 15 REBASE, after checkpoint 14 merges (coordinator says
  when): rebase onto origin/main, let the effect programs win in
  progress_bar.cpp and range_control_rendering.cpp/.hpp, drop the legacy
  progress-effects census exemption, update the census, and re-verify.
- Tooling note: the bash layer turns backslash-n in heredocs into real
  newlines, so write edit scripts with the Write tool and use chr(10).
- The arm64 goldens are the Mac worker's; never re-record them here.

## Design note: theme variants, automatic scheme, colour ownership (D1/D2, 2026-11-03)

Owner rulings in force: D2 option 2 (no host palette: GUI.Forms' own
high-contrast palette, swapped at paint time); D1 replaced by theme
variants; colour belongs to the theme, not to inline styles. Status:
DESIGN, sent to the coordinator; implement after acknowledgement.

1. Variants in the theme specification (theme language).
   - A theme declares the schemes it supports with CSS's own property on
     `:root`: `:root { color-scheme: light dark; }` (also `light`, `dark`,
     `only light`-style is not admitted). Absent, a theme is light-only.
   - Rules for one scheme stay what EC3 already compiles:
     `@media (prefers-color-scheme: dark) { ... }` and `(... light)`.
     Unconditioned rules apply to both variants, so a dark variant writes
     only what differs (typically the colour roles under `:root`).
   - A theme may hold both variants in one sheet or several sheets of one
     definition; there is no separate "dark theme object" for applications.
   - C++: `ThemeDefinition::colour_schemes` (a small set: light, dark),
     filled by the compiler from `color-scheme` and settable for
     definitions built in code. `Theme::supports(ColourScheme)`.
   - GUI.Forms' own themes: `gui-forms` declares `light dark`; its dark
     variant is the phase 5 derived records plus the sheet darkened by
     rule (kept as GUI.Forms' internal way to MAKE its standard dark; the
     derived option is not offered as the application's mechanism).
     `gui_forms_theme()` stays one theme with both variants;
     `gui_forms_dark_theme()` is not needed.
   - Compiler diagnostics: an unknown scheme name in `color-scheme` is an
     error; a `prefers-color-scheme: dark` block in a theme that does not
     declare `dark` is a warning (W4xx `scheme_rules_without_variant`).
2. The automatic switch (API).
   - `ApplicationResources::set_automatic_colour_scheme(bool)` (default
     false, so today's applications keep the light look they were written
     for; the toggle is per application, shared by its windows, like the
     theme). When on, each window's `PresentationSettings::colour_scheme`
     follows the host's reported preference (capability
     presentation_preferences; where unsupported it stays light). When off,
     the application sets colour_scheme itself (as today).
   - The window's effective theme for a scheme: the application's theme
     when it supports that scheme; otherwise the fallback, GUI.Forms' own
     `gui-forms` in that scheme. Light with a dark-only theme falls back to
     gui-forms light the same way.
   - Warning, once per (theme, missing scheme) per resources: structured
     diagnostic `theme_variant_missing` naming the theme id, the missing
     scheme and the fallback used (`gui-forms dark`), written to the log
     and queryable (`Window::theme_fallback()`: the fallback in force, or
     none), so tests and tools read it, not prose.
   - Reversible without restart: a scheme change already re-resolves every
     part (EC3); switching back to light restores the application's theme
     (the fallback is a resolution choice, the application's theme is never
     replaced in ApplicationResources). `theme_changed` fires on each swap.
   - Application-set colours on the fallback theme: SUSPENDED. They were
     written against the application's theme (light), which is not in use;
     applying them on gui-forms dark is exactly the breakage seen in the
     phase 5 record. They apply again when the application's theme returns.
     Under the application's own theme they apply in every scheme (today's
     compatibility; see 3).
   - High contrast still wins over both variants (our own palette).
3. Colour ownership: inline colour is deprecated in practice, not in API.
   - Stage A (now, with the variants): warning `inline_colour` (W4xx), once
     per document (form) or, for code-set colours, once per control and
     property, naming the control's stable id and the property
     (back_color, fore_color, style colour fields, set_background, an
     inline style's colour declarations). Recommended practice, never an
     error; WinForms' BackColor/ForeColor stay in the API. Queryable like
     W401. Inline colour still applies.
   - Stage B (after FD6 lands the Gallery on form documents; .form side
     coordinated with W through the coordinator): the RELOCATION
     instrument, `tools/relocate_colours.py` (later a designer action):
     reads a form document (and a C++-built control tree via a schema
     dump), writes theme rules keyed by `#stable-id`, `.class` or role into
     the application's theme sheet, light from the inline values and dark
     seeded from the phase 5 dark rule (a starting point to edit), adds
     `color-scheme: light dark`, and removes the inline colours. Dry-run
     diff by default.
   - Stage C: inline colour keeps applying through 1.0. Whether it ever
     stops is left to the owner after 1.0 (at most an opt-in strict mode
     that suspends inline colour, as the fallback theme already does).
4. Host hints (owner note 1): a theme MAY refer to the system accent and
   text-highlight colours as hints: `env(system-accent, <fallback>)` and
   `env(system-highlight, <fallback>)` in colour positions, the fallback
   required, resolved from a host capability (`system_colour_hints`;
   unsupported hosts give the fallback). gui-forms uses neither by default.
5. Recorded WinForms divergences: (a) Windows high-contrast themes are not
   honoured; GUI.Forms' own high-contrast mode on its own palette applies
   (owner: the host's are broken, ours has better epistemics); (b)
   SystemColors are not the default colours, the theme is; (c) BackColor/
   ForeColor warn as non-recommended practice and are suspended on a
   fallback theme; (d) dark mode by theme variants, not by system colours.
6. D2 closes phase 5f: no host palette ingestion; nothing to build beyond
   recording divergence (a).
7. Order: (i) color-scheme + ThemeDefinition::colour_schemes + compiler
   diagnostics; (ii) effective theme per scheme with the fallback, the
   warning and theme_fallback(); (iii) the automatic toggle following the
   host preference; (iv) suspension of application colours on the
   fallback; (v) inline_colour warning (stage A); (vi) env() hints;
   (vii) divergences recorded (W's manifests/winforms-divergences.json via
   the coordinator). Stage B after FD6.

## Plan: phases 5-8 and T5 (written 2026-11-03)

Phase 5, derived variants (notes PLAN.md row 5; spec 5.8 rules 2-3,
12.3, 12.4; RENDERING_REFERENCE 6; studies `.t-dark` tokens are the
accepted dark form, sketches not tokens).
- 5a The dark colour rule: a pure function from a light colour to its
  dark one in OKLCh. Lightness is inverted and compressed onto the
  studies' range (paper #FFFFFF -> #0B0F0C, stock #F4F4F5 -> #161C18,
  ink #1B2733 -> #DCEAD2). Chrome hues (the blues) move to the
  Phosphor green hue (#82B361) at reduced chroma. The accent and gold
  families (the File tab, the active tool) move to the complement
  (#B36182). Signal hues (amber, red, green) keep their hue. Fitted to
  the studies' token pairs, and the fit error is recorded. Unit tests.
- 5b Relief by rule ("the light stays upper left, inset stays inset"):
  upper and left lines stay lighter than their face and lower and right
  lines darker. Highlights dim (alpha scaled) and shadows deepen toward
  black. A well stays recessed.
- 5c Plumbing: the window's colour scheme reaches part_visual_context.
  ControlRoleRecipes gains dark and dark_selected records, derived at
  Theme creation when the definition asks for a derived dark variant
  (otherwise equal to the ordinary records). Theme::resolve selects them
  under a dark scheme, and high contrast wins over dark. Sheet-declared
  colours are remapped by the same rule at resolution under a dark
  scheme, as forced high contrast remaps them, unless a rule under
  `@media (prefers-color-scheme: dark)` declares the field.
- 5d GUI.Forms' own high contrast from the role model: the per-state
  hand table (high_contrast_colours) becomes a role table (each colour
  role's high-contrast pair), with the state palettes read from roles.
  Values are kept unless the 7:1 gate fails.
- 5e Instruments: the contrast report gains the dark presentation (4.5
  and 3) and checks high contrast at 7. The specimen sheets are written
  in dark and high contrast. Gate records go under notes results/.
- 5f The host palette: a capability (unsupported where the host does not
  report one). The host reporting is host work (P8a), asked of the
  coordinator; the core seam can be built first.
- Phase 6, the switch: default_theme() -> gui-forms. Gallery traces are
  re-recorded with a difference count. The owner approves the light,
  dark and high-contrast reference set.
- Phase 7, Windows-familiar themes: Classic (procedural bevels) and
  modern flat first, then XP-style and Windows 7-style (glass needs T5).
  Owner 2026-11-03: no glow or sheen effect for 1.0; the Windows 7-style
  theme only makes sure an effect could be built: the part-local pointer,
  transparency, glass and backdrop sampling exposed to effects.
  Each gets contrast, a specimen sheet and owner approval.
- Phase 8: the stage gate record (results/<date>-t4-gate).
- T5 moved to the hot-paths worker (coordinator, 2026-11-03). Phase 7's
  Windows 7-style glass theme depends on it.

### Phase 7 capability audit (reference image, 2026-11-03)

Reference only: a third-party Windows 7 theme screenshot the coordinator
supplied. No assets are copied and no theme is recreated. The table lists
each visual capability seen and what the engine already has for it.

| Capability seen | Engine today |
|---|---|
| Uniform rounding | corner_radius; PartShape rounded_rectangle, pill, circle |
| Per-corner rounding (tabs, joined button groups) | MaterialCornerRadii (E9.6) in the material primitive; PartShape cap_start/cap_end |
| Halo outside a box (search field) | MaterialGlow `under`, reach <= 8 (maximum_glow_reach) |
| Directional outer glow (stronger on one side) | MaterialShadow in a light colour with offset and spread (up to 4) |
| Inner glow, directional inner light | MaterialShadow `inset` with offset; MaterialGlow `over` |
| Glow or shadow past the bounds | material outsets (glow reach 8, filter reach 8); effect outsets (EffectDeclaration::outsets) |
| Directional light and gradients | linear (css_angle or endpoints), radial and conic fills; relief kernels lit from the upper left |
| Frosted glass (blurred backdrop + tint) | glass primitive over the lent backplane (blur <= 8, tint, colour filter via `recipe`); WindowBackdrop window_material |
| Hairline highlights and lowlights | MaterialKeyline per edge with inset (up to 8); border_edges |
| Soft drop shadows (tiles, buttons) | MaterialShadow (up to 4), MaterialDropShadow (follows the drawn shape) |
| Window and popup shadow | overlay shadow primitive (ThemeDefinition::overlay_shadow) |
| Glossy two-tone bars (progress, capacity) | hard-stop linear fill plus `finish`; progress effect programs |
| Separators, etched dividers | relief etched divider; keylines |
| Translucent rails and bars | alpha fills, part opacity, mask (`mask-image`), WindowBackdrop transparent |
| Text glow or shadow on dark grounds | text_shadow (offset 0, blur <= 8) |
| Hover light that follows the pointer | sheen primitive (pointer overlay); effects with EffectInputKind::pointer, part-local |
| Texture, grain | procedural fills (felt, paper_fibre, brushed_metal, ...), image fills |

True gaps (smallest addition each):

1. Glass carries a tint only. The reference panes have frosted glass with
   a gradient, hairline keylines, rounding and a shadow on the same part,
   and a part selects one primitive, so glass cannot be combined with the
   material primitive. Smallest addition: glass draws its `recipe`
   material's fills, keylines, border and shadows over the blurred
   backplane, in place of the single tint. (Glass is T5/E9 work; this
   would go to the hot-paths worker.)
2. Effects cannot sample the backdrop. The effect canvas draws with alpha
   but has no read of the backplane, so a frosted or refracting effect
   cannot be written. Smallest addition: EffectDeclaration gains
   `backdrop_margin`, and the engine lends the same read-only backplane
   view glass already gets, as one canvas call that draws the blurred
   backdrop into a rect (metered like the other calls).
3. Effects clip to rectangles only. The canvas has clip_rect and a
   uniform-radius fill, so an effect on a per-corner rounded part
   overdraws the corners. Smallest addition: one `clip_part_shape()` call
   that narrows the clip to the part's PartShape, corner radii included.

Not gaps: the directional glow (an offset light-coloured shadow), the
halo, inner glows, per-corner rounding, two-tone gloss, hairlines,
separators, translucency and text glow already exist. The part-local
pointer already reaches effects (EffectInputEvent::position) and the
sheen. Icon reflections and the gadgets are application content, not
theme capability.

## Cascade spec: where styles live (owner model, 2026-11-04; simplified the same day)

Rulings: themes live in two places only. (1) The project-associated theme,
built on the active library themes, with light and dark variants; user
themes are layered over it under the existing policy. (2) Truly inline
code: a program's C++ or C# setting a value on an instance. That is
discouraged but legitimate. `.form` documents carry no theme items: they
hold structure and behaviour only, and the project theme styles them
through role classes and escaped stable IDs (`#gallery\.typography\.heading`).
There is no form-local rules layer. Visual theme editing is done in a
special `.form`, a generic Gallery miniature that serves as the theme
preview and editor surface, never inside a program's own forms.

1. Sources and layer order (low to high). This is the compiled active
   ruleset for one window. Each source is compiled once and kept with its
   provenance.
   1. Built-in recipe (the floor; CascadeLevel::built_in).
   2. Library themes: the installed packs the project theme `@import`s,
      in import order, each pinned by hash. A later import beats an
      earlier one at equal specificity, as in CSS.
   3. The project theme's own sheet (`theme_sheet`, `class_rule`,
      `stable_id_rule` by specificity).
   4. Inline code (`inline_style`): values a program sets on one control
      in code (`set_back_color`, `set_fore_color`, `set_inline_style`,
      authored surfaces). Inline really is inline, as on the web: it
      beats every sheet rule that is not `!important`.
   5. `!important` declarations reverse the order, as CSS does:
      important library/project rules over inline code, important inline
      over everything, and forced high contrast between them
      (`important_theme_sheet` < `forced_high_contrast` <
      `important_inline_style`). This is today's CascadeLevel, unchanged.
   User themes sit where they do now: under the layered policy, the user
   theme's ordinary declarations go below layer 2 and its `!important`
   ones above layer 3's ordinary ones. Under replaces_program_sheet the
   user theme takes the place of layers 2 and 3.
2. Specificity within a layer: CSS's tuple (stable ID, classes and states,
   control type and part), then source order. There are no combinators,
   so a selector names one part of one control (RuleSelector). `:is()`
   counts its most specific argument.
3. Variants per layer. Each sheet layer (library, project, user) holds
   its own `@media (prefers-color-scheme: ...)` rules. The window's scheme
   selects them in every layer at once. Whether a dark variant exists is
   the project theme's `:root { color-scheme }` (Theme::supports). Without
   one, the window shows GUI.Forms' own theme in that scheme (W403), and
   inline code colours are suspended there.
4. Inline code and colour: an inline colour (back_color, fore_color, an
   inline style's colour declarations) still applies under the
   application's theme. Its first use writes W404 `inline_colour`, once
   per control and property. Geometry and behaviour inline are not
   warned.
5. Provenance per resolved value. Every field of a resolved part (each
   primitive parameter, recipe field, material field and colour) carries
   where it came from:
   - layer: built-in, library pack (name and hash), project theme, user
     theme, inline code, forced high contrast, or the fallback theme;
   - source: file, line and column of the winning declaration, or the
     C++ setter for inline code;
   - selector: the winning rule's RuleSelector and specificity;
   - colour origin: role or literal (the existing ColourOrigin).
   The engine already keeps the rule per declaration and ColourOrigin per
   colour parameter. Missing are the layer/pack and a source position
   kept per rule. An editor queries provenance; paint never computes it.
6. The editor surface (later; not built now). The Gallery-miniature
   `.form` shows the project theme. An edit asks for its scope (this
   stable ID, this class, this control type, everything) and writes a
   rule of that selector into the project theme sheet, with a light and
   a dark value where it is a colour. It never writes into a program's
   `.form`. The relocation tool (variants stage B) moves existing inline
   code colours into project theme rules the same way.
7. Escapes (built now). A theme sheet's tokenizer admits CSS name escapes
   (NameEscapes::in_names). The compiler refuses an escape outside a
   selector (E105). Form documents and layout styles refuse all escapes,
   as before.

## Font properties: the set (owner ruling 2026-11-04, note before the build)

Fonts join the theme language. W107 no longer covers the five longhands below;
the rest of the font family (font-variant, font-stretch, font-feature-settings,
font-kerning, line-height and the `font` shorthand as CSS has it) stays left out.

- `font-family: control | content | monospace`: a bundled font ROLE (FontRole),
  one name, any case. Quoted names, host names, generic families (sans-serif)
  and lists are errors (keyword_unknown, value_form_not_admitted). Never a host font.
- `font-size: <px> | <em>`: em is the control's own font size before the rule.
  Bounded 4px to 96px (value_out_of_range).
- `font-weight: normal | bold | <100..900>`: bolder/lighter refused.
- `font-style: normal | italic`: oblique refused.
- `letter-spacing: normal | <px> | <em>`: within valid_font_spec at the resolved
  size (-0.25em to 1em), else value_out_of_range.
- The existing `font: <text role>` (T3 P8c) stays as recorded (it gives a part
  its em); controls' text does not read it yet. The longhands change single
  fields of the control's own font.
- Built as recipe fields (RecipeField::font_family..letter_spacing), lowered by the
  material lowering; Theme::resolve_font resolves them at rest (ResolvedFont with
  per-field levels); Control::text_font(own) / resolved_text_font(own) cache it;
  set_font marks set_font_from_code. Forced high contrast keeps them.

Where they apply: rules matching a control (type, class, #id), each longhand a
cascaded field with its own CascadeLevel, so provenance is per field. A part rule
drops them with a warning, as recipe properties are. A rule with a state
(:hover, :focus...) refuses them (property_not_admitted "font in a state rule"):
a font is a layout input and states change without a layout. Under
`@media (prefers-color-scheme: dark)` they are admitted, so fonts may differ by
scheme. They resolve on the control's PRIMARY text, the font a control holds
(font_, set_font); a control's fixed secondary fonts stay as they are.

Inline code: set_font is inline code and wins over every theme field. Layer
order as the cascade spec: library < project theme < inline code.

Invalidation: the resolved font is cached per control and dropped on a theme,
scheme, class or attach change; a change of the resolved font marks LAYOUT dirty
(re-measure plus repaint), never paint only.

## Decisions in force

- T4-1 authored colours, inline level, dropped in forced HC (approved).
- T4-2 default theme id `gui-forms`; windows-professional stays.
- T4-3 default's sheets compiled at first use, gated by a measured startup
  bound.
- T4-4 four exemptions; only frames and backgrounds through the theme.
- Progress effects: legacy census exemption until registered effects
  (owner, 2026-11-02).
- Disabled text: the theme's own disabled text everywhere (owner,
  2026-11-02).
- Skeudoindustrial after T4, does not gate 0.5; no finishes phase now.
- Census check runs in the existing house-style job; no new job.
- Tell the W worker (via the coordinator) when painting colours works.
- Projects clone commits: only this file, with the coordinator's command.

## Next

- See RESUME POINTER above (the last "Next" bullet).
- NEXT ITEM (coordinator, 2026-11-04, cp16): a frameless engine well for the borderless
  TextBox. text_box.cpp's BorderStyle::none branch (FD6 sweep defect D12) still calls
  paint_surface_material directly for the editor ground, because the engine's well always
  draws its frame. The paint census baseline is 1 for panel/text_box/text_box.cpp until then.
  Remove the call and `paint_census.py update` back to 0.
