# Track: T5 compositor cooperation (worker w1)

Updated 2026-10-25. Workspace `D:\gui_forms_w1`; checkout
`D:/gui_forms_w1/gui_forms` (give Windows git `D:/...` paths, never `/d/...`).
Scratch files in `D:\gui_forms_w1\dev\`. The previous track (build) is
finished; its file is `build.md`. Release 0.3 Themeable; depends on T2.

## State

- #321 plan merged; #324 (decisions) auto-merge, docs only.
- **#329** `claude/t5-c0-model`, auto-merge: C0 portable model
  (window_backdrop.hpp, 4 capability bits 28-31, protocol 9,
  HostCompositorEvent, Window::declare_non_opaque/request_backdrop/
  backdrop_grant/backdrop_changed, HostServices::apply_window_backdrop,
  PresentationSettings::reduced_transparency, headless simulation). Its
  first CI failed house-style (stale API reference); fixed by eab8aef8
  (regenerated docs/api), main synced server-side (d28b258a).
- **#331** `claude/t5-c1-alpha` (base main, contains C0), auto-merge: C1
  copy composite and clear-start presentation. Merged C0's fixes in
  (5e119a67).
- C0 CI lessons: the house-style job runs generate_library_docs --check,
  and the docs job runs Sphinx -W; a public member initializer that spans
  lines breaks the generated `cpp:member` (fixed 549fecd3 with `entries{}`).
- **#336** `claude/t5-c2-probe` from main (depth 1), auto-merge: C2 part 1,
  tests/windows_compositor_probe.cpp + planning/stages/T5-compositor/
  MEASUREMENTS.md, now with both halves (641b02f7 added Server 2022).
  Windows 11 26200: `transparent` = blur behind with an empty region;
  extend_frame shows the frame's colour; materials =
  DWMWA_SYSTEMBACKDROP_TYPE (inactive: DWM solid grey); withdraw with blur
  off + margins 0 + DWMSBT_AUTO; clear pixels hit the window. Server 2022
  (20348): blur behind composes identically; the attribute set and read
  both return E_INVALIDARG (= os_version).
- **C2 part 2** `claude/t5-c2-windows` (from #331's 5e119a67), pushed, head
  58d61a18, **PR not opened yet**: open it (base claude/t5-c1-alpha, or main
  once #331 merges) when #329 has merged, so the stack stays two deep.
  Contents: src/host/windows/compositor/windows_compositor.{hpp,cpp} (state
  read, message filter, apply), windows_host.cpp (report after attach and
  on WM_SETTINGCHANGE/WM_THEMECHANGED/WM_DWMCOMPOSITIONCHANGED, DIB frame
  alpha + premultiplied capture, 4 capability bits), host services
  apply_window_backdrop_impl, tests/windows_compositor_tests.cpp (the
  gate), lifecycle fixture frame_alpha(), hosts-and-capabilities.md,
  results/2026-10-25-t5-compositor/ (Windows 11 granted run). Verified:
  4 touched tests release + sanitizer, lifecycle/frame_store/tooltip under
  the sanitizer with TRANSACTIONAL_DIB ON (set back OFF), checks clean.
- C2 still owed: the CI fallback run recorded in results (from the C2 PR's
  evidence SDKLastTest.log), and the Shadow Windows run with transparency
  effects off then high contrast on, which needs the owner (user settings):
  ask the coordinator in the report.
- **#338 merged** (macOS probe). CI macos-15 (15.7.9) result, raw lines in
  D:/gui_forms_w1/dev/mac_probe_338.jsonl (from run 40248265605's
  gui-forms-evidence-macos-arm64 SDKLastTest.log); record in MEASUREMENTS.md
  with C3 part 2:
  - the runner has **reduce transparency ON**: every NSVisualEffectView
    material shows AppKit's solid fallback (236..255 grey), so the material
    mapping cannot be confirmed on CI (asked the coordinator: runner
    setting step or a bench Mac);
  - non-opaque window (opaque NO, clear background): clear pixels show
    the backdrop exactly, active and inactive; the window's own image keeps
    premultiplied alpha (0 and 128);
  - **clear pixels are click-through by default** (windowNumberAtPoint gives
    the backdrop); setting `ignoresMouseEvents = NO` explicitly makes them
    hit the window: the adapter must set it (T5-Q6). Posted HID clicks
    reached neither view even though AXIsProcessTrusted is true;
  - half column composed 103,66,3 on a fresh window but exactly 100,80,0
    after a change (probe fills DeviceRGB, the host draws an sRGB CGImage:
    colour management, not alpha);
  - an opaque window shows its background (246) under a copy-cleared pixel.
- **#339** `claude/t5-c4-probe` from main, auto-merge: tests/x11_compositor_probe.cpp
  (Xlib + XFixes; blind compile). Records default vs argb32 visual, root
  composite, own alpha, hit test, _NET_WM_CM_S0 XFixes trace (xcompmgr if
  installed, else a stand-in owner), and a settings line with XWAYLAND
  extension / WAYLAND_DISPLAY so the same binary measures XWayland. CI has
  no xcompmgr or weston: asked the coordinator to admit them
  (tools/install_linux_ci_packages.sh toolkit list).
- **C5 part 1** branch `claude/t5-c5-base` (from #331's 5e119a67), head
  b3ac798b, pushed, PR not opened (open after #329 merges): ThemeDefinition::window_backdrop
  (ThemeWindowBackdrop: request + per-backdrop optional base), validated in
  Theme::create; Window follows the theme's request until the app calls
  request_backdrop (application_backdrop_request_), and draws its backplane
  from Window::backplane_base (presentation + engine_backplane);
  PrimitiveTarget::reduced_transparency (sampling primitives only); glass
  flattens (tint over the unblurred backplane pixel, opaque); a
  reduced_transparency change damages backplane readers. Gate
  tests/engine_backdrop_tests.cpp (hashes recorded; 15 area tests + 5 under the sanitizer pass).
  The @media side of Q8 waits for T3 P8 (EC3).
- Coordinator (2026-10-25): build C0 onward in plan order, one PR per
  phase with its gate, auto-merge, no CI waits. Stop at owner decisions or
  a genuine platform blocker. No Wayland-native planning; measure XWayland.

## C1 facts for C2 to C5

- The Windows DIB frame keeps no alpha (`maintain_alpha_` false at
  begin_frame; GDI writes alpha 0) and its capture encodes
  `PngSourceAlpha::opaque`. C2 must make the window DIB keep alpha under a
  non-opaque grant and capture premultiplied then.
- Skia captures already encode premultiplied alpha (macOS, Linux).
- Theme window materials are opaque today, so a non-opaque grant looks the
  same until C5 supplies the backdrop base. A translucent window material
  under an opaque grant still blends over the previous frame (unchanged).
- `near` is a Windows macro (windef.h): never name a variable `near`.
- CI's house-style job runs `tools/generate_library_docs.py --check`:
  regenerate docs/api after any public header change
  (`sh D:/gui_forms_w1/dev/checks.sh` runs every repository check).

## Local build notes

- Background Bash lacks USERPROFILE (ccache fails): export USERPROFILE='C:\Users\Shadow' first.
- Full build after a window.hpp change takes > 10 min on the shared cores;
  use `sh D:/gui_forms_w1/dev/t5_targets.sh '<targets>' '<regex>'` and run
  build.sh in the background.
- Python heredocs through the Bash tool collapse `\n` to `
`; write edit
  scripts with the Write tool and chr(92).

## Plan in one paragraph

Phases C0 portable model and headless traces (2 PRs); C1 premultiplied
presentation, opaque bytes unchanged (1); C2 Windows DWM (measure the
client-alpha route first; Mica/Acrylic via DWMWA_SYSTEMBACKDROP_TYPE,
build 22621+) (2-3); C3 macOS NSWindow non-opaque + NSVisualEffectView (2);
C4 X11 depth-32 visual, _NET_WM_CM_Sn + XFixes, Xvfb with/without xcompmgr
(2); C5 engine join (backdrop base, glass over clear backplane, reduced
transparency / high contrast rules) (2); C6 backplane replay for
full-fidelity glass (2); C7 reflection (2); C8 stage gate, lab program,
docs (1). C0, C1 first; C2-C4 any order; C5 beside them; C6 after C5; C7
after C6 (own-raster source may land after C5).

## Decisions in force

- Owner round 001 (2026-10-25): T5-Q1..Q16 all option 1 (see T5 README).
  Open owner concern: Wayland (coordinator owns that conversation).
- From the brief: documented capability-reported adapter behaviour; Win32
  in src/host/windows, AppKit in src/host/macos, X11 in src/host/linux; CPU
  only; never theme the compositor; no Wayland adapter; unsupported
  requests return explicit results.
- T2 facts relied on: E9-B / E9.3 backplane borrow
  (`PrimitiveTarget::backplane`, `Window::engine_backplane`,
  `backplane_skipped_commands`); reflection is CSS_COVERAGE row 10.

## Findings worth keeping

- No host reads OS high contrast or reduced motion today;
  `PresentationSettings` is app-set only (Q7 proposes T5 builds the signal).
- Windows loads dwmapi.dll at run time with decltype-typed entry points
  (`windows_application_run.inc`); present is a 32bpp BGRA premultiplied
  DIB via BitBlt/SetDIBitsToDevice. macOS presents a premultiplied CGImage
  with kCGBlendModeCopy. X11 packs RGB only (`linux_host.cpp`).
- CI runners: windows-2022 (build 20348, no system backdrop), macos-15,
  ubuntu-24.04 with xvfb-run depth 24.

## Next step

1. When #329 merges: open the C2 part 2 PR (body D:/gui_forms_w1/dev/c2b_pr.md)
   and the C5 part 1 PR, both on #331 (or main once #331 merges), with
   auto-merge. Record CI Server 2022's gate run in results/.
2. C3 part 2 (macOS adapter, blind compile): on a non-opaque grant set
   window opaque NO, clear background, `ignoresMouseEvents = NO`; materials
   via an NSVisualEffectView content container under the GUIFormsView
   (behindWindow, followsWindowActiveState); NSWorkspace accessibility
   display options observer (named selector) feeding reduced_transparency
   (and high_contrast per Q7); gui_forms_macos_compositor_tests reading
   back opacity, effect material/blending/state and frame alpha.
3. C4 part 2 after #339's Linux evidence.

## Owner's Windows runs (C2 gate, Shadow Windows) — RERUN, ready to hand over

The first runs (2026-10-25, kept in `D:\gui_forms_w1\owner_run\previous\`)
found two defects on our side, now fixed (MEASUREMENTS.md, "The owner's gate
runs"): composition was misread as absent under a contrast theme, and high
contrast lost to the host's refusals in the reason. The transparency-off run
read transparency on; the fixed program prints the raw settings it read, so
the rerun will say why.

The program is staged again, built from `claude/t5-c2-windows` 0ae565c3
(which contains the C0 precedence fix 2ede1403):
`D:\gui_forms_w1\owner_run\gui_forms_windows_compositor_tests.exe`, with its
two DLLs beside it. No arguments, no build. Each run is one double-click on
a script in that folder; the script shows the registry value first, waits
for a key, runs the program (two topmost windows for about 3 seconds:
leave the mouse and keyboard alone), then shows what the program read and
its last line, and waits again.

1. `run-transparency-off.cmd`: first Settings > Personalization > Colors >
   Transparency effects: **Off**, and wait a few seconds. The script must
   show `EnableTransparency REG_DWORD 0x0` before you press the key.
   Expected: exit code 0, last line `windows compositor tests passed`;
   `transparency_enabled : False`, `reduced_transparency : True`;
   sources `enable_transparency : 0`, `advanced_effects : 0`. The step lines
   in the .jsonl: `material` and `transparent` granted `opaque` with
   reason `reduced_transparency`. Then turn transparency effects back on.
2. `run-high-contrast.cmd`: first Settings > Accessibility > Contrast
   themes: any theme, Apply. Expected: exit code 0, last line passed;
   `high_contrast : True`, `compositing : present`; sources
   `high_contrast_flags` odd (HCF_HIGHCONTRASTON, 1, set); steps `material` and
   `transparent` granted `opaque` with reason `high_contrast`. Then set
   Contrast themes back to None.
3. `run-restored.cmd`: normal settings. Expected as the first restored run:
   material granted `window_material`, transparent `transparent` with
   clear (0, 160, 0), opaque granted; last line passed.

Any other last line is a failure; the .jsonl then ends with a
`{"gate":"windows_compositor","failed":"..."}` line saying which check.
Send back the three .jsonl files (`transparency-off.jsonl`,
`high-contrast.jsonl`, `restored.jsonl` in the same folder); w1 records them
in `results/2026-10-25-t5-compositor/`.

## Bench Mac run (C3 materials) — RERUN, ready to hand over once #351 is on main

The first bench run's probe windows were never active (macOS 14 and later
grant `[NSApp activate]` only when the active application yields, and
Terminal does not), so its material readings are not evidence
(MEASUREMENTS.md, "The bench Mac run"). The probe is now an application
bundle launched through LaunchServices, and it refuses to report a material
read in an inactive window.

- **Build** (the GUI.Forms repository at main once #351 has merged, or the
  branch `claude/t5-c3-probe-active`, on a Mac with Xcode's command-line
  tools). CI's own build, from the directory that holds `gui_forms/`, is the
  dependable route (it prepares Skia and the third-party trees; other tests'
  results don't matter here):

  ```text
  python3 gui_forms/tools/ci_build.py --jobs 8
  ```

  With that tree made, rebuilding only the probe is
  `cmake --build .build/native-macos-arm64/gui-forms --target gui_forms_macos_compositor_probe`.
  The bundle is
  `.build/native-macos-arm64/gui-forms/gui_forms_macos_compositor_probe.app`.
- **Before running:** System Settings > Accessibility > Display > Reduce
  transparency: **Off**. Increase contrast: Off. Appearance: Light. One
  display. Nothing else needs closing, but leave the machine
  alone for about 60 seconds: it shows a green window and a probe window,
  and posts one click per case.
- **Run** from the directory that holds `gui_forms/` (the output path must
  be absolute, because a bundle launched by LaunchServices starts in `/`):

  ```text
  sh gui_forms/tests/support/run_macos_probe_bundle.sh .build/native-macos-arm64/gui-forms/gui_forms_macos_compositor_probe.app "$PWD/mac-bench.jsonl"
  ```

  The script launches the bundle with `open`, waits for it to quit, prints
  its lines, and exits 0 only when the last line is
  `macos compositor probe finished`. Repeat with Appearance set to Dark
  (`mac-bench-dark.jsonl`), then with Reduce transparency **On**
  (`mac-bench-reduced.jsonl`).
- **Permissions.** The bundle is its own application for macOS privacy
  settings. Its readings of its own two windows need no permission. The
  posted clicks (`click_received`) need Accessibility trust for the bundle;
  the settings line reports it as `accessibility_trusted`. Granting it
  (System Settings > Privacy & Security > Accessibility, add the bundle) is
  the owner's choice; without it the click readings are not evidence, and
  criterion 3 below does not depend on them.
- **What it prints:** 18 case lines, one settings line, then the last line.
  Each case line starts with `key_window` and `app_active`; a material case
  read in an inactive window holds only `"not_exercised":"window inactive"`,
  and the last line is then `not exercised: window inactive` (a failed run:
  send it back as it is, with a screenshot if you can).
- **Pass criteria** for the Light and Dark runs:
  0. The last line is `macos compositor probe finished`; every case line
     has `"key_window":true,"app_active":true`; the settings line has
     `"bundle":true` and an `activation` of `activate` (recorded if it is
     `ignoring_other_apps`). A screenshot during a material case shows
     coloured traffic lights and the probe's menu bar.
  1. The settings line has `"reduce_transparency":false`.
  2. `clear`, `clear_receiving` and `effect_then_clear`: `clear` is
     (0, 160, 0) ±12.
  3. `clear_receiving` has `"window_at_clear":"probe"`, and `clear` has
     `"backdrop"` (`ignoresMouseEvents = NO` keeps clicks on the window,
     T5-Q6).
  4. Each material case (`window_background` to `tool_tip`) has
     `"material_shown":true`, and its `clear` shows the green behind,
     blurred and tinted: green exceeds both red and blue by at least 8, and
     it is not the flat grey CI recorded (for example 236, 236, 236). The
     effect views follow the window's active state; nothing forces them
     active. `clear_inactive` (another window key) may differ.
  5. **Mappings to confirm** (PLAN.md §2.2): `window_material` maps to
     `under_window_background`; `transient_material` to `popover`, with
     `menu` the alternative. A mapping passes when its case meets
     criterion 4 in Light and Dark; where `popover` fails and `menu`
     passes, the mapping becomes `menu`.
  6. `under_window_background_opaque_window` is recorded, not judged.
- Send back the three .jsonl files.

## PAUSED 2026-10-25

Paused on the owner's instruction (via the coordinator). Nothing pushed since the pause.

1. **Working on now: C3 part 2 (macOS adapter and gate).** Worktree `D:/gui_forms_w1/wt_c3b`, branch `claude/t5-c3-macos`, head 5e119a67 (= the C1 branch base; nothing committed yet). Uncommitted:
   - new `src/host/macos/compositor/macos_compositor.{hpp,mm}`: state read (present, all backdrops supported, reduced = NSWorkspace accessibilityDisplayShouldReduceTransparency, high_contrast false per T5-Q7); `make_macos_content_view` (backdrop container: hidden behind-window NSVisualEffectView + the raster view; only for a window declared non-opaque); `apply_macos_backdrop` (opaque / non-opaque clear + `ignoresMouseEvents = NO` per T5-Q6 / material shows the effect view at UnderWindowBackground or Popover; invalidateShadow after each).
   - `macos_host.mm`: include, `_compositorGeneration` ivar, `bind_macos_backdrop_view` in init, workspace-centre observer for NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification (removed in dealloc), `reportCompositorState` (end of initializeHost and on the notification; skips a reading equal but for the generation), `hostContentView` used at both window-creation sites (also `macos_application_run.inc`), four compositor capabilities added to `macos_capabilities()`.
   - `appkit_host_services.mm`: private `__weak NSView* backdrop_view_`, `apply_window_backdrop_impl` override, friend `bind_macos_backdrop_view`.
   - `CMakeLists.txt`: the .mm in gui_forms_host_macos; `gui_forms_macos_compositor_tests` (links gui_forms_application + AppKit + CoreGraphics; RUN_SERIAL, TIMEOUT 60) inside `if(TARGET gui_forms_application)`.
   - new `tests/macos_compositor_tests.mm` (the gate): steps host / material / transparent / opaque over a green floating backdrop window; with the host reporting reduce transparency it records the withdrawal (opaque, reduced_transparency), then sets the app's PresentationSettings.reduced_transparency=false and prints `"material_screen":"not exercised: reduce transparency on"`.
   - Not yet done: docs/user/hosts-and-capabilities.md macOS column and a **macOS** paragraph (mirror the Windows one on claude/t5-c2-windows), house-style + lifetime checkers, commit. The compile is blind (no Mac): CI's macOS job is the first build.
2. **Open PRs and pushed branches:**
   - #329 `claude/t5-c0-model` (C0): open, auto-merge, BLOCKED (waiting on CI/review).
   - #331 `claude/t5-c1-alpha` (C1, contains C0; head 160fce61): open, BLOCKED; base main.
   - #339 `claude/t5-c4-probe` (C4 part 1: X11 probe, XWayland under headless weston, xcompmgr + weston + xwayland in install_linux_ci_packages.sh): open, BLOCKED, head 1fac764b.
   - #336 (C2 probe, Server 2022 measurements) and #338 (macOS probe): merged.
   - `claude/t5-c2-windows` 58d61a18 (C2 part 2, Windows adapter + gate): pushed, no PR; body in `D:/gui_forms_w1/dev/c2b_pr.md`. Stacks on #331.
   - `claude/t5-c5-base` b3ac798b (C5 part 1, theme window backdrop + flattened glass): pushed, no PR. Stacks on #331.
   - `claude/t5-c3-macos`: local only (above).
3. **Next, in order (on resume):** finish C3 part 2 (docs, checkers, commit, push; open PR only once #331 is on main); when #329 then #331 merge, open the C2 part 2 and C5 part 1 PRs with auto-merge and record CI Server 2022's gate run in results/; read #339's Linux evidence (gui-forms-evidence-linux-x64 LastTest.log, incl. the XWayland case) and record X11/macOS measurements in MEASUREMENTS.md; then C4 part 2 (X11 adapter + gui_forms_linux_compositor_tests), C5 part 2, C6, C7, C8.
4. **Waiting on others:** owner — the Windows runs (section above, staged in `D:\gui_forms_w1\owner_run\`) and the bench Mac run (section above; ready). Coordinator — merges of #329, #331, #339 (update-branch loop). No open decisions.

## RESUMED 2026-10-25 (after the owner's reboot)

- C3 part 2 committed and pushed: `claude/t5-c3-macos` 8c28dc47 (adapter, gate, docs column). No PR yet: it stacks on #331, which stacks on #329 (would be 3 deep). Open it once #329 merges (base claude/t5-c1-alpha) or once #331 is on main (base main). The macOS compile is blind; CI's macOS job is the first build.
- #345 `claude/t5-c3-measure` (off main, docs only): the C3 macOS measurements in MEASUREMENTS.md; auto-merge set.
- #339: the XWayland case failed on CI (BadMatch on X_GetImage of Xwayland's rootless root window; Xlib's default handler ended the probe). Fixed at 99ee2295 (an error-recording handler; settings line gains x_errors / last_error_request / last_error_code); head 580878c7 after merging the server's branch update.
- X11 (Xvfb) evidence from #339's first run is saved in `D:/gui_forms_w1/dev/x11_probe_339.jsonl`: no compositor: argb32 clear shows black, own alpha kept; xcompmgr: argb32 composes exactly (0,160,0)/(100,80,0); depth-24 never; clear pixel hits the probe (no click-through on X); XFixes trace owner_set then owner_client_closed (xcompmgr) / owner_destroyed (stand-in). Record it in MEASUREMENTS.md C4 with the XWayland reading once #339's rerun gives it.
- Next: C4 part 2 (X11 adapter) on a branch from claude/t5-c1-alpha.
- C4 part 2 committed and pushed: `claude/t5-c4-x11` 349f3979 (from claude/t5-c1-alpha; no PR yet, same stacking rule as C3). Files: `src/host/linux/compositor/x11_compositor.{hpp,cpp}` (ARGB visual via XMatchVisualInfo + XRenderFindVisualFormat; `X11CompositorWatch` on `_NET_WM_CM_S<screen>` with XFixes; `x11_compositor_state`), linux_host.cpp (ARGB window creation + colormap, per-window visual/depth/alpha in the present, alpha channel in the conversions, `report_compositor_state` at attach and on each XFixes notification, capabilities transparent_windows + compositor_notifications only), linux_services.cpp (`apply_window_backdrop_impl`), CMake (X11::Xfixes, X11::Xrender required; gate `gui_forms_linux_compositor_tests`, RUN_SERIAL), docs X11 paragraph, `tools/package_programs.py` fallback depends + libxfixes3, libxrender1. Linux compile is blind (no Linux here).
- Gate design: Application::run, window found on UI-timer ticks (host requests may be unflushed at ready), green override-redirect backdrop, steps none/started/stopped/restarted, grants expected opaque/transparent/opaque/transparent in backdrop_changed; xcompmgr missing => not exercised, but fails when `CI` is set.
- Doc conflicts expected: C2, C3 and C4 each rewrite the "No native host advertises..." paragraph of docs/user/hosts-and-capabilities.md; resolve when each lands (merge main locally only for that).
- C6 part 1 committed and pushed: `claude/t5-c6-replay` db012c9d (from claude/t5-c5-base; no PR yet, stacked). Window::replay_backplane replays every backplane display chunk through the renderer's PaintFramebuffer (made once per extent and scale, reset on set_framebuffer_painter), falls back to the part-raster composite; MetricsSnapshot.backplane_replay (T5-Q13), RasterCacheStatistics.backplane_replays; gate gui_forms_engine_backplane_replay_tests (passes, also under the sanitizer). C6 part 2 still to do: the DIB/Skia cross-renderer scene with text hashes and the cost.
- C5 is likely one PR: part 1 already covers the C5 gate except reflection (C7).

## OWNER-RUN FIXES 2026-10-25 (coordinator: fix before any rerun)

- **Windows, C0 (#329, `claude/t5-c0-model` 2ede1403):** high contrast is now checked right after not_declared, before the host's support and no_compositor (window_backdrop_refusal; header comment, generated docs, unit tests in compositor_backdrop_tests). Merged into #331 (`claude/t5-c1-alpha` 9c5d3dd8) and into `claude/t5-c2-windows`.
- **Windows, C2 part 2 (`claude/t5-c2-windows` 0ae565c3, pushed, still no PR, stacked):** compositing present on Windows 8+ (IsWindows8OrGreater; DwmIsCompositionEnabled answers FALSE under a contrast theme, only asked on older systems). Gate: state line gains `"sources"` (build, enable_transparency + status, advanced_effects via WinRT UISettings, high_contrast_read/flags, composition, remote_session; links runtimeobject); withdrawn steps must give the setting as the reason (high_contrast, else reduced_transparency, else transparency_disabled); a failure also prints `{"gate":"windows_compositor","failed":...}`. Local run at normal settings passes; sanitizer passes.
- **Staged:** `D:\gui_forms_w1\owner_run\` rebuilt from 0ae565c3 (exe + 2 DLLs; runs with a clean PATH), run-transparency-off/high-contrast/restored.cmd regenerated by `D:/gui_forms_w1/dev/make_run_cmds.py` (show the registry value, pause, run by full path `%~dp0`, show state and sources); first runs moved to `owner_run\previous\`.
- **Mac probe: #351 `claude/t5-c3-probe-active` (off main), auto-merge:** MACOSX_BUNDLE probe launched via `tests/support/run_macos_probe_bundle.sh` (open -W -n --args --output); activateIgnoringOtherApps: fallback; per-case key_window/app_active; material case inactive => `"not_exercised":"window inactive"`, last line `not exercised: window inactive`, exit 1; success last line `macos compositor probe finished`. Risk: under LaunchServices clicks need Accessibility trust (CI may lose click readings).
- **MEASUREMENTS.md:** both findings recorded on #345 (`claude/t5-c3-measure` f6893b05): C2 "The owner's gate runs" and C3 "The bench Mac run".
- **Downstream branches still to merge the C0 fix:** c3-macos, c4-x11, c5-base, c6-replay (merge c1-alpha into each when next touched).
- Next: report to the coordinator (sheets above); then resume C6 part 2.

## SIDE TASK 2026-10-25: host double-click interval (for W's ListView label edit)

- Branch `claude/host-double-click-interval` (off main), worktree `D:/gui_forms_w1/wt_dbl`, squashed commit 99cce4c6; PR body in `D:/gui_forms_w1/dev/dbl_pr.md`.
- Field: `PresentationSettings::double_click_milliseconds` (+ `double_click_origin`, `SettingOrigin::{default_value, host}`), constants in new `include/gui_forms/window/presentation/input_settings.hpp` (default 400, maximum 5000). Event `HostInputSettingsEvent` (protocol version 9; C1 #331 also takes 9: whichever lands second becomes 10). Session copies only the two fields; refuses 0, >5000, bad origin as invalid_payload.
- Hosts: Windows GetDoubleClickTime after attach + WM_SETTINGCHANGE; macOS NSEvent.doubleClickInterval after attach + each activation; X11 `src/host/linux/settings/xsettings_{parse,watch}` (grabbed owner lookup, PropertyNotify/MANAGER/DestroyNotify, ForeignWindowScope; default 400 as default_value), X11 click counting uses it; headless fixed 500 (`host::headless_double_click_milliseconds`).
- Tests: host_protocol (new test), runtime_windows (headless value), new xsettings_parse_tests (all platforms), new windows_input_settings_tests, key_commands (version 9). Local Windows pass, sanitizer pass, api_reference pass. PR #358, auto-merge, CI pending. Next: resume C6 part 2.
- Coordinator: #331 keeps 9, #358 takes 10. Done: #358 head bumped to 10 with `host_protocol_revisions` (rows 8, 9 = C1's compositor report, 10 = input settings; current = last row; static_assert consecutive from 8). Auto-merge OFF on #358. **When #331 is on main:** rebase #358 onto main (resolve #331's `current_protocol_version = 9` / comment against the table; keep the table), check key_commands_tests pins 10 and the C1 tests see the table, then `gh pr merge 358 --auto --merge`.
- Standing rule (BRIEF, 2026-10-26): before every commit run `git diff --cached --stat`; files I didn't edit or whole-file changes => reset and restage from a fresh shell. Audit of all my 2026-10-25 commits (blobs read with Python, not msys pipes, which add CRs in transit): no CR and no whole-file rewrites.

## C6 part 2 (2026-10-26), on `claude/t5-c6-replay`

- Shared scene `tests/backplane_replay_scene.hpp` (backplane Page: light page, dark band, two Carlito text runs; Pane glass at (10,10,220,80) blur 1; hash = glass PartRaster rows, FNV-1a; scales 1, 1.5, 2). Gates: `gui_forms_windows_backplane_replay_tests` (DIB, fonts dir arg; include the scene BEFORE windows_host.cpp: windows.h `near`/`far` macros) and `gui_forms_backplane_replay_identity_tests` (Skia, inside `if(TARGET gui_forms_text_engine)`, Carlito via GUI_FORMS_TEST_CONTENT_FONT). Both check replays==1, skipped==0, metrics backplane_replay, hash == table. Print mode: GUI_FORMS_BACKPLANE_REPLAY_PRINT=1.
- DIB hashes (Windows x64): 1.0 0x14e91c188a71dda6, 1.5 0xe31497238996ea16, 2.0 0xfd9daabeb75fa8ee. Skia is blind (no local Skia): if CI's Skia prints other hashes, that is a DIB/Skia text-through-replay divergence => decision to the coordinator (ADR-004 says text is identical).
- Committed and pushed: `claude/t5-c6-replay` cd3a0cba (no PR yet; stacks on #331 like C5). DIB gate and sanitizer pass.
- Cost (Release, Windows x64, per repaint after backplane damage, includes the replay, glass re-bake with blur 1 and the window paint; 64 repaints): scale 1.0 (24,000 window px) 1.62 ms; 1.5 (54,000) 3.80 ms; 2.0 (96,000) 9.11 ms. Record in MEASUREMENTS.md C6 when the branch has main's MEASUREMENTS.md.

## #358 rebased (2026-10-26)

- #331 merged to main (997815c3). #358 squashed, rebased onto main and pushed: head 71369e11. Conflicts resolved by `D:/gui_forms_w1/dev/resolve358.py` (table kept; both events in the variant; equality joined; docs sections both). host_protocol_tests asserts rows 9 and 10 and current == 10; compositor_backdrop_tests now checks row 9 is the compositor report (was `== 9U`). Local pass (8 tests incl. api_reference), sanitizer pass (6), house-style / lifetime / state checkers clean. Body updated; auto-merge re-enabled (merge). CI pending.
- Next: open the stacked PRs off main (C2 part 2, C3 part 2, C4 part 2, C5 part 1, then C6 stacked on C5), each after merging main and resolving the hosts-and-capabilities.md paragraph.

## PAUSED for org move (2026-10-26)

Paused by the coordinator (the repository moves to github.com/malkuth-software/gui_forms). Nothing is to be pushed until the coordinator says resume; then repoint remotes.

**Open PRs (all off main, auto-merge set, CI pending at pause):**
- #358 `claude/host-double-click-interval` 71369e11: input settings, protocol 10. Rebased after #331.
- #362 `claude/t5-c2-windows` 1afe8a22: C2 part 2, the Windows adapter and gate. Main merged in, no conflict; local tests and sanitizer pass. Body is in `D:/gui_forms_w1/dev/c2b_pr.md`.
- #363 `claude/t5-c3-macos` 75ab4a25: C3 part 2, macOS. Main merged in. Compiled blind; CI's macOS job is the first build. Body is in `D:/gui_forms_w1/dev/c3b_pr.md`.
- #351 `claude/t5-c3-probe-active`: the Mac probe launched as a bundle.
- #345 `claude/t5-c3-measure`: MEASUREMENTS for C2/C3.
- #339 `claude/t5-c4-probe` head 11cf7c60: **FAILING.**
  - `gui_forms_xwayland_compositor_probe` fails on all three Linux jobs: x64, arm64 and the sanitizer.
  - Xwayland itself segfaults: "Segmentation fault at address 0xa0 ... Server aborting", with a backtrace in Xwayland (logs: `D:/gui_forms_w1/dev/c339_arm.log` around line 2849, and `c339_san.log`).
  - That is the server crashing, not the probe. To find out, either:
    - bisect which probe request triggers it (GetImage/Composite on rootless Xwayland), and skip that request under XWayland; or
    - mark the XWayland case not-exercised on a server crash, recording the crash as evidence.
  - Decide with the coordinator if it's a skip.

**Pushed, no PR (do not open until resumed):**
- `claude/t5-c4-x11` 32f1a4d0 (C4 part 2, main merged in). Its gate needs xcompmgr from #339's install script, so open it after #339 merges, or stacked on #339.
- `claude/t5-c5-base` b3ac798b (C5 part 1): needs a main merge, then a PR.
- `claude/t5-c6-replay` cd3a0cba (C6, stacked on C5): open after C5.

**Next on resume:**
1. Fix #339's Xwayland crash case.
2. Open C4 part 2.
3. Merge main into C5, then C6, and open them (C6 based on C5, at most two deep).
4. Resolve the conflicts on the "do not advertise yet" docs paragraph as C2, C3 and C4 land.
5. C2 vs #358 may conflict in windows_host.cpp (attach and WM_SETTINGCHANGE), and C4 vs #358 in linux_host.cpp (click counting). Resolve them by keeping both.
6. Record in MEASUREMENTS.md:
   - C6's cost: 1.62, 3.80 and 9.11 ms;
   - the DIB hashes;
   - C4's X11 evidence;
   - CI Server 2022's C2 gate run.
7. Then C7 and C8.

**Waiting on the owner:**
- the Windows reruns (staged in `D:\gui_forms_w1\owner_run\`);
- the bench Mac run (sheet above; goes out once #351 merges).

The `D:/gui_forms_w1/gui_forms` checkout is back on `claude/t5-c6-replay`.

## PAUSED for the night (2026-10-26)

Resumed after the org move (origin is now github.com/malkuth-software/gui_forms), then paused again for the night.

**#358 fixed and pushed:** `claude/host-double-click-interval`, head 171c891d. Nothing uncommitted.
- CI had failed on every platform: `gui_forms_host_boundary_audit` found the AppKit token NSEvent in the comment in `presentation_types.hpp`. The comment now points to the hosts page; the XSETTINGS mention in `input_settings.hpp` was neutralised too.
- The docs job had failed with a Sphinx "postfix expression" error: the table's `{{` on its declaration line ended up in the cpp:var signature. The inner brace moved to the first row and the API docs were regenerated.
- Ran locally: host_protocol, compositor_backdrop, runtime_windows, host_boundary_audit and api_reference (all pass); `generate_library_docs.py --check` passes.
- The Linux and macOS job logs were not readable while run 40293034288 was still in progress. Check them on the new run.

**Open PRs:**
- #358;
- #362 (C2) 1afe8a22;
- #363 (C3) 75ab4a25;
- #351;
- #345;
- #339 (`claude/t5-c4-probe` 11cf7c60, failing: Xwayland segfaults).

**Pushed, no PR:**
- `claude/t5-c4-x11` 32f1a4d0;
- `claude/t5-c5-base` b3ac798b;
- `claude/t5-c6-replay` cd3a0cba.

Checkout `D:/gui_forms_w1/gui_forms` is on `claude/t5-c6-replay`. The worktrees are clean.

**Next:**
1. Check #358's new CI run, including the Linux and macOS logs and the sanitizer.
2. #339's Xwayland segfault (address 0xa0, backtrace inside Xwayland; logs `D:/gui_forms_w1/dev/c339_arm.log` around line 2849 and `c339_san.log`). Find which probe request triggers it, then fix it or work around it without weakening the gate. If it is an upstream Xwayland crash, report that with the evidence.
3. Carry on from the "PAUSED for org move" list above: C4 part 2's PR after #339, then C5 and C6, then the docs-conflict resolutions and MEASUREMENTS.

## RESUMED (morning, 2026-10-27)

- #362 (C2) merged.
- #358 was green on 171c891d, then went DIRTY after C2 merged. Rebased onto main: head f851b609.
  - The conflict was in windows_host.cpp: attach now reports compositor then input settings. WM_SETTINGCHANGE now runs report_input_settings() and then the compositor check; I removed the duplicate case label.
  - Local: 10 tests pass, the sanitizer passes over 5, the checkers are clean. Auto-merge is still set.
- #363 (C3) rebased onto main: head 329e4479. Conflicts resolved by `D:/gui_forms_w1/dev/resolve363.py`:
  - CMake: the Windows gate's if() is closed before the macOS one opens.
  - Docs table: macOS column from C3, Windows column from main.
  - Docs paragraphs: macOS, then Windows, then "X11 does not advertise yet".
  - Also fixed CI's macOS compile error: `Size` was ambiguous with MacTypes.h, so the gate now names `gui_forms::Size`.
- Next: #339's Xwayland segfault.
- The merge queue is on. `gh pr merge N --auto` (with no method) queues a PR; #358 is queued.
- #345 and #351 are merged, so the Mac bench sheet (above) goes to the owner through the coordinator.
- **#339 Xwayland crash, diagnosed.**
  - The step trace (0c758103, arm64 job 121751381168) shows Xwayland 23.2.6 under headless Weston 13.0.0 segfaulting at 0xa0 straight after the probe's XWarpPointer, in its first case.
  - Workaround (2b610c25): under Xwayland the probe warps only when Xwayland has made its pointer device ("xwayland-pointer:N" via XListInputDevices). Without one, `window_at_clear` reads "not exercised: the Xwayland seat has no pointer". The settings line gains `xwayland_pointer_device`.
  - The probe links X11::Xi, and `libxi-dev` is in install_linux_ci_packages.sh.
  - CI pending. Check that the settings line shows xwayland_pointer_device false, then record the XWayland readings in MEASUREMENTS C4.
- **C5:** #371, `claude/t5-c5-base` with main merged in. Local gate, sanitizer and checkers pass. Auto-merge set.
- **C6:** worktree `D:/gui_forms_w1/wt_c6`. C5 merged in (817d5244), then MEASUREMENTS C6 (cost and DIB hashes) committed as df83834b. Not pushed yet; local build and test running. The PR body is in `D:/gui_forms_w1/dev/c6_pr.md`: replace #C5PR with #371 and use base claude/t5-c5-base.
- **C4 part 2:** #375, `claude/t5-c4-x11` 81199516, with main merged in (docs: Windows and X11 paragraphs, then "macOS does not advertise yet") and then #339's branch. Its base is `claude/t5-c4-probe`. Auto-merge is NOT set. When #339 merges, retarget it to main (`gh pr edit 375 --base main`) and run `gh pr merge 375 --auto`.
- **C6:** #376, `claude/t5-c6-replay` df83834b, base `claude/t5-c5-base` (stacked on #371). Local gates pass (4 tests), the sanitizer passes (2), the checkers and the docs check are clean. Auto-merge is NOT set: once #371 merges, retarget it to main and run `gh pr merge 376 --auto`. Skia's hashes are first checked on its CI.
- The build checkout `D:/gui_forms_w1/gui_forms` is parked, detached at origin/main (c6-replay lives in `wt_c6`; c5 lives in `wt_c5`).
- **Next:**
  1. Read #339's CI. It should show xwayland_pointer_device false and pass. Record the XWayland and X11 readings in MEASUREMENTS C4 (on #375 or a follow-up).
  2. Retarget #375 and #376 when their bases merge.
  3. Check Skia's hashes on #376.
  4. Then C7 (reflection) and C8.

## LAND AND FREEZE (2026-10-27)

Owner instruction (`D:/gui_forms_workers/LAND_AND_FREEZE.md`): land #358, #363, #339, #371, #375 and #376, retargeting the stacked ones as their bases land. C7 is not to be started.
- #363: the macOS gate failed on "an opaque grant presents opaque pixels". Fixed in e499bae0: GUIFormsView draws with kCGImageAlphaNoneSkipLast while its window is opaque, and the backdrop apply sets needsDisplay.
- #375: the Linux gate failed on "an opaque grant presents alpha 255". Fixed in a1c51dbb: `NativeWindow::present_alpha`, set by the services' apply; under an opaque grant every pixel ORs the alpha mask in.
- #375's CI shows the XWayland probe passing (the #339 workaround works).
- The "rtf2md_markdown / x64" fuzz crash on #375 is not mine; reported to the coordinator.
- **Merged:** #358, #363, #339, #371 and #376.
- **#375:** retargeted to main and DIRTY. Merged main locally (da037d5c), keeping both sides:
  - compositor watch and XSettings watch;
  - CMake Linux and macOS gates;
  - docs table and paragraphs.
  The X11 paragraph now says alpha is presented only under `transparent`. Auto-merge set. The "rtf2md_markdown / x64" fuzz failure is not mine and has been reported.
- **#400** (`claude/t5-compositor-gate-default`, off main): gives the Windows compositor gate's `switch (grant.granted)` a stated default, fixing an F1 finding. The state-machine gate over the Windows tree showed the F1 before and no F1 or F2 after. Auto-merge set.
- Gotcha: `git stash`/`pop` in an env.sh shell shows a phantom whole-tree CRLF diff, because the system gitconfig sets autocrlf=true. Check status and stage from a fresh shell.
- **Next:** when #375 and #400 merge, write FROZEN.

## PHASE D STEP 1 (template_harness, 2026-10-26)

Coordinator task: move gui_forms' generic build tools into D:/malkuth/template_harness (local only, never pushed), parameterised by a TOML project file. Done; 9 local commits on main, ee28a6e..d3925fa.
- Tests: `python -B -m unittest discover -s harness -p "test_*.py"` from the repo root, Python314. Result: 237 pass, 1 skipped (symlink drift; this account may not create symlinks).
- Source: origin/main e13a02a4, copied as LF blobs with D:/gui_forms_w1/dev/phased/copy_from_main.py. Edit scripts are in the same folder.
- harness_testing.py sets HARNESS_PROJECT=projects/gui_forms.toml and makes a temp workspace with a one-commit gui_forms/ checkout.
- Left in gui_forms:
  - the ci_build driver and its tests;
  - stack_frames;
  - package_programs;
  - the test_areas.json and CMake consistency tests;
  - the workflow tests;
  - the fetch-pin tests;
  - the pinned-revision cross-checks;
  - the lsan.supp first-party test.
- gui_forms itself is unchanged. Migrating it to consume the harness is a later step.
- **Next:** none; wait for the coordinator.

## PHASE D STEP 2 (checkpoint tool, 2026-10-27)

The work is in the clone D:/gui_forms_w1/template_harness, on the local branch `checkpoint-tool`, which sits on top of d3925fa. It has two commits: 0d89385 (the tool, its tests and the gui_forms [checkpoint] table) and c5b8ff5 (README). Nothing is pushed; the coordinator folds the branch in.
- Tests: `python -B -m unittest discover -s checkpoint -p "test_*.py"`. All 15 pass in about 44 s, almost all of it spent starting git processes on Windows. The harness suite still passes.
- Decisions in force:
  - The [checkpoint] table is parsed by checkpoint.py, not project.Project, so W's work does not overlap.
  - The integration tree needs the git config mark `harness.checkpoint-tree=true`, set by `init`.
  - `--title` takes N=TEXT; a bare TEXT works only with a single topic.
- **Next:** none; wait for the coordinator.

## PHASE D STEP 3 (init, bootstrap, CHANGELOG, 2026-10-27)

The work is on the branch `bootstrap` in D:/gui_forms_w1/template_harness, off origin/main c5b8ff5, and is local only. It has four commits:
- c50eb4b: the project tables;
- ff50f87: restore_local;
- 4006a2a: the bootstrap;
- 3faf996: the CHANGELOG and contract.

Results and decisions in force:
- `checkpoint init` adopted D:/gui_forms_int/gui_forms. The tree stays on its `integration` branch with the mark set.
- The suites pass with Python314: harness (1 skipped) and checkpoint.
- lsan.supp stays in gui_forms.
- bootstrap.sh is thin: it finds Python 3.11 or later and runs harness/bootstrap.py. bootstrap.ps1 finds Git Bash.

**Next:** none; wait for the coordinator.

## PHASE D STEP 4 (harness.lock; gui_forms through the harness, 2026-10-27)

Part A is done and pushed by the coordinator: template_harness main = 0047da2 (1cfaa1b lock.py and the harness-checkout action, 0047da2 bootstrap pins the harness).

Part B: the clone D:/gui_forms_w1/gui_forms, branch claude/harness-pin off origin/main ab67c008, commit c953ed55. In it:
- harness.lock pins 0047da2.
- .github/actions/harness-checkout (gui_forms' own composite action): it reads the lock, checks the harness out at template_harness/, and sets HARNESS_PROJECT and HARNESS_WORKSPACE. Its input product-path ('.') is for the root-checkout stx jobs.
- Workflows call template_harness/harness/*.py directly.
- release.yml is touched at only five places, per the coordinator's option 1: the action step, the apt key, the gui-runtimes key and the prune job. The standby's claude/source-sdk PR owns release.yml, package_sdk.py and publish_tested_release.py; whichever PR lands second rebases.
- Shims: 18 Python files in tools/ (harness_link.SHIMS) plus install_linux_ci_packages.sh. Each one runs the harness tool when run and becomes the harness module when imported.
- Deleted: the moved tools' tests and windows_toolchain*.json.
- ci_build.py imports ci_build_steps. test_ci_build.py was trimmed to GUI.Forms' own parts.
- skia_archive MOVED_INPUTS keeps the identity unchanged (df654e49...).
- The house-style job runs the pinned harness's harness/ suite, because the harness has no CI of its own. It also runs tools/test_harness_shims.py.
- Docs: docs/building/template-harness.md and a RELEASE_NOTES "Build and CI" line.

Local verification:
- house-style: 0 findings over 1529 files;
- lifetime: 0 findings;
- touched tests pass;
- checkpoint fold into D:/gui_forms_int (5d203860), lock checkout of D:/gui_forms_int/template_harness, then verify (build.sh all).

Follow-ups for the coordinator:
- The [bootstrap] smoke in the harness TOML passes `--tests` without `--sanitize`, which ci_build rejects.
- [state_machines].full_tree_triggers should include harness.lock.
- File Manager's CI must check out template_harness beside gui_forms.
- 2026-10-27, later:
  - Added 3f081300: harness.lock and harness_link select every test (test_areas.json).
  - PR #408 is open with auto-merge on (head 3f081300). The cache-hit/reuse measurement on its first CI run is still owed to the coordinator.
  - The first verify failed gui_forms_windows_compositor_tests once at -j3. It passes alone and beside the probe. Fix af0c6faa (claude/t5-windows-compositor-serial) makes the Windows compositor test and probe RUN_SERIAL; it is PR #409, auto-merge on.
  - Fold of both topics, then verify: 267/267 passed. push --dry-run OK.
- **Next:** wait for the coordinator. On wake, read #408's first native run (the ccache stats in "Report cache statistics", and the ci_reuse lookup) and report the hit rate.

### Step 4 follow-ups (2026-10-27)
- template_harness branch `smoke-and-triggers`, commit bb28740113d9e8edfee55be09aecab60c2d143b7 (local in D:\gui_forms_w1\template_harness, off origin/main 0047da2; the coordinator pushes it). It adds harness.lock and tools/harness_link.py to `[state_machines].full_tree_triggers`, rewords the smoke comment, updates CHANGELOG and test_project_names. Harness suite passed.
- The smoke choice: the TOML command (`ci_build.py --tests ^gui_forms_core_tests$`) is unchanged; ci_build now accepts `--tests` without `--sanitize` as a plain narrowed build (build what the tests read, run ctest, no junit/install/examples). gui_forms commit ff42d3b2.
- #408 merge with main (#407, the source SDK): the toolkit configure/host functions moved to tools/toolkit_configure.py, which imports no harness, so the source SDK's build_sdk.py runs without template_harness (test_harness_shims covers it). Commit 4fee1b3f.
- release.yml prepare job: gui_forms checkout under gui_forms/, the harness beside it, prune test from the harness. Commit 48535a0c.
- Pushed #408 to 0dedd952 (lock bump to bb28740, now template_harness main).
- Owed: from #408's first native run, the ccache hit rate and the ci_reuse lookup result, to the coordinator.
- #408 merged (e077f9d5). Main push run 40467387623 succeeded and published v0.2.1308 with gui-forms-sdk-0.2.1308-source.tar.gz. stx_conv standalone (40467387469) failed on macOS from a runner network outage (checkout retries, artifact upload timeouts); its tests passed; not #408. Cache: 100% ccache hits on all platforms, reuse lookup missed (new tree), as on main's previous run.
- Adoption guide: template_harness branch `adoption-guide` off bb28740, commit ebb3a07 (local; the coordinator pushes): docs/adopting.md, README row, CHANGELOG line. Written from gui_forms' side only (no File Manager sources read). Open offer: a generic harness test that no two projects/*.toml prefixes collide.
- adoption-guide second commit 3f7f4d8: harness/test_project_prefixes.py (every projects/*.toml pairwise; exact toolchain sharing allowed, as example_app shares GUI.Forms'), guide gate names it, line-endings pitfall reworded (no autocrlf claim; no run log prints it). Harness suite 266 OK (1 skipped). Awaiting the coordinator's push.
- template_harness branch `product-at-root` off 3f7f4d8 (local; coordinator pushes): 64f6a35 (code+tests: source_dir ".", optional fields with project.MissingField, native_job template "{platform}" via Project.native_job_matches; bootstrap/checkpoint/lock root layout) and cf54dcb (adopting.md, contract.md, CHANGELOG). Harness suite 283 OK (1 skipped); checkpoint 19 OK. No File Manager sources read.
- Merged origin/main 7f773d9 (PR #1) into product-at-root: head f73a6f9; harness 284 OK (1 skipped), checkpoint OK, example_app 15 OK; test_project_prefixes sees example_app.toml, no collisions.

## RESUME: paused for the night (2026-10-27), bootstrap proof

Earlier on 2026-10-27 (template_harness): docs/adopting.md, the prefix-collision test, the seven File Manager fixes (branch `product-at-root`), and the merge of origin/main (PR #1). All of it was pushed by the coordinator as f73a6f9.

- **Branch:** `bootstrap-proof` in D:\gui_forms_w1\template_harness, off origin/main f73a6f9. Head is f73a6f9. Nothing is committed, nothing is uncommitted, nothing was pushed, and no background work is running.
- **Done (reading only):** restore_local.py, bootstrap.py/.sh/.ps1, install_linux_ci_packages.sh and gui_forms' native-platform.yml (saved copy: D:\gui_forms_w1\dev\np.yml).
- **Task (from the coordinator):** add .github/workflows/bootstrap.yml, with:
  - triggers: push to main, pull_request, workflow_dispatch;
  - matrix: windows-2022, macos-15, ubuntu-24.04;
  - an empty workspace, checking out only the harness itself;
  - the real bootstrap for `--project gui_forms`, without --in-house (bootstrap.ps1 on Windows, bootstrap.sh elsewhere);
  - assertions on the second run, which must be a no-op;
  - the apt and macOS runtime steps actually run;
  - each job timed;
  - a CHANGELOG line.

### Decisions in force

1. **Install steps run in the workflow, not through a flag.** gui_forms' harness.lock pins bb28740, and the bootstrap runs the pinned harness's restore_local, so a new flag on this branch would not reach a gui_forms bootstrap.
2. **example_app is not bootstrapped.** projects/example_app.toml has no [bootstrap] table, and notes has no example_app branch. Say so in the report.
3. **The workflow sets the build environment itself.** Neither ci_build nor bootstrap sets the CCACHE_* variables or the compilers. Copy them from np.yml:
   - CC/CXX/OBJCXX=clang, and the ccache launchers;
   - CCACHE_DIR and CCACHE_BASEDIR set to the workspace (cygpath -m on Windows);
   - CCACHE_COMPILERCHECK=content;
   - CCACHE_MAXSIZE=500M.
4. **Python and the token:** 3.14 at least on Windows (stdlib zstd); GH_TOKEN=github.token for restore_local's gh calls.

### Workflow plan

1. Check out the harness into `harness`. Set WS=$RUNNER_TEMP/ws.
2. **Run 1:** `bootstrap --workspace WS --project gui_forms --no-smoke`.
3. **Linux:** `HARNESS_PROJECT=WS/template_harness/projects/gui_forms.toml sh WS/template_harness/harness/install_linux_ci_packages.sh toolkit $RUNNER_TEMP/apt`.
4. **macOS:**
   - set HOMEBREW_NO_INSTALL_CLEANUP=1 and HOMEBREW_NO_AUTO_UPDATE=1, then `brew install cmake ninja ccache llvm@22`;
   - put $(brew --prefix llvm@22)/bin on the PATH;
   - build the runtimes with `build_macos_runtimes.py --work $RUNNER_TEMP/llvm-runtimes --prefix WS/.build/toolchain/llvm-22.1.8-macos14 --jobs 4`;
   - export GUI_FORMS_LLVM_RUNTIME;
   - open question: an actions/cache for the runtimes, with a dispatch input to force a clean build.
5. **Run 2:** the full bootstrap, with the smoke build. Tee its output to a file and assert:
   - "the product: updating", and no "the product: cloning";
   - "notes branch gui_forms: updating";
   - on Windows, "is already at";
   - "is already restored" (or "No published"), and no "Restored";
   - "Bootstrap finished".
6. **Also assert** with git:
   - the HEAD of WS/template_harness equals the commit in gui_forms/harness.lock;
   - the notes worktree is on branch gui_forms.
7. **Timing:** time each step with `date +%s` and write it to $GITHUB_STEP_SUMMARY.

### Exact next step

1. Write .github/workflows/bootstrap.yml to the plan above.
2. Add the CHANGELOG line.
3. Run `git diff --cached --stat`, then commit locally with the trailer.
4. Report to the coordinator, who pushes. The report covers:
   - the head;
   - decisions 1 and 2;
   - that the timings come from the first CI run;
   - the risks: the Windows smoke under native Python with the restored msys64 PATH, and the macOS runtime build cost.

## BOOTSTRAP PROOF COMMITTED (2026-10-27, resumed)

- **Commit:** `bootstrap-proof` head 0b72728 (local, in D:\gui_forms_w1\template_harness), on f73a6f9; waiting for the coordinator to push it as a branch.
- **Files:**
  - .github/workflows/bootstrap.yml;
  - .github/scripts/proof_step.sh: times one step into the summary and keeps its log;
  - .github/scripts/bootstrap_entry.sh: bootstrap.ps1 on Windows, bootstrap.sh elsewhere;
  - harness/bootstrap_proof.py and harness/test_bootstrap_proof.py (9 tests);
  - CHANGELOG.
- **Verified:**
  - the harness suite: 293 OK, 1 skipped;
  - `sh -n` on both scripts;
  - the timer, exercised once;
  - a dry run through the shell entry.

  Not verified locally:
  - the PowerShell route, because this machine's execution policy blocks scripts and I must not change it (GitHub runners allow scripts);
  - the YAML, because no parser is available.
- **Runs:**
  1. bootstrap-first, with `--no-smoke`, then `bootstrap_proof.py first`;
  2. the printed administrator step:
     - Linux: the sed-extracted install command, with HARNESS_PROJECT set;
     - macOS: brew, then the extracted runtime build, with the runtime in an Actions cache;
  3. bootstrap-full, then `bootstrap_proof.py again`;
  4. bootstrap-again, then `bootstrap_proof.py again`.
- **Findings for the report:**
  - restore_local's printed Ubuntu command omits HARNESS_PROJECT, which install_linux_ci_packages.sh needs;
  - Windows names HARNESS_PYTHON, in case another python3 shadows 3.14.
- **Next:** read the first CI run when the coordinator wakes me. Watch for:
  - the Windows smoke build under native Python with the restored msys64 PATH;
  - the cost of the macOS runtime build;
  - whether the cache assets exist for all three platforms.

## 2026-10-27, later: the lock bump and the proof fixes

- **gui_forms PR #411** (`claude/harness-lock-4437b47`, head 8171366): harness.lock bb28740 -> 4437b47b82455a2003b7858416eebcc68c0e91cf. Auto-merge is on. projects/gui_forms.toml is unchanged between the two commits.
- **template_harness `bootstrap-proof`:** b52e29f on top of 0b72728, local. The coordinator pushes it.
  - The first CI run failed on two over-strict checks:
    - macOS: Skia's "Fetching ninja" matched the harness-fetch rule. The rules are now anchored at the start of the line.
    - Windows and Linux: gui_forms' main moved during the job, so the third run restored the newer cache (build-62d8499). `bootstrap_proof.py snapshot` now records the state before each repeated run, and `again --before` allows moves while failing on repeats.
  - Timings from run 40556324843:

    | Host | Job | First run | Install step | Full run | Again |
    | --- | --- | --- | --- | --- | --- |
    | Windows | 335 s | 72 s | — | 236 s | 12 s |
    | macOS | 438 s | 17 s | brew 28 s, runtime 89 s | 290 s | did not run |
    | Linux | 483 s | 47 s | apt 29 s | 392 s | 8 s |

    macOS failed its check after the full run, so its repeat did not run.
- **template_harness `ubuntu-install-line`:** f63eb7d off origin/main 4437b47, local. restore_local's Ubuntu line now begins with `HARNESS_PROJECT=<project file>`. The coordinator pushes it; gui_forms gets it at the next lock bump.
- **Next:** read the rerun of PR #2 when woken. Bump the lock again once the install-line fix is on main, if asked.

## 2026-10-27, evening: lock 523c2a7, the Ubuntu time, the ignore list, the matrix

- **gui_forms PR #413** (`claude/harness-lock-523c2a7`, head b253175): harness.lock 4437b47 -> 523c2a7. Auto-merge is on. #411 (4437b47) already merged.
- **Why Ubuntu took 40 minutes** (PR #2's passing run 40563520737): the apt step ran 2049 s. It fetched 270 MB in 33m45s at 133 kB/s, against 7 s for the same 270 MB in the failed run. That is the mirror: azure.archive.ubuntu.com and apt.llvm.org together, and the buffered log could not split them. The full bootstrap took 314 s. The fix is on bootstrap-matrix: the deb cache, and streaming step output.
- **Local template_harness branches** (the coordinator pushes):
  - `ignore-toolchain`, d9e28de off main 523c2a7: ROOT_LAYOUT_ENTRIES adds `.toolchain/` and `.bootstrap/`. restore_local gains TOOLCHAIN_DIRECTORY and DOWNLOAD_DIRECTORY, a test, the adopting guide and the CHANGELOG. 31 tests OK.
  - `bootstrap-matrix`, stacked on bootstrap-proof b52e29f, with main merged (54b4f46), then:
    - 7de97ea: restore_local picks the project's Ubuntu profile (toolkit, else the only one; plan-paint has "paint");
    - 959a000: the plan job and matrix, unlocked products, the apt cache, streaming proof_step.
- **The plan today** (real gh):
  - gui_forms and swiftedit run;
  - books is skipped: no smoke command;
  - file_manager is skipped: the notes repository has no file_manager branch;
  - games is skipped: tools/harness_smoke.py is missing;
  - plan-paint is skipped: scripts/smoke-build.py is missing.
- **Next:** push bootstrap-matrix once PR #2 merges, merging main first if needed, then read its first run.

- 2026-10-27, late: merged main 9bfd428 into bootstrap-matrix; head 9210ef8 (both sides kept in CHANGELOG and restore_local). Harness suite: 355 OK, 1 skipped. The coordinator pushes it onto bootstrap-proof (PR #2).

- 2026-10-27, night: PR #2 matrix run 1 failed:
  - swiftedit and file_manager: the cache check did not know "Cache restore skipped" (neither publishes caches);
  - file_manager: needs its submodules; the bootstrap now runs git submodule update after a clone or fast-forward;
  - python-tests: test_lockstep had no git identity in CI.
  Fixed in 7c800e1 on bootstrap-matrix (local; the coordinator pushes it to bootstrap-proof). The suite passes without a global git config. SwiftEdit and File Manager bootstraps ran clean locally.

- 2026-10-27, night 2: PR #2 run 40581903489. The Ubuntu jobs for books, games and file_manager failed at the package step:
  - books and games: several profiles, none named toolkit;
  - file_manager: its lock pins 5c95a77, which prints toolkit.
  Fixed in b4614a5 (local): an optional [bootstrap] linux_packages field, Project.workspace_linux_profile(), and the plan now skips one host at a time. main 054389b is merged in. The suite passes without a git identity (362).
  The macOS jobs and file_manager on Windows had not run yet when I reported.

## 2026-11-03: C7 part 1 done, Mac bench rerun recorded (W4, hot-paths worker)
- **C7 part 1** (local, D:/gui_forms_w4/gui_forms, branch claude/t5-c7-reflection 8d9d6eea, on claude/stx-skia-avx2 0032b0b6): the reflection of a part's own face.
  - It is a material field, `SurfaceMaterial::reflection` (`MaterialReflection`: edge, gap, reach, start and end opacity), and the theme property `reflection: none | above|below|left|right [gap [reach]] [start [end]]`, with the edge words taken from -webkit-box-reflect.
  - Gap plus reach is at most 8 (the material outsets, T5-Q15). Text is not reflected (Q14). It is omitted under high contrast and reduced transparency (Q16).
  - The bake pass is apply_reflection, after the mask, and composites beneath. axis_reach covers it.
  - The bake weight of 55 was not measured; it was copied from the mask's.
  - Tests: engine_reflection_tests (8 reference hashes), the CSS lowering and refusal tests, and conformance (finish-light-images-masks, COVERAGE row).
  - Run: plain 98/98 and sanitized 98/98 over the engine, theme and abi regex; docs, schema, style, lifetime and gate.
- **C7 part 2** (the backplane source, glass reflecting what is behind it) is not started. It needs material-aware backplane margins: samples_backplane is computed in theme_engine.cpp (about line 974) before the material is resolved (about line 1120).
- **Mac bench rerun** (mac_handoff_results w4-2026-11-03/3-t5, main a5fdb2e8, macOS 26.7.2):
  - METHOD CHANGED from the sheet. macOS 26.7 kept every window of the probe's bundle id off screen after one ad hoc grant, so the runs used a fresh-bundle-id ad hoc copy launched with `--no-image-reads`, plus an outside capture program (t5_watch.c) whose readings were merged in.
  - `click_received` is none throughout; there is no Accessibility trust.
  - Criteria 0-3 pass in Light, Dark and Reduced.
  - Criterion 4: 8 of 12 materials tint. window_background, under_page_background, content_background and sheet draw OPAQUE on macOS 26.7 in both appearances. Under Reduce transparency every material is flat grey.
  - Criterion 5: window -> under_window_background and transient -> popover pass in Light and Dark; menu would pass too.
  - Criterion 6: an opaque window still shows the tinted backdrop (205,230,205 Light; 43,82,42 Dark).
- Next: C7 part 2, then C8 (a lab per host, the gate record per PLAN.md §6, docs).

## 2026-11-03: C7 part 2 (backplane source), bake weight measured
- `MaterialReflection::source` (`MaterialReflectionSource::face` by default, or `backplane`); the theme writes a final `backplane` keyword: `reflection: below 6px 30% backplane`.
- The engine (theme_engine.cpp) lends the backplane image at margin 0 to a material primitive part whose material reflects the backplane, notes the part as a backplane reader, and does not share the raster.
  - A part in the backplane plane reads nothing.
  - When the window lends no backplane, or one of another size, nothing is reflected.
  - The mirrored source rows lie inside the box, so margin 0 covers them.
- Slicing: a backplane source varies along both axes, so it is never sliced.
- Caching and damage reuse glass's machinery:
  - backplane_generation is in the cache match;
  - damage_backplane_readers damages only the readers whose extent meets the damage.
- Gate (`gui_forms_engine_reflection_tests`):
  - 16 reference hashes, covering face and backplane, each edge, at x1 and x2;
  - mirrors_the_backplane;
  - follows_backplane_damage: rebaked on damage under it, not on damage elsewhere, and omitted under reduced transparency;
  - the CSS refusal `backplane` not last;
  - a conformance case (track-bar thumb).
- Bake weight: reflection is now charged per reflected pixel (reach in device pixels, rounded up, times the destination side along the edge) at 70.
  - Measured by a paired measurement, `--measure` (101 interleaved bake pairs, median difference): 9 runs gave 56-98, median 68. The machine was busy.
  - The pass is about 12 ns per reflected pixel, in floating point.
- Commits: 93e44031 (weight), ebff7702 (backplane source), on claude/t5-c7-reflection. Plain 98/98, sanitized 98/98; docs, style, lifetime, gate, schema and stx style all clean. The owner accepted the `reflection` vocabulary.
- Next: C8, meaning a compositor lab per host, the gate record (PLAN.md §6) and the docs (hosts-and-capabilities.md capability table, a user page on transparency, the T5 README, the stage board).

## 2026-11-04: C8 committed (W4, hot-paths worker)
- claude/t5-c8 0968c952 on cp16 (checkpoint/2026-11-04-2, 8b7b470a), local in D:/gui_forms_w4/gui_forms.
- Lab: demo/compositor_lab.{hpp,cpp} + mains for Windows/macOS/X11 (gui_forms_compositor_lab[_windows|_linux]); --backdrop list; JSON record per grant and once at host ready.
- Test gui_forms_compositor_lab_tests (Skia raster, 6 states x 4 requests, stop/start). Lesson: a raster test must set_framebuffer_painter(&raster) as hosts do, or the backplane falls back to part rasters only (fill_rect scenery skipped; backplane reflections then mirror only the base).
- Windows compositor test now prints dwm_backdrop_type and extended_frame.
- tools/t5_gate_record.py; record in notes results/2026-11-04-t5-compositor (Windows 11 granted + headless all states); pending: Server 2022, transparency off, HC (owner settings), macOS (handoff), X11 (CI/Xvfb).
- Docs: docs/user/transparency.md (capability table already complete in hosts-and-capabilities.md); notes T5 README status updated (uncommitted, notes repo). Stage board: coordinator's.

## RESUME POINTER (2026-11-04, backdrop render side)
- Branch claude/t5-backdrop 7b0c5ce8 (on claude/t5-c8 0968c952), D:/gui_forms_w4/gui_forms. Not pushed.
- Done: BackdropChain/Operation/Statistics (include/gui_forms/window/backdrop/backdrop_chain.hpp); Window::set_backdrop_chain (blur -> implied material request, tints as base); popup backdrops (PopupOptions::backdrop; src/core/window/backdrop/{backdrop_image,backdrop_cache,popup_backdrop}.cpp): tile cache (64 px) per PopupAttachment, invalidated in add_damage except popup-subtree damage (popup_damage_in_progress_), tiles under opaque popup content skipped, wholly covered -> occluded; fallbacks (reduced transparency / compositor absent / no framebuffer painter -> solid tint fill; high contrast -> none); opaque engine faces report covers (paint_part_with_built_in_recipe, friend of Control); RecordingPainter keeps the larger cover.
- Verified: plain + sanitized 15/15 (compositor_lab|display_chunk|part_painting|core|compositor_backdrop|engine_backdrop|popup|content_pixels|backplane_replay|canvas_raster|control_snapshot); docs, style, lifetime, gate (base origin/checkpoint/2026-11-04-2) clean.
- Measured (notes results windows-11-26200-popup-backdrop-cost.jsonl, lab_tests --measure): cached ~= occluded ~= no-backdrop + popup draw; recompute 256x192: blur0 +4.1 ms, blur8 +9.1, blur32 +19.3 (85-390 ns/px incl. owner replay into the framebuffer).
- Next: native floating owned windows (replay owner chunks with frame offset); top-level CPU saturate step if wanted; possible recompute speedup (replay only the strip union, reuse the source raster).

## RESUME POINTER (2026-11-04, before the restart: overlays, native popups phase (a))
- Supersedes the pointer above. Workspace D:/gui_forms_w4/gui_forms. Nothing pushed.
- claude/t5-backdrop 398b130c (on f5e7abb1, on t5-c8): owner ruling that in-surface "popups" are OVERLAYS (our own pixels): they paint with the theme's materials, take no backdrop chain. Removed PopupOptions::backdrop, popup_backdrop.cpp, BackdropStatistics, the popup tint pass; window chain + no-sampling rule (nothing outside our surface) stay. Verified: plain + sanitized 7/7 (compositor_lab|display_chunk|compositor_backdrop|engine_backdrop|popup|tooltip|menu), docs, style, lifetime, gate (base origin/checkpoint/2026-11-04-2) clean. Report this head to the coordinator if not yet done.
- claude/t5-native-popups b2148095 (on 398b130c): WIP, UNBUILT. Phase (a) seam per the approved plan: HostWindowKind::popup + traits.popup, HostPopupFocus {passive, takes_focus}, host_window_kind(...,bool popup) (native adapters pass false), virtual HostWindowing::supports_popup_windows (false; headless true; admit refuses popup unsupported), ApplicationPopup {focus, origin} on ApplicationWindow (validate refuses popups in the initial set; runtime refuses when unsupported), detail::attach_application_window at ready (runtime WindowBridge::notify, headless open), SurfaceRoot::open_native_popup(owner, content, bounds, NativePopupOptions) -> NativePopupToken (include/gui_forms/window/native_popup/native_popup.hpp; record in src/core/application/native_popup{_record.hpp,.cpp}; src/core/window/native_popup/native_popup_window.cpp), overlay fallback or refuse; headless places at origin, undecorated frame, traces "kind=popup focus=...". Tests: tests/native_popup_tests.cpp (target gui_forms_native_popup_tests). Patch script dev/tools/native_popup_seam.py, drafts dev/staging/.
- Next: build (build_slot / verify_win.sh 'native_popup|floating_windows|runtime_windows|application' with GATE_BASE=origin/checkpoint/2026-11-04-2), fix compile errors, run, amend into a clean commit, report the head. macOS file edited blind (macos_application_run.inc, 4th arg false): M4 must build it. Then passive key routing and dismissal; phase (b) Windows; (c) TaskDialog/FontDialog/gallery toggle; (d) macOS/X11 handoffs.
- NAMING RULE (coordinator, 2026-11-04): the semantic-tests worker renames the in-surface mechanism to OVERLAY on main for cp18 (open_popup->open_overlay, PopupOptions->OverlayOptions, PopupAttachment->OverlayAttachment, PopupToken->OverlayToken...). "Popup" now means only the native owned-window kind. Keep native names separate (HostWindowKind::popup, NativePopupOptions, NativePopupToken); never touch the in-surface names; the native surface choice and focus live on NativePopupOptions. The only in-surface references in my branch: NativePopupToken::overlay_ (PopupToken) and the fallback call open_popup(...PopupOptions) in native_popup_window.cpp; the coordinator restacks me onto the rename.
- Later: R-8 resize cost after W's M0.4; explain 21.2 wakes/s on ranges-and-progress.

## RESUME POINTER (2026-11-05, phase (a) verified; phases (b) to (d) PARKED)
- Supersedes the pointers above. Workspace D:/gui_forms_w4/gui_forms. Nothing pushed.
- claude/t5-native-popups d041b49f (on claude/t5-backdrop 398b130c): native popups phase (a), one clean commit. Seam as in the previous pointer, with open_native_popup on Window (not SurfaceRoot) and explicit host outcomes: NativePopupToken::refusal() -> NativePopupRefusal {none, no_run, unsupported, host_refused}.
- Sub-surface rule (owner, 2026-11-05, revised): nothing refuses; a sub-surface is not a root, so it has no capability. open_native_popup, its token and options belong to Window only. mark_sub_surface, the sub_surface refusal and its test were removed. Controls wanting a popup or tooltip go through their root's request API (the C ABI worker puts it on SurfaceRoot); the Window implementation may call open_native_popup.
- Verified on Windows (d041b49f): plain 5/5 (native_popup|floating_windows|runtime_windows|application_contract|headless_trace), foreground application_native 1/1 on a private desktop, docs (regenerated, committed), house style, lifetime, gate (base origin/checkpoint/2026-11-04-2) clean. Sanitized: left to the fold verify. macOS file (macos_application_run.inc, 4th arg false) edited blind: M4 must build it.
- Tools: dev/tools/np_run.sh (build slot -> np_build.sh: reconfigure, touched targets with -k 0, ctest), np_checks.sh (foreground + checkers; GATE_BASE env), np_subsurface_patch.py (superseded by np_window_move.py).
- PARKED (owner, 2026-11-05, to free CPU): phases (b) to (d) wait until W's M3 lands. Builds use only the shared build slot (build_slot.py; slot 0 is the critical path's).
- Phase (b) plan, when unparked (Windows host): add std::optional<ApplicationPopup> popup to Windows/Mac/LinuxApplicationWindow and copy it in runtime prepare_one; WindowsApplicationRun::supports_popup_windows() true; open(): kind via host_window_kind(..., entry.popup.has_value()); styles WS_POPUP, ex WS_EX_TOOLWINDOW | (passive ? WS_EX_NOACTIVATE : 0); place with request_frame_impl(id, {origin, size}) before showing; show SW_SHOWNOACTIVATE (passive) or SW_SHOWNORMAL (takes_focus); WM_MOUSEACTIVATE -> MA_NOACTIVATE for passive (flag on WindowsHostState set before CreateWindowExW). Alpha: no layered window; the popup model requests WindowBackdrop::transparent (when no chain) in open_native_popup_window, and the existing blur-behind/empty-region path plus prepare_frame_alpha keeps the WM_PAINT damage pipeline. Test: a popup case in tests/application_native_tests.cpp (foreground label). Also still open in (a)-scope: passive key routing (owner keeps focus, core routes keys to the newest passive popup) and dismissal.
- Then (c) TaskDialog, FontDialog, gallery toggle; (d) macOS/X11 handoffs (X11: override-redirect for passive, managed transient for takes_focus).

## RESUME POINTER (2026-11-05, unparked: phases (b) and (c) in the tree, unbuilt)
- Supersedes the parked pointer. M3 landed; coordinator unparked (b) Windows host and (c) TaskDialog, FontDialog, Gallery toggle; macOS/X11 hardware go to the Mac handoff.
- Rebased onto origin/main ebb9fbf4 (cp17) as two commits on claude/t5-native-popups: 20a8a1ff (backdrop chains, net of 7b0c5ce8+f5e7abb1+398b130c; compositor_lab_tests took main's default cases) and 338b5e81 (phase (a)). Old head kept as claude/t5-native-popups-pre-main (d041b49f). Restack again after cp18 (overlay rename, overlay ownership rule, S4 motion_* changes).
- Uncommitted in the tree (patch scripts in dev/tools): np_windows_patch.py (HostPopupPlacement replaces ApplicationPopup; native entries carry popup; runtime copies it; Windows run: supports_popup_windows true, WS_POPUP + WS_EX_TOOLWINDOW (+WS_EX_NOACTIVATE passive), placed by request_frame_impl before show, MA_NOACTIVATE for passive; popup model requests the transparent backdrop without a chain), np_native_test_patch.py (popup case in application_native_tests, Win32 style/owner/placement checks), np_closed_event_patch.py (NativePopupToken::closed_event; record made before the window, NativePopupClosed callback), np_dialogs_patch.py (DialogSurface {overlay, native_popup}; TaskDialog/FontDialog set_surface, presented_surface; native root holds the dialog so closing detaches at once; modal layer + empty focus scope stay in the owner), np_gallery_patch.py (checkbox gallery.host.task-dialog.native), np_dialog_tests_patch.py (headless dialog tests in native_popup_tests; links gui_forms_controls).
- Next: build/test via dev/tools/np_run.sh (shared slot), np_checks.sh (foreground + checkers), commit as phase (b) and phase (c), report. Follow-ups: our shadows in the popup's alpha surface (bounds outset), passive key routing.

## RESUME POINTER (2026-11-05, before the reboot: phases (b)+(c) WIP committed)
- Supersedes the pointer above. Workspace D:/gui_forms_w4/gui_forms, branch claude/t5-native-popups (on origin/main ebb9fbf4): 20a8a1ff backdrop chains, 338b5e81 phase (a), 693beac9 WIP phases (b)+(c) (Windows host popup windows, HostPopupPlacement, NativePopupToken::closed_event, DialogSurface on TaskDialog/FontDialog, Gallery toggle gallery.host.task-dialog.native, headless dialog tests). Old head: claude/t5-native-popups-pre-main d041b49f. Nothing pushed.
- Verified at 693beac9 minus the last test edit: plain 17/17 (np_run.sh set incl. task_dialog, font_dialog, gallery_pages, semantic_gallery); docs regenerated + committed, house style, lifetime, gate (origin/main) clean.
- OPEN FAILURE: foreground gui_forms_application_native_tests: the native popup opens (surface native, refusal none), but the Win32 check failed ("undecorated, non-activating tool"). The committed test now looks only for WS_POPUP windows owned by the main window ("GUI.Forms runtime windows fixture") with separate requires (found / exactly one / styles) and is UNRUN. Next: build_slot + dev/tools/np_fg.sh (rebuilds the one target, runs it on a private desktop); if the style check still fails, print GWL_STYLE/EXSTYLE (suspects: WS_EX_NOACTIVATE dropped by CreateWindowExW path, or WS_CAPTION bits from adjust paths) and fix in windows_application_run.inc open()/windows_window_styles.
- Then: squash the WIP into commits "phase (b)" and "phase (c)", report the head to the coordinator, restack after cp18 (overlay rename touches open_popup/PopupToken in task_dialog.cpp, font_dialog.cpp, native_popup_window.cpp). Follow-ups: our shadows in the popup alpha surface (bounds outset), passive key routing; (d) macOS/X11 to the Mac handoff.

## RESUME POINTER (2026-11-05, on the Mac: w4 = ~/Developer/gui_forms_workers/w4/gui_forms)
- Supersedes the pointer above. Windows remote gone; builds on the M4 via bin/m4run w4. Branch claude/t5-native-popups checked out from backup/claude/t5-native-popups (693beac9). The laptop's claude/macos-a11y-snapshot (0bb7a572) is left alone.
- Windows: the foreground application_native Win32 style check is "Windows: unverified, CI or a later Windows host".
- Tools: dev/tools/np_mac.sh (copy to gui_forms/.np_mac.sh, untracked, since m4ws syncs only gui_forms/): `M4WS_WAIT_IDLE=180 m4run w4 -- sh .np_mac.sh plain|san` = touched tests (native_popup|floating_windows|runtime_windows|application|headless_trace|task_dialog|font_dialog|gallery|semantic|doc_examples|binding, --gallery) + ninja -k 0 all targets; logs ../np-*.log on the M4 (read with m4run w4 --log ../np-plain.log).
- Phase (d) macOS in the tree (uncommitted): GUIFormsPopupPanel (macos_host.mm, borderless + non-activating NSPanel, canBecomeKey only for takes_focus), macos_application_run.inc (kind from entry.popup, min size 0, placed by request_frame_impl before show, makeKey only for takes_focus, supports_popup_windows true, abandon_open helper), application_native_tests macOS branch expects native surfaces.
- Next: plain run of 693beac9+(d); squash into commits phase (b), phase (c), phase (d); sanitized run; checkers + gate; report. Then C8 with the macOS compositor lab (stx_skia track).

## RESUME POINTER (2026-11-05, phases (b)-(d) committed on the Mac)
- Supersedes the pointer above. claude/t5-native-popups in ~/Developer/gui_forms_workers/w4/gui_forms (on origin/main ebb9fbf4): 20a8a1ff backdrop chains, 338b5e81 (a), 09a5961c (b) placement + Windows host (+ closed_event, DialogSurface enum), 66b1ef28 (c) dialogs + Gallery toggle + states-and-diagnostics digest (the toggle is one control more in the metrics line), 5f151c89 (d) macOS GUIFormsPopupPanel, aade809f probe switch defaults (gate). Nothing pushed.
- Verified on the M4 at 5f151c89: plain 26/26 and sanitized 26/26 (np_mac.sh set, incl. application_native activated, gallery display trace, macOS a11y client), all targets build, docs check, schema check + render check; house style + lifetime clean (run locally, pure Python). Gate: 2 F1 findings in tests/macos_message_dialog_key_tests.mm (main's file) -> aade809f; gate rerun pending in the C8 run.
- Windows: the application_native Win32 popup checks are unverified (CI or a later Windows host). Linux: CI only.
- Lesson: gallery_display_trace_tests catches any new Gallery control (metrics line on states-and-diagnostics); refresh with dev/tools/np_digest.sh.
- Tools (dev/tools, copy to gui_forms/.<name>.sh to run): np_mac.sh, np_checks.sh (san + docs + schema + gate), np_digest.sh, np_trace.sh (trace diff vs integration tree), np_c8.sh (macOS gate record: lab per request + macOS compositor test -> ../t5-gate-macos-granted.jsonl on the M4).
- Next: C8 macOS record into notes results/2026-11-04-t5-compositor (coordinator commits notes); restack after cp18 (overlay rename touches task_dialog.cpp, font_dialog.cpp, native_popup_window.cpp); then the glass recipe (stx_skia track).
- (later, same day) aade809f verified: gate rerun "no F1/F2 findings" (probe target builds). C8 macOS recorded on the M4 (macOS 26.7.2, 25G332, state granted): window_material 72..255, transient_material 104..255, transparent 0..255, opaque 255..255, each granted, exit 0; macOS compositor test exit 0; PASS. File: ~/Developer/gui_forms_workers/w4/dev/results/macos-26-granted.jsonl, for notes results/2026-11-04-t5-compositor (gui_forms branch; the coordinator commits notes). Still pending for the gate: macOS transparency-off / high contrast (owner settings), Server 2022, X11.
- Head reported to the coordinator: claude/t5-native-popups aade809f. Next: glass recipe after the cp18 restack.

## RESUME POINTER (2026-11-05, cp18 restack done; glass recipe started)
- cp18 landed (main 4ab9a107). claude/t5-native-popups restacked: 6fcced5f (b), 13828de0 (c), 6287627a (d), 216945a9 probe defaults (head). The backdrop-chain commit and phase (a) were already on main (85762b82..f50f7bcb, 8e272d0e) and were skipped. Overlay rename applied to the dialogs (overlay_layer_/overlay_token_/overlay_scope_/release_overlay/on_overlay_revoked/overlay_). states-and-diagnostics digest re-recorded (0xec2f44c215c67dad). Old head kept as claude/t5-native-popups-pre-cp18.
- Verified on the M4 (dev/tools/np_restack.sh): all targets build; plain 26/26 after the digest; sanitized 26/26; schema check + render check; gate no F1/F2; docs check (local) PASS; style + lifetime clean.
- Glass recipe: branch claude/t5-glass-recipe from origin/main (uncommitted at this point): glass `layers` choice {filter (default, unchanged), material}; material mode draws the recipe material's inside layers (fills, inset shadows, over glows, border, edges, keylines; mask) over the filtered backdrop in place of the tint; backdrop takes the material's corner; accepts() refuses missing/invalid material, image/procedural fills, image mask, bake past bound; glass descriptor 0.3. Tests: the_glass_recipe in tests/engine_glass_tests.cpp (hash recorded on the M4 by dev/tools/glass.sh). Owner decisions taken conservatively: opt-in `layers` (so existing recipe-as-filter themes are unchanged); no glass outsets, so outer shadows/under glows/drop shadow/reflection are not drawn; per-corner radii not matched by the backdrop clip.
- BLOCKED (2026-11-05 11:16): the M4 disk is full (121 MiB free of 228 GiB; ccache "No space left on device"). Glass WIP committed unbuilt as the head of claude/t5-glass-recipe (hash placeholder 0x0). w4 on the M4 holds 13G, including stale hot-paths trees .build/native-macos-arm64/hp-out (2.0G), gui-forms-profile (278M), hp-cpu (128M) that I did not delete (they may hold measurements). Next once space is back: sh dev/tools/glass.sh via m4run (records the hash on the M4 copy: copy it into tests/engine_glass_tests.cpp locally), check docs/controls/generated/schema.md diff from control_schema.py render (glass 0.3, new `layers`), amend into one commit, report.
- Coordinator (2026-11-05): `layers` stays opt-in; no outsets and single-radius clip are accepted limitations (in the commit message and transparency.md; follow-up if the owner wants the full Windows 7 look); leave the hp-* trees. Waiting for the coordinator's "space is free", then glass.sh, squash, report. Glass head now b/c amended (see git log claude/t5-glass-recipe).

## RESUME POINTER (2026-11-05, stopped for the app restart)
- claude/t5-native-popups 216945a9: verified on cp18 (main 4ab9a107) and reported; ready to fold.
- claude/t5-glass-recipe (from origin/main 4ab9a107): f47704b9 WIP glass recipe + a WIP commit recording the material-glass hash 0xd5a8856b87b8bab5 (macOS arm64). Not squashed.
- What ran (dev/tools/glass.sh on the M4, log dev/logs/glass2-*): engine_glass passes with the hash; plain 73/75 (engine_glass failed only on the placeholder hash, now fixed; gui_forms_gallery_pages_tests FAILED, cause not yet read); sanitized 74/75 (gallery_pages_tests only); schema render showed no diff; schema check, gate (no F1/F2) clean.
- Next: read the gallery_pages_tests failure in ../glass-plain.log on the M4 (grep -A8 "Test #301"); likely the control schema / primitive descriptor (glass 0.3, new `layers`) in manifests/control-schema.json or docs/controls/generated/schema.md. Fix, rerun glass.sh, squash the two WIP commits into one "T5: glass recipe" commit, report the head. Create no new M4 build trees beyond the plain and sanitize ones.

## RESUME POINTER (2026-11-05 evening, cp20: glass recipe done; C8 draft written)
- claude/t5-glass-recipe = one commit on origin/main 4ab9a107 (see git log; "T5: glass recipe ..."), reported. gallery_pages_tests had failed on the stale manifests/control-schema.json (new `layers` parameter); fixed by hand plus control_schema.py render. Glass stays introduced 0.2 (descriptor.introduced is the introducing version, not a revision).
- M4 environment issue: gui_forms_macos_accessibility_client_gallery_tests fails since about 16:00 ("the app wrote no semantic dump") in w1, w7 and w4 runs alike, even idle-gated; reported to the coordinator.
- C8 draft: ~/Developer/gui_forms_workers/w4/dev/results/README-macos-section.md + macos-26-granted.jsonl, for notes results/2026-11-04-t5-compositor (the coordinator places them). Waiting for the coordinator's Reduce transparency / Increase contrast / Dark data, then add rows.
- claude/t5-native-popups 216945a9 unchanged.
- (later) The a11y client failure was the external drive (apps launched from it are blocked). Rerun on dc773537, plain tree on the internal disk, M4WS_WAIT_IDLE=180: gui_forms_macos_accessibility_client_gallery_tests 1/1 passed. Rule from now: launched-app macOS tests run plain only (sanitize trees stay on the drive).

## RESUME POINTER (2026-11-07, cp19 popup check fix)
- claude/t5-popup-check-fix 9ee8bcd5 on origin/checkpoint/2026-11-05-4 (835c3599): application_native_tests' Win32 popup lookup keeps only WS_POPUP windows of the owner's window class (the thread's default IME window is an owned WS_POPUP of the active window: the likely second "popup", and the earlier style failure); the mismatch message lists every owned window (class, style, ex style); main catches failed checks and returns 1 instead of aborting. Host unchanged.
- Verified on the M4: all targets build; plain 26/26 of the np_mac.sh set (incl. application_native activated). Windows: CI proves it.
