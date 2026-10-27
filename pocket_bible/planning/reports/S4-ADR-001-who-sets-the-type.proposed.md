# ADR-001 (proposed): who sets the type, and how its libraries are linked?

Status: **proposed**. Not a decision until the owner's approval line is filled.
Date: 2026-10-03
Owner approval: pending

Written by the Stage 4 session as the handoff asks (part two, item 1). It
lives under `planning/reports/` because that is the directory Stage 4 owns;
on approval the coordinator moves it to `decisions/ADR-001-WHO-SETS-THE-TYPE.md`
and adds it to the index. The measurements it rests on are in
`planning/reports/S4-2026-10-03.md` and under `experiments/`.

## GIVEN constraints

- The application owns a page compositor that links HarfBuzz, FreeType,
  SheenBidi and libunibreak at the toolkit's pinned versions and renders
  whole pages to pixels (F010 A; INVARIANTS I16).
- The toolkit checkout is never modified (I33). No dependency is added
  without asking (I34).
- Greyscale antialiasing, light hinting, linear-light blending (I17).
- A page turn within 50 ms; zero work at rest (W303 C; I18).
- The page keeps its proportions and scales with the window; three type
  sizes, each with its own page breaks (R020 A; J003 B).
- Ceilings: 100 MB installed, 150 MiB resident, 400 ms to a readable page
  (I29–I31).

## Workloads and failure modes

The three proving passages (Mark 7:14–23 with Greek, Genesis 1:1–13 with
pointed Hebrew, Psalm 87 with Hebrew), in reading and study form, in both
Latin faces, at three type sizes, at 600 × 420, the default window and a
large window at display scale 2.

Failure modes the record must exclude: a second version of a text library in
one process; a library the toolkit does not pin; a page whose line breaks
change with the window; a page that differs between platforms; a shipped
binary that cannot be rebuilt from pinned inputs.

## Candidates

- **A. The application composes; the four libraries are built from source at
  the toolkit's pins.** (F010 A.)
- **B. The toolkit's text-mask service per run** (F010 B): one grey mask per
  paragraph, laid out by the toolkit.
- **C. Shaping done at build time and stored in the container** (F010 C):
  the client only places and rasterizes glyphs.
- **D. A prepared rich-layout interface in the toolkit** (F010 D).

## Evidence and measurements

All MEASURED in Stage 4 unless another label is given.

1. **The libraries.** `cmake/page_text_stack.lock.json` names four commits:
   HarfBuzz `56feae4`, FreeType `f4205da`, libunibreak `28a2756`, SheenBidi
   `829d999` (the `v2.9.0` tag; commit `83f7710`). `cmake/Page.cmake` refuses
   to configure the application unless the pinned toolkit's own files name
   the same four (`third_party/fetch_text_stack.sh`, `fetch_linebreak.sh`,
   `sheenbidi/PROVENANCE.md`).
2. **How they are linked, per platform.**

   | Platform | HarfBuzz, FreeType, SheenBidi | libunibreak |
   | --- | --- | --- |
   | Windows x64 | Fetched to `.build/text_stack/` and built here as static libraries with the toolkit's options; linked into the executable. The toolkit's Windows profile builds none of them. | Fetched and built here, static, in the executable. |
   | macOS arm64, Linux x64 and arm64 | The toolkit's own CMake targets (`harfbuzz`, `freetype`, `gui_forms_bidi`) are linked: one build of each, at the pin by construction. At this pin the linker binds the calls to the copy already inside the toolkit's shared library, so nothing is duplicated. | Fetched and built here, static, in the executable: the toolkit builds it only for its text-mask service, which is off. |
   | Toolkit-free configuration, every platform | Fetched and built here, static. | The same. |

   MEASURED on the Linux x64 package of CI run 37182223062: `readelf` shows
   35 `hb_`, `FT_` and `SB` symbols undefined in `pocket-bible` and defined
   in `libgui_forms_application.so.0`. HYPOTHESIS for macOS and Linux arm64,
   from size alone: their executables grew by the embedded assets plus about
   0.2 to 0.3 MB, too little to hold HarfBuzz. If a later pin stops
   exporting those symbols, the static targets on the same link line supply
   them: the same version either way.
3. **Nothing in the toolkit checkout is written.** The sources this project
   fetches go to `.build/text_stack/`; on macOS and Linux the toolkit's own
   fetch script, which CI already ran in Stage 2, puts its copies in its
   ignored `third_party/` directories.
4. **What it adds to each floor** (packaged application, same procedure as
   `planning/BUDGETS.md`; Stage 2's figure → Stage 4's; the faces and
   fixtures of the proving slice, 2,742,826 bytes, are compiled in and are
   part of every figure):

   | | Windows x64 | macOS arm64 | Linux x64 | Linux arm64 |
   | --- | --- | --- | --- | --- |
   | Executable, bytes | 82,074 → 5,358,807 | 98,912 → 3,068,336 | 71,984 → 3,115,312 | 67,664 → 3,082,320 |
   | Installed, bytes | 12,608,779 → 17,885,512 | 12,951,580 → 15,921,004 | 15,696,566 → 18,739,894 | 14,525,310 → 17,539,966 |
   | Share of 100 MB | 12.6% → 17.9% | 13.0% → 15.9% | 15.7% → 18.7% | 14.5% → 17.5% |
   | First frame, ms (median of 7) | 104 → 30 | 227 → 172 | 43 → 45 | 56 → 58 |
   | Private memory at rest, MiB | 5.0 → 11.1 | 26.3 → 31.8 | 14.6 → 23.6 | 14.0 → 22.9 |
   | Resident memory at rest, MiB | 16.9 → 24.5 | 77.3 → 92.5 | 21.4 → 30.3 | 20.2 → 29.2 |

   CI run 37182648303, scale 1 on every runner. On Shadow at scale 2 the
   first frame went from 211 to 65 ms, private memory from 13.8 to 36.5 MiB
   and resident from 25.5 to 48.8 MiB; the two page buffers at 2120 × 1360
   are 22 MiB of that.

   The Stage 4 window shows a composed spread of the slice where Stage 2's
   showed a 64 × 64 flat image, so the launch and memory rows include the
   work and the two page buffers, not only the libraries.
5. **Composition.** A spread of the slice is drawn in 1.1 ms (p50) and 2.9 ms
   (max) at 2120 × 1360 on Shadow, 2.3 and 4.6 ms at 2820 × 1721; laying out
   all three passages takes 1.3 to 5.9 ms depending on the setting. A page
   turn in the window, composing included, is 4.9 ms (p50) and 6.6 ms (max)
   in reading form on Shadow. (Experiment 003; experiment 001.)
6. **Determinism.** Layout is in units of a millionth of the page height and
   all arithmetic that reaches a pixel is integer. Eight stored reference
   renderings compare byte for byte on Windows (GCC 16), macOS arm64
   (Clang 20) and Linux x64 and arm64 (GCC 13).
7. **Shaping quality.** All 6,536 Hebrew sequences and every Greek word form
   shape without a missing glyph in the chosen faces (experiment 004).

## Decision

Proposed: **A.** The application sets its own type with HarfBuzz, FreeType,
SheenBidi and libunibreak at the commits the pinned toolkit names, recorded
in `cmake/page_text_stack.lock.json` and checked against the toolkit at
configure time; built from source by this project where the toolkit does not
build them, and taken from the toolkit's own targets where it does.

## Why the other candidates lost

- **B, text masks.** OBSERVED in `planning/TOOLKIT_NOTES.md` section 4: no
  justification, no per-span faces or sizes, no cluster geometry, one
  paragraph of at most 16 KiB, asynchronous. A justified page with raised
  numerals, a drop numeral, two scripts and a hit map cannot be built from
  it. Not measured further.
- **C, shaping at build time.** MEASURED on the same passages (Shadow, five
  runs each, `page_compose`): laying the book out with shaping takes 1.7 to
  1.9 ms in reading form and 3.6 to 5.5 ms in study form; laying it out again
  with every word already shaped — the most that stored shaping could save —
  takes 0.8 to 1.3 ms and 1.5 to 3.0 ms. Stored shaping would save one to
  three milliseconds on these passages, once per setting, and would put glyph
  numbers of particular font files into the container. Nothing in the
  measurements asks for it. Reopen if a whole book's first open is measured
  over the launch budget.
- **D, a toolkit interface.** Does not exist at the pin; filed as a request
  (`docs/TOOLKIT_REQUESTS.md` section 2, item 1). The product cannot wait on
  it and the owner chose A.

## Consequences

- On Windows the executable carries its own HarfBuzz and FreeType: about
  2.5 MB of code beside the faces. On macOS and Linux it carries libunibreak
  and the compositor only.
- The four commits are a dependency this project holds in its own lock file.
  A new toolkit pin that moves any of them stops the configure until the
  lock is moved with it.
- The first configure fetches the four repositories from the network (one
  commit of each); a configured tree builds offline.
- Each package must carry the licence texts of HarfBuzz, FreeType (already
  carried on macOS and Linux), SheenBidi and libunibreak, and now of
  HarfBuzz and FreeType on Windows too. The packaging scripts are not Stage
  4's; this is a step for the coordinator.
- The compositor draws what a face lacks; a face is never required to carry
  a glyph for the page to be set (F302 note).

## Reversal and migration path

Cheap. The compositor reaches the libraries only through
`src/page/font_set.cpp` (HarfBuzz, FreeType) and `src/page/text_layout.cpp`
(SheenBidi, libunibreak): about 400 lines. Moving to a toolkit interface (D)
replaces those two files' calls; the page model, the breaker, the page
builder, the rasterizer's blending and the hit map do not change. Moving to
build-time shaping (C) replaces the shaping cache's source and keeps the rest.

## Unresolved edges

- The symbols the toolkit's library exports on macOS and Linux are not a
  promise. The link works with or without them; the executable's size on
  those platforms would grow by the libraries if they went.
- F009 B asks for "a fixed, measured correction" to linear-light blending.
  None is applied; what it is measured against is the owner's to say
  (S4 report, question 3).
- macOS is the tightest platform on memory: 92.5 MiB resident with a spread
  open against the 150 MiB ceiling, of which 77.3 MiB was already there in
  the empty window.
- Embedding the faces in the executable is how the slice travels, not a
  decision; Stage 3's container or files beside the executable may carry
  them later.
