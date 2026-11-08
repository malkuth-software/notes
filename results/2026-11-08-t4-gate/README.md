# T4 stage gate (phase 8): draft record

Status: DRAFT. The code is written; none of the measurements below has been
taken. Branch `claude/t4-selectors-2` in GUI.Forms (heads 685e79f1 floor
sheet, fcf81b15 notice and badge, 36de6177 phase 7), unverified, queued for
the integrator. The integrator's run of `dev_t4/verify_once.sh` and then
`dev_t4/t4_gate.sh` (worker w1) produces every file this record cites
under `build/t4-gate/`; copy them here and fill the tables. No value here may
be filled in from anything but that run.

## What the gate requires (T4 PLAN.md, phase 8)

The record: specimen sheets, contrast reports, the census at zero, the
owner's approvals.

## What T4 built

| Phase | Result | Record |
|---|---|---|
| 1 instruments | style mapping, paint census, contrast report | `results/2026-11-01-t4-phase1` |
| 2 specimen sheet | `gui_forms_specimen_sheet` | `results/2026-11-01-t4-phase2` |
| 3 default theme, light | `gui-forms` floor and light sheet | `results/2026-11-01-t4-phase3-light` |
| 4 stock controls on the engine | batches; census ratchet | track `t4_themes.md` (census at checkpoint folds) |
| 5 derived variants | dark by rule, high contrast from the role model | no separate record (CLOSURE item 4 asks for one) |
| 6 the switch | step 1 landed; `default_theme()` unchanged pending the owner | track `t4_themes.md` |
| owner option 1 | floor written as `:family()` blocks (`gui-forms/floor.css`), new properties `muted-color`, `glyph-color`, `default-outline`, `keylines`; old C++ builders kept as the test's reference | `gui_forms_theme_tests` floor_matches_reference |
| 7 Windows-familiar themes | `classic`, `modern-flat`, `xp-style`, `windows7-style` as floor sheets, light only | this record |

## Measurements to take (all pending)

| Check | Command (in `dev_t4/t4_gate.sh`) | Result |
|---|---|---|
| floor equals the reference builders, every family, state, selection | `gui_forms_gui_forms_theme_tests` | pending |
| `gui-forms` contrast, light, dark, high contrast | `GUI_FORMS_CONTRAST_REPORT=.../gui-forms.tsv` | pending |
| familiar themes: zero diagnostics, work bound, round trip | `gui_forms_familiar_themes_tests` | pending |
| familiar themes contrast, light and high contrast | `GUI_FORMS_CONTRAST_REPORT_DIRECTORY=...` (`<id>.tsv`) | pending |
| specimen sheets, two byte-identical runs each | `gui_forms_specimen_sheet --repeat --theme <id>` for gui-forms and the four | pending |
| paint census at zero stock calls | `tools/paint_census.py check` | pending |

## Choices made in phase 7 (stated as choices, for the owner)

- Every familiar theme meets the same contrast gate as the default. Where a
  period look falls short, the sheet darkens the value and its comment says
  which: classic's outer frame is the dark shadow `#404040` on every side and
  its disabled text `#6d6d6d`; xp-style's field frame `#4f6f99` and disabled
  text `#7a7765`; windows7-style's field frame `#7a7a7a`, default button
  frame `#2c628b` and press gradient end `#98d1ef`; modern-flat's toolbar
  and menu hover show a `#767676` or accent frame.
- All four are light only (no derived dark variant); high contrast is
  derived as for every theme.
- They are themes a window or application shows, not compile bases
  (`ThemeBase` still holds only `windows_professional` and `gui_forms`):
  making them bases is an owner decision.
- windows7-style's glass waits for T5's glass material; its windows are
  opaque stock until then.
- SDRSharp fixtures (R) are not part of this record: none exist in the tree.

## Approvals (pending, the owner)

| Theme | Light | Dark | High contrast |
|---|---|---|---|
| gui-forms | pending | pending | pending |
| classic | pending | n/a (light only) | pending |
| modern-flat | pending | n/a | pending |
| xp-style | pending | n/a | pending |
| windows7-style | pending | n/a | pending |
