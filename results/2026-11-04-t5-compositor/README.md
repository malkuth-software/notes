# T5 compositor gate runs, 2026-11-04 (phase C8)

Stage T5's gate record (planning/stages/T5-compositor/PLAN.md §6), from the
compositor lab added in phase C8 (`demo/compositor_lab.hpp`): a window
declared non-opaque, with scenery in its backplane, a glass region, a face
reflecting itself and a face reflecting the backplane. Built from
`claude/t5-c8` on checkpoint/2026-11-04-2 (Release, the pinned clang64
toolchain, the Skia raster). The 2026-10-25 directory keeps the C2 runs.

## Windows 11, build 26200 (Shadow Windows): granted state

Light theme, transparency effects on, high contrast off; a streamed
desktop (the frame alpha comes from the presented framebuffer, not the
screen).

**MEASURED, the lab:** `windows-11-26200-granted-lab.jsonl`, from

```text
python -B tools/t5_gate_record.py --lab ".build/native-windows-x64/gui-forms/GUI.Forms Compositor Lab.exe" \
    --out windows-11-26200-granted-lab.jsonl --state granted --launcher "$GUI_FORMS_FOREGROUND_LAUNCHER"
```

(each run on a private desktop through run_on_desktop.py). The first line
names the machine; each run line carries the lab's record (the host's
`HostCapabilities::to_json`, the `CompositorState`, the request, the grant,
its reason and generation) and the captured frame's alpha. The frame is
1120 x 720 (the 560 x 360 window at scale 2).

| Request | Grant, reason | Frame alpha min..max | Non-opaque pixels |
|---|---|---|---|
| `window_material,transparent` | `window_material`, granted | 72..255 | 373835 |
| `transient_material,transparent` | `transient_material`, granted | 104..255 | 373774 |
| `transparent` | `transparent`, granted | 0..255 | 374003 |
| `opaque` | `opaque`, granted | 255..255 | 0 |

The minimum is the backplane base of each grant: the window material's
tint (alpha 72), the transient material's (104), clear over transparent.
Every run exited 0 with its frame saved.

**MEASURED, the host's read-back:** `windows-11-26200-granted-host.jsonl`,
from `gui_forms_windows_compositor_tests.exe` under
`foreground_lock.py` (the input desktop, since it reads the composed
screen). New in C8: `dwm_backdrop_type`, `DwmGetWindowAttribute(
DWMWA_SYSTEMBACKDROP_TYPE)` as read back (2 = DWMSBT_MAINWINDOW for the
material, 0 = DWMSBT_AUTO for transparent and opaque), and
`extended_frame` (DWMWA_EXTENDED_FRAME_BOUNDS). The composed clear pixel
was Mica (240, 244, 249), the green window behind (0, 160, 0), then
black (alpha ignored again) for opaque.

## Every state, headless (the Skia raster)

**MEASURED:** `headless-states.jsonl`, from
`gui_forms_compositor_lab_tests.exe --print`: the lab under each state a
host may report (every backdrop; transparent only, materials `os_version`;
no compositor; transparency effects off; reduced transparency; high
contrast) and each of the four requests, with the grant, the reason and the
frame's alpha. Every state renders; refused materials fall back to
`transparent` where it is listed and to `opaque` otherwise; reduced
transparency and high contrast give `opaque` and drop the reflections. The
test also runs a compositor stopping and starting under a standing request.

## Not recorded yet

- Windows on CI (Server 2022, build 20348), the fallback state: the same
  tool on the CI runner, or the C2 test's artifact line (now with
  `dwm_backdrop_type` -1, the attribute missing).
- Shadow Windows with transparency effects off, then high contrast on: user
  settings the owner changes; the same two commands, `--state
  transparency-off` and `--state high-contrast`.
- macOS (`macos-15` or the bench): `tools/t5_gate_record.py --lab
  "GUI.Forms Compositor Lab.app/Contents/MacOS/GUI.Forms Compositor Lab"
  --host-test gui_forms_macos_compositor_tests`, in Light, Dark and Reduce
  transparency; for the next Mac handoff.
- X11 under Xvfb without a compositor, with `xcompmgr`, and across it
  stopping and starting: `--lab gui-forms-compositor-lab --host-test
  gui_forms_linux_compositor_tests`; and XWayland, per desktop measured.
