# E4: stx_skia's instruction-set paths against a scalar build

Date: 2026-10-28. Stage: Phase E, item E4 (stx_skia's optimisations, each SIMD
path checked against a scalar reference, then timed). Status: correctness
recorded and traced on arm64 and x86-64; arm64 speed recorded on the M4; x86-64
speed comes from the stx_skia CI profiles job.

## Files

- `E4_INVENTORY.md`: stx_skia's optimisations and how each one is tested.
- `ISA_CORRECTNESS.md`: each SIMD path against the scalar build (pixels), the
  shipped arm64 path against the shipped x86-64 path, and speed on the M4.
- `DIFFERENCE_TRACE.md`: every corpus file that differs between a SIMD path and
  the scalar build, or between arm64 and x86-64, traced to its cause. It holds
  the fused-multiply-add list the owner asked for (2026-10-28) and the record of
  the accepted nearest-sampling difference.

## How it was taken

- **Method and tools:** gui_forms `stx_skia/docs/ISA_PROFILES.md`. The tools
  are `stx_skia/tools/compare_isa.py`, `measure_isa.py` and
  `run_recorded_workloads.py`, and the workload is
  `stx_skia/tests/isa_workload.cpp`.
- **Host:** the M4 mini (arm64, Homebrew Clang 22.1.8, Release). x86-64 builds
  are cross-compiled on the same machine and run under Rosetta 2, which is used
  for correctness only.
- **Branch:** gui_forms `claude/stx-skia-e4`, based on `origin/main` f8326690.
- **Attribution builds** (temporary, never committed): the raster pipeline with
  its fused `mad`/`nmad` replaced by a separate multiply and add, the same with
  high precision forced, `-ffp-contract=off` on the shipped profiles, and the
  scalar profile with single features restored. They are described where they
  are used in `DIFFERENCE_TRACE.md`.
