# Stage 4 handoff — the page: its route to the window, then the proving slice

For a session opened on the Pocket Bible repository. Paste the block as the
first message. It runs beside the Stage 3 session; the two own different
directories.

```
You are conducting Stage S4 (the page compositor's proving slice) of the Pocket
Bible project. Work on branch work/s4-page, created from main. If you were
started in a worktree, treat it as the repository root.

Read first, in order: AGENTS.md; planning/README.md; planning/INVARIANTS.md
(its dated note at the top prevails over the rules beneath it);
planning/OWNER_DIRECTION_2026-10-04.md; planning/RECONCILIATION_003.md, then
RECONCILIATION_002.md and RECONCILIATION_001.md;
planning/ARCHITECTURE_HYPOTHESES.md sections 1, 3 and 4; planning/BUDGETS.md;
docs/TOOLKIT_REQUESTS.md; planning/reports/S2-2026-10-03.md; docs/BUILDING.md;
sources/MEASUREMENTS.md section 3 (fonts); planning/TOOLKIT_NOTES.md;
planning/DECISION_PROTOCOL.md; planning/PROGRAMMING_HOUSE_STYLE.md. The owner's
answers are the three ARCHITECTURE_INPUTS files; sections R, P and F of round
001, J of round 002 and F and W of round 003 govern this stage. The interview
mockup (interview/index.html, the Book tab) is what the owner has seen.

Then state, before editing: the stage's exit condition in your own words; the
first smallest falsifiable step.

You own: src/page/, src/forms/, tests named page_* and forms_*, cmake/Page.cmake,
assets/ (fonts and fixtures), experiments/, docs/TOOLKIT_REQUESTS.md,
planning/reports/S4-*.md. In the root CMakeLists.txt you add exactly one include
line and change nothing else. You do not touch src/corpus/, src/core/, sources/
or tools/ — the Stage 3 session owns those. Shared planning documents are
changed only by a dated note at their top.

Part one — the page's route to the window. Stage 2 measured 146–166 ms to paint
one full-window image on Windows at display scale 2 (docs/TOOLKIT_REQUESTS.md
M1) and found a registered image held twice (M5). The owner's bar: a page turn
within 50 ms from key press to the new page presented (round 003 W303 C), pages
composed at device resolution (W304 A), zero work at rest (I18). Measure, on
Windows first and then on the CI platforms, every route the pinned toolkit
offers for getting an application-composed page onto the window: draw_image at
one image pixel per device pixel; update and patch of a registered image; a
live surface registered only for the moment of a turn and released after;
RasterCanvas; drawing the page as several strips. For each: time from request
to presented (p50, p95, max over at least thirty turns), memory held, and what
runs while idle. Report the table with its losers. If no route meets 50 ms on
the owner's machine, say so plainly, name the best, and stop for direction: do
not weaken the bar.

Part two — the proving slice, on the best route.
1. A proposed decision record for who sets the type (GIVEN: the application
   owns its compositor, F010 A): how HarfBuzz, FreeType, SheenBidi and
   libunibreak are linked at the toolkit's pinned versions on every platform
   without modifying the toolkit, and what that adds to each floor in
   planning/BUDGETS.md.
2. Shape, break, paginate, rasterize and hit-test three passages: Mark 7:14–23
   with Greek; Genesis 1:1–13 with pointed Hebrew; Psalm 87 with Hebrew. Their
   text comes from the pinned source files by an extraction tool, as committed
   fixtures that carry the source's hash — never from recall.
3. The page as the owner chose it: a two-page spread with one column per page
   (R001 A, R002 A), folding to one page when narrow; English alone for
   reading, and in study form the original on the left page with English on
   the right (R003 D note, J004 A); justified without hyphenation by
   whole-paragraph line breaking that may break early for better packing (R006
   B note); raised verse numerals, a drop numeral for the chapter, a running
   head, footnotes at the foot (R004–R011); words of Christ in red from the
   BSB's own markup; the page keeps its proportions and scales with the window
   (R020 A); three type-size steps (J003 B); usable at 600 x 420.
4. Faces: EB Garamond for English with its own Greek; Gentium Book for Greek
   beside Portsmouth; Ezra SIL for Hebrew. Portsmouth (the owner's own face,
   taken from plan-paint/packaging, English only, shipped without a licence
   file by the owner's direction) is the second English face: the compositor
   itself supplies what it lacks — the en dash, the dagger and small capitals
   (round 003 F302, F303 A). Greyscale antialiasing, light hinting,
   linear-light blending (F007–F009).
5. The shaped test Stage 1 could not run: every one of the 6,536 Hebrew
   base-plus-marks sequences and the Greek, shaped in each face, with any
   collision or unplaced mark reported. Re-test Noto Serif Hebrew this way
   (F305 B).
6. A hit map per page: pointer to word identity by search over sorted
   rectangles.
7. Measure: time to compose a page (p50, p95, max), memory per cached page,
   shaping-cache size, at 600 x 420, at the default window and at a large
   window at scale 2, on every platform; and page breaks computed ahead
   against computed on first open (J002 C).
8. Publish the page's verses to the toolkit's accessibility graph (I19).
9. Reference renderings as stored images with a test that compares them, and
   three screenshots for the owner to set beside the mockup.

Not in this stage: paper texture, ribbons, thumb index, the surround, gold,
slips, search. A flat page colour is enough.

Hold to these: C++20 in the house style; no subagents; two compile jobs at
most; builds under .build/; the sibling repositories and the toolkit checkout
are never modified — every gap met goes into docs/TOOLKIT_REQUESTS.md with
file, line and measurement; no third-party dependency beyond what the toolkit's
pin already fetches without asking; no scripture text from your own recall
anywhere, including tests and comments; every timing carries its machine, its
display scale and what it does not establish (this machine is a virtual machine
at scale 2). Commit freely on your branch; push it at the end of the stage; do
not merge to main.

End with planning/reports/S4-<date>.md in the form planning/HANDOFF_PROMPT.md
describes. Put every question for the owner in one section at its end, each
with the choice you recommend and why in two lines: the coordinator gathers
them into a single interview, and the owner will not look anywhere else. The
gate that follows this stage is the owner's look-and-feel sign-off (G2).
```
