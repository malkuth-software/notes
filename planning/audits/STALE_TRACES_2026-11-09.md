# Stale traces sweep, 2026-11-09

The owner asked: "dont we have out of date documentation traces?" This
sweep looks for statements that describe a GUI.Forms limitation,
workaround or stand-in, and checks each against current main
(e1fc3bf68, cp25 #475 merged) and the cp26 fold (D:/gui_forms_w3/fold23,
e17e5054a, not merged).

## Method

The search pattern was a regular expression over: until GUI.Forms / until
... lands / once ... lands, for now, not yet, stand-in, workaround,
temporary stand-in or shim, "GUI.Forms lacks / has no / does not", no ...
yet, TODO, FIXME, remains pending, pending on GUI.Forms.

- **Products** were searched in their working trees (worker edits
  included). Hits were kept only where GUI.Forms was named within one line.
- **GUI.Forms and the notes** were searched in full. The notes were read at
  `origin/gui_forms`.
- **Excluded:** `vendor/`, `third_party/`, `incoming/`, `experiments/`,
  language catalogues, and dialogue assets (PlaySuite's `assets/lines` are
  Eggy's speech).
- **Not edited:** dated records (notes `experiments/`, `results/`,
  `interviews/`, `studies/`, dated stage studies and track logs) are
  history and are left as they are.

Most hits are a product's own status ("GUI integration remains pending"),
or current-state mechanics ("a redraw not yet sent", `*_pending()`), rather
than GUI.Forms traces.

**Key to the cp25 and cp26 split:**
- **On main (cp25, #475):** the key and numeric ABI (NumericUpDown range in
  the schema), TextBox styled runs, the focus stack (modal and owned
  windows, window properties a5cdb1b19, focus: `windows.focused_control`,
  `Control::focus_target`), the host file gaps (1744d23b3), plain tooltips
  and hover leave (c6baab4df), RichTextBox and RTF conversion (9e301b637,
  earlier), GridPanel.
- **cp26 only:** the data grid ABI, the ribbon gaps, the look
  integration, the drag engine, the value bar, the drawing primitives and
  plots.

## Counts

| Repo | Pattern hits | GUI.Forms claims checked | Now false | Partly | Still true | Fixed here |
|---|---|---|---|---|---|---|
| swiftedit | 83 | 4 | 1 | 0 | 3 | - (route) |
| plan-paint | 10 | 1 | 0 | 0 | 1 | - (route) |
| playsuite | 159 | 9 | 1 | 3 | 5 | - (route) |
| malkuth-tools (swapin copy) | 11 | 3 | 1 | 1 | 1 | - (route) |
| probes/swapin | 15 | 4 | 1 | 1 | 2 | - (route) |
| gui_forms (fold23) | 384 | 9 | 1 | 2 | 6 | - (route) |
| notes (origin/gui_forms) | 181 | 10 | 4 files | 0 | 6 | 4 files, `claude/stale-traces` 6a76c78 |
| template_harness | 15 | 2 | 1 | 0 | 1 | 1, `claude/stale-traces` 8b99542 |

## Route to the workers (not edited)

### SwiftEdit (D:/malkuth/workspaces/swiftedit)

| File:line | Text | Verdict | Fixed by | Corrected wording |
|---|---|---|---|---|
| themes/swiftedit/swiftedit.theme.css:6 (committed HEAD 32cf3d9 and origin) | "Until GUI.Forms' text controls paint styled runs, only the status lines ... use this sheet" | **now false** | TextBox styled runs, cp25 #475 | Already rewritten in the worker's uncommitted tree ("GUI.Forms has no custom roles, so StylePalette ... carries them"), which is true. Commit it. |
| docs/KNOWN_GAPS.md:40-42 | "painted as GUI.Forms styled runs from a palette SwiftEdit holds; GUI.Forms 0.5 has no custom theme roles" | still true | - | `ColourRole` is still a closed enum (theme_rules.hpp:297); custom properties (P12) exist but are not roles. |
| docs/KNOWN_GAPS.md:36 | "The current document TextBox still lacks the required visible-control glyph view" | still true | - | No control-character glyph mode in TextBox on main or cp26. |
| docs/MACOS_BUILD_HANDOFF.md:21 | "no new prepared-text SDK is available yet" | unverified; probably obsolete | - | A 2026-10-01 handoff, before the source SDK. Suggest marking the section historical. |

### Plan Paint (D:/malkuth/workspaces/plan-paint)

| File:line | Text | Verdict | Corrected wording |
|---|---|---|---|
| tests/forms_tests.cpp:5489 | "gf::Ribbon centres a two-row stack in the plane (reported to GUI.Forms; no workaround here). Until it can align ..." | still true | The rows are still centred (ribbon_scaling.cpp:513, cp26 included). Keep it. |

### PlaySuite (D:/malkuth/workspaces/playsuite)

| File:line | Text | Verdict | Fixed by | Corrected wording |
|---|---|---|---|---|
| new-games/guide/04-controls.md:199 (and :19 "shorter until the declarative...") | "One declarative grid (rows and columns as `"34 auto * 34"` ...) \| Not yet" | **now false** | GridPanel on main (`grid_panel.hpp`: explicit and implicit tracks, named areas, flexible sizing) | "GridPanel (`grid_panel.hpp`) \| Available \| the capsule and shelf still use hand-written `arrange`; moving them is PlaySuite's choice" |
| docs/TOOLKIT_REQUESTS.md:222-238 "Hover when the pointer leaves" | "No host clears hover on window exit ... it cannot see the window exit" | **partly** | c6baab4df: a host `leave` now clears hover and sends `leave` (window.cpp:1723) | Add: "*Window exit answered by GUI.Forms c6baab4df (a host leave clears hover). The hidden-or-disabled-while-hovered case is unverified; keep `ShelfBox::forget_pointer` until checked.*" |
| docs/TOOLKIT_REQUESTS.md:210-220 "Live surface presentation" and item 5(b) | "the macOS host copies the whole control area ... on every new generation"; "no clear under an opaque one" | **partly** | 92c1065a6 "opaque live surfaces and skip covered Mac retained paints"; `src/host/macos/application/live_surface_damage.hpp` | Add an answered note citing 92c1065a6. Item 5(a), scaled small surfaces, is still open. Re-measure before closing. |
| docs/TOOLKIT_REQUESTS.md:3 | "the pinned toolkit (`7b260cf`)" | partly | the cp22 port (82906c2) | "the toolkit as pinned when each entry was found (each entry dates itself)" |
| src/open_link.hpp:4; docs/TOOLKIT_REQUESTS.md:204 | "GUI.Forms has no such service yet" (open a URL) | still true | - | No host URL service on main or cp26. |
| src/main.cpp:212; tests/playsuite_theme_tests.cpp:38 | "no GUI.Forms palette role" / "no application palette roles" | still true | - | Closed `ColourRole`. |
| src/suite.hpp:98; TOOLKIT_REQUESTS theme gap 1 | "the theme has no app-defined marks yet" | still true | - | No mark registration on main or cp26. |
| docs/TOOLKIT_REQUESTS.md item 1 | "`FontRole` has only `control`, `content` and `monospace`" | still true | - | paint_types.hpp:74. |
| docs/TOOLKIT_REQUESTS.md "Theme gaps" 2-3 | keyline inset and fade, `border-position` | still true | - | Not in theme_rules on main or cp26. The look integration (cp26) should be rechecked when it merges. |

### Swap-in (D:/malkuth/probes/swapin, mirrored in malkuth-tools/swapin)

The SDR# worker is active in both trees.

| File:line | Text | Verdict | Fixed by | Corrected wording |
|---|---|---|---|---|
| README.md:153-156 | "GUI.Forms' `numeric_up_down` publishes no minimum, maximum, increment or decimal places in the schema" | **now false** | cp25 #475 (key and numeric ABI: "the NumericUpDown range as schema properties") | "NumericUpDown's range is in the schema since cp25; mirror it" (and drop the text-field stand-in once the facade reads it) |
| facade_v1/System.Windows.Forms/StableBridge.cs:830 (and :841 Gap text) | "(the stable windows table has no title property once it runs)" | **partly** | a5cdb1b19, window properties (`text`) in ABI 1.1, cp25 | "(a library before window properties has no title once the window runs; 1.1's `text` window property sets it)". The code already calls SetWindowText when it can. |
| facade_v1/.../StableBridge.cs:2431 | "a popup menu (C overlays, not yet ported)" | still true (a facade gap) | - | `GF_TABLE_OVERLAYS` exists; the facade has not ported it. |
| README.md:151 | ComboBox "items and selected_index not yet mirrored" | unverified | - | Check against main's focus and combo work (cp25). |

### GUI.Forms (D:/gui_forms_w3/fold23; route to the owning workers)

| File:line | Text | Verdict | Fixed by | Corrected wording |
|---|---|---|---|---|
| file_manager_demoboard/src/fixture_model.cpp:268 | FM-W03 "custom host chrome/title participation is not yet public" | **partly** | the window band (e26ebe817), window properties (a5cdb1b19) | "measured-partial: window band and Form chrome are public; title-bar content participation is not" |
| include/gui_forms/controls/docking/dock_site/dock_site.hpp:296 | "the look ... is the default theme's (T4); until then it paints through paint_part with today's recipes" | partly | the gui-forms default theme (cp24) | No dock-caption rule was found in the built-in theme, so the "until" may still hold. The DK worker should confirm and either drop "until then" or name the missing part. |
| docs/user/theme-language.md:471-477 "Not yet built" | `backdrop-filter` refused; pointer gradient; procedural backgrounds warn | partly | - | Recheck after the look integration (cp26) merges; procedural fills exist in `surface_material/procedural_fill`. |
| demo/pages/responsive-layouts.form:11 | "The page keeps in code what a form does not yet express" | still true, as listed | - | - |
| docs/user/for-windows-forms-developers.md:180-185 | auto-complete sources unsupported; no outbound drag; no native menu-bar publication | still true | - | No host implements `query_auto_complete`; there is no drag source. The macOS host has only the application menu. |
| docs/user/input-and-focus.md:130 | "Outbound drag initiation is not yet available on any host" | still true | - | The drag engine (cp26) is in-window. |
| include/gui_forms/controls/ribbon/ribbon.hpp:311 | "Not yet: hosting the QuickAccessToolbar on the tab row" | still true | - | Roadmap 2.11. |
| include/gui_forms/controls/panel/text_box/auto_complete/auto_complete.hpp:27; host_services.hpp:79; host_types.hpp:224 | "which no host supplies yet" | still true | - | - |
| include/gui_forms/c/gf_abi.h:375, 428 | "a property of an object value has no gf_value form yet" | still true | - | - |

## Fixed directly

**Notes** (`D:/malkuth/notes_stale`, branch `claude/stale-traces` from
`origin/gui_forms`, 6a76c78):
- `planning/ROADMAP_TO_1.0.md`:
  - the status paragraph said "cp25 is folding". It now says that cp25
    (#475) merged and names what landed, and that cp26 folds the rest;
  - "Coded, waiting in cp25" becomes "folding in cp26";
  - "PlaySuite and SwiftEdit build once cp25's ... land" now says they can
    build against main;
  - rows 0.8, 2.1-2.3, 3.0, 3.5, 4.1 and 4.11 are updated to the actual
    split (the focus stack, including focus, is on main).
- `planning/stages/T3-theme-language/PLAN.md:4` and `README.md:7`: "No
  parser code exists yet" now reads "existed then", with "T3 is complete,
  7c25c6fc5".
- `planning/stages/RT-rich-text/PLAN.md:50`: "GUI.Forms has no RTF code"
  now reads "had then", with "RichTextBox over markdown and RTF conversion
  landed in 9e301b637".
- Checked and still true: `PAINT_PIPELINE_AVAILABILITY.md:144` (no host
  offers the presentation extensions), `coordination/tracks/stage_a.md:51`
  (NumericUpDown has no large step), roadmap 2.11's ribbon "not yet".

**template_harness** (`D:/malkuth/th_stale`, branch `claude/stale-traces`
from main, 8b99542):
- `docs/adopting.md:289` claimed two things. The first was "The harness has
  no CI of its own yet". It now has `bootstrap.yml` and `example.yml` on
  push and pull request. The second was "GUI.Forms' house-style job also
  runs the pinned harness's whole test suite". GUI.Forms' native job runs
  only `test_ci_reuse.py`.
- The paragraph now says both, and tells adopters to run the whole suite
  before a bump.

## Not covered

- Product hits that describe the product's own unfinished work (SwiftEdit's
  "GUI integration remains pending" family, Plan Paint's preset notes).
- GUI.Forms hits that are current-state mechanics.
- Notes records listed under Method as dated history.
