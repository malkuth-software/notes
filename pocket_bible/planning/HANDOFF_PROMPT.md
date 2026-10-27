# Handoff prompt

How a session begins and ends a stage. Paste the block below, with the stage
filled in, as the first message of a new Opus 5.5 session opened in
`C:/Users/Shadow/bible`.

---

```
You are conducting Stage S<N> of the Pocket Bible project in
C:/Users/Shadow/bible.

Before anything else, read, in order: AGENTS.md; planning/README.md;
planning/OWNER_BRIEF_2026-10-03.md; planning/CHARTER.md;
planning/PRODUCT_NEGATIVE.md; planning/DECISION_PROTOCOL.md;
planning/STAGE_PLAN.md (your stage and the rules at its end);
planning/ARCHITECTURE_INPUTS.md if it exists; planning/TOOLKIT_NOTES.md;
planning/PROGRAMMING_HOUSE_STYLE.md; the previous stage report under
planning/reports/; and the documents your stage names as inputs.

Then state, before editing anything: the stage's exit condition in your own
words; which of its needs are met and which are not; the first smallest
falsifiable step. If a need is unmet, stop and say so.

Hold to these throughout:
- Work only in this repository. file_manager, plan-paint, games and notepad are
  read-only references. Borrow the toolchain at
  C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin read-only.
- No scripture, lexicon or commentary text from your own recall. Text enters
  through hash-pinned sources and deterministic importers only.
- Label every claim GIVEN, OBSERVED, MEASURED, HYPOTHESIS, CANDIDATE, REJECTED
  or DECIDED. Do not write a decision record without the owner's approval line.
- The house style governs all code, tests and tooling you write. Review your
  own diff against it and record the reviewed scope and remaining violations.
- Do not create subagents or workers unless ARCHITECTURE_INPUTS.md records the
  owner's permission (interview W001).
- Outward-facing actions (creating or publishing a repository, pushing tags,
  releases, contacting a publisher) need the owner's explicit word.
- Two compile jobs at most. Build under .build/. Scratch under astra/.
- Stop rather than weaken a requirement.

End the stage with planning/reports/S<N>-<date>.md containing: what was done;
files changed; exact commands run and their outputs or where the outputs are
stored; measurements with environment; what is established and what is not;
rejected alternatives and why; open questions for the owner; unresolved risks;
house-style review scope and remaining violations; the next smallest
falsifiable step. Then update the "Current stage" line in STAGE_PLAN.md and the
status line in planning/README.md.
```

---

## Notes for whoever writes the next prompt

- If a stage spans sessions, each session after the first reads the newest file
  under `planning/reports/` and any `astra/` continuation note first, and
  resumes from recorded receipts, not assumptions.
- A correction to a tracked planning document is added at its top with a date;
  history is not rewritten.
- Sibling sessions working in parallel (for example S1 and S2) each own a
  directory set named at their start; the coordinator session owns commits.
- A gate is passed only by an owner statement quoted in the stage report.
