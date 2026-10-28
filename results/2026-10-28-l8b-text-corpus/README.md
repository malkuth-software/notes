# Stage L, L8b: the text corpus (2026-10-28)

Phase L8b of `planning/stages/L-layout/PLAN.md` (section 12.7): the text
fixtures, laid out by the bundled text engine and checked against Chromium
at text scales 100 % and 150 %. This exercises the text-scale dimension
that the L7 gate (`results/2026-10-26-l-layout-gate`) recorded as not yet
exercised. `report.json` is the whole corpus's per-node report.

## What L8b adds

- **The engine in the harness.** `gui_forms_layout_corpus_tests` links
  `gui_forms_text_engine`. With `--fonts assets/fonts` it registers the
  bundled faces: Carlito regular, bold and italic, and Portsmouth Rapids.
  Each fixture's window then measures text through `HarfBuzzFontEngine`
  and breaks lines with libunibreak (UAX #14). In a build without the
  engine, the text fixtures are reported as not run and the test fails.
- **Text leaves.** `data-gf-kind="text-block"` with a `data-gf-text`
  attribute, laid out as a `TextBlock`. The harness reads:
  - `font-size` in rem (13 px times the text scale);
  - `font-family` (Carlito or Portsmouth Rapids);
  - `font-weight` (400 or 700) and `font-style`;
  - `line-height` (`normal`, px, or a unitless number);
  - `text-align`, inherited as CSS inherits it.
- **Text-scale profiles.** `corpus.xml` gains 16 profiles with
  `text-scale="1.5"`: the four viewports at both device scales, in both
  directions. They apply only to text fixtures, and their report key ends
  in `/text150`.
  - Chromium side: the root font size is 13 px × 1.5.
  - GUI.Forms side: the window's text scale is 1.5.
- **First baselines.** `capture.py` puts a zero-size inline-block probe
  first in each text leaf and reads the probe's top. It refuses a capture
  in which any box moved with the probe in place. Box files carry
  `baseline` on text-leaf nodes, and both comparisons (C++ and Python)
  check it.
- **30 fixtures** in `tests/layout_corpus/fixtures/text/`:
  - block: content strut, empty and space-only text, a text block among
    inline boxes, line heights, collapsing margins, paragraphs, a large
    font's strut, and `text-align`;
  - flex: the automatic minimum, baseline alignment with padding and
    across sizes, centring, a column, growing, shrinking and a wrapping
    row;
  - grid: auto, `min-content`, `fit-content()` and `fr` tracks sized by
    text, auto rows, a spanning item, baseline alignment in a row, and a
    form of labels;
  - nested: a card, a flex row in a grid cell, and the atlas's form row;
  - faces and breaks: bold and italic, Portsmouth Rapids, and hyphen and
    slash breaks.

## Machines and Chromium

- References: Chrome for Testing `chrome-headless-shell` 155.0.8059.39
  (pin and hashes as in the L7 record). The text references are the
  `layout-oracle` CI job's own linux64 capture (run 40754140511, artifact
  `layout-oracle-references`). They were first captured with win64
  Chromium on the Shadow Windows host, but the linux64 build's glyph
  advances differ by up to 1/16 px (`text/bold-italic #large-bold`,
  `text/grid-baseline-row #note` and `#value`). That is over the 1/64 px
  drift bound, though far inside the 0.51 px text tolerance, so the job's
  drift check failed. Box references are identical between the two builds
  and stay as captured in L7.
- This run: the M4 Mac mini (macOS, arm64), LLVM 22, from the
  `claude/l8b-text-corpus` branch merged with main 31d974a0.
- Reference drift: 0 against the adopted linux64 capture. The next CI
  run of the job is the second, independent capture.

## Profiles and tolerances

| Dimension | Values | Fixtures |
| --- | --- | --- |
| Viewport | 320 x 240, 800 x 600, 1280 x 800, 1440 x 900 | all |
| Device scale | 1, 2 | all |
| Direction | ltr, rtl | ltr: all; rtl: the 186 marked bidirectional (21 of the 30 text fixtures) |
| Text scale | 100 %, 150 % | 100 %: all; 150 %: the 30 text fixtures |

Tolerances (decision L-Q9):
- box fixtures: 0.05 logical px per edge;
- text fixtures: 0.51 px per edge and 1.0 px on the first baseline.

## Result

| Group | Fixtures | Bidirectional | Node comparisons | Over tolerance | Recorded | Largest edge difference outside recorded fixtures | Largest baseline difference |
| --- | --- | --- | --- | --- | --- | --- | --- |
| text: block | 8 | 3 | 1,152 | 0 | 0 | 0.044 px | 0.0375 px |
| text: flex | 8 | 6 | 1,088 | 0 | 0 | 0.047 px | 0 |
| text: grid | 8 | 8 | 1,216 | 0 | 0 | 0.078 px | 0 |
| text: nested | 3 | 2 | 496 | 0 | 0 | 0.016 px | 0 |
| text: faces and breaks | 3 | 2 | 416 | 0 | 8 | 0.047 px | 0 |
| **Text total** | **30** | **21** | **4,368** | **0** | **8** | | |
| Box fixtures (L7) | 181 | 165 | 17,440 | 0 | 160 | as in L7 | |
| **Corpus** | **211** | **186** | **21,808** | **0** | **168** | | |

- The corpus runs 3,584 fixture profiles: 2,768 for the box fixtures, as
  in L7, and 816 for the text fixtures.
- The box fixtures' results are unchanged from L7.
- `compare.py report` agrees with the corpus test: 21,808 nodes, 0 over
  tolerance, 168 recorded.

### Recorded difference

| Fixture | Node | Profiles | Difference | Reason |
| --- | --- | --- | --- | --- |
| `text/hyphen-breaks` | `numbers` | the 8 at text scale 100 % | 15 px in height (one line) | UAX #14 rule LB25 forbids a break between a hyphen and a digit, so the engine keeps `10-20` whole; Chromium 155 breaks after the hyphen |

This difference is an open question: `PLAN.md` 12.8, question 4. The
recommendation is to keep UAX #14 and keep the difference recorded. At
the 150 % profiles the same node is within tolerance, so it is recorded
only at 100 %.

### A defect the corpus found

`text/grid-fit-content` found a grid defect, which L8b fixes. The solver
counted a `fit-content()` argument as a fixed maximum, so it clamped an
item's automatic minimum to the argument. A word wider than
`fit-content(40px)` was therefore squeezed to 40 px. CSS Grid 6.6 clamps
the automatic minimum only by fixed maximums. The argument still limits
the min-content and max-content contributions (12.5). The fix is in
`grid_solver.cpp`, with the test
`test_fit_content_keeps_an_automatic_minimum` in
`gui_forms_grid_solver_tests`.

## Not built, and open

- **Baselines of nested containers.** A nested `FlexPanel`, `GridPanel`
  or `BlockPanel` does not answer its first item's baseline. No text
  fixture needed it (12.3), so a nested panel is aligned by its border
  box's bottom. `docs/user/layout.md` lists this under "Not provided".
- **Text alignment is not in the boxes.** `text-align` moves glyphs
  inside a text leaf's box, not the box itself. `text/block-text-align`
  therefore has the same boxes in both directions, and it is not marked
  bidirectional. Its alignment is covered by `gui_forms_text_block_tests`,
  not by the corpus.
- **The hyphen-digit break** (above).

## Commands

```text
python3 -B tools/ci_build.py --jobs 8 --tests 'gui_forms_(layout_corpus|layout_style|grid_solver|grid_panel)_tests'
python3 -B tools/ci_build.py --jobs 8 --sanitize --tests 'gui_forms_(layout_corpus|layout_style|grid_solver|grid_panel)_tests'
gui_forms_layout_corpus_tests tests/layout_corpus --fonts assets/fonts --native-out <scratch>/native
python3 -B tools/layout_oracle/compare.py report --corpus tests/layout_corpus --native <scratch>/native --output report.json
python3 -B -m unittest tools/layout_oracle/test_layout_oracle.py
```

Results:
- all four tests pass plain and under ASan, UBSan and LSan;
- the oracle tool's tests pass (23).
