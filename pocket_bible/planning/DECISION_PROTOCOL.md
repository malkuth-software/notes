# Decision protocol

Modelled on `file_manager/planning/DECISION_PROTOCOL.md`.

## 1. Claim grammar

Every statement in a planning or design record carries one label.

| Label | Meaning | May it drive architecture? |
| --- | --- | --- |
| GIVEN | Required or excluded by the owner | Yes |
| OBSERVED | A fact in a named source, with locator | As a constraint, not a forecast |
| MEASURED | Reproduced by a named procedure, environment and data | Within the measured domain |
| HYPOTHESIS | A falsifiable mechanism | No; it creates an experiment |
| CANDIDATE | An option admitted for comparison | No |
| REJECTED | Failed a declared gate | No, absent a new mechanism |
| DECIDED | Accepted by a numbered record | Yes |

“Fast”, “small”, “light”, “accurate”, “faithful”, “literal” and “semantic” are
not statuses. A claim using them names its measurement or says it is unverified.

## 2. Decision record

Records live in `decisions/` as `ADR-NNN-SHORT-TITLE.md`.

```
# ADR-NNN: the question, as a question

Status: proposed | accepted | superseded by ADR-MMM
Date:
Owner approval: (quote and date, or “pending”)

## GIVEN constraints
## Workloads and failure modes
## Candidates
## Evidence and measurements
## Decision
## Why the other candidates lost
## Consequences
## Reversal and migration path
## Unresolved edges
```

No record is accepted merely because code already exists. No record is written
before Stage 0 closes.

## 3. Experiment record

Kept under `experiments/NNN-short-title/README.md` with its inputs and scripts.

```
Question:
Hypothesis and what would falsify it:
Control(s):
Data (source, hash, size):
Environment (machine, OS, compiler, flags):
Procedure (exact commands):
Results (table; p50/p95/max where timing):
What this establishes:
What this does not establish:
Next smallest falsifiable step:
```

A correctness oracle comes before any performance work. For the container the
oracle is the byte-exact round trip. For search it is the exhaustive scan. For
the compositor it is the reference rendering of the proving passages.

## 4. Negative-results ledger

`planning/NEGATIVE_RESULTS.md`, created with the first rejection. Each entry:
what was attempted; its strongest version; what improved; what was harmed; why
it was wrong for this product; what new evidence would reopen it.

## 5. Judged workloads

- **Search**: the owner's golden queries (interview S015), each with the verses
  the owner judges relevant. A channel is admitted only if it finds judged
  verses the exact channels miss, at a stated false-candidate rate.
- **Expansions**: the owner's Mark 7 exemplar and the Mark pilot review.
- **Compression**: the whole admitted corpus, against the controls of interview
  K016.
- **Page**: the three proving passages (Mark 7:14–23, Genesis 1:1–13, Psalm 23)
  at the minimum window, the default window and a large high-density window.

## 6. Owner direction

Interview answers are transcribed to `ARCHITECTURE_INPUTS.md` as GIVEN. Later
direction is filed as `planning/OWNER_DIRECTION_YYYY-MM-DD.md`; corrections as
`OWNER_CORRECTION_YYYY-MM-DD.md`. Later files prevail. Corrections are added at
the top of the affected document, not by rewriting history.
