# DataGridView breadth audit (2026-11-01)

The 513 `System.Windows.Forms.DataGridView` rows of
`planning/generated/WINFORMS_API_CATALOGUE.tsv` (all still marked
`admitted / missing` there), classified by member name against `main`
e1262237 and gap B19. A row counts as `covered` when GUI.Forms has the
member or an equivalent; the `On*` overrides count with their events. The
counts are approximate: the classification is by name pattern.

| Bucket | Rows | State |
|---|---:|---|
| covered | 97 | columns and rows, current cell, selection (cell and full row, multi), edit begin/end/cancel, validating, value changed, content click, virtual rows (provider by row ID), BindingSource, headers, scrolling, programmatic sort of unbound rows, semantic children |
| data errors and validation | 23 | missing: `DataError`, cell and row error text (and the `*ErrorTextNeeded` pair), `ShowCellErrors`, `ShowRowErrors`, `CellValidated`, `RowValidating`, `RowValidated` |
| row editing and dirty state | 38 | missing: the new row (`AllowUserToAddRows`, `NewRowIndex`, `DefaultValuesNeeded`, `UserAddedRow`, `NewRowNeeded`), deleting (`AllowUserToDeleteRows`, `UserDeletingRow`, `UserDeletedRow`), `CancelRowEdit`, dirty state (`IsCurrentCellDirty`, `IsCurrentRowDirty`, `CommitEdit`, `NotifyCurrentCellDirty`, `CurrentCellDirtyStateChanged`), `EditMode`, `RefreshEdit`, `ShowEditingIcon`, `RowTemplate` |
| cell and row events | 58 | mostly missing: cell click, double click and mouse events, cell/row enter and leave, row header clicks, rows added/removed, column added/removed and property-changed events, `Scroll` |
| styles and formatting | 78 | missing: `DefaultCellStyle` and its inheritance (column, row, alternating rows, headers), `CellFormatting`, `CellParsing`, `GridColor`, border styles, `CellPainting`, row pre/post paint |
| sizing and column order | 70 | missing: automatic column and row sizing, row resizing, user column reordering (`DisplayIndex`), divider widths, header size modes |
| sort | 8 | partial: `SortCompare`, `Sorted`, `SortOrder`, `SortedColumn`, sort modes |
| tooltips and context menus | 18 | missing: cell tooltip text, per-cell/row/column context menus |
| geometry and invalidation | 21 | missing: `HitTest`, display rectangles, displayed counts, first displayed column, `Invalidate*`, `UpdateCellValue` |
| clipboard | 3 | missing: `ClipboardCopyMode`, `GetClipboardContent` |
| host plumbing | 96 | not portable as such: `Process*Key`, `WndProc`, `CreateParams`, `Set*Core`, Control overrides, background image, `DataMember`, `AutoGenerateColumns` |
| unclassified | 3 | `SelectedColumns`, `VerticalScrollingOffset` |

The classifier is `dev/tools/dgv_audit.py` in the W workspace.

## Batch 1: the editing lifecycle (owner, 2026-11-01)

Accepted: data errors, validation, dirty state, the new row, deleting rows,
`CancelRowEdit`, and cell and row enter and leave, on `claude/w-dgv-breadth`.

- **B1-1 (a):** a `DataError` with no handler marks the cell invalid and
  keeps the old value; no modal dialog is shown.
- **B1-2 (a):** the new row is offered for unbound and BindingSource grids
  only; virtual grids get it later through a provider call.
- **B1-3 (a):** the row header's editing icons (the pencil on the current
  row while it is dirty, the star on the new row) are drawn through glyph
  roles; their look goes to T4.

### Batch 1 as built (2026-11-01, `claude/w-dgv-breadth`)

- `data_error` (parsing, commit) with `cancel` defaulting to true; clearing
  it discards the edit. Check box and choice stores raise it too.
- `cell_validated`, `row_validating` (cancellable), `row_validated`;
  `cell_enter`, `cell_leave`, `row_enter`, `row_leave`, in the order the
  header states; `set_current_cell` from their handlers throws
  `std::logic_error`.
- `edit_mode` (five modes), `commit_edit`, `refresh_edit`,
  `is_current_cell_dirty`, `is_current_row_dirty`,
  `notify_current_cell_dirty`, `current_cell_dirty_state_changed`.
- `allow_user_to_add_rows` and `allow_user_to_delete_rows` (both off by
  default, a worker choice: Windows Forms defaults them on, which would
  change every existing grid), `new_row_index`, `default_values_needed`,
  `user_added_row`, `user_deleting_row`, `user_deleted_row`,
  `cancel_row_edit` with `row_edit_cancelled` (the catalogue's
  `CancelRowEdit` maps to these).
- Error text by row ID for every source, a bound row falling back to its
  record's errors; `show_cell_errors`, `show_row_errors`,
  `show_editing_icon`; parts `cell_error` and `row_error`, rows gain the
  `invalid` state; marks `pencil`, `asterisk`, `error` (provisional
  drawings, look to T4).
- Left for later: the new row in virtual mode (a provider call),
  `CellErrorTextNeeded`/`RowErrorTextNeeded`, validation when focus leaves
  the grid, error tooltips.
