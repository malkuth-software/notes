# Track: SwiftEdit product worker

Standing brief: D:/malkuth/projects/briefs/PRODUCT_WORKER_BRIEF.md. Prior state:
D:/gui_forms_swiftedit/STATE.md (paused session, will not resume). Reports go
to the coordinator ("Web version sync and task setup").

## State (2026-10-27, P0 intake done)

Phase: P0 done; item 1 done (PR open); P1 in progress.
- swiftedit PR #2 (claude/rtf-import, head = local claude/rtf-import in
  C:/Users/Shadow/notepad): 6 replayed commits + pin 1794ab59 + baseline
  0.4.0 (last recorded release v0.3.204; run numbers may have reset). Single
  root 05c2bd9 verified before push. Local: 32/32 headless ctest (-I 1,32;
  tests 33-40 are native window tests, not run), Python 14/14,
  Audit-HouseStyle 0. SwiftEdit has no sanitizer configuration (gap for P3).
- Coordinator decisions (2026-10-27): A = merge may publish (baseline above
  last release); B = no `--auto` until the owner protects master; merge
  myself with `gh pr merge 2 --merge` only when every check on the final head
  is green. Required check names (sent to coordinator): prepare;
  native (windows-2022, windows-x64, msys2 {0});
  native (macos-15, macos-arm64, bash); native (ubuntu-24.04, linux-x64, bash).
- Picker: File Manager main 028a3f96 (#61 merge). Adopt only after the
  coordinator confirms main's run is green, with item 3 and GUIForms 0.2.
- Local build recipe: MSYSTEM=CLANG64 MSYS2_PATH_TYPE=inherit
  /d/a/_temp/msys64/usr/bin/bash.exe -lc 'export USERPROFILE=C:/Users/Shadow;
  . /d/gui_forms_swiftedit/toolchain_env.sh; cd /c/Users/Shadow/notepad;
  python -B tools/prepare_dependencies.py --platform windows-x64 --jobs 6;
  python -B tools/Build-Native.py --platform windows-x64 --build-only --jobs 6'
  then ctest --test-dir .build/native-windows-x64/app -I 1,32.

### Repository: malkuth-software/swiftedit
- History reset: master = 05c2bd9 "Initial commit" (PolyForm NC 1.0.0). Its
  tree equals old master 15846ce except LICENSE (MIT -> PolyForm).
- No open PRs, no releases, no Actions runs since the reset. Workflows active:
  native-builds.yml (push master/v*, pull_request), mac-profile-analysis.yml
  and portable-core.yml (dispatch only).
- **Every push to master (incl. a PR merge touching non-doc paths) publishes a
  release** (tools/publication_version.py: push + refs/heads/master).
- Repo setting allow_auto_merge=false; master unprotected. `gh pr merge --auto`
  will fail until the coordinator/owner changes that (not ours to change).

### Local checkout C:/Users/Shadow/notepad (OLD history; never push its branches)
- master 15846ce (old), clean tree, no stashes.
- rtf-import ed0921e: 6 commits on 15846ce (1f5fc59 import via stx_rtf2md,
  86a5e37 Markdown view dialect, be9f68a + c154b89 startup summary, 0194c74
  shared baseline, ed0921e pins -> malkuth-software/gui_forms). None touch
  LICENSE, so they cherry-pick onto 05c2bd9 cleanly by construction.
- gui-forms-0.2-sync c270106 on 2a7ea52 (old): CMakeLists GUIForms 0.2,
  ci/dependencies.json, tools/prepare_dependencies.py (HarfBuzz on Windows,
  fetch_zstd). Replay later with the 0.2 move (item 3/4).
- Ignored local dirs: .build (3.5G), .ccache, dist, gui_forms (pin 4e126d6),
  work/ (old rewrite_*.py scripts, 96K).
- ci/dependencies.json on rtf-import: SDK 4e126d6; picker
  falseywinchnet/file_manager 964261f3 (OLD file_manager history; still
  fetchable by SHA today, will not survive); stx_rtf2md placeholder 0101ebf0.

### GUI.Forms workspace D:/gui_forms_swiftedit
- gui_forms clone on claude/rt3-rtf-write; untracked stx_cng/ (leftover).
  Local branches rt-plan, rt1..rt3, rtf2md-destination-control, integration:
  all merged upstream; keep until the coordinator says.
- Scripts: env.sh, toolchain_env.sh, build.sh, sanitize.sh, swiftedit.sh.

## In flight (2026-10-27, end of session 1)
- PR #2 claude/rtf-import (bd4cfd9): prepare/macOS/Linux green; Windows red
  (msys2 LLVM 23). Body notes the stack plan.
- PR #3 claude/gui-forms-0.2 (efeb5c0), base claude/rtf-import: SDK
  e077f9d5 + folded stx_rtf2md, picker malkuth-software/file_manager
  028a3f96, GUIForms 0.2, template_harness checkout from gui_forms/
  harness.lock (bb287401) in prepare_dependencies, HarfBuzz on Windows,
  fetch_linebreak everywhere, csv.entry Enter/Escape + verify_entry_keys
  (both checks proven to fail without their opt-in), Windows CI restores
  GUI.Forms' toolchain, libxi-dev, summary job "SwiftEdit checks".
  Local: 32/32 headless, Python OK, checkers 0, audit 0. CI pending.
- Merge plan: #3 green -> `gh pr merge 3 --merge` (into claude/rtf-import);
  then #2 green on its final head -> `gh pr merge 2 --merge` (publishes 0.4.x).
  No --auto until the owner protects master (required: "SwiftEdit checks").
- Local test recipe: ctest needs PATH=<sdk>/gui-forms-sdk/bin and
  GUI_FORMS_FONT_DIR=<sdk>/gui-forms-sdk/share/GUIForms/fonts (0.2 is DLLs);
  else 0xc0000135. Local dependency clones use core.autocrlf=false.
- P1 prepared (see below), waiting on the coordinator: harness branch
  swiftedit-project 33229a9 in D:/malkuth/workspaces/swiftedit/template_harness;
  notes branch swiftedit dc3abb3 in D:/malkuth/workspaces/swiftedit/notes;
  product branch claude/harness-adoption f9c135c (adds .gitignore lines that
  #3 also adds; rebase onto master after #2 lands, then add harness.lock).
- Open issue: SwiftEdit's harness.lock should equal gui_forms/harness.lock
  (one template_harness/ at the root serves both, adopting.md s5.4), but
  that commit (bb287401) has no projects/swiftedit.toml.

- 2026-10-27 later: harness main 523c2a7 carries projects/swiftedit.toml;
  notes branch swiftedit pushed (dc3abb3). #3 run 1: Windows restore found
  no toolchain because the runner ignores a step-env GITHUB_REPOSITORY;
  269d804 prefixes it on the command and drops PR paths-ignore (approved).
- Lock decision (mine, coordinator asked): when T5's gui_forms PR bumping
  gui_forms/harness.lock to 523c2a7 merges, MOVE the GUI.Forms pin to that
  merge commit (once its build-<sha> cache release exists, digests from the
  release API) and set SwiftEdit's harness.lock to 523c2a7: one
  template_harness/ checkout, no mismatch allowance. The newer harness reads
  [toolchain] repository, so the GITHUB_REPOSITORY prefix can then go.

## Owner plan change (2026-10-27, PHASE_D_DESIGN.md "Local lockstep before cloud pins")
- Local builds will use the lockstep GUI.Forms (D:/malkuth/gui_forms_lockstep,
  cache D:/malkuth/cache/lockstep/) once W's harness/lockstep.py lands;
  D:/gui_forms_swiftedit stays usable until then.
- Cloud CI pins e077f9d5 (v0.2.1308; build-e077f9d5 cache release exists).
  It contains RT3, so the stx_rtf2md pin FOLDS into the SDK pin: lock has one
  revision; prepare_dependencies builds stx_rtf2md from gui_forms/ and checks
  out template_harness at gui_forms/harness.lock (bb287401) at the root,
  because e077f9d5's tools are harness shims. import_cache now compares the
  cache manifest repository with the lock's (malkuth-software/gui_forms).
- Downloads: shared copy in D:/malkuth/cache/, ask the coordinator.
- A SwiftEdit change needing a GUI.Forms change waits for that merge.
- Branch protection: one always-reporting summary job "SwiftEdit checks"
  (added on the 0.2 branch). Caveat: native-builds.yml has paths-ignore
  (docs/**, *.md) on pull_request, so docs-only PRs would never report it.

## Coordinator items
1. RT3 landed: #367 merge commit **1794ab59** on gui_forms main (contains
   #390 0f59a5f0). Plan: new branch `claude/rtf-import` from 05c2bd9,
   cherry-pick 1f5fc59..ed0921e, set stx_rtf2md_revision 1794ab59, rebuild +
   headless tests + audit, check `git merge-base --is-ancestor 05c2bd9 HEAD`,
   push, open PR. Merge publishes a release (see above): coordinator decides.
2. Fuzz crash `*[~~(~~](<a*\t>)~L*`: ALREADY FIXED on main by #390
   (f4fb535d, merged 0f59a5f0). The failing job 121761202885 ran on
   claude/t5-c4-x11 @81199516, which lacks #390. Regression: the text (mode
   byte stripped; 0x2a is even = markdown mode) is a fixed input in
   stx_rtf2md/fuzz/markdown_round_trip_fuzz.cpp plus a unit test. No branch.
   Optional: rerun stx_rtf2md tests at main during item 1's rebuild.
3. CSV entry (src/csv_view.cpp:52-62, csv.entry): set_accepts_enter(true),
   set_accepts_escape(true) + test pressing Enter and Escape; do it with the
   0.2 move only (owner, 2026-10-26).
4. Picker: File Manager owns it. Wait for the final pin (file_manager#61
   merge commit, new history) via the coordinator; then move picker_revision
   (and picker_repository -> malkuth-software/file_manager) and the GUIForms
   0.2 requests together with item 3.
5. P1-P4 per the brief.

## Decisions in force
- RT4 option 1: SwiftEdit builds only the standalone stx_rtf2md package from
  a second gui_forms pin; fold into the SDK pin when it moves.
- Don't touch picker code (windows.cpp picker windows, show_picker/hide_picker,
  picker_result, Picker members, picker tests) or file_manager/.
- Desktop launches need the owner's say-so (repo AGENTS.md).

## Session 2 state (2026-10-27)
- #3 merged into claude/rtf-import (8fdb777). #2 head 8fdb777: all native
  jobs green; waiting on "SwiftEdit checks" before `gh pr merge 2 --merge`
  (publishes 0.4.x). After it merges, tell the coordinator the summary check
  has reported on master (owner protects master with it).
- PR #4 claude/lockstep-harness-pin dc7810b (base claude/rtf-import; retarget
  to master after #2): lockstep local mode + pin gui_forms 8e288666 (cache
  digests incl. linux-arm64) + harness.lock 523c2a7 (checked equal to
  gui_forms/harness.lock) + no GITHUB_REPOSITORY prefix (gui_forms.toml
  [toolchain] repository is malkuth-software/gui_forms; the shim defaults
  HARNESS_PROJECT to it). CI pending.
- Local recipe now (lockstep): MSYS CLANG64 shell, USERPROFILE set,
  toolchain_env.sh, SWIFTEDIT_LOCKSTEP=D:/malkuth/cache/lockstep/current-windows-x64.json,
  PATH += pwsh dir (C:/Users/Shadow/.cache/codex-runtimes/codex-primary-runtime/dependencies/native/powershell;
  Windows PowerShell's policy refuses the cli test's -File; a CMake
  -ExecutionPolicy Bypass change was refused by the classifier, so don't),
  then prepare_dependencies.py --platform windows-x64 and Build-Native.py
  --platform windows-x64 (builds + ctest, 32/32, no packaging).
- claude/harness-adoption f9c135c is superseded by #4 (its .gitignore lines
  are in #4); leave it local.

## Session 2, later (2026-10-27)
- #2 MERGED into master as 5074fc9 at 20:34:13 by the shared falseywinchnet
  credential; my `gh pr merge 2 --merge` printed "already merged" but the
  timing matches it, so it was most likely mine (all checks green first).
  Master push run 40579228600 queued; when "SwiftEdit checks" reports there,
  tell the coordinator (owner then protects master).
- #4 retargeted to master; head 6d3ce2e (adds README note: cli test wants
  pwsh 7 on PATH locally; coordinator: leave the CLI test as is).
- #5 claude/sanitizer-config e56348c, base claude/lockstep-harness-pin:
  Build-Native.py --sanitize (ASan+UBSan on SwiftEdit's code, LSan on
  Linux/macOS with tests/lsan.supp, RelWithDebInfo -gline-tables-only,
  app-sanitize / app-lockstep-sanitize, headless tests, never packaged);
  every native job runs it after packaging. Local windows lockstep: 32/32,
  no reports.
- Local only, NOT pushed (stack depth): claude/linux-arm64 821225d on top of
  #5: ubuntu-24.04-arm matrix entry, platform choices, Skia linux script
  (handles aarch64), deb arch map, release contract/publish/verify lists,
  CMake platform + presets, README. Push + PR after #4 merges.

## After the review (coordinator: nothing wrong, proceed)
- #4 merged into master 2602374 (21:23) with the publish fix. Master run
  40586376534 should publish the first 0.4.x: report it.
- CAUTION: workflow concurrency `native-${head_ref || ref_name}` with
  cancel-in-progress means a second master merge cancels the running master
  run (and its publication). Merge the next PR into master only after the
  previous master run has completed.
- #5 sanitizer retargeted to master (head 4442c5d); merge when green AND the
  master run is done.
- #6 claude/mac-profile-source: mac-profile-analysis.yml takes run_id and
  repository inputs (the old falseywinchnet/file_manager run 36963650387 is
  gone; repo redirects to malkuth-software/file_manager).
- #7 claude/vendor-identity 9509f2c (base master): Malkuth Software vendor;
  com.malkuthsoft.swiftedit (app + pkg); NSIS CompanyName + Publisher;
  deb malkuth-swiftedit with Replaces/Conflicts/Provides: swiftedit;
  install paths, uninstall key, Start menu folder and commands unchanged.
  Picker owner_application_id org.malkuth.swiftedit left (picker code).
- claude/linux-arm64 8df4245 local: push after #5 merges; will conflict with
  #7 in tools/installers.py (rebase after #7).

## Next step
Report the first 0.4.x release; merge #5 (after the master run), #6, #7 one
master run at a time; then rebase and push claude/linux-arm64, PR, merge.

## HANDED OFF 2026-10-27
The worker has stood down; the coordinator and the standby merge from here.

Done today: #3, #2, #4 (publish fix), #8 (cancel-in-progress on PRs only) and
#5 (sanitizer) are merged. **v0.4.210** is published from master 2602374
(15 assets: 3 installers, 3 archives, sidecars and manifests).

Open PRs, in merge order (merge each only when every check on its final head
is green):
1. **#6** claude/mac-profile-source, head 793d7c9. mac-profile-analysis.yml
   now takes run_id and repository inputs; the stale falseywinchnet run
   36963650387 is gone. Checks were green at hand-off. Needs nothing.
2. **#7** claude/vendor-identity, head 0cd0bda: Malkuth Software vendor,
   com.malkuthsoft.swiftedit (app and pkg), NSIS CompanyName and Publisher,
   deb malkuth-swiftedit with maintainer `Malkuth Software
   <maintainer@malkuthsoft.com>` and Replaces/Conflicts/Provides: swiftedit.
   Install paths, the uninstall key, the Start menu folder and the commands
   are unchanged. Checks were still pending at hand-off. Left unchanged: the
   picker owner_application_id "org.malkuth.swiftedit" in src/windows.cpp
   (picker code; nothing is persisted under it).
3. **#9** claude/linux-arm64, head 76ccad9 (based on master 116942d): the
   ubuntu-24.04-arm matrix entry and the platform plumbing. It needs a
   **rebase onto master after #7**, because both change tools/installers.py.
   Keep #7's DEBIAN_* constants and debian_control(), and give
   debian_control the architecture from #9's DEBIAN_ARCHITECTURES map
   (linux() takes platform). Release notes in #9 say "four native jobs".
   Its CI is the first arm64 run, never tried locally.

Master runs: #8 is merged, so a master push no longer cancels a running
master run; it queues instead (only the newest waiting run is kept).
Publish-Release.py publishes only master's head revision. At hand-off,
40590234774 (d1d6d56) was running and 40590269515 (116942d) was waiting.
The one-run-at-a-time caution is no longer needed.

Unpushed local branches:
- C:/Users/Shadow/notepad: master, rtf-import and gui-forms-0.2-sync are
  OLD history; never push them.
- D:/malkuth/workspaces/swiftedit: claude/harness-adoption f9c135c,
  superseded by #4.

Workspace contents that are not in git: build trees under
C:/Users/Shadow/notepad/.build/ (native-windows-x64/{app, app-lockstep,
app-lockstep-sanitize, sdk-lockstep, picker-lockstep, ...}), and the ignored
gui_forms/ and template_harness/ checkouts (8e288666 and 523c2a7). All of
them can be regenerated with prepare_dependencies.py. The working tree is
clean. Keep the D:/gui_forms_swiftedit leftovers (stx_cng/ and 6 merged RT
branches), per the earlier decision.

Local recipe: see "Session 2 state" (lockstep plus pwsh 7 on PATH; add
--sanitize for the sanitized tree).

Still open for P3/P4 after the merges: confirm the first arm64 release
assets; the first release with the vendor identity; moving the notes docs
out of the product (AGENTS.md and README links).
