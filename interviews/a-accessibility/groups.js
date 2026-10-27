const GROUPS = [
  { key: "A", title: "Roles", blurb: "What a screen reader is told a thing is. Today every tree, list, grid and table publishes as list and list item, header cells as buttons, and dialogs and the status strip as plain groups. Adding a role is your decision (design language §12.7). Each new role maps to every host: MSAA, UI Automation, AT-SPI and macOS AX all have these.", questions: [
    { id: "A-R1", q: "Admit tree roles?",
      ctx: "TreeView, the object view and BreadcrumbTrail publish their items as list items with the depth written into the description. A screen reader then cannot announce “level 3, expanded, 4 of 7” the way it does in Explorer.",
      opts: [
        { t: "Admit tree and tree_item", w: "Items gain a level field beside the existing position-in-set and set-size, and keep their expanded state. Every host has a native tree role, so readers announce level and expansion properly.", rec: true },
        { t: "Keep list and list item", w: "No new atoms; depth stays in the description text, which readers speak as words rather than as structure." } ] },
    { id: "A-R2", q: "Admit table and grid roles?",
      ctx: "DataGridView and ListView in details mode publish rows as list items and column headers as buttons. A reader cannot move by column or announce “row 12, column Size”.",
      opts: [
        { t: "Admit table, row, cell, column_header and row_header, with row and column index and count fields", w: "Readers navigate by row and column and announce the header for each cell. This is what UIA's Grid and Table patterns, AT-SPI's Table and AX tables expect.", rec: true },
        { t: "Admit table and cell only", w: "Fewer atoms, but headers and row grouping stay invisible, so a reader cannot say which column a cell is in." },
        { t: "Keep list and list item", w: "No new atoms; tables read as long flat lists." } ] },
    { id: "A-R3", q: "Tree-grids (TreeListView)?",
      ctx: "TreeListView is a tree whose rows have columns.",
      opts: [
        { t: "A tree_grid role: table semantics with tree_item rows", w: "Maps to UIA's DataGrid with tree items, AT-SPI's tree table and AX's outline. Needs A-R1 and A-R2.", rec: true },
        { t: "Publish it as a tree", w: "Columns are lost to the reader." },
        { t: "Publish it as a table", w: "Levels and expansion are lost to the reader." } ] },
    { id: "A-R4", q: "A dialog role?",
      ctx: "FontDialog, TaskDialog and owned dialogs publish as panes or groups, so a reader does not announce “dialog” when one opens.",
      opts: [
        { t: "Admit dialog, with a modal flag", w: "Readers announce the dialog and its title on opening, and keep the reading inside a modal one.", rec: true },
        { t: "Keep pane and group", w: "No new atom; the opening is announced only as a focus change." } ] },
    { id: "A-R5", q: "A status bar role?",
      ctx: "StatusStrip publishes as a group.",
      opts: [
        { t: "Admit status_bar", w: "Readers have a key to read the status bar on demand (NVDA+End, for example); that needs the role.", rec: true },
        { t: "Keep group", w: "No new atom; the status text is found only by moving through the window." } ] },
    { id: "A-R6", q: "The docking navigator and the token field?",
      ctx: "The T1 recheck handed both to this stage. DockNavigator publishes as a group of two lists; TokenBox's tokens as list items beside its text box. No host has a navigator role or a token role.",
      opts: [
        { t: "No new roles: keep the compositions, with clear names", w: "Every host already reads groups of lists and lists of items well; a new role would map to nothing on most hosts.", rec: true },
        { t: "Admit navigator and token roles anyway", w: "Exact in our own tree, but each host would fall back to a generic role." } ] } ] },
  { key: "B", title: "Fields and states", blurb: "Smaller atoms the workers found missing while checking every control on Windows, Linux and macOS.", questions: [
    { id: "A-F1", q: "Checkable controls?",
      ctx: "A toggle button that is off looks like a plain button: nothing says it can be checked. UI Automation can therefore offer its Toggle pattern only while the button is on, and readers don't announce “not pressed”.",
      opts: [
        { t: "Admit a checkable state", w: "Any control that can be checked says so, on or off. Every host's toggle and check semantics follow from it.", rec: true },
        { t: "Admit a toggle_button role instead", w: "Covers toggle buttons only; other checkable things (menu items, tool buttons) still need their own handling." },
        { t: "Neither", w: "An off toggle stays indistinguishable from a button." } ] },
    { id: "A-F2", q: "Access keys?",
      ctx: "Labels and menus carry mnemonics (the underlined letter of “&File”), but nodes don't publish them, so MSAA's keyboard shortcut and UIA's AccessKey are always empty.",
      opts: [
        { t: "Admit an access_key field", w: "Readers announce “Alt+F” with the control. Filled from the mnemonic the control already has.", rec: true },
        { t: "No", w: "Readers never announce access keys." } ] },
    { id: "A-F3", q: "Step sizes on ranges?",
      ctx: "Sliders, scroll bars and numeric fields publish value, minimum and maximum, but not their small and large steps, which UIA's RangeValue and readers use for arrow and page keys.",
      opts: [
        { t: "Admit small_step and large_step fields on range nodes", w: "Filled from the controls' own increments.", rec: true },
        { t: "No", w: "Hosts report a default step that may not match the control." } ] },
    { id: "A-F4", q: "Multiple selection?",
      ctx: "Nothing says whether a list, tree or grid allows several selected items, which UIA's Selection pattern and readers report.",
      opts: [
        { t: "Admit a multiselectable state", w: "On lists, trees and grids that allow it.", rec: true },
        { t: "No", w: "Readers assume single selection." } ] },
    { id: "A-F5", q: "The docking pin?",
      ctx: "A pin publishes as a checked check box named “Pinned”. The T1 recheck asked whether it needs its own action.",
      opts: [
        { t: "Keep the check box", w: "Every host reads a named check box correctly; with A-F1's checkable state it is announced exactly.", rec: true },
        { t: "A pin action", w: "A new atom that maps to nothing native on any host." } ] } ] },
  { key: "C", title: "Editable text", blurb: "Stage A's text deliverable: reading a text box by character, word and line, following the caret, and changing the selection, through AT-SPI Text, UIA TextPattern and macOS text attributes. Today a text box publishes only its whole text, as its value, with no caret or selection. The Linux worker stopped here because it needs vocabulary.", questions: [
    { id: "A-T1", q: "How does text selection and layout reach the hosts?",
      ctx: "The text library already has typed offsets and grapheme segmentation, and TextBox already keeps its selection on grapheme boundaries.",
      opts: [
        { t: "Two node fields plus two on-demand window queries", w: "Fields: text_selection (anchor and caret as byte offsets into the value; absent for password boxes) and multiline. Queries: the visual lines, character positions and word starts, built only when a host asks; and setting the selection, refused when stale, off a character boundary, or outside the focus scope. Two new field atoms, no new action. Every host converts to its own units.", rec: true },
        { t: "The same, but setting the selection is a semantic action", w: "It reuses the existing action path, but the two offsets have to be packed into a string value. Adds one action atom as well." },
        { t: "No node fields: everything through window queries", w: "Avoids touching the field list, but hosts must poll every text node on each update to notice changes; the same capability under another name." } ] },
    { id: "A-T2", q: "Accept the text defaults?",
      ctx: "Offsets are kept in UTF-8 in the core and converted per host. Words follow the text box's own Ctrl+Arrow rule, so a reader's words match the keyboard's. Lines are visual (wrapped) lines. Sentences are reported unsupported, since nothing segments them yet. A caret placed inside a combining sequence is refused rather than moved. Password boxes publish no text or layout. This pass covers TextBox (single and multiline), MaskedTextBox and TokenBox's editor; the hidden editors of ComboBox, DomainUpDown and NumericUpDown come later.",
      opts: [
        { t: "Accept all of them", w: "The Linux worker builds AT-SPI Text on them; UIA TextPattern and macOS follow on the same core.", rec: true },
        { t: "Accept, with changes in the note", w: "Name which default changes, for example snapping a caret inside a combining sequence instead of refusing it." } ] } ] },
  { key: "M", title: "macOS", blurb: "Your M4 session wrote the macOS stage A drafts: #194 (green) and #195 (one mismatch: a timer label read at two moments). Both also need the shared runtime-ID routing.", questions: [
    { id: "A-M1", q: "Who finishes #194 and #195?",
      ctx: "Only the M4 can run the permission-granted outside client test, which is the gate evidence on macOS.",
      opts: [
        { t: "A worker here, through CI; the M4 runs only the final gate", w: "Keeps all three hosts under one review and one routing core; the M4 is needed once, at the end.", rec: true },
        { t: "The M4 session continues them", w: "You relay a short handoff covering the routing change and the #195 fix." } ] } ] }
];
