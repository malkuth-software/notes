# Families B: property capabilities, second family worker (w6)

Workspace (since 2026-11-05) `~/Developer/gui_forms_workers/w6/gui_forms` on the Mac laptop, builds on the M4 via `bin/m4run w6`; formerly `D:/gui_forms_w6`. Scratch and
logs under `D:/gui_forms_w6/dev/`, scripts under `D:/gui_forms_w6/dev/tools/` (`setter_survey.py` lists
every `Class::set_x` with its invalidation kinds and events; output `dev/setters_short.txt`).
Started 2026-11-05 by the coordinator. Sources read: PROPERTY_CAPABILITIES_STUDY.md 1-13 (desks/coordinator),
w_controls.md (RESUME 2026-11-05), CLOSURE_0.5.md P-0..P-8 and W-4, CONTROL_SUPERSET_STUDY.md 4.1/4.2/5,
M1 read-only from `w2/m0-settle` (fetched from D:/gui_forms_w2/gui_forms; heads fadf3b9c M1, 0fdab0d7 docs).

## State

- **BUILDING (2026-11-05, coordinator: M3 landed in W's stack).** Base: W's cbe8202c (M2d 84ec483f, codemod
  68b00859, M3 cbe8202c; fetched into D:/gui_forms_w6; branch `claude/families-b-base` marks it).
- **Workspace layout (changed):** the outer clone D:/gui_forms_w6 stays on `main`; the work happens in the
  worktree **D:/gui_forms_w6/gui_forms** (so ccache's relative paths match the other workspaces), with
  `env.sh`, `toolchain_env.sh` (copies of w2's, priority normal = shared slots), `template_harness/`
  (clone of w2's), `.build/native-windows-x64/gui-forms` (W's build.sh flags, Skia on) beside it, all
  excluded in `.git/info/exclude`. Text stack (freetype, harfbuzz, libunibreak) copied from w2. Scripts in
  `D:/gui_forms_w6/dev/tools/`: `configure.sh`, `build_tests.sh '<regex>'`, `kbuild.sh [target]` (-k 0,
  error summary), `verify.sh '<regex>'` (w2's, retargeted). All builds go through build_slot.py.
- **Configs:** the projects copies under families_b/property_families are the plan; once a family starts,
  its config lives in gui_forms `tools/property_families/<family>.json` and is refined there (that copy
  wins). Rules learned at the first family: a collection edited in place (set_column_width, set_cell_text,
  add_/remove_ helpers on big models) is not a cell unless every edit can go through a whole-value write
  (decided per family; TreeListView columns/nodes stay methods); scroll state clamped by layout
  (top_row, horizontal_offset) stays a method; a shared Text/Font `initial` is brace contents.
- **RESUME POINTER (2026-11-08, code-only coder, w6).** Branch **`claude/families_b-queue`** in
  `~/Developer/gui_forms_workers/w6/gui_forms` on cp20 815f7ae9 + W's codemod fix (1ae2f96f, selftest ok):
  bf79b2a0 B7 ListView, 3373da09 B9 TaskDialog, dfcb8a26 B2 GroupBox/ScaledPanel/ScaledGroupBox, b66ba5b5
  GroupBox TextChanged, a61537c5 B3 TextBox (ComboBox/DomainUpDown text() now return std::string),
  f8ddddb2 MaskedTextBox (text/set_text stay methods: MaskEditResult), 14363f52 RichTextBox (markdown/RTF
  stay methods), d905ffee B4 NumericUpDown (min/max split, WF crossing rule)/DomainUpDown; all registered
  in INTEGRATION_QUEUE.md, none built. Done ed41630b B5 PictureBox. B5 rebased on W collections: branch claude/families_b-b5 6165badf merge, 42cf677d ListBox, ca7be3d1 CheckedListBox, 3b835662 ComboBox (B5 complete). B6 done on the same branch: 7f7bb850 PropertyGrid, 987bcbce PropertyList, bb0351e5 Card/ReviewCard/MaterialPanel, f455bf44 BreadcrumbTrail, efa6711a AnchoredOverlayLayer, 9cd5d53f CorrespondenceView, 32283610 InstrumentRack (CutOffText: nothing); queued. B7 done: c6e284b1 TreeView, beb8c366 ObjectView (TreeListView W's f7513b19); queued. B8 done: 222b9d4f DateTimePicker, 90da7230 ColorValueEditor/FlagsValueEditor, 1e707785 FontDialog (MonthCalendar earlier). B9 done: 8c2dd64e components (Color/Open/Save/FolderBrowser dialogs, NotifyIcon, ToolTip; cells only, no table entries per D4; TaskDialog was 3373da09); B8+B9 queued at 8c2dd64e. B10 ChoiceGroup: nothing to migrate until M-S (selected_index stays Value<int>, G14; items a method); its presentation property is the W-4 segmented control. W-4 (D8 answered by T4: notice family with severity custom states, badge family, marks search/information/warning/success) done on the same branch and queued at 21ec9761: b52e6d75 vocabulary, 1519c2c5 Badge, f0d86a17 InfoBanner, c7a2dd4e EmptyState, 21ec9761 Gallery page notices-and-badges (display trace to re-record). Segmented ChoiceGroup presentation done at 7e70c10e (queued). Toast done at 93d11b44 (queued): coordinator option 1, a core ToastPresenter per window on ToolTipPresenter's pattern (moves with tooltips to Splash windows in Part S), Window::show_toast API, stock Toast control, InfoBannerSeverity renamed NoticeSeverity. ToggleSwitch and SearchField done at db095229 (queued), stacked on w7's roles 85c7e94e (merge 5eb0a3c4, which also brings main d722217c). Coordinator rulings done (queued at a16c1e88): d75e0e07 SearchField one search_box node; a16c1e88 switch thumb travel (MotionValue) and busy sweep (control ambient), reduced motion gives no travel. Toast slide and segment indicator motion not asked for. Scratch
  sweeps: fam_sweep.py TYPE names SKIPDIR [--write] (typed locals), rxg.py (members/accessors from headers). Deviations from the plan
  below: ListView focused_index stays a method (six internal writes that must not scroll); TaskDialog
  expanded and verification_checked are PropertyViews onto the page, progress_value stays a method.
  Next in order: B3 TextBox (17 properties; text a PropertyView over the engine; TokenBox's views then
  write TextBox cells), MaskedTextBox, RichTextBox, then B4... Laptop-only workflow: config, generate,
  hand hooks, hand caller sweep by typed variables (scratch scripts rw.py / gb_sweep.py pattern), tests,
  `property_codemod.py tests` with all configs, generate_library_docs (fix docstring Parameters when a
  constructor parameter is renamed initial_*; drop stale manual.json method notes), control_schema
  check/render, abi_event_catalog when events change. inventory.json needs clang (integrator).
- **RESUME POINTER (2026-11-08, handover to a code-only coder; the integrator builds batches).**
  Workspace `~/Developer/gui_forms_workers/w6/gui_forms`, branch **`claude/fb-families`** on cp19's
  `origin/checkpoint/2026-11-05-4` 835c3599; working tree clean. Rebase onto main once cp19 merges
  (drop what it contains). `claude/fb-tree-list-view` and `claude/families-b-base` are obsolete.
  **Done and verified** (all targets 0 errors; touched tests plain + sanitized; macOS a11y gallery
  plain; style, lifetime, docs --check, schema check/render --check, state-machine gate clean):
  B7 TreeListView 4fb5cb36 (in cp19), **93324014 B8 MonthCalendar**, **85b96ac0 B3 TokenBox**.
  **Remaining families** (order of the "Families" sections below; configs for most exist as plans in
  projects `tracks/gui_forms/families_b/property_families/*.json`, regenerated by
  `families_b/generate_configs.py`; refine each before copying into gui_forms
  `tools/property_families/`, where the copy wins):
  ListView and TaskDialog next (cp20 scope); then B1 Panel, B2 ScaledPanel/GroupBox/ScaledGroupBox,
  B3 TextBox, MaskedTextBox, RichTextBox, B4 NumericUpDown/DomainUpDown, B5 PictureBox, ListBox,
  CheckedListBox, ComboBox, B6 editors and composites, B7 TreeView and ObjectView, B8 DateTimePicker,
  ColorValueEditor, FlagsValueEditor, FontDialog, B9 components and dialogs (after W's generalised
  table), B10 ChoiceGroup. W-4 controls are cp21.
  **ListView plan** (read from the setters in src/controls/panel/list_view/list_view.cpp): cells
  check_boxes, full_row_select, draw_mode, default_group_header with generated hooks (prepare = the old
  refusals); hand hooks for font (written forwards to label_editor_ then invalidates), tile_size
  (written invalidates only when view is tile), view and show_groups (written: hot_index_ reset,
  ++model_revision_, conceal_cut_off_text, hot_column_ reset for view), label_edit (written cancels a
  label click and edit when disabled; no invalidation), multi_select (written trims the selection to
  the focused or first selected item and publishes; when allowing or <= 1 selected, only semantics),
  focused_index (prepare: out_of_range past item_count; written: ensure_visible then paint|semantics),
  small_image_list / large_image_list (prepare: a live list of the own Window; written: resubscribe the
  matching token; the private set_image_list(images, slot, token) helper goes). Methods (not
  properties): columns, items, groups (edited in place), virtual_source (replaces the model),
  horizontal_offset and top_line (clamped by layout), set_column_width, set_item_checked, set_selected,
  set_sort_indicator. The planned config's draft disagrees in places (it makes columns/items/groups and
  virtual_source cells): follow this list.
  **TaskDialog plan:** expanded (event expanded_changed; hand: written does the two invalidations of the
  setter), verification_checked (storage is page_.verification_checked: a PropertyView or keep a
  method), progress_value (no storage found: read the source first); set_page stays a method.
  **Codemod workflow** (`tools/property_codemod.py`, W's): `check CONFIG`; `generate CONFIG` (writes the
  capabilities header, the class block, the source block; deletes getters/setters/fields); write the
  hand hooks it lists ("move the body of X into its hooks, then delete it"); `rewrite --log LOG
  --build-dir DIR --build-command CMD CONFIG` (compiler-driven; needs builds: the integrator's job now);
  `audit CONFIG`; `tests CONFIG...` (pass every generated family's config explicitly; a config with a
  "test" block for a family not generated yet breaks the generated test); then
  `python3 -B tools/generate_library_docs.py` and fix docstring examples that call removed setters.
  **Pitfalls met:**
  - generate's field deletion misses an initializer containing ';' (TokenBox `delimiters_{",;"}`): the
    field stays and reads silently use it. After generate, check every config `field` is gone.
  - a value type declared in the control's own header before the class cannot be named by the
    capabilities header (included at the top): move it to its own header and list it in the config's
    `includes` (MonthCalendar: calendar_values.hpp).
  - a cell needs operator== on its value type (DateTimeFormatProvider got one).
  - locals or parameters named like a property (`font`) shadow the cell: rename them (day_font,
    body_font, token_font); `audit` catches parameters, not every local.
  - the rewrite rounds do not always recompile Gallery pages and their tests (order-only dependencies
    behind the broken library): grep src/controls/gallery/pages, tests/gallery_pages, docs docstrings,
    src/host/*, tests/linux_*, tests/windows_* and *.mm for the old calls by hand.
  - selected_id-like views return copies (optional<string>), not string_views borrowed until a change.
  - equal writes now do nothing (format_provider used to invalidate on every write): note it in the
    commit.
  - shared setter names (set_font) across families: rewrite one family at a time.
  - collections edited in place stay methods (coordinator 2026-11-05) until W proposes a shape.
  **Verification** (integrator): all targets with ninja -k 0; touched tests plus
  gallery_display_trace, gallery_pages, doc_examples, binding (and date_time_picker, text_box when
  their shared types move) plain and sanitized (sanitize with --skip-labels
  'area_host|area_a11y|gallery_accessibility|foreground'); macOS a11y gallery test plain with
  M4WS_WAIT_IDLE=180; style, lifetime, docs --check, schema, gate. Windows and Linux: CI only.
  Mac tools in `w6/dev/tools/` (need builds): `kbuild.sh [LOG] [targets]`, `verify.sh '<regex>'
  [--gallery]` (VERIFY_NO_SANITIZE=1 skips sanitize).

## Owner answers (2026-11-05, via the coordinator)

- **D3: components MIGRATE before the seal.** ToolTip, NotifyIcon and the dialogs get capabilities; the
  tables stop being Control&-only (G7). B9 is planned against a generalised table (W designs it).
- **D5: no spinner and no progress ring now.** Dropped from W-4.
- **D6: the segmented control is a ChoiceGroup presentation; ChoiceGroup (collections/) moves to me.**
- **D7: Badge is a standalone control now.**
- **D8:** the coordinator clears `switch` and `search_box` with the semantic-tests worker, and any new part
  roles with T4, once I start W-4.
- **W's answers (study section 14), all as recommended:** assign shape `prepare_write` (first statement
  `require_mutable()`, then validation, then `coerce_x`) / `replace` / `written` (touch, side effects,
  `publish_change`, then `on_x_changed`), private hook overloads chosen by cell type, `friend struct
  XCapability<D>;` (G4-G6). D1: shared `TextCapability` / `FontCapability` templates in
  include/gui_forms/binding/capabilities/, no text/font on Control. D2: `PropertyView` (M2). G3:
  `override_property_entries` (M2). D4: cells only, byte-equal. G7: thunks take `Component&`,
  `Component::property_table()`, table-only free functions; B9 widens binding, loader and C ABI to
  `Component&`. G8: child-property table with M-G. G9: ScrollableControl right after M3. G10: slots stay
  methods. G14: `Value<T>` stays until M-S.
- **Codemod configs WRITTEN (study 15.2, projects ce33679):** `tracks/gui_forms/families_b/property_families/`
  (40 configs, one per declaring class, 240 properties, 128 marked `"hooks": "hand"` with a `"hand"` list of
  reasons; `index.json` gives the B-step order and per-config counts). Regenerate with
  `python -I tracks/gui_forms/families_b/generate_configs.py <gui_forms checkout>`: it reads the SPEC table
  (the per-property decisions of this file) plus the checkout's headers and sources (field, initial,
  getter/setter/clear, localizable, comments, single-mask effects) and reports every lookup it could not
  make. Generated from main ebb9fbf4; rerun after M2/M3 land, since the checkout moves.
- **Extensions to 15.2 asked of W** (optional keys; accept or reject): `storage: "view"` (PropertyView),
  `effects` for cell-only properties (table null), `thunks` (today's custom read/write/reset/
  should_serialize functions on registered properties), `hand` (reasons), `comment_generated`, top-level
  `step`, `shared` (TextCapability/FontCapability taken) and `not_properties` (methods that stay).
  **All accepted by W (study 15.2, eb6f5c7).** Rules: `initial: null` = value initialization (`""` is
  refused); `comment_generated` blocks a family's commit until its comments are reviewed (most cell
  comments are generated: review them per family before committing); with a `table`, `effects` is absent.
- **W's answers (15.3):** capability struct names are unique in `namespace gui_forms`: a property name
  used by another family (mine, a schema type outside my share, or a setter in another header) takes the
  class prefix (`PanelStyleCapability`, `DateTimePickerStyleCapability`); the shared Text/Font keep plain
  names; the generator applies this (115 of 238 prefixed). Panel `style`: the cell holds the override, the
  effective style becomes a separate method. NotifyIcon `text`: shared `TextCapability<NotifyIcon>`, no
  invalidation. TextBox/RichTextBox `auto_scroll`: removed from my configs (ScrollableControl's cell;
  `override_property_entries` for descriptor changes, `on_auto_scroll_changed`/`coerce_auto_scroll` on
  ScrollableControl for behaviour; never a hiding method). **GroupBox TextChanged: owner approved** as a
  recorded schema change in its own commit (group_box.text `change_notifications`), listed in the
  WinForms compatibility notes.
- Reads per 15.1: `read()`; `type()` is the runtime PropertyType; `as_type(types::x)` / `as_type(t)`.
- FontDialog (15.3): `FontCapability<FontDialog>`, `written` invalidates nothing, `prepare_write` applies
  the size range and font list; nothing generic treats FontCapability as "has a face".

## Decisions in force (from the study and the owner)

- Capability mixins with dot syntax; CRTP parameter = the declaring class; reads `.read()`; a write is
  compare, store, touch, then the change event at the write (decisions 1-3); equal writes do nothing.
- UI thread only; other-thread writes throw `std::logic_error` and are counted (decision 4).
- Per-type constexpr table, base entries first (`join_property_tables`), `property_names_distinct` in a
  static_assert next to each table; the schema stays byte-equal for every moved property (study 11).
- P-8 = 1b: `coerce_<property>(Value&)` (protected virtual, before the store, compared again) only on the
  properties C# lets a subclass rewrite through a virtual setter; `on_<property>_changed()` (protected
  virtual, base raises the event) only where WinForms has `On<Property>Changed`; neither otherwise.
- Per family (rules as of 2026-11-05, coordinator): **no forwarders**: each family deletes its `set_x` /
  `x()` and rewrites every caller to dot syntax in the same step (controls, demos, generated form code and
  generator, tests, docs examples, C ABI shims through the table thunks). Reads are `read()`; conversions
  `as_type(types::x)` or `as_type(descriptor)` (study section 15: the read spelling and the codemod config format). No hook or function parameter shares the property's name
  (`-Werror=self-assign`). Schema byte-equal; the family's tests and Gallery pages; sizeof re-measured
  with `tools/property_capability_lab.cpp`.
- **The migration is generated:** W's codemod (tools/) takes a per-family config (each property's kind,
  dirtiness, event, On<X>Changed, coerce) and writes the cell, the table, the schema row, deletes the
  accessors and rewrites the callers (getter switch: projects tools/property_getter_switch.py, compiler
  driven, explicit name list). The per-property tables below become those configs as soon as W documents
  the format in the study; hand work is limited to the cases under "Hand work".
- One Haiku tedium helper (gui-forms-tedium, requested from the coordinator) may run the mechanical
  codemod/build/test loops once building starts.
- Ownership by directory: mine are `src/controls/panel/*` (+ their headers under
  `include/gui_forms/controls/panel/`), the pickers, then components and dialogs, then the rest; plus W-4
  in new directories. control.hpp/.cpp are W's; control-schema.json is regenerated by whoever lands.

## Legend

Dirty: M measure, A arrange, L layout, P paint, H hit_test, S semantics, X accessibility, Y style;
"all" = M A P H S X (the survey truncates the longest masks; read the setter when coding).
Schema: **R** registered today, so it gets a table entry and the schema stays byte-equal;
**C** cell only, no table entry (schema unchanged; widening is decision D4); **N** not a property
(a structural slot, per-item operation or multi-value call: stays a method).
WF = WinForms has `On<Property>Changed` (so `on_<property>_changed()` virtual); co = `coerce_<property>`.
Event = today's change event (none = no event today; a new event only where WinForms has one).

## Families, in order of work

Order rule (study 13): fewest registrations first, but Panel first because every table in my share joins
Panel's (all but ListView, TreeListView, MonthCalendar, TokenBox, TaskDialog derive from Panel).

### B1 Panel (`panel/panel.cpp`, `panel.hpp`; base ScrollableControl, W's)

| property | kind | dirty | event | WF | co | schema |
|---|---|---|---|---|---|---|
| border_style | enumeration (border_style) | P S | none | no | no | R |
| background (override colour) | color | P | none | no | no | C (see G11: may fold into back_color in M2) |
| style | enumeration (BasicControlStyle) | P S | none | no | no | C |
| visual_role | enumeration (ControlVisualRole) | Y P S | none | no | no | C |

Schema impact: none (one R). Tests: cell write/read/equal write; table found by name and byte-equal; Panel
face colour still follows the theme after the M2 defect fix (W's test, re-run). Gallery: every page uses
Panel; the display-trace digests are the check.

### B2 ScaledPanel, GroupBox, ScaledGroupBox (`scaled_panel/`, `group_box/`)

| control | property | kind | dirty | event | WF | co | schema |
|---|---|---|---|---|---|---|---|
| ScaledPanel | design_size | size | L | none | no | no | R |
| ScaledPanel | design_bounds(child, rect) | - | L | - | - | - | N (child property later, study 10.7) |
| GroupBox | text | text | M P S | none today; TextChanged | yes (OnTextChanged) | yes (Text) | R (G1) |
| GroupBox | font | font | M P S | none today; FontChanged | yes (OnFontChanged) | yes (Font) | R (G1) |
| GroupBox | use_mnemonic | boolean | M P S | none | no | no | C |
| ScaledGroupBox | design_size | size | L | none | no | no | R |

Schema: none. Tests: as B1, plus a subclass test overriding `on_text_changed` and `coerce_text` (P-8
evidence for my share), and the scaled-layout tests unchanged (layout_panel_tests).

### B3 Text boxes: TextBox, MaskedTextBox, RichTextBox, TokenBox (`text_box/`, `masked_text_box/`,
`rich_text_box/`, `token_box/`, shared `text_editing/`)

TextBox (registered: text):

| property | kind | dirty | event | WF | co | schema |
|---|---|---|---|---|---|---|
| text | text | i:text_content (M P S) | text_changed | yes | yes | R; engine-owned (G2) |
| placeholder_text | text | P S | none | no | no | C |
| read_only | boolean | P S | none | yes (OnReadOnlyChanged) | no | C |
| multiline | boolean | M P S | none | yes (OnMultilineChanged) | yes (Multiline virtual) | C |
| word_wrap | boolean | M P S | none | no | no | C |
| newline_sequence | text | - | none | no | no | C |
| accepts_tab | boolean | - | none | yes (OnAcceptsTabChanged) | no | C |
| accepts_enter / accepts_escape | boolean | - | none | no | no | C |
| maximum_length | unsigned_integer | S | none | no | yes (MaxLength virtual) | C |
| password_character | text (char32) | M P S | none | no | no | C |
| use_system_password_character | boolean | M P S | none | no | no | C |
| font | font | M P S | none | yes | yes | C (G1) |
| auto_complete_mode / _source | enumeration | S | none | no | no | C |
| auto_complete_custom_source | object (shared_ptr) | - | none | no | no | C |
| auto_complete_case_sensitive | boolean | - | none | no | no | C |
| auto_scroll | boolean | - | none | no | no | C |
| selection(sel, reveal), edit_filter, auto_complete_anchor, multiline_scroll | - | - | - | - | - | N |

MaskedTextBox: mask (text, -, mask_changed, WF yes OnMaskChanged), culture, text (engine-owned, G2),
text_mask_format, prompt_character, allow_prompt_as_input, ascii_only, skip_literals, reset_on_prompt,
reset_on_space, reject_input_on_first_failure (all C, no event, WF no), overwrite_mode (boolean,
overwrite_mode_changed, WF yes OnIsOverwriteModeChanged), read_only (P S, WF yes). All C.
RichTextBox: text (R, engine-owned G2, text_changed, WF yes, co yes), markdown / rtf (N: document loads,
not values), read_only (P S, WF yes), word_wrap (M P S), accepts_tab, control_bar_visible (M P S),
font (M P S, G1), auto_scroll, heading_level (N, per selection); selection, scroll N.
TokenBox (derives Control): selected (optional TokenId, P S, selected_token_changed, WF no), delimiters
(text), font (all, G1), auto_complete_* (as TextBox). All C.

Schema: none (two R). Tests: text_box_enter/escape, multiline_text_box, masked_text_box, rich_text_box,
token_box tests after the getter switch; a capability test per type (write, equal write, wrong thread,
`on_read_only_changed` override, `coerce_maximum_length`); the engine-owned text: write then type
in the editor then read back (G2 decides the form).

### B4 Up-downs: NumericUpDown, DomainUpDown (`numeric_up_down/`, `domain_up_down/`, `up_down/`)

| control | property | kind | dirty | event | WF | co | schema |
|---|---|---|---|---|---|---|---|
| NumericUpDown | value | number | P S | value_changed | yes (OnValueChanged) | no | R |
| NumericUpDown | minimum, maximum | number | P S | none (value_changed when value clamps) | no | no | C (split from set_range; set_range stays as a method) |
| NumericUpDown | increment | number | - | none | no | no | C |
| NumericUpDown | decimal_places | unsigned_integer | P S | none | no | no | C |
| NumericUpDown | hexadecimal | boolean | P S | none | no | no | C |
| NumericUpDown | button_width | number | all | none | no | no | C |
| DomainUpDown | items | collection | S | none (WinForms Items has none) | no | no | R |
| DomainUpDown | selected_index | signed_integer (optional) | via helper | selected_item_changed | yes (OnSelectedItemChanged) | no | C, derived (G2) |
| DomainUpDown | text | text | via helper | text_changed | yes | yes | C, derived (G2) |
| DomainUpDown | wrap, sorted, read_only | boolean | -, S, P S | none | no, no, no | no | C |
| DomainUpDown | button_width | number | all | none | no | no | C |

WinForms coupling for minimum/maximum: setting minimum above maximum raises maximum to it, and value is
clamped (value_changed raised after the range change), in that order. Tests: domain_up_down_tests,
numeric up-down tests, range coupling order, items equal-write (element-wise compare does nothing).

### B5 Lists: PictureBox, ListBox, CheckedListBox, ComboBox (`picture_box/`, `list_box/`, `combo_box/`)

| control | property | kind | dirty | event | WF | co | schema |
|---|---|---|---|---|---|---|---|
| PictureBox | image | image | M P S | image_changed | no | no | R (has reset + should_serialize) |
| PictureBox | size_mode | enumeration | M P S | none | yes (OnSizeModeChanged) | no | R |
| PictureBox | image_opacity | number | P S | none | no | no | R |
| ListBox | items | collection | M P S | none (selection_changed when selection drops) | no | no | R |
| ListBox | selection_mode | enumeration | P S | none | no | no | R |
| ListBox | item_stable_ids | collection | S | none | no | no | C |
| ListBox | top_index | unsigned_integer | P S | none | no | no | C |
| ListBox | item_height | number | M P H S | none | no | yes (ItemHeight virtual) | C |
| ListBox | font | font | M P S | none | yes | yes | C (G1) |
| CheckedListBox | check_on_click | boolean | S | none | no | no | C |
| CheckedListBox | indicator_size | number | M P S | none | no | no | C |
| CheckedListBox | item_check_state(index, state) | - | P S | item_check_state_changed | - | - | N |
| ComboBox | items | collection | M P S | items_changed (+ selected_index/text when they move) | no | no | R |
| ComboBox | selected_index | signed_integer | P S | selected_index_changed | yes (OnSelectedIndexChanged) | yes (SelectedIndex override) | R, derived (G2) |
| ComboBox | text | text | P S | text_changed | yes | yes | R, derived (G2) |
| ComboBox | placeholder_text | text | P S | none | no | no | R |
| ComboBox | drop_down_style | enumeration | all | none | yes (OnDropDownStyleChanged) | no | R |
| ComboBox | drop_down_filter | enumeration | - | none | no | no | R |
| ComboBox | auto_complete_mode / _source / _custom_source | enum / enum / object | - | none | no | no | C |
| ComboBox | maximum_drop_down_items | unsigned_integer | S | none | no | no | C |
| ComboBox | drop_down_width | number | P S | none | no | no | C |
| ComboBox | font | font | M P S | none | yes | yes | C (G1) |
| ComboBox | dropped_down | boolean | - | none (DropDown/DropDownClosed are action events) | no | no | C |

Schema: none (11 R). Tests: checked_list_box, combo_box_filter, collection tests; ComboBox
selected_index then text event order unchanged; items equal-write does nothing; image reset and
should_serialize through the table thunks.

### B6 Editors and composites (`property_grid/`, `property_list/`, `card/`, `material_panel/`,
`breadcrumb_trail/`, `cut_off_text/`, `anchored_popup_layer/`, `correspondence_view/`, `instrument_rack/`)

- PropertyGrid: property_sort (enumeration, -, none, WF yes OnPropertySortChanged, R);
  selected_object / selected_objects (N: retained observed controls, selected_object_changed; WF yes
  OnSelectedObjectsChanged: keep the method, add the virtual); converter_registry, editor_registry
  (object, C); property_expanded(name, bool) N.
- PropertyList: font, row_height, label_width, header_height (all-layout, C); scroll_offset (P H S, C);
  groups (collection, all, C); per-row value/description/validation/reset_enabled/expanded and
  header_content N. Events group_changed, row_expansion_changed stay per-item.
- Card: card_layout (enum, M A P H S, C), interactive (boolean, Y P H S, C), selected (boolean, Y P S,
  selected_changed, C), selection_behavior (enum, S, C); header/body/footer N (G10). ReviewCard: record
  (object, M A P S, record_changed, C).
- MaterialPanel: material (object SurfaceMaterial, i:paint_only, material_changed, C).
- BreadcrumbTrail: font (G1), appearance (enum, M P H S), segments (collection, M P H S + more),
  active_id (text, P S X), editing (boolean, P H S X), tab_completion_available (boolean, S X). All C,
  no events today, WF no (no counterpart).
- CutOffText: no setters (free layout helpers); nothing to move.
- AnchoredPopupLayer: placement (enum, all, C), dismiss_on_click_away, dismiss_on_escape (boolean, -,
  C); anchor, content N (G10).
- CorrespondenceView: items (collection, M P H S, selection/pin events, C), selected_id (P S,
  selection_changed), pinned_id (M P H S, pin_changed), compact_height, expanded_height (M P H S),
  status_rail_width (P), hover_intent_delay (unsigned ms, -), font (G1), scroll_offset (P H S); hovered_index N
  (interaction state, not authored). All C, WF no.
- InstrumentRack: modules (collection, all), module_width, module_height, rack_gap (i:bounds), scroll_offset
  (P H S) C; per-module enabled/state/field value/validation and action_content N.

Schema: none (one R). Tests: the controls' existing tests after the switch; one capability test file for
the group.

### B7 Item views (`tree_view/`, `list_view/`, `tree_list_view/`, `object_view/`), before M-R

All C (none registered), WF no unless stated, events per-item stay methods:
- TreeView: items (collection, M P H S), selected_id (text, P S, selection_changed; WF: OnAfterSelect is
  an action event, so no virtual), item_height (M P H S), indentation, show_expanders (P H S), top_row
  (P S), font (G1), image_list (object, P S); expanded(id, bool) N.
- ListView (derives Control): view (enum, A P H S), columns (collection, A P H S), items, check_boxes,
  multi_select (S), full_row_select (P H S), focused_index (P S), top_line, horizontal_offset (P H S),
  draw_mode (enum, P), font (G1), tile_size (size), groups, show_groups, default_group_header (P S),
  label_edit (-), small_image_list / large_image_list / image_list (object), virtual_source (object, -);
  column_width(i), item_checked(i), selected(i), sort_indicator(col, order) N.
- TreeListView: columns, nodes (A P H S), top_row, horizontal_offset, indentation (P H S), selected_id
  (P S), image_list, font (G1); column_width, cell_text, expanded, sort N.
- ObjectView: items, details_model (N: two-part call), view_mode (enum, M P H S), selected_id /
  selected_ids (derived, G2), selection_mode (S), icon_cell_size (size), details_row_height,
  show_secondary_text, top_row, font (G1), image_list, horizontal_offset, details_sort (object, P S);
  details_column_width(id, w) N.

Schema: none. Tests: list_view, tree_list_view, the collection and correspondence tests; M-R (content
regions) is a later separate step with W/SS-7, not mixed into B7.

### B8 Pickers (`date_time_picker/`, `month_calendar/`, `color_value_editor/`, `flags_value_editor/`,
`font_dialog/`)

| control | property | kind | dirty | event | WF | co | schema |
|---|---|---|---|---|---|---|---|
| DateTimePicker | value | object (DateTimeValue) | P S | value_changed | yes (OnValueChanged) | no | C |
| DateTimePicker | minimum, maximum | object | P S | value_changed when value clamps | no | no | C (split from set_range) |
| DateTimePicker | format | enumeration | M P S | none | yes (OnFormatChanged) | no | C |
| DateTimePicker | custom_format | text | M P S | none | no | no | C |
| DateTimePicker | format_provider | object | M P S | none | no | no | C |
| DateTimePicker | show_check_box, show_up_down | boolean | M P H S, P H S | none | no | no | C |
| DateTimePicker | checked | boolean | P S | checked_changed (ours; WinForms raises ValueChanged) | no | no | C |
| DateTimePicker | drop_down_alignment, style | enumeration | P S, P | none | no | no | C |
| DateTimePicker | font | font | M P S | none | yes | yes | C (G1) |
| DateTimePicker | dropped_down | boolean | - | none | no | no | C |
| MonthCalendar | today_date | object | P S | none | no | no | C |
| MonthCalendar | show_today, show_week_numbers, calendar_dimensions | bool, bool, size | all | none | no | no | C |
| MonthCalendar | show_today_circle | boolean | P S | none | no | no | C |
| MonthCalendar | first_day_of_week | enumeration | P H S | none | no | no | C |
| MonthCalendar | max_selection_count, scroll_change | unsigned_integer | - | none | no | no | C |
| MonthCalendar | format_provider, font | object, font | M P S | none | no, yes | no, yes | C |
| MonthCalendar | bolded / annually / monthly bolded dates | collection | - | none | no | no | C |
| MonthCalendar | date, selection_range(first,last), range(min,max) | - | - | date_changed | yes (OnDateChanged) on the selection | - | N (multi-value) |
| ColorValueEditor | value | color | P S | none | no | no | C |
| ColorValueEditor | swatch_width | number | L P H | none | no | no | C |
| FlagsValueEditor | descriptor | object | M P S | none | no | no | C |
| FlagsValueEditor | value | enumeration (flags) | P S | none | no | no | C |
| FlagsValueEditor | dropped_down, popup_width | bool, number | -, P S | none | no | no | C |
| FontDialog (Panel) | font | font | - | none | no | no | C (G1: name clash with Control font) |
| FontDialog | size_range(min,max) | - | - | - | - | - | N |
| FontDialog | show_apply, preview_text | bool, text | A P S, P S | none | no | no | C |

Schema: none. Tests: date_time_picker, month_calendar, font_dialog tests; value/minimum/maximum
coupling order; DateTimePicker checked event kept, divergence noted.

### B9 Components and dialogs (owner D3: migrate before the seal; table generalised beyond Control&, W's design)

ColorDialog (title), OpenFileDialog (title, initial_directory, filter, file_name), SaveFileDialog (+
default_ext), FolderBrowserDialog (description, initial_directory, selected_path), NotifyIcon (icon
collection, text, visible, context_menu), ToolTip (initial/reshow/auto_pop delay, show_on_hover,
show_on_focus, show_always, maximum_width; tool_tip(target, text) N), ThreadExceptionDialog (none),
MessageBox (static facade: nothing). All text/boolean/unsigned, no dirtiness (components do not paint),
no events, WF no. TaskDialog (a Control): page (object, all, N: page replacement), expanded (boolean,
all, expanded_changed, C), verification_checked (boolean, C), progress_value (number, C).
Schema today lists 0 properties for every component, so any entry widens it (D4, W).
Plan against the generalised table: component capabilities store and raise their event but do no touch
(components do not paint; the shown host window/tool tip re-reads on show); `assign` still checks the
thread first (G4, through the component's own owner thread check); entries' thunks take whatever base W
chooses (expected `Component&`). Tests: write/equal write/wrong thread per component; host requests
(show_dialog, notify icon update, tool tip show) carry the written values; color/open/save/folder dialog,
notify_icon, windows_notify_icon, thread_exception_dialog, task_dialog tests after the switch.

### B10 ChoiceGroup (`collections/choice_group.*`, moved to me by owner D6; base Panel)

items (collection of ChoiceItem, M A P H S, none, WF no, C); selected_index (signed_integer, P S, `changed`
from its `Value<int>` selection, WF no, C; G14: whether the Value<int> stays or becomes the cell's change
source is W's call); presentation (new, enum buttons/segmented, M A P H S, none, C, the W-4 segmented
control); bind/unbind and item(i)/part(name, i) N. Tests: existing ChoiceGroup tests after the switch,
then the segmented presentation (anatomy track + selection indicator, keyboard, radio_group semantics,
reduced motion).

## W-4 small-controls batch (capability-first, new directories, mine)

Common to each: header under `include/gui_forms/controls/<area>/<name>/`, source under
`src/controls/...`, controls.json entry + registry emit/render/readme/check, anatomy (Parts/States),
semantics, interactions, docstring, Gallery specimen, schema render/check, tests. Properties written as
capabilities from the start (table entries for every authored property, so their schema is new, not
widened). New semantic roles touch `semantic_types.hpp` (w7) and new part roles/families touch
`theme_types.hpp` (T4): ask both owners first (D8). Spinner and progress ring are out (owner D5). Motion (switch thumb, toast slide, segment indicator) runs on
the frame scheduler with the reduced-motion gate.

| control | base | anatomy (parts / states) | properties (kind, dirty, event, WF) | a11y role | Gallery |
|---|---|---|---|---|---|
| ToggleSwitch | ButtonBase (reuses M3's checked capability template with ToggleSwitch as declaring class) | track, thumb (mark toggle_knob), label, focus / checked, hot, pressed, disabled, focus_visible, reduced_motion | checked (boolean, P S, checked_changed, WF yes OnCheckedChanged); text from ButtonBase; on_label/off_label (text, M P S, none) | new `switch` (UIA Button + Toggle, AX AXCheckBox/AXSwitch); fallback check_box | Value setting |
| Toast (ToastOverlay + show_toast request) | overlay in the surface root's overlay layer; one presenter per surface like tooltips | face, icon, label, actuator (action), close mark / hot, pressed, focus_visible, reduced_motion | request record: message, action label + command, duration (ms), severity (enum); interactions action_invoked, dismissed | live_region (polite; assertive for error), action as button child | Dialogs and popups |
| InfoBanner | Panel | face, edge (severity keyline), icon, caption, label, actuator (close), content (actions) | severity (enum, Y P S), title, message (text, M P S), closable (boolean, M P H S); event closed | live_region named by title; children static_text + buttons | Information display |
| Badge | Control (standalone, owner D7) | chip, label / dot | count (unsigned_integer, M P S), maximum (unsigned, M P S, default 99 then "99+"), mode (enum count/dot, M P S), show_zero (boolean) | static_text standalone; attached: suffix in the host's accessible name | Information display |
| EmptyState | Panel | icon, caption, label, content (action) | image (image, M P S), title, description (text, M P S); action child slot N | group named by title, description = description | Selection from a list (beside an empty ListBox) |
| SearchField | Panel composite owning a TextBox (TextBox is final) | well, icon (glyph), content (TextBox), actuator (clear), activity (busy) / focused, busy, disabled | text (text, M P S, text_changed, WF yes, co yes, forwards to the inner TextBox: G2), placeholder_text (P S), search_delay (unsigned ms, -), busy (boolean, P S); events search_requested (debounced), cleared; Esc clears then bubbles, Enter searches now | new `search_box` (UIA Edit "search box", AX AXSearchField); fallback text_box | Text editing |
| Segmented control | ChoiceGroup presentation (owner D6; ChoiceGroup moves to me) | track, selection (sliding indicator), items as segments / selected, hot, pressed, reduced_motion | presentation (enum: buttons/segmented, M A P H S) on ChoiceGroup; selection stays ChoiceGroup's | radio_group + radio_button children | Selection from a list |

W-4 tests per control: behaviour, events, keyboard, semantics, published parts and states, refusals,
reduced motion (switch thumb, toast slide, segment indicator), the capability write/equal-write/wrong-thread cases.

## Hand work (cases the codemod config cannot express; mark them in each family's config)

- **PropertyView (engine-owned/derived):** TextBox, RichTextBox, MaskedTextBox text; ComboBox text and
  selected_index; DomainUpDown text and selected_index; ObjectView selected_ids. `read` and `assign`
  by hand against the engine or item state; ComboBox and DomainUpDown keep their event order
  (selected_index, then text).
- **Shared Text/FontCapability adoption:** GroupBox text/font and every control owning a `font_`
  (TextBox, RichTextBox, TokenBox, ListBox, ComboBox, BreadcrumbTrail, CorrespondenceView, TreeView,
  ListView, TreeListView, ObjectView, PropertyList, DateTimePicker, MonthCalendar): delete the own
  storage; each class's `written(font)` keeps its own invalidation mask (they differ: M P S, A P H S,
  all-layout). FontDialog's `font` is the chosen value, not its face: ask W before B8 whether it takes
  FontCapability or a distinct name.
- **Coupled ranges:** NumericUpDown and DateTimePicker minimum/maximum/value split from `set_range`
  (WinForms order: range moves, value clamps, value_changed); MonthCalendar date/selection_range stay
  methods.
- **Collections:** ListBox, ComboBox, DomainUpDown `items` (element-wise compare; selection drop and its
  events in `written`); CorrespondenceView items (selection and pin events).
- **Custom thunks:** PictureBox image (reset, should_serialize) and size_mode (converter); the
  text_items_property items thunks; ComboBox selected_index (optional<size_t> stored, int64 in the
  schema, -1 for none).
- **Setters with side effects beyond a touch:** Panel background (after M2's back_color defect fix),
  TextBox multiline / password_character / use_system_password_character (engine reconfiguration),
  MaskedTextBox mask / culture / prompt_character (reformat; mask_changed), ComboBox drop_down_style
  (editor rebuild), RichTextBox control_bar_visible (child), Card selected (selection behaviour),
  TaskDialog expanded, DateTimePicker show_check_box / format (field rebuild), ListView view / columns.
- **Components (B9):** each component's own thread check in `prepare_write`, no touch; binding, loader
  and C ABI paths widened to `Component&` through W's free functions.
- **ChoiceGroup:** the new `presentation` property (segmented control) and its anatomy; selection stays
  `Value<int>` (G14).

## Interface gaps in M1 (for W, through the coordinator; answered in study section 14)

- **G1 text and font on Control (M2 plan) vs per-class storage.** Control has neither today. GroupBox,
  TextBox, RichTextBox, MaskedTextBox, ComboBox, DomainUpDown own `text`; about 18 of my controls own a
  `font_` with their own `set_font`. If M2 puts `text`/`font` cells on Control, a form writing `font`
  through the table would write Control's cell while the subclass paints its own `font_`, and a subclass
  cannot declare a second `text` cell (study 2.2). Needs a rule before M2 lands (D1).
- **G2 Engine-owned and derived values have no cell form.** TextBox/RichTextBox/MaskedTextBox text live
  in the editing engine; ComboBox text/selected_index, DomainUpDown text/selected_index, ObjectView
  selected_ids derive from item state. A PropertyCell would be a second owner. Needs either a cell-less
  sibling with the same spelling (`operator=` to `Capability::assign`, `.read()` returning by value
  through the declaring class) in M1's header, or a rule that such properties stay getter/setter with
  table thunks (D2).
- **G3 A derived table cannot replace a base entry.** `join_property_tables` appends and
  `property_names_distinct` forbids duplicates, so a subclass cannot refine a base property's entry
  (description, dirtiness, change connector) the way a registration replaces one today. Needed if G1
  puts text/font on Control: a consteval join-with-override by name that checks the name exists in the
  base.
- **G4 Thread check before compare.** `PropertyCell::operator=` compares before any touch, so an equal
  write from another thread passes silently and races the read. Every `assign` must call
  `require_mutable()` first; please make that the M2 convention (or a shared helper) and test it.
- **G5 Access.** `assign` (a static member of a non-derived struct) needs the declaring class's protected
  members (`require_mutable`, `publish_change`, events, `on_x_changed`, `coerce_x`). The M1 test uses
  public members; please fix the convention in M2 (`friend struct XCapability<Declaring>;` in the
  declaring class works: a friend of D reaches B's protected members through D).
- **G6 Events through publish_change.** The M1 test emits directly; begin_init/end_init coalescing needs
  `publish_change`. One helper from M2 used by every family.
- **G7 Components have no table.** `property_table()` and the `PropertyEntry` thunks take `Control&`;
  ToolTip, NotifyIcon, the dialogs, ErrorProvider and Timer derive from Component and list 0 schema
  properties. Either the table moves to Component (thunks take `Component&`) before the 0.5 seal, or
  components keep setters through 0.5 (D3).
- **G8 Child-property table.** Study 10.8 wants a second table per container; M1 has no type for it and
  `define_child_property` is not in w2/m0-settle. My share provides no child properties today (the
  compat layout panels are W's), so low impact; ScaledPanel/ScaledGroupBox design_bounds would use it later.
- **G9 Ordering with ScrollableControl.** Panel derives from ScrollableControl (W's, 4 registered
  properties). Please migrate ScrollableControl right after M3, before my B1; otherwise Panel joins
  Control's table and W later edits panel.cpp to re-base the join.
- **G10 Structural slots stay methods.** Card header/body/footer, AnchoredPopupLayer anchor/content,
  InstrumentRack action_content, PropertyList header_content, PropertyGrid selected_object hold
  children or observed controls; a cell would duplicate the children list's ownership. Proposed: N.
- **G11 Panel back_color-when-themed defect is in M2 and touches panel.cpp.** W fixes it in M2; Panel's
  `background` override may fold into back_color there. I start B1 after M2, on W's result.
- **G12 Getter-switch script.** Names that already exist as members with other types (`items()` returns
  a span today and the cell holds a vector; `value()`; `selected_index()` returning optional vs int64 in
  the schema) need per-family exceptions; please make the script take an explicit name list per class.
- **G13 Multi-value setters** (set_range, set_size_range, set_selection_range) split into separate cells
  with WinForms coupling; the coupled write goes through the other cell's public `operator=` (so it
  touches and raises), which M1 already allows. No change needed; noted for review.
- **G14 Value<T> observables** (ChoiceGroup's selection, others) sit beside capabilities; the migration
  should say whether `Value<T>` stays (W-4's segmented control rides ChoiceGroup's).

## Decisions (as asked; answers in "Owner answers" above)

- **D1 (G1) text and font:** (1, rec.) keep text and font per declaring class (as today and as study 2's
  `TextCapability<ButtonBase>`), with a shared `FontCapability<Declaring>` template each declaring class
  takes; Control gets no text/font cell in M2. (2) Control's cells, subclasses override
  `on_font_changed`/`coerce_text` and drop their storage (needs G2 for TextBox and G3 for entries).
- **D2 (G2) engine-owned/derived values:** (1, rec.) a cell-less `PropertyView<Value, Capability>` in
  M1's header with the same spelling; (2) they stay `set_x`/`x()` with table thunks.
- **D3 (G7) components:** (1, rec.) components keep setters through 0.5, revisit after the seal (their
  schema is empty; nothing is lost); (2) move the table to Component before the seal.
- **D4 schema widening for C properties:** (1, rec.) cells only in M4; adding entries is a separate,
  reviewed step per form need (FD/C ABI), so every family stays byte-equal; (2) entries for every cell
  now (schema and C ABI grow with each family).
- **D5 spinner/ring:** (1, rec.) one ProgressRing type on RangeControl with `indeterminate`; (2) two
  types.
- **D6 segmented control:** (1, rec.) a ChoiceGroup presentation (ChoiceGroup is in collections, "the
  rest": hand it to me); (2) a new type.
- **D7 badge:** (1, rec.) standalone Badge now, host items adopt a BadgeValue later through their owners;
  (2) badges only as item properties of PageStrip/ToolStrip/nav items (W's files).
- **D8 roles:** new semantic roles `switch` and `search_box` (w7) and any part role/family (T4); ask
  before coding.

## Order of work (after M3 lands)

1. Rebase on origin/main; read M2/M3 as landed (thunk helpers, hooks convention, getter script); fold
   the D1-D8 answers into this file.
2. B1 Panel (after G9: ScrollableControl migrated), then B2, B3 (TextBox first), B4, B5, B6.
3. W-4 alongside, one control per change: InfoBanner, EmptyState, Badge (Panel/Control only), then
   SearchField (after B3), ToggleSwitch (needs M3's checked template), Toast (overlay presenter),
   segmented ChoiceGroup presentation (ChoiceGroup migrated as its family first). Ask the coordinator to
   clear the D8 roles before the first W-4 step.
4. B7 item views, B8 pickers, B9 components and dialogs (after W's generalised table), ChoiceGroup, then
   the rest as assigned.
Each step: one branch commit series; touched tests plain and sanitized on the build host, house style,
schema render --check, registry/docs checks touched, state-machine gate `--changed-from origin/main`.

## Next step

IDLE, no builds. When M3 lands: rebase, rerun generate_configs.py against the new checkout (check its
lookup report), then B1 after ScrollableControl with W's codemod and these configs; hand work per the
`hand` lists; review generated comments before each family's commit. Left to confirm with W at B8:
whether DateTimePicker's `style` should reuse Panel's override cell instead of its own.

## cp21 M-G (CLOSURE P-6), 2026-11-08 (w6)

Done on claude/families_b-b5: merge 8548e0ab (w7 fd6-bindings 7d7fe052, two-way into table entries via codemod "binding_direction"), 1d4f60bc (segmented edges snapped to device pixels, batch4 return of 7e70c10e), b06b3253 (generator assigns capability cells through PropertyDescriptor::assignable_member and assigned_property_value; unmigrated properties stay by name). Next migration of a family flags itself through the codemod; hand tables set assignable_member where the write is the cell's assignment.

## M-S survey (CLOSURE P-7 + C-8), 2026-11-08 (w6): plan in desks/coordinator/MS_PLAN.md

- Survey at b06b3253; scripts in the w6 scratchpad (ms_survey.py, ms_registry.py, ms_pairs.py, per_header.txt).
- Duplicates of a capability: only ScrollableControl::set_auto_scroll_margin(x, y) (3 callers incl. the 0.x shim).
- P-4 is NOT complete. Registry: 26 classes, about 75 properties (Label/TextBlock/TypographySpecimen, CheckBox/RadioButton/Button, ranges, layout panels, ComboBox/ListBox/DomainUpDown items, FlagsValueEditor, Control name/bounds/auto_scroll_offset). Unmigrated: 59 headers with no capabilities, 352 public setters (split, pages/strips, ribbon, docking, components, ...).
- C ABI schema today: per-type instance recipe, then property_descriptors() (registry + virtual table, sorted). C-8 needs uniform static table accessors in the emit-abi catalog, the registry emptied, and a foreign-object fallback. gf_abi.h is name-based, so audit (report mode) expects 0 findings; schema content changes hit the byte-equal gates instead.
- Steps: M-S1 auto_scroll_margin, M-S2 text leaves, M-S3 buttons, M-S4 ranges, M-S5 layout panels, M-S6 remaining registry, M-S7 collections (G14), M-S8.. non-registry families (cells only, off the C-8 path), then C-8a static walk, C-8b ABI writes through entries, C-8c registry plumbing removed. Five decisions open (MS_PLAN section 7).

- 2026-11-08 M-S2a Label committed 39420051 on claude/families_b-b5 (forwarders Label::set_text, set_font for the integrator rewrite); next M-S2b TextBlock and TypographySpecimen.

- 2026-11-08 base repair 82b9fbd1 (w8 report + M4 syntax check) and M-S2b TextBlock/TypographySpecimen b7c73951; next M-S3 buttons.

## 2026-11-08 M-S sweep (code first, verify once; coordinator)
Scope: whole remaining migration M-S3..M-S7 + C-8a..c in ONE sweep on claude/families_b-b5, no per-item commits/queue/M4/docs.
Delete the transient forwarders (Label/TextBlock/TypographySpecimen set_text/set_font) and rewrite callers directly. Leave w8's M-S8 headers.
End: one M4 compile check (m4ws w6 sh .m4_syntax.sh <TUs>; scratch helper m4fix.py applies cell/setter fixes from its output), one queue entry, <=5-line report.
Progress:
- M-S3 source done (uncommitted): check_box.json (views check_state/checked, bound_checked for Value<bool>), radio_button.json (checked view, clear_checked_peer), button.json (visual_style, flat_border_width); values headers check_box_values.hpp, button_values.hpp. Callers not yet swept.
- Remaining registrations (git grep define_bindable_property): button_base image; easing_preview title; sweep_scope caption/sweep_period; combo_box items; domain_up_down?; flags_value_editor; list_box selection_mode; progress_bar visual_style/effect*; range_control value/min/max; scroll_bar large_change; track_bar tick_frequency/visual_style; layout panels (block/flex/grid/flow/table/responsive/command_overflow incl. child props); visual_inspector target_stable_id; control.cpp name/bounds/auto_scroll_offset (structural); ChoiceGroup/TabControl selected_index (G14).
- 2026-11-08 sweep committed 9b947eec on claude/families_b-b5 (M-S3..M-S7 + C-8, forwarders deleted), queued uncompiled; final M4 run stopped per owner directive.
- 2026-11-08 rule migration committed 67ad80bb on claude/families_b-rule-migration (base 0da5baa1), queued uncompiled.
- 2026-11-08 items on f16f8fd9: Gallery grounds to theme d28c726b, P-8 c76e8652, C-10/C-12 5308d7d8 on claude/families_b-b5; template_harness claude/families_b-c10 d423a57; both queued unverified.
- 2026-11-08 codemod generate guard ba6a88ca; docs/examples 0.5 sweep 0125d0c7; template_harness claude/families_b-x3-pattern 1d7ac74.
- 2026-11-08 EP-3 effect-name registry check (E417) b3638f23 on claude/families_b-ep3 (base 06a78004), queued unbuilt.
- 2026-11-08 dark-scheme grounds fix f9e30b65 on claude/families_b-dark-grounds (base 819f8114), queued unbuilt.
- 2026-11-08 rack modules + EasingPreview on theme roles 4e6ace8b on claude/families_b-dark-grounds, queued unbuilt.
- 2026-11-08 module severity states + Gallery easing 1fa1996e on claude/families_b-dark-grounds, queued unbuilt.
