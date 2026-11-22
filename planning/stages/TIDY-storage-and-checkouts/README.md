# TIDY: one checkout per repository, one place for work, no sprawl

Written by the coordinator, 2026-11-21, from the owner's direction of the
same day. Order of operations **step 2**, which must finish "before we touch
anything new" (owner). Status: steps 1 to 6 and 8 done on 2026-11-21; the state,
what was lost and what is open are in the private `projects` repository,
`desks/coordinator/TIDY_STATUS_2026-11-21.md` (kept out of this public repository). The `inventory/` folder named below no longer exists (it was in a
session's side notes and went when the session was archived).

## 1. The owner's direction (2026-11-21)

- Local storage, worker environments and the rest are scattered.
- The owner does not like `~/Developer/` (the models all prefer it) and could
  tolerate **`/repository/`** for the repositories (gui_forms, stxlib and the
  rest) and **`/work/`** for worker storage.
- Many instances have their own Python virtual environment that was never
  standardised.
- Documentation, notes and the like have sprawled into a dozen places.
- Identify everything that was done and not committed (probably very little).
- **Keep only one checkout per repository on each machine: the best and
  newest.** Then clean up remote branches.
- The integrator is already cleaning up the pre-existing test material; warn
  them before `gui_forms` or their branch moves.

## 2. First inventory (2026-11-21, this Mac, read-only)

Raw data: the coordinator's notes folder, `inventory/checkouts.json` (every
checkout and worktree: path, repository, branch, uncommitted files,
unpushed commits) and `inventory/sizes.txt`.

- **Disk:** 192 GiB of 228 used, **13 GiB free (94 %)**.
- **324 checkouts and worktrees of 71 repositories** under `~/Developer`,
  `~/bfft`, `~/Documents`, `~/paymenottowork` and the scratchpads, among them
  136 of gui_forms, 25 of stxlib, 22 of template_harness, 20 of stx, 18 of
  games (PlaySuite), 14 of notes, 7 of stx_auxlib, 5 of plan-paint and 5 of
  bfft.
- **Largest folders:** `~/Developer/gui_forms_workers` 25.3 GB, `~/.codex`
  24.7 GB (Codex's own sessions and worktrees), `~/paymenottowork` 10.9 GB,
  `~/Documents` 9.0 GB, `~/Library/Caches` 8.8 GB, the PlaySuite game
  checkouts about 18 GB together (`playsuite-games-*`, 15 folders),
  `~/bfft` 4.5 GB, `~/.cache` 3.6 GB, `pocket-bible` 2.6 GB, `emsdk` 1.8 GB
  (the Emscripten SDK, unneeded until the web build after 1.0),
  `conformance` 1.8 GB.
- **Virtual environments on this Mac:** few (`~/bfft/.venv-jpeg` 0.7 GB,
  `~/.venvs/ducklips` 0.3 GB, two small ones); the many per-instance ones are
  probably on the M4, which this pass did not read.
- Uncommitted or unpushed work exists in several repositories (for example
  `projects`, `games`, `gui_forms_tests`, `demonstrators`, `bfft`,
  `paymenottowork`); the counts need a per-checkout reading, since worktrees
  of one repository share their branches.

## 2.1 Steps 1-4 done (2026-11-21)

The one-off worker's report is `inventory/REPORT.md` in the coordinator's
notes folder: 390 checkouts on the Mac and 114 on the M4, 91 repositories;
44 rescue branches and 42 verified bundles; one keeper per repository. The
owner approved the 147.5 GB list that leaves the integrator's trees alone
and ran the scripts (`delete_mac.sh`, `delete_m4.sh`):
- Mac: 266 items removed, 46.5 GB; free space 9.5 GiB to 49 GiB;
- M4: 65 items removed, 79.9 GB (21.1 GB internal, 58.8 GB Storage); Storage
  30 to 84 GiB free; the internal disk stays at 23 GiB until its Time Machine
  local snapshots age out (they hold the deleted blocks);
- skipped by the guards: Codex's own folders, the PlaySuite and gui_forms
  checkouts that still hold unrescued media, and anything modified in the
  last 12 hours; the integrator's 106 GB class is untouched.
Owner, 2026-11-21: the three repositories gone from GitHub
(`Modern.Forms.Codex` and `RiemannHypothesisSolutionProof` on the M4,
`airspyhf_musl` inside a radio backup on the Mac) are deletable; the owner
runs the deletion. Left for the owner: the rest of the report's "needs the
owner's eye" list (repositories
gone from GitHub, unpushed branches on one disk, orphaned worktrees, media
too large to rescue, large folders outside the repositories).

## 2.2 Steps 5, 6 and 8 done (2026-11-21, evening)

The move, the reference update, the path fixes in tracked files and the
branch cleanup (merged and patch-merged, local and GitHub, and the M4
mirrors) are done. `projects` is reconciled on a local branch. Step 7's
leftovers are kept aside; the old folders wait for the owner's removal. Everything is in `projects`, `desks/coordinator/TIDY_STATUS_2026-11-21.md`.

## 3. The layout (owner, 2026-11-21: stays in `~/Developer`)

The owner first proposed `repository` and `work` folders in the home folder,
then chose to keep `~/Developer` ("it can remain in ~/Developer"), which
spares the reference churn of a rename:

| Path | Holds |
|---|---|
| `~/Developer/<name>` | the one checkout of each repository, at its newest remote state: `gui_forms`, `stxlib`, `stx`, `notes`, `projects`, `gui_forms_tests`, `plan-paint`, `games`, `swiftedit`, `template_harness`, `bfft`, ... |
| `~/Developer/work/<seat>` | each worker's storage: its worktree of a repository (a branch per seat), logs, scratch |
| `~/Developer/work/venvs/<purpose>` | one standard virtual environment per purpose, with pinned requirements kept in a repository |

Each keeper must be the newest state, so no tool lands on a stale copy: a few
days earlier Codex could not find the latest Plan Paint while studying CONV
(`~/Developer/plan-paint` was at `1f56e91`, the newest work at `e3b2a09` in a
Codex worktree). Notes are not kept here: they go to the notes repository's
branches or `projects`.

## 4. The steps

1. **Full inventory, both machines** (read-only): every checkout and
   worktree with its uncommitted files, unpushed commits per branch, last
   activity and size; every virtual environment; every place notes and
   documents collected; the M4's build trees and environments.
2. **Rescue:** anything uncommitted or unpushed that is real work is
   committed on a named local branch (never pushed without the owner) and
   listed; anything already merged or superseded is listed as such.
3. **Choose** the best, newest checkout of each repository; list the others
   for deletion, each with the reason it is safe (clean, nothing unpushed,
   or rescued).
4. **The owner approves the lists**; then the extra checkouts, worktrees,
   stale build trees and environments are removed. The integrator is warned
   before `gui_forms` or their branch moves.
5. **Move** the seats under `~/Developer/work`, bring every keeper to its
   newest remote state, and standardise the environments under
   `~/Developer/work/venvs`.
6. **Update every reference** to the old paths: the agent definitions
   (`~/.claude/agents`, the STXlib workspace's `.claude/agents`), the
   workspace `CLAUDE.md` files, the briefs and track files in `projects`,
   the M4 helper scripts (`m4ws`, `m4build`, `m4sb*`), the coordinators'
   memories, and `AGENTS.md` files that name `~/Developer`.
7. **Consolidate the sprawl:** documents and notes found outside the notes
   repository and `projects` are moved there or listed for the owner.
8. **Remote branches:** per repository, the merged and abandoned branches,
   listed for the owner's approval, then deleted.

## 5. Who

A dedicated one-off worker (agent type `gui-forms-oneoff`, 1M context) runs
steps 1-3 and prepares the lists; destructive steps wait for the owner's
approval. The coordinator coordinates with the integrator.
