# Planning index

> **Note from the Stage 4 session, 2026-10-03.** The page now exists: the
> proving slice (Mark 7:14–23 with Greek, Genesis 1:1–13 with Hebrew, Psalm 87
> with Hebrew) is composed by this project's own compositor and shown in the
> window, on branch `work/s4-page` (pushed, not merged). Read
> `reports/S4-2026-10-03.md` first: it holds the measurements, what is and is
> not established, and seven questions for the owner, the first of which is
> gate G2. New artifacts: `reports/S4-ADR-001-who-sets-the-type.proposed.md`
> (a proposed decision record, awaiting the owner), `reports/S4-screenshots/`,
> `../experiments/` (four experiment records), `../assets/`, and section 1b
> of `../docs/TOOLKIT_REQUESTS.md` (M7–M13). The status line and table below
> are the coordinator's and are left as they were.

> **Note added 2026-10-03 by the Stage 1 session.** Sources have now been
> fetched: 118 files are pinned in `../sources/lock.json` and kept under
> `.build/` (never in the tracked tree). New artifacts: `../sources/README.md`
> (the record per source), `../sources/MEASUREMENTS.md`, `../sources/IMPORTERS.md`,
> `ATTRIBUTION.md`, `../THIRD_PARTY_NOTICES.md`, `proposals/` (lists for the
> owner to strike), `reports/S1-2026-10-03.md`. `SOURCE_REGISTER.md` has a
> dated Stage 1 section that prevails over its older tables. Gate G1 awaits the
> owner. The status line and table below are left as written for the Stage 2
> session to update when both branches are brought together.

> **Note from the Stage 2 session, 2026-10-03.** There is now application code
> and a build: an empty book window, packaged and installer-tested on four
> platforms in CI, on branch `work/s2-skeleton` (not merged). Read
> `reports/S2-2026-10-03.md` for what is and is not met, `BUDGETS.md` for the
> measured floors, `../docs/BUILDING.md` for the build, and
> `../docs/TOOLKIT_REQUESTS.md` for six toolkit gaps met with their
> measurements. `TOOLKIT_NOTES.md` carries a dated correction note. The status
> line and table below are the coordinator's and are left as they were; two
> rows are now out of date: `BUDGETS.md` exists, and `TOOLKIT_REQUESTS.md` is
> no longer only anticipated.

Status on 2026-10-03: **both interview rounds answered and reconciled; Stages 1
and 2 open.** No application code, no build, no corpus has been fetched. Read
the two `ARCHITECTURE_INPUTS` files (the owner's answers), the two
`RECONCILIATION` files (what they settled), `INVARIANTS.md` and `NAME_RULE.md`
before anything else in this directory.

## Artifacts

| File | Role | Status |
| --- | --- | --- |
| `OWNER_BRIEF_2026-10-03.md` | The founding direction, verbatim | GIVEN |
| `PROGRAMMING_HOUSE_STYLE.md` | The owner's coding standard, complete | GIVEN |
| `CHARTER.md` | Objective, GIVENs, governing question, refusals | current |
| `PRODUCT_NEGATIVE.md` | What the product is not | proposed absences await interview A007 |
| `DECISION_PROTOCOL.md` | Labels, decision and experiment records, negative-results ledger | current |
| `TOOLKIT_NOTES.md` | What the sibling repositories provide, with locators | OBSERVED via digest; re-verify at first use |
| `SOURCE_REGISTER.md` | Every text, lexicon, font and dataset with its licence status | CANDIDATE rows; audit is Stage 1 |
| `ARCHITECTURE_HYPOTHESES.md` | Component map and the mechanisms proposed for each | HYPOTHESIS |
| `TEXT_CONTAINER_STUDY.md` | Protocol for the compression study | protocol only; no measurements |
| `EXPANSION_PROTOCOL.md` | How the sibling model reads, writes, cites and is audited | HYPOTHESIS; voice gate pending |
| `STAGE_PLAN.md` | Stages, dependencies, gates | current |
| `HANDOFF_PROMPT.md` | How an Opus 5.5 session starts and ends a stage | current |
| `ARCHITECTURE_INPUTS.md` | Round 001 answers, transcribed mechanically | GIVEN |
| `RECONCILIATION_001.md` | Corrections, new GIVENs, contradictions, consequences | settled by round 002 |
| `ARCHITECTURE_INPUTS_002.md` | Round 002 answers, transcribed mechanically | GIVEN |
| `RECONCILIATION_002.md` | What round 002 settled; nine small open items with working readings | current |
| `NAME_RULE.md` | The divine-name rule gathered as a specification | GIVEN, with marked open cases |
| `handoff/S1-sources.md`, `handoff/S2-skeleton.md` | Prompts for the two open stages | current |
| `INVARIANTS.md` | Rules that now hold | current |
| `reports/S0-2026-10-03.md` | Stage 0 report | current |
| `interview/README.md` | How to run the interviews and what happens to the answers | current |
| `../interview/` | The interview itself: 15 sections, 255 questions, live mockups | issued |
| `../docs/TOOLKIT_REQUESTS.md` | Gaps recorded for the GUI.Forms owner | open |
| `../decisions/README.md` | Decision record index | empty |

## Deliberately absent until their inputs exist

- `BUDGETS.md` — written in Stage 2 from measured floors; the owner's ceilings
  are in `INVARIANTS.md`.
- Any decision record — none may be written before Stage 0 closes.
- `PLAN.md` for implementation detail inside a stage — written at the start of
  that stage, not before.

## The interview loop

1. The owner answers in the interview page; answers are saved to
   `planning/interview/answers.json` (or exported and placed there).
2. Stage 0 transcribes each answer into `ARCHITECTURE_INPUTS.md` as GIVEN, moves
   confirmed absences into `PRODUCT_NEGATIVE.md`, and lists deferred questions.
3. Contradictions between answers, or between an answer and the brief, are
   written down and returned to the owner. They are never resolved by inference.
4. Deferred and newly raised questions form interview round 002.
5. The interview files themselves are not edited after issue; a later round is a
   new set of question files.

## Current facts worth knowing before reading further

- GUI.Forms has no rich typesetting interface. A justified page with
  superscripts, small capitals, four scripts and per-word hit-testing is not
  expressible through its stock text API. The page compositor is therefore the
  central engineering question (interview F010; `ARCHITECTURE_HYPOTHESES.md` §3).
- The File Manager engine has no finished fuzzy or semantic search. What
  transfers is its law — approximate channels propose, exact records decide,
  coverage is disclosed — and one single-edit candidate channel.
- Hart's New Testament is under copyright. “The Prophecy Bible” is unidentified.
  Both are open questions to the owner (T001, T002).
