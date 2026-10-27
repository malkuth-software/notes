# GUI.Forms T5 bench Mac run (for the Mac sibling session)

From the GUI.Forms coordinating session, 2026-10-26. Owner-approved. Read-only measurement: build GUI.Forms main, run one probe three times, send back three .jsonl files. Change no code; push nothing.


Reduce transparency is on on the macos-15 runner, so CI shows only
AppKit's solid fallback. The bench run measures the real materials with
the probe that is on main since #338.

- **Build** (the GUI.Forms repository at main, on a Mac with Xcode's
  command-line tools):

  `GUI_FORMS_ENABLE_MACOS_HOST` is on by default on Apple, and it needs
  the Skia adapter. The simplest dependable route is CI's own build, from
  the directory that holds `gui_forms/`. It prepares Skia and the other
  third-party trees, then builds and tests everything. That takes a long
  time, and other tests' results don't matter for this run.

  ```text
  python3 gui_forms/tools/ci_build.py --jobs 8
  ```

  The binary is then
  `.build/native-macos-arm64/gui-forms/gui_forms_macos_compositor_probe`.
  Run it directly as below, because ctest hides its output. With the build
  tree already made, rebuilding only the probe is
  `cmake --build .build/native-macos-arm64/gui-forms --target gui_forms_macos_compositor_probe`.
- **Before running:** System Settings > Accessibility > Display > Reduce
  transparency: **Off**. Increase contrast: Off. Appearance: Light. One
  display. Leave the machine alone for about 60 seconds. It shows a green
  window and a probe window, and posts one click per case.
- **Run:** `<binary> > mac-bench.jsonl`.
  Repeat once with Appearance set to Dark (`mac-bench-dark.jsonl`), then
  once with Reduce transparency **On** (`mac-bench-reduced.jsonl`), to
  compare against CI.
- **What it prints:** 18 case lines, then one settings line. Each case
  line holds:
  - `clear`, `half` and `opaque`: the composed screen;
  - `clear_inactive`: the clear column with another window key;
  - `own_*`: the window's own image with alpha;
  - `window_at_clear` and `click_received`;
  - `effect`: material, blending, state, appearance.
- **Pass criteria** for the Light and Dark runs:
  1. The settings line has `"reduce_transparency":false`.
  2. `clear`, `clear_receiving` and `effect_then_clear`: `clear` is
     (0, 160, 0) ±12. Non-opaque clear pixels show what lies behind
     (already true on CI).
  3. `clear_receiving` has `"window_at_clear":"probe"`, and `clear` has
     `"backdrop"`. This shows that `ignoresMouseEvents = NO` is what keeps
     clicks on the window (T5-Q6).
  4. Each material case (`window_background` to `tool_tip`) has
     `"material_shown":true`. Its `clear` is not the flat grey CI
     recorded (for example 236, 236, 236), and shows the green behind,
     blurred and tinted: green exceeds both red and blue by at least 8.
     The case's `clear_inactive` may differ from `clear`, because the
     state follows the window's active state.
  5. **The mappings to confirm** (PLAN.md §2.2):
     - `window_material` maps to `under_window_background`;
     - `transient_material` maps to `popover`, with `menu` the
       alternative.

     A mapping passes when its case meets criterion 4 in Light and Dark.
     Where `popover` fails and `menu` passes, the mapping becomes `menu`.
  6. `under_window_background_opaque_window` shows whether an effect view
     composes behind an opaque window too. It is recorded, not judged.
- Send back the three .jsonl files. A screenshot during a material case
  is welcome but not needed.

