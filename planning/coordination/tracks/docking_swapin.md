# Track: DockPanel Suite on GUI.Forms (SDR# swap-in docking)

Owner ruling (option a): our own clean-room code maps the DockPanel Suite API
onto GUI.Forms docking; never copy DockPanel Suite source. Coordinator ruling
(b), 2026-11-09: float windows stay shown while the app is inactive
(per-workspace option plus per-window override); implement the lost
behaviours in GUI.Forms docking. Only DockPane/FloatWindow as managed shells
and CloseButton=false placement on DockPanel=null may stay logged.

## RESUME POINTER (2026-11-09, evening)

- Library: D:/malkuth/probes/swapin_lib_dock/gui_forms, branch
  claude/dock-abi-docking. 94befba1b (all lost behaviours), then
  03bafb4ab (first focusable follows tab order), then 8765f019d and
  f1b5360e1 (the flyout takes the site's theme override and ambient
  colours). Conformance at 8765f019d: 1518 checks, 3 failed
  (the kit bugs fixed on claude/swapin-gaps-conformance-fixes @ cfe6537d3);
  every docking case passes. f1b5360e1 is spelling-checked only (owner
  plan 2026-11-09: everyone finishes coding, then one shared build).
  INTEGRATION_QUEUE names f1b5360e1. Not run: sanitizer, state-machine
  gate, docs regeneration.
- Swap-in commit 3ccb587 (dock shim, ScrollLayoutBridge.cs, run script,
  facade_hooks.py). Env-gated diagnostics remain in DockScript
  (SWAPIN_DOCK_REFRESH, the flyout up/down native dumps via NativeTree).
- Swap-in repo D:/malkuth/probes/swapin (shared with the swap-in worker):
  my files are dock_shim/, dev/dock/, facade_v1/System.Windows.Forms/
  DockPanelBridge.cs and ScrollLayoutBridge.cs. Three hook calls in
  Surface.g.cs (the swap-in worker's file) are applied in the working tree
  and by dev/dock/tools/facade_hooks.py in my snapshots; they must be folded
  into the swap-in worker's next Surface.g.cs commit.
- run_sdr_dock.sh snapshots the facade at HEAD (not 5a4951f7d any more),
  adds my two bridge files, applies the hooks, patches the sweep driver's
  TrackBar test by name and takes the swap-in worker's uncommitted
  BundledAssemblies.cs (the shared launcher names its startup hook).

## Done since the reboot

- Pane sizing (owner's top item). The facade docks children of an
  AutoScroll container in the area less the scroll bars it will show
  (ScrollableControl.__ScrollDockClientSize, bar 16 = the library's
  scrollbar_thickness_), and a docked child's native margin is cleared, so
  the library's scroll extent no longer counts it. SDR#'s Source table now
  takes the pane width less the vertical bar and tracks shrink and grow;
  no horizontal bar. TabIndex is forwarded to the library ("tab_index").
- Source opening scrolled to the bottom: DockSite's first_focusable walked
  child order (the bottom control first). Now tab order (03bafb4ab). In the
  test copy, with no device, the first selectable control in tab order is
  still the bottom Shift box, so activating Source still scrolls there, as
  WinForms would; with a device the Device SN combo is first.
- UI thread: every public shim entry point that reaches the library
  marshals with UiMarshal (UiThread.cs, named call records). The suite
  layout loads from a work thread in the script and lands correctly.
- DockPanel Suite XML read in full (Persistence.cs LoadSuiteFormat):
  panes, NestedPanes (PrevPane, Alignment, Proportion = the new pane's
  share), the edge windows from ZOrderIndex 0 outwards, float windows,
  auto-hidden panes, IsHidden, IsFloat, AutoHidePortion, ActiveContent,
  ActivePane. Show(prevPane, ...) and Show(pane, before) register a document
  beside or into a document pane.
- Plugin panels: SDR#'s View/Plugins items call MainForm.SwapDockState,
  which maps DockXAutoHide to DockX and Hidden to Float (measured by
  calling it for each state). So a hidden or never-shown panel opens
  floating with the real suite too; with a saved layout the reader places
  them. Not a shim defect.
- Flyout colours: the auto-hide flyout is an overlay root, so the form's
  BackColor/ForeColor (WinForms ambient colours; the facade sets inline
  colours, never a theme override) stopped at it and it drew light. It now
  takes the site's theme override and ambient colours; verified in SDR#:
  frame and caption dark.
- Verified by script on a private desktop: splitter drag (posted mouse
  messages, the left column grew by 60), caption drag onto a dock guide
  (Audio and Display moved left), auto-hide hover opens the flyout after
  the delay and leaving closes it (site flyout_changed events), window
  shrink and grow, View menu after close (floats, as SwapDockState says).

## Done in 94befba1b (library)

- HostWindowing::set_shown_while_inactive and
  ApplicationWindowHandle::set_shown_while_inactive. DockWorkspace has
  set_floats_shown_while_inactive and set_floating_window_shown_while_inactive
  (per site, kept through reopening), applied at open and in on_window_ready.
- on_window_ready requests the stored frame. request_frame was refused before
  the window was ready, so floats sat at the cascade position.
- attach_windows names the owner "main" (bf8dae1ed). Before that, every float
  was refused.
- DockAllowedPlacements has left/top/right/bottom, and DockNodePlace carries
  the side. DockLayout::side_refusal refuses a transaction that moves a panel
  onto a disallowed side or edge; a panel left where it is, is not refused.
- DockLayout::update_panel and DockWorkspace::update_panel: restrictions
  changed after registration.
- DockPanelDescriptor::close_button_visible hides the caption close button,
  its semantic node and the tab's close button.
- Workspace options:
  - document_tabs_shown (SDI);
  - auto_hide_opens_on_hover;
  - area_background, painted in place of the document-area well. This is a
    DockSite colour, not a theme override: deriving a Theme with one extra
    rule needs the theme compiler, which the ABI does not expose. Say so in
    the report.
- Windows host: at creation, a floating window takes its owner's DWM dark
  caption (attribute 20).
- C ABI:
  - GF_DOCK_ALLOWED_SIDE_* (none set means every side) and
    GF_DOCK_ALLOWED_CLOSE_BUTTON_HIDDEN;
  - slots 13 update_panel, 14 set_options (gf_dock_options, 16 bytes) and
    15 set_floating_window_visibility;
  - conformance part `restricting`;
  - regenerated C#/C++/manifest/probe.

## Done in 6264a08 (shim, unbuilt)

- DockingAbi.cs: the new constants, DockOptionsRecord and the three new slots.
- DockPanel.cs:
  - ApplyOptions: floats are always shown; DockingSdi; hover; DockBackColor
    when the program set it;
  - DescribePanel;
  - UpdateRegistration.
- DockContent.cs: AllowedFlags adds the side and close-button bits. The
  DockAreas, CloseButton, CloseButtonVisible, HideOnClose and
  PinButtonVisible setters call UpdateRegistration.

## Next items, in order

0. OPEN BUG (first after the shared build): the auto-hide flyout shows its
   frame and caption but not the content's children. In SDR#, AGC's
   controls have correct managed and native bounds inside the flyout
   (window y 554..673, flyout at 548) and are visible, and content.Refresh()
   does not paint them. Next: run with SWAPIN_DOCK_SCRIPT_LAST=15 and read
   the "flyout hover up/down" lines (native ancestry and subtree of the
   content) in dev/dock/logs/row_dock.log; suspects: the overlay root's
   paint not walking the moved view's page content (move_view_to_flyout /
   supply_content in dock_site.cpp), or the page not current after the
   programmatic select under syncing_.

1. Fold: the coordinator lands Surface.g.cs's three hooks with the swap-in
   worker (or applies facade_hooks.py), then the owner's session gets the
   pane sizing. The owner's session runs the REAL DockPanel Suite on the
   facade (run_sdr.sh); its auto-hide hover needs WinForms MouseHover in
   the facade, or the session switching to this shim (run_sdr_dock.sh),
   which has hover through the library.
2. Library: sanitizer and state-machine gate for 94befba1b..HEAD, docs
   regeneration (docs/api/generated, inventory.json, library.c-abi.rst).
3. Floats: check their theme (the float window's root may need the same
   theme inheritance as the flyout) and their placement in the shots.
4. Remove the script's diagnostics that are no longer needed
   (SWAPIN_DOCK_STACKS, scroll-layout trace) once the owner signs off.

## Decisions in force

- Panel ids are stable ids (StableIdOf(PersistString), "-N" for uniqueness).
- A refused program CLOSE (reason 18) falls back to HIDE, and the content is
  marked detached (it reads Unknown).
- Layout files use our format "gui-forms-1". The DockPanel Suite format is
  also read (being completed, item 5).
- Output goes only into out/dock_*. Commit only my files, by path.
- Scratch scripts are in the session scratchpad's dockw/ folder; edit.py is
  the exact-replacement helper.

## Running

- SDR#: from D:/malkuth/probes/swapin, run
  `DOCK_FAST=1 SWAPIN_DOCK_SCRIPT=1 SWAPIN_DOCK_TRACE=1 bash dev/dock/tools/run_sdr_dock.sh`.
  - SWAPIN_INTERACTIVE_DESKTOP=1 runs it on the owner's desktop (authorized
    for SDR#).
  - SWAPIN_DOCK_SCRIPT_FOREGROUND=1 asks for the foreground. It did not get
    it: this cloud PC had no foreground window.
- Library: run dev/dock/tools/build_dock_lib.sh in the CLANG64 shell
  (DOCK_TESTS=ON, EXTRA_TARGETS=gui_forms_abi_conformance). After a header
  change, run dev/dock/tools/regen_abi.sh first.
- Conformance: run `run_on_desktop.py -- .\gui_forms_abi_conformance.exe` in
  .build/native-windows-x64/gui-forms, with clang64\bin on PATH.

## Still logged as lost (allowed)

- DockPane and FloatWindow are managed shells.
- Contents with CloseButton=false stay placed when DockPanel is set to null.
- DockPanel.Theme's palette: the GUI.Forms theme draws the chrome.
- DocumentStyle DockingWindow and SystemMdi dock as DockingMdi.
