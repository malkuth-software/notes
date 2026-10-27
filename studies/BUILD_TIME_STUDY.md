# Build-time study, 2026-10-20 (worker w1)

Why: the owner said most time is spent waiting on sanitizer builds. This
measures the local and CI builds, finds the repeated work, and lists options.
Everything here was measured unless marked *estimate*. Raw logs, the load
samples and the scripts are in the w1 session scratchpad (paths at the end).

## Conditions

- Machine: AMD EPYC 9354, 4 cores / 8 threads, 16 GB, Windows 11; LLVM 22.1.8
  (MSYS2 CLANG64), ccache 4.14.1, one shared cache `D:/gui_forms_main/.ccache`
  (`max_size` 4 GB).
- Source: `origin/main` at `1c58f24e` (tree `0eacf5ff`), in `D:\gui_forms_w1`.
- **Contention**: CPU was at 100% for nearly every sample (8-second sampler).
  w2 and w3 were building and testing their sanitizer trees throughout, and a
  non-GUI.Forms project (`plan-paint`) was building too. Mean clang processes
  machine-wide: 12.1 during the sanitizer catch-up, 6.9 during the ordinary
  catch-up. Two runs were repeated when the load differed (marked).
- Hit counts come from ccache's `stats_log` for my run only, so other
  workspaces' compilations do not mix in and the shared counters were not
  zeroed (zeroing them would erase the other workers' statistics).
- The Gallery-wide accessibility tests (`accessibility.*gallery`) were
  excluded from every ctest run (`-E`), because another workspace may run
  them. Their sanitized times below come from their CMake comments
  (measured 2026-10-20 by another worker).

## Local measurements

| # | Workload | Wall | Compiles hit / missed | Notes |
|---|---|---:|---:|---|
| L1 | Ordinary tree, catch-up from an older branch to main | 286 s | 485 / 14 | 194 executable links summed 403 s; the ten lab/Gallery programs take 14 to 22 s each to link |
| L2 | Ordinary tree, one `.cpp` changed in `src/controls` | 65 s | 0 / 1 | 1 compile (1.7 s) + 120 executable relinks (373 s summed): 98% link |
| L3 | **Sanitizer tree, same catch-up as L1** | **2,102 s** | **1 / 498** | compiles summed 11,552 s (23 s per TU), links 609 s; 696 steps |
| L4 | Sanitizer tree, one `.cpp` changed, `sanitize.sh 'tool_strip'` | 202 s | 0 / 1 | fetch 9 s + configure 29 s + build 158 s (1 compile + 125 relinks) + 2 tests |
| L5 | Sanitizer tree, nothing changed but one file's timestamp, `sanitize.sh 'tool_strip'` | 331 s | 1 / 0 | fetch 75 s, configure 24 s, 125 relinks 226 s |
| L6 | Ordinary suite, serial (`-j1`) | 370 s | | 177 tests; under load 11.2 clang |
| L7 | Ordinary suite, `-j4` | 79 s | | |
| L8 | Sanitized suite, serial | 460 s, repeated: **186 s** | | 177 tests; first run under heavier load (8.6 vs 4.1 clang) |
| L9 | Sanitized suite, `-j4` | **67 s** | | load 2.9 clang |
| L10 | The two Gallery accessibility tests, sanitized | 340 s + 741 s | | from CMakeLists comments; serial ctest adds both |
| L11 | Fresh sanitizer tree, first configure | 218 s | | compiler checks, FetchContent, HarfBuzz/FreeType configure |

Sizes: a sanitizer tree is 11 to 13 GB in every workspace (174 executables
of about 52 MB, 8.97 GB in w1; objects 0.97 GB); the ordinary tree is about
0.9 GB. The shared ccache is full (4.0 of 4.0 GB, 4,114 cleanups, lifetime
hit rate 28%).

Slowest sanitized tests (serial, lighter-load repeat; first-run value in
brackets where the heavier load changed it):

| Test | s |
|---|---:|
| gallery_pages_tests | 34.4 [30.5] |
| engine_part_table_tests | 16.8 [30.0] |
| data_grid_view_tests | 14.2 |
| windows_gdi_compat_shim_tests | 11.0 [87.9] |
| multiline_text_box_tests | 10.1 |
| windows_canvas_raster_tests | 9.1 [56.5] |
| owned_event_compile_failures | 7.4 [19.6] |
| dock_layout_tests | 3.9 |
| collection_controls_tests | 3.3 |
| prepared_window_shape_rows_verify | 3.0 |
| host_boundary_audit | 2.7 [8.2] |
| data_grid_view_order_tests | 2.7 |
| windows_text_tests | 2.6 [8.1] |
| audio_tests | 2.3 |
| inspection_controls_tests | 2.2 |
| (excluded) windows_accessibility_uia_gallery_tests | about 741 |
| (excluded) windows_accessibility_msaa_gallery_tests | about 340 |

The Windows host tests swing 5x to 8x with machine load
(`gdi_compat_shim` 11 s to 88 s); their 900 s budget still holds.

### Why the sanitizer tree never hits the cache across workspaces

The sanitized build type is RelWithDebInfo (`-O2 -g`). With `-g`, ccache adds
the build directory to every object's hash (`hash_dir`), and each workspace's
build directory is different, so **no sanitized object compiled in one
workspace is ever reused in another**, even for byte-identical sources.
Probe (private cache, one TU, two workspaces with the CI layout):

| Flags | second workspace |
|---|---|
| CI sanitizer flags | miss |
| `CCACHE_NOHASHDIR=1` | hit (but debug info names the first workspace) |
| `-fdebug-prefix-map=<ws>=.` | miss (the absolute path is in the option) |
| `-fdebug-compilation-dir=.` or `-ffile-compilation-dir=.` | **hit**; DWARF `comp_dir` is `.` |

Ordinary (Release, no `-g`) objects do hit across workspaces (L1: 485 of
499), which is why the ordinary catch-up took 5 minutes and the sanitizer
catch-up 35.

### What `ci_build.py --sanitize --tests REGEX` builds

Everything. `cmake --build` has no target, so a run of two tests builds all
174 sanitized programs (L4: 125 relinks for one changed file, 2 needed).
`ninja -n` for the same change: 130 steps for all, 4 for the two
tool_strip test executables.

### How ctest runs

- `ci_build.py` (and so `sanitize.sh` and every CI job): serial, `--timeout`
  120 (900 sanitized).
- `build.sh all`: `-j 3`, `--timeout 300`; `build.sh <regex>`: serial.
- Concurrency limits: no test has `RUN_SERIAL` or `RESOURCE_LOCK`. The two
  Gallery accessibility tests find their window with `FindWindowW` by a
  title unique to each test, so they do not collide with each other inside
  one ctest run; they collide only with the same test running in another
  workspace (the fix another worker is making).

### Overhead per invocation

- `fetch_text_stack.sh`: 9 to 75 s per run, all of it `git checkout --detach`
  of the commit already checked out (8 s for HarfBuzz alone when quiet;
  `rev-parse` takes 0.8 s).
- Configure: 21 to 29 s on every run of an existing tree, though nothing
  changed.
- Docs: not part of any build or test target (`gui_forms_api_reference` is
  not in ALL; `generate_library_docs.py` is run by hand); nothing repeats.

### Critical path, local

A narrowed sanitizer check after a one-file change (L4) was 202 s, of which
the two tests took about 3 s; the rest was setup (38 s) and relinking 123
programs nobody asked for. After any upstream change (a restack, a merge of
main) every workspace's sanitizer tree recompiles from scratch: 35 minutes
at 100% CPU (L3), four times over, once per workspace, for the same sources.

## CI measurements

20 runs (10 `source and compile`, 10 `programs and release`; 4 main pushes and
6 pull requests each), 2026-10-20 20:11 to 23:49 UTC. Job API timestamps.

| Job | Event | Queue median (max) | Duration median (min to max) |
|---|---|---:|---:|
| Sanitizers (Linux) | main | 463 s (1,061) | 1,130 s (206 to 1,246) |
| Sanitizers (Linux) | PR | 92 s (1,237) | 1,086 s (177 to 1,329) |
| native windows-x64 | main | 356 s | 147 s (70 to 633) |
| native windows-x64 | PR | 123 s | 694 s (207 to 1,163) |
| native linux-x64 | PR | 81 s | 628 s (117 to 1,317) |
| native macos-arm64 | PR | 64 s | 867 s (408 to 1,326) |
| programs linux-x64 | PR | 3 s | 1,371 s (1,013 to 1,454) |
| programs windows-x64 | PR | 2 s | 1,090 s (260 to 1,220) |
| programs macos-arm64 | PR | 4 s | 1,052 s (213 to 1,196) |
| house-style / docs | PR | 2 s | 42 s / 100 s |

Run wall times (creation to last job): 9 to 57 minutes; typically 20 to 25
minutes per workflow per push when caches miss.

Findings:

- **Sanitizer job**: build 700 to 1,250 s, tests 36 to 48 s. The cache always
  restores, but from an older tree: a PR run can see only main's caches (and
  its own), and a main push only main's, so the PR that tested the very tree
  being merged is invisible. Every main push rebuilds the sanitizer tree for
  15 to 20 minutes (main runs: 1,211, 1,004, 1,179, 161 s of building). The
  job prints no ccache statistics.
- **Native**: main pushes are cheap when the tree equals a tested PR
  (promotion; Windows 70 to 78 s). Fresh PR builds hit 46 to 66% (Linux) and
  spend 160 to 350 s more in the development text-mask step, which
  reconfigures the same tree with `GUI_FORMS_PREPARED_TEXT` (a PUBLIC define on
  `gui_forms_core`), recompiling the core for those targets.
  `Reuse identical successful native validation` sometimes waits 280 to 604 s
  for an older identical run still in progress (by design).
- **Programs duplicates native**: it builds the whole toolkit and all tests
  again (with Gallery and demoboard ON) and runs the full ctest again
  (65 to 110 s on Windows and macOS). Its ccache hit rate is 39 to 47% in 9 of
  10 Linux runs (267 of 680 in seven of them): it restores native's older
  object cache, because its own entries are gone.
- **The Actions cache is at its 10 GB limit and thrashing**: 27 entries,
  10.98 GB, the oldest 5 hours old; main holds three entries. Per run the
  sanitizer job saves a new 583 MB entry (key includes `github.sha`) and each
  native platform about 550 MB. Evictions remove the dependency caches and
  the programs object caches, so later runs miss (seen: `gui-deps-v3-linux-x64`
  not found).
- **Queue waits**: up to 31 minutes (programs windows, main); medians 1.5 to 8
  minutes on main, when several PR pushes and a merge arrive together. Each
  push starts about 16 jobs (7 native workflow + 6 programs workflow + 3).
- **Critical path**, PR: house-style (40 s) then the slowest of sanitizers,
  native macOS and programs Linux, each 18 to 24 minutes on a cache miss.
  Main: sanitizers (15 to 21 min) and programs (12 to 23 min); native is
  short through promotion.

## Rework across the developer loop

1. Worker runs `build.sh all` (ordinary, hits across workspaces), then
   `sanitize.sh` (sanitized, never hits across workspaces): the same sources
   are compiled under ASan once per workspace: 4 x 35 min of CPU per main
   change.
2. `sanitize.sh 'regex'` relinks every program (up to 125 x 52 MB) to run a
   few tests, and repeats a 30 to 100 s fetch and configure each time.
3. CI then recompiles the sanitized tree on the PR (15 to 20 min) and again on
   main after merge (15 to 20 min), because main cannot see the PR's cache.
4. CI builds the toolkit and tests twice per platform (native and programs)
   and runs the suite twice.
5. CI caches evict each other, so later runs of 1 to 4 start colder.

## Options

Savings are from the measurements above; "per event" says when it applies.

1. **Share sanitized objects across workspaces** (`-fdebug-compilation-dir=.`
   in `SANITIZER_FLAGS`). Saving: the second to fourth workspace's catch-up
   after a main change goes from about 35 min with 0.2% hits toward the
   ordinary tree's profile (L1: 5 min with 97% hits); *after-measurement in
   progress, see PR*. Cost: one flag; one full sanitized rebuild per tree
   (flags change). Risk: low; reports name files relative to the build
   directory. Does not change what is tested. **Recommended; implemented
   (PR "Share sanitized objects across checkouts").**
2. **Build only what `--tests` needs.** Saving: L4 202 s to 52 s (build phase
   158 s to 29 s) per narrowed sanitizer run after a library change. Risk: a
   test reading a build product not named on its command line; mitigated by
   falling back to a full build for unknown outputs, fixtures and
   dependencies, and the full suite is unchanged. **Recommended; #217.**
3. **Parallel sanitized ctest.** Saving: L8/L9 186 s to 67 s for the suite
   without the Gallery tests; with them about 21 min serial to about 12.5 min
   (*estimate*: bounded by the 741 s UIA Gallery test). Risk: timing-sensitive
   host tests under heavier load; `build.sh all` already runs the ordinary
   suite at `-j 3` without failures. **Recommended; #219 adds `--test-jobs`
   (default 1); the coordinator's `sanitize.sh` needs `--test-jobs 4` to use
   it.**
4. **Skip the fetch and configure when nothing changed.** Saving: 38 to 99 s
   per `sanitize.sh` call (to about 2 s). Risk: low; any change to the cache,
   the command or the compiler environment configures again, and ninja's
   regeneration still runs. **Recommended; #218.**
5. **Raise the shared ccache to 16 GB** (`CCACHE_MAXSIZE` in each workspace's
   `env.sh`, coordinator's files). Saving: not measured directly; the cache
   is full with 4,114 cleanups, and four sanitizer trees of about 1 GB of
   objects each plus ordinary trees and branch states do not fit in 4 GB.
   Disk has 470 GB free. Risk: none. **Recommended (coordinator).**
6. **CI: sanitizer cache across the PR-to-main boundary.** Save the sanitized
   `.ccache` as an artifact and seed main (and stacked PRs) from the newest
   one with the same compiler identity, like native's `ci_promote.py seed`;
   every test still runs. Saving: main sanitizer job 15 to 20 min to about
   3 min (the 161 to 206 s runs with a good cache). Cost: a ~600 MB artifact
   per run, a seed step. Risk: low for the gate. **Recommended; owner, not
   started.**
7. **CI: stop the cache thrash.** Save the sanitizer cache only on main (PRs
   restore main's), and do not save native object caches from PRs, which
   artifacts already cover. Saving: keeps dependency and programs caches
   alive (programs hit 39%; Linux deps were evicted). Risk: a PR's second push
   restores main's cache instead of its own first push's. **Owner decision.**
8. **CI: exact-tree reuse for the sanitizer job** (skip when the identical
   tree, compiler and environment already passed, as native does). Saving:
   15 to 20 min per main merge of a tested tree. Risk: **it changes the gate**
   (a result is reused instead of rerun). **Owner decision.**
9. **CI: merge the programs and native builds** (native builds with the
   Gallery ON and programs packages from it, or programs seeds from native's
   archive). Saving: up to 18 to 24 min of runner time per platform per push
   and one suite run. Risk: restructures two workflows; the programs ctest
   is the only CI run of Gallery-dependent tests. **Owner decision.**
10. **Parallel ctest in CI** (`--test-jobs 4`). Saving: about 30 s on the
    Linux sanitizer job, less elsewhere. Risk: flake exposure on 4-core
    runners. **Not recommended now.**
11. **Gallery accessibility tests in their own sanitizer label or job.**
    They are 1,081 of about 1,270 sanitized test-seconds locally. With 3 they
    overlap the rest; a label (`-L`/`-LE`) would let narrowed runs and other
    workspaces skip them cleanly. Risk: none if CI still runs them. **Worth
    doing after the other worker's window fix; not started.**
12. **Smaller sanitizer debug info** (`-gline-tables-only` instead of `-g`).
    Links dominate small edits (L2, L4) and the programs are 52 MB each;
    line tables keep function names and file:line in reports. Not measured.
    Risk: debugger sessions lose variables. **Owner decision; worth a probe.**
13. **Precompiled headers / unity builds.** Heaviest sanitized TUs take 50 to
    119 s (`doc_examples_tests` 119 s, `anatomy_tests` 75 s, `c_api` 74 s).
    Not measured; high effort; unity builds change ODR exposure. **Not
    recommended now.**
14. **Per-test timeouts tuned to measured times.** They cost time only on a
    hang; measured passes are far inside the budgets. **Not recommended.**
15. **Skip docs regeneration when no header changed.** No build or test step
    regenerates docs. **Not applicable.**

## Phase 2 status

Implemented as stacked PRs (each with before/after numbers): option 1
(`claude/sanitizer-shared-cache`), 2 (#217), 4 (#218), 3 (#219, with the
contributing note). Options 5 to 9 and 12 are for the coordinator or the
owner.

## Status (2026-10-22, w1)

- #226 (option 1, plus a CMake fix): ready. The first after-measurement
  failed because HarfBuzz's CMakeLists sets THREADS_PREFER_PTHREAD_FLAG
  after GUI.Forms' own Threads lookup, so a tree's first configure lacked
  -pthread and every reconfigure added it (0 to 597 lines on the same tree).
  Fixed by preferring the flag before the lookup. Private-cache pair on the
  tool_strip test program: 0/267 hits in the first checkout (728 s), then
  267/267 in the second (136 s).
- #217, #218, #219: rebased on main; CI green apart from superseded cancelled
  runs, which are being rerun.
- #220 (options 6+7): ready. Second push seeded from the first: sanitizer
  609/609 hits, 150 s (the first push took 1,307 s at 47%).
- #222 (option 9): programs builds at 87% hits, 316 to 550 s per platform;
  combined run 18.4 min wall and 50.6 runner minutes (median before: 25 min,
  122).
- #227 (font staging): ready; three clean -j16 builds and 183/183 tests.
- #232 state-machine hunter and first report (F1 52, F2 9, C1 49, C2 50,
  C3 15, compliant 99).
- #234 option 12 (line tables by default, `--debug-info full`,
  `[sanitizer-full-g]`). Measured: program 59.2 to 17.8 MB, libraries 335 to
  138 MB, compile -28%. macOS LSan baseline still to be checked on a Mac.
- Ordinary cold configure-and-build in a fresh checkout with the shared
  cache: 212 s configure + 387 s build, 698/700 hits (after a second
  configure for -pthread, until #226 lands).

## Raw data

Scratchpad `C:\Users\Shadow\AppData\Local\Temp\claude\C--Users-Shadow-gui-forms\005dc99f-1086-463f-978e-5723b295c49c\scratchpad\`:
`results.tsv` (every timed run), `load.csv` (load samples, UTC), `logs\`
(per-run output and ccache stats logs), `ci\` (job JSON and filtered logs),
`cifull\` (full sanitizer job logs), and the scripts `timed.sh`, `san.sh`,
`suite.sh`, `cwd_hash_probe*.sh`, `ninja_log.py`, `ci_jobs.py`,
`ci_summary.py`.
