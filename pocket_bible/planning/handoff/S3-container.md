# Stage 3 handoff — importers and the container study

For a session opened on the Pocket Bible repository. Paste the block as the
first message. It runs beside the Stage 4 session; the two own different
directories.

```
You are conducting Stage S3 (importers and the container study) of the Pocket
Bible project. Work on branch work/s3-container, created from main. If you were
started in a worktree, treat it as the repository root.

Read first, in order: AGENTS.md; planning/README.md; planning/INVARIANTS.md
(its dated note at the top prevails over the rules beneath it);
planning/OWNER_DIRECTION_2026-10-04.md; planning/RECONCILIATION_003.md, then
RECONCILIATION_002.md and RECONCILIATION_001.md; planning/TEXT_CONTAINER_STUDY.md;
planning/ARCHITECTURE_HYPOTHESES.md sections 1 and 2; planning/DECISION_PROTOCOL.md;
sources/README.md, sources/IMPORTERS.md, sources/MEASUREMENTS.md;
planning/reports/S1-2026-10-03.md and S2-2026-10-03.md; docs/BUILDING.md;
planning/BUDGETS.md; planning/PROGRAMMING_HOUSE_STYLE.md. The owner's answers
are the three ARCHITECTURE_INPUTS files; consult them by question ID (section K
of round 001 and J001 of round 002 govern this stage).

Then state, before editing: the stage's exit condition in your own words; the
first smallest falsifiable step.

You own: src/corpus/ (importers, encoder, the study tools), additions to
src/core/ for the container reader, tests named corpus_* and container_*,
cmake/Corpus.cmake, sources/, tools/, experiments/, planning/reports/S3-*.md.
In the root CMakeLists.txt you add exactly one include line and change nothing
else. You do not touch src/page/, src/forms/, cmake/Page.cmake, assets/ or
.github/ — the Stage 4 session owns those. Shared planning documents are changed
only by a dated note at their top.

The pinned source files already exist on this machine at
C:/Users/Shadow/bible/.claude/worktrees/lucid-bartik-8ac458/.build/sources.
Several upstreams replace files in place, so do not download them again: copy
that directory into your own .build/sources and run
tools/fetch_sources.py verify against it. First task, already approved by the
owner (round 003 B310): archive the pinned bytes as one file and attach it to a
private release of this repository named sources-2026-10-03, with its SHA-256
in sources/README.md.

Work, in order:
1. The importer for the BSB's USFM, written in C++ against bible_core, into the
   normalized form of sources/IMPORTERS.md. Falsifiable step: Mark round-trips
   byte for byte with only the markers that document lists. Then every book.
2. Importers for STEPBible's tagged Greek and Hebrew, the Berean alignment
   tables, the versification table, the cleared lexica, the World English
   Bible, Charles and Lightfoot, each with its round-trip or checksum test and
   its list of markup not carried. Carry the Berean alignment across to
   STEPBible's Greek word by word (round 003 B306 A: mark the 1,553 words that
   differ in substance).
3. Audit and pin the clean-provenance fallback for the displayed originals
   (SBL's Greek or Nestle 1904; Open Scriptures morphology), as B304 C asks.
4. The study of the text itself, then the candidate table, exactly as
   planning/TEXT_CONTAINER_STUDY.md sets out, with these GIVENs: size first,
   then memory, speed, verifiability (K001); words are the logical unit and
   subword pieces a coding layer beneath them (K002 C with J001 A); spaces and
   capitals restored by rule with flagged exceptions (K006 C); structure and
   layout in separate streams (K007 A, K008 A); a chapter is the smallest unit
   opened alone (K009 A); one memory-mapped read-only file with a checksummed
   section table (K010 A); Hebrew in three sparse layers (K011 A); one
   dictionary for all English (K014 A); controls are raw UTF-8, zstd and xz,
   on whole works and on chapter-sized blocks (K016). Every candidate's row is
   kept, losers included, and no number omits its dictionary.
5. The encoder and the decoder for the candidate you would choose, the
   round-trip test over every work in CI, and the reproducibility check
   (identical container bytes on every platform).
6. A proposed decision record for the format, for the owner. Do not mark it
   accepted.

Hold to these: C++20 in the house style for everything that reads or writes
corpus text; no scripture, lexicon or commentary text from your own recall,
including in tests — fixtures are extracted by a tool from the pinned files and
carry the source's hash; a source file is never edited (recorded patches only);
no subagents; two compile jobs at most; builds under .build/; the sibling
repositories are read-only; no new third-party dependency in the product
without asking (zstd and xz are measuring instruments run as programs, not
linked); every size or speed statement carries its measurement, environment and
what it does not establish. Commit freely on your branch; push it at the end of
the stage; do not merge to main.

End with planning/reports/S3-<date>.md in the form planning/HANDOFF_PROMPT.md
describes. Put every question for the owner in one section at its end, each
with the choice you recommend and why in two lines: the coordinator gathers
them into a single interview, and the owner will not look anywhere else.
```
