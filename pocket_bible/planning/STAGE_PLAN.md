# Stage plan

> **Note from the Stage 4 session, 2026-10-03.** S4's work is done and
> reported in `reports/S4-2026-10-03.md`; its branch is `work/s4-page`, not
> merged. Part one found a route inside the owner's bar: a page turn on the
> owner's machine takes 4.9 ms at the median and 9.3 ms at worst, composing
> included, against 50 ms, with zero work at rest. Part two built the proving
> slice on it. **Gate G2 is open**: it waits for the owner's look at the four
> screenshots in `reports/S4-screenshots/` (question 1 of the report). The
> third passage is Psalm 87 with Hebrew, as round 003 set it, not Psalm 23
> with Syriac as the S4 entry below still says. One item of that entry is met
> only in part: of the alternatives to the chosen way of setting type, one is
> measured on the same passages and two are argued, not measured. The
> "Current stage" line below is the coordinator's and is left as it was.

> **Note added 2026-10-03 by the Stage 1 session.** S1's work is done and its
> report is `reports/S1-2026-10-03.md`; branch `work/s1-sources` is pushed and
> not merged. **Gate G1 is open**: it waits for the owner's signature on the
> cleared list in that report (§2) and for the short round 003 (§10). The
> "Current stage" line below is left as written because the Stage 2 session is
> still working under it. Two findings change what later stages may assume:
> the ETCBC Peshitta text is CC BY-NC, so S11's Syriac work has no cleared
> base yet; and three of the lexica of L001 exist only as uncorrected OCR, so
> S7's word slip stands on Strong, Abbott-Smith, BDB, TBESG and TFLSJ.

> **Note from the Stage 2 session, 2026-10-03.** S2's work is done and reported
> in `reports/S2-2026-10-03.md`; its branch is `work/s2-skeleton`, not merged.
> Two parts of its exit are not met and wait on the owner: the artifacts are
> not tagged (a tag needs the owner's word), and Linux does not idle at zero
> paints, for a reason inside the pinned toolkit (`docs/TOOLKIT_REQUESTS.md`
> M6). S2 therefore stays open until the owner rules on both. `BUDGETS.md` now
> exists. S4's first step is changed by a finding: drawing a page-sized image
> on Windows costs 145–166 ms (M1), so S4 measures the page's route to the
> window before it sets any type. The “Current stage” line below is the
> coordinator's and is left as it was.

For Opus 5.5 (or any successor) conducting the work. One stage per session or
run of sessions; each starts and ends as `HANDOFF_PROMPT.md` describes. A stage
may not begin until its “needs” are met. Gates marked **G** need the owner's
explicit sign-off (subject to interview W002).

```
S0 ─► S1 ─────────► S3 ─► G3 ┐
  └─► S2 ─► S4 ─► G2 ─► S5 ─► S6 ─► S7 ─► S8 ─► S9
                                    └────────► S10 (pilot) ─► G4 ─► S10 (scale)
                              S11, S12 after S7      S13 ─► G5
```

Current stage: **S3 and S4 open (2026-10-04).** S1 is complete and gate G1 is
passed (round 003 B301 A). S2 is complete and closed by the coordinator with
two recorded misses: no tag, and Linux idle is not zero because of the toolkit
(`RECONCILIATION_003.md` §5). Both are merged to `main`. The owner chose two
visible sessions again (C302 A): `handoff/S3-container.md` (importers and the
container study) and `handoff/S4-page.md` (the page's route to the window,
then the compositor's proving slice). Search is now two engines and the
source-use policy has changed: read `OWNER_DIRECTION_2026-10-04.md` and
`RECONCILIATION_003.md` before any stage document below.

Superseded status line, kept for the record: S1 and S2 open (2026-10-03).
The text that followed it:

**S1 and S2 open (2026-10-03).** S0 is complete: both interview
rounds are answered and reconciled (`RECONCILIATION_001.md`,
`RECONCILIATION_002.md`). The owner chose two visible sessions (V011 C); their
prompts are `handoff/S1-sources.md` and `handoff/S2-skeleton.md`, each with its
own branch and directories. Nine small OPEN items in `RECONCILIATION_002.md` §3
have working readings and go to the owner at gate G1.

The paragraph and list that follow were written between the rounds. Round 002
has since settled the name rule, the rendition, Syriac, order and token
layering, so nothing in the “may not start” list is blocked any longer; later
stages take those answers from `RECONCILIATION_002.md` and `NAME_RULE.md`.

### What may start now, and what may not

- May start: S2 skeleton and delivery (platforms, packaging, toolkit pin, idle
  floor are all GIVEN); S1 audit and fetch of sources whose admission no OPEN
  item affects — the BSB and Berean tables, STEPBible data, the lexica of L001,
  World English Bible, Charles's Enoch and Jubilees, Lightfoot, fonts.
- May not start: anything that depends on the name rule (D), the rendition (E),
  Syriac handling (Y), book order (O), the token layering (J001), or the
  worship division (U).

### Amendments from round 001 (2026-10-03)

- S3 controls are raw UTF-8, zstd and xz (K016). Token unit: subword pieces
  (K002 C), with the word layer of J001 pending.
- S4 proving passages: Mark 7:14–23, Genesis 1:1–13, and the psalm of Y005 in
  place of Psalm 23. One column per page, justified without hyphenation,
  facing-page study form. S4 also measures build-time pagination (J002).
- S5: India paper, mahogany, gilt, leather edge, three satin ribbons with
  lift-and-set, hierarchical thumb index, instant turns, night mode and two
  alternative-visibility pairs. Gold speech rendering joins S5 after D010.
- S6 gains the divine-name rule engine and its tables (after section D).
- S7: lexicon entries are merged authored text (L002 A) under the protocol.
- S10 gate G4 is the Mark pilot only; later books ship audited with review
  state recorded (X010 note; V006).
- New S10r: the rendition (after section E).
- S11 grows: full deuterocanon, Jubilees, Apostolic Fathers by attestation,
  rabbinic and patristic commentary, own translations. After the first public
  version unless O008 says otherwise.
- New S14: Worship and Practice (after section U; after the first public
  version unless U011 says otherwise).
- First public version: I015 B — S2 through S8's exact search.

---

## S0 — Reconcile the interview

Needs: `planning/interview/answers.json`.

Work
- Transcribe every answer into `ARCHITECTURE_INPUTS.md`, by question ID, as
  GIVEN, with the owner's notes verbatim. Unanswered and deferred questions are
  listed, not guessed.
- Move confirmed absences into `PRODUCT_NEGATIVE.md`; strike refused ones.
- Write `INVARIANTS.md`: the rules that follow from answers plus the absolute
  rules in `AGENTS.md`.
- List contradictions (answer against answer, answer against brief, answer
  against an OBSERVED toolkit limit) in `planning/RECONCILIATION_001.md` and
  return them to the owner. Do not resolve them.
- Draft interview round 002 from deferrals and from anything the answers open.
- Update `SOURCE_REGISTER.md` rows the answers exclude or add.
- List the decision records the answers make ready (name and question only).

Exit: the owner confirms the reconciliation. No code, no downloads.
Does not establish: any architecture.

## S1 — Sources and licences  (**G1**)

Needs: S0; interview W006 (download permission), T, C, L answers.

Work
- For each admitted row of `SOURCE_REGISTER.md`: locate the canonical upstream,
  save the licence statement, record URL, date, size, SHA-256; write the fetch
  entry (`tools/fetch_sources` with a lock file, as Plan Paint's
  `gui-forms.lock.json`).
- Write one deterministic importer per upstream format into a normalized plain
  form per work (interview K013), with a round-trip or checksum test and a
  written list of markup not carried.
- Verify completeness: book, chapter and verse counts against the versification
  table; every verse non-empty or explicitly absent with the edition's reason
  (for example Mark 7:16 in the BSB).
- Produce the attribution page text and `THIRD_PARTY_NOTICES.md`.
- Resolve the open identifications: Hart (T001), “Prophecy Bible” (T002),
  Peshitta edition (T006).
- Fonts: fetch candidates, record licences, verify coverage of every codepoint
  and mark sequence that actually occurs in the admitted texts (cantillation,
  polytonic Greek, Syriac points). A face that fails coverage is REJECTED here.

Exit **G1**: the owner signs the cleared list. Anything not cleared is out of
the tree.
Does not establish: that any text renders or compresses well.

## S2 — Repository skeleton and delivery

Needs: S0 (A002, A004, I001, I003, W003). May run beside S1.

Work
- CMake project in the Plan Paint shape: `bible_core`, tests without a
  framework, style checker, `.clang-format`, `.gitattributes` for byte-exact
  fonts and licence files.
- Toolkit pin (I003) and a window that shows one image; `--smoke-test` and
  `--profile-idle` as in PlaySuite.
- CI on the chosen runners: core tests everywhere; application build; packaging
  scripts adapted from `games/tools/` (zip + NSIS, .pkg, tar.gz + .deb);
  installer smoke with isolated state.
- Measure the floor: executable and library size, idle CPU, idle memory for the
  empty window on each platform. Write `BUDGETS.md` from these floors and the
  I004–I006 answers.
- Create the GitHub repository only on the owner's explicit word (A002).

Exit: a tagged pre-release artifact per platform that opens an empty book
window, idles at zero, and passes installer smoke. Evidence in the stage report.
Does not establish: anything about text.

## S3 — Corpus study and container  (**G3** after S4)

Needs: S1; interview K.

Work: `TEXT_CONTAINER_STUDY.md` in full. Then the encoder in `bible_build`, the
decoder in `bible_core`, the round-trip test over every work in CI, the
reproducibility check, and the decision record for the format.

Exit: decision record proposed; container builds deterministically; every work
round-trips; the laboratory record is complete with losers.
Gate **G3** (format freeze) is taken only after S4 has rendered pages from the
container.
Does not establish: page performance.

## S4 — Page compositor proving slice  (**G2**)

Needs: S2; normalized text for the three proving passages from S1 (or, before
S1 clears, the BSB and Berean tables alone); interview F010, R, F.

Work
- Decision record for “who sets the type” with at least the chosen candidate and
  one alternative measured on the same passages.
- Shape, break, paginate, rasterize and hit-test Mark 7:14–23 with Greek,
  Genesis 1:1–13 with pointed Hebrew, Psalm 23 with Hebrew and Syriac, in the
  layouts the owner chose (R001–R013).
- Reference renderings checked by eye against the interview mockup and by test
  against stored images.
- Measure: time to compose a page (p50, p95, max), memory per cached page,
  shaping-cache size, at the minimum window, the default window and a large
  2× window, on Windows, macOS and Linux.
- Expose page text to the accessibility graph.
- Record toolkit gaps met in `docs/TOOLKIT_REQUESTS.md`.

Exit **G2** (look and feel): the owner compares the native pages with the
mockup and accepts or redirects.
Does not establish: whole-book pagination or furniture.

## S5 — The book as an object

Needs: G2; interview P, N001–N005, N009–N012, R018–R020.

Work: paper material by the chosen method (P013) with its cost measured; gutter
and curvature; edge stack from position; thumb index with hit regions; ribbons
(sprites, drag and click); surround; night mode; page turning within the motion
rules; window-size behaviour including 600 × 420. Idle profile after every
addition.

Exit: the book can be leafed through by notch, ribbon, wheel and keys; idle is
zero; a page turn meets its measured budget.

## S6 — The reading client

Needs: S5, S3.

Work: whole-Bible pagination cache; reference entry and parsing (N006, N007);
history; contents page; running heads; footnotes and cross-references in their
chosen places; selection and copy; highlights, notes, subjects and tabs (H, N013–N016);
mark storage with backups; state directory and override; start-up behaviour.

Exit: a complete English reader. This is the earliest possible first public
version (I015 A).

## S7 — Original languages and lexica

Needs: S6; S1 cleared originals and lexica.

Work: Greek and Hebrew columns in every chosen arrangement; versification
mapping; alignment; interlinear; word slip with brief gloss and verbatim entries
(L002); concordance grouped by rendering (L014); pointing toggles; divine-name
handling (T018).

Exit: any word reaches its original, its entry and its other occurrences in one
gesture. (I015 B is S7 + exact search from S8.)

## S8 — Search

Needs: S7; interview S, including the golden queries (S015).

Work: the oracle first; exact and phrase index; folded forms; spelling
candidates with verification; word pairs; original-language link; result slip,
fore-edge density, coverage line; cancellation on keystroke. Then each further
channel as an experiment against the judged workload; admitted or REJECTED on
the record.

Exit: every golden query answered or its failure explained; every accelerator
proven equal to the oracle; latency reported.

## S9 — Hapax and word study

Needs: S7, S8; interview L009–L015.

Work: compute hapax sets; build each evidence channel from stored data only;
back-matter indexes; the hapax slip. Channels with no clean data source are
absent, with the reason recorded.

Exit: every hapax in both Testaments has a slip whose every row cites stored data.

## S10 — Expansions  (**G4**)

Needs: S7; interview X; `EXPANSION_PROTOCOL.md`.

Work: source format, build validation and the slip (S10a); the Mark pilot,
authored and audited (S10b); **G4** voice gate with the owner; then the campaign
in the chosen order, one reviewed book at a time (S10c). Budget per W008.

Exit: per book — authored, audited, sampled, merged, with miss rate reported.

## S11 — The wider corpus

Needs: S7; G1 cleared rows.

Work: Judith, 1 Enoch, 1 Clement (and whatever C001–C005 admit) with their
Greek; Peshitta with Syriac shaping; Psalms translation under the two-model
rule against Oliver; verse-anchored rabbinic excerpts and the Vilna Gaon set,
translated under the same rule; the boundary marking of C012–C013.

Exit: each added work round-trips, renders, is searchable, and is labelled for
what it is.

## S12 — Maps

Needs: S5; interview M.

Work: projection, vector rasterizer, relief underlay; gazetteer with confidence;
period layers each with cited sources; place links from text; plates in the
back matter. Accuracy means citation and marked uncertainty (M006, M007).

Exit: each chosen period has a map whose every feature names its source.

## S13 — Hardening and release  (**G5**)

Needs: whatever the first public version comprises (I015).

Work: accessibility pass (I011); memory and leak audit in the Plan Paint manner;
security review of every parser that reads a file; idle and start-up profiles on
all platforms; installers; licences and attribution in every package; release
notes with dedication and credits; reproducibility hash of the container.

Exit **G5**: the owner approves publication.

---

## Rules that hold in every stage

- The stage report (see `HANDOFF_PROMPT.md`) is the only way a stage ends.
- A requirement that cannot be met stops the stage; it is not weakened.
- Every performance or size statement has its measurement beside it.
- New questions for the owner are batched into the next interview round unless
  they block the stage.
- Sibling repositories are read-only. Toolkit gaps are requests.
- Scripture bytes are never touched outside importers and recorded patches.
