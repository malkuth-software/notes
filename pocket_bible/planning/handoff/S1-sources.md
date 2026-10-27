# Stage 1 handoff — sources and licences

For an Opus 5.5 session opened in `C:/Users/Shadow/bible`. Paste the block as
the first message. This session runs beside the Stage 2 session; the two own
different directories.

```
You are conducting Stage S1 (sources and licences) of the Pocket Bible project
in C:/Users/Shadow/bible, on branch work/s1-sources (create it from main).

Read first, in order: AGENTS.md; planning/README.md;
planning/OWNER_BRIEF_2026-10-03.md; planning/CHARTER.md;
planning/INVARIANTS.md; planning/RECONCILIATION_001.md;
planning/RECONCILIATION_002.md; planning/NAME_RULE.md;
planning/SOURCE_REGISTER.md; planning/DECISION_PROTOCOL.md;
planning/STAGE_PLAN.md (S1 and the closing rules);
planning/PROGRAMMING_HOUSE_STYLE.md. ARCHITECTURE_INPUTS.md and
ARCHITECTURE_INPUTS_002.md are the owner's answers; consult them by question ID.

Then state, before editing: S1's exit condition in your own words; which rows
of the register you will take first and why; the first smallest falsifiable
step.

You own: sources/ (lock file, saved licence statements, per-source README),
tools/fetch_sources.py, planning/SOURCE_REGISTER.md, THIRD_PARTY_NOTICES.md,
planning/ATTRIBUTION.md, planning/proposals/ (lists for the owner to strike),
planning/reports/S1-*.md. Downloads go to .build/sources/ (ignored), never into
the tracked tree. You do not touch CMakeLists.txt, cmake/, src/, tests/,
scripts/, packaging/, .github/ — the Stage 2 session owns those. Shared
planning documents other than the ones named are changed only by a dated note
at their top, and only where S1's findings require it.

Work, in this order:
1. First wave, no open question touches these: the BSB (text, headings,
   footnotes, cross-references) and the Berean interlinear tables; STEPBible
   TAGNT, TAHOT, TBESG, TBESH, TVTMS; the lexica of round 001 L001 (Strong's,
   Abbott-Smith, LSJ or Intermediate Liddell, BDB, Payne Smith, MACULA Greek
   and Hebrew, Trench, Moulton–Milligan); World English Bible with its
   words-of-Jesus markup; Charles's 1 Enoch and Jubilees; Lightfoot's Apostolic
   Fathers (1 Clement, Didache, Ignatius, Polycarp only); the Septuagint
   candidates; the fonts (EB Garamond; Portsmouth from the Plan Paint packaging
   with its notices; Gentium for Greek; Ezra SIL and the other Hebrew
   candidates).
   For each: find the canonical upstream; save its licence statement verbatim
   under sources/licences/; record URL, retrieval date, size and SHA-256 in
   sources/lock.json; write the fetch entry; record format, encoding and counts.
   A row is cleared only with all of that. A row whose licence cannot be
   established is REJECTED with the reason kept.
2. Completeness: book, chapter and verse counts for every scripture text
   against TVTMS; every verse present or its absence explained by the edition.
3. Fonts: for each candidate, verify coverage of every code point and every
   base-plus-marks sequence that actually occurs in the admitted texts
   (cantillated Hebrew, polytonic Greek). A face that fails is REJECTED here.
4. The alignment question (J007 A): measure how many Greek words differ between
   STEPBible's text and the Berean base, book by book, and report.
5. Peshitta: establish the edition and terms behind ETCBC/peshitta and any
   public-domain printed edition obtainable as text. Syriac is a build-time
   source only (Y001 A). Read Psalm 87 in it and report, in English, what verse
   7 says — the owner's proving psalm (Y005) rests on a guess until you do.
   Locate Oliver 1861 and both Latin Psalters with Douay–Rheims.
6. Proposals for the owner to strike and add (planning/proposals/): about fifty
   hymns; creeds, classic prayers, homilies and pastoral rules; later Christian
   philosophical writers on hard spiritual subjects (not Luther); the rites'
   source editions for rabbinic Judaism, Anglican, older Roman, Messianic
   Jewish practice; the curated commentary on the Gemara and the Vilna Gaon set;
   reading cycles. Each entry: what it is, why it earns a place, its source and
   licence. These are proposals; fetch nothing from them yet.
7. Write the importer specification for each first-wave format (what is
   carried, what markup is dropped and why, the normalized form). The importers
   themselves are C++ and wait for the Stage 2 skeleton.
8. The attribution page text and THIRD_PARTY_NOTICES.md.

Hold to these: fetch only rows whose licence is plainly open (W006 A); anything
uncertain may be read for research outside the tracked tree but is not
committed (V002 A). Restricted texts — Hart, the Tree of Life Version — are
never fetched, stored or quoted, and nothing here removes copy protection. No
scripture or lexicon text from your own recall anywhere, including in examples
and tests. No subagents (W001 A). The fetch script is Python in the house
style's language-neutral rules, as the sibling projects' fetch and packaging
scripts are; everything that reads or writes corpus text is C++ (K020 A). Use
C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/python.exe, read-only.
Commit freely on your branch; push it at the end of the stage; do not merge to
main.

End with planning/reports/S1-<date>.md as planning/HANDOFF_PROMPT.md describes,
and the cleared list for the owner's G1 sign-off, with a short round 003 of
whatever only the owner can settle (begin from RECONCILIATION_002.md §3).
```
