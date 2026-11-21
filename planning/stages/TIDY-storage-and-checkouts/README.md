# TIDY: one checkout per repository, one place for work, no sprawl

Written by the coordinator, 2026-11-21, from the owner's direction of the
same day. Order of operations **step 2**, which must finish "before we touch
anything new" (owner). Status: plan, with a first read-only inventory.

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

## 3. The layout

| Path | Holds |
|---|---|
| `/repository/<name>` | the one checkout of each repository: `gui_forms`, `stxlib`, `stx`, `notes`, `projects`, `gui_forms_tests`, `plan-paint`, `games`, `swiftedit`, `template_harness`, `bfft`, ... |
| `/work/<seat>` | each worker's storage: its worktree of a repository (branch per seat), logs, scratch |
| `/work/venvs/<purpose>` | one standard virtual environment per purpose, each with a pinned requirements file kept in a repository |
| `/work/notes` | nothing permanent: notes go to the notes repository's branches or `projects`, as `AGENTS.md` says |

**On macOS the root volume is read-only** (the signed system volume), so
`/repository` and `/work` cannot simply be created there. The supported way
is `/etc/synthetic.conf`, one line each, mapping the names to folders on the
data volume, for example:

```text
repository	System/Volumes/Data/repository
work	System/Volumes/Data/work
```

then a reboot (or `apfs.util -t`). That edits system configuration and needs
an administrator password, so it is the **owner's action**, on this Mac and
on the M4. The alternative without touching the system is `/Users/Shared/`
or the home folder, which the owner did not want. On Linux and FreeBSD the
two folders are ordinary directories.

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
5. **Move** the survivors to `/repository` and `/work` (after the owner's
   `synthetic.conf` change), and standardise the environments under
   `/work/venvs`.
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
