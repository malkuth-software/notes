# T4 stage gate (phase 8): draft record

Status: DRAFT. The code is written; none of the measurements below has been
taken. Branch `claude/t4-selectors-2` in GUI.Forms (heads 685e79f1 floor
sheet, fcf81b15 notice and badge, 36de6177 phase 7, 35c202a5 the
disabled exemption and the reference pass), unverified, queued for
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

- The contrast gate exempts disabled controls (owner, 2026-11-08; WCAG
  exempts inactive components): disabled text and disabled boundaries
  carry the verdict `exempt`, minimum 0, in every theme's report, the
  specimen sheet and the dark-variant test. No theme keeps exceptions.
  The one colour darkened only for the old 3:1 disabled gate was
  gui-forms' disabled ink (and a disabled-tab rule); both are back to
  Plan Paint's `#627997`. Familiar disabled text: classic `#808080`
  (98.css), xp-style `#aca899` (Luna), windows7-style `#838383` (7.css),
  modern-flat `#6e6e6e` (our own, never darkened).
- The familiar themes follow their references wherever the engine can
  express it (GUI.Forms 35c202a5): 98.css for classic, XP.css for
  xp-style, 7.css for windows7-style (read from their pages and CSS, not
  vendored). Only enabled-element contrast fixes remain; each is listed
  below and in its sheet's comment.
- All are light only (no derived dark variant); high contrast is derived
  as for every theme.
- They are themes a window or application shows, not compile bases:
  making them bases is an owner decision.
- windows7-style's glass waits for T5's glass material.
- SDRSharp fixtures (R) are not part of this record: none exist in the tree.

## Reference comparison, control by control

What our engine cannot express in any familiar theme yet: padding and
minimum sizes (not recipe fields); more than one inner keyline ring;
check and radio marks, tabs, scroll bars, the menu bar and the title bar
as separate parts (the familiar themes have no parts sheet, so those parts
draw from the family recipes); a separate focus treatment beyond the
structure's ring (no `:focus` family state); outline offsets; text
shadows; animation.

### classic (98.css)

| Control | Changed to follow 98.css | Contrast fix (enabled) | Not expressible yet |
|---|---|---|---|
| surface | face `#d4d0c8` (Windows 2000) to silver `#c0c0c0` | none | none |
| button | bevel white/`#808080` ring inside the frame; pressed ring `#808080`/`#dfdfdf` (inverted) | frame `#0a0a0a` on every side (98.css's white top-left is 1.8:1) | second ring (`#dfdfdf`/grey inside white/`#0a0a0a`); padding 0 12px, min 75x23 |
| default button | outer `#0a0a0a` ring as `default-outline` | none | none |
| focus | dotted black ring (structure) | none | offset -4px |
| field | white well; outer edges `#808080` top-left / white bottom-right as per-edge frame, inner ring `#0a0a0a`/`#dfdfdf`; disabled silver | top-left edge `#696969` (`#808080` is 2.2:1) | none |
| check and radio | none | none | marks are parts |
| group box (card, notice) | silver groove: per-edge frame, white/`#808080` ring | top-left edge `#696969` | legend placement |
| tabs, scroll bars, title bar | none | none | parts (title bar navy to `#1084d0`) |
| progress | navy `#000080` fill; well track | none | segmented blocks |
| menus and selection | navy `#000080` with white text | none | none |
| pending frame | ours | `#705800` (`#8a6d00` is 2.7:1 on silver) | none |

### xp-style (XP.css)

| Control | Changed to follow XP.css | Contrast fix (enabled) | Not expressible yet |
|---|---|---|---|
| surface | `#ece9d8` (unchanged) | none | none |
| button | gradient `#fff`, `#ecebe5` 86%, `#d8d0c4`; frame `#003c74`, radius 3 | none | padding |
| hover | inner ring `#fff0cf`/`#fbc761`/`#e5a01a`/`#fdd889` (from four offset inset shadows) | none | the shadows' 2-unit depth |
| pressed | gradient `#cdcac3`, `#e3e3db` 8%, `#e5e5de` 94%, `#f2f2f1` | none | none |
| default button / focus | inner ring `#cee7ff`/`#bcd4f6`/`#89ade4`/`#98b8ea` | none | focus ring and default ring are one ring |
| field | white | frame `#4f6f99` (`#7f9db9` is 2.3:1) | none |
| check and radio | none | none | marks are parts (green check `#22a122`) |
| group box | notice: white, `#d0d0bf`, radius 4 | card frame `#86867b` (`#d0d0bf` is 1.3:1) | legend colour `#0046d5` |
| tabs, scroll bars, title bar | none | none | parts |
| progress | white track, frame `#686868`, radius 4; green fill gradient `#acedad`, `#2ed330` 42%, `#fff` | muted text on the fill `#1a1a1a` | segmented blocks; the full stop list |
| menus and selection | `#316ac5` to `#2267cb`, white text | none | none |

### windows7-style (7.css)

| Control | Changed to follow 7.css | Contrast fix (enabled) | Not expressible yet |
|---|---|---|---|
| surface | `#f0f0f0` (unchanged); text `#000` to `#222222` | none | none |
| button | hard 45% gradient `#f2f2f2`, `#ebebeb`, `#cfcfcf`; inner ring `#ffffffcc`; radius 3 | frame `#898a8a` (`#8e8f8f` is 2.9:1) | padding 0 12px, min 75x23 |
| hover | `#eaf6fd` 45%, `#bee6fd` 45%, `#a7d9f5`; frame `#3c7fb1` | none | none |
| pressed | `#e5f4fc`, `#c4e5f6` 30 to 50%, `#98d1ef` 50%, `#68b3db` | frame `#6c8fa9` (`#6d91ab`, 2.9:1); muted text `#3f3f3f` | none |
| default button | frame `#3399ff` to `#5586a3` on the hover gradient | none | the pulsing inner glow `#34deffdd` |
| focus | dotted black ring (structure) | none | the 2-unit `#98d1ef` inner glow; offset -4px |
| field | white, radius 2; hover and focus frames | rest `#898a8f`, hover `#5590ba` (top edges `#abadb3` 2.0:1, `#5794bf` 2.9:1) | per-edge colours with rounded corners (the lighter side edges) |
| check and radio | none | none | marks are parts |
| group box | notice: `#cdd7db`, radius 3, white ring | card frame `#858c8e` | none |
| tabs, scroll bars, title bar | none | none | parts; window glass (T5) |
| progress | track `#dddddd`, frame `#8e8f8f`, radius 3; fill `#06b025` to `#0bd82c` | muted text on the fill `#1a1a1a` | paused and error fills (no state) |
| menus | translucent gloss `#ffffff99`, `#e6ecf5cc` 90%, `#ffffffcc`, radius 3 | frame `#6e8fa2` (`#aaddfa` 1.3:1) | the menu bar's `#3399ff` hover |
| selection | `#3399ff` with white text | `#2878c9` (white on `#3399ff` is 2.9:1) | none |
| disabled | `#f4f4f4`, `#adb2b5`, text `#838383`; fields `#f0f0f0`, `#afafaf` | exempt | none |

## Approvals (pending, the owner)

| Theme | Light | Dark | High contrast |
|---|---|---|---|
| gui-forms | pending | pending | pending |
| classic | pending | n/a (light only) | pending |
| modern-flat | pending | n/a | pending |
| xp-style | pending | n/a | pending |
| windows7-style | pending | n/a | pending |
