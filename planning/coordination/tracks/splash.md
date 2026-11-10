# Track: splash (Splash window kind, APP_WINDOW_RULES_DESIGN §S and K)

Worker: w5 (`~/Developer/gui_forms_workers/w5`). The w5 tree has `claude/abi-application` checked out
(the C ABI track's): leave it alone. Step 2 branches `claude/splash` from the base the coordinator names.

## State (2026-11-08, step 2)
- w5 now has `claude/splash` checked out (from claude/app-create b8027bbd). Leave claude/abi-application alone.
- Step 1 study done (`desks/coordinator/archive/SPLASH_STUDY.md`, §6 ruled; §S of APP_WINDOW_RULES_DESIGN.md updated
  with D5 (b)). Coordinator commits those.
- Step 2 committed: claude/splash @ 3fa98f1d (queued). M4 run: all built, 18/19 passed; splash_window_tests
  failed on stable ID "tool_tip", fixed (dashed segment) but not rerun (directive: no further runs).
  No VM run. Content: HostTransientKind + supports_splash_windows in the
  host protocol; core TransientWindow (warm standby, src/core/application/transient_window.*); tooltip
  presenter shows its bubble in a Splash window when the host admits one, overlay otherwise; macOS
  (ignoresMouseEvents, status/floating level, colour space kept), Windows (WS_EX_NOACTIVATE|TOPMOST,
  WM_NCHITTEST -> HTTRANSPARENT), X11 (override-redirect ARGB windows, _NET_WM_WINDOW_TYPE, empty XFixes
  input region; only with ARGB visual + compositor); legacy host tooltip retired (HostTooltipRequest and
  host_ready's two tooltip parameters gone; C ABI v0 show_tooltip/hide_tooltip drive the core presenter
  through a Registry friend on SurfaceRoot; Windows tooltip window and its fixture test removed).
- Then (coordinator 2026-11-08: menus are Popups before 0.5) b0571f07, unverified: menus on warm Popup
  windows (Window::show_popup_window family, detail::PopupStandby; ContextMenu panel per depth in a kept
  PopupFrame with pop+slide; keys forwarded by the owner's layer), Splashscreen API + three host
  surfaces + headless surface, handover at first window attach, K.2 warm_tooltip/warm_popup/
  cold_splashscreen (baselines to record), warm tooltip show asserted zero allocations. §S as-built
  paragraphs in APP_WINDOW_RULES_DESIGN.md (S.3 splashscreen, S.4 menus).
- Then (owner rulings 2026-11-08) 3ffb7584, unverified: Q-S1 (a) splashscreen = artwork PNG (window
  size = pixel size) + fixed status/progress styling, no title/colours/sheet; Q-S2 (b) X11 without a
  compositing manager keeps real Splash/Popup windows, opaque (default visual), no shadow:
  HostWindowing::transient_windows_translucent(), Window::popup_windows_translucent(), shadow flags on
  ToolTipBubble and the menu panel. Both recorded in §S (S.3, S.5) and §D of APP_WINDOW_RULES_DESIGN.md.
- Open (after the seal, Q-S3): menu alpha fade (needs window frame opacity), splashscreen animation.
- docs/reference/inventory.json not regenerated (api_reference.py needs clang; not a CI check).

- cp22 batch8 fix 17dc8945 on f4ec4f19 (claude/splash-warm-alloc): warm tooltip show back to 0 allocations
  (std::function copy in ApplicationWindowHandle requests; monitor table copy in desktop_placement).
  splash_window_tests passed alone on the M4. Then dd6aa7a0: macOS/Windows/X11 fill the monitor table in place
  (query_monitors_into_impl; make_logical_monitor), unverified.

- Splash C ABI (owner: before the seal) 523e211d on batch10 06a78004 (claude/splash-abi), unverified: runtime
  splashscreen_show/set_status/set_progress/close, GF_HOST_CAPABILITY_TRANSIENT_SURFACES + kinds field;
  generated files (manifest, probe, gf_abi.hpp, C#, c-abi.rst) to regenerate on the M4.

- Part S 0.5 scope (owner Q-S3, 2026-11-08; recorded §S.4/§D): IN pickers as Popups + menus onto the
  desktop; AFTER the seal menu fade + splashscreen animation. Built on claude/splash-abi (on 523e211d), see
  the queue entry: Window::popup_work_area, picker_popup_window_slot (7), forward_input_to_popup_window,
  PopupLift + drop_down_popup_face; ComboBox (both styles), DateTimePicker, RibbonColorPicker,
  FlagsValueEditor lifted; ContextMenu placed against the work area (root flips up, submenu flips left).
  Then e0cfecd2: TextBox auto-complete and RibbonGallery drop-down lifted; ErrorProvider's message was
  already on the Splash tooltip path (asserted in splash_window_tests).

- cp22 Linux warm-show fix 00475c31 on batch11 819f8114 (claude/splash-linux-alloc): libstdc++ SSO strings in the
  tooltip presenter recycled; VM splash_window_tests passes. Tool: w5/dev/tools/vm_splash.sh.

## Decisions in force
- Owner 2026-11-08: Splash and Popup share one transient window; Splash = no input (splashscreen,
  tooltip), Popup = same plus input through the owner. Reuse each host's present path.
- Ruled: D1(a) small Window model per transient window, warm; D2(a) fade painted into frames; D3(a) gate
  core allocations/time, host show apart; D4(a) Windows HTTRANSPARENT; D5(b) X11 override-redirect ARGB
  Splash/Popup windows, overlay only where the host truly cannot (no ARGB visual or no compositor);
  D6(a) retire the legacy host text tooltip.
- Owner rule "code first, verify once" (BRIEF 340-348): write all, one M4 run + one VM run at the end.

## Key measurements (M4, Release, in a run; SPLASH_STUDY §3)
- native popup open ~3 ms / 103 allocs; warm request_frame+show ~81 µs / 3 allocs; overlay open
  0.3 µs / 8 allocs; tooltip show 2.8 µs / 68 allocs (two controls per show).

## Map
- Popup core: src/core/window/native_popup/native_popup_window.cpp, src/core/application/native_popup.cpp,
  native_popup_record.hpp; host protocol include/gui_forms/host/windowing/host_windowing.hpp
  (HostWindowKind::popup, HostPopupPlacement, supports_popup_windows).
- macOS: macos_application_run.inc `MacApplicationRun::open` (popup branch), macos_host.mm
  `GUIFormsPopupPanel` (~3869), `GUIFormsView initWithModel` (~1871), `drawRetainedRect:` CGImage present
  (~2823-3175), legacy `showHostTooltip:` (~2340).
- Windows: windows_application_run.inc `windows_window_styles`, windows_host.cpp `show_tooltip` (~1859),
  raster/dib_frame_store.cpp, compositor/windows_compositor.cpp.
- Tooltip presenter: src/core/window/tool_tips/tool_tip_presenter.cpp (show at ~171, placement ~246).
- Creation gate: benchmarks/creation/ (in w5's tree via the stacked Part A branch).

## Tools (w5/dev/tools)
- `splash_probe/` (splash_probe.cpp, run.sh): in-run cost probe. rsync to M4 `w5/probe/`, then
  `M4WS_WAIT_IDLE=1 bin/m4ws w5 sh ../probe/run.sh` (links w5's Release dylibs; ~3 runs).

## Next step
Wait for the integration build of 3ffb7584 (head; includes b0571f07); fix what it shows (likely:
zero-allocation assertion, Popup-model focus/key forwarding, platform splashscreen and X11 opaque-path
compile errors). Then record the three new K.2
baselines (creation_gate.py record on the M4).
