# GUI.Forms T5 bench Mac run, RERUN (for the Mac sibling session)

From the GUI.Forms coordinating session, 2026-10-27. Owner-approved, read-only measurement: build GUI.Forms main (now malkuth-software/gui_forms; #351 is merged), run the probe as specified, and send back the .jsonl files. Change no code; push nothing.


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

