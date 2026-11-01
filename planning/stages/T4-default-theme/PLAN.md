# Stage T4 plan: the default theme and Windows-familiar themes

Date: 2026-11-01. Status: accepted (owner, 2026-11-01: T4-1 to T4-4 as
recommended); phase 1 built, phases 2 and 3 in progress. Authority: the stage brief (`README.md`),
`RENDERING_REFERENCE.md` and `planning/DESIGN_LANGUAGE_SPECIFICATION.md`.
Base: T3 complete (checkpoint 2026-11-01-4: P11 and user themes).

## Where the engine stands

- `paint_part` already hands every part to the theme's engine (T2). A part
  no rule selects is drawn by the built-in recipe: the
  `ControlVisualRecipe` records of `windows_professional_theme_definition()`
  (`src/core/theme/theme/theme.cpp`), one per family (`ControlVisualRole`,
  14 families) and surface state, with hand-authored high-contrast arrays.
- The engine's own rules (`built_in_rules.cpp`) already draw the ribbon's
  dithered bulge with its rise, the docking tiles' plates and the overlay
  shadow.
- T3 compiles theme packages and user themes, colour roles (§5.4's set),
  `@media (prefers-color-scheme)`, custom properties and recipe fields. A
  theme made from a definition keeps an empty sheet, where T4's default
  sheets go (T3 P12b). No host reports the system colour scheme yet.
- 51 files under `src/controls/` still call `Painter` drawing methods or
  `paint_surface_material` directly (phase 1's census gives the exact
  count); 34 call `paint_part`.

## Phases

Each phase lands on its own through the checkpoint path, and each gate is a
measurement recorded under `results/`.

| Phase | Work | Gate (measured) |
|---|---|---|
| 1. Instruments and the style mapping | `STYLE_MAPPING.md` (DataGridView cell styles and control colours, decision T4-1); the paint census (`tools/paint_census.py`, a baseline manifest and a ratchet check); the contrast report (§5.7 pairs for every family, state, selection and presentation of a theme, from its recipes) | census baseline recorded (files and calls, stock and exempt); contrast report of `windows-professional` recorded (rows, failures by pair); tests plain and under the sanitizer |
| 2. Specimen sheet | every stock control of the Gallery driven through its published states (rest, hot, pressed, keyboard focus, checked or selected, disabled, invalid) in light, dark and high contrast at scales 1 and 2, written as contact sheets with a hash manifest; the contrast measured in pixels where text sits | sheet of `windows-professional`: controls times states captured; two runs byte-identical on one host; pixel and recipe contrast agree, or each difference listed |
| 3. The default theme as theme data (light) | the default's sheets in the theme language, compiled into the built-in theme: colour roles from Sapphire Dusk and Plan Paint (sampled at `6b901fc`, never approximated), families (action plate, recessed wells, near-white stock, gold active tool, flat pale status, dotted focus ring, default perimeter); engine primitives only where the vocabulary has no drawing (Plan Paint's tab stack, etched dividers fading at both ends, the work pane's felt surround) | zero diagnostics; startup compile time and work counters with a stated bound; byte round trip and raster reference hashes (T3 P11 items 5 and 2a); contrast passes for light; each new primitive against Plan Paint's own pixels (largest channel difference); first owner look at the light sheet |
| 4. Stock controls onto the engine | in batches: buttons and choices; range controls; lists, trees, combos and grids (with DataGridView's marks); menus and strips; text boxes and rich text; containers and panels; inspection and metrics views. `BasicControlStyle` and literal colours become recipes, colour roles or inline styles | per batch: the census for its files at zero stock calls; touched and Gallery-wide tests plain and sanitized; Gallery trace differences under `windows-professional` listed and explained. At the end: zero stock calls, enforced by the ratchet |
| 5. Derived variants | dark derived by rule (Phosphor pair, the light upper left, inset stays inset) under `prefers-color-scheme`; GUI.Forms' own high contrast derived from the role model, replacing the hand-authored arrays; the host's palette where a host reports one (a capability, `unsupported` elsewhere) | contrast passes for dark (4.5 and 3) and high contrast (7); dark and high-contrast sheets |
| 6. The switch | `default_theme()` becomes the new default; `windows-professional` stays, as a Windows-familiar theme; demonstration programs unchanged | Gallery traces updated with a recorded difference count; the owner approves the light, dark and high-contrast reference set |
| 7. Windows-familiar themes | Classic (procedural bevels) and modern flat (M076, high-colour icons) first, then XP-style and Windows 7-style (glass with T5), recreated, never Microsoft's bitmaps | per theme: contrast passes, specimen sheet, owner approval; SDRSharp fixtures (R) where they exist |
| 8. Stage gate | the record: sheets, contrast reports, census at zero, approvals | `results/<date>-t4-gate` |

Phases 2 and 3 can run side by side once phase 1 is in; phase 4's batches
can start after phase 2 and run beside phase 3.

## Decisions

Owner decisions (all accepted as recommended, 2026-11-01; design language §12.14):

- **T4-1** How application colours reach a part (DataGridView cell styles):
  `STYLE_MAPPING.md`, options 1 to 4; recommended option 1, authored colours
  on `PartPaint` at the inline cascade level.
- **T4-2** The default theme's id (it can no longer be
  `windows-professional`, which stays as a Windows-familiar theme).
  Recommended: `gui-forms`.
- **T4-3** How the default ships: its sheets compiled at first use
  (recommended, with phase 3's measured startup bound) or generated into C++
  data at build time.
- **T4-4** Which controls draw content rather than chrome and so are exempt
  from the census. Accepted: `DrawingSurface`, `RasterCanvas`,
  `EasingPreview`, `TypographySpecimen`, `MetricsView` and `PictureBox`;
  only their frames and backgrounds go through the theme.

Dependencies outside T4 (for the coordinator):

- The skeudoindustrial plan runs after T4 (owner, 2026-11-01) and does not
  gate 0.5.
- Host colour-scheme reporting (P8a, opt-in per the owner's 2026-10-28
  direction) is host work, needed before phase 5's dark variant can follow
  the host.
- Skia is on for every host (stx_skia in-tree; Windows since #427), so
  phase 2's specimen sheets are compared across hosts where CI captures
  them.
- `windows-professional`'s accent-button plates failed its own text
  contrast (1.0 to 1.3:1); fixed on a separate branch, since the theme
  ships as a Windows-familiar theme.
