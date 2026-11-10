# Open gaps

Date: 2026-11-10. One list of every open gap found in the last two days, so
none lives only in a chat, a scratch file or a worker report. Each row names
its owner (the worker or role that closes it), its source and its status.
`planning/ROADMAP_TO_1.0.md` places the larger items in blocks; this page is
the checklist. When a gap closes, mark it here with the commit or PR and
leave the row until the next checkpoint's record refresh removes it.

"cp27" is PR #480 (checkpoint/2026-11-10-cp27 @ 757db03a7, open on
2026-11-10). "Combined tree" is the integrator's fold/bleeding-edge.

## Text and fonts

| # | Gap | Owner | Source | Status |
|---|---|---|---|---|
| F1 | Font family does not cross the C ABI or reach C#: the C ABI font record has no family, so a round trip drops a theme's `font-family: Selawik` | C ABI (next ABI additions, row 3.0) | queue, `claude/professional-theme` 114bddcda risks | open |
| F2 | Prepared text ignores the family: the prepared-text font bank and the Skia executor's engines use the role face only | engine (T2/F) | same | open |
| F3 | Products and template_harness stage their own fixed font lists without Selawik, so windows-professional falls back to Carlito silently | product workers, template_harness | same | open; fix when the products repin |
| F4 | Font face identity: `FontSpec` has a role and no face (audit D8); Stage F and the Windows Forms `Font` need one before 1.0 | owner decision, then T2 | `planning/audits/AUDIT_TRIAGE_2026-11-10.md` D8 | open, needs a ruling |
| F5 | The control face renders "a" closed into "o" at 12.5 units at 1x ("macOS" reads "mocOS"): small-size rendering or hinting of the Portsmouth control face | engine (F) | exhibits worker, STATE 2026-11-10 | open |

## Painting and hosts

| # | Gap | Owner | Source | Status |
|---|---|---|---|---|
| P1 | Scroll copies existed only on macOS; Windows (default raster profile) and the Linux X11 host now apply them | canvas-pan worker | PlaySuite gap; `claude/canvas-pan-perf` 699055b6a | coded, in cp27; Linux never compiled locally; check on Linux and macOS (the bar-ink change touches macOS) |
| P2 | ThreadSanitizer run of the ABI sub-surface tests (audit N9: the event stack is thread_local now) | Mac / Linux | Mac handoff item 5 | open |
| P3 | DIB fixture: on a private desktop the fixture's later step fails with "initial Window receipt not accepted" (transaction-frame-not-presented) | integrator | `claude/automation-off-by-default` risks; w6 | open: check against main at build time |
| P4 | Menu repaint scope: `ribbon_tests` menus_repaint_only_their_command failed on cp27 (with `link_router_tests`) | integrator | STATE 2026-11-10 | being fixed on top of cp27 |
| P5 | PlaySuite tile hover blink: unresolved after the card cache was reverted; "on the old version we had that fixed" | owner, later | tracks/playsuite.md; STATE 2026-11-10 | parked by the owner |
| P6 | Invalidation overlay compositing on macOS and Linux never compiled | w4 | `claude/invalidation-overlay` 4fa3dfaa | in cp27; Mac/Linux build checks it |
| P7 | Splash baking: tooltip prebake records outside the window's recording; a theme change between prebake and show can blit the old theme once; macOS/Linux edits uncompiled | w1 | `claude/splash-baking` 703d7ac87 | in cp27 |

## Themes and look

| # | Gap | Owner | Source | Status |
|---|---|---|---|---|
| T1 | Forced high contrast: a gauge's value arc and its track share the edge colour (the needle carries the reading) | final-controls / theme | `claude/final-controls` e5e4e2d6b risks | open: needs a design call |
| T2 | windows-professional redesign (owner direction 2026-11-10) not in cp27; captures for review owed | w1 | `claude/professional-theme` 114bddcda, follow-up `claude/be-professional-theme` 96401f3cd | coded, unbuilt; the site is recaptured after it |
| T3 | Look ledger leftovers: T1 default base partial (overtaken by cp24), T13 partial, T12 not audited; R17 minimized-ribbon popup and R24 QAT on the tab row (ruled a follow-up RB item, N3) | w1 / RB | `planning/stages/T4-default-theme/LOOK_INTEGRATION_LEDGER.md` | open |
| T4 | The Ribbon's help "?" part is reserved, not drawn or wired (ruling N4: with the help system) | RB + help | ledger R23 | open |

## Help system (rows 2.13-2.20)

| # | Gap | Owner | Source | Status |
|---|---|---|---|---|
| H1 | No language-file system exists, so `HelpCatalog` stores only the help fields | help worker / translation design | `claude/help-system` ecf4167f4 | open |
| H2 | Search folds ASCII only | help worker | same | open |
| H3 | No help cursor for Shift+F1; no document or heading semantic role | help worker, A | same | open |
| H4 | Device printing deferred (no print path); the navigation pane is not drag-resizable (`set_navigation_width` only) | help worker | same | open |
| H5 | Help-book page captures owed (`--page help-book --capture`) | help worker | same | open |

## Final controls (row 2.5)

| # | Gap | Owner | Source | Status |
|---|---|---|---|---|
| W1 | Deferred: W-1 public bases (see audit F1), W-2 DataGridView breadth, W-3 MDI (with DK), W-7 menu first-letter, W-8 FileDialog settings; W-6 Diagram/DataLayout have their own stages | final-controls worker | `claude/final-controls` | open |

## Exhibits (row 4.9; built against the cp25 install)

| # | Gap | Owner | Status |
|---|---|---|---|
| E1 | Ancestor snapshots drop GroupBox and DateTimePicker | core | open |
| E2 | Snapshots draw theme part rasters only at the window's own scale | core | open |
| E3 | ErrorProvider icons absent from subtree snapshots | core | open |
| E4 | No public query for the theme's window backplane colour | core | open |
| E5 | MaskedTextBox `text` is a method pair, not a property capability | controls | open |
| E6 | Docking runs in child order, unlike WinForms' reverse z-order (document the divergence) | docs / layout | open |
| E7 | Painter has no polygon or path fill | drawing | open (check the drawing primitives in cp27) |
| E8 | No industrial/skeuomorphic style in the SDK | theme | open |
| E9 | Menu command and check items require a `Command` object | controls | open |
| E10 | No stock icon set | resources | open |
| E11 | DateTimePicker long/short date follow US format; no culture setting visible to consumers | controls | open |
| E12 | Label wrapping not usable as the exhibits need it | controls | open |

Source: `planning/EXHIBITS_SELECTION.md` "Gaps found in GUI.Forms". Some may
be closed on main; re-check when the exhibits rebuild against the lockstep
SDK. The site images (windows-professional) are recaptured then too.

## SDR# swap-in (row 5.5)

| # | Gap | Owner | Status |
|---|---|---|---|
| S1 | Server browser (AIRSPY "browse") fails: `HtmlLinkLabel` via `MapControl` throws in the bridge constructor | swap-in worker (facade) | open |
| S2 | Submenus do not open | facade (menu system in cp27 may cover the library side) | re-check after cp27 |
| S3 | Hosted/overlay attach (`AttachOverlay`) unported | facade + library overlays table | open |
| S4 | Custom-painted combo, numeric and button controls keep the stock look; by ruling, no WndProc/WM_PAINT emulation: the sdrsharp theme supplies the look | facade + theme | partly done (theme field rules) |
| S5 | Combo editing carries no caret, selection or undo | library (`claude/swapin-gaps-combo-editing`, unfolded) | open |
| S6 | Splash made on a thread-pool thread is parked | facade; decision pending | open |
| S7 | Field height fixed at 23, not font-measured; cursor kinds unmapped; Focus refused on containers; check box `ContentAlignment` | facade / library | open |
| S8 | Open from the second coding pass: volume "0" label, SDR# picker forms, GroupBox "Smoothing" frame, AutoScale, combo ForeColor, caption text #d0d0d0, live-signal check of re-fit and "spazzing" on the owner's desktop | swap-in worker | verify in the combined build |
| S9 | The swap-in stubs tool crashes (null reference) on non-framework interfaces, so dock stubs are generated without the shim | swap-in worker | open |
| S10 | Release zip for Youssef Touil: packaging script ready; built after the combined build; the owner tests it first | swap-in worker | pending the build |

Sources: `planning/stages/S-csharp-swap-in/SDRSHARP_SESSION_2026-11-09.md`
and `SWAPIN_WORKER_STATE_2026-11-10.md` (the swap-in repository is local
only, so these are its copies).

## Docking for the swap-in (DockPanel worker)

| # | Gap | Status |
|---|---|---|
| K1 | Open bug: the auto-hide flyout shows its frame and caption but not the content's children (AGC in SDR#) | first after the shared build |
| K2 | Floating tool windows are hidden whenever the application is not foreground (never shown on a private desktop); DockPanel Suite keeps them visible; their requested frame is not applied while hidden | open |
| K3 | Three hooks in the facade's `Surface.g.cs` are applied in a working tree, uncommitted (`dev/dock/tools/facade_hooks.py`) | to land with the swap-in worker |
| K4 | Library: sanitizer and state-machine gate for 94befba1b..f1b5360e1; docs regeneration | integrator build |
| K5 | Lost behaviours, allowed and logged: DockPane and FloatWindow are managed shells; contents with CloseButton=false stay placed when DockPanel is set to null; DockPanel.Theme's palette is replaced by the GUI.Forms theme; DocumentStyle DockingWindow and SystemMdi dock as DockingMdi | accepted as lost |

Source: `planning/coordination/tracks/docking_swapin.md`.

## Build, CI and records

| # | Gap | Owner | Status |
|---|---|---|---|
| B1 | CI area selection (#479) does not yet classify a tools-only change as its own area (named by the coordinator 2026-11-10; no written record found beyond that) | CI worker | open |
| B2 | Release notes: `docs/release/unreleased.md` does not list cp27's work; see the cp27 docs follow-up list in projects STATE.md | docs | open |
| B3 | Plan Paint: 22 language packs carry machine translations of 2 new strings; a native-speaker check is owed | Plan Paint / owner | open |
| B4 | Product pins move to the post-cp27 main (Plan Paint -look and combined-build, PlaySuite, SwiftEdit, theme editor) | product workers | after cp27 merges |
| B5 | Audit batches not yet ruled (see `planning/audits/README.md`) | owner | open |
