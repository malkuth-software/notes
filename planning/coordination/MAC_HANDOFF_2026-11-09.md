# Mac handoff, 2026-11-09

From the Windows coordinator. Build and test on the M4 with the usual recipe
(memory: m4-build-recipe). Report results to `malkuth-software/mac_handoff_results`
(one file per item, plus the exact commands). All the branches below are pushed to
`malkuth-software/gui_forms`. Never push to main.

## 1. #474 macOS accessibility failure (urgent, blocks cp24)

- Base: `checkpoint/2026-11-09-cp24`, now at c6b78b4b6 with the fix ("settled" = no value moving on the clock; endless ambient effects no longer block it).
- Failing test: `gui_forms_macos_accessibility_client_gallery_tests`. After pressing
  `gallery.page.spinners-and-rings`, the page never reports settled within 10000 ms
  ("motion armed yes"). Probably the gui-forms default theme keeps spinner motion armed.
- Wanted: reproduce it locally, and say whether the page settles with motion frozen. The
  Windows integrator is writing the fix; when its head appears on the PR, rerun this test on it.

## 2. AppKit paths that no CI or Windows build exercises interactively

Branch `claude/abi-focus` (65e4f4d2), stacked on `claude/abi-window-properties` (9d14d219)
and `claude/abi-modal-windows`:
- Window properties written before the run: they reach the NSWindow at host attach
  (no zoom button, fixed frame, top-most level, start position). Icon and taskbar
  report unsupported on AppKit; confirm that.
- Modal windows during a run: open, then close. The owner window gets key focus back
  natively, and the last focused control is focused again.
- Focus: composites (ComboBox, NumericUpDown, SearchField and the others) take focus
  in their edit part. Tab order and the focus ring are correct.

Branch `claude/host-file-gaps` (1744d23b3):
- Finder "Open With" / dropping files on the Dock icon (`application:openURLs:`): the files
  reach Application as a queued open-files event, during and before the run.

## 3. Standing post-0.5 Mac list

On current main (c9b11f0a, cp22 merged):
- Smooth-scroll feel (trackpad pixel deltas are immediate; momentum; rubber-band at the edges),
  on the Gallery's long pages and a ListView.
- Cmd-Q, logout/restart while running (no hang, and the save prompt is honoured), live resize
  (no tearing or stale damage).
- The C8 macOS rows in the closure checklist.
- `engine_motion` tile-fade hash: the cp24 re-record uses one constant for every host, and
  the Mac must match it (cp24 branch).

## 4. Optional: cp25 work in progress

`claude/cp25-wip` (5a9019f18): cp24 plus the keys/numeric ABI and TextBox styled runs.
Unbuilt. A macOS build plus a plain suite run here would catch .mm breakage early.

## 5. Added 2026-11-10: ThreadSanitizer run (audit N9)

Run the ABI sub-surface tests under ThreadSanitizer on the M4 (or Linux) against the combined tree (fold/bleeding-edge, which contains claude/abi-seal-fixes 5deb8e2b; the event stack is now thread_local). The Windows clang64 toolchain has no TSan.
