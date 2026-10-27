# Toolkit and sibling-project notes

> **Corrections from Stage 2, 2026-10-03.** The Stage 2 session re-opened the
> files this digest cites for §3, §4, §6 and §7 in the pinned toolkit
> (`falseywinchnet/file_manager` at `7b260cfb9f3267392e1470b0fcf4cd2497437819`,
> cloned to `.build/toolkit`) and in `games` at `c1e42a0` (PlaySuite 0.4.2,
> not 0.4.1) and `plan-paint` at `1ca31d9`. The lines below are OBSERVED
> directly, not via digest; where they differ from the text that follows, they
> prevail. §1, §2, §5 and §8 were not re-read.
>
> - **Checkout state (header).** `C:/Users/Shadow/file_manager` is now on
>   `codex/file-manager-preview-text-work` at `95b76f6` with uncommitted
>   changes under `gui_forms/`. It is not a build input; the pin is.
> - **§3 images.** Confirmed: `Window::load_bgra32_premultiplied`,
>   `update_…`, `patch_…` (`include/gui_forms/window/window.hpp:363-382`).
>   Limits per window: 4096 × 4096 and 64 MiB decoded per image, 1,024 images,
>   256 MiB decoded in total
>   (`include/gui_forms/resources/types/image_resource_types.hpp:22-32`). A
>   window may be built with its own limits (`window.hpp:239`).
> - **§3 images on Windows, new.** `Painter::draw_image` on the Win32 host is a
>   per-pixel software loop with no copy path for an unscaled opaque image
>   (`src/host/windows/application/windows_host.cpp:1371-1379`, `:1471-1516`).
>   MEASURED: 145–166 ms to paint one full-window image (2120 × 1360 device
>   pixels at scale 2), against 1.2 ms for
>   `fill_rect` of the same area. See `docs/TOOLKIT_REQUESTS.md` M1.
> - **§3 scale.** The device scale is available as `Window::scale()`
>   (`window.hpp:254`); the painter does not expose it.
> - **§4 fonts.** Of the seventeen bundled faces every host requires only the
>   first seven: Portsmouth Rapids regular and bold, Carlito in four styles,
>   Cousine regular (`windows_host.cpp:3479-3497`, `linux_host.cpp:123-160`,
>   `macos_host.mm:992-1005`). The other ten, 18.5 MB of the 21.9 MB directory,
>   are optional. Without the seven the renderer reports itself incomplete
>   (Windows, macOS) or the host throws (Linux).
> - **§4 Windows profile.** Confirmed by the running host: renderer name
>   “Win32 DIB CPU · Uniscribe/GDI text · WIC PNG · bundled fonts” with Skia and
>   HarfBuzz off. The macOS host cannot be configured without Skia
>   (`gui_forms/CMakeLists.txt:52`), nor the Linux host (`:842`).
> - **§4 text stack.** libunibreak is fetched only for the text-mask service
>   (`third_party/fetch_linebreak.sh:10`); with text masks off it is not built.
>   SheenBidi is vendored in the toolkit tree; HarfBuzz and FreeType are
>   fetched at pinned commits by `third_party/fetch_text_stack.sh`.
> - **§6 PlaySuite's pin.** Confirmed: `7b260cf…`
>   (`games/tools/package_common.py:10`, `applications.yml:77`). This project
>   pins the same commit (`cmake/toolkit.lock.json`).
> - **§6 licence.** The toolkit's `LICENSE` is MIT with the owner's dedication
>   line; the digest did not record it.
> - **§7 idle protocol.** In PlaySuite `--profile-idle N` takes a view index
>   (−1 for the shelf, 0–16 for a game), not a duration; the sample is always
>   ten seconds (`games/src/main.cpp:26-49`, `:64-69`, `:86-97`;
>   `games/docs/PERFORMANCE.md`). The collector is external and Windows-only
>   (`games/tools/profile_windows.py`).
> - **§7 metrics receipt.** `print_metrics_on_close` prints one JSON line,
>   `{"stable_id":"main","window":{…},"host":{…}}`, with `frames_presented`,
>   `paint_passes` and `renderer_name` under `window`. `reset_activity_metrics`
>   zeroes `frames_presented` (`src/core/metrics/metrics/metrics.cpp:14-40`),
>   so a receipt after an idle profile counts only frames since the sample began.
> - **§9 evidence gaps.** “No accepted idle-memory figure for GUI.Forms itself”
>   is closed for the empty window by `planning/BUDGETS.md`.

Read boundary: digests made on 2026-10-03 by four read-only passes over
`C:/Users/Shadow/file_manager` (branch `codex/file-manager-budgeted-scan-reader`),
`plan-paint` (1.1.3), `games` (PlaySuite 0.4.1) and `notepad` (SwiftEdit 0.3.7).
Every line below was reported with a locator in those trees. Status is
**OBSERVED via digest**: re-open the named file before relying on a line, and
correct this document where it has drifted. Lines marked INFERRED are mine.

## 1. The laws of GUI.Forms

There is no single laws file. `file_manager/planning/README.md` names an
`INVARIANTS.md` that does not exist. The rules are spread over:

`gui_forms/AGENTS.md` “Hard boundaries”
- Retained core. No immediate-mode control API, no perpetual redraw loop.
- No GPU capability of any kind.
- Skia only behind a private CPU adapter; no Skia type in public headers.
- Windows, macOS and Linux are equal targets; differences are capability-reported.
- PNG is the only image decoder.
- Platform types stay inside `src/host/<platform>/`.
- Applications other than File Manager use owned popup dialogs, not persistent
  side panels (also `decisions/ADR-013`).
- Host fonts are not used silently; direction is HarfBuzz + FreeType + bundled packs.
- Instrumentation is queryable as structured counters.
- Nothing is called fast or lightweight without a recorded workload and measurement.

`gui_forms/planning/MASTER_IMPLEMENTATION_PLAN.md` §1.1
- The application owns semantic state; the toolkit retains presentation state only.
- Mutations are synchronous and typed; undeclared invalidation fails in development builds.
- Ownership is parent-to-child strong, child-to-parent weak; subscriptions are tokens.
- One deterministic event and property order (`gui_forms/decisions/ADR-003`).
- A retained semantic graph backs accessibility.
- Non-goals: no web renderer, no invisible native text overlay, no frame-driven tree rebuild.

Lifecycle (`gui_forms/docs/LIFECYCLE_CONTRACT.md`, `decisions/ADR-014`)
- Four identities: retained control, window/session, native host, handle lease.
- Phases `constructed → attached → close_authorized → closed → shutdown`; an
  illegal transition returns `invalid_lifecycle` without mutating state.
- Load runs after attach and before first presentation. No half-initialized
  frame; no input before Load.

Paint (`gui_forms/planning/PAINT_PIPELINE_AVAILABILITY.md`)
- The complete-frame law: mutation rate may exceed render rate, which may exceed
  present rate, “without an unbounded queue or incomplete pixels becoming
  visible”. A paint failure restores the prior chunks. Input never re-enters
  paint. Paint is host-demand driven; no repaint when idle.

Dispatcher (`gui_forms/docs/CURRENT_API_REFERENCE.md`)
- One FIFO snapshot per turn; no nested message pump; queue bounded at 4,096.

Other
- Capability honesty: absence is reported as `unsupported`, never simulated.
- Text: layout consumes measured shaped runs; fallback is per indivisible
  cluster; never estimate by code-point count. Owner verdict T3A: greyscale
  antialiasing by default, no globally forced LCD
  (`planning/gui_forms/GUI_FORMS_ACCESSIBILITY_AND_TEXT_ROUND_001.md`).
- Topology: one primary document surface; secondary tools are owned popups or a
  bounded overlay; zero idle animation
  (`gui_forms/planning/FUTURE_APPLICATION_CONSUMER_PROFILE.md`).

## 2. What “the Plan Paint enhancements” are

`gui_forms/docs/PLAN_PAINT_BACKPORT_2026-09-29.md`: the toolkit as it evolved
inside Plan Paint (release v0.2.3, source commit `64248bcc06a1`), re-imported
into File Manager as commit `ddade5b`, tag `v0.001-alpha.20260930`. It brought
the portable `Application` interface, the Windows package and Linux X11 host,
clipboard/image/drop services, popup focus and lifetime fixes, text fallback and
memory-mapped fonts, MSAA and ATK accessibility adapters, custom cursors, and
the Paint-era controls (`RasterCanvas`, `DropDownButton`, full-screen toggle,
`arrange_scroll_viewport`). Declared limits: Windows GDI profile with Skia and
HarfBuzz disabled; MSAA not UIA; Linux X11/XWayland.

Plan Paint itself carries, as application code (not toolkit): the fibre renderer
(`src/carpet.cpp`), paper-tooth media (`src/material.cpp`), OKHSL themes,
printing per platform, hardened codecs, localization packs, atomic safe files
and recovery, and font inventory.

## 3. What an application can draw

`Painter` (`gui_forms/include/gui_forms/types/painter/painter.hpp`): save,
restore, translate, rectangular and rounded clip; fill and stroke of rectangles
and rounded rectangles; lines; linear and radial gradients; outer and inset box
shadows; `draw_text_utf8` and `measure_text_utf8`; `draw_image`,
`draw_image_region[_sampled]` with opacity, `fill_image_pattern`,
`draw_live_surface`. **No path fill, no rotation or scale, no blend modes, no
masks.**

Per-pixel routes: `Window::load_bgra32_premultiplied` / `update_…` / `patch_…`
(4096 × 4096 per image); `LiveSurface` (triple-buffered, latest complete frame);
`RasterCanvas`. Images: PNG only.

Popups: `Window::open_popup` gives in-window overlays with focus scopes. Native
owned windows must be declared at startup (at most 64; no creation afterwards);
no drop shadow on native popups.

Scrolling is stepwise; no inertial scrolling. Input: pointer, wheel, keys with
preview/target/bubble phases, text input, inbound drag and drop. No touch or
gestures. Timers: `Timer`, `schedule_ui_timer`, `AnimationTimeline` with a
reduced-motion policy. Printing: absent. Localization catalogue: absent.
Accessibility: semantic snapshot graph; MSAA, ATK, AppKit.

INFERRED consequence: paper, ribbons, notches, curved-page shading and maps are
application-rendered pixels uploaded as images, or PNG tiles.

## 4. Text

Two stacks exist.
- Portable private stack: HarfBuzz + FreeType + SheenBidi 2.9.0 + libunibreak
  8.0 + in-house Unicode 17 grapheme tables; no ICU. Used on macOS and Linux
  under the Skia renderer.
- Windows shipped profile: GDI + Uniscribe with ClearType; Skia and HarfBuzz off
  (`gui_forms/tools/Build-Windows.ps1`).

`Painter::draw_text_utf8` takes one string and one
`FontSpec{role ∈ control|content|monospace, size, weight, italic, letter_spacing}`.
No way to name a family or register an application font on this path. No spans,
superscripts, small capitals, OpenType feature control, justification,
hyphenation, or public cluster hit-testing.

`TextMaskService` (`include/gui_forms/text_mask/**`, profile
`logical_wrapped_mask_v1`): source-only behind `GUI_FORMS_BUILD_TEXT_MASKS`; SDK
install refused. Application font banks (≤ 8 faces, 4 MiB each, 8 MiB total);
one UTF-8 paragraph ≤ 16 KiB and 256 lines; full bidi, contextual shaping, UAX
#14 greedy wrap; asynchronous; returns a grey coverage mask and per-line
records. Not offered: hyphenation, justification, per-span fonts or sizes,
cluster geometry. PlaySuite uses it (`games/src/game_text.hpp`,
`games/docs/PORTABLE_TEXT_REQUIREMENTS.md`) under the complete-frame rule.

Prepared text and DocumentView D1 exist as development-only or plain-text
protocols; D2–D4 and printing are proposals. None is rich typesetting.

Bundled toolkit fonts: Portsmouth Rapids (owner evaluation build; redistribution
unresolved), Carlito, Cousine, Noto Sans CJK JP, Noto Emoji, Noto Sans Arabic,
Hebrew, Devanagari, Bengali, Gurmukhi. **No Syriac face, no serif book face, no
dedicated polytonic Greek face.**

Measured: Windows GDI text 3.6 s → about 8.5 ms per frame for 100 captions after
caching; a full 1340 × 850 repaint at 2× was about 0.4 s before fill
optimizations (`gui_forms/results/windows-text-latency-2026-09-29/README.md`).
SwiftEdit: 10,000 distinct words cost 760–1,654 ms of layout before slicing;
per-word native measurement is the cost centre
(`notepad/performance/2026-10-02-markdown-visibility/README.md`).

## 5. Materials in Plan Paint

Fibre renderer (`plan-paint/src/carpet.cpp`, `docs/CARPET.md`): deterministic
PRNG generates up to 380,000 pile fibres and 160,000 web fibres as capsule
segments; analytic ray/capsule rasterization into an orthographic z-buffer with
a light-view depth raster; Marschner-style R/TT/TRT lobes with Fresnel and
Beer–Lambert absorption, soft shadows, ambient occlusion; double precision, no
fast-math; 2 × 2 supersampling; tiles by cross-blending a 32-pixel overlap.
Measured 0.63–1.02 s per 512 tile on Apple Silicon; a cached tile is 1 MiB;
transient memory roughly 80 MiB (INFERRED). No woven warp/weft, laid lines,
deckle or curvature.

Paper-tooth media (`plan-paint/src/material.cpp`): three octaves of hashed value
noise plus an anisotropic fibre term, used to modulate pigment. It does not draw
visible paper.

Compositing pattern (`plan-paint/src/forms/surface.cpp`): premultiplied BGRA
tile, `fill_image_pattern`, gradient overlay, box shadow.

## 6. How applications consume the toolkit

- PlaySuite: pinned source, `add_subdirectory`, commit
  `7b260cfb9f3267392e1470b0fcf4cd2497437819`; links `GUIForms::Application`,
  `GUIForms::Audio`, `gui_forms_text_masks`; `GUI_FORMS_ENABLE_HARFBUZZ_TEXT ON`,
  Skia off on Windows (`games/CMakeLists.txt`, `games/cmake/Application.cmake`).
- SwiftEdit and Plan Paint: hash-pinned installed SDK,
  `find_package(GUIForms 0.1 CONFIG REQUIRED COMPONENTS Application)`.
- Toolchains: Windows MinGW-w64 GCC 16.2.0 at
  `C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin`, Ninja, CMake;
  macOS Homebrew llvm@20 (PlaySuite); Linux clang, X11. No MSVC lane.
- Entry: `gui_forms::make_control<T>(StableId)`, `gui_forms::Window`,
  `ApplicationWindowOptions`, `Application::run` (`games/src/main.cpp`).
- Views subclass `gf::Control`, override `arrange`, `on_paint`, `on_pointer`,
  `on_key`, `hit_test_local`; animate with `gf::Timer` that stops when settled.

## 7. Delivery practice to copy (PlaySuite)

- `.github/workflows/applications.yml`: `prepare` (assets, style check), a
  four-runner matrix (windows-2022 MSYS2, macos-15 arm64, ubuntu-24.04,
  ubuntu-24.04-arm), tests with `ctest --timeout 180`, tag-gated `release` with
  `SHA256SUMS.txt`.
- Packaging scripts in `games/tools/`: Windows import-closure walk, zip and
  per-user NSIS installer; macOS `pkgbuild` .pkg with ad-hoc signature; Linux
  tar.gz and .deb. Each bundle carries `package-manifest.json` and `licenses/`.
- Smoke: `--smoke-test` must present at least one frame; installers are
  installed, smoke-run and uninstalled in CI with isolated state.
- Storage: header with magic, version and checksum; write temporary then rename;
  load refuses oversize or mismatched files and assigns only on full success
  (`games/src/storage.cpp`). Plan Paint's `safe_file` adds fsync and bounded reads.
- State directory with environment override (`games/src/runtime_paths.cpp`).
- Idle profile protocol: `--profile-idle N`, 5 s warm-up, 10 s sample
  (`games/docs/PERFORMANCE.md`). Shelf: 0.0% CPU, 34.2 MiB private after work.
- Minimum window 600 × 420 tested for every view.
- Style check: `games/scripts/check-style.py` flags `auto`, `->`, coroutines,
  ranges, defaulted comparisons and lambda-shaped brackets.
- Tests: no framework; one executable per test file with a local `require()`.
- Credits: “Author: Astra / Sponsor: Rainstar”; dedication first.

## 8. Search in File Manager, as it actually is

- Law (`engine/docs/QUERY_AND_RANKING.md`; root `AGENTS.md`): “Candidate
  generation is not ranking; ranking is not identity; cutoff is not candidate
  generation.” Approximate indexes propose; exact records are authoritative.
- Implemented: a lowered-substring exhaustive scan with page budgets and
  continuation (the oracle any accelerator must match); an exact name index; a
  generation-bound cursor; cooperative cancellation; coverage flags surfaced to
  the reader (“search incomplete”, “partial search”).
- One fuzzy channel, disabled and off the public path
  (`engine/internal/similarity/`): a hashed single-deletion neighbourhood over
  Unicode scalars (length 2–64, modulus 2^61−1, two coupled 20-bit cells, 12-byte
  postings) with an O(n) exact one-edit verifier that classifies substitution,
  transposition, insertion or deletion. Every hit is `candidate_only`. Measured
  on one million names: p95 0.276 ms against 85 ms for a flat scan.
- Not implemented: n-gram indexes, radius two, relevance ranking, any semantic
  channel. Zeta's research notes describe trigram and bigram term maps, Jaccard
  filtering, bounded Damerau–Levenshtein, BM25F-like scoring
  (`research/zeta_search/ALGORITHM_INVENTORY.md`).
- REJECTED as a semantic claim: ConeDAG similarity. It lost to plain lexical
  search on judged semantic queries (0.583 against 0.750)
  (`planning/EVIDENCE_REGISTER.md`; `research/zeta_search/FILE_MANAGER_APPLICABILITY.md`).
  Kolmogrov's ledger: “semantic equivalence is not supplied by lexical geometry
  alone.”
- Storage spine (`engine/internal/generation/format.go`): magic, 512-byte header
  with SHA-256, components as fixed-width sorted little-endian arrays each with
  offset, length, count and digest; no compression until it earns admission.

INFERRED for this project: at about 5 MB of scripture an exhaustive scan is
already interactive and is the mandatory oracle. Error tolerance applies per
word against the vocabulary, then through word-to-verse postings. Greek and
Hebrew need a declared, versioned folding (accents, final sigma, points), which
the engine deliberately lacks. The honest “semantic” channel available here that
File Manager never had is the original-language word behind the English.

## 9. Evidence gaps

- No accepted idle-memory figure for GUI.Forms itself.
- Cantillation coverage of the bundled Noto Sans Hebrew is unverified.
- Windows IME, horizontal wheel and touch are absent or unverified.
- The text-mask service is unfrozen; its budgets (9 request slots, 32 MiB live
  coverage) are untested against a dense four-script page.
- Nothing above was re-read by the author of the stage plan after the digest.
