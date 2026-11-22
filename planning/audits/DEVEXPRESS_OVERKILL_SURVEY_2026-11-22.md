# DevExpress takes: the overkill survey

Date: 2026-11-22. A follow-up to `DEVEXPRESS_ROUNDUP_2026-11-22.md`
(revision 2). Owner (2026-11-22): "I dont want data grid features unless they
can all be added in one swift pass. i need to see a survey on what controls to
add where it might be overkill."

Same clean-room rule as the roundup: behaviour and appearance from
DevExpress's public demos and documentation only. The grid section reads only
our own code (`include/gui_forms/controls/data_grid_view*.hpp`,
`src/controls/data_grid_view/`, `src/abi/stable/tables/abi_data_grid.cpp`).

**Who uses it** names the owner's programs where one would use the item:
Plan Paint (painting), SwiftEdit (code editor), the file manager demoboard,
PlaySuite, and the SDR-type apps (the SDR# reflection and the instruments).
"Business apps" means ordinary data-entry applications written in C# or C++.

**Cost** is the library surface (new public types and members, plus C ABI
rows where the control is in the stable ABI) and the maintenance it brings
(themes, three hosts, semantics, tests, docs). None means a Gallery page only.

**Verdicts:**
- **worth it**: add it to the library;
- **demo only**: build it as a Gallery page from existing parts, with no new
  API, and note any gap it finds;
- **overkill**: don't build it (reason given).

## 1. Tier-1 takes

| Take | User gets | Who uses it | Cost | Composes today? | Verdict |
|---|---|---|---|---|---|
| Grid grouping | Rows fold into groups under a column dragged to a strip above the grid. | Business apps; file manager (group by type or date, which ListView groups already do) | High: group rows are a new row kind across the row order, heights, hit tests, keyboard, selection, semantics, the cells sub-surface and the ABI (section 3) | No | **overkill** for 1.0: not one pass (section 3) |
| Grid summaries | A footer under the grid and each group with sum, count or average. | Business apps | High: footer and group-footer rows, as for grouping | Partly: a footer of Labels under the grid, computed by the app | **demo only** (an app-computed footer row); a built-in summary is overkill |
| Find panel | One box filters rows by any column and highlights matches. | Business apps; file manager (filter the current folder); SwiftEdit (symbol lists) | Medium: unbound rows need a grid-side filter, which the grid lacks; bound and virtual rows filter in the app already | Yes for virtual rows: SearchField plus `set_virtual_row_order`; highlighting is a CellFormatting handler | **demo only** |
| Auto-filter row and Excel-style filters | A filter cell under each header and a drop-down of distinct values to tick. | Business apps | High: a filter row, the distinct-value drop-down, and in window mode the grid cannot know the values at all | No | **overkill** |
| Column chooser | A window listing hidden columns to drag back. | Business apps; file manager (choose columns) | None: columns already have `visible` | Yes: a CheckedListBox of the columns toggling `visible` | **demo only** |
| Any editor in any cell | A cell edits with any editor (date, lookup, colour, spin). | Business apps | High: the grid has two editing kinds (`text_box`, `choice_list`); a general hosted-editor contract touches focus, parse and commit, the sub-surface and the ABI | No | **overkill** for 1.0 |
| Grid layout save and restore | Column order, widths, sort and visibility come back on the next start. | Business apps; file manager | None to low: widths, display indices, `visible` and the sorted column are already public | Yes: the app writes them to its settings | **demo only** |
| Tree List node checkboxes | Tick boxes on tree nodes that check their children. | File manager (choose folders to copy); installers; SDR apps (channel trees) | Low: a check state per node, the tri-state rule and one event | Partly: TreeView has check boxes; TreeListView lacks them | **worth it** (the checkboxes only; tree filtering and summaries are overkill, like the grid's) |
| ButtonEdit | A text box with buttons of your choice inside its edge. | File manager (path box), SwiftEdit (find box options), SDR apps (frequency entry), Plan Paint (file fields) | Low: one control, generalising SearchField's buttons | Partly: SearchField has fixed buttons | **worth it** |
| CheckedComboBoxEdit | A drop-down with tick boxes; the box shows the ticked items. | File manager (file-type filter), PlaySuite (genre filter), business apps | Low to medium: one control on ComboBox and CheckedListBox | Partly: DropDownButton plus CheckedListBox, but no summary text, typing or keyboard model | **worth it** |
| PopupContainerEdit | A combo-style box whose drop-down holds any panel. | Every drop-down demo below; Plan Paint (brush picker), SDR apps (band picker) | Low: one control on AnchoredOverlayLayer | Partly: DropDownButton opens a menu, not a panel | **worth it**: it turns about eight tier-2 items into demos |
| Input box | A prompt with one editor and OK and Cancel. | Every app (rename, "go to line" in SwiftEdit, "new folder" in the file manager) | Very low: one function and a small dialog | Partly: TaskDialog has no editor | **worth it** |
| Pie and doughnut charts | Round charts of slices with labels and a legend. | Business apps; file manager (disk usage); PlaySuite (library breakdown) | Medium: a new series kind with its own hit test, labels and legend in PlotView, which today is axis-based | No | **worth it** |
| OS toast notifications | A notification in the system notification centre. | PlaySuite (now playing), file manager (copy finished), SDR apps (signal found), SwiftEdit (build finished) | Medium: three host back ends (macOS UserNotifications, Windows toasts, Linux freedesktop notifications) and permission handling | No: Toast is in-window only | **worth it** |
| MDI ribbon and toolbar merging | A document window's tabs and buttons join the main ribbon while it is active. | Business apps converted from WinForms (MDI is WinForms parity); SwiftEdit if it goes MDI | Low: finishes the cp36 MDI follow-ups; WinForms has `ToolStripManager.Merge` | Partly | **worth it** (WinForms parity, already under way) |

## 2. Tier-2 takes

| Take | User gets | Who uses it | Cost | Composes today? | Verdict |
|---|---|---|---|---|---|
| Card, layout and tile views | Records as a wall of cards instead of rows. | PlaySuite (album and track cards), file manager (large-icon view with details) | None | Yes: VirtualRowsView or FlexPanel of Cards | **demo only** |
| Office compact task list | Outlook's two-line task rows. | Hardly anyone outside an Outlook clone | None | Yes | **overkill**: too Outlook-specific to be worth a page |
| Vertical Grid | One record as a two-column field and value table. | Business apps (record detail) | None | Yes, but PropertyGrid and ObjectView already show a record this way | **overkill**: PropertyGrid covers it |
| MemoEx box | A one-line box that drops down a multi-line editor. | Rare | None | Yes, with PopupContainerEdit | **overkill**: a multiline TextBox does the job |
| Calculator drop-down | A number box with a calculator. | Rare (accounting) | None | Yes, with PopupContainerEdit | **overkill**: users have a calculator |
| Zoom slider with minus and plus | A status-bar zoom control. | Plan Paint, SwiftEdit, the file manager's preview | None | Yes: TrackBar and two Buttons | **demo only** |
| Duration box | Durations with spin buttons per part. | PlaySuite (track lengths), SDR apps (recording timers) | None | Yes: MaskedTextBox or NumericUpDowns | **demo only** |
| Date range picker | Two dates on side-by-side calendars with presets. | Business apps (reports by period), file manager (modified between) | None | Yes: two MonthCalendars, or one with a selection range, in PopupContainerEdit | **demo only** |
| Grid and search lookups | A lookup whose drop-down is a sortable grid with a search box. | Business apps | None | Yes, with PopupContainerEdit, DataGridView and SearchField | **demo only** |
| Tree lookup | A lookup whose drop-down is a tree. | File manager (folder picker in a dialog) | None | Yes, with PopupContainerEdit and TreeListView | **demo only** |
| Rating | Stars to click for a rating. | PlaySuite (track ratings), file manager (photo ratings) | None | Yes: a ChoiceGroup with star icons | **demo only** |
| Image slider | One picture at a time with arrows and a slide. | PlaySuite (cover art), file manager (image preview) | None | Yes: PictureBox and animation | **demo only** |
| Thumbnail gallery | A scrolling grid of thumbnails in groups. | File manager (photos), Plan Paint (brush presets), PlaySuite (albums) | None | Yes: ListView tiles with groups | **demo only** |
| Transitions | Content changes with a slide or fade. | PlaySuite, the Gallery itself | None | Yes: animation | **demo only** |
| Workspaces | Save and switch whole-window arrangements by name. | SwiftEdit (layouts per task), SDR apps (receiver layouts) | None | Yes: dock layout save and restore | **demo only** |
| Guided tour | A first-run tour spotlighting one control at a time. | Every shipped app's first run; Plan Paint | None | Yes: ModalLayer and Callout | **demo only** |
| Accordion | A side menu of collapsible groups that shrinks to icons. | Business apps, settings windows | None | Yes: ExpandableSections and NavigationPane | **demo only** |
| Office navigation bar | Outlook's Mail, Calendar, People strip. | An Outlook clone only | None | Yes | **overkill**: too Outlook-specific |
| Tile Bar | A strip of big coloured tiles across the top. | Hardly anyone today (Windows 8 era) | None | Yes | **overkill**: dated look, no program of ours wants it |
| Tile navigation pane | A top bar whose categories open bands of tiles. | As Tile Bar | None | Yes | **overkill**: as Tile Bar |
| Mini toolbar | A formatting toolbar beside selected text. | SwiftEdit, Plan Paint (text tool) | None | Yes: CommandBar in AnchoredOverlayLayer | **demo only** |
| Command search ("Tell me") | Type a command's name and run it. | SwiftEdit (command palette), Plan Paint, every Ribbon app | None for a demo; a small helper over the command registry if SwiftEdit wants it in the library | Yes: SearchField and commands | **demo only**, moved into the library if SwiftEdit's palette needs it |
| Radial menu | A round pop-up menu around the pointer. | Plan Paint (pen and touch) | None | Yes: DrawingSurface and commands | **demo only** |
| WindowsUI View | A full-window tablet-style tile app. | Nobody we know | None | Yes | **overkill**: Windows 8 era, covered by touch work if it ever matters |
| Widget View | A dashboard of rearrangeable cards. | SDR apps (instrument dashboards), business dashboards | None | Yes: Cards in GridPanel or DockWorkspace | **demo only** |
| TreeMap | Nested rectangles sized by value. | File manager (disk usage) | None | Yes: DrawingSurface | **demo only** |
| Sunburst | A hierarchy as rings of slices. | File manager (disk usage, a second look) | None | Yes, with the pie series once it exists | **overkill**: one disk-usage view (TreeMap) is enough |
| Scheduler, Gallery version | A week-view page with no scheduler control behind it. | Nobody: it would look like a control we don't have | None | Yes | **overkill**: misleading without the control |
| Theme and palette gallery | A ribbon gallery that switches themes live. | Every themed app; the Gallery | None | Yes: RibbonGallery and user themes | **demo only** |
| AI chat panel | A chat panel with bubbles and streamed replies. | SwiftEdit (assistant), the MCP work | None | Yes: the chat stress demonstrator already is one | **demo only** (point the Gallery at chat stress) |
| Carousel | Rotating slides with dots. | PlaySuite (featured albums) | None | Yes: PageView and animation | **demo only** |
| Drawer | A side panel that slides over the content. | Settings panes, small windows | None | Yes: NavigationPane and animation | **demo only** |
| Pager | Page-number buttons under a list. | Hardly anyone: desktop lists scroll | None | Yes | **overkill**: a web idiom; VirtualRowsView scrolls millions of rows |
| Bar gauge | Several values as concentric arcs. | SDR apps (levels), business dashboards | None | Yes: DrawingSurface or stacked RadialGauges | **demo only** |
| Outlook-Inspired demo application | An Outlook look-alike with mail, tasks and contacts. | A showcase only | None, but a large demo to keep working | Yes | **overkill**: the file manager already is our "real application" showcase |
| HTML-template cards demo | Records drawn as styled cards from markup. | Shows off form documents | None | Yes | **demo only** (one Gallery page, not an application) |
| Touch and tablet demo applications | A tablet-style app with large touch targets. | Touch work (week plan 3.1) | None | Yes, once touch lands | **demo only**, with the touch work |
| Fluent Mail look-alike | A Windows 11 Mail look-alike. | A showcase only | None | Yes | **overkill**: the window backdrop lab already shows the look |
| CRM dashboard | Grids and charts on one dashboard. | Business apps | None | Yes, once pie charts exist | **demo only** |
| Visio-style flowchart | A flowchart editor. | Node graph demonstrator already | None | Already exists | **overkill**: the node graph demonstrator covers it |
| Gallery source button | Each Gallery page shows the code or form document behind it. | Every GUI.Forms learner | None in the library; a Gallery feature | Yes | **worth it** (Gallery feature) |
| UI templates | Ready sign-in, settings, record and dashboard screens as form documents. | Every new app; the WYSIWYG editor's starting points | None in the library | Yes | **worth it** (Gallery content) |

## 3. The data grid as one block

### 3.1 The grid features in the takes

From the roundup (tiers 1 and 3): grouping with the group-by panel, total
and group summaries, the find panel, the auto-filter row and Excel-style
filters, the column chooser, any editor in any cell, layout save and restore,
conditional formatting, banded views, cell merging, HTML row templates, and
tree list filtering, summaries and node checkboxes.

### 3.2 DataGridView as it is today

- About 4,100 lines of public headers, 9,700 lines of sources (`data_grid_view.cpp`
  5,630, `data_grid_view_parts.cpp` 2,576, row order 464) and 1,116 lines of
  stable C ABI tables. About 520 public member declarations in
  `data_grid_view.hpp` alone.
- **Three row sources** (`DataGridViewRowSource`: unbound, virtual rows,
  BindingSource) and two virtual row-order modes (`edit`, where the grid holds
  the whole order, and `window`, where the application keeps the order and
  the grid asks only for the IDs at the positions it shows).
- **Sorting** works only on unbound rows: `sort()` returns false for virtual
  or bound rows. **Filtering** is the application's job: it passes a new order
  to `set_virtual_row_order` (ObjectView does this).
- **Every row is a data row** with a `DataGridViewRowId`. The row order,
  `DataGridViewRowHeights`, hit testing (`DataGridViewHitTestType`),
  selection, keyboard navigation, semantics (each cell's semantic ID is the
  row ID plus the column name), the cells sub-surface snapshot
  (`DataGridViewCellsSnapshot`, cells in row then column order) and the ABI
  tables all assume it.
- **Editing** has two kinds (`DataGridViewEditingControlKind`: `text_box`,
  `choice_list`). Column kinds are text, check box, combo box, button,
  image, link and sparkline.
- Already there: column `visible`, display index, width, frozen columns and
  rows, sort glyphs, CellFormatting, RowPrePaint and RowPostPaint, owner
  drawing.

### 3.3 Does it all fit one swift pass?

| Feature | Fits a short pass? | Why |
|---|---|---|
| Column chooser | yes | `visible` exists; it is a CheckedListBox |
| Layout save and restore | yes | every value is public; a helper or app code |
| Find panel | partly | virtual and bound rows filter in the app already; unbound rows need a grid-side filter that doesn't exist |
| Conditional formatting | partly | CellFormatting can colour cells now; data bars and icon sets need new cell parts in the painter, the snapshot and the theme |
| Tree list node checkboxes | yes | a TreeListView change, not a DataGridView one |
| Auto-filter row and Excel-style filters | no | a non-data filter row; distinct values are unknowable in window mode; unbound rows need the missing filter |
| Grouping | no | group rows are a second row kind through every path in 3.2, plus a filter, sort and group pipeline across all three row sources; window mode can't group at all without the application |
| Summaries | no | footer and group-footer rows, the same row-kind change |
| Any editor in any cell | no | a general hosted-editor contract replacing the two editing kinds, through focus, parse, commit, the sub-surface and the ABI |
| Banded views, cell merging, HTML row templates | no | each changes the cell layout model (multi-row records, cells spanning rows, templated rows) |

**Blockers to one pass:**

1. **One row kind.** Group rows, group footers, the summary footer and the
   filter row are not data rows. Adding a second row kind touches the row
   order, heights, hit testing, selection, keyboard, semantics, the
   sub-surface snapshot, painting and the ABI.
2. **No view pipeline over the three row sources.** Sorting is unbound-only
   and filtering is the application's. Grouping, Excel filters and the find
   panel need one filter, sort and group stage that works for unbound, bound
   and virtual rows alike. In window mode the grid never holds the order, so
   it cannot do any of them without new application callbacks.
3. **Two editing kinds.** Any editor in any cell is a new contract, not a
   flag.
4. **The stable ABI.** `abi_data_grid.cpp` mirrors the grid; every new kind,
   event and row type is a versioned ABI addition with C# bindings to match.
5. **WinForms parity.** DataGridView follows Windows Forms' DataGridView,
   which has none of these features; each one is a GUI.Forms extension that
   has to be specified, documented and kept apart from the compatibility
   contract.

**Verdict: no, not in one swift pass.** Grouping, summaries, Excel filters
and any-editor-in-any-cell need the row-kind and view-pipeline changes first.
That is a grid programme of its own, after 1.0. What is cheap is already
mostly possible: the column chooser, layout save and restore and the find
panel over virtual rows are Gallery demos with no new API.

## 4. Summary

**Take (10): into the library (8):** PopupContainerEdit, ButtonEdit,
CheckedComboBoxEdit, the input box, pie and doughnut charts, OS toast
notifications, MDI ribbon and toolbar merging, and TreeListView node
checkboxes; and two Gallery features with no library API: the source button
and the UI templates.

**Demo only (30):** app-computed summaries, the find panel, the column
chooser, layout save and restore, card views, zoom slider, duration box, date
range, grid and tree lookups (two pages), rating, image slider, thumbnail
gallery, transitions, workspaces, guided tour, accordion, mini toolbar,
command search, radial menu, widget view, treemap, theme gallery, AI chat
panel, carousel, drawer, bar gauge, HTML-template cards, the touch demo
application (with the touch work) and the CRM dashboard.

**Overkill (17):** grid grouping, Excel-style filters, any editor in any
cell, the Office compact task list, Vertical Grid, MemoEx, the calculator
drop-down, the Office navigation bar, Tile Bar, the tile navigation pane,
WindowsUI View, Sunburst, the Gallery scheduler, the pager, the
Outlook-Inspired application, the Fluent Mail look-alike and the Visio-style
flowchart (the node graph demonstrator covers it). Tree list filtering and
summaries go with the grid's.

**The grid: not one pass.** No grid features for 1.0 beyond what composes
today. A grid programme (a second row kind and a view pipeline over all row
sources) after 1.0 would unlock grouping, summaries, Excel filters and
editors in cells together.
