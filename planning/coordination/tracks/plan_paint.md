# Track: Plan Paint product worker

Standing brief: `D:/malkuth/projects/briefs/PRODUCT_WORKER_BRIEF.md` (product
worker; reports to the coordinator). Repository: malkuth-software/plan-paint
(public, PolyForm Noncommercial 1.0.0). Product rules: the repo's `AGENTS.md`
(explicit types, no auto/lambdas/->/structured bindings; double-precision
kernels; `python3 scripts/check-style.py` + CTest; diaries go in ignored
`astra/`; credits "Author: Astra / Sponsor: Rainstar", dedication before
credits).

## State (2026-10-27, P2 + P3 pushed)

Phase: P1 done except the bootstrap gate (no smoke command yet). P2 pushed
as #42; waiting on CI. Notes branch pushed (6491a13), archive committed,
harness main 5c95a77 has projects/plan-paint.toml. Next: when #41/#42 are
green, merge in order #41, #42 (retargets to main), then #39, #40, each with
`gh pr merge N --merge` only after all checks on the final head are green.
The first green main push publishes v1.2.<run>. Then P3.

### Progress 2026-10-27
#42 MERGED at ef2240a (merge 0b02111; #41 merged with it). Main run
40574178852 should publish v1.2.<run>. #39/#40 updated server-side,
awaiting green; #43 retargeted to main and updated. Next: merge #39,
#40 when green, then #43; report release + merges to the coordinator.

### Progress 2026-10-27 (later)
v1.2.241 published from main run 40574178852. #39, #40 MERGED. #43 updated
to main (head 6a0c898), waiting for green incl. 'Plan Paint CI summary'.
Lockstep local build (gui_forms 8e288666 prefix, record
D:/malkuth/cache/lockstep/current-windows-x64.json): script
scratchpad pp/lockstep_build.sh full|sanitize; trees .build/lockstep/{full,sanitize}.
Codecs: webp 1.6.0 + dav1d 1.5.4 staged in .build/lockstep-codecs from
C:/Users/Shadow/file_manager/.build/toolchain/msys64 (no downloads);
FetchContent sources from C:/Users/Shadow/plan-paint/.build/native-clang/app/_deps.
Full: 66/66 ctest, style, packaged OK. Needed fix: find_package(GUIForms 0.2)
(branch claude/lockstep-sdk, uncommitted at time of writing).

### Progress 2026-10-27 (P3 merged)
#43 MERGED (b9cf5f3); main run should report 'Plan Paint CI summary' and
publish the next v1.2.N. PR #44 claude/lockstep-sdk (57321b1): GUIForms 0.2
find_package + libtiff win32-io OFF (UBSan function-type defect). Lockstep
sanitized run 66/66 clean after the fix. OPEN DEFECT: paint-icon-production-forms
intermittently fails on Windows (1/30 locally): 'Could not save the image
without replacing the existing file.' (safe_file.cpp MoveFileExW). Investigating
with a LOCAL-ONLY diagnostic in src/safe_file.cpp (original at /tmp/safe_file.orig;
restore before committing).

### Vendor identity (owner 2026-10-27: "Malkuth Software")
PR #45 claude/vendor-identity (5bfe38c): com.malkuthsoft.planpaint bundle +
pkg id, Windows VERSIONINFO (packaging/windows_version.rc.in). Install paths,
portable folders, archive names and the rainstar/RainstarPaint data dir kept.
Open decision for coordinator: Linux archive rename to malkuth-plan-paint-*
(no Linux packages exist yet). First published release: v1.2.241.

### Open PRs 2026-10-27
- #44 claude/lockstep-sdk: GUIForms 0.2, libtiff win32-io OFF, + safe_file
  POSIX-rename fix (pending commit after the 200-run loop): MoveFileExW replace
  failed ERROR_ACCESS_DENIED (5) when the async file_stamp probe had the target
  open (paint-icon-production-forms ~1%).
- #45 claude/vendor-identity (b94a686): + Maintainer 'Malkuth Software
  <maintainer@malkuthsoft.com>' and malkuth-plan-paint recorded in docs.
- #46 claude/release-concurrency: cancel-in-progress only for pull_request.
Worktree for side branches: scratchpad pp/wt.

### Merge plan (coordinator-approved)
#41 cannot go green alone (old pin), so #42 was retargeted to main and
carries #41's commits; merging #42 merges #41. Then #39, #40, then #43.
Merge with `gh pr merge N --merge` only when every check on the final head
is green. Report to the coordinator when v1.2.N publishes.

### P3 (#43, branch claude/p3-harness-ci, head a1e1060, stacked on #42)
- build.yml: harness windows_toolchain.py restore (script
  restore-windows-toolchain.py and lock windows_toolchain removed),
  install_linux_ci_packages.sh paint + apt cache, compiler_identity.py keys
  caches (plan-paint-objects/-sources/-codecs/-skia), harness-house-style.yml
  @5c95a77 over src tests (FileStamp operator== written out), package
  artifacts packages-<platform>, objects saved only on push.
- Summary check: job `summary`, name **"Plan Paint CI summary"**, if
  always(), needs prepare/house-style/application/release; release may be
  skipped. This is the one required check for branch protection.
- #42 fix ef2240a: package-windows.py SYSTEM set now matches GUI.Forms'
  WINDOWS_SYSTEM_DLLS (UIAutomationCore.dll etc.); third run failed only
  Windows Package on that (macOS/Linux green).
- #42 fix c2c78cc: package the stx_zlib/stx_png/stx_conv notices; Skia at
  e077f9d has no externals/libpng or zlib (second #42 run failed Package
  on that). #43 updated server-side (gh pr update-branch, head a1e1060);
  never force-push (the classifier denies it). Workspace remote is origin.
- #42 fix 8e7461d: fetch_linebreak.sh (GUI.Forms needs libunibreak since
  2026-10-26); first #42 run failed configure on that, all hosts.

### Bootstrap gate (P1 gate 2)
- Smoke: CMake option PLAN_PAINT_SMOKE (image.cpp + warp.cpp + paint-warp,
  no codecs, no GUI.Forms) + scripts/smoke-build.py (in #43, 2cc784a);
  passes locally on Windows with the harness toolchain file.
- Harness: local branch `plan-paint-smoke` (523c2a7) in the workspace's
  template_harness: [bootstrap] smoke = ["{python}", "scripts/smoke-build.py"]
  + names test; coordinator pushes it. Clean-machine proof runs in
  template_harness bootstrap.yml (PR #2), T5 adds products to its matrix.
- Local toolchain: junction .toolchain/a6c66b.../msys64 -> D:/a/_temp/msys64
  (spec digest a6c66b10...); restore_local toolchain finds it, no
  download. restore_local cache: none published yet (expected).
  .toolchain/ and .bootstrap/ ignored (#43 aff542c).

### P2 (#42, branch claude/gui-forms-e077f9d, head 40fc376, stacked on #41)
- Cloud pin: malkuth-software/gui_forms e077f9d5 (v0.2.1308) +
  build-e077f9d5 release; toolchain digests unchanged; cache digests from
  the GitHub asset digests (no download).
- Drift fixed: FlowLayoutPanel -> FlowLayoutPanelCompat; clipboard image
  result is std::optional<HostImage> image (editor.cpp, forms_tests.cpp).
- Harness: build.yml native job checks out template_harness at harness.lock
  (inline form), sets HARNESS_PROJECT/HARNESS_WORKSPACE; scripts use
  template_harness/harness/{windows_toolchain,build_macos_runtimes,
  macos_runtime_manifest,audit_macos_minimum}.py and harness/cmake/llvm22.cmake
  (native-build.sh exports HARNESS_LLVM_RUNTIME from GUI_FORMS_LLVM_RUNTIME).
- CMake baseline 1.2.0 (owner-approved).
- Local evidence: clang 22.1.8 -fsyntax-only over src/forms + tests against
  e077f9d headers (worktree of C:/Users/Shadow/gui_forms at
  D:/malkuth/workspaces/plan-paint/gui_forms); check-style, check-languages,
  test_toolkit_cache, test_publication, harness house style. No full local
  build: local MSYS2 lacks libwebp/dav1d (downloads need the coordinator);
  owner plan: local builds will use the lockstep GUI.Forms install
  (D:/malkuth/gui_forms_lockstep -> D:/malkuth/cache/lockstep/) once W's
  harness/lockstep.py lands. Plan Paint's SDK path (find_package GUIForms,
  RAINSTAR_TOOLKIT_SOURCE_DIR unset) is the hook for that.

### Owner direction 2026-10-27 (PHASE_D_DESIGN.md "Local lockstep before cloud pins")
- Local: one lockstep GUI.Forms; cloud: pinned pushed commit built from
  source; source SDK + pinned caches in cloud later.
- Downloads: shared copy in D:/malkuth/cache/, coordinator fetches.
- A Plan Paint change needing a GUI.Forms change waits for that merge.
- P3: add one always-reporting summary check; protection after P3.

### Milestones
- PR #39 `claude/relicence-followups` (head b5e6262): 22-language
  translations of the two licence strings, leftover MIT claims, README typo/
  link, dialog.cpp indentation, help literal wrap, malkuth-software links.
  check-languages + check-style pass locally; CI green on macOS and both
  Linux; Windows fails (pin, expected). Cannot merge until P2 fixes Windows.
- PR #40 `claude/icon-study-to-notes` (head 48329ec): removes docs/icon-editor-study,
  repoints ICON_EDITING.md to notes plan-paint/interviews/icon-editor.
  Merge only after the notes branch is published.
- Notes branch prepared: `D:/malkuth/workspaces/plan-paint/notes-plan-paint`
  (orphan `plan-paint`, commit 6491a13): README + interviews/icon-editor/
  (study files, answers.json from Downloads, app-icon.png). Coordinator
  pushes it.
- Archive for private projects repo: source path
  `C:/Users/Shadow/plan-paint/astra/icon-capability-study` (20 MB) ->
  archive/plan-paint/icon-capability-study/ (coordinator commits).
- Harness: branch `plan-paint-project` (c6b5618) in
  `D:/malkuth/workspaces/plan-paint/template_harness`: projects/plan-paint.toml
  + harness/test_project_names_plan_paint.py. Whole harness suite passes
  (292 tests). Coordinator pushes/merges.
- Workspace `D:/malkuth/workspaces/plan-paint/` = product clone (root
  layout, source_dir "."), with template_harness/, notes/ (main) and
  notes-plan-paint/ worktree inside. Branch `claude/harness-adoption`
  (local, 03e8950): .gitattributes `* text=auto eol=lf` (Cairo notices 1-2
  stay CRLF -text) + .gitignore harness entries. harness.lock is added
  once the harness commit with plan-paint.toml is on harness main; then push.

### Owner decisions (2026-10-27, via coordinator)
- icon-capability-study receipts -> private projects archive; study text ->
  public notes branch.
- Keep publish-on-green-main; raise the base version above anything
  installed before the first release. Fact: version = CMake baseline 1.1.4
  + workflow run_number (run numbers survived the transfer, 229 now), so
  old releases went up to ~1.1.229. Plan: baseline 1.2.0 in the P2 PR
  (also safe if P3 renames the workflow, which resets run_number).
- Branch protection: owner applies it once I send required check names.
  Until then never `--auto`; `gh pr merge N --merge` only when every check
  on the final head is green.
- origin (falseywinchnet/plan-paint) redirects to the new repo; only my
  local remote-tracking refs hold old history. Push only branches based on
  895abb2.

### Hazards noted for P3
- build.yml uploads package artifacts named `plan-paint-<platform>` and the
  release job downloads `plan-paint-*`; the harness artifact prefix is also
  `plan-paint-`. Rename package artifacts (e.g. `packages-<platform>`) or
  narrow the pattern before harness artifacts appear.
- Required check names today: prepare; application (windows-2022,
  windows-x64, msys2 {0}); application (macos-15, macos-arm64, bash);
  application (ubuntu-24.04, linux-x64, bash); application
  (ubuntu-24.04-arm, linux-arm64, bash). They may change in P3.

### Repository facts
- Remote history: `895abb2` Initial commit (PolyForm) <- `c91df3e` owner's
  README edit (licence sentence; typo "persuant"). No other branches.
- 38 old PRs remain listed on GitHub (all merged; #38 MIT licence only
  touched LICENSE, superseded).
- No releases on malkuth-software/plan-paint (old ones gone with the reset).
- Main is not branch-protected. **A green push to main publishes a release**
  (`build.yml` release job, "automatic publication"); it ran on c91df3e and
  was skipped only because Windows failed.
- Main CI is red: Windows job "Restore pinned Windows toolchain" fails,
  `gh release download build-75da5767... --repo falseywinchnet/gui_forms`
  -> release not found (the old cache release was pruned; commit 75da576
  still exists in malkuth-software/gui_forms). Linux x64/arm64 and macOS
  arm64 pass. Fixed in P2 by re-pinning (`third_party/gui-forms.lock.json`).

### Local checkout C:/Users/Shadow/plan-paint
- Remote `origin` = falseywinchnet/plan-paint (old URL; holds OLD history).
  I added remote `malkuth` = malkuth-software/plan-paint. **Never push from
  this checkout's old branches.** New work goes on branches cut from
  `malkuth/main`, checked with `git merge-base --is-ancestor 895abb2 HEAD`.
- 34 local branches, all tracking origin; every local and origin branch is
  an ancestor of old main 3edc452. Tree(3edc452) == tree(3040675, current
  checkout HEAD) == tree(895abb2) minus the relicence diff (29 files).
  => **No git work is local-only; nothing to replay from branches.**
- No stashes, no uncommitted changes, no extra worktrees. Untracked a.txt
  is in gui_forms (not plan-paint).
- Ignored, local only: `astra/` (1.9 GB: diaries, PR bodies, receipts,
  packages, icon-capability-study 20 MB, interview build scripts,
  msys2-base.tar.xz), `gui_forms/` (toolkit checkout), `.build/`, `.ccache/`,
  `build-deps/`, `dist/`, `cache-manifest.json`.
- Icon interview answers: `C:/Users/Shadow/Downloads/plan-paint-icon-interview-answers*.json|html`
  (8 files; authoritative per astra/post-icon-tasks.md: Q03 B, Q04 B, rest A).
- Last session state (astra/post-icon-tasks.md, 10-21): all queued work
  (stickies, stroke styles, Organic, endpoint markers, colour boxes, corner
  softening, native clipboard check) merged as PRs 32-37. Nothing in flight.

### Relicence defects found in 895abb2 (to fix in one PR on malkuth/main)
1. 22 language files carry the two new English licence strings untranslated
   (About label `about-license`, and the About help topic's licence
   sentence): ar bn-bd de-de es-es fa-ir fr-fr he-il hi-in id-id it-it ja-jp
   pa-in pa-pk pcm-ng pt-br ru-ru sw-ke uk-ua ur-pk vi-vn zh-cn zh-tw.
   How: I translate them myself, the same way the earlier sessions produced
   all existing strings (astra/*translations.py), reusing each file's own
   wording from the old MIT strings; keep "PolyForm Noncommercial License
   1.0.0" and "LICENSE" untranslated as names; verify with
   `scripts/check-languages.py`. Independent proofreading stays an open
   item, as for all translations (docs/releases/1.1.1.md).
2. Leftover first-party MIT claims: `packaging/README.txt:26`,
   `THIRD_PARTY_NOTICES.md:17` ("Paint's MIT license"),
   `tests/fixtures/formats/README.md:3`, `assets/fonts/cairo-unicode/SOURCE.md:8`
   (wording). README.md:236 was rewritten by the owner (typo "persuant",
   unlinked "License"): fix spelling and link only.
3. `src/forms/dialog.cpp` about-license string continuation lost its
   indentation in the relicence.
4. Old-org links (falseywinchnet/...) in README badges and docs: update to
   malkuth-software where they point at plan-paint; gui_forms lock in P2.

### Notes side branch (`plan-paint` on malkuth-software/notes, orphan like `gui_forms`)
Prepared in my own notes clone (not the shared D:/malkuth/notes). Proposed
layout: README.md; interviews/icon-editor/ (INTERVIEW.md, mockups.html,
interview.css, the answers JSON); studies/icon-editor/ (README, CAPABILITIES,
CONSERVATION, EVIDENCE, SPECIFICATIONS). Then a product PR removes
`docs/icon-editor-study/` and repoints `docs/ICON_EDITING.md:964`.
Open: whether astra/icon-capability-study (20 MB receipts) goes too.

## Decisions in force
- Push only branches based on 895abb2; PRs with auto-merge; never main.
- Downloads: ask the coordinator first.

## Plan
P0 done -> relicence PR #39 -> notes branch + PR #40 -> P1 workspace D:/malkuth/workspaces/plan-paint/ + projects/plan-paint.toml
+ harness.lock -> P2 SDK v0.2.1308+ source + build-<sha> cache pin -> P3 CI on
windows-x64, macos-arm64, linux-x64, linux-arm64 -> P4 releases.

## HANDED OFF 2026-10-27

The product worker stood down at the owner's request. The coordinator's watcher
merges the remaining PRs; the worker merged nothing after #43.

### Open PRs (malkuth-software/plan-paint)
- #44 `claude/lockstep-sdk`, head 776ba86. Contains: `find_package(GUIForms 0.2)`;
  libtiff built with `win32-io` OFF, fixing the UBSan call through a mismatched
  function type at tif_open.c:611; the safe_file.cpp POSIX-rename fix (d8b06df), with a
  Windows regression case in tests/core_tests.cpp that fails without it. Needs: green CI
  on 776ba86, then merge. Local lockstep runs (gui_forms 8e288666 prefix) on the fix:
  Release 66/66 plus package; ASan+UBSan 65/66. The one failure is described under
  "Open defect" below.
- #45 `claude/vendor-identity`, head b94a686. Contains: com.malkuthsoft.planpaint
  (app and pkg), Windows VERSIONINFO naming Malkuth Software, and docs recording
  `malkuth-plan-paint` with Maintainer `Malkuth Software <maintainer@malkuthsoft.com>`.
  Needs: a branch update after #44, green CI, then merge. Open decision for the
  coordinator: renaming the Linux archive to malkuth-plan-paint-* (it is not renamed,
  and there is no .deb yet).
- #46 MERGED (cancel-in-progress only for pull_request).

### Release state
- v1.2.241 (first release, from #42's merge) and v1.2.249 (#43's merge, the latest)
  are published. "Plan Paint CI summary" has reported on main, so branch protection
  can be applied.

### Open defect (not started)
- paint-forms failed once in the sanitized Windows tree with
  "expanded shape hover displays an accessible tooltip" (tests/forms_tests.cpp:684,
  150 s against 95 s in a passing run). It is likely timing under sanitizer load and is
  unrelated to the save fix. It must be investigated, not accepted. Log:
  scratchpad pp/sanitize3.log.

### Unpushed or local-only state
- Nothing unpushed in git. The notes `plan-paint` branch (6491a13) is pushed. Its clone
  is the worktree D:/malkuth/workspaces/plan-paint/notes-plan-paint (the notes main
  clone is D:/malkuth/workspaces/plan-paint/notes). The icon-editor study moved to that
  notes branch under interviews/icon-editor (#40 merged; docs/ICON_EDITING.md links
  there). The 20 MB icon-capability-study archive was committed by the coordinator to
  the private projects archive. The harness smoke entry (523c2a7) is on
  template_harness main.
- Old checkout C:/Users/Shadow/plan-paint holds the pre-reset history; never push from
  it. Its .build/native-clang/app/_deps holds the hash-verified tiff 4.7.2, libavif
  1.4.2, lunasvg and tinyxml2 sources that the local lockstep build reuses through
  FETCHCONTENT_SOURCE_DIR_*.
- Local lockstep build: script
  C:/Users/Shadow/AppData/Local/Temp/claude/C--Users-Shadow-gui-forms/005dc99f-1086-463f-978e-5723b295c49c/scratchpad/pp/lockstep_build.sh
  (full|sanitize). Trees: D:/malkuth/workspaces/plan-paint/.build/lockstep/{full,sanitize}.
  Codecs: libwebp 1.6.0 and dav1d 1.5.4, staged in .build/lockstep-codecs from
  C:/Users/Shadow/file_manager/.build/toolchain/msys64 (no download).
- Scratchpad worktree pp/wt is on claude/vendor-identity (pushed); remove it with
  `git worktree remove` when done.
- Would be lost if the scratchpad is cleared: only the build script and logs, which are
  reproducible from this section.

## Pending from GUI.Forms (2026-10-28)
Owner: Plan Paint's interface is refactored later, after this GUI.Forms stage is finished and finalized, by another sibling on the owner's schedule. Nothing here is done before then; these are notes for that work.
- GUI.Forms now validates StableIds (dot-separated segments of [a-z0-9]+(-[a-z0-9]+)*; consteval for literals; a bare const char* is refused). When Plan Paint moves to that GUI.Forms: atlas.cpp builds 6 StableIds from a ternary of literals, and pattern_canvas uses StableId(ids[i]) with const char*: wrap them in std::string. Other runtime IDs are fine.
