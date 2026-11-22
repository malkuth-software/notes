# Open gaps

Date: 2026-11-22. This page lists every open gap, so that none lives only in
a chat, a scratch file or a worker report. Each row names its owner (the
worker or role that closes it), its status and its place: do now, before
1.0, or after 1.0. `planning/ROADMAP_TO_1.0.md` places the larger items in
blocks; this page is the checklist. When a gap closes, mark it here with the
commit or PR, and leave the row until the next record refresh removes it.

This revision comes from the gap refresh of 2026-11-22, which checked each
row of the 2026-11-10 list against gui_forms `main` c7c9f4f0c (PR #488).
It also applies the owner's rulings of the same day. 37 of the 54 rows are
gone: they are closed, abandoned or not planned (see "Closed since
2026-11-10" below).

PR key: #480 cp27, #481 cp28, #482 cp29-31, #483 cp32, #484 cp33, #487 and
#488 cp34 and cp35.

## Text and fonts

| # | Gap | Owner | Status |
|---|---|---|---|
| F5 | The control face rendered "a" closed into "o" at 12.5 units at 1x ("macOS" read "mocOS"). The rasterizer has been replaced since then: STXlib's glyph engine replaced FreeType and HarfBuzz in #483. | engine | check with a capture of "macOS" at 12.5 units at 1x; drop the row if it does not reproduce |

## Painting and hosts

| # | Gap | Owner | Status |
|---|---|---|---|
| P1 | Scroll copies on Windows and the X11 host are coded (#480, #483) and compile on every host in CI, but they have never been run on Linux or macOS. CI is compile-only. | Mac / Linux | housekeeping: one run of `linux_host_tests` and `host_scroll_damage_tests` |
| P2 | ThreadSanitizer run of the ABI sub-surface tests (audit N9). The cp29 block passed on 2026-11-11. Since the tests split, `tools/abi_tsan.py` packs a tsan cache that gui_forms_tests runs (e92f02256, #488), but no run is recorded. | Mac / Linux | housekeeping: one run after the split |
| P3 | DIB fixture: on a private desktop, the fixture's later step fails with "initial Window receipt not accepted" (`windows_dib_lifecycle_fixture.inc:100`, unchanged). | Windows | fold into the one Windows session |
| P5 | PlaySuite tile hover blink. The owner does not recall whether it still happens. | owner, PlaySuite | verify with a capture; drop the row if it does not reproduce |
| P7 | A tooltip prebake can show the previous theme once: `ToolTipBakeKey` (`src/core/window/tool_tips/tool_tip_bubble.hpp:56-68`) has no theme identity. The compile half of the old row is closed (#483). | engine | being fixed; before 1.0 |

## Themes and look

| # | Gap | Owner | Status |
|---|---|---|---|
| T2 | The windows-professional redesign is in #481 and the owner's captures are made. The site recapture is still owed, and the dark variant's light plates await owner review. | site | housekeeping |
| T3 | Look ledger: only T12 is left, the audit that only progress bars animate ambiently. T1, T13, R17 and R24 are done (#483). | theme | before 1.0 |
| E8 | The industrial look as a stock theme. The engine side is in: finish kernel (#482), light planes, carbon fibre and lamps (#488). The theme itself is a Gallery demo theme (`demo/themes/industrial`), with no `FamiliarTheme` entry. | theme | before 1.0, after the industrial work lands |

## Host services (new, 2026-11-22)

| # | Gap | Owner | Status |
|---|---|---|---|
| HS1 | Drag and drop with the host. **Outbound:** drag sources on macOS, Windows and Linux (an XDND source; Linux had none). **Inbound:** drag and drop on Windows ("we do use this or need it"). A drag that starts while another is running is refused with `modal_limit`. If HostServices itself is destroyed mid-drag, the drag ends without a report. | host worker | in progress, `claude/host-drag-and-drop`; next checkpoint |
| HS2 | Host autocomplete and platform suggestion services. IME already works on all three hosts. | host | after 1.0 |

## Build, CI and records

| # | Gap | Owner | Status |
|---|---|---|---|
| B1 | CI area selection does not give a tools-only change an area of its own: `tools/test_areas.json:36` maps `tools/` to `docs`. The tests have left gui_forms and CI is compile-only, so the map has little effect now. | CI | after 1.0 |
| B2 | The release notes stop at #486 (cp34 and cp35 are not described). `docs/dev/roadmap.md` still calls MDI and the DataGridView members deferred. | docs | being fixed on `claude/docs-housekeeping` |
| B3 | Plan Paint: 22 language packs carry machine translations of 2 new strings, and no native-speaker check is recorded. | Plan Paint / owner | after 1.0 |

## Not planned

| # | Gap | Ruling |
|---|---|---|
| H4 | Device printing for the help system. `print_requested` is raised and `help_print.hpp` paginates, but there is no print path. The drag-resizable navigation pane is done (9ac2a621b, #483). | Not planned (owner, 2026-11-22). A CHM-style help product with its own menu may come later, possibly outside GUI.Forms. |

## Closed since 2026-11-10

Each closure below was checked in the code at main c7c9f4f0c.

- **Fonts.**
  - F1, F2, F4: the font family registry (6ae847879, aca87c4df; #483).
  - F3: font pack stage 4 (b7d53b109, #483). The products are pinned after it.
- **Painting.**
  - P4: local menu repaint (5cb16aca2, #480).
  - P6: the invalidation overlay (4fa3dfaa, #480), compiled on every host since #483.
- **Themes.**
  - T1: the high-contrast gauge track (a916d80e0, #483).
  - T4: the ribbon help button (aab777599, #483).
- **Help.**
  - H1: language files (58f763efd, #482).
  - H2: Unicode search folding (d693147d9, #483).
  - H3: the Shift+F1 cursor, and the document and heading roles (14b79d8d7, #483).
  - H5: help-book captures (4f5a21702, #483).
- **Final controls.** W1:
  - W-1: the public bases (1da03e3f4, #482).
  - W-2: DataGridView breadth (7e5862d32, 157135bf5, b534e3aae; #483).
  - W-3: MDI (9ce667dae, d74b59ebc; #488).
  - W-7: menu first-letter jumps (61706b1e3, #483).
  - W-8: file dialog checks (6e30c7b21, #483).
- **Exhibits.**
  - E1 to E4, E6 and E12: 426c1a32a (#483).
  - E5 and E9: eed139627 (#481).
  - E7: curved paths (dfd62912a, #483).
  - E10: stock icons (739236a2e, #483).
  - E11: the host locale (dfd62912a and 426c1a32a, with the Windows locale in 8c86ef5b5; #483).
- **Library rows of the swap-in.**
  - S5: editable combo box editing (12dc37867, #480).
  - K2: floats shown while inactive (94befba1b, #480).
  - K4: the docking library was merged in #480 and has been gated and regenerated since.
- **Records.**
  - B4: the products are pinned at 0ebb04d65 (#484).
  - B5: the audit rulings are recorded in `planning/audits/README.md` (2026-11-22).

**Abandoned (owner, 2026-11-22: "lets stop on sdrsharp. we wont touch it
anymore"):** the SDR# swap-in rows S1 to S10, and K1, K3 and K5. The library
halves that had landed stay. S2's submenus landed in 4292ee6d0 (#488), and
MDI, which K5 could have used, landed in #488.
