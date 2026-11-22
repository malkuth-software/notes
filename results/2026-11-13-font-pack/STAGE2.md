# Font pack stage 2: measurements (2026-11-13)

Code: notes `gui_forms-fontpack` @ 5bf77f9, copied into
integration/gui_forms/experiments/font-pack. It builds cleanly with clang 22.1.8,
`-std=c++20 -O2 -Wall -Wextra`: no compile fixes, nothing committed to the branch.

Machine: Mac mini M4 (10 hardware threads), Homebrew LLVM 22. The timing ran with
the M4 cleared of STXlib work (pause confirmed "clear" at 09:43:56 M4 time,
resumed at about 10:52). Load average was about 2 before the start; the only other
process was the GUI.Forms Linux VM, idle.

Input: the 37 fonts of assets/fonts, 27,888,812 bytes.

Commands (from the README's "Integrator steps", each through `sh m4.sh '…'`):
`sh build.sh`; `sh evaluate.sh`; the sanitize step (pack, claims, damage 300);
`build/ft_check build/groups.fpk` and `build/ft_check build/all.fpk`.

## Baseline, single group (the four stage-1 conditions)

| Condition | Stage 2 | Stage 1 | Limit |
| --- | --- | --- | --- |
| Pack size / font bytes | 42.55% (11,865,436) | 42.54% (11,865,270) | < 50%, PASS |
| Pack time | 36.8 s | 37.2 s | 120 s, PASS |
| Load, 20 fresh processes, median | 24.7 ms | 23.5 ms | 50 ms, PASS |
| Load, 20 fresh processes, slowest | 33.8 ms | 29.3 ms | 50 ms, PASS |
| Loader scratch / font bytes | 171.9% | 171.9% | 200%, PASS |

## Grouped pack (fonts.manifest, 5 groups)

The whole pack is 11,853,097 bytes (42.50%), 12,339 bytes *smaller* than the
single-group baseline (-0.10% of it). Pack time is 45.5 s.

| Group | Load | Fonts | Font bytes | Body bytes | Ratio |
| --- | --- | ---: | ---: | ---: | ---: |
| interface | eager | 15 | 4,681,212 | 1,105,066 | 23.61% |
| families | lazy | 15 | 5,581,444 | 1,446,930 | 25.92% |
| scripts | lazy | 5 | 739,616 | 237,217 | 32.07% |
| emoji | lazy | 1 | 418,804 | 243,364 | 58.11% |
| cjk | lazy | 1 | 16,467,736 | 8,819,191 | 53.55% |

Load time per group (median of 25 rounds in one process; "all" = every
thread), with the CRC-32 check's share:

| Group | 1 thread | 4 threads | all threads | CRC check (1 / 4 / all) |
| --- | ---: | ---: | ---: | --- |
| interface (eager) | 20.06 ms | 6.03 ms | 4.19 ms | 1.90 / 0.50 / 0.35 ms |
| families | 23.10 ms | 6.97 ms | 5.10 ms | 2.27 / 0.60 / 0.45 ms |
| scripts | 3.11 ms | 1.23 ms | 1.33 ms | 0.30 / 0.10 / 0.11 ms |
| emoji | 3.63 ms | 2.97 ms | 3.06 ms | 0.17 / 0.18 / 0.19 ms |
| cjk | 88.69 ms | 27.33 ms | 19.60 ms | 6.72 / 1.79 / 1.23 ms |
| eager groups together | 20.03 ms | 6.03 ms | 4.14 ms | 1.90 / 0.50 / 0.34 ms |
| all groups in turn | 139.20 ms | 44.84 ms | 33.46 ms | 11.38 / 3.17 / 2.35 ms |

Fresh-process loads (20 each, every thread): eager groups have a median of
4.5 ms and a slowest of 6.7 ms; all groups have a median of 34.4 ms.

Peak scratch per group, as a share of that group's font bytes (all threads;
1 thread in brackets): interface 178.0% (138.7%), families 180.4% (145.6%),
scripts 293.6% (238.9%), emoji 246.8% (243.1%), cjk 198.9% (191.5%). With
all groups loaded in turn, the largest peak is 32,757,993 bytes, 117.5% of
all the fonts. Peak process RSS is 97,566,720 bytes.

What the CRC check adds: about 8 to 9% of each group's single-thread load
(interface 1.90 of 20.06 ms; cjk 6.72 of 88.69 ms) and 8.2% of all groups at
1 thread (11.38 of 139.20 ms). At all threads it is 7% (2.35 of 33.46 ms).

## Correctness

- Both packs load back to the identical bytes (checked by `fontpack pack`).
- ASan + UBSan: the grouped pack and verify pass; `claims: 201 cases, 0
  failures`; `damage: 300 rounds, 83 refused at the index, 423 groups
  refused, 662 groups loaded (each checked by CRC-32)`; no sanitizer report.
- FreeType: `ft_check` opens all 37 fonts and loads 14,698 sampled glyphs
  with 0 failures from both packs. Pack to fonts takes 56.03 ms from the
  grouped pack (all groups) and 24.48 ms from the single-group pack.

## Notes

- The single-group slowest fresh load moved from 29.3 to 33.8 ms; the median
  moved 1.2 ms, the order of the added check (about 2.4 ms at all threads for
  the whole set). Still within 50 ms.
- At all threads, scripts and emoji are no faster than at 4 threads: these
  small groups are bound by thread start-up and single streams.
- Raw outputs: the other files in this folder (evaluate.out, evaluate-*.txt,
  bench_groups.txt, ft_*.txt, san_*.txt).
