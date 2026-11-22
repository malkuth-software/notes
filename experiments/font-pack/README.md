# font-pack: one compressed file for all bundled fonts

An experiment. It packs every font in `assets/fonts` into one file and loads
that file straight back into the original font files' bytes in memory, ready
for `FT_New_Memory_Face`. It uses no WOFF, WOFF2, Brotli, zlib or any other
existing codec; everything is in `src/`.

Nothing here is wired into GUI.Forms. The format is not frozen.

Stage 2 (2026-11-13, code only, not yet built or measured) splits the pack
into independently decodable groups, bounds every size claim before
allocating, and checks every rebuilt font against a stored CRC-32; see
"Groups", "Bounded claims" and "Integrator steps". The figures under
"Result" were measured on the stage-1 format (one solid set of streams, no
check values) and must be measured again on stage 2.

## Result (stage 1)

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

## Groups

The owner's decision (2026-11-13): the pack is embedded in the library; the
interface faces decode eagerly, every other group lazily on first need.

A pack (`FPK2`) is a container of groups. Each group is a complete set of
the streams above over its own fonts only: matches, contexts, clustered
tables and every shared stream reach within the group and never across, so
any group decodes alone, and decoding a later group never decodes an
earlier one again. The packer chooses the groups from a manifest
(`fonts.manifest`, read by `fontpack pack --manifest`): `group <name>
eager|lazy` lines, each followed by file name patterns; a font joins the
first group with a matching pattern, and a font matching no group, or a
group matching no font, stops the pack. Without a manifest the packer
writes one eager group `all`, the single-group baseline the split is
measured against.

The proposed split (for measurement, not decided): `interface` (eager:
every Portsmouth face, Portsmouth Mono, and Carlito, the content family),
`families` (Arimo, Cousine, Tinos, Selawik), `scripts` (Noto Arabic,
Hebrew, Devanagari, Bengali, Gurmukhi), `emoji`, `cjk`.

Layout (all container integers little-endian or varints):

| Part | Contents |
| --- | --- |
| magic | `FPK2` |
| group count | varint, 1..16 |
| per group | name (varint length, bytes); flags (u8: 1 eager, 0 lazy); font count; font bytes (sum of the font sizes); stream bytes (sum of the decoded stream sizes, a CFF number counting 2); body size; then per font: name, size (varint) and the font file's CRC-32 (u32) |
| per group, in order | the body, then 32 padding bytes (the decoders read up to 32 bytes past a stream) |

A body is each font's start in every stream (33 varints per font), each
stream's coded size (33 varints), then the coded streams. Font names and
sizes live only in the container, so `pack_open` can list every font and its
group without decoding anything.

Loader interface (`src/fp_load.hpp`): `pack_open` reads and checks the
container into a fixed-capacity `PackIndex` and allocates nothing;
`pack_load_group` decodes one group into storage the caller owns and says
why when it refuses (bad index, bad claims, bad streams, bad check value, no
memory); `LoadedSet` is the application's view: `set_open` decodes the eager
groups, `set_find_font` decodes a lazy group on the first request for one of
its fonts and never again (a refused group is not retried).

### Bounded claims

Every decoded size is checked before anything is allocated for it:

| Limit | Value | Checked |
| --- | --- | --- |
| groups | 16 | `pack_open` |
| fonts, whole pack | 256 | `pack_open` |
| name bytes | 128 | `pack_open` |
| one font | 64 MiB | `pack_open` |
| one group's fonts | 256 MiB, and exactly the sum of its font sizes | `pack_open` (64-bit sum, at most 2^34) |
| one group's decoded streams | 4 x its font bytes, and exactly the sum of the sizes its streams declare | `pack_open`, then `pack_load_group` before any stream opens |
| decode ratio | a group's font bytes at most 64 x its body size | `pack_open` |
| stream chunk records | at most the remaining header bytes / record minimum | `stream_open`, `number_open` before the chunk array |

The ratio limit ties every claim to bytes actually present, so a short
damaged pack cannot claim a large group (the bundled set packs at 2.35x).
The font storage is allocated only after the rebuild plan has tied every
font's size to its decoded recipe (table placement and tail, or a verbatim
font's extent), so a size claim within the limits that the streams do not
bear out is still refused before the storage exists. The packer enforces
the same limits, so every pack it writes loads.

`fontpack claims` checks this: per group it writes packs with one claim
wrong (a font size of 2^31 - 1 or + 4096 with the group total to match, the
group total + 1 or above its limit, the stream total + 1 or above its limit,
each stream's own declared size raised in place, a flipped CRC-32, and a
group count of 127), and requires each to be refused at `pack_open`, or by
`pack_load_group` with no font storage asked for and no more scratch than
the genuine load (single-threaded, so the comparison is exact), or, for the
CRC case, as a bad check value.

### Check values

Each font's CRC-32 (the zlib, PNG and stx_zlib polynomial) is stored in the
container and checked after the font is rebuilt; a mismatch refuses the
whole group. `src/fp_crc.hpp` is a slicing-by-8 stand-in for
`stx_zlib::crc32` until stage 3. Fonts are checked in 1 MiB pieces on all
workers and the pieces' values combined (`crc32_combine`), so the CJK font
does not check on one core; the check time is reported separately
(`check` in `fontpack bench`).

## Files

- `src/fp_base.hpp`, `src/fp_format.hpp`: constants, container layout and limits, token and triplet grammars shared by both sides.
- `src/fp_crc.hpp`: CRC-32, a stand-in for stx_zlib::crc32.
- `fonts.manifest`: the proposed groups.
- `src/fp_codec.hpp`, `src/fp_codec_enc.cpp`, `src/fp_codec_dec.cpp`: the two stream codecs.
- `src/fp_pack.cpp`: font transforms and packing. `src/fp_load.cpp`: the loader.
- `src/fontpack.cpp`: command line (`pack`, `verify`, `bench`, `unpack`, `damage`, `claims`).
- `src/ft_check.cpp`: FreeType smoke test. `src/lab_codec.cpp`, `src/lab_fuzzy.cpp`: experiment tools.
- `study/`: the Python scripts behind the measurements in "What was tried".
- `m4.sh`: mirrors this folder and the fonts to the M4 and runs a command there. `build.sh`, `evaluate.sh`.

No compiler runs on the laptop; build and run through `m4.sh` (see
"Integrator steps").

`fontpack bench <pack> [--threads 1,4,0] [--rounds N] [--baseline <single.fpk>] [--only <label>]`
prints the pack ratio per group (body against fonts) and for the whole set,
and with `--baseline` the difference against the single-group pack; then,
for each thread count (0 is all cores), load time (min, median, max over the
rounds), peak scratch and check time for each group alone, for the eager
groups together and for all groups one after another. `--only` measures one
of those labels (`evaluate.sh` uses it for fresh-process loads).

## Integrator steps

Stage 2 is unbuilt. `m4.sh` reads the fonts from `../../assets/fonts`, so
first copy this directory to `experiments/font-pack/` of a gui_forms
checkout the integrator owns (not a shared one), then run from there, one
at a time (each run resynchronises the M4 mirror):

```sh
# 1. Build (optimised tools, and ft_check when FreeType is installed).
sh m4.sh 'sh build.sh'
# 2. Evaluate: packs build/all.fpk (single group, the four stage-1 conditions)
#    and build/groups.fpk (fonts.manifest), verifies both byte for byte, times
#    20 fresh-process loads of each (all groups, and the eager groups alone),
#    and writes the full grouped bench to build/bench_groups.txt.
sh m4.sh 'sh evaluate.sh && cat build/bench_groups.txt'
# 3. Sanitizers: ASan + UBSan pack, claims and 300 damaged loads.
sh m4.sh 'sh build.sh sanitize && build/sanitize/fontpack pack build/s.fpk --manifest fonts.manifest fonts/* && build/sanitize/fontpack claims build/s.fpk && build/sanitize/fontpack damage build/s.fpk 300'
# 4. FreeType: every font of every group through FT_New_Memory_Face, groups loaded on demand.
sh m4.sh 'build/ft_check build/groups.fpk && build/ft_check build/all.fpk'
```

Expected: step 2 prints PASS for the four baseline conditions (the stage-2
check step adds a little load time; record it if a condition moves), step 3
ends `claims: N cases, 0 failures` and prints `damage:` with no sanitizer
report, step 4 ends with `0 failures`.

The measurements belong in the notes repository under `results/` (for
example `results/2026-11-13-font-pack/`), with the commands, the machine
and the raw outputs (`build/evaluate-*.txt`, `build/bench_groups.txt`), not
in this README. Once measured, add the group split to "What was tried"
(ratio cost against the baseline, eager load time at 1, 4 and all threads,
scratch per group) and update "Result".

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

- Groups decode whole: loading one font decodes its group's shared streams.
  The split's ratio cost and start time are not measured yet (stage 2).
- The bounds argument above is written out, not proved; stage 3 states it as
  the format's invariants (concern 7 of the integration plan).
- No Cairo or poster faces are in `assets/fonts` today; the manifest has no
  group for them.
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
