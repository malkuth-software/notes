# SK2: state at the pause and how to resume

Date: 2026-10-29. Stage: SK2, BGRA8 premultiplied sRGB everywhere, with PNG only
at the image seam. The spec is in projects/handoffs/SKIA_HANDOFF.md §4.

Status: paused by owner direction. x86-64 testing belongs on the RunPod (native
x86-64), never under Rosetta. Windows testing belongs on the Windows host. Every
SK2 commit has been checked on the M4 (macOS arm64) only, to the extent listed
below. Nothing has been pushed.

## Branches

The gui_forms branches all sit on `origin/main` 8cde98df (checkpoint 3).

| Branch | Head | Contents |
|---|---|---|
| `claude/stx-skia-sk2` | 2635c89d | E4 chain, present workload, SK2 stages 1-8 |
| `claude/stx-skia-present-workload` | b23871d8 | E4 chain and the workload alone (checkpoint 4 folds this alone) |
| `claude/stx-skia-e4` | 102797ca | E4 chain: 22d0234f, d855751c, 102797ca |
| `claude/stx-skia-evidence` | 8ad37e3f | untouched |

`claude/stx-skia-sk2`, from oldest to newest:

| Commit | Stage | Verified on the M4 |
|---|---|---|
| 22d0234f, d855751c, 102797ca | E4 (ISA checks, traced bounds, HLG black) | yes, but its x86-64 evidence came from Rosetta; see "E4 runs to redo natively" |
| b23871d8 | present-per-frame workload (D4) | builds and runs on macOS |
| 364746e6 | 1: Skia and CoreGraphics rasters BGRA, presents copy, `FramebufferChannelOrder` removed | touched tests plain + sanitize, gate, checkers |
| bdac4b6f | 2: live surfaces BGRA only (D1: `LiveSurfacePixelFormat::rgba32_premultiplied_srgb` removed) | 197/197 plain + sanitize, gate, checkers |
| 2feca4bb | 3: engine buffers and part rasters BGRA | 221/221 plain + sanitize, gate, checkers |
| 63f2c213 | 4 (D5): `gui_drawing::PixelFormat` one value; `GD_PIXEL_RGBA32_PREMULTIPLIED` removed | 227 plain + sanitize, gate, checkers |
| 639e67ae | 5: HostImage premultiplied BGRA8; GFIM v2; v2 clipboard names written, v1 read (D2) | macOS + headless narrow set 13/13 plain + sanitize; checkers |
| edced66e | 6: X11 present plain copy for the common visual, three named fallback kernels | syntax-checked on the M4 against Homebrew libx11 headers only |
| 478b9f30 | 7: RGBA grep gate | 230/230 plain + sanitize, gate, checkers |
| 2635c89d | 8 (partly verified): `SK_R32_SHIFT=16`, so N32 is BGRA everywhere | 230/230 plain and checkers only |

"Touched tests" means ctest `-R` with this regex:
`skia|font|retained_draw_cache|canvas|drawing|text_identity|backplane|part_composite|opaque_live|live_surface|sha256|application_native|control_snapshot|coregraphics|compositor|layered_material|engine|theme_compiler|presentation|damage|text_composite|png|clipboard|image|macos|gallery|idle_wake|notify_icon|host_service|command_chain|doc_examples`.

The runs used `python3 -B tools/ci_build.py [--sanitize] --jobs 10 --tests "<regex>"`. The gate is `python3 -B ../template_harness/harness/check_state_machines.py --root . --build-dir ../.build/native-macos-arm64/gui-forms --jobs 8 --gate --changed-from origin/main`.

The checkers, run from gui_forms:
- `tools/check_house_style.py include src tests tools`
- `tools/check_lifetime_rules.py include src tests tools`
- `code_integrity_checker/check.py include src stx_skia --gate`
- `tools/check_stx_skia_style.py`
- `tools/generate_library_docs.py --check`

## What SK2 still needs

### 1. Stage 8 verification (macOS M4)

Stage 8 is committed, but only partly verified. Remaining:

- The touched tests under `--sanitize`, plus the state-machine gate.
- stx_skia corpora, from `stx_skia/`. Build the candidate, candidate-sanitize and baseline profiles with `tools/build_native.sh` (baseline takes `STX_SKIA_REFERENCE_ROOT=<gui_forms>/.tmp_stx_skia_reference`). Then run `tools/compare_profiles.py <root>/baseline <root>/candidate` and `tools/check_build_scope.py <root>/candidate`. The define is PUBLIC, so both profiles take it and the comparison stays like for like.
- `tools/compare_isa.py <root>/candidate-scalar <root>/candidate` on arm64, against `tests/isa_bounds.txt`.
- The macOS present A/B after stage 8 (see "Present measurements").

Why stage 8 exists: the spec puts the BGRA shifts "in the macOS and Linux profiles". Without them, stages 1-7 move macOS and Linux surfaces off Skia's N32 blitters onto its raster pipeline.

### 2. Linux, natively on x86-64 (RunPod)

These assertions exist only in Linux CI. None have run anywhere yet.

- `tests/linux_host_tests.cpp`:
  - `present_kernels` checks six fake visuals for the kernel chosen and exact pixels: depth 24 → plain_copy; ARGB presented → plain_copy; ARGB opaque → byte_channels; swapped → byte_channels; 10-bit depth 30 → scaled32; 565 depth 16 → generic.
  - Clipboard exactness: owned read-back; PNG incoming; a v1 `application/x-gui-forms-rgba` peer.
  - X11 tray icons, which draw `straight_channel` colours.
- The whole touched set, plain and under the sanitizer, under Xvfb as in `.github/workflows/native-platform.yml`.
- The present workload on X11, before (b23871d8) and after (stage 8), 5 interleaved runs each.
- stx_skia on x86-64 natively, at stage 8:
  - the candidate, candidate-scalar, candidate-x86-64-v2 and candidate-x86-64-v3 profiles;
  - `compare_isa.py` for each against candidate-scalar;
  - the corpora, baseline against candidate.

### 3. Windows host

These are CI-only so far.

- `drawing_windows_probe.c` must print `drawing-windows-bgra-only=pass`.
- `windows_canvas_raster_tests` `clipboard_images` must be exact for:
  - v2 `GUI.Forms.BGRA8.v2`;
  - CF_DIBV5 only (straight bytes 50,100,199,128 at offset 128);
  - v1 `GUI.Forms.RGBA8.v1`.
- Notification icons (unpremultiplied per row), the drawing-platform DIB copies (memcpy, no swap) and the present workload before and after.
- Cross-host identity hashes. Windows Skia was already BGRA N32, but stage 1 moved its SkiaRaster surfaces from kRGBA, which ran on the raster pipeline, to kBGRA, which runs on the N32 blitters. Any Windows test that hashes SkiaRaster output may change at stage 1. Check before SK5 re-records goldens.

### 4. Open items for the coordinator

- `docs/reference/inventory.json` is not regenerated. It is maintainer-only and still lists the removed enumerators.
- The removal of the C ABI constant `GD_PIXEL_RGBA32_PREMULTIPLIED` (stage 4) is flagged for objection.
- The macOS status item image is now built through a CGImage (`PremultipliedFirst | ByteOrder32Little`), because NSBitmapImageRep planes ignore the little-endian flag.
- `stx_skia_swizzle` has no caller left in `src`. Decide whether to keep it for input entry kernels (stx_skia `docs/SWAP_KERNEL_PLAN.md`) or remove it.
- stx_skia docs that still describe per-host N32: `docs/COORDINATOR_REQUIREMENTS.md:90` and `docs/CONVERSION.md` (swap kernel section).
- `renderer_benchmark.cpp` (stage 7 PPM writer fix) needs `GUI_FORMS_BUILD_BENCHMARKS=ON` to compile and has not been compiled.

## Present measurements (macOS, M4, 800 x 560 at scale 1, 300 frames)

| Run | present µs/frame | screen copy µs/frame |
|---|---|---|
| before b23871d8, alone (3 runs) | 792-809 | 385-399 |
| stage 7 alone, after a sanitize build (3 runs) | 857-863 | 406-409 |
| interleaved before (5) | 797, 821, 836, 822, 817 (median 821) | 391, 413, 413, 400, 401 (median 401) |
| interleaved stage 7 (5) | 871, 846, 793, 878, 904 (median 871) | 419, 402, 379, 415, 429 (median 415) |

The before binary was built from `git archive b23871d8`, with tests and audio off and the fetched `third_party/` copied in. The ranges overlap.

Stage 8's numbers have not been taken. If stage 8 does not bring the median back to the "before" level, profile the paint side. The present time minus the copy is about 420 µs before and 455 µs at stage 7.

## E4 runs to redo natively on x86-64 (RunPod)

The E4 x86-64 correctness evidence was cross-compiled on the M4 and run under Rosetta 2. Redo it natively. The affected material:

- Notes, `results/2026-10-28-e4-stx-skia-isa/`:
  - `ISA_CORRECTNESS.md`: the x86-64 host description (lines 25-31), the SSE2, SSE4.2 and AVX2 comparisons, scalar arm64 against scalar x86-64 (line 93 on), and the build note (149-150).
  - `DIFFERENCE_TRACE.md`: the 10 files that differ between the arm64 and x86-64 scalar builds, the NaN-sign and libm causes, and the AVX2 entries.
  - The x86 attribution builds (`e4-abl-x-*`).
  - Native x86-64 on the RunPod is Linux with glibc's libm, not Apple's x86-64 libm. The cross-architecture comparison therefore becomes cross-OS as well. Record which differences remain, and re-trace any new ones.
- gui_forms, on this branch's E4 commits:
  - `stx_skia/tests/isa_bounds.txt` (x86-64 entries "under Rosetta", d855751c);
  - `stx_skia/docs/ISA_PROFILES.md` (the x86 build instructions);
  - `stx_skia/docs/CONVERSION.md:1986` (Rosetta method).
  Replace them with the native runs and the native method.
- Commands, from `stx_skia/` on the RunPod:
  ```sh
  for p in candidate candidate-scalar candidate-x86-64-v2 candidate-x86-64-v3; do
    STX_SKIA_BUILD_ROOT=<root> BUILD_JOBS=<n> sh tools/build_native.sh $p
  done
  for p in candidate candidate-x86-64-v2 candidate-x86-64-v3; do
    python3 -B tools/compare_isa.py <root>/candidate-scalar <root>/$p --json <root>/isa-$p.json
  done
  ```
  Then compare the arm64 scalar outputs (copied from the M4) with the x86-64 scalar outputs: `compare_isa.py <x86>/candidate-scalar <arm>/candidate-scalar`. x86-64 timings can now be taken there as well, with `tools/measure_isa.py` and `tools/run_recorded_workloads.py`.

## Open review

The AAA clip-invariance review for the hot-paths worker (w4) is open. w4 is to send a diff that makes stx_skia's analytic antialiasing (`SkScan_AAAPath`, `aaa_fill_path`) clip-invariant. It has not arrived. When it does, check it against:
- the corpora (`aa-scan-converters.raw` and the canvas and path corpora, baseline against candidate);
- the oracle;
- the instruction-set comparisons (arm64 on the M4, x86-64 on the RunPod).

Then say whether it needs an `intentional_differences.txt` entry.

## Resume steps

1. Rebase `claude/stx-skia-sk2` (with `-e4` and `-present-workload`) onto `origin/main` at whatever checkpoint the coordinator names. Delete the contained branches.
2. On the M4, finish the stage 8 verification (§1), including the present A/B. Amend stage 8's subject to drop "partly verified" once it passes.
3. On the RunPod: run §2, then the E4 native redo. Update the E4 notes and the gui_forms bounds and docs in a new commit on the E4 chain.
4. On the Windows host: run §3.
5. Report to the coordinator:
   - the branch heads;
   - what ran where;
   - the present before/after for macOS, X11 and Windows;
   - the open items in §4.
