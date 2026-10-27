# Budgets

> **Note from the Stage 4 session, 2026-10-03.** The floor table below is
> Stage 2's and is not edited. With the proving slice in the window (a
> composed spread on screen; the slice's six faces and six fixtures,
> 2,742,826 bytes, compiled into the executable; HarfBuzz, FreeType, SheenBidi
> and libunibreak linked), the same procedure on CI run 37182648303 gives,
> Stage 2 → Stage 4:
>
> | | Windows x64 | macOS arm64 | Linux x64 | Linux arm64 |
> | --- | --- | --- | --- | --- |
> | Executable, bytes | 82,074 → 5,358,807 | 98,912 → 3,068,336 | 71,984 → 3,115,312 | 67,664 → 3,082,320 |
> | Installed, bytes | 12,608,779 → 17,885,512 | 12,951,580 → 15,921,004 | 15,696,566 → 18,739,894 | 14,525,310 → 17,539,966 |
> | First frame, ms (median of 7) | 104 → 30 | 227 → 172 | 43 → 45 | 56 → 58 |
> | Private at rest, MiB | 5.0 → 11.1 | 26.3 → 31.8 | 14.6 → 23.6 | 14.0 → 22.9 |
> | Resident at rest, MiB | 16.9 → 24.5 | 77.3 → 92.5 | 21.4 → 30.3 | 20.2 → 29.2 |
> | Paints at rest, 10 s and 30 s | 0, 0 | 0, 0 | 1, 1 (M6) | 1, 2 (M6) |
>
> On Shadow at scale 2: first frame 211 → 65 ms; private 13.8 → 36.5 MiB;
> resident 25.5 → 48.8 MiB, of which the two page buffers are 22 MiB. A page
> on the route now used holds twice its pixels (two buffers), as a registered
> image did. The receipts are `experiments/003-proving-slice/results/`; the
> reasons are in `planning/reports/S4-2026-10-03.md` and the proposed
> decision record beside it. Where the floor "already presses" (section 3):
> launch on Windows no longer does; macOS memory presses harder.

Written in Stage 2 (2026-10-03) from measured floors. A floor is what the
empty book window costs before the product has put anything in it: one
window, one view, one 64 × 64 application-rendered BGRA image stretched over
the client area, no text, no container, no timers. Every later stage is held
against these numbers: it reports what it added to each, and why.

The ceilings are the owner's (INVARIANTS I29–I31). They are ceilings, not
targets, and the aim is to be far under them (J008 A).

| Quantity | Ceiling | Source |
| --- | --- | --- |
| Installed size | under 100 MB | I29, K022 |
| Resident memory with a spread open | under 150 MiB | I30, I005 C |
| Launch to a readable page | under 400 ms | I31, I006 B |
| Idle with the book open and untouched | zero timers, zero paints, 0% CPU | I18, I004 A |

## 1. How the floors are measured

Procedure: `scripts/measure_floor.py` against a packaged executable with an
isolated state directory; see `docs/BUILDING.md`. One receipt per platform is
stored with the stage report's evidence. What each figure is:

- **Executable, toolkit library, installed size, package size.** Bytes on
  disk, after the packaging script has stripped the binaries. Installed size
  is every file in the installed directory.
- **Launch.** Seven runs of `--profile-launch`. `first paint` is the time from
  the first statement of `main` to the return of the first `on_paint`.
  `first frame` is the time from the same origin to the first 1 ms poll that
  saw the host's `frames_presented` reach 1; it is late by up to one poll
  interval plus scheduling delay (`docs/TOOLKIT_REQUESTS.md` M4).
  `spawn to report` is wall time in the measuring script from starting the
  process to reading its report line, so it adds process creation, dynamic
  loading and pipe delivery. The table gives the median and the range.
- **Idle.** Two runs of `--profile-idle`, each five seconds of warm-up and
  then a sample: one of 10 s and one of 30 s. Processor time is the process's
  own user plus kernel time over the sample. The toolkit's activity counters
  are reset at the start of the sample and read at its end. Two lengths are
  used because work that recurs while idle grows with the sample and a cost
  paid once does not.
- **Memory.** Read at the end of the idle sample by
  `bible::core::read_process_counters`. `private` is each platform's own
  notion of memory charged to this process alone: committed private bytes on
  Windows, `Private_Clean + Private_Dirty` on Linux, `phys_footprint` on
  macOS. They are not the same quantity; compare within a platform only.
  `resident` is the working set or resident set, shared pages included.

What the procedure does not establish:

- Nothing about text, the container, or a real page. The floor is the cost of
  the toolkit and the process, not of the product.
- The time from the host's present call to light on the display.
- Launch on a cold disk cache: runs are back to back.
- Behaviour on hardware. Every figure below is from a virtual machine; the CI
  runners are shared virtual machines with no physical display (Linux runs
  under Xvfb). Treat launch times as indicative and sizes as exact.

## 2. Floors

All MEASURED on 2026-10-04 at commit `487b58b`, toolkit pin `7b260cf`. The
receipts are `planning/reports/S2-2026-10-03-receipts.md`.

Environments:

| Label | Machine | System | Window (logical, scale) |
| --- | --- | --- | --- |
| Windows, Shadow | QEMU guest, AMD EPYC 9354, 8 logical processors, 16 GiB, in interactive use | Windows 11 22621 | 1060 × 680, scale 2 |
| Windows, CI | GitHub `windows-2022`, 2 processors | Windows Server 2022 20348 | 1028 × 680, scale 1 |
| macOS, CI | GitHub `macos-15`, arm64, 3 processors | macOS 15.7.9 | 1024 × 656, scale 1 |
| Linux x64, CI | GitHub `ubuntu-24.04`, 2 processors, Xvfb | Linux 6.17, glibc 2.39 | 1060 × 680, scale 1 |
| Linux arm64, CI | GitHub `ubuntu-24.04-arm`, 2 processors, Xvfb | Linux 6.17, glibc 2.39 | 1060 × 680, scale 1 |

The window asks for 1060 × 680; the Windows and macOS runners gave less.

### 2.1 Size on disk (bytes; exact)

| | Windows x64 | macOS arm64 | Linux x64 | Linux arm64 |
| --- | --- | --- | --- | --- |
| Executable | 82,074 | 98,912 | 71,984 | 67,664 |
| GUI.Forms application library | 6,192,514 | 7,958,848 | 12,192,760 | 11,025,824 |
| Installed, all files | 12,608,779 | 12,951,580 | 15,696,566 | 14,525,310 |
| Share of the 100 MB ceiling | 12.6% | 13.0% | 15.7% | 14.5% |
| Portable archive | `.zip` 4,811,198 | `.zip` 5,326,005 | `.tar.gz` 6,222,000 | `.tar.gz` 5,828,354 |
| Installer | `-setup.exe` 5,012,472 | `.pkg` 5,318,842 | `.deb` 4,967,224 | `.deb` 4,659,588 |

In every installed tree 3,376,728 bytes are the seven faces the toolkit's
hosts require (`docs/TOOLKIT_REQUESTS.md` M2). On Windows 2,874,913 bytes are
the three compiler runtime libraries. The application's own code is the
executable: under 100 kB on every platform.

### 2.2 Launch (milliseconds; median of seven, range in brackets)

| | First paint | First frame presented | Spawn to report | First frame, share of 400 ms |
| --- | --- | --- | --- | --- |
| Windows, Shadow (2120 × 1360 device pixels) | 39 (35–45) | 211 (205–218) | 228 (221–259) | 53% |
| Windows, CI | 28 (26–40) | 104 (83–116) | 117 (93–135) | 26% |
| macOS, CI | 213 (167–249) | 227 (191–289) | 249 (210–305) | 57% |
| Linux x64, CI | 17 (11–18) | 43 (36–43) | 47 (40–47) | 11% |
| Linux arm64, CI | 24 (15–25) | 56 (47–56) | 60 (51–60) | 14% |

### 2.3 Idle (`--profile-idle`, a 10 s sample and a 30 s sample)

| | Processor time, 10 s | Processor time, 30 s | `paint_passes` | Frames presented | Timer ticks |
| --- | --- | --- | --- | --- | --- |
| Windows, CI | 0 ns | 0 ns | 0, 0 | 0, 0 | 0, 0 |
| macOS, CI | 22.3 ms | 8.9 ms | 0, 0 | 0, 0 | 0, 0 |
| Linux x64, CI | 24.4 ms | 24.4 ms | 1, 1 | 0, 0 | 0, 0 |
| Linux arm64, CI | 30.1 ms | 61.3 ms | 1, 2 | 0, 0 | 0, 0 |
| Windows, Shadow | 0 ns in three samples; later 46.9–187.5 ms | 62.5–578.1 ms | 0 in five samples; 1 and 13 in two | 0 in five; 1 and 2 in two | 0 in all |

Reading it:

- “Timer ticks” is `frame_deadlines_fired`, and `scheduler_wakes` and
  `active_surface_ticks` are zero in every sample on every platform. The
  application arms no timer when it is not profiling. **The application adds
  nothing to idle on any platform.** What follows is the toolkit and the
  system.
- **Windows:** zero paints, zero frames, and no processor time charged on the
  quiet CI runner in both samples. Windows charges process time in 15.625 ms
  ticks, so 0 ns means no tick was charged in 30 s, not that no instruction
  ran. On Shadow, three samples taken while the desktop was quiet were also
  zero; four taken while it was in use were not, and two of those saw the
  window repainted. Those are listed in the receipts with what is and is not
  known about them.
- **macOS:** zero paints, zero frames, zero ticks, and 8.9 ms of processor
  time in 30 s (0.03% of one core); 22.3 ms in the earlier 10 s sample. The
  longer sample costing less than the shorter one says it is not a steady
  drain, but its cause is not established.
- **Linux: not zero.** Each sample contains one full-window repaint, caused by
  the timer tick that opens the sample: after an expose the toolkit's X11 host
  repaints the whole window on every pass of its event loop
  (`docs/TOOLKIT_REQUESTS.md` M6). On x64 the cost is the same 24.4 ms in 10 s
  and in 30 s, so nothing recurs between passes. On arm64 the 30 s sample has
  a second repaint: something woke the loop once more. What woke it is not
  established. The application cannot prevent either from its side of the
  toolkit boundary.

### 2.4 Memory at the end of the 10 s idle sample

| | Private | Resident | Resident, share of 150 MiB |
| --- | --- | --- | --- |
| Windows, Shadow (scale 2) | 14,467,072–14,520,320 (13.8 MiB) | 26.8 MB (25.5 MiB) | 17% |
| Windows, CI (scale 1) | 5,275,648 (5.0 MiB) | 17,723,392 (16.9 MiB) | 11% |
| macOS, CI | 27,593,088 (26.3 MiB) footprint | 81,084,416 (77.3 MiB); 83,738,624 after 30 s | 52–53% |
| Linux x64, CI | 15,343,616 (14.6 MiB) | 22,487,040 (21.4 MiB) | 14% |
| Linux arm64, CI | 14,667,776 (14.0 MiB) | 21,209,088 (20.2 MiB) | 13% |

The two Windows rows differ by the window's backing store: 2120 × 1360 device
pixels at scale 2 is 11.5 MB, 1028 × 680 at scale 1 is 2.8 MB.

## 3. Where the floor already presses on a ceiling

Three places. Each is measured on the empty window, before any text exists.

1. **Launch on Windows.** On Shadow the first frame arrives at 211 ms, 53% of
   the 400 ms ceiling, and 165 ms of that is one call: drawing the page image
   (`docs/TOOLKIT_REQUESTS.md` M1; with a plain fill instead of an image the
   first frame arrived at about 40 ms). The same call is paid on every page
   turn and resize. Opening the container, composing a page and shaping its
   text must fit in what remains unless the toolkit's image path is fixed or
   the page reaches the window another way.
2. **Launch on macOS.** The first paint comes 213 ms after `main` begins on the
   CI runner, 57% of the ceiling by first frame, before this project's code has
   done anything but ask for a window. Where that time goes inside the toolkit
   and AppKit is not established.
3. **Resident memory on macOS.** The empty window is 77–80 MiB resident, a
   little over half of the 150 MiB ceiling, while its footprint, the memory
   charged to the process alone, is 26 MiB. The difference is system framework
   pages shared with other processes. Which of the two the ceiling means is the
   owner's to say (Stage 2 report, open questions). Whatever the answer, each
   page image held costs twice its pixels on Windows (M5): one page at
   2120 × 1360 is 11.5 MB of pixels and 23 MB of memory.

And one place where a rule, not a ceiling, is not met: **idle on Linux is not
zero paints** (§2.3), for a reason inside the pinned toolkit (M6).

## 4. Rules for later stages

- A stage that changes a floor quantity re-runs `scripts/measure_floor.py` on
  every platform it touched and records the new figure beside the old one in
  its report. The floor table above is not edited; additions are a new dated
  table below it.
- A number is never called small, fast or light. It is stated with its
  ceiling and its floor.
- If a ceiling cannot be met, the stage stops and says so. The ceiling is not
  moved by the stage.
- Idle is not a budget to spend. Any non-zero idle counter is a defect.
