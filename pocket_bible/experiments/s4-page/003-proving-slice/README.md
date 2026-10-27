# Experiment 003 — the proving slice composed

Stage 4, part two, item 7. Status: **MEASURED** on the owner's machine and on
the four CI runners. Every machine is virtual.

**Question.** With the application setting its own type, what does a page of
the proving slice cost: to lay out, to draw, to hold in memory — at 600 × 420,
at the default window and at a large window at display scale 2, on every
platform? And is anything gained by computing page breaks ahead of time
rather than on first open (J002 C)?

**Hypothesis, and what would falsify it.** Composing a spread is a small part
of the 50 ms a page turn is allowed, at every size, so pages can be composed
when they are asked for and none need be kept. Falsified by a p95 over about
20 ms anywhere. **Not falsified**; one draw on one runner exceeded it (below).

**Data.** The three passages in `assets/fixtures/` (experiment 002), the six
faces in `assets/fonts/`, compiled into the tool.

**Environment.** Shadow: QEMU guest, AMD EPYC 9354, Windows 11 22621, MinGW
GCC 16.2.0, Release, in interactive use. CI run 37182648303 at commit
`cb914e6`: `windows-2022` (GCC 16.2.0), `macos-15` arm64
(Clang 20), `ubuntu-24.04` x64 and arm64 (GCC 13.3.0).

**Procedure.**

```
cmake --build .build/app --parallel 2 --target page_compose
.build/app/page_compose --form reading|study --face garamond|portsmouth --step small|medium|large \
    --width W --height H [--page N] [--png FILE] [--measure 90]
```

`page_compose` loads the faces and fixtures, lays out the whole book for the
setting (timed: `layout_ms`), draws one spread into memory (timed:
`first_draw_ms`, with an empty glyph cache), then with `--measure` draws 90
more spreads walking through the book (`draw_ms`). It checks the hit map
against itself (every word's centre must find that word) and can write the
spread as a PNG. On CI the same runs are tests (`page_compose_*`); the result
lines are read from `Testing/Temporary/LastTest.log`. Widths and heights are
device pixels: 600 × 420 and 1060 × 680 are the minimum and default windows
at scale 1; 1200 × 840, 2120 × 1360 and 2820 × 1721 the minimum, the default
and the largest Shadow's display allows at scale 2.

## Results

### Drawing a spread (ms: p50, p95, max over 90 draws), EB Garamond, medium

| Window | Form | Shadow | Windows CI | macOS CI | Linux x64 CI | Linux arm64 CI |
| --- | --- | --- | --- | --- | --- | --- |
| 600 × 420 | reading | 0.2, 0.3, 0.9 | 0.1, 0.2, 0.9 | 0.3, 1.0, 1.5 | 0.2, 0.3, 1.2 | 0.2, 0.2, 0.8 |
| 600 × 420 | study | 0.3, 0.5, 2.0 | 0.3, 0.3, 1.9 | 0.2, 0.3, 1.7 | 0.3, 0.3, 1.9 | 0.3, 0.3, 1.8 |
| 1200 × 840 | reading | 0.5, 0.9, 1.6 | 0.4, 0.6, 1.6 | 0.5, 1.0, 2.9 | 0.4, 0.6, 1.3 | 0.5, 0.7, 1.3 |
| 1200 × 840 | study | 0.7, 1.2, 3.1 | 0.7, 0.9, 2.7 | 0.6, 1.0, 2.3 | 0.7, 0.9, 2.7 | 0.7, 1.0, 2.6 |
| 2120 × 1360 | reading | 1.1, 1.8, 2.9 | 1.0, 1.3, 2.2 | 2.4, 6.0, **50.9** | 0.9, 1.2, 1.9 | 1.1, 1.4, 1.9 |
| 2120 × 1360 | study | 1.6, 2.9, 4.6 | 1.4, 1.9, 3.8 | 1.6, 3.2, 3.5 | 1.3, 1.9, 3.7 | 1.4, 2.1, 3.7 |
| 2820 × 1721 | reading | 2.3, 3.5, 4.6 | 1.6, 2.0, 2.9 | 3.8, 5.8, 7.1 | 1.4, 1.9, 2.5 | 1.6, 2.1, 2.5 |
| 2820 × 1721 | study | 3.1, 5.3, 6.5 | 2.4, 3.2, 6.3 | 3.7, 5.2, 7.2 | 2.0, 2.9, 4.9 | 2.2, 3.1, 4.9 |

One draw of ninety on the macOS runner took 50.9 ms (reading, 2120 × 1360);
the other 89 were at or under 6.0 ms. Its cause is not established; the same
runner's study run at that size peaked at 3.5 ms.

The first draw at a size, which rasterizes every glyph it meets, took 2.0 to
8.8 ms across all of the above on every machine except one macOS run at 15.3 ms
(run 37182223062, reading, 2820 × 1721).

### Laying out the book (all three passages; ms)

| Setting | Shadow | Windows CI | macOS CI | Linux x64 CI | Linux arm64 CI |
| --- | --- | --- | --- | --- | --- |
| reading, medium, EB Garamond (6 pages) | 1.8 to 2.5 | 1.6 to 2.8 | 1.1 to 3.1 | 1.3 to 2.1 | 1.3 to 1.4 |
| study, medium, EB Garamond (8 pages) | 4.0 to 5.9 | 3.6 to 3.7 | 2.7 to 2.9 | 3.3 to 3.4 | 3.3 to 3.4 |

How much of that is shaping (Shadow, five runs each, medium, EB Garamond):
with shaping, 1.7 to 1.9 ms in reading form and 3.6 to 5.5 ms in study form;
the same layout again with every word already in the shaping cache, 0.8 to
1.3 ms and 1.5 to 3.0 ms. This is the most that shaping at build time could
save (candidate C of the proposed decision record).

On Shadow, across all twelve settings (two forms, two faces, three sizes):
1.3 to 5.6 ms; 6 to 14 pages; no missing glyph, no overfull line, no
overflowing page; 0 to 10 lines set looser than twice their natural spaces
(the price of justifying without hyphenation, R006 B — most in Portsmouth at
the large size, whose letters are wide).

### The whole page turn in the window (ms: p50, p95, max over 30 turns)

From experiment 001's probe with the real `BookView`: request, compose,
publish, paint, present.

| Form | Shadow, 2120 × 1360 | Windows CI, 1028 × 680 | macOS CI, 1024 × 656 | Linux x64 CI, 1060 × 680 | Linux arm64 CI, 1060 × 680 |
| --- | --- | --- | --- | --- | --- |
| reading | 4.9, 6.2, 6.6 | 1.1, 1.8, 2.3 | 2.0, 2.8, 5.3 | 15.2, 15.8, 16.1 | 25.9, 26.4, 26.7 |
| study | 5.2, 8.2, 9.3 | 1.3, 2.5, 4.0 | 2.1, 3.5, 4.9 | 15.4, 16.4, 17.6 | 27.0, 28.2, 28.9 |

On Linux the host's own paint is 13.6 ms (x64) and 25.5 ms (arm64) of that
with nothing drawn (`docs/TOOLKIT_REQUESTS.md` M10).

### Memory (Shadow, EB Garamond, medium)

| What | Reading form | Study form |
| --- | --- | --- |
| The laid-out book, all pages | 117 kB for 6 pages | 205 kB for 8 pages |
| Shaping cache | 380 words, 1,788 glyphs, 6 kB of keys | 645 words, 3,638 glyphs, 13 kB of keys |
| Glyph coverage cache at 2120 × 1360 | 96 kB, 488 glyphs | 202 kB, 925 glyphs |
| Glyph coverage cache at 2820 × 1721 | 147 kB | 323 kB |
| Page pixels held (the surface's two buffers) | 23.1 MB at 2120 × 1360 | the same |

A laid-out page is about 20 to 26 kB: its glyph runs, word boxes and verse
texts. A shaped glyph is 20 bytes. The glyph cache is emptied when the window
changes size.

### Launch and rest, with a spread of the slice open

| | Shadow (scale 2) | Windows CI | macOS CI | Linux x64 CI | Linux arm64 CI |
| --- | --- | --- | --- | --- | --- |
| First frame, ms, median of 7 (range) | 65 (59–67) | 30 (28–39) | 172 (151–257) | 45 (39–47) | 58 (50–60) |
| Stage 2's empty window | 211 | 104 | 227 | 43 | 56 |
| Paints, frames, timer ticks at rest, 10 s and 30 s | 0, 0, 0 | 0, 0, 0 | 0, 0, 0 | 1 paint each (M6) | 1 and 2 paints (M6) |
| Processor time at rest, 10 s; 30 s | 0 ms; 187.5 ms | 0; 0 | 15.0; 20.3 ms | 14.8; 14.9 ms | 25.7; 50.8 ms |

Shadow's 30-second sample was taken while the desktop was in use; Stage 2 saw
the same there with the empty window and did not establish its cause either.

## Page breaks ahead, or on first open (J002 C)

- MEASURED: laying out and paginating the three passages (30 verses of English
  in reading form; with their originals in study form) costs 1.3 to 5.9 ms on
  Shadow, and less on every CI runner. That is the cost of "on first open"
  for this much text: under 0.2 ms a verse.
- HYPOTHESIS, by proportion only: a long chapter (Psalm 119, 176 verses)
  would be about 30 ms, and the whole of the BSB (31,086 verses) several
  seconds. A chapter is the smallest unit opened alone (I24), so first open
  paginates a chapter, not the book — but a page *number* that counts from
  the front of the book needs every earlier chapter's page count.
- What a table made ahead would hold: one page count per chapter per setting
  (two forms, two faces, three sizes: twelve numbers for each of 1,189
  chapters, about 14 kB as bytes) — enough to number pages and to open at a
  page without laying out what precedes it. The breaks themselves need not
  be stored: any machine recomputes them identically, as the reference
  renderings show.
- So the measurements favour neither extreme: lay the open chapter out on
  first open (it is far inside the budget), and ship only the per-chapter
  page counts if page numbers are wanted. This is CANDIDATE, for the owner
  (S4 report, question 5).

## What this establishes

- MEASURED: drawing a spread of the slice takes 1 to 3 ms at the default
  window at scale 2 and under 7 ms at the largest, at p95, on five machines;
  layout of the book 1 to 6 ms. Composition is about a tenth of the page-turn
  budget, so no composed page needs to be cached: the page is made when it is
  asked for, and only the visible spread is held (W304 A).
- MEASURED: the whole turn in the window is inside 50 ms on every platform:
  9.3 ms at worst on Shadow, 28.9 ms at worst on the Linux arm64 runner,
  where the host's paint alone is 25.5 ms.
- MEASURED: with a real page the window reaches its first frame sooner than
  Stage 2's empty window did on Windows (65 against 211 ms on Shadow; 30
  against 104 ms on CI), because the page no longer goes through
  `draw_image`.
- MEASURED: at rest the book adds nothing: zero paints, frames and timer
  ticks on Windows and macOS. Linux still repaints once per wake (M6).
- MEASURED: the same input gives the same pixels on all four platforms: the
  eight reference renderings compare byte for byte in CI.

## What this does not establish

- Anything on hardware, or at display scale 2 on macOS and Linux.
- The cost of a dense page: the slice's pages are a chapter opening, a psalm
  and a short paragraph of narrative. A full page of small type has perhaps
  twice the glyphs.
- The cost of the first open of a long chapter or of a whole book (the
  hypothesis above is a proportion, not a measurement).
- The cause of the one 50.9 ms draw on the macOS runner.
- Memory as the operating system counts it per cached page: the layout and
  cache figures are counted from the vectors' capacities by the tool.

## Next smallest falsifiable step

With Stage 3's container: lay out Psalm 119 and the whole of Genesis in every
setting, and measure first open against the hypothesis above.
