# Stage 2 handoff — repository skeleton and delivery

For an Opus 5.5 session opened in `C:/Users/Shadow/bible`. Paste the block as
the first message. This session runs beside the Stage 1 session; the two own
different directories.

```
You are conducting Stage S2 (repository skeleton and delivery) of the Pocket
Bible project in C:/Users/Shadow/bible, on branch work/s2-skeleton (create it
from main).

Read first, in order: AGENTS.md; planning/README.md; planning/CHARTER.md;
planning/INVARIANTS.md; planning/RECONCILIATION_001.md;
planning/RECONCILIATION_002.md; planning/TOOLKIT_NOTES.md;
planning/ARCHITECTURE_HYPOTHESES.md §1; planning/STAGE_PLAN.md (S2 and the
closing rules); planning/DECISION_PROTOCOL.md;
planning/PROGRAMMING_HOUSE_STYLE.md. Then read, in the sibling repositories and
read-only: games/CMakeLists.txt, games/cmake/, games/src/main.cpp,
games/src/storage.cpp, games/src/runtime_paths.cpp, games/scripts/check-style.py,
games/tools/package_*.py, games/tools/smoke_package.py,
games/.github/workflows/applications.yml, games/docs/BUILDING.md,
games/docs/PERFORMANCE.md; plan-paint/CMakeLists.txt and its .clang-format.
TOOLKIT_NOTES.md is a digest: re-open each file it cites before relying on it,
and correct the digest where it has drifted.

Then state, before editing: S2's exit condition in your own words; the toolkit
commit you propose to pin and why; the first smallest falsifiable step.

You own: CMakeLists.txt, cmake/, src/, tests/, scripts/, packaging/, .github/,
.clang-format, docs/BUILDING.md, docs/TOOLKIT_REQUESTS.md, planning/BUDGETS.md,
planning/reports/S2-*.md. You do not touch sources/, tools/fetch_sources.py,
planning/SOURCE_REGISTER.md or THIRD_PARTY_NOTICES.md — the Stage 1 session
owns those. Shared planning documents are changed only by a dated note at their
top.

GIVEN for this stage: C++20 in the house style; platforms Windows x64, macOS
arm64, Linux x64, Linux arm64 (A004); packages NSIS per-user installer and
portable zip, macOS .pkg ad-hoc signed, Linux .deb and tar.gz (I001); pinned
toolkit source built with the application, PlaySuite's model (I003 A); one
executable, the toolkit library, fonts, and one container file (I002 A); no
network code linked (A008 A); no audio subsystem (U009 B); one window (N019 A);
minimum window 600 x 420 (I014 A); state directory under a Rainstar folder with
an environment override (I008 A); MIT licence with the owner's dedication line
(A010 A). Ceilings: 100 MB installed, 150 MiB resident, 400 ms to a readable
page — ceilings, not targets; you measure the floors (J008 A).

Work:
1. CMake project in the Plan Paint shape: bible_core (toolkit-free static
   library), bible_page (toolkit-free, empty for now), bible_forms (links
   GUIForms::Application), the executable pocket-bible as a thin main.
   An option to build without the application so the core and its tests build
   everywhere. Tests without a framework, one executable per test file.
   -Wall -Wextra -Wpedantic; Release with section GC; .clang-format.
2. Style checker adapted from games/scripts/check-style.py, run in CI.
3. Toolkit: clone falseywinchnet/file_manager at a pinned commit into
   .build/toolkit (never use or modify C:/Users/Shadow/file_manager); build
   gui_forms from it as PlaySuite does; record the pin in one place. Turn off
   everything not needed (gallery, tests, audio). Record what must stay on.
4. A window that shows one application-rendered BGRA image through
   load_bgra32_premultiplied and draw_image — a flat page colour is enough —
   with --smoke-test (at least one frame presented) and --profile-idle N, as
   PlaySuite's main.cpp does.
5. bible_core beginnings that Stage 1's importers and Stage 3's container will
   need and that no open question touches: bounded file read, atomic write
   with fsync, the checksummed envelope for small state files, state-directory
   resolution with POCKET_BIBLE_STATE_DIR, SHA-256. Each with tests.
6. CI on four runners: core tests everywhere; application build and tests;
   packaging adapted from games/tools; installer install, smoke and uninstall
   with isolated state. Private-repository Actions minutes are the owner's;
   keep the matrix lean and cache the toolkit build.
7. Measure and record in planning/BUDGETS.md, per platform: executable size,
   toolkit library size, package size, launch-to-first-frame, idle CPU and
   private memory for the empty book window by the --profile-idle protocol.
   These are the floors every later stage is held against.
8. Every toolkit gap you actually meet goes into docs/TOOLKIT_REQUESTS.md with
   the file and line that shows it; remove anticipated items that prove wrong.

Hold to these: no subagents (W001 A); two compile jobs at most; builds under
.build/; nothing in the sibling repositories is modified; the Plan Paint
toolchain at C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin is
borrowed read-only; do not add a third-party dependency beyond the toolkit and
what it already brings without asking (W009 A); do not publish a release or
change repository visibility (W009 D). Commit freely on your branch; push it at
the end of the stage; do not merge to main. Dedication and credits follow the
root README.md exactly.

End with planning/reports/S2-<date>.md as planning/HANDOFF_PROMPT.md describes:
a pre-release artifact per platform that opens the empty book window, idles at
zero and passes installer smoke, with the evidence.
```
