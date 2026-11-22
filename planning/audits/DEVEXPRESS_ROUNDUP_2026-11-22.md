# DevExpress roundup (week plan item 3.2, DevExpress half)

Date: 2026-11-22; revision 2 the same day, after the owner's read. Owner's words
(2026-11-22): "for devexpress, they have more controls and some nice demos and
tools i would like to have an equivalent of." Telerik's missing piece, a
converter integrated into MCP, is now MCP study stage G7 and is not repeated
here.

Revision 2 follows the owner's pass on the first draft: "takes, but - not
all. print preview- no. we use the native dialogue. a scheduler? in what
sense? is this a demo app, is this controllers? we need to break down these
takes. some of them are just saying, hey, let's see what we'd need to
implement this in a demo gallery, so that people *could* have this. some
genuinely matter." Every take now says what kind of thing it is, which tier it
belongs to, and what it is from the user's side. Print preview, reporting and
the PDF/XLSX export that goes with them are now leave: GUI.Forms uses the
native print dialog.

**Clean-room rule.** Everything below comes from DevExpress's public
documentation (docs.devexpress.com), product pages (devexpress.com) and demo
listings. No DevExpress code or binaries were read, downloaded, decompiled or
installed. Building any take must follow the same rule: copy behaviour and
appearance from the public demos and documentation only.

**What it was mapped against:** the GUI.Forms public headers at cp36
(`include/gui_forms/controls`, `components`, `plots`, `docking`,
`application`), the Gallery catalogue (`demo/gallery_catalog.inc`, about 170
pages), the demonstrators (`chat_stress`, `node_graph`, the ribbon
demonstrator), the file manager demoboard, Plan Paint,
`planning/CONTROL_COMPLETENESS_MATRIX.md`, `planning/WINFORMS_API_CATALOGUE.md`
and `docs/controls/generated/`. Our tools: the form compiler, `theme_check`,
the theme editor and the WYSIWYG editor (malkuth-tools), the WinForms
reflector, `tools/winforms_catalogue`, `tools/compat_capture`, the help book,
the visual inspector and the stock icon generator.

## How to read the tables

**Mark** (the tier, for a take):

- **1**: genuinely matters for 1.0. Application authors will expect it in the
  library.
- **2**: a Gallery demo, or a demo application built from parts we have. It
  shows people *could* build it with GUI.Forms; building it notes any small
  library gaps it reveals.
- **3**: after 1.0, a programme of its own.
- **have**: we already have it (our name in the Ours column).
- **leave**: not worth an equivalent (reason in the What column).

**Kind** (for a take):

- **control**: a new library control or component in GUI.Forms' public API;
- **feature**: a feature added to a control we already have;
- **Gallery demo**: a composition of existing parts on a Gallery page, with
  no new library API;
- **demo app**: a demo application, like the file manager demoboard;
- **tool**: a design-time or productivity tool (malkuth-tools or `tools/`).

**What** is one plain sentence saying what the take is from the user's side.
"Partial" in the Ours column means we have the base control but not the
feature named in that row.

Links point to the public page that shows the item best: a product page
(screenshots and videos), the docs topic, or the sample-apps page. The Demo
Center demos (`dxdemo://` links) need DevExpress installed, which the
clean-room rule forbids, so their public listing pages are linked instead.

## 1. WinForms controls and components

### 1.1 Data Grid (GridControl) and its views

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Data Grid, Grid View (table) | have | | | DataGridView, VirtualRowsView | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Group-by panel, grouping and group rows | 1 | feature | Drag a column header to a strip above the grid and the rows fold into collapsible groups by that column's values. | partial: ListView groups only | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Total and group summaries | 1 | feature | A footer under the grid, and under each group, shows the sum, count, average, minimum or maximum of a column. | none | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Find panel | 1 | feature | A search box over the grid that filters rows to those containing the text in any column, with matches highlighted. | SearchField, not wired to grids | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Auto-filter row and Excel-style column filters | 1 | feature | Each column gets a filter cell under its header and a drop-down listing its distinct values to tick, as in Excel. | partial: filtering through ObjectView | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Column chooser | 1 | feature | A small window listing hidden columns, which the user drags back into the grid or hides again. | none | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Any editor in any cell (repository items) | 1 | feature | A grid cell can edit with any of our editors (date picker, lookup, colour, toggle, spin), not only text, check and combo. | partial: DataGridView cell types | [Editors](https://docs.devexpress.com/WindowsForms/114580/controls-and-libraries/editors-and-simple-controls) |
| Grid layout save and restore | 1 | feature | The grid's column order, widths, sort, grouping and filters save to a file and come back on the next start. | partial: workspace transactions | [Workspace Manager](https://docs.devexpress.com/WindowsForms/17674/controls-and-libraries/form-layout-managers/workspace-manager) |
| Conditional formatting | 3 | feature | Cells colour themselves by rule: data bars, colour scales, icon sets, or "value above 100 in red". | none | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Banded and advanced banded views | 3 | feature | Column headers grouped under shared band headers, optionally with each record spread over several rows. | none | [Data Grid](https://docs.devexpress.com/WindowsForms/3455/controls-and-libraries/data-grid) |
| Card View, Layout View, Tile View | 2 | Gallery demo | Records shown as a scrolling wall of cards or tiles, each laid out like a small form, instead of table rows. | Card, VirtualRowsView, FlexPanel | [Sample apps: Contacts](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Office Compact View | 2 | Gallery demo | An Outlook-style task list where each record is a two-line row with a flag, subject and due date. | VirtualRowsView, Card | [Sample apps: Tasks](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| WinExplorer View | have | | | ListView (large and small icons, tiles) | [Sample apps](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Cell merging | 3 | feature | Adjacent cells with equal values in a column merge into one tall cell. | none | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| HTML and CSS row templates | 3 | feature | Each grid row is drawn from a markup template with shadows, rounded corners and buttons, instead of cells. | form documents (controls, not grid rows) | [HTML & CSS](https://www.devexpress.com/winforms/html-css/) |
| Master-detail (nested detail grids) | have | | | MasterDetailView | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Frozen columns and rows | have | | | DataGridView | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Server mode and Instant Feedback | have | | | VirtualRowsView, virtual mode | [Large sources](https://docs.devexpress.com/WindowsForms/8398/controls-and-libraries/data-grid/data-binding/large-data-sources-server-and-instant-feedback-modes) |
| Drag-and-drop between grids and lists | have | | | drag | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Export of grids to XLSX, CSV and PDF | leave | | Leave: goes with print preview and reporting; GUI.Forms prints through the native print dialog. | | [Printing](https://www.devexpress.com/products/net/controls/winforms/printing/) |

### 1.2 Other grids and data tools

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Tree List | have | | | TreeListView | [Tree List](https://www.devexpress.com/products/net/controls/winforms/tree_list/) |
| Tree List filtering, summaries and per-node checkboxes | 1 | feature | The tree grid gets the same find, filter and summary features as the flat grid, and tick boxes on nodes that check their children. | partial | [Tree List](https://www.devexpress.com/products/net/controls/winforms/tree_list/) |
| Property Grid | have | | | PropertyGrid, PropertyList, ObjectView | [Property Grid](https://docs.devexpress.com/WindowsForms/119885/controls-and-libraries/property-grid) |
| Vertical Grid | 2 | Gallery demo | One record shown as a two-column table of field names and values, or several records side by side as columns. | PropertyGrid, DataGridView | [Vertical Grid](https://www.devexpress.com/products/net/controls/winforms/vertical_grid/) |
| Gantt Control | 3 | control | A project plan: a task list on the left and a timeline on the right with bars you drag, dependency arrows and a critical path. | none | [Gantt](https://www.devexpress.com/winforms/gantt/) |
| Pivot Grid | 3 | control | An Excel pivot table: drag fields into rows, columns and values to summarise a data set, then drill into any cell. | none | [Pivot Grid](https://www.devexpress.com/products/net/controls/winforms/pivot_grid/) |
| Filter Editor | 3 | control | A dialog where the user builds a filter as a tree of conditions ("Country is Germany and Amount above 100"). | none | [FilterEditorControl](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.FilterEditorControl) |
| Expression Editor | 3 | control | A dialog for writing a calculated column's formula, with field and function lists. | none | [Editors](https://docs.devexpress.com/WindowsForms/114580/controls-and-libraries/editors-and-simple-controls) |

### 1.3 Data editors

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| TextEdit (masks, null text) | have | | | TextBox, MaskedTextBox | [TextEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.TextEdit) |
| ButtonEdit | 1 | control | A text box with buttons of your choice inside its right edge ("...", clear, open). | partial: SearchField | [ButtonEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.ButtonEdit) |
| CheckedComboBoxEdit | 1 | control | A drop-down list with a tick box per item; the closed box shows the ticked items separated by commas. | none | [CheckedComboBoxEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.CheckedComboBoxEdit) |
| PopupContainerEdit | 1 | control | A combo-style box whose drop-down holds any panel you give it. | partial: DropDownButton, AnchoredOverlayLayer | [Popup Container](https://docs.devexpress.com/WindowsForms/612/controls-and-libraries/editors-and-simple-controls/popup-container-editor) |
| MemoEdit | have | | | TextBox (multiline), ContentTextEditor | [Text inputs](https://www.devexpress.com/products/net/controls/winforms/editors/text-inputs.xml) |
| MemoExEdit | 2 | Gallery demo | A one-line box that drops down a larger multi-line editor. | TextBox, AnchoredOverlayLayer | [Text inputs](https://www.devexpress.com/products/net/controls/winforms/editors/text-inputs.xml) |
| SpinEdit | have | | | NumericUpDown | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| CalcEdit | 2 | Gallery demo | A number box that drops down a pocket calculator. | NumericUpDown, GridPanel, Button | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| TrackBarControl | have | | | TrackBar | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ZoomTrackBarControl | 2 | Gallery demo | A slider with minus and plus buttons at the ends, as on a status-bar zoom. | TrackBar, Button | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| RangeTrackBarControl | have | | | RangeControl | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| SparklineEdit | have | | | Sparkline | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ProgressBarControl, MarqueeProgressBarControl | have | | | ProgressBar, ProgressRing, Spinner | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| CheckEdit | have | | | CheckBox | [CheckEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.CheckEdit) |
| ToggleSwitch | have | | | ToggleSwitch | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| DateEdit, TimeEdit | have | | | DateTimePicker | [Date editors](https://www.devexpress.com/products/net/controls/winforms/editors/date-time-picker-and-calendar.xml) |
| TimeSpanEdit | 2 | Gallery demo | A box for durations ("2 d 03:15") with spin buttons per part. | MaskedTextBox, NumericUpDown | [Date editors](https://www.devexpress.com/products/net/controls/winforms/editors/date-time-picker-and-calendar.xml) |
| CalendarControl | have | | | MonthCalendar | [Date editors](https://www.devexpress.com/products/net/controls/winforms/editors/date-time-picker-and-calendar.xml) |
| Date range picker | 2 | Gallery demo | Two dates picked on side-by-side calendars, with presets such as "last 7 days". | MonthCalendar, DateTimePicker | [Blazor Date Range Picker](https://docs.devexpress.com/Blazor/405111/components/data-editors/daterangepicker) |
| PictureEdit | have | | | PictureBox | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ImageEdit | leave | | Leave: a picture in a drop-down; PictureBox in a popup covers it. | | |
| ImageComboBoxEdit | have | | | ComboBox (owner-drawn items) | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ColorEdit, ColorPickEdit | have | | | ColorPicker, ColorValueEditor, RibbonColorPicker | [ColorEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.ColorEdit) |
| ComboBoxEdit | have | | | ComboBox | [ComboBoxEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.ComboBoxEdit) |
| FontEdit | have | | | RibbonFontControl, FontDialog | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| MRUEdit | have | | | ComboBox with AutoComplete | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| PopupGalleryEdit | have | | | RibbonGallery (drop-down) | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| RadioGroup | have | | | ChoiceGroup | [RadioGroup](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.RadioGroup) |
| TokenEdit | have | | | TokenBox | [Token Edit](https://docs.devexpress.com/WindowsForms/17088/controls-and-libraries/editors-and-simple-controls/token-edit-control) |
| LookUpEdit | have | | | LookupComboBox | [Data lookups](https://www.devexpress.com/products/net/controls/winforms/editors/data-lookups.xml) |
| GridLookUpEdit, SearchLookUpEdit | 2 | Gallery demo | A lookup box whose drop-down is a full grid with sorting and a search box. | LookupComboBox, DataGridView, PopupContainerEdit once taken | [Data lookups](https://www.devexpress.com/products/net/controls/winforms/editors/data-lookups.xml) |
| TreeListLookUpEdit | 2 | Gallery demo | A lookup box whose drop-down is a tree. | TreeListView in a drop-down | [Data lookups](https://www.devexpress.com/products/net/controls/winforms/editors/data-lookups.xml) |
| RatingControl | 2 | Gallery demo | A row of stars (or other icons) the user clicks to give a rating. | ChoiceGroup, stock icons | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| HyperLinkEdit | have | | | LinkLabel | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| BreadCrumbEdit | have | | | BreadcrumbTrail | [Breadcrumb](https://docs.devexpress.com/WindowsForms/16973/controls-and-libraries/editors-and-simple-controls/breadcrumb-edit-control) |
| Rich text in a grid cell | 3 | feature | A grid cell that shows and edits formatted text. | RichTextBox, not as a cell editor | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| Masked input | have | | | MaskedTextBox | [Blazor Masked Input](https://docs.devexpress.com/Blazor/405118/components/data-editors/maskedinput) |

### 1.4 Simple controls

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| SimpleButton, CheckButton, DropDownButton | have | | | Button, CheckBox (button look), DropDownButton | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| LabelControl, HyperlinkLabelControl | have | | | Label, TextBlock, LinkLabel | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ListBoxControl, ImageListBoxControl, CheckedListBoxControl | have | | | ListBox, CheckedListBox | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| Templated list box items | 3 | feature | List items drawn from a small layout template (picture, title, subtitle, badge) instead of one line of text. | none | [Templated items](https://docs.devexpress.com/WindowsForms/119680/controls-and-libraries/editors-and-simple-controls/templated-listbox-items) |
| ControlNavigator, DataNavigator | have | | | BindingNavigator | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| BarCodeControl | 3 | control | Shows a value as a barcode or QR code that scanners and phones can read. | none | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ImageSlider | 2 | Gallery demo | One picture at a time with previous and next arrows and a slide animation. | PictureBox, animation | [Image Slider](https://docs.devexpress.com/WindowsForms/12364/controls-and-libraries/editors-and-simple-controls/image-slider) |
| ProgressPanel | have | | | Spinner, EmptyState | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| RangeControl | have | | | RangeControl | [Range Control](https://docs.devexpress.com/WindowsForms/11739/controls-and-libraries/editors-and-simple-controls/range-control) |
| GalleryControl | 2 | Gallery demo | A scrolling grid of thumbnails in named groups, as in a photo browser. | ListView tiles, RibbonGallery | [Navigation](https://www.devexpress.com/products/net/controls/winforms/navigation/) |
| CameraControl | 3 | control | Shows live video from a webcam and takes snapshots. | none | [Camera](https://docs.devexpress.com/WindowsForms/114582/controls-and-libraries/editors-and-simple-controls/camera-control) |
| StepProgressBar | have | | | StepIndicator | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| SearchControl | have | | | SearchField | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| GroupControl, PanelControl, RoundedSkinPanel | have | | | GroupBox, Panel, MaterialPanel, Card | [Layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| Scroll bars, XtraScrollableControl | have | | | HScrollBar, VScrollBar, ScrollableControl | [Editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| SplitterControl, SplitContainerControl, SidePanel | have | | | Splitter, SplitContainer, DockSite | [Layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| XtraTabControl | have | | | TabControl, PageView, PageStrip | [Tab Control](https://docs.devexpress.com/WindowsForms/114576/controls-and-libraries/form-layout-managers/tab-control) |
| HTML Content Control and Popup | have | | | form documents rendered as controls | [HtmlContentControl](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.HtmlContentControl) |
| Code editor (Monaco in a web view, an example) | 3 | control | A source-code editor with syntax colouring, line numbers, folding and completion. | ContentTextEditor; SwiftEdit's CodeEditor (malkuth-tools) | [Example README](https://github.com/DevExpress-Examples/winforms-monaco-code-editor) |

### 1.5 Components

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Image collections (raster, SVG, DPI-aware) | have | | | ImageList, stock icons | [Included](https://docs.devexpress.com/WindowsForms/401381/controls-and-libraries/editors-and-simple-controls/included-controls-and-components) |
| ToolTipController | have | | | ToolTip, RibbonToolTip | same |
| AlertControl | have | | | Toast, NotifyIcon | [Alert Windows](https://docs.devexpress.com/WindowsForms/5487/controls-and-libraries/messages-notifications-and-dialogs/alert-windows) |
| ToastNotificationsManager | 1 | control | Sends a notification to the operating system's notification centre, which stays there after the app is hidden. | Toast (in-window), NotifyIcon balloons | [Toast Manager](https://docs.devexpress.com/WindowsForms/17020/controls-and-libraries/messages-notifications-and-dialogs/toast-notification-manager) |
| DXErrorProvider, DXValidationProvider | have | | | ErrorProvider, validation | same |
| SplashScreenManager (splash, wait and overlay forms) | have | | | application splashscreen, ModalLayer | [Splash](https://www.devexpress.com/winforms/splash-screen/) |
| StyleController | leave | | Leave: deprecated by DevExpress; our CSS covers it. | | |
| TransitionManager | 2 | Gallery demo | A control changes content with an animated slide, fade or zoom instead of an instant swap. | animation | [Included](https://docs.devexpress.com/WindowsForms/401381/controls-and-libraries/editors-and-simple-controls/included-controls-and-components) |
| WorkspaceManager | 2 | Gallery demo | Saves the whole window's arrangement (docked panels, grid layouts) under a name and switches between saved arrangements. | dock layout, workspace transactions | [Workspace Manager](https://docs.devexpress.com/WindowsForms/17674/controls-and-libraries/form-layout-managers/workspace-manager) |
| PersistentRepository | leave | | Leave: a designer artefact of their repository-item model. | | |
| TaskbarAssistant | 3 | control | Puts progress, overlay icons, a jump list and thumbnail buttons on the app's taskbar or dock icon. | partial: host taskbar progress | [Taskbar Assistant](https://docs.devexpress.com/WindowsForms/114579/controls-and-libraries/forms-and-user-controls/taskbar-assistant) |
| Adorner UI Manager (badges over controls) | have | | | Badge, Callout | [Adorner](https://docs.devexpress.com/WindowsForms/114584/controls-and-libraries/forms-and-user-controls/adorner-ui-manager) |
| Adorner guides (guided tour) | 2 | Gallery demo | A first-run tour that dims the window and spotlights one control at a time with a note. | ModalLayer, Callout | same |
| Behavior Manager, drag-and-drop behaviours | have | | | drag | [Grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |

### 1.6 Navigation

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Accordion Control | 2 | Gallery demo | A side menu of collapsible groups and items, with a search box, that can shrink to icons. | ExpandableSections, NavigationPane | [Accordion](https://docs.devexpress.com/WindowsForms/114553/controls-and-libraries/navigation-controls/accordion-control) |
| Hamburger menu style | have | | | NavigationPane (compact) | [Hamburger](https://docs.devexpress.com/WindowsForms/120498/controls-and-libraries/navigation-controls/accordion-control/hamburger-menu-view-style) |
| Office Navigation Bar | 2 | Gallery demo | Outlook's bottom strip (Mail, Calendar, People, Tasks) that previews a module when hovered. | PageStrip, Callout | [Office Navigation Bar](https://docs.devexpress.com/WindowsForms/114558/controls-and-libraries/navigation-controls/office-navigation-bar) |
| Navigation Frame, Tab Pane | have | | | PageView, PageStrip | [Navigation Frame](https://docs.devexpress.com/WindowsForms/114554/controls-and-libraries/navigation-controls/navigation-frame-and-tab-pane) |
| Navigation Pane, Navigation Bar | have | | | NavigationPane, ExpandableSections | [Navigation Pane](https://docs.devexpress.com/WindowsForms/114555/controls-and-libraries/navigation-controls/navigation-pane) |
| Tile Bar | 2 | Gallery demo | A horizontal strip of large coloured tiles across the top of a window, each opening a section or a drop-down. | Card, FlexPanel | [Tile Bar](https://docs.devexpress.com/WindowsForms/114556/controls-and-libraries/navigation-controls/tile-bar) |
| Tile Navigation Pane | 2 | Gallery demo | A top navigation bar whose categories open full-width bands of tiles. | Card, FlexPanel | [TileNav Pane](https://docs.devexpress.com/WindowsForms/114557/controls-and-libraries/navigation-controls/tilenav-pane) |
| Tile Control | 3 | control | A Windows-Start-style wall of live tiles in groups that users drag to reorder and resize. | none | [Tiles](https://www.devexpress.com/products/net/controls/winforms/tile/) |
| Toolbox Control | 3 | control | A searchable palette of categorised items to drag onto a surface, as in a designer's toolbox. | none (the WYSIWYG editor will need one) | [ToolboxControl](https://docs.devexpress.com/WindowsForms/DevExpress.XtraToolbox.ToolboxControl) |
| WindowsUI Button Panel | have | | | CommandBar | [Button Panel](https://docs.devexpress.com/WindowsForms/114583/controls-and-libraries/navigation-controls/windowsui-button-panel) |
| Wizard Control | have | | | Wizard, WizardPage | [Wizard](https://docs.devexpress.com/WindowsForms/4735/controls-and-libraries/form-layout-managers/wizard-control) |

### 1.7 Ribbon, bars and menus

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Ribbon, contextual tabs, galleries, Backstage, Quick Access Toolbar, status bar | have | | | Ribbon, RibbonGallery, BackstageView, QuickAccessToolbar, StatusStrip | [Ribbon](https://www.devexpress.com/winforms/ribbon/) |
| Ribbon and bar merging for MDI children | 1 | feature | When a document window is active, its own ribbon tabs and toolbar buttons join the main window's ribbon and go away when it closes. | partial: MDI follow-ups in cp36 | [Ribbon merging](https://docs.devexpress.com/WindowsForms/3451/controls-and-libraries/ribbon-bars-and-menu/ribbon/runtime-capabilities/ribbon-merging) |
| Simplified (single-line) ribbon | 3 | feature | A one-row ribbon, as in the newer Office, that the user switches to from the classic three-row one. | none | [Ribbon styles](https://docs.devexpress.com/WindowsForms/14624/controls-and-libraries/ribbon-bars-and-menu/ribbon/ribbon-styles) |
| Mini Toolbar | 2 | Gallery demo | A small formatting toolbar that fades in beside selected text. | CommandBar, AnchoredOverlayLayer | [RibbonMiniToolbar](https://docs.devexpress.com/WindowsForms/DevExpress.XtraBars.Ribbon.RibbonMiniToolbar) |
| Search menu ("Tell me") | 2 | Gallery demo | A box in the ribbon where the user types a command's name and runs it from the results. | SearchField, commands | [Search menu](https://docs.devexpress.com/WindowsForms/400621/controls-and-libraries/ribbon-bars-and-menu/ribbon/visual-elements/search-menu) |
| Ribbon runtime customisation | 3 | feature | Users add their own tabs and groups and move commands into them, and the choice is saved. | none | [Customisation](https://docs.devexpress.com/WindowsForms/11528/controls-and-libraries/ribbon-bars-and-menu/ribbon/runtime-capabilities/runtime-customization) |
| Bars, popup menus, side-strip toolbars, touch mode | have | | | ToolStrip, ToolStripContainer, ToolStripPanel, MenuStrip, ContextMenu, CommandBar | [Bars](https://www.devexpress.com/products/net/controls/winforms/bars/) |
| Toolbar customisation dialog | 3 | feature | Users drag buttons on and off toolbars and create new toolbars from a dialog. | none | [Bars](https://docs.devexpress.com/WindowsForms/5361/controls-and-libraries/ribbon-bars-and-menu/bars) |
| Radial Menu | 2 | Gallery demo | A round pop-up menu with commands around a centre button, for pen and touch. | DrawingSurface, commands | [Radial Menu](https://docs.devexpress.com/WindowsForms/14628/controls-and-libraries/ribbon-bars-and-menu/radial-menu) |

### 1.8 Forms

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| XtraForm, DirectX Form, Ribbon Form, Toolbar Form | have | | | Window with custom chrome, form documents | [XtraForm](https://docs.devexpress.com/WindowsForms/114560/controls-and-libraries/forms-and-user-controls/xtraform) |
| Tabbed Form | 3 | feature | A window whose title bar holds browser-style tabs that drag out into their own windows. | none | [Tabbed Form](https://docs.devexpress.com/WindowsForms/115183/controls-and-libraries/forms-and-user-controls/tabbed-form) |
| Fluent Design Form | have | | | window backdrop (acrylic, mica), NavigationPane | [Fluent Form](https://docs.devexpress.com/WindowsForms/120214/controls-and-libraries/forms-and-user-controls/fluent-design-form) |
| XtraUserControl | have | | | UserControl | [XtraUserControl](https://docs.devexpress.com/WindowsForms/114564/controls-and-libraries/forms-and-user-controls/xtrausercontrol) |
| Form Assistant | leave | | Leave: converts a WinForms Form type at design time; our forms choose their chrome by property. | | |

### 1.9 Messages, notifications and dialogs

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| XtraMessageBox, XtraDialog, Flyout Dialog | have | | | MessageBox, TaskDialog, ModalLayer | [Message box](https://docs.devexpress.com/WindowsForms/114566/controls-and-libraries/messages-notifications-and-dialogs/xtramessagebox) |
| XtraInputBox | 1 | control | A one-line "ask the user for a value" dialog: a prompt, one editor (text, number, date) and OK and Cancel. | none | [Input Box](https://docs.devexpress.com/WindowsForms/117729/controls-and-libraries/messages-notifications-and-dialogs/input-box) |
| File and folder dialogs | have | | | OpenFileDialog, SaveFileDialog, FolderBrowserDialog | [File dialogs](https://docs.devexpress.com/WindowsForms/403445/controls-and-libraries/messages-notifications-and-dialogs/file-and-folder-browsers) |
| File Explorer Assistant | have | | | file manager demoboard | [Folder browsers](https://docs.devexpress.com/WindowsForms/403431/controls-and-libraries/messages-notifications-and-dialogs/custom-folder-browsers) |
| Flyout Panel | have | | | Callout, AnchoredOverlayLayer | [Flyout Panel](https://docs.devexpress.com/WindowsForms/114578/controls-and-libraries/messages-notifications-and-dialogs/flyout-panel) |
| Templates for message boxes and alerts | have | | | form documents | [Templates](https://docs.devexpress.com/WindowsForms/404058/controls-and-libraries/messages-notifications-and-dialogs/html-and-css-templates-in-messages-and-dialogs) |

### 1.10 Layout, docking and the application UI manager

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Layout Control, flow layout, stack and table panels | have | | | FormLayout, GridPanel, FlexPanel, FlowLayoutPanelCompat, TableLayoutPanelCompat | [Layout Control](https://docs.devexpress.com/WindowsForms/3407/controls-and-libraries/form-layout-managers/layout-and-data-layout-controls/layout-control/layout-control) |
| Layout runtime customisation | 3 | feature | Users drag fields around a finished form, hide some and save their arrangement. | none | [Layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| Data Layout Control | 3 | feature | Point a form at a data object and it lays out a labelled editor for every field. | partial: `<bind>` elements in form documents | [Data Layout](https://docs.devexpress.com/WindowsForms/4921/controls-and-libraries/form-layout-managers/layout-and-data-layout-controls/data-layout-control) |
| Responsive breakpoints | have | | | ResponsiveTrackPanel | [Breakpoint](https://docs.devexpress.com/Blazor/405072/components/layout/layout-breakpoint) |
| Docking, Tabbed MDI, Tabbed View, Native MDI View, document selector, deferred load | have | | | DockSite, DockWorkspace, DockNavigator | [Docking](https://www.devexpress.com/products/net/controls/winforms/docking/) |
| WindowsUI View | 2 | Gallery demo | A full-window tablet-style app: a start screen of tiles, pages that slide in, and flyouts. | PageView, Card, ModalLayer | [WindowsUI View](https://docs.devexpress.com/WindowsForms/11854/controls-and-libraries/application-ui-manager/views/windowsui-view) |
| Widget View | 2 | Gallery demo | A dashboard of resizable cards (widgets) that the user rearranges and maximises one at a time. | Card, GridPanel, DockWorkspace | [Widget View](https://docs.devexpress.com/WindowsForms/16239/controls-and-libraries/application-ui-manager/views/widget-view) |

### 1.11 Data visualisation

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Charts: line, spline, step, area, bar, scatter; crosshair, zoom, scroll | have | | | PlotView | [Charts](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Charts: pie and doughnut | 1 | feature | Round charts showing each value as a slice of a whole, with labels and a legend. | none | [Charts](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Charts: funnel, pyramid, nested doughnut | 3 | feature | Stage-by-stage shrinking shapes for sales funnels, and rings within rings. | none | [Charts](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Charts: financial (candlestick, OHLC, indicators) | 3 | feature | Stock price charts with open, high, low and close per period, moving averages and trend lines. | none | [Sample apps: Finance](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Charts: range bar, range area, bubble, box plot, error bars | 3 | feature | Charts that show spreads and uncertainty rather than single values. | partial: histogram | [Charts](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Charts: polar and radar | 3 | feature | Values plotted around a circle, as in a wind rose or a skills spider chart. | none | [Blazor Polar Chart](https://docs.devexpress.com/Blazor/401769/components/charts/get-started-with-charts) |
| Charts: 3D | have | | | Plot3DView | [Charts](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Chart Designer | 3 | tool | A dialog where the end user changes a chart's type, series, colours and labels without code. | none | [Chart Control](https://docs.devexpress.com/WindowsForms/8117/controls-and-libraries/chart-control) |
| Gauges: circular, linear, digital, state indicator | have | | | RadialGauge, LinearGauge, Dial, SevenSegmentDisplay, IndicatorLamp | [Gauges](https://www.devexpress.com/products/net/controls/winforms/gauges/) |
| Gauge Designer | 3 | tool | A dialog with ready gauge presets and an editor for scales, needles and ranges. | none | [Gauges](https://docs.devexpress.com/WindowsForms/18218/controls-and-libraries/gauges) |
| Diagram Control | 3 | control | A Visio-like canvas: drag shapes from a library, connect them with arrows, and let it lay out flowcharts and org charts. | NodeCanvas, node graph (nodes and links, no shapes library) | [Diagrams](https://www.devexpress.com/products/net/controls/winforms/diagrams/) |
| Map Control | 3 | control | An interactive map with online tiles, markers, routes and shaded regions. | none | [Map](https://www.devexpress.com/products/net/controls/winforms/map/) |
| TreeMap | 2 | Gallery demo | Nested rectangles sized by value, as in a disk-usage view. | DrawingSurface | [TreeMap](https://www.devexpress.com/products/net/controls/winforms/treemap/) |
| Sunburst | 2 | Gallery demo | A hierarchy drawn as rings of slices around a centre. | DrawingSurface | [Sunburst](https://www.devexpress.com/products/net/controls/winforms/sunburst/) |
| Sankey Diagram | 3 | control | Flows between stages drawn as bands whose width is the amount. | none | [Sankey](https://www.devexpress.com/winforms/sankey-diagram/) |
| Heatmap | have | | | PlotHeatmapSeries | [HeatMap](https://www.devexpress.com/winforms/heatmap/) |
| Range selector over a chart | have | | | RangeControl, SweepScope | [Range Selector](https://docs.devexpress.com/Blazor/405266/components/range-selector) |

### 1.12 Office-inspired

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Rich Text Editor (Word-level) | 3 | control | A word processor in a control: pages, tables, styles, headers and footers, images, DOCX. | RichTextBox (WinForms parity), ContentTextEditor | [Rich Editor](https://www.devexpress.com/products/net/controls/winforms/rich_editor/) |
| Spreadsheet | 3 | control | Excel in a control: sheets, formulas, formatting, charts in cells, XLSX. | none | [Spreadsheet](https://www.devexpress.com/products/net/controls/winforms/spreadsheet/) |
| Scheduler | 3 | control | An Outlook-style calendar control with day, week, month and timeline views, appointments you drag and resize, and repeating events. | none | [Scheduler](https://www.devexpress.com/products/net/controls/winforms/scheduler/) |
| Scheduler, Gallery version | 2 | Gallery demo | A week-view calendar page built from our grid panel and cards, to show the shape of a scheduler before the control exists. | GridPanel, Card, drag | [Sample apps: Calendar](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Spell Checker | 3 | component | Red squiggles under misspelt words in text boxes as you type, with suggestions on right-click. | none | [Spell Checker](https://www.devexpress.com/products/net/controls/winforms/spell_checker/) |
| Office File API | leave | | Leave: a document-processing library, not a toolkit. | | |

### 1.13 Printing, reporting and documents

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Printing and exporting (print preview, export of any control) | leave | | Leave: GUI.Forms uses the native print dialog (owner, 2026-11-22). | help printing | [Printing](https://www.devexpress.com/products/net/controls/winforms/printing/) |
| Reporting: Report Viewer and banded reports | leave | | Leave: built on print preview and export, which are left. | | [Reporting](https://www.devexpress.com/subscriptions/reporting/) |
| BI Dashboard | leave | | Leave: a separate BI product. | | |
| PDF Viewer | 3 | control | Opens a PDF inside the app to read, search, zoom, and fill in its form fields. | none | [PDF Viewer](https://www.devexpress.com/products/net/controls/winforms/pdf-viewer/) |

### 1.14 Cross-cutting technologies

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Skins, HTML and CSS UI, DirectX rendering, MVVM, accessibility, high DPI, localisation, Coded UI | have | | | themes and surface materials, form documents, our renderer, commands and binding, semantics, scaling, localizable text, MCP automation | [Skins](https://www.devexpress.com/products/net/controls/winforms/skins/) |
| Skin and palette selector galleries | 2 | Gallery demo | A ribbon gallery listing the themes and their colour palettes, so the user switches the app's look live. | RibbonGallery, user themes | [Skins](https://www.devexpress.com/products/net/controls/winforms/skins/) |
| AI Chat control | 2 | Gallery demo | A chat panel with message bubbles, a typing box and streamed replies from a language model. | chat stress demonstrator, VirtualRowsView, TextBox | [AI Chat](https://docs.devexpress.com/WindowsForms/405218/ai-powered-extensions/ai-chat-control) |
| AI extensions (smart paste, autocomplete, search, assistant) | 3 | feature | Editors gain model-backed helpers: paste a text and fields fill in, suggestions as you type, rewrite or translate a selection. | none | [AI extensions](https://docs.devexpress.com/WindowsForms/405151/ai-powered-extensions) |
| Icon library | 3 | tool | Thousands of ready icons in one consistent style, picked by name or from a browser. | stock icon generator | [Icon Library](https://docs.devexpress.com/WindowsForms/16864/devexpress-icon-library/image-gallery-and-context-dependent-images) |

## 2. Notable WPF and Blazor items with no WinForms counterpart

| DevExpress | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| WPF ThemedWindow, LoadingDecorator, Badge | have | | | Window, ModalLayer, Spinner, Badge | [ThemedWindow](https://docs.devexpress.com/WPF/DevExpress.Xpf.Core.ThemedWindow) |
| Blazor Carousel | 2 | Gallery demo | Slides that rotate automatically or by arrows, with dots showing the position. | PageView, animation | [Carousel](https://docs.devexpress.com/Blazor/405183/components/layout/carousel) |
| Blazor Drawer | 2 | Gallery demo | A side panel that slides over or pushes the content, opened by a menu button. | NavigationPane, animation | [Drawer](https://docs.devexpress.com/Blazor/404362/components/layout/drawer) |
| Blazor Pager | 2 | Gallery demo | Page-number buttons (1 2 3 ... 10, next, previous) under a list. | Button, FlexPanel | [Pager](https://docs.devexpress.com/Blazor/405054/components/utility-controls/pager) |
| Blazor Bar Gauge | 2 | Gallery demo | Several values as concentric arcs around a centre. | RadialGauge, DrawingSurface | [Bar Gauge](https://docs.devexpress.com/Blazor/404946/components/bar-gauge) |
| Blazor Button Group, Split Button, Color Palette, Grid Layout | have | | | ChoiceGroup, DropDownButton, ColorPicker, GridPanel | [Button Group](https://docs.devexpress.com/Blazor/405195/components/utility-controls/button-group) |
| Blazor HTML Editor | leave | | Leave: HTML output is a web concern; rich editing is the Rich Text Editor. | | |
| Blazor Upload, File Input | leave | | Leave: web upload widgets; the desktop has file dialogs and drop. | | |

## 3. Demo applications

| DevExpress demo | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| Outlook-Inspired Application (mail, tasks, contacts) | 2 | demo app | An Outlook look-alike with mail, tasks and contacts modules switched from a navigation bar, built from our grid, ribbon and panes. | file manager demoboard and Gallery cover parts | [Demos](https://www.devexpress.com/Support/Demos/) |
| Outlook-Inspired Calendar module, Dental Clinic | 3 | demo app | The calendar half of the Outlook demo and a clinic appointment book; both need the Scheduler control. | none | same |
| Stock Market Trader, Finance and Trading | 3 | demo app | A live trading screen of price charts and ticking grids; needs the financial charts. | chat stress shows the real-time load | same |
| HTML and CSS Templates demo | 2 | demo app | Records drawn as styled cards from markup templates, in an Explorer-style list. | form documents | same |
| Touch-Enabled Hybrid, Touch-First Tablet | 2 | demo app | A tablet-style app with large tiles and touch targets; with the touch work (week plan 3.1). | WindowsUI View Gallery demo once built | same |
| Fluent Mail Client | 2 | demo app | A Windows 11 Mail look-alike with an acrylic window and a collapsing side menu. | window backdrop, NavigationPane | same |
| Microsoft CRM-inspired | 2 | demo app | A sales dashboard of grids and charts; pie and doughnut come with the tier-1 chart feature. | PlotView, DataGridView | [Sample apps](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| CRUD Applications | have | | | Gallery bound values, form documents | same |
| Visio-inspired Applications | 2 | demo app | A flowchart editor; the node graph demonstrator is ours, and the full version waits for the Diagram control. | node graph demonstrator | same |
| Browser-like UI | 3 | demo app | A web-browser look-alike with tabs in the title bar; needs the Tabbed Form. | none | same |
| File Explorer-like UI | have | | | file manager demoboard | same |
| Visual Studio-inspired Docking | have | | | DockWorkspace pages in the Gallery | same |
| Excel-, Word-, Adobe- and GIS-inspired applications, Excel analytics | 3 | demo app | Showcases for the Spreadsheet, Rich Text Editor, PDF Viewer, Map and Pivot Grid; each follows its control. | none | same |
| Reporting demo | leave | | Leave: reporting is left. | | |
| Dashboard demos | leave | | Leave: the BI dashboard is left. | | |
| AI demos (AI rich edit) | 3 | demo app | Editors with AI helpers; follows the AI extensions. | none | [Demos](https://www.devexpress.com/Support/Demos/) |
| XAF Employee Management | leave | | Leave: XAF is an application framework, not a toolkit item. | | |
| Per-control Main Demos | have | | | the Gallery | [Demos](https://www.devexpress.com/Support/Demos/) |
| Demo Center "open the source" button | 2 | feature | Each Gallery page gets a button that shows the code (or form document) that builds it. | the Gallery, without a source button | [Demo apps](https://docs.devexpress.com/WindowsForms/14962/whats-installed/demo-applications) |

## 4. Design-time and productivity tools

| DevExpress tool | Mark | Kind | What | Ours | Page |
|---|---|---|---|---|---|
| WinForms Skin Editor, WPF Theme Designer | have | | | theme editor (malkuth-tools) | [Skin Editor](https://docs.devexpress.com/SkinEditor/1630/winforms-skin-editor) |
| Palette editor | 3 | tool | Edit a theme's named colours and see every control recolour at once. | partial: theme editor, `theme_check` | [Skins](https://www.devexpress.com/products/net/controls/winforms/skins/) |
| Project Converter | leave | | Leave: upgrades their own assembly versions; our version policy does that, and G7 converts WinForms projects. | | |
| Project Settings page | 3 | tool | One page of app-wide defaults (theme, font, DPI behaviour) for a project. | partial: build profile, theme defaults | [Project Settings](https://docs.devexpress.com/WindowsForms/120180/whats-installed/project-settings-page) |
| Template Gallery and Template Kit | 3 | tool | "New project" templates for typical app shapes (Ribbon app, Outlook-style, tabbed documents). | none | [Template Kit](https://docs.devexpress.com/WindowsForms/405275/whats-installed/project-template-kit) |
| WinForms UI Templates | 2 | Gallery demo | Ready screens (sign-in, settings, record editor, dashboard) as form documents people copy into their apps. | form documents, Gallery | [UI Templates](https://www.devexpress.com/ui-templates/winforms/) |
| Instant Layout Assistant, Bar and Ribbon designers, Grid and Document Manager designers | 3 | tool | Design a form, ribbon, toolbar or grid by dragging; all part of the WYSIWYG editor. | WYSIWYG editor (open) | [Bar Manager Designer](https://docs.devexpress.com/WindowsForms/5353/controls-and-libraries/ribbon-bars-and-menu/bars/bar-manager-designer) |
| Layout Converter | 3 | tool | Turns a form of absolutely placed controls into a flowing layout; part of the WinForms converter (G7). | WinForms converter (G7) | [Layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| Image picker | 3 | tool | Browse and insert icons while designing. | stock icon generator | [Icon Library](https://docs.devexpress.com/WindowsForms/16864/devexpress-icon-library/image-gallery-and-context-dependent-images) |
| SVG Icon Builder | have | | | stock icon generator | [Store page](https://www.microsoft.com/en-us/store/p/svg-icon-builder/9mxbbwvknrvr) |
| HTML and CSS Template Editor | 3 | tool | Edit a form document's markup and styles with a live preview beside it. | form compiler; WYSIWYG editor | [HTML & CSS](https://www.devexpress.com/winforms/html-css/) |
| Report designers (IDE and end-user) | leave | | Leave: reporting is left. | | |
| Dashboard designers | leave | | Leave: the BI dashboard is left. | | |
| Data Source Wizard | leave | | Leave: data access belongs to the application; GUI.Forms binds to objects. | | |
| Scaffolding Wizard | 3 | tool | Generates list and edit screens from a data model. | none | [WinForms](https://www.devexpress.com/products/net/controls/winforms/) |
| Online Localization Service and UI Localization Client | 3 | tool | Translators edit the app's strings in a web or in-app editor and see them in place. | localizable text, locale files | [Localization](https://localization.devexpress.com/) |
| Documentation MCP Server | 3 | tool | AI assistants answer questions from GUI.Forms' own documentation; part of the MCP work. | MCP study, help book | [Docs MCP](https://docs.devexpress.com/GeneralInformation/405551/help-resources/dev-express-documentation-mcp-server-configure-an-ai-powered-assistant) |
| F1 help on API | have | | | API reference search, help book | [F1 help](https://docs.devexpress.com/WindowsForms/117844/whats-installed/getting-help-on-api) |
| Visual Studio toolbox integration | leave | | Leave: Visual Studio integration comes after 1.0, with the Windows coordinator. | | |
| CodeRush | leave | | Leave: an IDE product. | | |
| Unified Component Installer | leave | | Leave: GUI.Forms ships as a source SDK. | | |

## 5. Counts

Tier 1: 15. Tier 2: 42. Tier 3: 53. Have: 85. Leave: 21 (counted from the Mark column of sections 1 to 4; a row can group several DevExpress items).

## 6. Shortlist by tier, for the owner's pass

**Tier 1: matters for 1.0** (library work application authors will expect):

1. Grid grouping with group rows and the group-by panel.
2. Grid total and group summaries.
3. Grid find panel.
4. Grid auto-filter row and Excel-style column filters.
5. Any editor in any grid cell.
6. Grid column chooser.
7. Grid layout save and restore.
8. Tree List filtering, summaries and node checkboxes.
9. ButtonEdit, CheckedComboBoxEdit and PopupContainerEdit.
10. Input box.
11. Pie and doughnut charts.
12. Operating-system toast notifications.
13. Ribbon and toolbar merging for MDI children.

**Tier 2: Gallery demos and demo applications** (no new library API; each
notes the small gaps it finds), roughly in order of how much they show:

1. Outlook-Inspired demo application (mail, tasks, contacts).
2. Card, layout and tile views of records; Office compact task list.
3. Accordion, Office navigation bar, tile bar and tile navigation pane.
4. Grid and tree lookups in drop-downs.
5. The Gallery source button.
6. Widget View dashboard and WindowsUI View tablet app, with the touch and
   hybrid demo applications.
7. Scheduler, Gallery version (a week view from our parts).
8. AI chat panel.
9. UI templates (sign-in, settings, record editor, dashboard screens).
10. CRM dashboard, Fluent Mail, HTML-template cards, flowchart editor.
11. Small compositions: Vertical Grid, MemoEx, calculator, zoom slider,
    duration box, date range, rating, image slider, thumbnail gallery,
    transitions, workspaces, guided tour, mini toolbar, command search,
    radial menu, treemap, sunburst, theme gallery, carousel, drawer, pager,
    bar gauge.

**Tier 3: after 1.0, each a programme of its own**, roughly by value:
Scheduler control; Diagram control; Gantt; conditional formatting and banded
views; more chart families (financial, funnel, range, polar) with a chart
designer; Spell Checker; Tabbed Form; PDF Viewer; Map; Pivot Grid;
Spreadsheet; Word-level Rich Text Editor; the tools (template kit, palette
editor, localisation service, scaffolding, the WYSIWYG editor's designers).

**Moved to leave in revision 2:** print preview, export of controls and grids
to PDF, XLSX and CSV, reporting (viewer, designers, demo).
