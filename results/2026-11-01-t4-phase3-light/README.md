# T4 phase 3: the `gui-forms` default theme, light

Date: 2026-11-01. Branch `claude/t4-themes` at 143a1cbf (T4 phase 3: the
gui-forms default theme, light), on main 3e0c8d5a. Host: Windows 11 x64,
MSYS2 clang64, CPU Skia raster, bundled fonts. Plan Paint pin:
`malkuth-software/plan-paint` at `5e28a5e` (the earlier pin `6b901fc` went
with the history reset of 2026-10-27).

## What was built

- `gui_forms_theme()` / `gui_forms_theme_build()`
  (`include/gui_forms/theme/gui_forms_theme/gui_forms_theme.hpp`): the
  theme `gui-forms` (T4-2), built at first use (T4-3).
  - **Floor** (`gui_forms_theme_definition()`): Plan Paint's action plate for
    buttons (face `#D2E4FB` to `#A3C1EC`, border `#6384B2` (hot `#355E96`),
    white inner top and left line, `#315686` at 53 % bottom and right, contact
    shadow one unit below; pressed inverts the lines and drops the shadow),
    the gold active tool for selected buttons, flat command members at rest,
    recessed wells (white, inner top shade) for editors and choices,
    near-white stock `#F4F4F5` for windows and panels, pale selection
    `#CFE2FA`, the accent in the File-tab blue, and text colours meeting the
    gate. `default_theme()` is unchanged until phase 6.
  - **Light sheet** (`gui_forms_light_sheet()`, `gui-forms/light.css`): the
    colour roles of design language 5.4 in Plan Paint's palette, Plan Paint's
    tab stack for `tab-control`/`page-strip` tabs, the command plane for
    tool and menu strips, the flat pale status strip.
- `stock_control_anatomies()`: the 91 stock control anatomies, generated
  from `manifests/control-schema.json` by `tools/stock_anatomies.py`
  (`--check` in the house-style job), so the core's sheet can name stock
  types.
- `detail::compile_built_in_theme`: a sheet compiled onto a given floor
  at the program levels, keeping the sheet as the theme's source.
- The specimen sheet takes `--theme gui-forms` and `--light`.

## Gate (PLAN.md, phase 3)

| Item | Result |
|---|---|
| Zero diagnostics | 0 (`gui_forms_gui_forms_theme_tests`) |
| Startup compile and work counters, stated bound | 2,466 sheet bytes, 8,711 compiler operations (3.5 per byte); stated bound 64 operations per byte, asserted. Build time (floor, compile, theme) 2.0 to 2.5 ms on this host, recorded, not asserted |
| Byte round trip (T3 P11 item 5) | the sheet parses completely and writes back identical bytes (asserted) |
| Raster reference hashes (T3 P11 item 2a) | `manifest.tsv`: 896 cells of the specimen sheet under `gui-forms`, light at scales 1 and 2; two captures byte-identical (0 differ) |
| Contrast passes for light | recipe report `contrast_gui_forms.tsv`: 840 light rows, 0 fail (asserted). Pixels: see below |
| Each new primitive against Plan Paint's pixels | no new primitive in this phase (see Open) |
| First owner look at the light sheet | `sheet-light-1x-0.png` to `-3.png` here |

### Values changed from Plan Paint to meet the gate

Each is commented where it is set (`src/core/theme/gui_forms_theme/gui_forms_theme.cpp`):

- Disabled text `#52637A` (Plan Paint's `#627997` reaches 2.6:1 on its own
  disabled face).
- The gold plate's border `#A67822` (Plan Paint's `#C49332` reaches 2.5:1 on
  the stock).
- Well borders `#6384B2` (Plan Paint's gallery well `#94ACC8` reaches 2.2:1 on
  the stock).
- Accent face from `#2F69BE` (the File tab's `#528BE0` gives white text 3.4:1).
- Muted text `#4A545F` on stock and `#2C3A4E` on action plates.

### The contrast report's two corrections

The phase 1 tool measured two pairs where they do not sit; both changes
apply to every theme (windows-professional now: 1680 rows, 464 fail, was
1688 and 466):

- The house focus ring is inset at least 3 units inside the face
  (`paint_focus_ring`), so it is measured against the face on both sides; an
  authored outline is still measured against the surround.
- A selection row's boundary is gated when selected, hot or pressed: a row
  is flat until then.

### Pixel contrast under `gui-forms`, light

2162 runs under threshold under windows-professional (all presentations)
become 257 under `gui-forms` light (`contrast.tsv`, `contrast_groups.tsv`):
256 runs are colours the theme does not supply (literal `#848F99`/`#858F98`
disabled text and Gallery literals: phase 4), and 1 is the theme's white on
a block panel's pressed literal ground. No run with a theme colour on a
theme ground fails.

## Open

- **Tab stack and command plane reach no pixels yet.** The sheet's
  `::part(tab)` and `::part(face)` rules compile, but `PageStrip` and
  `ToolStrip` paint those parts without a `PartKey` naming them, so no rule
  selects them. Phase 4 keys them (or the tab stack becomes the engine
  primitive the plan foresees); then they are compared with Plan Paint's
  pixels.
- **Dotted focus ring.** The built-in recipe draws the ring solid (dark ink
  inside light faces, white inside the accent); the dotted ring needs a
  primitive.
- **Hot cues.** On the Gallery's specimens hot changes 4 of 64 cells: most
  first instances are style-overridden or draw no hot state (phase 4).
