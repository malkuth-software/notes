# T4 phase 2: the specimen sheet of `windows-professional`

Date: 2026-11-01. Branch `claude/t4-themes` at ee1da5c3 (T4 phase 2: the
specimen sheet), on main 3e0c8d5a. Host: Windows 11 x64, MSYS2 clang64
toolchain, CPU Skia raster, bundled fonts; plain build tree.

Command (from the build tree):

    gui_forms_specimen_sheet --out <dir> --repeat

`--repeat` captures every cell twice and compares; ctest runs `--smoke`
(eight controls, light at scale 1, twice), plain and under the sanitizer.

## Gate (PLAN.md, phase 2)

- **Controls times states captured:** 64 stock controls (the first wholly
  visible instance of each anatomy in the Gallery's 31 pages) times 7 states
  (rest, hot, toggled, disabled, invalid, focus, pressed) times 6
  configurations (light, dark, high contrast; scales 1 and 2): 2688 cells,
  2106 applicable (toggled applies to controls that declare checked or
  selected, focus to controls that take keyboard focus).
- **Two runs byte-identical on one host:** 0 of 2688 cells differ between
  the two captures; a later single run after the contrast measurement was
  refined reproduced every hash (`manifest.tsv`).
- **Pixel and recipe contrast:** 2162 text runs fall under their threshold
  (`contrast.tsv`; grouped by presentation, text and ground in
  `contrast_groups.tsv`, each group matched against the phase 1 recipe
  report `../2026-11-01-t4-phase1/contrast_windows_professional.tsv`). The
  differences are listed by class below.

Sheets are not committed (7.5 MB); `sheets.sha256` records them, and the
command above regenerates them.

## How contrast is measured

Each cell's window is painted a second time into a recorder that keeps the
translation and clip stack and every text call. A run is cut to the clip in
force (a run the clip hides is not measured). Its ground is the most frequent
colour under the run once glyph pixels (within 24 of the text colour, summed
over channels) are left out; the text colour is composited over that ground.
The threshold is 7:1 in high contrast, 3:1 for text a disabled control
paints (the disabled column, or a disabled control inside a specimen), and
4.5:1 otherwise.

## Differences, by class

- **A**: the same pair fails in the recipe report; pixels and recipe agree.
- **B**: disabled text (`#848F99`, and `#858F98`/`#A0A0A0` painted as
  literals) on container grounds the recipe pairs do not cover. The recipe
  report fails the same colour on its own disabled grounds (2.61:1 and
  2.18:1), so pixels and recipe agree in kind; the fix is the theme's
  disabled text colour (phase 3 for `gui-forms`, a palette change for
  `windows-professional`).
- **C**: a theme text colour on a ground the control paints as a literal
  (phase 4 moves those grounds onto the engine).
- **D**: a literal text colour the theme does not supply (phase 4).
- **E**: invisible text: in high contrast the theme's white text lands on a
  literal white ground (property grid and list values, scaled panels, user
  control), or literal black text on the high-contrast black (tab pages).

| Presentation | Class | Runs | Smallest | Controls |
|---|---|---|---|---|
| dark | A same pair fails in the recipe report | 8 | 4.02 | 1: text_box |
| dark | B disabled text on a ground the recipe pairs do not cover | 506 | 1.49 | 35: binding_navigator, block_panel, button, check_box, checked_list_box, choice_group, color_value_editor, combo_box, command_bar, command_overflow_panel, container_control, domain_up_down, drop_down_button, expandable_sections, flags_value_editor, flex_panel, flow_layout_panel_compat, grid_panel, instrument_rack, list_box, master_detail_view, menu_strip, numeric_up_down, panel, property_grid, property_list, radio_button, review_card, scaled_group_box, scaled_panel, scrollable_control, split_container, splitter_panel, tab_control, table_layout_panel_compat |
| dark | C theme text colour on a literal ground | 48 | 1.09 | 7: binding_navigator, block_panel, dock_site, page_strip, tab_control, table_layout_panel_compat, text_box |
| dark | D literal text colour (no theme colour) | 20 | 3.86 | 3: card, instrument_rack, table_layout_panel_compat |
| high-contrast | A same pair fails in the recipe report | 82 | 4.02 | 5: binding_navigator, dock_site, page_strip, tab_control, text_box |
| high-contrast | B disabled text on a ground the recipe pairs do not cover | 298 | 1.49 | 16: button, check_box, command_bar, date_time_picker, drop_down_button, flags_value_editor, group_box, label, link_label, property_grid, property_list, radio_button, scaled_group_box, scaled_panel, scrollable_control, user_control |
| high-contrast | C theme text colour on a literal ground | 64 | 1.08 | 7: breadcrumb_trail, command_bar, flags_value_editor, link_label, scaled_group_box, scaled_panel, tree_view |
| high-contrast | D literal text colour (no theme colour) | 478 | 1.39 | 14: card, correspondence_view, dock_site, easing_preview, expandable_sections, instrument_rack, page_view, scaled_panel, tab_control, tab_page, table_layout_panel_compat, text_block, typography_specimen, visual_inspector_view |
| high-contrast | E invisible: text on its own colour | 76 | 1.00 | 7: property_grid, property_list, scaled_group_box, scaled_panel, tab_control, tab_page, user_control |
| light | A same pair fails in the recipe report | 8 | 4.02 | 1: text_box |
| light | B disabled text on a ground the recipe pairs do not cover | 506 | 1.49 | 35: binding_navigator, block_panel, button, check_box, checked_list_box, choice_group, color_value_editor, combo_box, command_bar, command_overflow_panel, container_control, domain_up_down, drop_down_button, expandable_sections, flags_value_editor, flex_panel, flow_layout_panel_compat, grid_panel, instrument_rack, list_box, master_detail_view, menu_strip, numeric_up_down, panel, property_grid, property_list, radio_button, review_card, scaled_group_box, scaled_panel, scrollable_control, split_container, splitter_panel, tab_control, table_layout_panel_compat |
| light | C theme text colour on a literal ground | 48 | 1.09 | 7: binding_navigator, block_panel, dock_site, page_strip, tab_control, table_layout_panel_compat, text_box |
| light | D literal text colour (no theme colour) | 20 | 3.86 | 3: card, instrument_rack, table_layout_panel_compat |

## Other findings

- **Dark is light.** All 448 light cells at scale 1 hash the same as their
  dark cells: `windows-professional` has no dark variant, and the Gallery's
  `colour_scheme` presentation setting changes nothing. Phase 5 derives dark.
- **High contrast is incomplete.** Most class D and E runs are high
  contrast: controls that paint literal colours keep them while the theme's
  grounds turn black (`#1C2732` on black in cards and tab pages, 1.39:1).
  T4-1 drops authored colours in forced high contrast only on the part path,
  so the census controls are untouched until phase 4.
- **States that paint what rest paints** (light, scale 1): hot 58 of 64
  applicable, invalid 48 of 64, pressed 41 of 64, disabled 16 of 64, focus
  9 of 28, toggled 1 of 3. Hot changes only metrics view, grid, block and
  flow panels, choice group and card; buttons, check boxes, radio buttons,
  scroll bars and track bars show no hot cue (the Gallery's first button
  is style-overridden, which `ButtonBase` paints through its own frame
  rather than the theme). These are phase 4 inputs, per control in
  `manifest.tsv` (`same_as_rest`).
