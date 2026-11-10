# SDR# owner session, 2026-11-09

The owner ran the swap-in SDR# on the interactive desktop from 05:22 to 05:25:45 (launcher log
`dev/logs/row_sdr_desktop.log`, capture log `out/work_sdr_desktop/capture.jsonl`). They closed it
normally, with exit code 0. Their verdict: most of SDR# is docking; mostly nothing works; submenus
don't open; popups (the AIRSPY server browser) are missing; "most things I couldn't hit, because I
couldn't open anything".

## What the running instance recorded, and what it did not

The capture log records each distinct event once (first hit, with a sequence number but no
timestamp). It records unmapped members, bridge gaps (including callback exceptions with their
stack), lost behaviours and load failures. It did **not** record menu opens or submenu requests,
it did not record forms opened, and it did not take periodic window captures. So per-gap hit counts
are not available from this session. Fixed for later runs: see the sweep below.

## Gaps seen in the session (first hits)

| # | Gap | Evidence | Owner |
|---|-----|----------|-------|
| 1 | The server browser (AIRSPY "browse" popup) fails to open: creating `HtmlLinkLabel` (via `MapControl`) throws "the control has no property of that name" in the `NativeControlBridge` constructor | capture seq 554, stack `ControllerPanel.browseButton_Click` -> `ServerBrowser..ctor` | facade |
| 2 | Submenus do not open (owner) | not logged; reproduced by the sweep | facade |
| 3 | Hosted/overlay attach is unported: `AttachOverlay` (used when a non-top-level form is projected into a host, and by overlays) | capture seq 566 | facade + GUI.Forms library (overlays table) |
| 4 | DockPanel: pane chrome, splitters, drag, float windows, auto-hide never receive messages (WndProc, NativeWindow subclassing, hooks) | lost_behaviour seq 299, 465-470, 557; preflight | DockPanel worker |
| 5 | Custom-painted combo, numeric and button controls keep the stock look | seq 58, 64, 125 | facade (paint endpoint) |
| 6 | Combo editing carries no caret, selection or undo | seq 219 | GUI.Forms library (branch claude/swapin-gaps-combo-editing, unfolded) |
| 7 | Splash made on a thread-pool thread is parked, not shown | seq 4 | facade (decision pending) |
| 8 | Field height fixed at 23, not measured from the font | seq 240 | facade |
| 9 | Per-control cursor kinds not mapped | seq 222 | facade |
| 10 | Focus refused on panel, container and user controls | seq 480, 498, 507 | facade |
| 11 | `ContentAlignment` on check boxes has no counterpart | seq 352 | GUI.Forms library |
| 12 | `XmlSerializers` probe and `spybrowser.history` not found (expected, harmless) | seq 431, 432, 549 | none |

## DockPanel split (for the DockPanel worker)

**There is no DockPanel shim in the facade today.** SDR# ships the real WeifenLuo DockPanel Suite
assemblies (`WeifenLuo.WinFormsUI.Docking`, `WeifenLuo.WinFormsUI.Docking.ThemeVS2015`), and they
run unmodified on top of the facade's `System.Windows.Forms`. Their controls derive from
`Control`, `Panel`, `UserControl`, `Form` and `NativeWindow`, so they are bridged like any other
custom control: `DockPane` and the strips paint into raster or paint-endpoint surfaces, and
`DockContent` forms are projected into their host as non-top-level forms.

Facade code that DockPanel currently runs through (all shared with other programs):

- `facade_v1/System.Windows.Forms/Surface.g.cs`: `Form` hosted projection (`__ProjectHostedRoot`,
  `__ownerForm`, `TopLevel = false`), `NativeWindow`, `WndProc`, `CreateParams`, `Control`.
- `facade_v1/System.Windows.Forms/StableBridge.cs`: `AttachOverlay` (unported), raster and paint
  endpoint bridging of custom controls.
- `facade_v1/System.Windows.Forms/ToolStripRendering.cs`: draws through
  `VisualStudioToolStripRenderer` (the DockPanel theme's tool strip renderer).
- `facade_v1/System.Windows.Forms/Win32Routing.cs`: routes program P/Invokes to the compat shim.
  The DockPanel assembly is not routed (its imports are not all covered).

Proposal: the DockPanel worker owns a new file set,
`facade_v1/System.Windows.Forms/DockPanel*.cs` (or a substitute WeifenLuo assembly built and
substituted by the launcher), and the hosted-form projection code in `Surface.g.cs` once the
split is agreed. Until the coordinator confirms the split, I edit none of the four files above in
their DockPanel-specific paths (`__ProjectHostedRoot`, `AttachOverlay`, `ToolStripRendering.cs`).

What the DockPanel Suite uses that is lost (preflight `lost_behaviours`, component DockPanel Suite):

- drag: `DragDetect`, `DragHandlerBase.WndProc`, `DragForm.WndProc` and its `CreateParams`,
  `NativeWindow` subclassing in `DockDragHandler`, `DragHandler`, `DragHandlerBase`;
- float: `FloatWindow.WndProc`;
- focus hook: `SetWindowsHookExW` (never sees facade controls' messages);
- MDI client: `MdiClientController` (WndProc, NativeWindow);
- pane chrome: `DockPane.WndProc`, `DockPaneCaptionBase.WndProc`, `DockPaneStripBase.WndProc`;
- splitter: `SplitterBase.WndProc`, `SplitterDragHandler`;
- other: `PostMessageW`, `SendMessageW`, `WindowFromPoint`, `SetWindowLongW`, `SetWindowPos`,
  `ShowScrollBar` on control handles, all with no effect.

DockPanel types deriving from WinForms types: `dev/logs/dock_derived.txt` (43 types; `DockPanel`,
`DockPane`, `DockContent`, `DockWindow`, `FloatWindow`, `DragForm`, the VS2012/2013/2015 theme
strips, captions, splitters and indicators).

What SDR# calls on the DockPanel Suite (`DockPanel`, `DockContent.Show(DockPanel, DockState)`,
`LoadFromXml`/`SaveAsXml` layout persistence, themes) is not recorded by the preflight, which
lists only references into substituted assemblies. The DockPanel worker should add a member
reference dump of SDR#'s assemblies into WeifenLuo (the bundle is extracted under
`out/work_*/private/` during a run).

## Coverage sweep

The in-process sweep driver (facade_v1/System.Windows.Forms/SweepDriver.cs, `SWAPIN_SWEEP=<file>`,
run by `dev/tools/run_sweep.sh` on a private desktop) clicks every input control of the main form
physically (posted mouse messages at the library's laid-out bounds, scrolled into view), opens every
tool strip item, the main menu and its submenus (every leaf command), every context menu, and makes
and shows every form type of the SDR# assemblies that has a parameterless constructor. Pass means it
opened (or toggled, or selected) with no exception and no refused call. Results:
out/sweep_sweep.tsv; log out/work_sweep/capture.jsonl; summary `python -I dev/tools/sweep_summary.py
out/sweep_sweep.tsv [--markdown]`.

Progress (pass of total): 92/115 (first sweep) -> 116/123 -> 122/123 -> **123/123** (final).

| kind | pass | fail | skip |
|------|-----:|-----:|-----:|
| button | 5 | 0 | 0 |
| check_box | 7 | 0 | 0 |
| combo_box | 3 | 0 | 0 |
| custom | 15 | 0 | 0 |
| form | 4 | 0 | 0 |
| menu | 76 | 0 | 0 |
| numeric_up_down | 2 | 0 | 0 |
| radio_button | 11 | 0 | 0 |
| **all** | **123** | **0** | **0** |

Fixed on the way: re-entrant Show of a presented form (FloatWindow.OnLayout; stack overflow through
Visible), Dispose made idempotent (DockPane double dispose), docked content closed in place, the
library's show_dialog request id (every message box and common dialog was refused), check boxes never
read the native state after a click (an enumeration property read without room for its name, so
clicks in SDR# changed nothing), stale native events after a form disposed itself (the splash).

Outside the matrix (docking, routed to the DockPanel worker): plugin panels open as FloatWindows,
DockPanel Suite controls are made off the UI thread during layout loads (marshalled, recorded as
gaps). Remaining non-docking gaps by first-hit count: WndProc overrides (custom-painted borders),
Focus refused on panels and user controls, combo text editing, cursor kinds.
