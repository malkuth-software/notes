# Look integration ledger

## RESUME (2026-11-09 night, head f528d4f76 on main def3b2df6)

- Rebased onto main def3b2df6 (panel tabs and labelled fields joined by hand); not built after the rebase, by the owner's plan (one combined build).
- 2016ad143 hosted check box sized as the CheckBox lays itself out (R48); 685488bce up-down fields in the control plane, so their spin buttons show where they are pressed (R49), with raster label-fit checks (R50).
- Decision pending (Splash phase 2): menus as a Splash kind contradicts Part S (owner 2026-11-08: menus are Popups); worker baking not started.

## RESUME (2026-11-09 evening, head 15f90ab3b)

- 15f90ab3b: one rounded, tapered outline for the tab face, its line and its shadow (the owner's top-corner bump and the pale fringe). 6x corner crops: screenshots/corner_{before,after}_scale{1,2}_6x.png.
- Branch claude/look-integration, head 7215ae906 on base 25cb93999, four commits since 3ac1a577e: Skia scale resampling, items 10-13, overlapping tabs and dither hover, gallery whole rows. Full suite 529/529 before the last commit; that commit's run was cut short (its own test ribbon_gallery_tests added).
- 3x crops: C:/Users/Shadow/gui_forms/screenshots/look_after_tab_crop_3x_{light,dark}.png; scale captures look_after_ribbon_scale{1,2}.png.

## RESUME (earlier, 2026-11-09)

- Branch claude/look-integration in D:/gui_forms_w1/gui_forms, base origin/main c9b11f0a, head dfa43cb41 (five commits), registered in INTEGRATION_QUEUE.md. Built locally on Windows: the whole suite passes, 528 of 528.
- Captures (before = c9b11f0a, after = dfa43cb41; both on the gui-forms base through a local, uncommitted capture patch): C:/Users/Shadow/gui_forms/screenshots/look_{before,after}_{commands-and-menus,docking-workspace,text-and-input}_{light,dark}.png, the 3x tab crops look_{before,after}_tab_crop_3x_{light,dark}.png, and look_{before,after}_search_field_crop_3x_light.png.
- Sources read: RIBBON_STUDY, RIBBON_SCALING_STUDY, RB PLAN and README, RIBBON_INTERVIEW_ROUND_001, DESIGN_LANGUAGE_SPECIFICATION (§11 and §12), DESIGN_LANGUAGE_SYSTEM_MODEL, T3 README and PLAN, T4 README, RENDERING_REFERENCE, the t3 and t3-round2 answers, DOCKING_STUDY (§8 J and the taste questions), DOCKING_INTERVIEW_ROUND_001 (parts 5 and 15), PAGEVIEW_STUDY, and the Downloads answers (t2, t5, w, l, open-calls, plan-paint-icon).
- Not read: DOCKING_LOOK_PAGE.html, theme_studies_source.html, the specimen finishes, glow_examples.svg and the Downloads screenshots (images; see N7).
- The rows below are written. The Plan Paint gaps (1 to 7) and the owner's three tab capabilities are implemented, with their tests in tests/ribbon_look_tests.cpp.

Status key:
- **implemented**: in main, or in this branch (marked "branch").
- **partial**, **missing**, **contradicted**: as found in main at c9b11f0a before this branch.

Counts are at the end.

## 1. Ribbon

| # | Decision | Source | Status at c9b11f0a | Now | Evidence |
|---|---|---|---|---|---|
| R1 | Rail: flat `#93ADD0` | RENDERING_REFERENCE §4; spec §11 item 8 | missing (rail unkeyed, panel family) | implemented (branch) | gui_forms_theme.cpp `ribbon::part(tab-rail)`; ribbon.cpp on_paint, key `tab_rail` |
| R2 | Rail 27 units | spec §11 item 4 | missing (fixed 24) | implemented (branch) | `height: 27px` on tab-rail; ribbon_scaling.cpp row_metrics |
| R3 | Tab faces `#9BB6DA` / `#ACC6E7` hot / `#E8F2FF` selected | §4 | partial (TabControl and PageStrip only; ribbon tabs unkeyed, menu-item plate) | implemented (branch) | gui_forms_theme.cpp: the shared tab rule now includes `ribbon::part(tab)` |
| R4 | Wash fading by 72 %, white top line, double right keylines, hard 2-unit cast, soft (3,1) shadow | §4 | partial (as R3) | implemented (branch); on the Ribbon the keylines and casts give way to the folder taper and edge shadow (R32, R33) | same rule; tab-control and page-strip keep them |
| R5 | Tabs abut; earlier tabs sit above later ones | §4; owner 2026-11-09 (the folders overlap) | missing (gap 2, painted first to last) | implemented (branch), now overlapping (R42) | `gap: -5px`; on_paint paints the unselected tabs last to first |
| R6 | Selected: +1 px font, +3 px height, bold ink `#173F79` | §4 | missing | implemented (branch) | `margin-top: 3px` at rest and 0 when selected; `ribbon::part(tab-label):state(selected)` sets bold, 1.1em and #173f79; Ribbon::tab_label_font |
| R7 | Selected runs into the plane with no bottom edge | §4; §11 item 4 | missing | implemented (branch) | `margin-bottom: -1px`; the selected tab is painted after the plane, over its top line |
| R8 | Radius 4, top corners only | §4 | partial (radius 4 on all corners, for TabControl and PageStrip) | implemented (branch) | `border-radius: 4px 4px 0 0` on `ribbon::part(tab)` |
| R9 | File tab: gradient `#528BE0` → `#173F88`, white text | §4; §11 item 4 | partial (accent-button family) | implemented (branch) | `ribbon::part(application-button)`, `application-label` |
| R10 | Plane: `#E8F2FF`, `#CADEFB`, `#A5C4F1`, `#769FDF`; shade line beneath (the white top line removed, R31) | §5; §11 item 4 | partial (ToolStrip and MenuStrip only; plane unkeyed) | implemented (branch) | `ribbon::part(command-plane)` |
| R11 | Group labels `#485B70` beneath thin etched dividers fading at both ends | §5; §11 item 4 | missing | implemented (branch) | `ribbon::part(group-caption)`, `group-divider`; the group parts are now keyed |
| R12 | Ribbon buttons: dithered bulge, flat at rest, raised when hot | RB 15; T4 decision 1; T2-G4; owner 2026-11-09 (every ribbon hover) | implemented (commands and group buttons only) | implemented, extended to gallery items and tabs (R43) | built_in_rules.cpp add_dithered_rise |
| R13 | Themes may author a raised face at rest; flat at rest stays the stock default only | Plan Paint defect 2 | contradicted (a built-in primitive beat authored material) | implemented (branch) | theme_rules.cpp CompiledThemeRules::resolve and resolve_provenance (authored_material) |
| R14 | Marks: dialog launcher, more, overflow, customize | RB 20 | implemented | implemented | part_painting.hpp:57-60; tool_strip.cpp:733; ribbon_gallery_parts.cpp:448; quick_access_toolbar.cpp:312 |
| R15 | Dithered button on large elements, never on tabs; menus open as popups over the ribbon | RB 23; superseded for tabs by the owner, 2026-11-09 (R43) | implemented | superseded for tabs: the tabs' hover is the dithered rise too (R43) | ribbon menus are ContextMenu overlays (the menu worker's area) |
| R16 | QAT buttons flat | RB 24 | implemented | implemented | quick_access_toolbar.cpp (menu-item family) |
| R17 | The minimized ribbon's temporary popup casts the overlay shadow | RB 24 | missing (no temporary popup yet: ribbon.hpp "Not yet") | missing | see N3 |
| R18 | Contextual sets are a tint on their tabs and plane | RB 25 | implemented | implemented | ribbon_contextual.cpp:47; tab_parts and control_parts accent |
| R19 | Backstage: left column set apart by colour contrast | RB 26 | partial (window `#F4F4F5` beside card `#FFFFFF`; parts unkeyed) | implemented (branch) | backstage_view.cpp keyed parts; `backstage-view::part(column)` uses the plane's blue |
| R20 | Collapsed group button: icon, caption, chevron | RB 27 | implemented | implemented | ribbon.cpp group_parts (collapsed) |
| R21 | Overlay shadow, soft, offset down-right | RB 37 | implemented | implemented | ribbon_group_overlay.cpp:61; ribbon_application_menu.cpp:139 |
| R22 | Tooltip visual family | RB 38 | implemented | implemented | ControlVisualRole::tool_tip; floor `:family(tool-tip)` |
| R23 | Help question mark at the right end of the tab row, opening the window's help card | RB 41-43 | missing | partial (branch): the `help_button` part is reserved in the anatomy, never placed or drawn | N4 resolved: drawn and wired with the help-system design; test in ribbon_tests |
| R24 | QAT on the tab row beside File (or below) | RB 12, 44 | missing (a separate control: ribbon.hpp "Not yet") | missing | see N3 |
| R25 | Every Ribbon part keyed (tab-rail, tab, tab-label, command-plane and the rest) | Plan Paint defect 1 | missing | implemented (branch) | ribbon.cpp on_paint, paint_tab, paint_groups |
| R26 | Selected tab joins the plane: face open at the bottom, plane edge broken under it, tab painted over the edge | defect 3 | missing | implemented (branch) | paint order and margins (R7); no lip: the selected tab's #E8F2FF meets the plane's #E8F2FF top, and the plane has no top line (owner correction 2026-11-09) |
| R27 | Disabled tabs (`RibbonTabSpec::enabled`, disabled state) | defect 4 | missing | implemented (branch) | Ribbon::selectable; TabStates::disabled; anatomy tab states |
| R28 | Theme metrics (row, padding, gap, minimum widths, plane height) and authored tab widths | defect 5 | missing (constants 24, 12, 2, 56, 88) | implemented (branch) | RowMetrics; `height`, `padding-inline`, `gap`, `min-width`, `margin-top`, `margin-bottom`; RibbonTabSpec::width |
| R29 | Per-state tab label font (size, spacing, weight) | defect 6 | missing | implemented (branch) | Theme::resolve_font(control, own, part, states); Ribbon::tab_label_font |
| R30 | Activate tabs on release, at least as an option | defect 7 | missing | implemented (branch), default press | `tabs_activate_on_release` (generated property). The study does not decide; Office selects on press |
| R31 | No lip or step where the selected tab meets the plane, and no white line between the tab row and the plane | owner correction 2026-11-09 (replaces B1) | contradicted (the plane's white top line) | implemented (branch) | stock theme: the plane's top inset removed, no `flare`; Plan Paint's snippet drops `flare`. The `flare` capability stays in the language, defaulted to none |
| R32 | Smooth faded shadow from every tab onto the next, so each reads as tucked under its left neighbour (stacked folders) | owner (B2, corrected 2026-11-09) | missing | implemented (branch) | `edge-shadow: <length> <colour>` following the taper; stock `10px rgb(24 56 97 / 45%)`, falling onto the overlapped tab (R42); test_default_theme_row, test_overlapping_tabs |
| R33 | Tapered, angled folder edge on the right of every tab, selected or not | owner (B3, corrected 2026-11-09) | missing | implemented (branch) | `taper: <length> [<colour>]`; stock `5px #496c9f` on every tab, the shared tab rule's keylines and casts dropped for the Ribbon; test_default_theme_row |
| R34 | Plan Paint keeps its proportions and blue/gold identity; the theme decides command rendering | plan-paint-icon Q28 | partial (its package's resting faces were overridden) | implemented (branch) | R13; Plan Paint's package compiles as a test fixture |
| R35 | Selected tab 29 units, others 23 | RIBBON_STUDY table, line 388 | contradicted by §4's "+3 px" | as §4: 28 (one unit into the plane) and 24 | see N2 |
| R36 | A group never vanishes: shrink, collapse to a button, then scroll (Plan Paint defect 8) | RIBBON_SCALING_STUDY §1.1; coordinator 2026-11-09 | missing (authored layouts without a collapsed step never reduced) | implemented (branch) | ribbon_scaling.cpp prepare_group_layouts appends the collapsed button; test authored_layouts_collapse |
| R37 | A click on a scrolled, unfocused in-ribbon gallery picks the item under the pointer (defect 9) | coordinator 2026-11-09 | contradicted (focus scrolled before the hit test) | implemented (branch) | RibbonGallery::on_pointer_preview hit-tests first; focus does not scroll; test click_after_scroll |
| R38 | Contextual tint survives a themed tab or plane face | RB 25 | regressed inside this branch (themed faces skipped the accent) | implemented (branch) | part_painting.cpp paint_primitive_accent; test_contextual_tint |
| R39 | Dark variant: a rule's dark ink (the selected tab's label) turns light | T4 (dark derived) | missing (authored part inks stayed light-scheme ink) | implemented (branch) | theme.cpp Theme::resolve darkens authored text inks; test_dark_tab_ink |
| R40 | SearchField's text area stays inside its frame | Mac handoff 2026-11-09 (02c-captures), confirmed on Windows | contradicted (the editor's ground covered the well's top and bottom edges) | implemented (branch) | search_field.cpp placement (well_frame); display_controls_tests |
| R41 | Tab colours: only File (the application button's colour) and contextual tabs (their set's tint, T4-2) carry a colour; no other tab has its own | owner correction 2026-11-09 | implemented | implemented (branch) | the stock tab rule has one face per state; Ribbon::tab_accent gives a ramp colour to contextual tabs only, and the plane under a selected contextual tab takes the same colour. No set label or strip is drawn yet; when one is, it takes tab_set_accent, the same colour |
| R42 | The tabs overlap like stacked folders: each tab's tapered right edge lies over the next tab, a visible soft shadow falls from it onto the tab beneath, and the selected tab lies over both neighbours | owner 2026-11-09 (via coordinator) | missing | implemented (branch) | `gap` admits negative lengths (to -64); stock `gap: -5px` (the taper); File never overlaps; an overlap stops at half the narrower tab; hit testing takes the selected tab first, then left to right; test_overlapping_tabs; 3x crops look_after_tab_crop_3x_{light,dark}.png |
| R43 | The soft-dither hover (the engine's dithered rise) on ribbon commands, gallery items and tabs, in the stock theme and Plan Paint's | owner 2026-11-09 (via coordinator); RB 15; T2-G4 | partial (commands and group buttons; Plan Paint's authored faces hid it) | implemented (branch); Plan Paint's half in the section 5 snippet | built-in rules for `ribbon_gallery` `item_face` (now keyed) and the Ribbon's new `tab_hover` layer over each tab's face; engine_motion_tests machines_are_rule_data |
| R44 | Ribbon labels and captions are not clipped at scales 1 and 2 | owner 2026-11-09 (via coordinator) | contradicted (Plan Paint's Shapes gallery showed a cut third row) | implemented (branch) | RibbonGallery::visible_line_end shows whole rows in the ribbon (7215ae906); ribbon_gallery_tests. Captions and command labels fit at 1x and 2x in the Gallery captures (look_after_ribbon_scale{1,2}.png); no other clipping found |
| R45 | Part rasters baked at the window scale paint into a framebuffer of another scale (the 1x Gallery and Plan Paint showed no ribbon faces) | coordinator 2026-11-09 | contradicted | implemented (branch) | skia_raster.cpp composite_part resamples; tests/ribbon_scale_raster_tests.cpp (stock, Plan Paint's sheet, Gallery commands page at scales 1 and 2) |
| R46 | Plain-text ribbon commands (`RibbonCommandKind::label`, after ToolStrip's label and the study) and stacks spanning rows (`beside_previous`, after WRF's ControlGroup): Save over Undo and Redo, the Tool tab's three rows; two-row stacks top-aligned with large commands | Plan Paint port items 10-12 | missing | implemented (branch) | ribbon.hpp; ribbon_scaling.cpp place_group; ribbon_tests labels_and_rows; a full `row_span` is not implemented |
| R47 | A signal-on-paper palette role (with success-on-paper, pending- and invalid-ink-on-tint) | Plan Paint port item 13 | missing | implemented (branch) | gui_forms_theme.cpp palette; theme_package_compile_tests reads_signal |
| R48 | A hosted check box's width follows the CheckBox: its well, content padding and label in its own text font | Plan Paint (Fill over Edge) and the owner (View tab Show/Hide cut off) | contradicted | implemented (branch) | ribbon_scaling.cpp measure_command; ribbon_scale_raster_tests labels_fit |
| R49 | Hosted number fields paint their border and spin buttons; a press on a drawn arrow reaches the field (they painted on the backplane, under the Ribbon plane) | the owner (invisible buttons at the fields' right ends) | contradicted | implemented (branch) | numeric_up_down.cpp, domain_up_down.cpp paint plane; ribbon_scale_raster_tests hosted_commands_at |
| R50 | No ribbon label is clipped: every command label and check box label fits its part at scales 1 and 2, measured with the bundled faces | the owner | unverified | implemented (branch) | ribbon_scale_raster_tests hosted_commands_at (stock and Plan Paint) and gallery_labels_at |

## 2. Theme language and the default theme

| # | Decision | Source | Status | Evidence |
|---|---|---|---|---|
| T1 | The default is GUI.Forms' own (Pearl Ribbon toward Plan Paint), not windows-professional | T4 README; spec §11 | partial: gui-forms is built (gui_forms_theme.cpp), but default_theme() and the Gallery's base are still windows-professional (theme.cpp:2160; theme_compiler.hpp:111) | see N5 |
| T2 | Push buttons: Plan Paint's action plate | T4; spec §11 item 4 | implemented | gui_forms_theme.cpp floor `:family(button)` |
| T3 | Stock near-white `#F4F4F5`; side panels flat | T4; spec §11 item 3 | implemented | floor `:family(window)`, `:family(panel)` |
| T4 | Recess only: wells and the work pane deepest | T4; §5 | implemented (wells) | floor `:family(choice), :family(editor), :family(progress)` inset shadow |
| T5 | Command strips flat at rest; shading carries structure | T4 | implemented | light sheet `tool-strip::part(face)` |
| T6 | Status strip flat, pale, no sunken cells | §5 | implemented | light sheet `status-strip::part(face)` |
| T7 | Identity: File tab, rail and plane; no fresco band | T4; spec §11 item 5 | implemented (branch for the ribbon) | R1, R9, R10 |
| T8 | Focus: dotted dark ring, never collapses | T4 | implemented | gui_forms_theme.cpp:383-391 (dotted_focus) |
| T9 | Dark variant derived from the Phosphor pair | T4 | implemented | dark_variant.cpp:239-243 |
| T10 | High contrast: host palette wins; own mode near-colourless | T4 | implemented | src/core/theme/contrast; theme.cpp forced high contrast (the new metric fields keep geometry and drop colours) |
| T11 | Contrast gate 4.5:1 / 3:1 / 7:1 | T4 | implemented | gui_forms_theme_tests contrast report |
| T12 | Only progress bars animate ambiently | T4 | not audited | |
| T13 | Windows-familiar themes (Classic, modern flat, then XP-like, Aero-like) | T4 decision 3 | partial | familiar_themes.cpp |
| T14 | Rail colour `#9BB5D8` (§11 item 4) or `#93ADD0` (§11 item 8, §4) | spec | contradicted inside the spec | see N1 |
| T15 | CSS names lowered onto engine parameters, plus engine names (T3-A1); common shorthands (A2); em (A3) | t3 answers | implemented | theme_material_lowering.cpp named table; read_length (em) |
| T16 | Roles first (A4); engine inputs in a reserved namespace (A5) | t3 | implemented | theme compiler colour roles (`role(...)`) |
| T17 | `:is()` only (B1); a bad declaration is dropped with a diagnostic, an error refuses the theme (B2, B3) | t3 | implemented | theme_compiler.cpp diagnostics |
| T18 | Transitions and keyframes on published states only (C1); timing functions in transition (C2); reduced motion automatic plus `@media (prefers-reduced-motion)` (C3) | t3 | implemented | theme_media_query.cpp:25; motion_value.cpp:151 |
| T19 | One sheet with `@media` (D1); `@import` by name (D2); a JSON manifest (D3) | t3 | implemented | theme_compiler.cpp:606, 757 |
| T20 | Inline styles: `set_inline_style`, inline wins unless only the theme says `!important` (E1) | t3; 2-10 | implemented | Control::set_inline_style |
| T21 | Limits (E2); internal seam now, public with V (E3) | t3 | implemented | theme language limits |
| T22 | §5.4 pairs become colour-role atoms; the palette is a `:root` block (2-03) | t3 round 2 | implemented | light sheet `:root` |
| T23 | Each material field cascades on its own (2-02); full CSS cascade per part (2-05, "check with me") | t3 round 2 | implemented per field (RecipeFields cascade per field; this branch adds the part metrics) | theme_rules.cpp; see N6 |
| T24 | Named CSS colours admitted (2-21) | t3 round 2 | implemented | theme_compiler.cpp:339 named_colour |
| T25 | Easing keywords lower to their exact cubic-bezier (2-17) | t3 round 2 | implemented | motion_value.cpp:151 |
| T26 | A colour and an image combine as CSS does, colour under image (open call T3-C6) | open-calls | implemented | material lowering background layers |
| T27 | A part's own family recipe (T3-C5); a material selects the material primitive implicitly (T3-C7) | open-calls | implemented | built-in rules; the defect 2 fix (R13) |
| T28 | Two families, link and visited_link (W-Q5); link role states (W-Q4) | open-calls; w | implemented | ControlVisualRole::link, visited_link |
| T29 | Owner draw paints only the item's content; plate, selection and focus stay theme-drawn (W-Q6) | open-calls | implemented | PageStrip raise_item_content_paint (page_strip.cpp, paint_tab) |
| T30 | Dither: rescale quantization to the field's tone range (T2-G1); keep the studies' formula (G2); draw at the nearest integer scale (G3) | t2 | implemented | bulge_primitives.hpp:54; bulge_primitives.cpp:309 |
| T31 | Text shadow as an engine primitive with a first-party blur (T2-E1); content colour drawn exactly (E2) | t2 | implemented | ControlTextShadow (data); engine content colour |
| T32 | Grip primitive filling its part with the dot pattern (T2-H2); parts keyed by control, part and item ID (H3) | t2 | implemented (and extended to every Ribbon part in this branch) | R25 |
| T33 | Backdrops: opaque, transparent, window_material, transient_material; a fallback chain ending at opaque; glass flattened to its tint (T5-Q1, Q3, Q5, Q16) | t5 | implemented (T5) | compositor backdrop chain |

## 3. Docking and PageView

| # | Decision | Source | Status | Evidence |
|---|---|---|---|---|
| D1 | VS 2005 period in house colours: pale blue-grey chassis, coloured active caption, slanted document tabs | DK taste T1-2 | implemented | dock_site.cpp:28-39 |
| D2 | Quiet tiles fading in over about 100 ms; only the lit tile at full strength | T2-4 | implemented | dock_site.cpp:43-45; dock_site_drag.cpp:821 |
| D3 | Area highlight: translucent blue tint with a 1-unit edge | T3-1 | implemented | dock_site_drag.cpp:776 |
| D4 | Active caption: pale blue tint with a 2-unit accent line on top; tool tabs a small Plan Paint tab stack below the content | T4-3 | implemented | dock_site.cpp:39, 1329, 2525 |
| D5 | Flyout slides out over about 180 ms and casts the overlay shadow | T5-1 | implemented | dock_site.cpp:98, 2480 |
| D6 | Floating window: a drawn caption with one stack; host title bar with two or more | T6-3 | implemented (per DK) | DockSite floating chrome |
| D7 | Seams: graphite, 3 units, grip of dots | T7-1 | implemented | dock_site_internal.hpp:21-22 |
| D8 | Drag proximity: only the pane under the pointer regains emphasis | T8-3 | implemented | dock_site.cpp identity key (2529) |
| D9 | Split indicators only over the hovered pane, blue on black | D2 settled | implemented | dock_site_drag.cpp tiles |
| D10 | Postage stamp: miniature animated over about 220 ms; the vacated place closes up | D1 corrected | implemented | dock_site.cpp:43 (drag_gesture.hpp curve) |
| D11 | Strip tabs take Plan Paint's tab stack; document tabs notched and slanted | PT1 | implemented | gui_forms_theme.cpp `page-strip::part(tab)`; page_strip.cpp:1036 |
| D12 | The selected tab joins its page by an open seam | PT2-1 | implemented | page_strip.cpp:851-859 (tab_seam) |
| D13 | Close glyph gets a hover plate; on pinned tabs the pin replaces close | PT3-2 | implemented | page_strip.cpp:765 |
| D14 | `modified` is a large dot in the close slot, replaced by close while hot | PT4-1 | implemented | page_strip.cpp:34, 77, 401 |
| D15 | Pinned tabs compressed to their icon, expanding on hover | PT5-2 | implemented | page_strip.cpp:792 |
| D16 | Overflow button: count of hidden pages and a down arrow; hidden pages italic | PT6-3 | implemented | page_strip.cpp:38, 1390, 1993 |
| D17 | Navigation pane buttons: rows of Plan Paint's tab material; selected near-white; rail-blue grip | PT7-3 | implemented | navigation_pane.cpp:24 |
| D18 | Rotated side-tab labels read bottom to top on the left | PT8-1 | implemented | page_strip.cpp:344; navigation_pane.cpp:1028 |
| D19 | Pin mark; tab-into mark; `modified` custom state; text orientation; pane role; close action | DK J1-J6 | implemented | part_painting.hpp marks; page_strip.cpp:2307 |

## 4. Needs owner (all resolved, owner 2026-11-09)

- **N1. Rail colour: resolved (a).** `#93ADD0` in the stock theme; Plan Paint keeps `#9BB5D8` in its own package. Applied.
- **N2. Tab heights: resolved (a).** +3: the others 24, the selected 28 (27 plus one unit into the plane). Applied.
- **N3. Minimized-ribbon popup and the QAT on the tab row: resolved (a).** A follow-up RB item, outside this batch (R24 stays missing).
- **N4. Help question mark: resolved.** The `help_button` part is reserved in the Ribbon's anatomy now (R23); it is drawn and wired with the help-system design.
- **N5. Default theme: overtaken.** main switched the default to gui-forms (cp24, #474); this branch is rebased onto it. The Gallery's own sheet now has a dark variant (separate commit), so the dark Gallery is dark outside the ribbon too.
- **N6. Per-part cascade: resolved.** Per-field cascade is the intent, as implemented.
- **N7. Downloads screenshots: resolved.** References, not rulings; no rows.

**Colour rule (owner correction, 2026-11-09).** Only File (the application button's colour) and contextual tabs (their set's tint, T4-2) carry a colour; no other tab has its own. A contextual tab's tint is its set's ramp colour, which the plane under it and any set label or strip share (R41).

## 5. Plan Paint's package: the CSS snippet

For themes/plan-paint/plan-paint.theme.css. The original tab row's widths go on `RibbonTabSpec::width` in Plan Paint's code (src/forms/ribbon_tabs.cpp already sets them): Home 61, View 63, Patterns 100, Tool 142, Atlas 72, Icon 72, Gradients 105. File takes its label's width; the snippet sets no `min-width` for it (61 was the original Home width, not File's: corrected 2026-11-09).

```css
/* The look integration's tab-row metrics and tab capabilities (GUI.Forms
   claude/look-integration): Plan Paint's 27-unit rail and 116-unit plane.
   The tabs are stacked folders (owner, 2026-11-09): each overlaps the next
   by its four-unit taper (a negative gap), its tapered right edge lying
   over the next tab, and a visible soft shadow falls from that edge onto
   the tab beneath. The selected tab lies over both neighbours, stands
   three units taller and runs two units into the plane with no lip,
   flowing straight into it. */
ribbon::part(tab-rail) {
  height: 27px;
}

ribbon::part(command-plane) {
  height: 116px;
}

ribbon::part(tab) {
  gap: -4px;
  margin-top: 3px;
  taper: 4px #496c9f;
  edge-shadow: 9px rgba(24, 56, 97, 0.45);
}

ribbon::part(tab):state(selected) {
  margin-top: 0;
  margin-bottom: -2px;
}

ribbon::part(tab):disabled {
  background-color: #b9c4d2;
}

ribbon::part(tab-label):state(selected) {
  color: #173f79;
  font-weight: bold;
  font-size: 1.08em;
}

/* The soft-dither hover (owner, 2026-11-09): command faces are flat at
   rest and rise in the engine's dithered bulge while hot, as gallery items
   and tabs now do on their own. Delete the package's resting
   `ribbon::part(command-face)` rule and its `:hover` rule (an authored
   resting face beats the built-in rise, R13); keep the `:active`,
   `:checked` and `:disabled` rules, and put this rule, which gives them
   the resting rule's frame, where the resting rule stood (before them, so
   the disabled border colour still wins). */
ribbon::part(command-face):active,
ribbon::part(command-face):checked,
ribbon::part(command-face):disabled {
  border-radius: 3px;
  border: 1px solid #6384b2;
}
```

## Counts

| Section | Rows | Implemented before | Implemented by this branch | Partial left | Missing left | Contradicted left | Not audited |
|---|---|---|---|---|---|---|---|
| Ribbon and look defects | 40 | 10 | 27 | 0 | 3 (R17, R23, R24) | 0 (R35 decided as §4, N2) | 0 |
| Theme language and default | 33 | 27 | 1 (T7) | 2 (T1, T13) | 0 | 1 (T14, N1) | 1 (T12) |
| Docking and PageView | 19 | 19 | 0 | 0 | 0 | 0 | 0 |
| **Total** | **92** | **56** | **28** | **2** | **3** | **1** | **1** |

Found at c9b11f0a: 27 missing, 8 partial, 5 contradicted, 49 implemented, 1 not audited; R38 regressed inside the branch and is fixed in it. The Gallery's dark capture is poor outside the ribbon (headings and sidebar plates), because the Gallery's own sheet is written for light only; that is outside this batch (see N5). The R35 and T14 conflicts are inside the sources. The docking rows were checked against the code's citations of the DK decisions, not by capture.
