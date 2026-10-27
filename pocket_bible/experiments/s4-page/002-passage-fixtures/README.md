# 002 — passage fixtures

Not an experiment: the tool that makes the proving slice's text, kept beside
the experiments because Stage 4 owns no other place for tools.

**Rule.** No scripture, in any language, is typed, corrected or completed by
hand or by a model anywhere in this repository (AGENTS.md, absolute rules).
The three passages of the proving slice are whole lines cut from the pinned
source files by `extract_passage.cpp`, written to `assets/fixtures/` under a
header that carries the SHA-256 of the file read, of the archive it was
unpacked from, and of the body.

**Procedure** (on the owner's machine, where the pinned sources are):

```
cmake --build .build/app --parallel 2 --target page_extract_passage
python3 experiments/002-passage-fixtures/make_fixtures.py \
    --tool .build/app/page_extract_passage.exe \
    --sources <.build/sources of the Stage 1 checkout> --output assets/fixtures
```

`berean/extracted/bsb_usfm/` is `bsb_usfm.zip` unpacked with `unzip`.

**The fixtures.**

| File | Passage | Source file (pinned in `sources/lock.json`) | Source lines | Rows |
| --- | --- | --- | --- | --- |
| `gen-1-1-13.bsb.fixture` | Genesis 1:1–13, BSB | `GEN.usfm` in `bsb_usfm.zip` | 10–24 | 15 |
| `gen-1-1-13.tahot.fixture` | Genesis 1:1–13, Hebrew | `TAHOT_Gen-Deu.txt` | 83–361 | 159 |
| `psa-87.bsb.fixture` | Psalm 87, BSB | `PSA.usfm` in `bsb_usfm.zip` | 4199–4223 | 25 |
| `psa-87.tahot.fixture` | Psalm 87, Hebrew | `TAHOT_Job-Sng.txt` | 38792–38898 | 54 |
| `mrk-7-14-23.bsb.fixture` | Mark 7:14–23, BSB | `MRK.usfm` in `bsb_usfm.zip` | 232–237 | 6 |
| `mrk-7-14-23.tagnt.fixture` | Mark 7:14–23, Greek | `TAGNT_Mat-Jhn.txt` | 39365–39608 | 149 |

**What is checked, and where.**

- MEASURED by the tool at extraction: the SHA-256 of each file read. The
  three STEPBible files and `bsb_usfm.zip` hash to the values in
  `sources/lock.json`; `tests/page_fixture_tests.cpp` holds those values and
  fails on a fixture cut from other bytes.
- MEASURED at every load: the body's SHA-256 against the header's
  (`read_fixture`, `src/page/passage_reader.cpp`). A fixture with one changed
  bit is refused; tested.
- Not established by the tool: that the USFM file is the member of the
  archive. The tool hashes the archive and the unpacked file; it does not
  unpack. `unzip -p bsb_usfm.zip bsb_usfm/MRK.usfm | sha256sum` ties the two
  by hand.

**What the fixtures hold beyond the passage.** Whole lines are kept, so each
BSB fixture carries its section headings, its parallel-passage line (`\r`,
carried and not printed: cross-references belong to a slip, R014 C) and its
footnotes. Mark 7:16 is not in the BSB's running text; the edition carries it
in the note on verse 15, and so does the page. The STEPBible rows keep every
column of the source: transliteration, gloss, grammar, variants. Of the 149
Greek rows, ten are not on the Nestle-Aland side and are not displayed
(B305 A). The psalm's title is verse 0 in STEPBible's numbering
(`Psa.87.0(87.1)`) and part of verse 1 in the BSB.

**Limits.** The USFM mode refuses a range whose last line runs on into the
next verse, rather than cutting inside a line: a passage must end with its
paragraph. The reader in `src/page/` carries the markers these three passages
use and lists any other it meets (`unhandled_markers`, empty for all three,
tested). It is a reader for the slice, not Stage 3's importer.
