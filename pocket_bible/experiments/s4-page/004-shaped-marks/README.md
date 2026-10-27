# Experiment 004 — the shaped test of marks

Stage 4, part two, item 5. Status: **MEASURED** on the owner's machine.

**Question.** Stage 1 tested the faces statically, from the font file alone,
and said a pass was "necessary, not sufficient" (`sources/MEASUREMENTS.md`
section 3). When every base-plus-marks sequence of the admitted Hebrew and
Greek is actually shaped, does any face leave a glyph missing, a mark
unplaced, or two marks on top of each other? And does Noto Serif Hebrew,
rejected on three sequences, fare better or worse when shaped (round 003
F305 B)?

**Hypothesis, and what would falsify it.** Ezra SIL, which passed statically
on all 6,536 sequences, shapes them without collision; Noto Serif Hebrew's
three failing sequences are placed acceptably by the shaper. Falsified by any
ink collision in Ezra SIL, or by collisions in Noto beyond the three.
**Both halves were falsified**, in different degrees.

**Data.** The four `TAHOT_*.txt` files and the two `TAGNT_*.txt` files pinned
in `sources/lock.json`. The tool reads the same word rows Stage 1's
`text_inventory` reads and counts the same sequences: **6,536 distinct,
876,361 occurrences, longest 6 code points** for Hebrew — Stage 1's figures
exactly — and 1 distinct sequence (U+1FC6 U+0345, 1,488 occurrences) for
Greek, with 25,079 distinct Greek word forms.

**Environment.** Shadow (QEMU guest, Windows 11 22621), MinGW GCC 16.2.0,
Release; HarfBuzz `56feae4`, FreeType `f4205da` (the toolkit's pins).

**Procedure.**

```
cmake --build .build/app --parallel 2 --target page_shaped_marks
S=<.build/sources of the Stage 1 checkout>
.build/app/page_shaped_marks hebrew \
  --face EzraSIL=$S/font-ezra/extracted/EzraSIL2.51/SILEOT.ttf \
  --face EzraSIL-SR=$S/font-ezra/extracted/EzraSIL2.51/SILEOTSR.ttf \
  --face NotoSerifHebrew=$S/font-notoserifhebrew/extracted/NotoSerifHebrew/full/ttf/NotoSerifHebrew-Regular.ttf \
  --text $S/stepbible/TAHOT_Gen-Deu.txt --text $S/stepbible/TAHOT_Jos-Est.txt \
  --text $S/stepbible/TAHOT_Job-Sng.txt --text $S/stepbible/TAHOT_Isa-Mal.txt \
  --details results/hebrew-details.tsv
.build/app/page_shaped_marks greek \
  --face EBGaramond=assets/fonts/EBGaramond-Regular.ttf \
  --face GentiumBook=assets/fonts/GentiumBook-Regular.ttf \
  --text $S/stepbible/TAGNT_Mat-Jhn.txt --text $S/stepbible/TAGNT_Act-Rev.txt
.build/app/page_shaped_marks glyph --face Portsmouth=assets/fonts/Portsmouth.ttf \
  --code 2014 --code 2013 --code 2D --code 2020
```

The whole Hebrew run takes under a second. Each sequence is shaped right to
left as Hebrew script with one cluster to the character, at an em of 1000
units. Four tests:

- **missing**: a glyph the face lacks.
- **unplaced**: a mark glyph whose offsets are the same with the `mark` and
  `mkmk` features on and off, that is, one the face's mark anchors did not
  move. A mark the face composes with its base into one glyph is not counted.
  This is a statement about the font's tables, not about the result on the
  page: a face may place a mark by another feature.
- **collision**: two marks of one sequence whose *ink* overlaps. Both glyphs
  are rasterized at 128 pixels to the em where the shaper puts them; three or
  more pixels more than half covered by both is a collision.
- **on base**: a mark whose ink overlaps its base's ink, in the same sense.
- **detached**: a mark whose ink box lies wholly more than a tenth of an em to
  one side of the width its base stands on.

## Results

### Hebrew: distinct sequences (occurrences), of 6,536 (876,361)

| Face | Clean on all tests | Missing | Unplaced | Collision | On base | Detached |
| --- | --- | --- | --- | --- | --- | --- |
| Ezra SIL 2.51 | 6,335 | 0 | 141 (3,918) | 37 (695) | 16 (592) | 12 (167) |
| Ezra SIL SR 2.51 | 6,299 | 0 | 159 (3,965) | 47 (713) | 26 (872) | 12 (167) |
| Noto Serif Hebrew 2.004 | 5,783 | 0 | 7 (120) | **702 (41,897)** | 52 (1,029) | 0 |

Which marks, by distinct sequences (occurrences); the full list is
`results/hebrew-details.tsv`, as code point numbers:

- **Ezra SIL, collision**: 35 of the 37 involve holam, U+05B9 (693
  occurrences), against an accent above the same letter: zaqef qatan U+0594
  (10), qadma U+05A8 (8), geresh U+059C (6), zaqef gadol U+0595 (4), pazer
  U+05A1 (4).
- **Ezra SIL, on base**: munah U+05A3 in 12 sequences (585 occurrences);
  U+05C4 and U+05AA in two each.
- **Ezra SIL, detached**: dehi U+05AD in 7 (24); etnahta U+0591 in 3 (135);
  merkha U+05A5 and munah U+05A3 in one each. Dehi is a prepositive accent
  and stands before its letter by rule, so its seven are expected; the
  others are not explained.
- **Ezra SIL, unplaced**: almost all accents below the letter — dehi (40),
  munah (18), etnahta (15), mahapakh U+05A4 (12), tevir U+059B (11), yetiv
  U+059A (9), darga U+05A7 (8) — with meteg U+05BD (7) and hiriq U+05B4 (7).
  Only the ones also listed under collision, on base or detached show a fault
  in the ink.
- **Noto Serif Hebrew, collision**: the marks most often in a colliding pair
  are meteg U+05BD (375 sequences; 30,014 occurrences) and the vowels that
  stand under the same letter — qamats U+05B8 (88; 14,494), hiriq U+05B4
  (89; 9,164), segol U+05B6 (68; 5,819), patah U+05B7 (89; 5,617), tsere
  U+05B5 (27; 2,461) — and, above the letter, holam U+05B9 (155; 2,984).
- **Noto Serif Hebrew, unplaced**: qubuts U+05BB (5; 80) and qamats U+05B8
  (2; 40). The second is Stage 1's "final nun + qamats (40)".

### Greek

| Face | The one sequence | Words with a missing glyph, of 25,079 |
| --- | --- | --- |
| EB Garamond Regular | clean | 0 |
| Gentium Book Regular | clean | 0 |

### Portsmouth's dashes and dagger (thousandths of the em)

| Code point | Glyph | Advance | Ink, y from the baseline |
| --- | --- | --- | --- |
| U+002D hyphen | 14 | 528 | 298 to 361 |
| U+2013 en dash | none | — | — |
| U+2014 em dash | 326 | 728 | **192 to 675** |
| U+2020 dagger | none | — | — |

## What this establishes

- MEASURED: no face tested lacks a glyph for any Hebrew or Greek sequence or
  Greek word form in the admitted texts.
- MEASURED: Ezra SIL sets 6,335 of 6,536 sequences with no fault on any test.
  Its real faults in the ink are 37 sequences with colliding marks (695 of
  876,361 occurrences, 0.08%), nearly all a holam meeting an accent above, and
  16 with a mark on its base (592, 0.07%), nearly all munah. Ezra SIL is
  ahead of Ezra SIL SR, or level with it, on every count. Ezra SIL stays the
  Hebrew face.
- MEASURED: Noto Serif Hebrew, shaped, has colliding marks in 702 sequences
  and 41,897 occurrences (4.8% of all sequences in running text), most of
  them a meteg printed over the vowel beside it. The static test's three
  sequences understated it. **REJECTED stands** (F305 B answered).
  What would reopen it: a release whose meteg is placed beside the vowel.
- MEASURED: EB Garamond's and Gentium Book's Greek set every word form of the
  Greek New Testament without a missing glyph or a misplaced mark.
- MEASURED: Portsmouth has no en dash and no dagger, and its glyph at U+2014
  is not a dash: it is 483 units high where its hyphen is 63. The compositor
  draws all three itself (`src/page/text_layout.cpp`, `page_raster.cpp`).

## What this does not establish

- That a collision of three pixels at 128 to the em is visible at reading
  size, or that an untouched sequence looks right: the tests find ink on ink
  and marks far from their base, not marks that are merely badly placed. The
  eye is the test for that, at gate G2.
- Anything about sequences in context: each is shaped alone, so a mark that
  collides with the next letter's mark is not seen.
- The cause of Ezra SIL's unexplained cases (etnahta detached in three
  sequences, munah on its base in twelve).
- Hinted rasterization at small sizes; ink is compared at 128 pixels to the
  em with light hinting.

## Next smallest falsifiable step

Set the 37 colliding and 16 on-base Ezra SIL sequences at the page's size and
look: if they read cleanly, the face needs nothing; if not, the compositor
can nudge the second mark, as it already supplies what Portsmouth lacks.
