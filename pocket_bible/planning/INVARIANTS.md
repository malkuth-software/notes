# Invariants

> **Amended 2026-10-04 after round 003 and the owner's direction of that day.**
> I37 is replaced: the BSB is shown exactly as published and the name rule
> applies only in A Subjective Interpretation (B303 C). I12–I14 and I41–I42 are
> read with `RECONCILIATION_003.md` §4: anything may be read to inform our own
> text; ancient texts and facts are free whatever label a digitizer attached;
> public domain in the United States is enough; the verbatim wording of works
> still under someone's copyright is not placed in the repository or installer,
> and is re-expressed by us instead. Portsmouth ships as the owner's own,
> unlicensed, and the compositor supplies what it lacks (F302–F304). Search is
> two engines (`RECONCILIATION_003.md` §3); I25–I28 describe the knowledge
> engine's ground layer and will be restated with its interview. A page turn is
> within 50 ms (W303 C). New: every open question of every session reaches the
> owner in one report (W307 note).

Rules that hold from 2026-10-03, each with its source. An invariant is changed
only by a later owner direction, recorded and dated. Items still OPEN in
`RECONCILIATION_001.md` are not here.

## Text

- I1 Scripture, lexicon and commentary text enters only through a hash-pinned
  source and a deterministic importer. No model supplies such text from recall.
  (AGENTS absolute rule; G10.)
- I2 A source file is never edited. Corrections are recorded patches, reported
  upstream. (T023 B.)
- I3 Every rendered text round-trips byte-exactly to its normalized source in
  CI; markup deliberately not carried is listed. (K012 C, K013 B.)
- I4 English numbering is the master reference system; other witnesses map onto
  it by table. (T017 A.)
- I5 Where the page shows something other than a source's published wording
  (the name rule, toggles), the published wording is one gesture away and the
  title page says the text has been adapted. (A009 fidelity first; T018; pending
  E005 for the exact form.)

## Authored text

- I6 Expansions, merged lexicon entries, key terms, notes, translations and
  rite explanations are source files in this repository, compiled into the
  container; nothing is generated on the reader's machine. (X011 A; G19; A007 E.)
- I7 Every expansion cites a stored, cleared source, or is labelled as going
  beyond them; the build rejects one that does neither. (X004 A, X005 A.)
- I8 One model authors, an independent model audits; disagreement goes to the
  owner. (X006 B, T022 B.)
- I9 Where meaning is disputed, readings are listed with their support and
  nothing is adjudicated. (X009 A.)
- I10 Expansions anchor to original-language words and are shown in a slip on a
  click or by keyboard, never inline as translation. (X012 A, X015, X019 B.)
- I11 An expansion's phrase is about 25 words at most; its evidence about 40.
  (X021 A.)

## Licences and privacy

- I12 Only material cleared in `SOURCE_REGISTER.md` is committed or shipped.
  The repository stays private until the audit passes. (A002 B, A003 B.)
- I13 Restricted texts never enter the repository, tracked or ignored. No tool
  here removes copy protection. (AGENTS; E002 A, V003 A; see I41.)
- I14 Fonts are OFL or public domain. Portsmouth is the owner's, shipped under
  the OFL with the source-font notices Plan Paint carries. (F013 A, J006 A.)
- I15 The client links no network code; no telemetry, accounts, update checks.
  (A008 A, A007.)

## Page

- I16 The application composes its own pages to pixels and publishes only
  complete pages. (F010 A; toolkit complete-frame law.)
- I17 Greyscale antialiasing, light hinting, linear-light blending. (F007 A,
  F008 B, F009 B.)
- I18 Nothing moves or repaints while the book lies open and untouched: zero
  timers, zero paints, 0% CPU, measured on every platform. Page turns are
  instant. (I004 A, R018 A.)
- I19 Every action is reachable from the keyboard; page text is exposed to
  screen readers; reduced motion is respected; a high-contrast mode drops the
  paper. (I011 A–D, N020.)
- I20 Every view is usable at 600 × 420. (I014 A.)
- I21 Secondary tools are slips and popups inside the one book window; no side
  panels; one window. (Toolkit law; N019 A; X016 A.)

## Container and search

- I22 One read-only, memory-mapped, section-checksummed container file beside
  the executable; identical bytes from identical sources on every platform; a
  SHA-256 manifest published with each release. (I002 A, K010 A, K019 A, K024 B.)
- I23 Structure and layout are separate streams; the text stream carries no
  verse markers, spaces or case that a rule can restore. (K006 C, K007 A, K008 A.)
- I24 A chapter is the smallest unit opened alone. (K009 A.)
- I25 Search never decompresses text to search it; the index ships prebuilt and
  is read in place. (K018 A, S011 A, S012 A.)
- I26 The exhaustive scan is the oracle; every accelerator returns exactly its
  results. Approximate matches are verified, shown in their own labelled group,
  and every result set states its coverage. (S007 A, S013 A; File Manager law.)
- I27 Exact results are in canonical order. (S008 A.)
- I28 A candidate channel is admitted only by finding judged-relevant verses
  the exact channels miss. (S016 C.)

## Added by round 002 (2026-10-03)

- I37 The divine names follow `NAME_RULE.md` in every English text shown; the
  title page of an adapted text says so; the slip shows the source's wording.
  (E005 A, section D.)
- I38 No Syriac text, face or column ships; Syriac is a build-time source for
  our English. (Y001 A.)
- I39 Words are the logical unit of identity, search, anchoring and shaping;
  subword pieces are a coding layer beneath them. (J001 A.)
- I40 The BSB is the page. Our rendition, A Subjective Interpretation, is a
  second text that begins as a recorded, evidenced revision of the BSB and is
  offered book by book as reviewed. (E001 C, E003 A, E004.)
- I41 Restricted texts are consulted only on the owner's machine, outside the
  repository; our wording is translated from the original with stored evidence
  and measured against them for independence. (E002 A, E007 A, V003 A.)
- I42 Models may read outside sources while arguing; authored text cites only
  stored, cleared sources or is labelled as going beyond them. (V004 A.)
- I43 Old Testament in Hebrew order; deuterocanon, then 1 Enoch and Jubilees,
  between the Writings and the Gospels; Apostolic Fathers after Revelation.
  (O001–O003.)
- I44 In study form the original is on the left page and English on the right.
  (J004 A.)
- I45 Latin faces are EB Garamond and Portsmouth; three type-size steps, each
  with its own page breaks. (J005 A, J003 B.)
- I46 God's speech is set in an antique gold that meets the body-text contrast
  floor; words of Christ in red. (D010 A, D011, R010 B.)
- I47 Silent: no audio subsystem. (U009 B.)
- I48 The public repository will start from a fresh history of a reviewed tree;
  this working history stays private. (V013 A.)
- I49 The dedication opens the About section, before anything else; credits are
  “Authors: Opus, Fable, Astra, Rainstar”, with no sponsor line. (V007, V008.)
- I50 Review state of authored text is recorded in its source files, not shown
  in the slip. (V006 B.)

## Ceilings (ceilings, not targets: J008 A; Stage 2 measures the floors)

- I29 Installed size under 100 MB. (K022.)
- I30 Resident memory with a spread open under 150 MiB. (I005 C.)
- I31 Launch to a readable page under 400 ms. (I006 B.)

## Conduct

- I32 No subagents or workers. (W001 A.)
- I33 Sibling repositories are read-only; toolkit gaps are recorded requests and
  are built here meanwhile. (W004 A.)
- I34 Without asking, never: add a dependency; change a decided record; alter
  scripture source bytes; publish a release or change visibility; delete files
  not created by the project; touch another repository. (W009.)
- I35 Corpus tools, encoder and decoder are C++ in the house style. (K020 A.)
- I36 Every stage ends in something runnable or a measured report. (W010 A.)
