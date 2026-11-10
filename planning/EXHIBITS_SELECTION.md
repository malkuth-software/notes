# Exhibits selection for screenshots (exhibits worker, 2026-11-09)

Proposal for the owner to pick from. Sources: EXHIBITS_PLAN.md (16 exhibits)
and the template examples (GUI.Forms `examples/reference`, template_harness
`example_app`). Each program lives in its own folder under
`D:/malkuth/workspaces/exhibits` (one local git repository, no remote),
consumes GUI.Forms as an installed lockstep SDK through public API only, has a
deterministic fixture, a `--screenshots <dir>` option, and captures light and
dark at 1x and 2x (`exhibit_<name>_<variant>_<scale>x.png`).

"Mockup": looks real, fake data, minimal logic. "Working": real behaviour.

| # | Exhibit | Kind | Shows | Status |
|---|---|---|---|---|
| E2 | Oscilloscope and signal analyser: four channels, FFT pane, trigger, cursors, measurement table | mockup | custom painting, dense engineering UI, animation | obviously in; coding |
| E4 | Industrial process control: pumps, valves, tanks, alarms, trend chart (C++) | interactive mockup | custom controls, alarms list, skeudoindustrial look | obviously in; coding |
| E9 | Data-entry form: insurance claim, tabs, masked boxes, date pickers, combo boxes, validation | mockup | the forms audience, tab order, validation | obviously in; coding |
| E14 | Wizard: multi-step claim/installer dialog with step rail | mockup | dialogs, page transitions | obviously in; coding |
| E1 | Orders · Central: inventory and orders, master-detail grid, filters, edit dialog | mockup | data grid, list views, the site hero | recommended |
| E8 | Registry-editor-style tree browser: tree plus value list in a split container | mockup | tree/list views, splitters | recommended |
| E15 | PropertyGrid-driven settings screen | mockup | object view / property editing | recommended |
| E13 | MDI parent with child windows | mockup | docking / child windows | optional (depends on MDI support; see gaps) |
| T1 | Reference `collection_gallery` (GUI.Forms' complete reference application) | working | the recommended form-document workflow | optional, cheap |
| T2 | template_harness `example_app` greeter | working | the minimal SDK consumer | optional, cheap |

## Owner decision (2026-11-09, via the coordinator)

Approved: E2, E4, E9, E14, E1, E8, E15, T1, T2. E13 (MDI) waits. Kinds
follow EXHIBITS_PLAN.md exactly; only its "(mockup)" items may be mockups:

| # | Kind (final) | What that means here |
|---|---|---|
| E2 oscilloscope | mockup | synthetic waveforms, controls move the display only |
| E8 registry-style browser | mockup | fixed in-memory tree and values |
| E9 insurance-claim form | mockup | validation runs, nothing is filed |
| E4 process control | interactive mockup (C++) | a simulated factory you operate: pumps, valves, tanks, alarms and trends respond to input |
| E1 Orders · Central | working, download link | real records persisted to a data file, master-detail, filters and search, edit with validation, invoices with print preview, the ribbon |
| E14 wizard | working | a real multi-step dialog with animated transitions that produces its result |
| E15 settings screen | working | edits real settings that persist to a settings file |
| T1 collection_gallery | working | GUI.Forms' reference application |
| T2 harness greeter | working | the minimal SDK consumer |

**Scope now (owner, 2026-11-09): mockups only.** Building E2, E8, E9 and E4.
E1 Orders · Central, E14 wizard, E15 settings, T1 and T2: **later (working
apps)**.

Screenshot runs use a deterministic fixture (a fixed data set written to a
temporary data directory), so persistence never changes a screenshot.

Original recommendation: E2, E4, E9, E14, E1, E8, E15 (seven), plus T1 if the site
wants a "working" row. E3 (accounting) and E11 (Process Explorer) overlap E1
and E8 visually; E5/E6/E10 are existing products with their own screenshots;
E7, E12, E16 are later by the plan.

## Gaps found in GUI.Forms

(Filled in while building; each gap is noted, not patched.)

Built against the private cp25 install `gui-forms-ffbdc2a77-private-windows-x64`
(the active lockstep record `ec23f9d8` is stale; main is 27 commits ahead,
including cp26). Rebuild the exhibits when the lockstep SDK advances; some
gaps below may already be closed on main.

1. **Ancestor snapshots drop GroupBox and DateTimePicker.** `Window::snapshot_control`
   of the root omits them, though each paints when snapshotted alone. The
   exhibits capture them separately and composite (`ShotTarget::separately_captured`);
   the claim form's group boxes are childless frames with sibling radios.
2. **Snapshots draw theme part rasters only at the window's own scale.** A 1x
   capture on a 2x desktop loses tab faces and other part rasters; the runner
   calls `Window::set_scale` before each capture.
3. **ErrorProvider icons are absent from subtree snapshots.**
4. **No public query for the theme's window backplane colour.** Snapshots
   exclude the backplane, so the exhibits composite over their own palette colour.
5. **MaskedTextBox text is a method (`set_text`/`text`), not a property
   capability** like every other control's `text`.
6. **Docking runs in child order** (first child docks first), unlike WinForms'
   reverse z-order. A real divergence for ported code; document it.
7. **Painter has no polygon or path fill.** Triangles (valves, pump arrows) are
   drawn as scanline strokes.
8. **No industrial/skeuomorphic style in the SDK.** E4 draws its own metal
   tanks, plates and lamps in a custom control.
9. **Menu command and check items require a `Command` object**, even in a
   mockup with no command routing.
10. **No stock icon set.** Tree/list icons (folders, registry value kinds) need
    PNG assets in an image list; the exhibits go without.
11. **DateTimePicker `long_date`/`short_date` follow US format** with no culture
    setting visible to the consumer (the claim form is British).
12. **Label has no wrapping in the exhibits' use**: the E4 hint is two labels.
