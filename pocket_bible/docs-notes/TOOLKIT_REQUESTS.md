# Toolkit requests

Gaps in GUI.Forms met by this project, recorded for the GUI.Forms owner in the
manner of `games/docs/TOOLKIT_REQUESTS.md`. This project never modifies the
toolkit checkout. Locators are in the pinned toolkit,
`falseywinchnet/file_manager` at `7b260cfb9f3267392e1470b0fcf4cd2497437819`,
under `gui_forms/`.

Status on 2026-10-03: none filed with the owner. Section 1 holds gaps actually
met in Stage 2, each with the file and line that shows it and the measurement
that makes it matter. Section 2 holds needs still only anticipated from the
digest; they must be re-verified at the pin by the stage that meets them.
Section 3 records anticipated items removed and why.

Status after Stage 4 (2026-10-03): M1, M2, M3, M5 and M6 went to the
GUI.Forms threads by the owner's word (round 003 W302). Section 1b holds what
Stage 4 met while measuring every route to the window and building the page
on the best one: M7 to M13. M7 is the one the product now stands on.

## 1. Met in Stage 2

### M1. Drawing an image on Windows costs 86–166 ms per full-window paint

- **OBSERVED.** `Painter::draw_image` on the Win32 host forwards to
  `draw_image_region_sampled` with `ImageSampling::linear`
  (`src/host/windows/application/windows_host.cpp:1371-1379`, `:1471-1475`).
  That routine visits every destination pixel in a double loop and, per pixel,
  builds a `Point`, tests the clip and the destination rectangle, calls
  `pixel_allowed`, computes the source position in `double`, samples and blends
  (`:1499-1515`). There is no path for an opaque image drawn at its own size.
  `draw_live_surface` in the same file does have one: a same-size, full-opacity
  frame is copied row by row with `memcpy` (`:1404-1444`).
- **MEASURED** (Shadow: a QEMU guest on an AMD EPYC 9354, 8 logical
  processors, Windows 11 22621, MinGW GCC 16.2.0, Release; window 1060 × 680
  logical at `Window::scale()` 2, so 2120 × 1360 = 2,883,200 device pixels;
  the figure is `present_duration_nanoseconds` of the first frame in the
  close-time receipt of `pocket-bible --smoke-test`, whose timed region is
  model paint plus commit, `windows_host.cpp:3000-3028`; each value is one
  run of a temporary edit of `BookView`, not kept in the tree):

  | What `BookView::on_paint` draws over the client area | Paint + present |
  | --- | --- |
  | `fill_rect` | 1.2 ms |
  | `draw_image`, 64 × 64 image, stretched | 166.0, 165.5 ms |
  | `draw_image`, 1060 × 680 image, stretched 2× | 164.5 ms |
  | `draw_image`, 2120 × 1360 image, one image pixel per device pixel | 146.5, 145.3, 164.3 ms |
  | `draw_image_region_sampled(..., nearest)`, 1060 × 680 image | 85.5, 114.0, 128.4 ms |

  2,883,200 pixels in 145–166 ms is 50–58 ns per pixel, the same whether the
  image is stretched or not. Launch-to-first-frame moved with it: about 40 ms
  with `fill_rect`, 200–270 ms with `draw_image`.
- **Why it matters here.** The application composes whole pages to pixels and
  hands them to the toolkit as images (INVARIANTS I16). Every page turn, resize
  and expose repaints a page image. At this cost a page turn on Windows cannot
  be instant (I18), and the empty window already spends about half of the
  400 ms launch ceiling (I31) inside this one call.
- **Request.** In `draw_image_region_sampled`: when opacity is 1, the source
  rectangle is the destination size in device pixels and the image is opaque
  or the destination may be replaced, copy rows as `draw_live_surface` does;
  otherwise hoist the clip and rectangle tests out of the pixel loop and step
  the source coordinate incrementally.
- **For comparison, MEASURED on the CI runners** (run 37164267391, scale 1,
  the same first-frame figure from the packaged application's smoke test):
  Windows 77.8 and 78.6 ms for 1028 × 680; Linux x64 24.4 and 24.9 ms and
  Linux arm64 30.1 and 31.3 ms for 1060 × 680, which includes sending the
  frame to Xvfb; macOS 3.8 and 4.3 ms for 1024 × 656. Skia draws the image on
  the last three.
- **Does not establish:** any of this on hardware; every machine above is
  virtual. It also
  does not establish the cost through `LiveSurface`, which has the fast copy
  but, per `games/docs/TOOLKIT_REQUESTS.md`, keeps the Windows presentation
  clock armed while registered and would break the zero-idle rule.
- **Meanwhile:** Stage 4 measures the page path it chooses on Windows first.

### M2. Every host requires seven bundled faces, whatever the application draws

- **OBSERVED.** The Windows host loads a fixed list of file names from
  `fonts\` beside the executable and reports an incomplete renderer unless the
  first seven load (`windows_host.cpp:3479-3497`, `:2415-2427`). The Linux host
  throws if any of the first seven is missing (`linux_host.cpp:123-160`). The
  macOS host requires the same seven from the bundle (`macos_host.mm:992-1005`,
  renderer name at `:1028-1032`). The seven are Portsmouth Rapids regular and
  bold, Carlito in four styles and Cousine regular.
- **MEASURED.** The seven faces are 3,376,728 bytes of every package. This
  product draws no toolkit text in Stage 2 and will set its pages in its own
  faces (I45).
- **Request.** Let an application declare the font pack its window needs, down
  to none for a window that only presents images, or to its own faces for the
  three roles.
- **Meanwhile:** the packages carry exactly those seven and omit the ten
  optional faces (18,539,376 bytes, of which Noto Sans CJK JP is 16,467,736).

### M3. A required face is an evaluation build whose redistribution is unsettled

- **OBSERVED.** `assets/fonts/ATTRIBUTION.md` describes Portsmouth Rapids as an
  “experimental personal and artistic evaluation build” and labels it
  “CANDIDATE … subject to the production redistribution-rights gate”. No
  licence file for it is in `assets/fonts/`. M2 makes it impossible to ship a
  working window without it.
- **GIVEN.** Portsmouth is the owner's to license and ships under the OFL with
  the source-font notices Plan Paint carries (J006 A). Those notices are
  `plan-paint/packaging/licenses/Portsmouth-source-notices.txt`; they are not in
  the toolkit and not in this repository.
- **Request.** The toolkit's font directory should carry the licence and
  source-font notices of every face a host requires.
- **Meanwhile:** packages ship the toolkit's `ATTRIBUTION.md` beside the fonts,
  as PlaySuite does. The Stage 1 session owns `SOURCE_REGISTER.md` and is asked
  to enter the toolkit faces; see the Stage 2 report, open questions.

### M4. No notification that a frame has been presented

- **OBSERVED.** `Metrics::record_present` counts presented frames
  (`src/core/metrics/metrics/metrics.cpp:183`), but the public window interface
  has no callback or timestamp for it; a control learns only that `on_paint`
  ran.
- **Consequence.** `--profile-launch` polls `frames_presented` from a 1 ms UI
  timer, so its figure is late by up to one poll interval plus scheduling
  delay, and the smoke test learns of presentation only at its scheduled close.
- **Request.** A one-shot “first frame presented” notification, or the time of
  the last present in `MetricsSnapshot`.

### M5. A registered image is held twice on Windows

- **OBSERVED.** The window's `ImageRegistry` keeps the bytes it was given, and
  the Win32 raster copies them again into its own `images_` map when it
  synchronizes (`windows_host.cpp:741-807`, the copy at `:779-781`).
- **MEASURED** (same machine and build as M1; `private_bytes` at the end of
  `--profile-idle`): 14.5 MB with the 64 × 64 page image; 37.6 MB with one
  2120 × 1360 image, which is 11.5 MB of pixels. The growth, 23.1 MB, is twice
  the image.
- **Why it matters here.** A spread at device resolution is two such pages;
  cached neighbouring pages multiply it. Against the 150 MiB ceiling (I30) the
  page cache must be budgeted at twice its pixel bytes on Windows.
- **Request.** Let the host draw from the registry's storage, or let the
  registry release its copy once the host holds one.
- **Does not establish:** the factor on macOS and Linux.

### M6. After an expose, the X11 host repaints the whole window on every wake

- **OBSERVED.** `NativeWindow::update` paints whenever the host's own
  `damage` region is not empty, and clears that region only when
  `Window::notify_presented` accepts the receipt
  (`src/host/linux/application/linux_host.cpp:670-674`, `:749-750`). An
  `Expose` event adds to the region without changing the model
  (`:509-515`). The repaint that follows renders the same revision as the
  frame already presented, so `notify_presented` rejects it as not monotonic
  (`src/core/window/presentation/window_presentation.cpp:254-262`) and the
  region stays. From then on every pass of the event loop, whatever woke it,
  repaints and re-sends the full window.
- **MEASURED** (CI run 37120901173, `ubuntu-24.04` under Xvfb, window
  1060 × 680 at scale 1). The close-time receipt of a two-second
  `--smoke-test` shows `paint_passes` 4, `full_window_paints` 4,
  `display_chunks_reused` 3 and `frames_presented` 1: four paints, one
  accepted. In `--profile-idle 10` the sample holds `paint_passes` 1,
  `frames_presented` 0 and 23.05 ms of processor time; one paint-and-send on
  that runner is 23.1–23.7 ms (`present_duration_nanoseconds`). The one paint
  is the pass of the loop that delivered the tick opening the sample.
  Run 37164267391 repeated it with a 10 s and a 30 s sample: on x64 one
  paint and 24.4 ms in each, so nothing recurs while the loop is blocked; on
  arm64 one paint and 30.1 ms in 10 s, two paints and 61.3 ms in 30 s, so the
  loop was woken once more during the longer sample and repainted. What woke
  it is not established.
- **Why it matters here.** With nothing waking the loop it blocks in `poll`
  and the book idles; but every key, pointer move and timer tick costs a
  full-window repaint whether or not anything changed. Zero paints at rest
  (I18) holds only while the window receives nothing at all.
- **Request.** Clear the host's region after an expose-only repaint has been
  sent: an expose needs the pixels re-sent, not the model's revision advanced.
- **Does not establish:** the behaviour under a compositing window manager on
  hardware; CI has only Xvfb.

## 1b. Met in Stage 4

Measurements are from `experiments/001-page-routes/` unless another is named;
its README states the procedure, the machines and what the figures do not
establish. "Shadow" is the owner's machine: a QEMU guest, Windows 11, display
scale 2, so the default window is 2120 × 1360 device pixels.

### M7. The only route inside the bar is a behaviour, not a contract

- **OBSERVED.** `Painter::draw_live_surface` on the Win32 host copies rows
  with `memcpy` when the frame is the destination's size and opacity is 1
  (`windows_host.cpp:1404-1444`). A `LiveSurface` drawn that way from
  `on_paint` need not be registered with the window, and an unregistered
  surface arms nothing: the live clock is armed only while
  `has_live_surface_presentations()` (`:2919-2927`).
- **MEASURED** (Shadow, thirty turns of a full-window page from rest, request
  to presented):

  | Route | p50 ms | p95 ms | max ms |
  | --- | --- | --- | --- |
  | unregistered surface through the painter | 2.4 | 2.6 | 2.7 |
  | `draw_image`, image updated each turn (M1) | 139.0 | 148.3 | 184.4 |
  | `draw_image_region_sampled(nearest)` | 82.4 | 85.3 | 85.4 |
  | `fill_image_pattern`, one tile | 58.3 | 66.8 | 85.2 |
  | `RasterCanvas` | 89.5 | 97.0 | 125.0 |

  With the book's own compositor in the turn: 4.9 ms p50, 6.6 ms max in
  reading form. Idle: zero paints, frames, timers and cycles.
- **Why it matters here.** The owner's bar is 50 ms a page turn with zero
  work at rest (W303 C, I18). This is the only route at the pin that meets
  both on Windows, and the book is built on it
  (`src/forms/book_view.cpp`). Nothing in the toolkit's documents says an
  unregistered surface may be drawn through the painter, or that the copy
  path will stay.
- **Request.** Say so: that a control may draw a `LiveSurface` it has not
  registered, that a same-size full-opacity frame is copied, and that this
  arms no clock. Then give `draw_image` the same path (M1), so an image
  consumer is not sent to a surface for speed.

### M8. A live surface holds at least two buffers

- **OBSERVED.** `minimum_live_surface_buffer_count` is 2
  (`include/gui_forms/live_surface/types/live_surface_types.hpp:15`).
- **MEASURED** (Shadow): the surface route holds 23.1 MB beyond the control
  run for an 11.5 MB page. The book window with a spread open is 38.3 MB
  private against the empty window's 14.5 MB.
- **Why it matters here.** It is the same factor of two as M5 by another
  cause. A book that changes page only on a key has no producer racing a
  reader: one buffer written between paints would do.
- **Request.** Allow a pool of one for a surface whose producer and consumer
  are the same thread, or let a control hand the painter a borrowed frame of
  its own for the length of the paint.

### M9. A surface registration cannot be withdrawn, and holding one is not idle

- **OBSERVED.** `queue_live_surface_presentation` adds a registration;
  nothing removes it but the control's death or detachment
  (`src/core/window/presentation/window_presentation.cpp:285-307`, the erase
  at `:377`). While one exists the Win32 host keeps a 16 ms waitable timer
  armed (`windows_host.cpp:2919-2927`).
- **MEASURED** (Shadow, ten seconds at rest): 42.6 megacycles with a
  registration held against 0.2 to 1.0 for every other route; on the Windows
  CI runner 15.7 against 0.3 in five seconds; on the macOS runner 627 context
  switches against 5 to 10. The toolkit's own counters show nothing
  (`active_surface_ticks` 0, `frame_deadlines_fired` 0), and processor time
  as Windows charges it shows nothing either.
- **Why it matters here.** A registered surface presents within one clock
  period (p95 16.7 ms on Shadow), but it breaks zero work at rest, invisibly
  to the counters Stage 2's idle profile reads. Registering for the turn
  alone needs a child control added and removed, costs a second paint, and
  is no faster than not registering (2.7 against 2.4 ms).
- **Request.** A way to withdraw a registration, and the live clock's wakes
  in `MetricsSnapshot`.

### M10. On the X11 host a paint costs 17 to 25 ms whatever is drawn

- **MEASURED** (CI run 37171572751, Xvfb, 1060 × 680 at scale 1): a turn that
  only calls `fill_rect` takes 17.4 ms (p50) on x64 and 25.1 ms on arm64;
  every image and surface route is within two milliseconds of that, except
  `fill_image_pattern` at five more. The figure is the host's own
  `present_duration_nanoseconds`.
- **Why it matters here.** Linux is inside the 50 ms bar, with a floor under
  it that the application cannot lower. Together with M6 (a repaint on every
  wake) it is what a page turn costs there.
- **Request.** Send the damaged rows only, or a shared-memory image.
- **Does not establish:** the cost on hardware or under a compositing window
  manager.

### M11. The X11 host never drains surface registrations

- **MEASURED** (the same run): after `surface-turn` has removed its
  registrant, `has_live_surface_presentations()` is still true on both Linux
  runners and false on Windows and macOS. OBSERVED: `linux_host.cpp` has no
  call to `take_live_surface_presentations`, which is where a dead
  registration is erased.
- **Why it matters here.** Not at all on the route the book uses, which
  registers nothing. Recorded because a capability that is absent should be
  reported as unsupported.

### M12. `draw_image` is slower than `fill_image_pattern` over the same pixels

- **MEASURED** (Shadow, 2120 × 1360): 137.4 ms (p50) for `draw_image` and
  58.3 ms for `fill_image_pattern` with one tile the size of the page.
  OBSERVED: the pattern loop skips the per-pixel `Point`, clip and rectangle
  tests of `draw_image_region_sampled` (`windows_host.cpp:1543-1565` against
  `:1499-1515`). Not profiled.
- **Request.** None beyond M1; it shows how much of M1 is the tests in the
  loop.

### M13. Small things

- **OBSERVED.** `PhysicalKey` names the letters and the navigation keys and
  no digit (`include/gui_forms/events/input_events/input_events.hpp:87-`);
  the hosts do deliver them (`linux_host.cpp:44-45`). The book uses the HID
  usages 0x1E to 0x20 by number.
- **OBSERVED.** `third_party/sheenbidi/PROVENANCE.md` gives
  `829d99918adf05761e9f35611dc13e76e6810f80` as a commit; it is the `v2.9.0`
  tag object, and the commit is `83f77108a2873600283f6da4b326a2dca7a3a7a6`.
  The vendored source equals that commit's (compared).
- **OBSERVED.** `ApplicationWindowOptions::ready` runs on macOS before the
  window has its final client size: the probe saw 1060 × 680 at `ready` and
  1024 × 656 afterwards (CI run 37171572751). A control that sizes a resource
  at `ready` must size it again at paint; the book does.

## 2. Anticipated, not yet met

1. **Rich prepared text layout.** Spans with per-run face, size, colour and
   OpenType features; justification; cluster and word geometry for hit-testing
   and selection; identical output on every host. The application composes its
   own pages meanwhile (F010 A), so this is a request for the toolkit's own
   future, not a blocker.
2. **Application font registration for the painter path**, or a book role, so
   slips and menus can use the book face. M2 shows the Windows host's face list
   is fixed by file name; PlaySuite request 1 is the same need.
3. **A serif book face and a guarantee about cantillation positioning** for the
   bundled Hebrew face, if any toolkit-drawn text ever shows scripture.
4. **Anti-aliased path fill on the painter** (PlaySuite request 4): ribbons,
   thumb notches and map features are otherwise application-rasterized.
5. **Popup shadows / layered popups**: in-window overlays work; a native owned
   window has no shadow.

## 3. Anticipated items removed

- **Syriac face in a toolkit font pack.** No Syriac text, face or column ships
  (Y001 A).
- **Device scale on the painter.** The scale is available without the painter:
  `Window::scale()` (`include/gui_forms/window/window.hpp:254`), reachable from
  a control through `attached_window()`. PlaySuite's request stands for
  PlaySuite; this project can size page images from the window.
- **Horizontal wheel and touch for a tablet form.** A005 chose no tablet form.
- **Printing.** I013 A and H008 A: none; Copy is enough.
- **A larger text-mask budget or an SDK export of the service.** F010 A: the
  application sets its own type; the text-mask service is not built
  (`GUI_FORMS_BUILD_TEXT_MASKS OFF`).
