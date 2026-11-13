# font-pack: one compressed file for all bundled fonts

An experiment. It packs every font in `assets/fonts` into one file and loads
that file straight back into the original font files' bytes in memory, ready
for `FT_New_Memory_Face`. It uses no WOFF, WOFF2, Brotli, zlib or any other
existing codec; everything is in `src/`.

Nothing here is wired into GUI.Forms. The format is not frozen.

## Result

Measured on the Mac mini M4 (10 cores, Homebrew clang 22.1.8, `-O2`) with
`sh m4.sh 'sh evaluate.sh'` on 2026-11-13. Input: the 37 fonts of
`assets/fonts` (36 TrueType, 1 OpenType/CFF), 27,888,812 bytes.

| Condition | Measured | Limit |
| --- | --- | --- |
| Pack size / font bytes | 42.54% (11,865,270 bytes) | below 50% |
| Pack time | 37.2 s | 120 s |
| Load, 20 fresh processes, median | 23.5 ms | 50 ms |
| Load, 20 fresh processes, slowest | 29.3 ms | 50 ms |
| Loader scratch / font bytes | 171.9% | 200% |

The same build under ASan and UBSan packed, verified and survived 300 damaged
loads (293 refused, 7 still loaded) with no report.

The loader's output is compared byte for byte with the input files after
every pack (`fontpack pack` refuses to report success otherwise), and
`ft_check` opens all 37 fonts from the loader's memory with FreeType and loads
14,698 sampled glyphs without a failure.

For scale, on the same files: `xz -9e` 50.7%, `brotli -q 11 -w 24` 52.5%,
`zstd --ultra -22 --long=27` 55.9% (TrueType files concatenated, CFF font
alone; none of them loads in 50 ms on one core). `woff2_compress` was not
available on either machine, so there is no real WOFF2 figure; Brotli on the
raw files is the nearest stand-in and lacks WOFF2's glyf transform.

### What the load time means

"Load" is `pack_load`: pack bytes in memory to all 37 fonts' bytes in memory,
including starting and stopping its worker threads. Reading the pack file from
disk is not included.

- Fresh process, machine recently busy: 23 to 27 ms.
- Fresh process after 20 s idle, or the first run of a newly written binary:
  33 to 49 ms observed. Every phase slows by the same factor, so this is the
  machine (cores asleep, code pages cold), not one slow step.
- 4 threads: 35 ms. 1 thread: 126 ms. The 50 ms figure needs about 4 cores.

### What the scratch figure means

Scratch is every byte the loader allocates other than the fonts themselves,
at its peak: decoded streams, sequence arrays, decode tables and per-worker
buffers. The pack's own bytes (the input) are not counted. It is counted by
the loader, not sampled from the process.

## How it works

A font file is taken apart into streams by kind; every font adds to the same
streams, in font order, so a stream is "solid" and a match can reach into an
earlier font. Each stream is then coded on its own.

| Part | What is done |
| --- | --- |
| sfnt | Header and table directory verbatim; gaps between tables as lengths (bytes only if non-zero). |
| glyf, loca | Per glyph: kind, contour sizes, instructions, and one WOFF2-style triplet per point (a flag byte and 1 to 4 payload bytes). loca, the bounding box and the flag run-lengths are recomputed. A glyph the loader would not reproduce exactly is stored verbatim. |
| TrueType instructions | Opcodes and pushed data in separate streams (also fpgm and prep). |
| hmtx | Advances and bearings apart; each bearing as its difference from the glyph's xMin (almost always zero). |
| CFF | Charstrings and subroutines tokenised: one symbol per (operator, argument count), number tokens, mask bytes. The INDEX offset arrays are recomputed. |
| other tables | Verbatim, optionally through a stride filter (each byte minus the byte s places earlier) chosen per table. |

Two codecs code the streams (`fp_codec.hpp`):

**Byte streams.** LZ77 with three repeat offsets and a cost-based parse.
Literals are coded with static rANS tables selected by a context: a class the
loader derives from structure it has already decoded (for example "second
payload byte of a 12-bit triplet"), optionally combined with the previous
literal. Contexts are clustered into at most 255 tables by the packer.

**The CFF number stream.** The same, on whole numbers instead of bytes, with a
second kind of match: a *fuzzy match* copies earlier numbers and adds a coded
correction to each. CJK glyphs reuse the same components with slightly
different coordinates; exact matching misses those, fuzzy matching does not.
A literal number's first byte is coded with a table chosen by (operator,
argument count, argument index); a correction with a table chosen by the same
class and the size of the previous corrections.

Loading is built around a two-phase decode. A stream is cut into chunks whose
entropy-coded parts are independent, so chunks decode on all cores and write
literals straight to their final places; matches may reach anywhere earlier,
so one sequential pass per stream then copies them (for numbers: a `memcpy` or
a vectorised add). Fonts are rebuilt in parallel, the CFF charstrings in
chunks of 256.

Everything the loader reads from the pack is bounds checked.
`fontpack damage` loads randomly damaged packs; under ASan and UBSan it
reports nothing.

## Files

- `src/fp_base.hpp`, `src/fp_format.hpp`: constants, token and triplet grammars shared by both sides.
- `src/fp_codec.hpp`, `src/fp_codec_enc.cpp`, `src/fp_codec_dec.cpp`: the two stream codecs.
- `src/fp_pack.cpp`: font transforms and packing. `src/fp_load.cpp`: the loader.
- `src/fontpack.cpp`: command line (`pack`, `verify`, `bench`, `unpack`, `damage`).
- `src/ft_check.cpp`: FreeType smoke test. `src/lab_codec.cpp`, `src/lab_fuzzy.cpp`: experiment tools.
- `study/`: the Python scripts behind the measurements in "What was tried".
- `m4.sh`: mirrors this folder and the fonts to the M4 and runs a command there. `build.sh`, `evaluate.sh`.

No compiler runs on the laptop; build and run through `m4.sh`:

```sh
sh m4.sh 'sh evaluate.sh'
sh m4.sh 'sh build.sh sanitize && build/sanitize/fontpack pack build/s.fpk fonts/* && build/sanitize/fontpack damage build/s.fpk 300'
sh m4.sh 'build/ft_check build/all.fpk'
```

## What was tried

Sizes are for the whole 37-font set unless a line says otherwise. "CJK
numbers" means the 9.89 million charstring numbers of NotoSansCJKjp, the
largest single component.

### Kept

| Step | Effect |
| --- | --- |
| Operator-first token streams with (operator, count, index) contexts for number first bytes | CJK first bytes 7.09 MB (no contexts) to 6.46 MB, equal to xz on that stream, at rANS speed |
| First full pipeline (byte codec everywhere) | 49.3% |
| One symbol per (operator, count) pair instead of two streams | 1.155 MB to 0.956 MB; 48.6% |
| Fuzzy matches on whole numbers | CJK numbers 8.42 MB to 7.65 MB; 45.8% |
| No limit on consecutive corrected numbers | 7.65 to 7.43 MB |
| Correction contexts from previous corrections | 7.43 to 7.39 MB |
| Corrections of any size; fuzzy matches starting up to 16 numbers before an exact seed | 7.39 to 7.00 MB; 43.5% |
| Stride filter for raw tables | cmap 136 KB to 63 KB, layout 305 to 253 KB |
| hmtx split, bearings relative to xMin | TrueType bearings 113 KB raw to 0.7 KB |
| Correction contexts that include the number's class | CJK numbers 7.00 to 6.90 MB; 42.5% |

### Tried and dropped

| Idea | Finding |
| --- | --- |
| Predicting a coordinate from recent same-axis coordinates (return to a visited x or y) | 3% on the value entropy; not worth the decoder state |
| Contexts from the previous one to three numbers' magnitudes | at most 4% before matching, less after |
| XOR of each hint mask with the previous one | no gain (535 KB vs 538 KB) |
| A table of popular match contents with cheap indices | 82% of matched contents occur once; identity entropy 18.8 bits per match |
| Offsets coded relative to the previous offset | 3.5% of new offsets lie within 8 of the previous one |
| Fuzzy matching for TrueType point deltas | 1.67 MB exact vs 1.64 MB fuzzy in simulation; the styles of a family are not near-repeats |
| Rice or Golomb codes for coordinates | not built: the coordinate distributions are multi-modal per context and static rANS tables already sit on the measured context entropy |
| 16 KB single-lookup decode tables | same speed as 5 KB two-lookup tables, 6 MB more scratch |

### Speed notes

- The largest single win was replacing the rANS renormalisation branch with
  mask arithmetic: it mispredicted on about half the symbols (loader 30 ms to
  21 ms at the time). Writing it as a conditional expression was not enough;
  the compiler turned that back into a branch.
- Literals of a chunk are gathered, decoded in one flat 8-state loop and
  scattered, because literal runs average about ten symbols and an unrolled
  loop per run never gets going.
- A correction's table depends on the previous correction, so that decode is
  a serial chain. Four lanes per chunk run four chains at once, and the
  packer keeps at most 24 correction tables so they stay cache-resident.
- Decoded numbers are kept as 16-bit values, so resolving a fuzzy match is an
  element-wise add.

## Limits and open ends

- The pack is all-or-nothing: no loading one font without decoding the shared
  streams. A per-font or per-family split would cost ratio (matches across
  fonts) and has not been measured.
- A damaged pack is refused, but a header can still claim large decoded sizes
  and make the loader ask for that much memory before it fails.
- Only the fonts in `assets/fonts` have been packed. CFF2, variable fonts,
  TrueType collections and bitmap tables fall back to verbatim tables, which
  is correct but untuned.
- Scratch is 30 points under its limit and the cold-machine load is close to
  50 ms; a machine with fewer or slower cores than the M4 will exceed it.
- CJK hmtx and vmtx bearings come from charstring outlines the loader does
  not interpret; they are stored (through the stride filter) rather than
  derived.
- Per AGENTS.md, measurements belong on the notes repository's `gui_forms`
  branch; this file has not been copied there.

## Provenance (coordinator, 2026-11-13)

Copied from an untracked `experiments/font-pack/` in a gui_forms checkout, where the study ran (Fable sibling,
2026-11-13). `data/` (103 MB of intermediate study dumps) was left out. `m4.sh` reads the fonts from
`../../assets/fonts`, so copy this directory to `experiments/font-pack/` in a gui_forms checkout before running it,
as with `experiments/png-crc-suitability`.
