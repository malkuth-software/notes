# T3 resume plan (paused 2026-10-29)

T3 stopped at the owner's direction on 2026-10-29, partway through P12a.
Everything through P10 and P9 is on `main`. This note lets a successor take
up the work from here without the earlier conversation.

## Where things stand

| Item | State | Where |
|---|---|---|
| P1 to P8, P10 | On `main` | earlier checkpoints |
| P9 (`!important`, inline styles, forced high contrast) | On `main` as one squashed commit | 947a02c6, checkpoint 4 (#448, merge 7bca3c66) |
| `ThemeCompileResult` as a result type | Committed locally; the coordinator has it for checkpoint 5 | `claude/t3-p12-custom` at aba9d2a5 (the coordinator's clone holds it as e7f17abb) |
| P12a | Partial, not verified | `claude/t3-p12-custom` at 7b54b5b9, one commit above aba9d2a5 |
| P12b | Not started | |
| P11 | Not started | |
| User themes (`USER_THEMES.md`) | Designed, with the owner's decisions; no code | this directory |

The gui_forms clone is `~/Developer/gui_forms_workers/w1/gui_forms`.
`claude/t3-p12-custom` is rebased onto `origin/main` 7bca3c66 and holds only
two commits above it: aba9d2a5, then 7b54b5b9.

## P12: the settled design

The owner decided on 2026-10-29 that custom properties are resolved when the
theme loads, not at paint. PLAN.md section 1.11 records the decision. It
replaces that section's per-part lowering and cache.

**The hybrid.**
- A name defined in only one place is substituted directly at load, as
  before P12. It makes no copies and stays global: a single definition in a
  rule's scope is still read everywhere.
- A name defined in more than one place is expanded only for the rules that
  read it. A rule reads a name directly, or through other names'
  definitions, or through fallbacks.

**Three kinds of name**, set by `Variation` in the compiler.
- *fixed*: one definition, or palette definitions of which one holds
  everywhere. When a later unconditional `:root` definition replaces an
  earlier one, only the later one counts.
- *media*: defined only in the palette (`:root`), under `@media`
  conditions.
  - A reader is copied once per world. A world is a subset of the distinct
    conditions among the media names the rule reads, taken as holding.
  - The worlds are emitted in counting order, each copy carrying the
    conditions joined with the rule's own media. In any window the last
    copy that holds is the right one, which is the same argument as P8b's
    palette copies.
  - Two kinds of world are skipped: one no window can meet, and one where a
    condition it leaves out already holds wherever it does.
  - So light and dark tokens cost 2 copies however many tokens a rule
    reads, not 2^n.
- *scoped*: defined in some rule's scope as well.
  - Each definition is emitted as an unconditioned engine rule
    (`ThemeRule::scoped_definitions`: the name and an id, the id being the
    declaration's node index). It keeps its rule's selector, level (with
    the `!important` split) and media.
  - The palette's definitions of a scoped name are emitted first in the
    sheet, at `theme_sheet` with an empty selector and zero specificity:
    ordinary ones first, then `!important` ones. Any rule's own definition
    therefore beats them, as a CSS declaration beats an inherited value.
  - A reader is copied once per combination of the candidates of the
    scoped names it reads. Each copy carries `scoped_conditions`
    (name → id, or `custom_property_undefined`).
  - At resolve, the engine computes the part's winners once: the last
    matching definition per name, in cascade order. `RuleWalk` then admits
    only the copies whose conditions name the winners.

**Pruning.** The copies are found by a depth-first search over the scoped
candidates, which drops combinations that cannot win together:
- a palette candidate needs its media condition, and the conditions under
  which a later palette definition replaces it must not hold;
- a scoped candidate's rule matches wherever it wins, so a palette
  candidate (or none) of another name excludes that rule when it belongs to
  that name's scoped definitions.

So one class block that redefines three names costs 2 copies, not 8.

**Undefined.** When no definition holds and the `var()` has no fallback,
that copy does not declare the declaration (CSS's "invalid at computed
value time"). E409 stays only for a name defined nowhere.

**Diagnostics** (new):
- E416 `variable_cannot_vary`: a varying name read in `@keyframes` or in
  the palette;
- E508 `too_many_variable_combinations`: the copies per rule;
- E515 `too_many_scoped_variables`: the scoped names read per theme.

**Limits.** These are provisional, and the test theme sets them:
- `maximum_variable_combinations` = 64 copies per rule;
- `maximum_variable_steps` = 4096 candidate combinations considered per
  rule (also E508);
- `maximum_scoped_custom_properties` = 64. The engine keeps a fixed winner
  array of this size.

**The token-heavy test theme** sets the limits with headroom, so that
ordinary themes are never refused. It is in the new test file and has:
- about 100 tokens: colour scales, semantic colours through palette roles,
  radii, widths, shadows, motion, opacity, and spacing, type and size
  scales;
- light and dark and reduced motion through `@media`;
- four scoped overrides: `.primary`, `.danger`, `.compact`, `slider`;
- 30 style rules over five controls.

The test prints the rule count, copies, the most copies for one rule, the
scoped names and the size in memory. It is to require at least four times
the headroom on copies and eight times on scoped names. Record the measured
numbers in PLAN.md section 1.11 when it runs.

## What the P12a commit holds

7b54b5b9 holds the following.
- The engine (`theme_rules.hpp/.cpp`):
  - `ScopedDefinition` and `ScopedCondition`;
  - the two new `ThemeRule` fields;
  - `scoped_property_invalid`;
  - the scoped names table and `scoped_name_count()`;
  - `resolve_scoped` winners;
  - `RuleWalk` admission;
  - every resolve path passing the winners.
- The custom properties module:
  - the definition's rule, media block and importance;
  - `definitions_named()` and `definitions()`;
  - `append_variable_names()`;
  - `SubstitutionInput::choices`, with `variable_unassigned` and
    `variable_undefined`;
  - the `SubstitutionOutcome::undefined` outcome.
- The compiler:
  - `VariationStorage`, `NameVariation` and `VariableCandidate` in the
    header;
  - `collect_variations` (it runs before `collect_palette`);
  - `palette_variants` and `scoped_candidates`;
  - `emit_definitions` and `lower_definitions`;
  - `read_variations` and `candidates_meet`;
  - the new `lower_rule` (worlds, then the search), with the old body as
    `lower_rule_palette`;
  - undefined handling in `lower_declarations` and the primitive loop;
  - conditions in `emit_rule`;
  - the E999 for `!important` on a custom property in a sheet removed.
  Inline custom properties stay E999 until P12b.
- Diagnostics E416, E508 and E515 with their names and labels.
- Updated existing tests:
  - `theme_value_tests`: the twice-defined name read without choices is
    now E416;
  - `theme_compiler_tests`: the refusal list (E416 in keyframes replaces
    the old `!important` E999 entry) and the codes array.
- New: `tests/theme_language/theme_custom_property_css_tests.cpp`, with
  its CMake target.

**Verified, on the M4 only, before the rebase:** the library and the
`gui_forms_theme_` and `gui_forms_engine_` tests built and passed, plain
(45 of 45, without the new test file).

**Not verified:**
- the new test file was never built or run;
- no sanitizer run;
- no checkers (house style, lifetime rules);
- `control_schema` and the library docs neither regenerated nor checked;
- nothing built since the rebase onto 7bca3c66.

## Resume steps

1. In `~/Developer/gui_forms_workers/w1/gui_forms`, check out
   `claude/t3-p12-custom`. If `origin/main` has moved, run
   `git rebase --onto origin/main aba9d2a5` once the coordinator has landed
   aba9d2a5 (checkpoint 5); otherwise rebase the two commits.
2. Build and run the touched tests on the M4 (from
   `~/Developer/gui_forms_workers/w1`), then fix what the new file finds:
   `./../bin/m4ws w1 python3 tools/ci_build.py --jobs 8 --tests "'gui_forms_theme_|gui_forms_engine_|gui_forms_gallery_pages_tests'" > dev/logs/p12.log 2>&1`.
   Points to check first:
   - whether the token theme compiles with no diagnostics (literal colours
     at edge or text positions give W201; the theme uses palette roles
     there for that reason);
   - the exact copy counts the tests assert;
   - the `transition:` shorthand with `var()` durations.
3. Read the printed token-theme numbers. Set the limits from them with at
   least four times the headroom on copies and eight on scoped names.
   Record the numbers and limits in PLAN.md section 1.11, which still says
   "recorded here when measured", and tighten the test's assertions to the
   measured values.
4. Check the existing `var()` uses in tests (about 51: 44 in
   `theme_value_tests`, the rest in `theme_compiler_tests`,
   `theme_colour_role_css_tests`, `theme_animation_css_tests` and
   `layout_style_tests`). Only the twice-defined case in `theme_value_tests`
   read more than one definition, and it is updated. Confirm on the run.
5. Run the same tests with `--sanitize`. Then, from `gui_forms/` on the
   laptop:
   - `python3 -B tools/check_house_style.py include src demo tests examples tools file_manager_demoboard stx_vorbis stx_zlib stx_png stx_conv fuzz`;
   - `python3 -B tools/check_lifetime_rules.py include src demo tests examples tools file_manager_demoboard stx_vorbis`;
   - `python3 tools/control_schema.py check` and `render --check`;
   - regenerate the docs locally with `python3 -B tools/generate_library_docs.py`
     (never on the M4 copy), then run it with `--check`.
6. Rewrite the commit message of 7b54b5b9 to the verified state, update the
   T3 track file, and report P12a.
7. P12b, as in PLAN.md section 1.11, "Across compilations":
   - inline custom property definitions (the E999 is in `lower_rule_once`);
   - checking each one against the active theme's readers that could match
     the control's parts, refusing a bad value with
     `variable_kind_mismatch`, so the control keeps its previous inline
     style;
   - the reverse check when a theme is installed or switched live: refuse
     a theme whose readers read an existing inline definition as the wrong
     kind;
   - the engine's winners across the theme's and the inline rule sets,
     which are matched by name. The names table today is per
     `CompiledThemeRules`, and `resolve_scoped` reads the theme's.
8. P11, the gate (PLAN.md): the conformance corpus and coverage table, the
   malformed and hostile suite, a recorded fuzz run, and
   `docs/user/theme-language.md`, recorded under
   `results/<date>-t3-theme-language-gate/`.
9. The user themes, after P12 and P11, as `USER_THEMES.md` describes with
   the owner's decisions of 2026-10-28 (section 9):
   - the `@theme` header;
   - no tracked state: export is a verbatim copy;
   - one theme at a time, possibly multi-file, with sub-files in filename
     order and the main file last;
   - `@import` of packages and partials;
   - icon overrides by file convention;
   - `ApplicationResources::set_user_theme_policy`;
   - the six-level order, with `user_theme` above `built_in` and
     `important_user_theme` between `inline_style` and
     `important_theme_sheet`;
   - the 0.5 opacity floor;
   - the per-user folder winning;
   - no hold-key at launch.

   The owner's 2026-10-28 direction stands: system high contrast and light
   or dark changes are tracked only when the program opts in.

## Rules that still apply

- Build only on the M4 through `~/Developer/gui_forms_workers/bin/m4ws`.
- Never push. The coordinator lands checkpoints.
- No agents.
- Run the python-only generators on the laptop tree.
