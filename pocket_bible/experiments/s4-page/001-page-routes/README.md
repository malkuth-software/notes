# Experiment 001 — the page's route to the window

Stage 4, part one. Status: **MEASURED** on the owner's machine and on the four
CI runners. Every figure names its machine and scale; every machine is virtual.

**Question.** Which route of the pinned toolkit puts an application-composed
page on the window within the owner's bar — a page turn within 50 ms from
request to presented (round 003 W303 C), at device resolution (W304 A), with
zero work at rest (INVARIANTS I18)?

**Hypothesis, and what would falsify it.** OBSERVED before measuring: the
Win32 host has one row-copy path, in `draw_live_surface`, taken for a
same-size frame at full opacity (`windows_host.cpp:1404-1444`); a `LiveSurface`
drawn through the painter in `on_paint` is never registered with the window,
so the 16 ms live clock (`:2919-2927`) has no reason to arm. HYPOTHESIS: that
route turns a 2120 × 1360 page in under 50 ms on Shadow and idles at zero.
Falsified if the record-and-replay display list defeats the copy path, if the
clock arms, or if the time is over the bar. **It was not falsified.**

**Control.** `fill`: the same turn with `fill_rect` in two colours and no
image. Its time is what a turn costs before any page pixels move; its memory
is the baseline the other routes' "held" figure is taken from.

**Data.** Two synthetic opaque pages at device resolution, made by the probe:
rows of ink boxes on the paper colour, widths from a linear congruential
sequence. No text of any kind. The `book-*` routes use the proving slice's own
pages instead (see experiment 003).

**Environment.**

| Label | Machine | System, compiler | Window (logical, scale) |
| --- | --- | --- | --- |
| Shadow | QEMU guest, AMD EPYC 9354, 8 logical processors, 16 GiB, in interactive use | Windows 11 22621, MinGW GCC 16.2.0, Release | 1060 × 680, 600 × 420 and the largest the display allows, all at scale 2 |
| Windows CI | GitHub `windows-2022` | MSYS2 MinGW GCC 16.2.0 | 1028 × 680, scale 1 |
| macOS CI | GitHub `macos-15`, arm64 | Homebrew LLVM 20 | 1024 × 656, scale 1 (see note) |
| Linux x64 CI | GitHub `ubuntu-24.04`, Xvfb | GCC 13.3.0 | 1060 × 680, scale 1 |
| Linux arm64 CI | GitHub `ubuntu-24.04-arm`, Xvfb | GCC 13.3.0 | 1060 × 680, scale 1 |

Toolkit pin `7b260cf`. CI run 37171572751 at commit `38a469c`.

**Procedure.**

```
cmake -S . -B .build/app -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .build/app --parallel 2 --target page_route_probe
python3 experiments/001-page-routes/run_routes.py --probe .build/app/page_route_probe.exe \
    --output results.jsonl --turns 30 --idle 10 [--width 600 --height 420]
python3 experiments/001-page-routes/summarize_routes.py results.jsonl
```

One run is one route. The probe opens the window, waits a second, then turns
the page thirty times, 250 ms apart, each from rest. A turn begins at the
first statement of a UI-timer callback that stands in for the key handler
(the *request*) and ends at the first dispatcher callback, posted from
`on_paint`, that sees `frames_presented` advanced (*observed*): a posted
callback runs after the host has finished the paint that called `on_paint`.
For `surface-held`, which presents without a paint, the observer re-posts
itself until the surface reports the published generation sampled. After the
turns: two seconds of warm-up, then an idle sample (ten seconds on Shadow,
five in CI) over which the toolkit's activity counters, the process's
processor time and its cycle count (Windows) or context switches (POSIX) are
read. On CI the same runs are registered as tests (`forms_page_route_*`); the
result lines are read from `Testing/Temporary/LastTest.log`, which the
workflow keeps.

The routes:

| Route | What a turn does | What `on_paint` does |
| --- | --- | --- |
| `fill` | nothing | `fill_rect` |
| `image-swap` | chooses the other of two registered images | `draw_image` (linear sampling) |
| `image-swap-nearest` | the same | `draw_image_region_sampled(nearest)` |
| `image-update` | `update_bgra32_premultiplied` of one image | `draw_image` |
| `image-patch` | `patch_bgra32_premultiplied` over the whole image | `draw_image` |
| `image-strips` | updates eight strip images | eight `draw_image` calls |
| `image-pattern` | chooses the other of two images | `fill_image_pattern`, one tile the size of the page |
| `surface-painter` | copies the page into an **unregistered** `LiveSurface` and publishes | `draw_live_surface` |
| `surface-held` | publishes into a surface registered once with `queue_live_surface_presentation` | `draw_live_surface` (for exposes) |
| `surface-turn` | publishes, and registers the surface through a child control added for the turn and removed once presented | `draw_live_surface` |
| `raster-canvas` | rewrites a GUI.Drawing bitmap shown by a `RasterCanvas` at one bitmap pixel per device pixel | the canvas's own tiles |
| `book-reading`, `book-study` | the real `BookView`: composes the next spread into the surface and publishes | `draw_live_surface` |

## Results

### Shadow, 1060 × 680 at scale 2 (2120 × 1360 device pixels, 11.5 MB a page)

Second of two runs; the first agreed within a few milliseconds at p50
(`results/shadow-1060x680-run1.jsonl`).

| Route | p50 ms | p95 ms | max ms | Held beyond `fill`, MB | At rest (10 s): paints, frames, megacycles |
| --- | --- | --- | --- | --- | --- |
| `fill` | 1.6 | 1.9 | 1.9 | — | 0, 0, 0.4 |
| **`surface-painter`** | **2.4** | **2.6** | **2.7** | 23.1 | 0, 0, 0.2 |
| `surface-turn` | 2.7 | 3.2 | 3.3 | 23.2 | 0, 0, 0.6 (two paints per turn) |
| `surface-held` | 2.2 | 16.7 | 17.1 | 23.1 | 0, 0, **42.6**; registration held |
| `image-pattern` | 58.3 | 66.8 | 85.2 | 46.2 | 0, 0, 0.3 |
| `image-swap-nearest` | 82.4 | 85.3 | 85.4 | 46.2 | 0, 0, 0.3 |
| `raster-canvas` | 89.5 | 97.0 | 125.0 | 35.7 | 0, 0, 1.0 |
| `image-swap` | 137.4 | 181.9 | 182.2 | 46.2 | 0, 0, 0.3 |
| `image-update` | 139.0 | 148.3 | 184.4 | 23.1 | 0, 0, 0.3 |
| `image-strips` | 139.6 | 143.5 | 148.9 | 23.1 | 0, 0, 0.3 |
| `image-patch` | 142.6 | 150.3 | 152.1 | 23.1 | 0, 0, 0.2 |

The book itself on the chosen route, composing each spread (commit `c6bee30`;
see experiment 003 for the composition figures alone):

| Route | p50 ms | p95 ms | max ms | Of which composing, p50 ms |
| --- | --- | --- | --- | --- |
| `book-reading` | 4.9 | 6.2 | 6.6 | 2.4 |
| `book-study` | 5.2 | 8.2 | 9.3 | 2.8 |

### Shadow, other sizes (scale 2)

| Route | 600 × 420 (1200 × 840): p50, p95, max ms | Largest window, 1410 × 860.5 (2820 × 1721): p50, p95, max ms |
| --- | --- | --- |
| `fill` | 0.7, 0.9, 0.9 | 2.6, 2.7, 2.8 |
| **`surface-painter`** | **1.0, 1.1, 1.2** | **3.8, 4.2, 4.2** |
| `surface-turn` | 1.3, 1.6, 1.7 | 4.1, 4.4, 4.5 |
| `surface-held` | 15.9, 16.3, 31.7 | 3.8, 18.0, 18.1 |
| `image-pattern` | 20.6, 24.8, 36.9 | 98.6, 158.7, 177.1 |
| `image-swap-nearest` | 29.1, 37.9, 46.7 | 139.3, 172.5, 175.6 |
| `raster-canvas` | 31.5, 34.9, 36.3 | 149.9, 157.7, 159.2 |
| `image-swap` | 48.2, 50.2, 52.3 | 232.9, 254.0, 255.5 |
| `image-update` | 49.0, 51.0, 73.3 | 233.8, 238.5, 239.2 |
| `image-strips` | 49.0, 50.5, 50.6 | 232.2, 238.5, 238.8 |
| `image-patch` | 50.3, 63.5, 77.7 | 236.0, 253.0, 257.1 |

The window asked for 1800 × 1000 and the display gave 1410 × 860.5.

### CI runners (scale 1; p50, p95, max ms over thirty turns)

| Route | Windows, 1028 × 680 | macOS, see note | Linux x64, 1060 × 680 | Linux arm64, 1060 × 680 |
| --- | --- | --- | --- | --- |
| `fill` | 0.8, 0.9, 0.9 | 3.1, 10.2, 19.3 | 17.4, 17.6, 18.0 | 25.1, 25.4, 25.5 |
| **`surface-painter`** | **1.0, 1.1, 1.1** | **7.2, 14.3, 15.7** | **18.8, 18.9, 18.9** | **25.4, 25.9, 25.9** |
| `surface-turn` | 1.1, 1.1, 1.1 | 4.9, 23.2, 27.3 | 18.8, 19.0, 21.4 | 26.9, 27.2, 27.2 |
| `surface-held` | 1.1, 16.2, 16.3 | 18.6, 31.0, 73.3 | 18.4, 19.0, 27.8 | 26.5, 27.1, 27.4 |
| `image-pattern` | 15.5, 15.9, 30.4 | 11.0, 20.2, 24.6 | 22.9, 23.0, 23.3 | 29.5, 29.7, 29.8 |
| `image-swap-nearest` | 24.4, 24.9, 25.2 | 4.2, 7.6, 11.9 | 18.3, 18.7, 19.1 | 25.2, 25.5, 25.5 |
| `raster-canvas` | 25.8, 26.1, 26.2 | 2.8, 4.7, 4.7 | 19.6, 20.6, 21.5 | 27.3, 27.7, 27.7 |
| `image-swap` | 40.9, 41.9, 43.0 | 6.6, 13.8, 17.6 | 18.6, 18.7, 19.0 | 26.1, 26.6, 26.8 |
| `image-update` | 41.2, 41.6, 41.9 | 6.8, 12.4, 15.1 | 19.0, 20.4, 20.4 | 26.7, 27.5, 30.9 |
| `image-strips` | 41.5, 42.7, 43.2 | 8.2, 15.0, 16.0 | 19.1, 19.3, 19.3 | 26.6, 26.9, 26.9 |
| `image-patch` | 41.8, 42.3, 42.4 | 8.0, 25.9, 40.0 | 18.9, 19.6, 19.8 | 25.6, 26.3, 26.8 |

The book itself on the surface route, composing each spread (CI run
37182648303, commit `cb914e6`; p50, p95, max ms):

| Route | Windows, 1028 × 680 | macOS, 1024 × 656 | Linux x64, 1060 × 680 | Linux arm64, 1060 × 680 |
| --- | --- | --- | --- | --- |
| `book-reading` | 1.1, 1.8, 2.3 | 2.0, 2.8, 5.3 | 15.2, 15.8, 16.1 | 25.9, 26.4, 26.7 |
| `book-study` | 1.3, 2.5, 4.0 | 2.1, 3.5, 4.9 | 15.4, 16.4, 17.6 | 27.0, 28.2, 28.9 |

The book composes for the size it finds at paint, so on macOS it takes the
same-size path the probe's synthetic pages missed (note below).

At rest in the five-second CI samples: zero paints and zero frames for every
route on Windows and macOS; on Linux `surface-held` shows one paint. The held
registration costs 15.7 megacycles in five seconds on Windows CI against 0.3
for the others, and 627 context switches on macOS against 5 to 10.

Note on macOS: the probe sized its pages from the client size it was given at
`ready` (1060 × 680); the window then settled at 1024 × 656. The pages were
therefore drawn scaled, not one to one, and the surface route did not take a
same-size path. The macOS column is the cost of that, not of the route as the
book uses it: the book composes for the size it finds at paint.

## What this establishes

- MEASURED: on Windows — the platform Stage 2 found over the bar — the
  unregistered surface drawn through the painter turns a full-window page in
  2.4 ms (p50) and 2.7 ms (max) at 2120 × 1360 on Shadow, 4.2 ms (max) at
  2820 × 1721, and idles at zero paints, frames, timers and cycles. It is the
  only route under 50 ms at the default window that is also silent at rest.
- MEASURED: with the real compositor in the turn, Shadow presents a new spread
  in 4.9 ms (p50) and 6.6 ms (max) in reading form, 5.2 and 9.3 ms in study
  form: about a tenth of the bar.
- MEASURED: every registered-image route on Windows misses the bar at the
  default window on Shadow (58 to 143 ms at p50) and at the larger one. At
  600 × 420 the four `draw_image` routes sit at the bar (48 to 50 ms at p50)
  and the other three under it (21 to 32 ms).
- MEASURED: holding a surface registered keeps work running at rest: 42.6
  megacycles in ten seconds on Shadow against 0.2 to 1.0 for every other
  route. OBSERVED cause: the 16 ms waitable timer stays armed while a
  registration exists (`windows_host.cpp:2919-2927`, cancelled only when
  none remains, `:2957`).
- MEASURED: registering only for the turn is no faster than not registering
  (2.7 against 2.4 ms) and costs a second paint to remove the registrant.
- MEASURED: on Linux every route costs what an empty paint costs there (17 ms
  on x64, 25 ms on arm64 for `fill_rect` alone): the X11 host's own
  paint-and-send, not the route, sets the time. All routes are under 50 ms.
- MEASURED: a page on the surface route holds twice its pixels (23.1 MB for an
  11.5 MB page): the smallest pool the toolkit allows is two buffers.

## What this does not establish

- Anything on hardware. Shadow is a virtual machine in interactive use; the
  CI runners are shared virtual machines without a physical display.
- The time from the operating system's key event to the request: a UI-timer
  callback stands in for the key handler. Nor the time from the host's
  present call to light on a display.
- macOS with pages at the window's settled size (see the note); macOS and
  Linux at display scale 2.
- Why `fill_image_pattern` is twice as fast as `draw_image` over the same
  pixels on Windows; the loops were read (`windows_host.cpp:1499-1515`,
  `:1543-1565`), not profiled.
- That the unregistered-surface path is a contract. It is how the pin's code
  behaves; nothing in the toolkit's documents promises it (filed as a
  request).

## Next smallest falsifiable step

Taken in experiment 003: the proving slice composed into the surface, with the
whole turn — compose, publish, paint, present — under the bar on Shadow.
On a second machine with a physical display: thirty turns of `book-reading`
filmed against a timer, to bound what the probe cannot see.
