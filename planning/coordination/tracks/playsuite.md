# Track: PlaySuite (malkuth-software/games)

Product worker, started 2026-10-27 under `D:/malkuth/projects/briefs/PRODUCT_WORKER_BRIEF.md`
(the standing brief). Reports to the coordinator. No agents; no pushes to main; never push
old history (check `git merge-base --is-ancestor bc229b8 HEAD` before every push).

## Repository (2026-10-27)
- GitHub `malkuth-software/games`, public, PolyForm Noncommercial 1.0.0; main = one root
  commit `bc229b8` ("Initial commit", 2026-10-26). No other branches, no tags, no releases,
  no Actions runs. Workflows: applications.yml (installers; publishes a release on every push
  to main), native-core.yml (PRs only, 4 hosts), native-text.yml, runtime-assets.yml,
  readme.yml (regenerates README and pushes to main itself).
- Old history is still reachable on GitHub: the 30 old PRs (#1 to #30, all merged/closed) and
  their `refs/pull/N/head` refs point at old commits. Only GitHub Support can purge those.
- Version: `GAMES_BASE_VERSION 0.5.0`; main publishes `0.5.(run_number)`. The last old release
  was v0.5.91; run numbers restarted with the history, so the next release would be 0.5.1 or
  so, lower than what users have installed.

## Local checkouts (old history; keep until the coordinator says)
- `C:/Users/Shadow/games`: main at c5ea5a6 (old), clean. Remote URL still
  falseywinchnet/games (redirects). Branch codex/cross-platform-cleanup (in old main). Stale
  remote-tracking refs (codex-handoff/stx-adoption, shell-gui-paradigms,
  codex/rock-stack-cache-fix: all in old main; trial/windows-overlays ceb3bd2: one trial commit
  "not for main"). 110 local tags v0.x (all in old main): NEVER push tags.
  Ignored data: `astra/` (1.8 GB, Astra's 2026-10-01..04 records and screenshots), `.build/`
  (2.8 GB; old dev builds per memory shadow-dev-environment.md).
- `D:/playsuite/games`: a worktree of the above, detached at 8f0d489 (old main, 2026-10-13),
  one uncommitted line in shared/render/ui/overlays.hpp (refresh_overlays off on Windows, the
  same trial). Beside it: before/after/regions/stillwater JSON captures, trace/, .build/fetch,
  .ccache, runtime-artifact (120 MB). Data only.
- Inventory result: the bc229b8 tree equals old main c5ea5a6 except LICENSE. Every branch,
  remote-tracking ref and tag is contained in old main, except the trial (ceb3bd2 and the
  worktree's one-line edit), which is superseded: old main 727525b removed overlays.hpp
  entirely. **Nothing to replay.**
- Prior notes: memory games-ui-rework.md and shadow-dev-environment.md (2026-10-02). The only
  transcript (b3077e57) covers 2026-10-02 only (round 2, committed as 0554156, in old main).
  Later work (to 2026-10-20) was done elsewhere: AGENTS.md names the M4 Mini checkout
  `/Users/ultimussecundai/Developer/playsuite-games` as authoritative and "Astra" sessions as
  release owners. Local-only work there cannot be seen from this machine.

## GUI.Forms changes vs PlaySuite (item 2)
Pin: GUI.Forms 6fd0071a (2026-10-13); main e077f9d5 is 1282 commits later (include/: 163 files,
+23.7k/-74, mostly additive). Latest SDK v0.2.1308; cache releases build-<sha>.
- One pixel format (BGRA8 premultiplied sRGB, RGBA input-only, stage SK2, not yet landed for
  live surfaces: `LiveSurfacePixelFormat::rgba32_premultiplied_srgb` and
  `FramebufferChannelOrder::rgba` still exist on main). PlaySuite loads images as BGRA and
  follows `native_live_surface_pixel_format()`, but names the RGBA enumerators explicitly in
  games/cube/src/cube_view.cpp:277, shared/puzzles/puzzle_view.cpp:127-128,308,354,775,
  shared/render/ui/surface.cpp:35, src/main.cpp:62, tests/puzzle_native_framebuffer_tests.cpp,
  and keeps its own `render::Order::rgba` paths. When SK2 removes the enumerators these stop
  compiling; the rgba branches then become dead and go (BGRA only).
- CNG removed: PlaySuite uses no CNG/JPEG/WebP at runtime (PNG via GUI.Forms; Koi-Koi's webp
  is an authoring-only input to tools/make_backs.py). No change.
- Skia mandatory: CMakeLists.txt forces `GUI_FORMS_ENABLE_SKIA OFF` on WIN32 (lines 49-50).
  Must go; Windows then needs the CPU Skia archive (prebuilt from GUI.Forms releases; the
  Skia port is paused while the owner builds stx_skia; check availability in P2).
  tools/package_common.py copies Skia's libpng/zlib/freetype notices (Skia is now codec-free:
  check those paths in P2).
- libunibreak: already fetched (fetch_linebreak.sh) and its notice packaged. No change.
- Harness: P1.
- Other drift: workflows and docs name falseywinchnet/gui_forms and falseywinchnet/books.
  GUI.Forms options PlaySuite sets all still exist.

## Owner decisions in force (2026-10-27, via the coordinator)
1. Releases: keep publish-on-green-main; GAMES_BASE_VERSION 0.6.0 before the first release (done in #31).
2. readme.yml: a PR check (make_readme.py --check), never a push to main (done in #31).
3. Branch protection + auto-merge: the owner applies them once I send PlaySuite's required check
   names. Until then: NO `--auto`; `gh pr merge N --merge` only after every check on the final
   head is green.
4. AGENTS.md: the PR rule replaces "proven on the M4, straight to main" (done in #31).
5. M4 checkout: the owner's local M4 worker reports local-only work; the coordinator relays it.
6. Old PR refs: no Support request (owner not worried).
7. Windows CPU Skia archive: in gui_forms' build releases (3.13 MB, unpinned; SK port paused).
   Check availability in P2 and report rather than block.

## P1 state (2026-10-27)
- Workspace `D:/malkuth/workspaces/games/` = fresh clone of malkuth-software/games (new history
  only; core.autocrlf=false locally). Inside, ignored: `template_harness/` (my clone, branch
  `games-project`), `notes/` (clone of malkuth-software/notes) with worktree `notes-games/`
  (local orphan branch `games`, commit 2751ec5), `.build/`, `.ccache/`.
- Harness branch `games-project` @ 08a1865 (base f73a6f9): projects/games.toml +
  harness/test_project_games.py. Whole harness suite 295 OK (1 skipped). WAITING: the
  coordinator pushes it to template_harness.
  Names: name games, source_dir ".", build_name games, env_prefix GAMES, artifact_prefix games-,
  no cache_asset_prefix, native_workflow applications.yml / native_job "application" (prefix
  form; the linux-x64 entry is "linux-amd64" today, so it won't match until P3 renames it),
  toolchain = GUI.Forms', Linux profiles core/application/packages/assets, bootstrap smoke
  tools/harness_smoke.py, notes_branch games.
- Notes branch `games` (local, 2751ec5): README, records/ (= product evidence/), proposals/
  (ACCESSIBILITY_TOOLKIT_REQUEST.md, GUI_FORMS_CONTROLS_PROPOSAL.md). WAITING: the coordinator
  creates the branch on malkuth-software/notes. Then a product PR removes evidence/ and the
  two proposals from games. TOOLKIT_REQUESTS.md stays (AGENTS.md rules point to it).
- PR #31 `claude/pr-rule-and-release-baseline` (bde9790): AGENTS PR rule, 0.6.0, readme.yml
  check, applications.yml on every PR, .gitattributes LF, .gitignore harness dirs.
  First CI on the new root: native-core Windows FAILED: GUI.Forms' build-6fd0071a release
  (Windows toolchain + compiler caches) no longer exists, so setup-msys2 installed Clang 23.1.4,
  whose headers dropped std::abs via <algorithm> (games/cube/src/generator.cpp). Fixed by
  3c715af (#31 head). Locally the whole portable core builds with the pinned LLVM 22.1.8.
  Expect the application job to compile GUI.Forms 6fd0071a from source (no cache, Clang 23 on
  Windows, macOS runtime from source): P2's repin is what really fixes CI.
- PR #32 draft `claude/harness-adoption` (922a37c, base #31, restacked on 3c715af): harness.lock -> 08a1865,
  tools/harness_smoke.py, AGENTS.md layout. Smoke passed locally (Clang 22.1.8, storage 1/1)
  with --define FETCHCONTENT_SOURCE_DIR_QUICKJS=D:/playsuite/.build/fetch/quickjs-src.
  Undraft only after the harness commit is on GitHub (re-pin with lock.py bump if its SHA
  differs).
- Local toolchain: D:/a/_temp/msys64/clang64/bin (the pinned GUI.Forms CLANG64, read only);
  CCACHE_DIR/BASEDIR = the workspace.
- Check names seen on #31: "native (windows-2022, windows-x64, msys2 {0})",
  "native (macos-15, macos-arm64, bash)", "native (ubuntu-24.04, linux-x64, bash)",
  "native (ubuntu-24.04-arm, linux-arm64, bash)", "readme", "prepare", and the application
  jobs "application (<runner>, <platform>, <shell>)" x4 (names to confirm from the run).

## Plan
- P0 intake: DONE 2026-10-27.
- P1: in progress (above). Remaining: harness push, notes branch creation, #31 green and merged,
  #32 re-pinned/undrafted and merged, bootstrap gate on a clean directory.
- P2: source SDK v0.2.1308+ (or pinned submodule) plus build-<sha> cache, digest-checked;
  Skia on Windows (report archive availability); API drift; drop RGBA branches once SK2 lands;
  repoint falseywinchnet/* names.
- P3: CI on 4 hosts through the harness; rename the linux-amd64 matrix entry to linux-x64;
  test_reuse.py with no releases; an always-reporting summary check for protection.
- P4: release workflow publishes installers/archives from CI.

## Lockstep rule (owner, 2026-10-27; PHASE_D_DESIGN.md "Local lockstep before cloud pins")
- Local builds will use the one lockstep GUI.Forms (D:/malkuth/gui_forms_lockstep, installed to
  D:/malkuth/cache/lockstep/, shared ccache D:/malkuth/cache/ccache). Until W's harness/lockstep.py
  lands: port drift against e077f9d5 (v0.2.1308): the source SDK from D:/malkuth/cache/gui_forms/
  v0.2.1308, extracted to <ws>/.build/sdk/gui-forms-sdk-0.2.1308 (verified sha256).
- Cloud CI pins a pushed GUI.Forms commit: e077f9d5 (build-e077f9d5 has toolchain, caches,
  Windows Skia). Source SDK in the cloud comes later.
- Downloads only via the coordinator into D:/malkuth/cache/. A product change needing a GUI.Forms
  change waits until that change is merged.
- One always-reporting summary check is added in P3; protection after P3.
- At e077f9d5 GUI.Forms itself builds Windows without Skia (SDK manifest), so PlaySuite keeps
  GUI_FORMS_ENABLE_SKIA OFF on WIN32 for this pin.

## State (2026-10-27, P2 local lockstep)
- Merged on main: #31 (63ab62d: decisions, IWYU, repin e077f9d5, Windows system DLLs) and #32
  (9fd93cb: harness.lock 4437b47, tools/harness_smoke.py). Merge rule: `gh pr merge N --merge` only
  when every check on the final head is green; no --auto until protection exists.
- #33 `claude/notes-move`: update-branch onto 9fd93cb done server-side (head 402cb21); merge when green.
- #34 `claude/lockstep-local-build` (0d0194d): with GAMES_TOOLKIT_SOURCE_DIR empty, find_package
  GUIForms (Application Audio TextMasks AudioLoopTransport), fonts from GUIForms_FONT_DIR, Windows
  copies GUIForms::Threading's DLL too; docs/BUILDING.md "Local builds". CI keeps the source path.
  Merge when green.
- Local build (no GUI.Forms source): record D:/malkuth/cache/lockstep/current-windows-x64.json
  (8e288666, --features all); toolchain <ws>/template_harness/harness/cmake/llvm22.cmake;
  PATH /d/a/_temp/msys64/clang64/bin first; CCACHE_DIR D:/malkuth/cache/ccache; LANG en_US.UTF-8;
  quickjs FETCHCONTENT_SOURCE_DIR_QUICKJS=D:/playsuite/.build/fetch/quickjs-src; runtime
  <ws>/.build/runtime. Tree <ws>/.build/native-windows-x64/games-lockstep: builds, ctest -LE design
  52/52 (GAMES_STATE_DIR=D:/malkuth/workspaces/state). The older tree .build/native-windows-x64/games
  is the source-built e077f9d5 one.
- No sanitizer profile exists for PlaySuite yet (P3 item). Design tests not run (includes only).
- Worktree D:/malkuth/workspaces/games-pr32 (detached) for branch edits; local branch
  claude/p2-gui-forms-e077f9d is obsolete.

## P3 (2026-10-27, in progress)
- #35 `claude/p3-harness-ci` (base main): own .github/actions/harness-checkout (root layout,
  HARNESS_PROJECT=projects/games.toml), harness.lock -> 8868f87, windows_toolchain.py restore
  (copied restore deleted, no setup-msys2 fallback), Linux install_linux_ci_packages.sh profiles
  (packages/core/assets), compiler_identity.py, harness check_house_style.py (scripts/check-style.py
  = contributors' copy, same rules), macOS runtime tools from harness, app tree
  .build/native-<platform>/games, matrix linux-amd64 -> linux-x64 (asset names keep Debian amd64).
- `claude/p3-ci-summary` (stacked on #35): applications.yml holds every PR check; readme/native-core/
  native-text/runtime-assets become workflow_call; tools/ci_changes.py replaces the text/audio path
  filters (merge-base diff; no base = run all); job "Sanitizers (Linux, ASan + UBSan + LSan)" runs
  tools/sanitize_core.py (harness sanitizer_options, tree games-core-sanitize, lsan.supp empty);
  job "PlaySuite CI summary" (if: always(), needs all, fails on failure/cancelled). Release collects
  core results from applications.yml PR runs. runtime-assets artifact renamed audio-runtime-data
  (release downloads games-*).
- test_reuse.py with no releases: fetch says "No release carries", collect "none yet", plan runs all
  (inspected; #31/#32 CI ran green with no releases).

## Next step
- Coordinator's watcher merges #35, retargets #36 to main and merges it when green; don't wait on that.
- #37 `claude/music-64bit-clocks` (base main, head 9157949): std::int64_t sample positions (Band clock/
  plan/due, Note::at, harp, koto, lounge/marimba/song next_bar_), plus a test band crossing 2^31
  samples (with 32-bit clocks it hangs). Release core 25/25. #36 changes the same band.cpp line
  (`numeric_limits<long>`); whichever lands second resolves it to `numeric_limits<std::int64_t>`
  (merge main locally into #37 if #36 lands first).
- P4: there is no new code. Every non-docs push to main publishes v0.6.<run number>
  (publication_version.py) through the release job (publish_tested_release.py: 8 packages, SHA256SUMS,
  test results). Main push runs cancel each other (concurrency group applications-main), so only
  the newest merge publishes. Run #297 (41c516f, pre-#35) was in flight on 2026-10-27; it publishes
  0.6.297 unless #35's merge cancels it. To verify the release: `gh release view v0.6.N` has
  8 packages, SHA256SUMS.txt and test-results-*.json; then report it.

## Vendor identity and bootstrap (2026-10-27)
- Owner: vendor "Malkuth Software" for every product. #38 `claude/vendor-identity` (base main, head
  d2feab3). tools/package_common.py holds VENDOR, BUNDLE_IDENTIFIER com.malkuthsoft.playsuite,
  LINUX_PACKAGE malkuth-playsuite, and the legacy names.
  - Windows upgrades stay in place: the RainstarGames key is kept, and InstallDirRegKey plus .onInit
    use 0.5.x's %LOCALAPPDATA%\Rainstar\Games. New installs go to Malkuth Software\PlaySuite.
  - Linux: Conflicts/Replaces/Provides rainstar-games, plus a /usr/bin/rainstar-games symlink.
  - Save folders are unchanged.
  - CI: tools/test_windows_installer.py upgrades a fake 0.5.x install; tools/test_deb_upgrade.py
    installs over a stand-in deb.
  - Never run the installer locally (it writes the real HKCU entry).
  - #38 touches applications.yml (the Linux install step). If it conflicts after #35/#36 land,
    merge main locally.
- Bootstrap CI: template_harness commit eeaf0e0 (branch games-bootstrap-profile in
  D:/malkuth/workspaces/th_games_bootstrap) adds `[bootstrap] linux_packages = "core"` and a test.
  The coordinator pushes it.
- Still to do:
  - #37: after #36 merges, merge main, set band.cpp to numeric_limits<std::int64_t>, and merge #37
    when green.
  - P4: report the first v0.6.N; none was published as of #38's opening.
- Update (2026-10-27):
  - #38 head ced0f4e now sets Maintainer "Malkuth Software <maintainer@malkuthsoft.com>" (owner).
  - #36 head 7f5b519: cancel-in-progress only for pull_request, so main release runs finish.
  - The coordinator is pushing harness eeaf0e0.
