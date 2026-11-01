# T3 theme language gate (P11), 2026-11-01

GUI.Forms branch `claude/t3-p11` at `cf701bc9` (on P12b `07ddedcf`, on main
`6856347b`). Host: the M4 (arm64 macOS), Homebrew clang 22.1.8.

## What ran

- `python3 tools/ci_build.py --jobs 8 --gallery [--sanitize] --tests
  'gui_forms_theme_|gui_forms_engine_|gui_forms_gallery_pages_tests|gui_forms_basic_controls_tests|gui_forms_part_painting_tests|gui_forms_composition_controls_tests|gui_forms_shared_resources_tests|gui_forms_layout_style'`:
  55 of 55 pass plain and under ASan, UBSan and LSan. The house-style and
  lifetime checkers, the docs check, the schema and schema-render checks and
  the state-machine gate (`--changed-from origin/main`) pass.

## Gate items (PLAN section 6)

1. **Conformance**: `tests/theme_language/conformance/`, 65 fixtures with
   their expected compiled records (selector, specificity, declarations with
   program opcodes, transitions, ambient, material digest, recipe, custom
   property definitions and reads) or diagnostics. `COVERAGE.tsv` lists each
   form with its fixture; `gui_forms_theme_conformance_tests` fails when a
   form has no fixture, a fixture lists no row, or a code 1..1999 has no
   fixture or `code-elsewhere` test. Every fixture is also checked for engine
   acceptance and a byte-identical round trip. **Gap**: drawing rows carry no
   raster reference hash yet; the material digest stands for it.
2. **Malformed input**: one fixture per error and warning code reachable from
   a sheet, checking code, line and column; refused sheets are checked to
   leave the running theme and its images untouched. Codes reached only from
   packages, imports or inline styles are covered by the tests the table
   names. E408 is unreachable from a sheet (engine input tests cover it).
3. **Hostile input**: `gui_forms_theme_hostile_tests`, each limit at its
   value compiles and one past it is refused with its code: sheet size, rule
   count (with expansion), 64 declarations, 32 nesting levels, 4 KiB strings,
   the expression budget and stack, 32 `:is()` arguments, 32 simple
   selectors, 1,024 custom properties, 4,096 substituted tokens, 128 copies
   per rule (7 names compile, 8 refused), 64 scoped names, 64 keyframes,
   1,000 diagnostic records. **Gap**: no work counter exists, so "a fixed
   count of operations per input byte" is not measured (open decision).
4. **Fuzzing**: `fuzz/theme_sheet_fuzz.cpp` under libFuzzer, ASan and UBSan
   (`-fno-sanitize-recover=undefined`, LSan on), seeded with the 65
   conformance fixtures copied to `fuzz/corpus/theme_sheet/`:
   **2,429,002 inputs, 0 failures, corpus grown to 2,214 entries**,
   `-max_len=65536 -timeout=10 -rss_limit_mb=0 -malloc_limit_mb=2048`.
   The RSS limit is off because libFuzzer's own RSS watch thread is reported
   as a leak at exit on macOS (seen in a first run of 2,543,155 inputs with
   no other finding); no suppression was used. The standalone build is the
   ctest `gui_forms_theme_sheet_fuzz_tests`.
5. **Round trip**: every fixture, through the conformance test. The T4
   default theme's sheets do not exist yet.

User documentation: `docs/user/theme-language.md`.

## Plan and build differences found

- `backdrop-filter` is `E999 not_yet_built`; the plan lists it as present.
- A gradient at the pointer (the `sheen` lowering) is refused with E405.
- `felt()` and the procedural backgrounds are W103 until registered.
