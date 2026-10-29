# stx_skia's optimisations and how each was tested — 2026-10-28

Phase E, item E4. Each optimisation the results records claim is listed with the host, the
instruction-set path and the workload it was measured on, and what E4 adds. A record name such
as SWAP_KERNEL is `stx_skia/results/<date>/SWAP_KERNEL.md` in gui_forms; tool and document
paths are relative to gui_forms' `stx_skia/`. Records that only convert code, and say it is no
slower, are not optimisations and are left out, except where their checks cover a SIMD path.

**The pattern.** Almost every timing was taken on one host, the M4 (arm64, NEON). The CI hosts
ran the same tests for correctness, never the timings. The x86-64 numbers that exist come from
one Zen 5 pod and cover the swap kernel only. E4 closes that in two ways:
- **Correctness:** each shipped SIMD path, and the wider x86 paths, against a scalar build of
  the same source (gui_forms `stx_skia/docs/ISA_PROFILES.md`, and `ISA_CORRECTNESS.md` here).
- **Speed:** every timing tool now runs on each CI host from one driver,
  `tools/run_recorded_workloads.py`, in the existing stx_skia profiles job.

## SIMD kernels and the instruction-set policy

| Optimisation | Record | Measured on | Path | Workload | E4 |
|---|---|---|---|---|---|
| Baseline-SIMD policy: SSE2/NEON only, no run-time dispatch | `cmake/prune.cmake`, OPTIMIZATION_STUDY | Its cost was measured only for the swap kernel (Zen 5: Skia's AVX2 swap 1.4–6.7× faster in cache) | SSE2, NEON | — | The raster pipeline's SSE2 against AVX2 (`x86-64-v3`) on every x86 CI host; the fill kernel's reference (AVX/ERMS dispatched) against the candidate |
| Red/blue swap kernel (`stx_skia::swizzle`) | SWAP_KERNEL, SWAP_KERNEL_GUI_INTEGRATION | M4; Zen 5 EPYC 9575F (Linux, one pinned CPU, loaded host) | NEON; SSE2; SSE4.1 (`x86-64-v2`) | `measure_swizzle.py`: 64 px to 3840×2160, in place and disjoint | Timed on all CI hosts (`--no-code`). Open: the Zen 5 code-placement split (8 against 15 Gpx/s by binary) |
| Rectangle fill kernel (`memset32`, `rect_memset32`): plain loop | FILL_KERNEL (not adopted) | M4 | NEON | `measure_fill.py`; GUI.Forms renderer benchmark | Timed on all CI hosts. Open: why the plain loop slowed full-window repaints |
| Raster kernel macros and casts | RASTER_KERNEL_MACROS | Object code compared, no timing | arm64 NEON and scalar; x86-64 SSE2, SSE4.1, AVX, AVX2, AVX-512, scalar | — | The workload now checks the pixels of four of these paths against scalar |
| Raster pipeline spelling (named functions kept `inline`, not forced) | RASTER_PIPELINE_SPELLING | Object code, M4 | Eight configurations | — | As above |
| Raster stage pruning (519 to 229 operations) | RASTER_STAGE_PRUNING | M4 (no timing claim) | NEON | 3,168-case pipeline test | As above |
| Pixel conversion (alpha extraction 3.8–7.5×) | PIXEL_CONVERSION | M4 | NEON | `measure_conversion.py` | Timed on all CI hosts; the tool no longer needs `baseline/` in the tree |
| HQ mipmap kernels | MIPMAP_KERNELS | M4 | NEON (scalar code, compiler-vectorized) | `measure_mipmap_kernels.py` | Timed on all CI hosts (`--no-assembly`; the assembly step needs the macOS SDK) |
| Blur styles and analytic rectangle kernels | BLUR_STYLES | M4 | NEON (128-bit, compiler) | `measure_masks.py --rect` | Timed on all CI hosts |
| Mask descriptors and Gaussian blur | MASKS | M4 | NEON | `measure_masks.py` | Timed on all CI hosts |

## Scalar algorithms and containers

These are not ISA paths, but their claims were also measured on the M4 alone, so the same
driver times them on every CI host.

| Optimisation | Record | Measured on | Workload | E4 |
|---|---|---|---|---|
| Hash storage and probe relocation | HASH_STORAGE | M4 | `measure_hash_storage.py` | All CI hosts |
| Sort | SORT | M4 | `measure_sort.py` | All CI hosts |
| TDArray | TD_ARRAY | M4 (insert/erase 6.4% slower, kept) | `measure_td_array.py` | All CI hosts |
| Priority queue | PRIORITY_QUEUE | M4 | no tool | Not timed: no recorded workload |
| Writer32 | WRITER32 | M4 | `measure_writer32.py` | All CI hosts |
| Read buffer | READ_BUFFER | M4 | `measure_read_buffer.py` | Not run: it times against a build of the reader's prior source (`tools/build_read_buffer_comparison.py`), not the pin |
| Owned cache invalidation | CACHE_INVALIDATION | M4 | `measure_cache_invalidation.py` | All CI hosts |
| Discardable pool | DISCARDABLE_POOL | M4 | `measure_discardable_pool.py` | All CI hosts |
| Synchronization | SYNCHRONIZATION | M4 (macOS branch only) | `measure_synchronization.py` | All CI hosts: the first timings of the Linux and Windows branches |
| Factory registry removal | FACTORY_REGISTRY | M4 | no tool | Not timed |

## Leads carried into E4

- **Fill-kernel slowdown.** The plain loop matched the original in isolation but slowed
  full-window repaints by 1.08–1.35× (FILL_KERNEL). The cause is unknown.
- **Portable in-place swap.** Skia's portable loop fails its own overlap check in place and runs
  one pixel at a time (about 1.8 against 7.5 Gpx/s on Zen 5). stx_skia's kernel replaced it in
  GUI.Forms.
- **Run-time dispatch** never runs under the baseline policy, so every dispatched variant is
  dead code in the candidate.
- **Zen 5 placement:** 15 against 8 Gpx/s for the same kernel, depending on the binary.
- **Noise floor:** up to 6% on the M4 between two runs of the same function.
- **Fill volume (hot-paths track, w4):** `rect_memset32` is 40.7% of paint on the M4 and 29.4%
  on linux-x64. 99.66% comes through `SkiaRaster::fill_rect`, and opaque fill area is 4.91× the
  damaged area. The lever is overdraw, which the hot-paths worker is taking; a kernel change
  from its banded-paint design comes here for review.
