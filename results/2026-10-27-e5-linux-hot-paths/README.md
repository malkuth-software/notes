# E5 baseline: Linux hot paths under the upstream-Skia renderer

Date: 2026-10-27. Stage: Phase E, E5's dynamic half (profiling). Status: the
first baseline, recorded. It holds the hot-path list of the current
renderer (GUI.Forms through the CPU Skia adapter as it is, upstream Skia),
for comparison when stx_skia replaces it and when later hot-loop work lands.

**No speed claims.** The shares below are fractions of perf samples, not
timings. The frame times in `paint-profile.jsonl` were taken while perf was
sampling. They are context for this run only and say nothing about whether
anything is fast or slow.

## How it was taken

- **Workflow:** `.github/workflows/profile-linux.yml` in gui_forms ("Linux
  hot-path profile"). It runs on a weekly schedule, on demand, and on a pull
  request that changes the profiler.
- **Tool:** `tools/profile_hot_paths.py run` builds the workloads, samples
  them and writes the list.
- **Run:** [40626318907](https://github.com/malkuth-software/gui_forms/actions/runs/40626318907),
  PR #423, head `ca145287`, merge commit `d16c7910`, event `pull_request`.
  The artifact is `hot-paths-linux-x64`, kept for 90 days. Its files are
  copied here.
- **Build:** a Release toolkit build with
  `-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -gline-tables-only`.
  The cached Skia archive is built without frame pointers, so a sample inside
  Skia may not reach its GUI.Forms callers. GUI.Forms inclusive shares are
  therefore lower bounds.
- **Sampling:** `perf record -F 999 -g` per workload. Two listings are taken
  with `perf report --stdio --sort dso,symbol`: one by self share
  (`--no-children`) and one by inclusive share (`--children`).
- **Host:**
  - GitHub-hosted `ubuntu-24.04` (image ubuntu24 20261023.334.3);
  - AMD EPYC 7763, 4 CPUs;
  - Linux 6.17.0-1022-azure;
  - perf 6.17.13;
  - Ubuntu clang 22.1.8.

  A hosted runner is shared and virtualised, so its machine varies between
  runs. One earlier run was on an EPYC 9V45 with Linux 7.0 and showed the
  same ranking.

## Workloads

| Workload | What | Samples |
|---|---|---:|
| paint | The Gallery through the CPU Skia adapter, in two phases (`benchmarks/paint_profile_workload.cpp`). **M2d damage matrix:** the control-spectrum page at 900 × 620; scales 1, 2 and 3 × damage 1, 10 and 100 %; 24 warm-up and 240 measured frames each. **Pages:** all 30 Gallery pages at scales 1 and 2; 4 warm-up and 24 measured frames each. | about 11,000 |
| text_engine | 100 caption labels through the HarfBuzz/FreeType engine, 180 passes (`benchmarks/text_engine_benchmark.cpp`) | about 15,000 |
| document_view | The renderer-neutral DocumentView D1 workload, run 100 times in one recording (`benchmarks/document_view_benchmark.cpp`) | about 13,000 |

## Shares by area (self)

| Area | paint | text_engine | document_view |
|---|---:|---:|---:|
| skia | 47.13 % | – | – |
| gui_forms | 40.19 % | 15.48 % | 43.46 % |
| freetype | 3.13 % | 71.80 % | – |
| harfbuzz | – | 2.22 % | – |
| workload (benchmark's own code) | 1.00 % | 0.05 % | 52.23 % |
| runtime, kernel, other | 1.59 % | 7.60 % | 2.52 % |

## The top functions

**paint**

| Function | Area | Self | Inclusive |
|---|---|---:|---:|
| `sse2::rect_memset32` | skia | 28.62 % | 28.66 % |
| `gui_forms::detail::composite_text_coverage` | gui_forms | 24.46 % | 34.60 % |
| `gui_forms::detail::prepared_clip_coverage` | gui_forms | 10.29 % | 10.30 % |
| `sse2::lowp::evenly_spaced_2_stop_gradient` | skia | 5.29 % | 5.29 % |
| `aaa_fill_path` | skia | 1.48 % | 1.49 % |
| `sse2::lowp::store_8888` | skia | 1.23 % | 1.23 % |

`replay_display_chunk` is 40.57 % inclusive.

Three functions take 63 % of the samples between them:
- Skia's solid rectangle fill;
- GUI.Forms' own text compositing;
- GUI.Forms' clip coverage.

The raster pipeline stages and the path fill come after them.

**text_engine**

| Function | Area | Self | Inclusive |
|---|---|---:|---:|
| `af_glyph_hints_reload` | freetype | 10.64 % | 12.82 % |
| `gray_render_line` | freetype | 7.98 % | 7.98 % |
| `gui_forms::detail::composite_text_coverage` | gui_forms | 7.15 % | 10.23 % |
| `af_latin_hints_compute_segments` | freetype | 4.79 % | 4.79 % |
| `af_latin_hints_apply` | freetype | 4.70 % | 33.67 % |

FreeType's autohinter and rasteriser take most of the samples, through
`GlyphRunRasterizer::Impl::Runner::run` (77.55 % inclusive).

**document_view**

| Function | Area | Self | Inclusive |
|---|---|---:|---:|
| `main` (the benchmark's inlined loops) | workload | 52.23 % | 98.58 % |
| `gui_forms::validate_utf8` | gui_forms | 29.74 % | 29.74 % |
| `gui_forms::DocumentViewState::publish` | gui_forms | 8.45 % | 38.74 % |

Most of `main`'s own share is the linear-reference mapping, which the
benchmark keeps as a comparison and which is not GUI.Forms code.

## Files

- **`hot-paths.md`:** the published report. It lists the top 40 functions
  per workload.
- **`hot-paths.json`:** the same list in machine-readable form, schema
  `gui_forms.hot_paths.v1`.
- **`paint-profile.jsonl`:** the paint workload's own records, one line per
  matrix cell and per page. Each holds:
  - frame-time median, maximum and total, taken under sampling;
  - GUI.Forms counter deltas (paint passes, controls painted, display chunks
    rebuilt and reused, painted damage area);
  - a hash of the final pixels.

  The hash lets a later renderer's run be checked against the same output.

## Earlier runs on the same PR

These runs fixed the tool; their numbers are not the baseline:
- **40622834612** (head `6e865409`): perf's padded symbol column broke the
  parse.
- **40625121564** (head `d302d118`): split `--children` rows gave inclusive
  shares below self, and the two benchmarks had too few samples (826 and 81).
