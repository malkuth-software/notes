# Experiment 001 — the fallback originals against the displayed ones

Stage 3, item 3 (round 003 B304 C: “audit and pin the clean-provenance
fallback now, so a switch is a rebuild”). Made 2026-10-04.

**Question.** If STEPBible's licence statement were withdrawn or contested, how
far would the page's Greek and Hebrew move if they were rebuilt from texts
whose provenance is plain: Nestle's 1904 Greek, the SBL Greek New Testament,
and the Open Scriptures Hebrew Bible?

**Hypothesis and what would falsify it.** HYPOTHESIS: each fallback agrees with
the displayed text in at least 98% of its words, in order, when words are
compared by their letters. Falsified by a paired share below 98% for any of
the three.

**Controls.** The same program, with the same folding and pairing, reproduces
Stage 1's independent count for the Berean base against TAGNT (137,112 paired;
1,019 and 534 unpaired; `sources/normalized/greek-alignment/MANIFEST.txt`).

**Data.**

| Text | Source id in `sources/lock.json` | Upstream, commit | Terms as read in the pinned statements |
| --- | --- | --- | --- |
| Nestle 1904 with morphology, lemmas, Strong's numbers | `nestle1904`: `Nestle1904.csv` (9,098,695 bytes) | `biblicalhumanities/Nestle1904` at `713f28a3` | OBSERVED, `licences/nestle1904-morph-README.md`: CC0 — “biblicalhumanities.org has waived all copyright and related or neighboring rights to the biblicalhumanities.org Nestle 1904 Morphology.” The 1904 text is in the public domain by date. The repository's XML markup is CC BY-SA 4.0 and its glosses differ again; neither is pinned |
| SBL Greek New Testament v1.2 | `sblgnt`: 27 text files | `LogosBible/SBLGNT` at `c4d241a9` | OBSERVED, `licences/sblgnt-README.md`: CC BY 4.0, “Copyright 2010 by the Society of Biblical Literature and Logos Bible Software” |
| Open Scriptures Hebrew Bible | `oshb-morphhb`: 39 books and `VerseMap.xml` | `openscriptures/morphhb` at `3d15126f` | OBSERVED, `licences/oshb-morphhb-LICENSE.md` and `-README.md`: lemma and morphology CC BY 4.0, credit “Original work of the Open Scriptures Hebrew Bible”; “The text of the WLC remains in the Public Domain” |
| Displayed Greek | committed work `tagnt`, words whose type contains N or n | | |
| Displayed Hebrew | committed work `tahot`, every word | | |

All three fallback sources are pinned by Git commit, so they do not have the
mutable-upstream weakness; `tools/fetch_sources.py verify` reports 217 verified.

**Environment.** Windows 11 Home 10.0.22621, x86-64; g++ 16.2.0 (MSYS2
MinGW-w64, borrowed read-only), Release (`-O3`).

**Procedure.**

```
cmake -S . -B .build/core -G Ninja -DCMAKE_BUILD_TYPE=Release -DBIBLE_BUILD_APPLICATION=OFF
cmake --build .build/core --parallel 2
.build/core/bible_build compare-fallback > experiments/001-fallback-originals/results.tsv
```

Run time 2.8 s. `results.tsv` (98 lines, SHA-256 `faae51a8…4fc7a8`) has one
line per comparison and book. Method: `src/corpus/study_fallback.cpp` — per
book, both texts as sequences of words folded to their letters (no accents,
breathings, points, cantillation, capitals or punctuation), paired in order by
a shortest edit script.

**Results (MEASURED).**

| Comparison | Fallback words | Displayed words | Paired | Fallback only | Displayed only | Paired share of displayed |
| --- | --- | --- | --- | --- | --- | --- |
| Nestle 1904 against TAGNT (N side) | 137,779 | 137,646 | 135,891 | 1,888 | 1,755 | 98.72% |
| SBLGNT against TAGNT (N side) | 137,741 | 137,646 | 135,512 | 2,229 | 2,134 | 98.45% |
| Open Scriptures Hebrew Bible against TAHOT | 305,507 | 305,638 | 301,018 | 4,489 | 4,620 | 98.49% |

For scale: the Berean base, which the English was translated from, stands
closer to the displayed Greek than either fallback does (137,112 paired of
137,646, 99.61%). The Open Scriptures file also holds 1,278 words inside notes
(qere readings and editors' notes), which were not compared.

**What this establishes.**
- The hypothesis is not falsified: each fallback pairs with more than 98% of
  the displayed words.
- Nestle 1904 is the nearer of the two Greek fallbacks, and the only one that
  arrives with morphology, lemmas and Strong's numbers in one CC0 file.
  CANDIDATE recommendation: Nestle 1904 for the Greek fallback, the Open
  Scriptures Hebrew Bible for the Hebrew.
- A switch would move about 1,800 Greek words and about 4,500 Hebrew words on
  each side, and every one of them would need its alignment and its
  expansions checked.

**What this does not establish.**
- **That a switch is a rebuild.** The fallbacks are pinned and measured; their
  importers are not written. Nestle 1904's Robinson-style morphology codes and
  plain Strong's numbers, and the Open Scriptures lemma and morphology codes,
  differ from STEPBible's disambiguated numbers and codes; mapping them is
  work, and nothing here measures it.
- What the roughly 4,500 Hebrew differences are. Both texts descend from the
  Westminster Leningrad Codex; HYPOTHESIS, not measured: most are ketiv and
  qere (this comparison takes the Open Scriptures running text, which prints
  the ketiv, against TAHOT, which prints the qere) and differences in where a
  word is divided.
- Differences of accent, pointing or cantillation, which the folding removes.
- Whether any difference changes the English.
- Anything about the legal position; only the wording of each statement was
  read.

**Next smallest falsifiable step.** Classify the Hebrew differences:
HYPOTHESIS: once Open Scriptures' qere readings replace their ketiv words, and
words are compared after removing the maqqef and STEPBible's separators, fewer
than 500 differences remain. Falsified by a count above 500.
