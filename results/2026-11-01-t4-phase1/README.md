# T4 phase 1: census and contrast baselines (2026-11-01)

Machine: Shadow Windows x86-64 build host, LLVM 22.1.8 (MSYS2 CLANG64),
Release, Skia off. Tree: gui_forms `claude/t4-themes` 03efd251 on
checkpoint/2026-11-01-4 (local, not pushed).

## Paint census

    python -B tools/paint_census.py scan     # paint_census.txt
    python -B tools/paint_census.py update   # paint_census.json

| Class | Files | Direct drawing calls |
|---|---:|---:|
| stock | 47 | 400 |
| demo (Gallery code) | 3 | 57 |
| exempt (DrawingSurface, RasterCanvas) | 2 | 5 |

The T4 gate is zero stock calls; `check` refuses any rise.

## Contrast report of `windows-professional`

    GUI_FORMS_CONTRAST_REPORT=contrast_windows_professional.tsv gui_forms_theme_contrast_tests

1688 rows (14 families x 7 surface states x selected or not x ordinary and
high contrast, with the pairs each gates): 1222 pass, 466 fail, 0 not
measured. Failures by pair: text 55, muted text 376, glyph 15, boundary 19,
focus inner 1, focus outer 0. A measurement of recipe colours (faces sampled
at 17 gradient positions over the panel surface); phase 2 adds the pixel
measurement where text sits.

Notable: the accent button's hot, pressed, pending and invalid plates are
near-white under white text (1.0 to 1.3:1). Muted text (118,128,138) fails
4.5:1 on almost every face.
