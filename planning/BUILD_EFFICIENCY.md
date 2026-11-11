# Building GUI.Forms efficiently

Status: practice guide, 2026-11-10. For anyone building GUI.Forms or a
product on it, and for the agents that do. The numbers come from the churn
study of one SDR# swap-in worker over 48 hours (26.3 active hours, 3,108 tool
calls, 177 builds, tests and packages; coordinator desk
`CHURN_STUDY_SDRSHARP_2026-11-10.md`) and from the coordinator's own
measurements the same week.

The short version: most of the lost time was not compiling. It was fresh
build trees, waiting, rebuilding for each small fix, and machines
oversubscribed by parallel builds.

## 1. Build reuse

- **Keep one build tree per base and reuse it.** Nine fresh-tree or
  new-worktree builds took 262 of 349 cmake minutes (75%). The other 44
  library builds were incremental, at a median of about 60 s. The same
  branch was built privately three times for three successive heads.
- **Hand over a built SDK, not an unbuilt head.** A consumer (a product, a
  facade, a packager) builds against an installed prefix. The integrator
  builds library heads once; nobody else rebuilds them.
- **One shared compiler cache with one size cap.** Set `max_size` once in
  the cache's `ccache.conf`. A per-shell `CCACHE_MAXSIZE` (one environment
  file set 500M) trims the shared cache for everyone.
- **ccache settings that matter:**
  - `CCACHE_SLOPPINESS=pch_defines,time_macros` wherever a precompiled
    header is used. Without it ccache refused 596 compiles ("could not use
    precompiled header"), so every PCH test object recompiled every time.
  - `CCACHE_BASEDIR` at the common root of all workspaces, and
    `CCACHE_COMPILERCHECK=content`. BASEDIR only rewrites paths on the
    compiler's own path arguments; it does not rewrite absolute paths inside
    other flags. A clang plugin argument such as
    `-fplugin-arg-...-root=<absolute source dir>` must be given relative to
    the build directory, or those objects never hit across worktrees.
  - The compiler launchers (`CMAKE_C/CXX/OBJCXX_COMPILER_LAUNCHER=ccache`)
    in the build environment. CI sets them in its environment, so a local
    tree without them caches nothing.
- **Reproducible links.** Anything stamped at link time (a timestamp in a
  PE header, a build date baked into a binary) changes every output hash
  and defeats downstream reuse and comparison. Link reproducibly.
- **Result:** with the sloppiness fix, relative plugin roots, reproducible
  links and one shared cache, a touch-everything rebuild went from 2,751 s
  to 982 s. Most of what remains is relinking 366 static test programs.
- **One install prefix per options set.** A shared stx package was
  reconfigured twice with different options (Skia off), which broke every
  other tree that used it (a missing md4c object until a reconfigure).
  Give each options set its own prefix, or have the package refuse a
  configure whose options differ from the recorded ones.

## 2. Parallelism and CPU budget

- When every worker built at once there were 28 clang processes on 8 cores
  (33 compiler and build processes in the study at one sample). Builds
  slowed, and one failed under load with exit 4294967295 and had to be
  rerun (18 minutes).
- **Rule:** one priority builder runs at full `-j`; every other build runs
  at `-j2` unless told otherwise. Never let every worker build at once.
- Don't answer contention by turning off incremental builds. The packager's
  `dotnet build -m:1 --no-incremental` made six zip builds cost 60 minutes.
  Use `-m:1` only while the machine is loaded, and keep incremental on.

## 3. Working rhythm

- **Batch hand-in.** Code the whole batch, build once, register. A new bug
  that arrives mid-batch goes into the next batch unless it blocks this
  one. One worker absorbed seven bugs into one unreleased head and looped
  patch, build, fix for hours. Per-fix hand-ins are churn too.
- **Never wait in the foreground.** Run builds in the background and keep
  coding. In the study a job was pending for 13.2 of 26.3 active hours;
  98 sleep or poll loops held the foreground for 3.1 hours; 241 turns only
  waited or polled, and each re-sent a context of up to 370k tokens.
- **Compile before you run.** One loop ran 8 failing sweeps in 73 minutes,
  half of them dying on compile errors a 16-second compile would have
  shown. Run the cheap compile first, then one sweep per batch.
- **Keep target names in the build script,** not in memory: 5 of the 42
  failed builds were guessed ninja target names.
- **Keep an index of giant files** (symbols and line numbers) in your resume
  notes. 62% of the study's shell commands only searched or read; after
  each of its 13 compactions, 41% of the next calls were re-reads. One file
  had 78 edits and 194 reads.

## 4. Packaging

- Package from a **frozen snapshot**, not the live tree. The tree moved
  during packaging and two zips got different native DLLs. The SDR# packager
  now freezes a Release build into `<out>/<commit>/` (`freeze_native.py`:
  the runtime modules checked byte-for-byte against the installed SDK of
  that commit, the fonts, the notices and a `SNAPSHOT.txt` of hashes), and
  every zip is made from that.

## 5. Look iteration

- Iterate on a look with a **runtime theme sheet**, not a rebuild. The SDR#
  package reads its sheet at start-up (`--theme-sheet`, or
  `GUI_FORMS_THEME_SHEET`); a universal `--theme` option is in progress.
  Edit the sheet and rerun: the owner got three candidates in minutes
  instead of one build and capture each.

## 6. Review hygiene

- **Review the branch you mean to review.** Read files with
  `git show <branch>:<path>`, or diff against the branch, not the checkout.
  A review of main's working tree instead of the staged branch raised a
  false alarm this week.
- **Coordinators:** one ordered message per hand-in; interrupt only for a
  stop, a reboot or the owner waiting; settle the owner's rulings before
  work starts. In the study 41 of 62 coordinator messages landed on an open
  batch and 15 were real redirects (one feature was coded, then ruled out
  18 minutes later).

## 7. Pitfalls

- **`; echo rc=$?` hides failures.** The command then exits 0 whether the
  build failed or not. Check the real exit code (`set -o pipefail`, or test
  `$?` and exit with it), and read build outcomes from the exit status, not
  from a printed line.
- **Symlinks out of the tree.** `ci_build.py` followed a `gui_forms` symlink
  up to `D:/` and found a stale clone beside the tree. Keep build roots
  free of links that point outside the workspace.
- **Heredocs for patches.** 12 bash heredocs failed on quoting (`unexpected
  EOF`). Write patches and edit scripts as files and run them.
- **Edit before Read.** 12 edits were refused because the file had not been
  read in the session. Read, then edit.
- **`/tmp` versus `$TEMP`** on Windows under Git Bash: they are different
  places. Use one explicit scratch directory.
