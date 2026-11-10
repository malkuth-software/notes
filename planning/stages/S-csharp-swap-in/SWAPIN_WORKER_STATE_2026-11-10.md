# WinForms swap-in launcher (private tools, licence undecided)

Local git only; never pushed. First real target: SDR# (not run yet: needs
the owner's authorization and a sandbox). Study:
`D:/malkuth/projects/desks/coordinator/MIRRORED_MODE_STUDY.md`; readiness:
`SDRSHARP_SWAPIN_READINESS.md`; P0: `D:/malkuth/probes/mirrored_p0/RESULT.md`
(the bundle row b1 is there).

Owner rulings in force: real native window for top-level `Form.Handle`
only, children on leases; non-PNG images through launcher-private WIC;
.NET 10; DockPanel Suite option a first (its own code on our facade, drag,
float and focus hook lost and **logged by name**), a shim over our docking
later.

## Parts

| Part | Path | Role |
|---|---|---|
| Launcher | `launcher/swapin_launcher.cpp`, `single_file_bundle.{h,cpp}` | `run` (parent: preflight, bundle extraction, child, summary) and `host` (child: hostfxr, exact-name TPA substitution, private desktop) |
| Managed tool | `tools/Preflight/` (`swapin.dll`) | `preflight`, `stubs`, `summarize`; metadata only |
| Capture | `capture/` (`GuiForms.Swapin.Capture.dll`) | JSON Lines capture, first-hit records, counters, lost behaviours; lives in the facade directory |
| P1 fixtures | `tests/p1app`, `tests/mapcontrol`, `tests/facade_p1` | program with missing members, overrides, NativeWindow, hook P/Invoke, an app assembly named `System.Windows.Forms.MapControl`; a partial-types test facade |

## Rules

- **Substitution by exact name.** Substituted set = simple names of the facade
  directory's assemblies (minus `GuiForms.Swapin.Capture`, `GuiForms.Interop`).
  The whole Windows Desktop framework leaves the TPA (its folder), any other
  entry with a facade file name is removed, facade assemblies are added; the
  host refuses unless each facade name appears exactly once, from the facade
  folder. No prefix rule (MapControl stays: `kept_by_exact_name`).
- **Preflight** refuses WPF references, compressed bundles, missing main
  assembly, apphost without its .dll; R2R verdict per assembly (calibrated
  P0 rule) sets `DOTNET_ReadyToRun=0` for `mvid_fail_fast` images.
- **Stubs**: for each referenced member or type the facade lacks, C# with the
  exact metadata identity (shapes from the reference pack), body = counter
  increment + disposition (noop, default, throw; `--dispositions` TSV
  overrides). Facade types must be partial. Missing interfaces and enums are
  declared in full. Not emitted (reported): generic types and methods,
  operators, explicit implementations, members of enums or interfaces that
  already exist in the facade, accessors of a property the facade already
  declares.
- **Capture** (`SWAPIN_CAPTURE_LOG` runtime property): `stub_hit`,
  `supported_hit`, `override_present`, `lost_behaviour`, `load_failure`,
  `exception`, `counts`, `loaded_assemblies`. Hot path: one interlocked
  increment; only first hits write (flushed). Not yet: ring buffer, writer
  thread, pipe (the parent reads the file after the child exits).
- **Summary**: deterministic (sorted, no timestamps, no paths); lost
  behaviours grouped by component, each with area (drag, float, focus hook,
  splitter, auto-hide, mdi client, pane chrome, other), static and observed
  flags.

## Commands

```text
bash dev/tools/run_p1.sh     # builds everything offline, generates stubs, runs rows p1a, p1a_again, p0_bundle, p0_bubble
```

## State (2026-11-09)

- P1 rows pass: P1App (apphost) with 17 stubbed members, 10 hit, all lost
  behaviours listed; P0 bundle through the launcher; P0 input-bubble R2R
  image runs with R2R off; summary byte-identical across two runs; program
  files unchanged; no Windows Desktop assembly loaded.
- Preflight over the SDR# extract (as data): 933 substituted member
  references (matches the readiness count), DockPanel Suite lost-behaviour
  list of 22 items by area.
- Step 3 done: `facade_v1/` (facade-v1 and GuiForms.Interop from gui_forms
  d896d01a1, cp22, see `facade_v1/SOURCE.txt`) runs on the real library
  (`gui_forms_abi1.dll` from `D:/gui_forms_int/.build/native-windows-x64`).
  `NativeBridge.g.cs` (v0 table, `gf_get_api_v0`) is replaced by
  `System.Windows.Forms/StableBridge.cs` over the stable tables; Surface.g.cs
  is unchanged except two hooks (supported hits to the capture, control-type
  observation). Members the port does not carry yet are `bridge_gap`
  records in the capture and `bridge_gaps` in the summary (the largest:
  managed painting, i.e. custom control + windows paint endpoint; then
  overlays, property grid, scrolling, dialogs, tooltips, pointer and key
  ingress, range, alignment and style mappings).
- `bash dev/tools/run_v1.sh`: stubs per program against facade-v1 (P0: 1,
  `Application.EnableVisualStyles`; SDR#: ToolboxBitmapAttribute and its two
  constructors, AccessibleRole, ComboBox.ObjectCollection.CopyTo,
  DataGridViewDataErrorContexts, IDataGridViewEditingControl,
  IMessageFilter.PreFilterMessage (added to the partial facade interface),
  TrackBar, VisualStyleElement; cp22's facade already had UITypeEditor()
  and DropDownControl), one runtime facade per program (the SDR# one builds,
  is not run), and row `v1_p0`: the P0 bundle on the private desktop, "Greet"
  clicked through posted mouse messages, the label's native text reads
  "Hello, World!", window closed, exit 0, no Desktop assembly, 3 runs alike.
- Library findings: `windows.run` refuses a client size under the host's
  default 150 x 150 minimum with only "the host refused the window" and has
  no minimum-size option (the bridge raises the size: a gap); stable ids
  must be lowercase segments ("forms.main-form.1").
- The launcher now counts only managed images in the facade folder as
  facade assemblies, so native libraries can sit beside them.
- Step 4 done (first cut): `themes/sdrsharp/` (sheet + manifest from
  `dev/tools/make_theme.py`) is applied by `swapin_launcher run --theme <dir>`
  (runtime property `SWAPIN_THEME_PACKAGE`; the facade's
  `SwapinApplication.cs` makes the application with it, base GUI.Forms,
  before the first object). Row `v1_p0_theme`: outcome applied, 12 warnings
  (all W201: literal border colours, which forced high contrast cannot
  remap); `out/snapshot_v1_p0_theme.png` shows the P0 window in the SDR#
  dark look (#303030 ground, #3f3f46 button with #007acc accent, #333337
  field, light text), against `out/snapshot_v1_p0.png` without the theme.
  Palette source: DockPanel's ThemeVS2015 dark palette in SDR#'s extract,
  read as data (`swapin resources`, outputs in `out/theme_ref/`, never
  committed).
- Facade changes the theme needed: only program-set colours reach GUI.Forms
  (Surface hook `__ExplicitForeColor/__ExplicitBackColor`; unset ones are
  nulled where nullable), so the theme shows instead of facade-v1's
  defaults; single-line field height 23 (gap: not font-measured); arranged
  bounds flow back to managed controls.
- Launcher: bundled fonts. GUI.Forms' Windows host reads fonts from
  `GUI_FORMS_FONT_DIR` or `fonts` beside the executable; the launcher sets
  `GUI_FORMS_FONT_DIR` to `<facade>onts` for the child (without it no text
  is drawn at all, silently: a library finding).
- Theme findings: `text-box::part(face|well|content)` backgrounds had no
  visible effect, `:family(editor)` did (the field look is a family recipe);
  `user-control`, `scrollable-control`, `container-control` and
  `split-container` have no `face` part (W106 drops the whole rule list).
- Step 5 (managed painting) works: a control whose type overrides
  `OnPaint`/`OnPaintBackground` on a background-only base (panel, user,
  scrollable or container control) becomes a GUI.Forms custom control
  (`GF_TABLE_CUSTOM_CONTROLS`, base user or scrollable control). The
  facade's own paint path (bitmap, `Graphics`, `LockBits`) hands the pixels
  to `SetRasterPixels`; the bridge copies them (`RasterStore`), invalidates,
  and the paint callback presents them (nearest sampling at scale != 1).
  The windows paint endpoint is not needed for this; it stays for GDI
  handle leases (child `Handle`, DockPanel's `GetDC` use), not yet ported.
  Row `v1_paint` (`tests/paintapp`: a `Control` with `OnPaint`, a
  `UserControl` with `OnPaintBackground`, a Greet click that changes the
  painting through `Invalidate`): the snapshot holds 16000 band pixels in
  the after-click colour (200x20 logical at dpi 192), none before-click,
  44916 pane pixels; strip stored 3 frames, presented 2; rc=0, three
  identical runs (`dev/tools/check_paint.py`).
  Known limits: the facade paints at logical size, so at scale 2 the
  painting is upscaled (soft text); a custom-painted button, field or form
  keeps its stock look (gap named with the base); PNG frames (`SetRaster`)
  are a gap.
- Step 6, first batch (bridge gaps by SDR# usage weight; weights from the
  inventory's `memberrefs.tsv`: Control 584, Graphics 124, Form 113,
  TableLayoutPanel 78, DataGridView 74, ComboBox 64+28+28, CheckBox 60,
  NumericUpDown 53+12, MouseEventArgs 27, Label 19, Panel 16, ButtonBase 14):
  - Pointer ingress: every control subscribes `mouse_down/up/move/wheel/
    enter/leave` (MouseEventArgs fields `button`, `location`,
    `wheel_delta`; notches x 120) and raises facade-v1's pointer records,
    so OnMouseDown etc. reach managed and custom-painted controls (the
    spectrum, frequency edit and DockPanel chrome need this).
  - ComboBox: `SelectedIndex` crashed (text_editing serves text boxes only);
    combo field edits now run on the managed side (gap), and the field is
    `drop_down` style so the facade's text shows (items and selected_index
    not yet mirrored: gap).
  - NumericUpDown was a text field then; its range (minimum, maximum,
    increment, decimal places) is in the schema since cp25, and the facade
    mirrors it onto a native `numeric_up_down` when the schema has it (see
    below).
  - Mapped: label alignment and vertical alignment, button text alignment,
    FlatStyle onto `visual_style` and `flat_border_width`, panel BorderStyle
    onto `border_style`.
  - Named gap: key ingress (no key interaction or key payload in the ABI;
    text fields edit natively).
  Row `v1_paint` (extended app): pointer press at (20,30) in the painted
  strip reads back "down 20,30 Left"; the NumericUpDown shows 100000000, the
  combo "WFM", the right-aligned label and the framed panel show in the
  snapshot; rc=0 over three runs; p0 rows unchanged.
- Step 6, second and third batches (commits e8a621f and the next):
  - Combo items and selection mirrored into `combo_box` (`items` as a TEXT
    collection, `selected_index`, `drop_down_style` from DropDownStyle);
    selections in the native list come back (`selected_index_changed`); the
    facade's own drop-down is off for mirrored combos.
  - Scrolling: `scroll_state` into the facade's scroll record,
    `set_scroll_axis`, the auto-scroll properties, `scroll` events.
  - Tooltips: ToolTip.SetToolTip sets the control's `tool_tip_text` (GUI.Forms
    presents it; the owner's one-presenter direction); explicit
    ToolTip.Show at a position is a gap.
  - Host dialogs: MessageBox and the open, save and folder dialogs go through
    `host.show_dialog` (WinForms filters to GF filters).
  - Run guard: a Form.ShowDialog or popup during the window run returns at
    once with a named gap instead of crashing (one window at a time; the
    library's modal run is being made on claude/abi-modal-windows).
  - WinForms defaults the library lacked: DefaultSize at creation (a Label in
    a TableLayoutPanel cell was 0 x 0), labels top-left, panels without a
    border, strips docked (ToolStrip/MenuStrip top, StatusStrip bottom).
  - ToolStrip family and PictureBox take the raster path (facade-v1 paints
    them); custom-painted controls handling focus or keys are focusable.
  - Fix: gf colours are 0xRRGGBBAA (ToArgb was passed).
  - DataGridView: named gap (no column/row properties in the ABI).
  Row `v1_paint` covers all of these; rc=0, three identical runs.
- Step 6, fourth batch (commits bed6246 to the latest; 2026-11-09):
  - Owner-drawn ListBox (raster path, DrawItem) and ProgressBar/TrackBar
    ranges.
  - Keys: the ABI 1.1 key interactions (preview_key_down, key_down, key_up,
    key_press) raise PreviewKeyDown, ProcessCmdKey up the parents, the
    KeyPreview form's KeyDown, ProcessDialogKey, then KeyDown/KeyUp, with
    handled and suppress_key_press written back. Detected at run time.
  - NumericUpDown is a native `numeric_up_down` (minimum, maximum,
    increment, decimal_places, value, value_changed) when the schema has the
    range (detected through the schema table); else the text field.
  - Focus: Control.Focus/Select through `controls.focus` (a refusal returns
    false and logs its reason), Focused/ContainsFocus through `focus_state`,
    native focus moves adopted from `focus_observed`.
  - Dialogs: Form.ShowDialog during the run through `windows.show_modal`; a
    dialog closed without a result gives Cancel.
  - Window properties: the top-level run makes a window object
    (`windows.create`) and writes FormBorderStyle, the boxes, WindowState,
    TopMost, Opacity, StartPosition, ShowInTaskbar, MinimumSize and the title
    while running, plus Form.Icon/ShowIcon (themes.load_image). Written
    before the run; the library applies them at host attach
    (abi-window-properties 9d14d219, verified on abi-focus 65e4f4d2).
  - ToolStrips paint through ToolStripRenderer (Professional from its
    ProfessionalColorTable, application renderers override OnRender*), so
    SDR#'s DockPanel VisualStudioToolStripRenderer applies. Drop-downs are
    in-surface child overlays and already show (File menu in the paint app).
  - Private library builds: `dev/tools/build_keys_lib.sh <commit> <repo>`
    builds one branch's C ABI library offline in D:/malkuth/probes/swapin_lib
    (dependency sources from the integration build's _deps; never a fetch),
    extracting its ABI manifest when the branch has not; run rows against it
    with `SWAPIN_NATIVE=<its build dir> bash dev/tools/run_v1.sh`.
    `build_lib_incremental.sh` rebuilds after a local fix there.
  - Bindings (facade_v1/GuiForms.Interop/Generated) are from claude/abi-focus
    65e4f4d2's extracted manifest; regenerate from the integrated header
    after the fold (`python -B tools/abi_bindings.py csharp --output ...`).
  Rows pass on cp22 (fallbacks) and on the private abi-keys-numeric and
  abi-focus builds; three identical runs each.
- Findings and owed work:
  - The theme's scroll-bar part rules (now per type, not `*`) still leave
    the embedded scroll bars in the default look: a theme-engine question.
  - Library work in flight (coordinator): abi-keys-numeric, abi-focus
    (composite focus, focusable in the schema), abi-modal-windows,
    abi-window-properties (pre-run chrome), data grid columns and rows from
    C (w2). Form.Icon (themes.load_image) is not yet wired.
  - After the folds: regenerate the bindings and verify keys and focus
    together on one library.
  - TrackBar: not needed; SDR#'s DoubleBufferedTrackBar derives from its
    own ColorSlider (a Control), which the raster path covers.

## Plan after step 3

### Step 4: the SDR# look-alike theme (owner requirement, 2026-11-08)

A custom GUI.Forms theme that looks like SDR#: its dark palette, control faces,
fonts, the slider, combo and numeric looks, and the ToolStrip and DockPanel
chrome as far as GUI.Forms draws them. It ships in the SDR# zip as a theme
package (the `theme.json` package format, `demo/themes/gallery/` is the
template) and the launcher applies it through the Application config
(`gf_application_config.packages`, `theme_base` dark), from the facade's
first use of the application table. Built once facade-v1 runs on the real
library (step 3).

Reference look, gathered without running SDR#:
- `gui_forms/sdrsharp_reflection_system/screenshots/` (the 2026-08-10
  reflected run: main surface with toolbar, dock panels, spectrum, sliders;
  plugin surface) and stage R's brief
  (`planning/stages/R-sdrsharp-reflection/README.md`: owner-drawn combos and
  lists, `ToolStripProfessionalRenderer` gradients, GDI+ brushes, docking
  panels and splitters, custom range and frequency controls; Segoe UI,
  Tahoma, Microsoft Sans Serif).
- The readiness study's control table (`SDRSHARP_SWAPIN_READINESS.md`:
  `DoubleBufferedComboBox`, `DoubleBufferedNumericUpDown`, `BorderColor`,
  the dark title bar through DWM, the VS2015 theme's
  `VisualStudioColorTable`).
- As metadata only (no execution): colour constants and designer resources
  in the extract (`SDRSharp` .resources, DockPanel `ThemeVS2015` palette
  resources), read with the preflight's metadata reader.

Only matchable once SDR# runs under the launcher (needs the owner's
authorization and a sandbox): colours SDR# computes at run time or loads from
its settings, exact owner-drawn faces (its `OnPaint`/`OnDrawItem` output),
the spectrum and waterfall palettes, DockPanel's drawn chrome, font metrics as
it lays them out, and the per-control visual differential stage R's gate asks
for.

Theme packages are not shipped as SDR# assets: the package carries only our
own values (measured colours, our faces, bundled fonts per AGENTS.md: no
host fonts).

## RESUME POINTER (2026-11-09, SDR# night)

- PACKAGE (2026-11-09, 9e7cfff): dev/tools/package_zip.sh [--smoke] makes out/package/sdrsharp-guiforms-<date>.zip from the combined tree's Release build (SWAPIN_NATIVE). SDRSharp.GUIForms.exe = the launcher's start mode, running SDR# in place (owner ruling: settings, layouts and plugins persist; SWAPIN_PRIVATE_COPY=1 is the test-only private copy). Owner rulings: no licence for the launcher and facade, only the third-party notices; no contact line. Pending: build the zip once the integrator's combined build passes, then --smoke on a private desktop (never the owner's desktop).

- RULING (owner, 2026-11-09): do not emulate SDR#'s WndProc/WM_PAINT self-repainting of its combo box, text box and numeric up-down classes; their look comes from our theme, which is the solution (themes/sdrsharp/sdrsharp.theme.css field rules, matched to colours sampled from screenshots/sdrsharp_owner_compare_*). "WndProc border painting" is off the open list for good.

- RESUME POINTER (2026-11-09 night, second coding pass): facade 65694fe; library claude/swapin-sdr-present-path 400decc4f (registered). Added: NumericUpDown thousands_separator/text_alignment and TextBox text_alignment (library, codemod) with facade mapping; TrackBar.thumb_extent (library) set to 13 by the facade; custom-painted Buttons (mute button volume figure) on the raster path; field text colour in the theme; GroupBox caption inset and padding 3 plus light frame; ColorDialog through the host colour dialog; selectable panels and group boxes focusable. Not done: WM_PAINT emulation for SDR#'s DoubleBufferedComboBox/TextBox/NumericUpDown WndProc (they paint the whole control through BeginPaint + Graphics.FromHdc; the theme approximates them); AutoScale (no-op at matching fonts; the host scales DPI); combo text editing needs a run to find the gap; GradientDialog relied on ColorDialog (now present) and owner-drawn ListBox (supported). All unverified until the combined build.

- RESUME POINTER (2026-11-09 night, coding finished for the combined build): facade cf2c28f; library claude/swapin-sdr-present-path dac28e039 (registered). Done in code, unbuilt as a whole: waterfall zero-copy (borrowed DIB, opaque endpoint, one publisher copy; dev/tools/surface_bench: flush 2.6-3.2 ms to 0.12-0.16 ms), plugins inline (bundle served from bytes), menu font/metrics/selection/tooltip suppression, check/radio/field colours, dark scroll bars, caption close box per ControlBox and #d0d0d0 title, ambient fonts. Open: NumericUpDown thousands separator and TextAlign (needs a library property), TrackBar thumb size (library thumb extent is fixed per visual style), volume "0" label, ColorDialog and SDR# picker forms, GroupBox "Smoothing" frame, AutoScale, combo ForeColor, WndProc borders, Focus on containers, combo text editing; Display float window chrome/scroll/label clipping belong to the DockPanel worker. Verify everything in the combined build against screenshots/sdrsharp_owner_*.

- RESUME POINTER (2026-11-09 late, rendering cleanup): facade 98a73d5; library claude/swapin-sdr-present-path 84028ccf4 (= cherry-pick of dib-bottom-band-present 17c12563 + GdiFlush before DIB reads), registered in the integration queue. Done: one presentation path (managed paint composited into the endpoint DIB under its lease; no submit/touch/drain), resize guard (native Bounds notice with an unchanged size raises no Resize; 0.5 tolerance on arranged sizes), Enabled follows the parent, theme colours from the real SDR#. Sweep 123/123 (needs SWAPIN_FACADE_EXCLUDE=ScrollLayoutBridge.cs while the DockPanel worker's file does not compile); conformance 1402/1402 plain + ASan. Sweep evidence: waterfall endpoint published 11 of 11 frames once each, 0 compositions, 0 bounced arranged sizes, every endpoint resize managed (SDR#'s own). Left: live-signal check on the owner's desktop (re-fit and "spazzing"; capture.jsonl arranged_size/endpoint_resize/endpoint_stats), invalidation overlay once claude/invalidation-overlay lands, caption text #d0d0d0 (no part found yet), WndProc border emulation, Focus on containers, cursor kinds.
- STOP (2026-11-09, reboot). Heads: facade (this repo) see `git log -1` on its branch (sweep script now uses its own facade folder out/facade_v1_dev and a copy of SDR# at D:/malkuth/probes/sdrsharp_x64_sweep, so it never disturbs an owner session); library claude/swapin-gaps-conformance-fixes cfe6537d3 on dialog-request 46b3fb9b2. Done: sweep 123/123; live-radio proof of concept. Owner session log and captures saved in C:/Users/Shadow/gui_forms/screenshots/sdrsharp_session/ (capture_2026-11-09_0905.jsonl, row_sdr_desktop_2026-11-09_0905.log, PNGs).
- Owner note: "it does have some minor glitches that remind me of the playsuite glitch. sometimes the interface spazzes out". Probably the same GUI.Forms presentation bug as PlaySuite's doubled-toolbar bounce. The owner discusses how SDR# is rendered tomorrow.
- Open non-docking list (coordinator, next): WndProc / WM_NCPAINT / WM_PAINT emulation for custom-painted borders; Focus on Selectable containers (panels, user controls); combo text editing; cursor kinds; the colour table against the real SDR# (sdrsharp_compare.png: waterfall palette, field colours).
- MILESTONE (2026-11-09 09:05): first proof of concept. The real SDR# on GUI.Forms through the swap-in, on the owner's desktop, with a live radio: waterfall running, audio running (owner). Facade 647f590; library native build from claude/swapin-gaps-dialog-request 46b3fb9b2 (conformance-fixes cfe6537d3 touches tests only). Screenshots C:/Users/Shadow/gui_forms/screenshots/sdrsharp_desktop_first.png and sdrsharp_compare.png (real left, ours right); session captures in screenshots/sdrsharp_session/. Launch: `SWAPIN_REUSE=1 SWAPIN_INTERACTIVE_DESKTOP=1 SWAPIN_TIMEOUT_MS=86400000 SWAPIN_NATIVE=... bash dev/tools/run_desktop.sh`, then dev/tools/session_capture.ps1 -ProcessId <pid>.
- Update (2026-11-09, sweep): every UI-thread table call is marshalled (UiMarshal.cs, Marshalled.g.cs from dev/tools/make_marshal.py); owned windows for non-modal forms; Application.Run on foreign threads; session log with timestamps (capture/Capture.cs, SessionEvents.cs); coverage sweep 123/123 (SESSION_2026-11-09.md). Library stack on base 9585805d0: gdi-alpha, flat-button, combo-editing, panel-plane, dialog-request 46b3fb9b2, conformance-fixes cfe6537d3 (all in INTEGRATION_QUEUE.md). Conformance kit against the private build: dev/tools/build_conformance.sh.
- State: SDR# (AIRSPY SDR# Studio 1.0.0.1922, zip sha256 354e342f...5115e1 extracted at D:/malkuth/probes/sdrsharp_x64_next) comes up end to end through the swap-in, exit 0: toolbar, menus, dock panes, sliders, data grids, and the spectrum analyzer drawn through a child paint endpoint (screenshots C:/Users/Shadow/gui_forms/screenshots/sdrsharp_first_up.png, sdrsharp_docked_panels.png, sdrsharp_spectrum.png). Waterfall black is expected without a device.
- Facade head: 99a6485 (child handles as paint endpoints, Win32Routing DllImport resolver to gui_forms_win32_compat, data grid bridge, control images, System.Drawing forwards, thread-pool control parking).
- Private library (D:/malkuth/probes/swapin_lib/gui_forms, build .build/native-windows-x64/gui-forms via dev/tools/build_lib_incremental.sh): claude/swapin-gaps-base 9c1f6d4d0 (cp24 d5c8ef2ad + keys + window-props + focus + data grid, slots show_modal 6, show_owned 7, window_properties 8, focused_control 9, key_state 10), then claude/swapin-gaps-compat-abi1 a8bde9221 (shim abi1 endpoint fallback, Polygon). Both registered in desks/coordinator/INTEGRATION_QUEUE.md.
- Run: `SWAPIN_FAST=1 SWAPIN_CLICK=__none__ SWAPIN_NATIVE=D:/malkuth/probes/swapin_lib/.build/native-windows-x64/gui-forms bash dev/tools/run_sdr.sh` (snapshot out/snapshot_sdr.png, log dev/logs/row_sdr.log).
- Next gaps by SDR# weight: the splash (made on a thread-pool thread, parked); custom-painted combo/numeric/button raster; combo text editing; cursor kinds; field heights; check_box alignment; focus on panels; per-op Graphics native-surface flush cost; GDI-written black stays transparent in HDC leases (library lease-alpha fix candidate).
- Rules: owner authorized SDR# runs on the interactive desktop with network (SDR# only; close it after each run); GUI.Forms suites on private desktops; ask before any download; local git only.
