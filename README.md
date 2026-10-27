# File Manager notes

This is the `file_manager` branch of `malkuth-software/notes`. It holds everything
about File Manager (`malkuth-software/file_manager`) that is worth keeping but
isn't needed to build it. The layout follows the `gui_forms` branch.

| Folder | What | From (File Manager, reset `main` 028a3f96 unless named) |
|---|---|---|
| `planning/` | charter, program and experiment maps, evidence register, question atlas, interview round 002, reassessments, resume and shutdown records, the GUI.Forms 0.2 consumption record, and the `gui_forms/` and `search/` planning sets (their interviews included) | `planning/`, without `PROGRAMMING_HOUSE_STYLE.md` (the house style is `HOUSE_STYLE.md` on `main`) |
| `decisions/` | File Manager's decision records | `decisions/` |
| `studies/` | `zeta_search/`: the search-algorithm study and its File Manager applicability; `PRV_package.zip`, its source package | `research/` |
| `experiments/` | experiment reports: `jpeg_decode/` and `preview_admission/` (their code stays in the product under `frontend/experiments/`); `copy_metadata/` and `preview_paint/` in full | see below |
| `results/` | dated gate records, measurements and native test logs | `frontend/results/` |

Copied file for file from git blobs, so recorded logs keep their bytes; the
nested `.gitattributes` in `results/` keep CRLF logs as captured. Relative links
that pointed into the product's source tree (`frontend/`, `tools/`) no longer
resolve here; read them against the product at 028a3f96.

## Never-merged research kept here

Two research branches of the pre-reset history were never merged into the
product. Their material lives only here:

- **Copy metadata** (`experiments/copy_metadata/`, `results/2026-10-03-copy-metadata/`;
  branch `codex/file-manager-copy-metadata`, 5d01f4d3): a cross-platform probe of
  what a staged copy preserves (times, modes, file-clock representation), its
  workflow (`copy-metadata.yml`) and measurements. The shipped behaviour's
  evidence is `results/2026-10-03-directory-copy-metadata/`.
- **Preview paint** (`experiments/preview_paint/`, `results/2026-10-04-preview-paint/`,
  `results/2026-10-04-preview-admission/native/`; branch
  `codex/file-manager-preview-paint`, 77d78673, closed PR #49): measuring
  preview image admission through forced native macOS paint. A rejected
  approach, kept as a negative result. `PREVIEW_ADMISSION_README_at_preview_paint.md`
  is that branch's version of the admission experiment's README.

From now on File Manager's plans, decisions, interviews, studies and results
are kept here, not in the product repository.
