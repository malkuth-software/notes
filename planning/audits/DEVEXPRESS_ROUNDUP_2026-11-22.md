# DevExpress roundup (week plan item 3.2, DevExpress half)

Date: 2026-11-22. Owner's words (2026-11-22): "for devexpress, they have more
controls and some nice demos and tools i would like to have an equivalent
of." Telerik's missing piece, a converter integrated into MCP, is now MCP study
stage G7 and is not repeated here.

**Clean-room rule.** Everything below comes from DevExpress's public
documentation (docs.devexpress.com), product pages (devexpress.com) and demo
listings. No DevExpress code or binaries were read, downloaded, decompiled or
installed. Building any item marked take must follow the same rule: copy
behaviour and appearance from the public demos and documentation only.

**What it was mapped against:** the GUI.Forms public headers at cp36
(`include/gui_forms/controls`, `components`, `plots`, `docking`,
`application`), the Gallery catalogue (`demo/gallery_catalog.inc`, about 170
pages), the demonstrators (`chat_stress`, `node_graph`, the ribbon
demonstrator), the file manager demoboard, Plan Paint,
`planning/CONTROL_COMPLETENESS_MATRIX.md`, `planning/WINFORMS_API_CATALOGUE.md`
and `docs/controls/generated/`. Our tools: the form compiler
(`tools/form_compiler`), `theme_check`, the theme editor (T1, malkuth-tools),
the WYSIWYG editor (T2), the WinForms reflector (T3), `tools/winforms_catalogue`,
`tools/compat_capture`, the help book, the visual inspector and the stock icon
generator.

**Marks.** **take**: worth an equivalent of our own. **have**: we already have
it (our name given). **leave**: not worth an equivalent (reason given).
**Size** applies to takes: S is days, M is a week or two, L is a month, XL is
a product. "Partial" in the Ours column means we have the base control but not
the DevExpress feature named in that row; such a row is marked take.

Links point to the public page that shows the item best: a product page
(screenshots and videos), the docs topic, or the sample-apps page. The
installed Demo Center demos (`dxdemo://` links) need DevExpress installed,
which the clean-room rule forbids, so their public listing pages are linked
instead.

Base URLs: `DX` = `https://docs.devexpress.com/WindowsForms`,
`P` = `https://www.devexpress.com/products/net/controls/winforms`.

## 1. WinForms controls and components

### 1.1 Data Grid (GridControl) and its views

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Data Grid, Grid View (table) | DataGridView, VirtualRowsView | have | | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Banded Grid View (column bands) | none | take | M | [DX/3455](https://docs.devexpress.com/WindowsForms/3455/controls-and-libraries/data-grid) |
| Advanced Banded Grid View (multi-row bands) | none | take | M | [DX/3455](https://docs.devexpress.com/WindowsForms/3455/controls-and-libraries/data-grid) |
| Card View | Card, but no card view over records | take | M | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Layout View (record cards with a layout per card) | none | take | M | [Sample apps: Contacts](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Tile View (records as tiles, HTML templates) | none | take | M | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| WinExplorer View (Explorer-style icons and captions) | ListView (large/small icons, tiles) | have | | [Sample apps: File Explorer-like UI](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Office Compact View (Outlook task list) | none | take | S | [Sample apps: Tasks](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Group-by panel, grouping and group rows | partial: ListView groups only | take | M | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Total and group summaries (footer, group footers) | none | take | M | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Find panel (search across all columns, highlight) | SearchField, not wired to grids | take | S | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Auto-filter row and Excel-style column filter drop-downs | partial: DataGridView filtering through ObjectView | take | M | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Conditional formatting (data bars, colour scales, icon sets, rules) | none | take | M | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Master-detail (nested detail grids) | MasterDetailView | have | | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| In-place editors from a repository (any editor in any cell) | partial: DataGridView cell types | take | M | [DX editors](https://docs.devexpress.com/WindowsForms/114580/controls-and-libraries/editors-and-simple-controls) |
| Column chooser, column customisation form | none | take | S | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Frozen (fixed) columns and rows | DataGridView frozen rows and columns | have | | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Cell merging (equal adjacent values) | none | take | S | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| Server mode and Instant Feedback (async large sources) | VirtualRowsView, DataGridView virtual mode | have | | [DX/8398](https://docs.devexpress.com/WindowsForms/8398/controls-and-libraries/data-grid/data-binding/large-data-sources-server-and-instant-feedback-modes) |
| HTML and CSS row templates | form documents (XML + CSS) for controls, not for grid rows | take | M | [HTML & CSS](https://www.devexpress.com/winforms/html-css/) |
| Layout save and restore (columns, sort, filter) | partial: workspace transactions | take | S | [DX/17674](https://docs.devexpress.com/WindowsForms/17674/controls-and-libraries/form-layout-managers/workspace-manager) |
| Data export to XLSX/CSV/PDF from grids | none | take | M | [P/printing](https://www.devexpress.com/products/net/controls/winforms/printing/) |
| Drag-and-drop behaviour between grids and lists | drag (library-wide) | have | | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |

### 1.2 Other grids

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Tree List (hierarchical grid, unbound and bound) | TreeListView | have | | [P/tree_list](https://www.devexpress.com/products/net/controls/winforms/tree_list/) |
| Tree List summaries, filtering and checkboxes per level | partial | take | S | [P/tree_list](https://www.devexpress.com/products/net/controls/winforms/tree_list/) |
| Vertical Grid (records as columns, rows as fields) | none | take | M | [P/vertical_grid](https://www.devexpress.com/products/net/controls/winforms/vertical_grid/) |
| Property Grid (categories, editors, tabs, search) | PropertyGrid, PropertyList, ObjectView | have | | [DX/119885](https://docs.devexpress.com/WindowsForms/119885/controls-and-libraries/property-grid) |
| Gantt Control (tasks, dependencies, critical path, split tasks) | none | take | L | [P gantt](https://www.devexpress.com/winforms/gantt/) |
| Pivot Grid (OLAP-style pivot, drill-down, chart link) | none | take | XL | [P/pivot_grid](https://www.devexpress.com/products/net/controls/winforms/pivot_grid/) |
| Filter Control and Filter Editor (visual and text criteria builder) | none | take | M | [Filter Editor](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.FilterEditorControl) |
| Expression Editor (calculated fields) | none | take | M | [DX editors](https://docs.devexpress.com/WindowsForms/114580/controls-and-libraries/editors-and-simple-controls) |

### 1.3 Data editors (XtraEditors)

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| TextEdit (with masks, null text, buttons) | TextBox, MaskedTextBox | have | | [TextEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.TextEdit) |
| ButtonEdit (text box with custom buttons) | partial: SearchField, TextBox decorations | take | S | [ButtonEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.ButtonEdit) |
| MemoEdit | TextBox (multiline), ContentTextEditor | have | | [P text inputs](https://www.devexpress.com/products/net/controls/winforms/editors/text-inputs.xml) |
| MemoExEdit (drop-down multi-line panel) | none | take | S | [P text inputs](https://www.devexpress.com/products/net/controls/winforms/editors/text-inputs.xml) |
| SpinEdit | NumericUpDown | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| CalcEdit (calculator drop-down) | none | take | S | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| TrackBarControl | TrackBar | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ZoomTrackBarControl (track bar with minus and plus) | none | take | S | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| RangeTrackBarControl (two thumbs) | RangeControl | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| SparklineEdit | Sparkline | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ProgressBarControl, MarqueeProgressBarControl | ProgressBar (marquee style), ProgressRing, Spinner | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| CheckEdit (three states, custom glyphs) | CheckBox | have | | [CheckEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.CheckEdit) |
| ToggleSwitch | ToggleSwitch | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| DateEdit (calendar drop-down, Vista-style picker) | DateTimePicker | have | | [P date editors](https://www.devexpress.com/products/net/controls/winforms/editors/date-time-picker-and-calendar.xml) |
| TimeEdit (spin or drop-down time selector) | DateTimePicker (time format) | have | | [P date editors](https://www.devexpress.com/products/net/controls/winforms/editors/date-time-picker-and-calendar.xml) |
| TimeSpanEdit (durations) | none | take | S | [P date editors](https://www.devexpress.com/products/net/controls/winforms/editors/date-time-picker-and-calendar.xml) |
| CalendarControl (dates and date ranges, several months) | MonthCalendar | have | | [P date editors](https://www.devexpress.com/products/net/controls/winforms/editors/date-time-picker-and-calendar.xml) |
| Date range picker (two calendars, presets) | partial: MonthCalendar selection range | take | S | [Blazor Date Range Picker](https://docs.devexpress.com/Blazor/405111/components/data-editors/daterangepicker) |
| PictureEdit (image with zoom, crop, context menu) | PictureBox | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ImageEdit (image in a drop-down) | none | leave | | covered by PictureBox in a popup; no demand |
| ImageComboBoxEdit (images in a combo list) | ComboBox (owner-drawn items) | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ColorEdit, ColorPickEdit (palettes, custom colours) | ColorPicker, ColorValueEditor, RibbonColorPicker | have | | [ColorEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.ColorEdit) |
| ComboBoxEdit | ComboBox | have | | [ComboBoxEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.ComboBoxEdit) |
| CheckedComboBoxEdit (multi-select drop-down) | none | take | S | [CheckedComboBoxEdit](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.CheckedComboBoxEdit) |
| FontEdit (font list drop-down) | RibbonFontControl, FontDialog | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| MRUEdit (remembers entered values) | AutoComplete on ComboBox | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| PopupGalleryEdit (gallery in a drop-down) | RibbonGallery (drop-down) | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| RadioGroup | ChoiceGroup | have | | [RadioGroup](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.RadioGroup) |
| TokenEdit | TokenBox | have | | [DX/17088](https://docs.devexpress.com/WindowsForms/17088/controls-and-libraries/editors-and-simple-controls/token-edit-control) |
| LookUpEdit (tabular drop-down) | LookupComboBox | have | | [P data lookups](https://www.devexpress.com/products/net/controls/winforms/editors/data-lookups.xml) |
| GridLookUpEdit (a full grid in the drop-down) | none | take | M | [P data lookups](https://www.devexpress.com/products/net/controls/winforms/editors/data-lookups.xml) |
| SearchLookUpEdit (grid drop-down with find panel) | none | take | M | [P data lookups](https://www.devexpress.com/products/net/controls/winforms/editors/data-lookups.xml) |
| TreeListLookUpEdit (tree in the drop-down) | none | take | S | [P data lookups](https://www.devexpress.com/products/net/controls/winforms/editors/data-lookups.xml) |
| PopupContainerEdit (any content in a drop-down) | partial: DropDownButton, AnchoredOverlayLayer | take | S | [DX/612](https://docs.devexpress.com/WindowsForms/612/controls-and-libraries/editors-and-simple-controls/popup-container-editor) |
| RatingControl (stars or icons) | none | take | S | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| HyperLinkEdit | LinkLabel | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| BreadCrumbEdit | BreadcrumbTrail | have | | [DX/16973](https://docs.devexpress.com/WindowsForms/16973/controls-and-libraries/editors-and-simple-controls/breadcrumb-edit-control) |
| RepositoryItemRichTextEdit (RTF in a cell) | RichTextBox, not as a cell editor | take | S | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| Masked input (numeric, date, regular-expression masks) | MaskedTextBox, masked text provider | have | | [Blazor Masked Input](https://docs.devexpress.com/Blazor/405118/components/data-editors/maskedinput) |

### 1.4 Simple controls

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| SimpleButton, CheckButton | Button, CheckBox (button appearance) | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| DropDownButton | DropDownButton | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| LabelControl (formatted text, images, HTML tags) | Label, TextBlock | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| HyperlinkLabelControl | LinkLabel | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ListBoxControl, ImageListBoxControl, CheckedListBoxControl | ListBox, CheckedListBox | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| Templated list box items (HTML item templates) | none | take | S | [DX/119680](https://docs.devexpress.com/WindowsForms/119680/controls-and-libraries/editors-and-simple-controls/templated-listbox-items) |
| ControlNavigator, DataNavigator | BindingNavigator | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| BarCodeControl (1D and 2D codes, QR) | none | take | M | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| ImageSlider (images with navigation and animation) | none | take | S | [DX/12364](https://docs.devexpress.com/WindowsForms/12364/controls-and-libraries/editors-and-simple-controls/image-slider) |
| ProgressPanel (wait message) | Spinner, EmptyState | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| RangeControl (range over a chart or numbers) | RangeControl | have | | [DX/11739](https://docs.devexpress.com/WindowsForms/11739/controls-and-libraries/editors-and-simple-controls/range-control) |
| GalleryControl (grouped image gallery) | partial: RibbonGallery, ListView tiles | take | S | [P navigation](https://www.devexpress.com/products/net/controls/winforms/navigation/) |
| CameraControl (live capture device) | none | take | M | [DX/114582](https://docs.devexpress.com/WindowsForms/114582/controls-and-libraries/editors-and-simple-controls/camera-control) |
| StepProgressBar | StepIndicator | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| SearchControl (filters an attached control) | SearchField | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| GroupControl, PanelControl | GroupBox, Panel, MaterialPanel | have | | [P/layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| HScrollBar, VScrollBar, XtraScrollableControl | HScrollBar, VScrollBar, ScrollableControl | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| SplitterControl, SplitContainerControl | Splitter, SplitContainer | have | | [P/layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| XtraTabControl (tab headers on any side, close buttons) | TabControl, PageView, PageStrip | have | | [DX/114576](https://docs.devexpress.com/WindowsForms/114576/controls-and-libraries/form-layout-managers/tab-control) |
| RoundedSkinPanel | MaterialPanel, Card | have | | [P editors](https://www.devexpress.com/products/net/controls/winforms/editors/) |
| SidePanel (one-pixel borders, snapping) | SplitContainer with a collapsed side, DockSite | have | | [DX/118337](https://docs.devexpress.com/WindowsForms/118337/controls-and-libraries/form-layout-managers/side-panel) |
| HTML Content Control and HTML Content Popup | form documents rendered as controls | have | | [HtmlContentControl](https://docs.devexpress.com/WindowsForms/DevExpress.XtraEditors.HtmlContentControl) |
| Code editor (Monaco hosted in WebView2, an example) | ContentTextEditor; SwiftEdit's CodeEditor | take | M | [GitHub example page](https://github.com/DevExpress-Examples/winforms-monaco-code-editor) (read the README only) |

### 1.5 Components

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| ImageCollection, SharedImageCollection | ImageList | have | | [DX included controls](https://docs.devexpress.com/WindowsForms/401381/controls-and-libraries/editors-and-simple-controls/included-controls-and-components) |
| SvgImageCollection, DPIAwareImageCollection | ImageList with vector icons, stock icons | have | | same |
| ToolTipController (super tips, HTML tips) | ToolTip, RibbonToolTip | have | | same |
| AlertControl (desktop alert windows) | Toast, NotifyIcon | have | | [DX/5487](https://docs.devexpress.com/WindowsForms/5487/controls-and-libraries/messages-notifications-and-dialogs/alert-windows) |
| ToastNotificationsManager (OS toasts) | Toast (in-window); NotifyIcon balloons | take | S | [DX/17020](https://docs.devexpress.com/WindowsForms/17020/controls-and-libraries/messages-notifications-and-dialogs/toast-notification-manager) |
| DXErrorProvider, DXValidationProvider (rules per editor) | ErrorProvider, validation | have | | same |
| SplashScreenManager (splash, wait form, overlay form) | application splashscreen, ModalLayer | have | | [P splash](https://www.devexpress.com/winforms/splash-screen/) |
| StyleController | themes and CSS | leave | deprecated by DevExpress; our CSS covers it |
| TransitionManager (animated transitions between states) | animation (library-wide), not a ready transition component | take | S | [DX included controls](https://docs.devexpress.com/WindowsForms/401381/controls-and-libraries/editors-and-simple-controls/included-controls-and-components) |
| WorkspaceManager (save and restore whole-app layouts) | partial: workspace transactions, dock layout | take | S | [DX/17674](https://docs.devexpress.com/WindowsForms/17674/controls-and-libraries/form-layout-managers/workspace-manager) |
| PersistentRepository | none | leave | a designer artefact of the repository-item model |
| TaskbarAssistant (taskbar progress, jump list, thumbnail buttons) | partial: host taskbar progress | take | S | [DX/114579](https://docs.devexpress.com/WindowsForms/114579/controls-and-libraries/forms-and-user-controls/taskbar-assistant) |
| Adorner UI Manager (badges, guides over any control) | Badge, Callout | have | | [DX/114584](https://docs.devexpress.com/WindowsForms/114584/controls-and-libraries/forms-and-user-controls/adorner-ui-manager) |
| Adorner guides (step-by-step tour highlighting) | none | take | S | [DX/114584](https://docs.devexpress.com/WindowsForms/114584/controls-and-libraries/forms-and-user-controls/adorner-ui-manager) |
| Behavior Manager and drag-and-drop behaviours | drag | have | | [P grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |

### 1.6 Navigation

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Accordion Control (hierarchical side menu, search, HTML items) | ExpandableSections, NavigationPane | take | M | [DX/114553](https://docs.devexpress.com/WindowsForms/114553/controls-and-libraries/navigation-controls/accordion-control) |
| Hamburger menu style (accordion collapsed to icons) | NavigationPane (compact) | have | | [DX/120498](https://docs.devexpress.com/WindowsForms/120498/controls-and-libraries/navigation-controls/accordion-control/hamburger-menu-view-style) |
| Office Navigation Bar (Mail, Calendar, People with peek) | none | take | S | [DX/114558](https://docs.devexpress.com/WindowsForms/114558/controls-and-libraries/navigation-controls/office-navigation-bar) |
| Navigation Frame (SDI page switching with animation) | PageView | have | | [DX/114554](https://docs.devexpress.com/WindowsForms/114554/controls-and-libraries/navigation-controls/navigation-frame-and-tab-pane) |
| Tab Pane | PageStrip with PageView | have | | same |
| Navigation Pane (side tabs that expand) | NavigationPane | have | | [DX/114555](https://docs.devexpress.com/WindowsForms/114555/controls-and-libraries/navigation-controls/navigation-pane) |
| Navigation Bar (Outlook-style groups) | NavigationPane, ExpandableSections | have | | [DX/4870](https://docs.devexpress.com/WindowsForms/4870/controls-and-libraries/navigation-controls/navigation-bar) |
| Tile Bar (horizontal tile strip with drop-downs) | none | take | M | [DX/114556](https://docs.devexpress.com/WindowsForms/114556/controls-and-libraries/navigation-controls/tile-bar) |
| Tile Navigation Pane | none | take | S | [DX/114557](https://docs.devexpress.com/WindowsForms/114557/controls-and-libraries/navigation-controls/tilenav-pane) |
| Tile Control (live tiles, groups, drag reorder) | none | take | M | [P/tile](https://www.devexpress.com/products/net/controls/winforms/tile/) |
| Toolbox Control (categorised item palette, search) | none (the WYSIWYG editor will need one) | take | S | [ToolboxControl](https://docs.devexpress.com/WindowsForms/DevExpress.XtraToolbox.ToolboxControl) |
| WindowsUI Button Panel (flat round buttons) | CommandBar | have | | [DX/114583](https://docs.devexpress.com/WindowsForms/114583/controls-and-libraries/navigation-controls/windowsui-button-panel) |
| Wizard Control (Aero and Wizard 97 styles) | Wizard, WizardPage | have | | [DX/4735](https://docs.devexpress.com/WindowsForms/4735/controls-and-libraries/form-layout-managers/wizard-control) |

### 1.7 Ribbon, bars and menus

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Ribbon (styles: Office 2007 to 2019, Office Universal, Tablet) | Ribbon | have | | [Ribbon](https://www.devexpress.com/winforms/ribbon/) |
| Simplified (single-line) ribbon style | none | take | S | [DX/14624](https://docs.devexpress.com/WindowsForms/14624/controls-and-libraries/ribbon-bars-and-menu/ribbon/ribbon-styles) |
| Quick Access Toolbar | QuickAccessToolbar | have | | [DX/2496](https://docs.devexpress.com/WindowsForms/2496/controls-and-libraries/ribbon-bars-and-menu/ribbon/visual-elements/quick-access-toolbar) |
| Backstage View | BackstageView | have | | [DX/11017](https://docs.devexpress.com/WindowsForms/11017/controls-and-libraries/ribbon-bars-and-menu/ribbon/main-menus/backstageview-control) |
| Ribbon galleries (in-ribbon and drop-down) | RibbonGallery | have | | [DX/2641](https://docs.devexpress.com/WindowsForms/2641/controls-and-libraries/ribbon-bars-and-menu/ribbon/galleries) |
| Contextual tabs (category colours) | Ribbon contextual tabs | have | | [DX/3327](https://docs.devexpress.com/WindowsForms/3327/controls-and-libraries/ribbon-bars-and-menu/ribbon/visual-elements/categories-and-contextual-tabs) |
| Ribbon status bar | StatusStrip | have | | [DX/2498](https://docs.devexpress.com/WindowsForms/2498/controls-and-libraries/ribbon-bars-and-menu/ribbon/the-ribbon-ui/ribbon-status-bar) |
| Mini Toolbar (fading toolbar at the selection) | none | take | S | [RibbonMiniToolbar](https://docs.devexpress.com/WindowsForms/DevExpress.XtraBars.Ribbon.RibbonMiniToolbar) |
| Search menu (command search, "Tell me") | none | take | S | [DX/400621](https://docs.devexpress.com/WindowsForms/400621/controls-and-libraries/ribbon-bars-and-menu/ribbon/visual-elements/search-menu) |
| Ribbon runtime customisation (users add groups, commands) | none | take | M | [DX/11528](https://docs.devexpress.com/WindowsForms/11528/controls-and-libraries/ribbon-bars-and-menu/ribbon/runtime-capabilities/runtime-customization) |
| Ribbon and bar merging for MDI children | partial: MDI follow-ups in cp36 | take | S | [DX/3451](https://docs.devexpress.com/WindowsForms/3451/controls-and-libraries/ribbon-bars-and-menu/ribbon/runtime-capabilities/ribbon-merging) |
| Bars (docked and floating toolbars, menus, customisation) | ToolStrip, ToolStripContainer, MenuStrip, CommandBar | have | | [P/bars](https://www.devexpress.com/products/net/controls/winforms/bars/) |
| Toolbar customisation dialog (users rearrange toolbars) | none | take | S | [DX/5361](https://docs.devexpress.com/WindowsForms/5361/controls-and-libraries/ribbon-bars-and-menu/bars) |
| Popup menus | ContextMenu | have | | [DX/1090](https://docs.devexpress.com/WindowsForms/1090/controls-and-libraries/ribbon-bars-and-menu/popup-menus) |
| Radial Menu (circular, touch-friendly) | none | take | S | [DX/14628](https://docs.devexpress.com/WindowsForms/14628/controls-and-libraries/ribbon-bars-and-menu/radial-menu) |
| Touch support (larger ribbon, touch mode) | touch (week plan 3.1) | have | | [DX/16870](https://docs.devexpress.com/WindowsForms/16870/controls-and-libraries/ribbon-bars-and-menu/ribbon/runtime-capabilities/touch-support) |
| Side-strip layout (vertical toolbars) | ToolStripPanel (left and right) | have | | [DX/1095](https://docs.devexpress.com/WindowsForms/1095/controls-and-libraries/ribbon-bars-and-menu/bars/appearance-and-hotkeys/side-strip-appearance) |

### 1.8 Forms

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| XtraForm (skinned title bar) | Window with custom chrome | have | | [DX/114560](https://docs.devexpress.com/WindowsForms/114560/controls-and-libraries/forms-and-user-controls/xtraform) |
| DirectX Form (HTML/CSS-styled whole form) | Window, form documents | have | | [DX/403878](https://docs.devexpress.com/WindowsForms/403878/controls-and-libraries/forms-and-user-controls/direct-x-form) |
| Ribbon Form (ribbon merged into the title bar) | Ribbon in custom chrome | have | | [DX/114562](https://docs.devexpress.com/WindowsForms/114562/controls-and-libraries/forms-and-user-controls/ribbon-form) |
| Toolbar Form (toolbar in the title bar) | custom chrome | have | | [DX/401889](https://docs.devexpress.com/WindowsForms/401889/controls-and-libraries/forms-and-user-controls/toolbar-form) |
| Tabbed Form (browser-style tabs in the title bar, tear off) | none | take | M | [DX/115183](https://docs.devexpress.com/WindowsForms/115183/controls-and-libraries/forms-and-user-controls/tabbed-form) |
| Fluent Design Form (acrylic, reveal highlight, hamburger) | window backdrop (acrylic, mica) and NavigationPane | have | | [DX/120214](https://docs.devexpress.com/WindowsForms/120214/controls-and-libraries/forms-and-user-controls/fluent-design-form) |
| XtraUserControl | UserControl | have | | [DX/114564](https://docs.devexpress.com/WindowsForms/114564/controls-and-libraries/forms-and-user-controls/xtrausercontrol) |
| Form Assistant (turns a form into Ribbon or Fluent form) | none | leave | a designer conversion of a WinForms Form type; our forms choose chrome by property |

### 1.9 Messages, notifications and dialogs

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| XtraMessageBox (skinned, auto-close, "do not show again") | MessageBox, TaskDialog | have | | [DX/114566](https://docs.devexpress.com/WindowsForms/114566/controls-and-libraries/messages-notifications-and-dialogs/xtramessagebox) |
| XtraInputBox (one editor in a dialog) | none (TaskDialog has no editor) | take | S | [DX/117729](https://docs.devexpress.com/WindowsForms/117729/controls-and-libraries/messages-notifications-and-dialogs/input-box) |
| XtraDialog (any user control as a dialog) | ModalLayer, Window | have | | [DX/114567](https://docs.devexpress.com/WindowsForms/114567/controls-and-libraries/messages-notifications-and-dialogs/xtradialog) |
| File and folder dialogs (skinned) | OpenFileDialog, SaveFileDialog, FolderBrowserDialog | have | | [DX/403445](https://docs.devexpress.com/WindowsForms/403445/controls-and-libraries/messages-notifications-and-dialogs/file-and-folder-browsers) |
| File Explorer Assistant (build an Explorer from grid, tree, breadcrumb) | file manager demoboard | have | | [DX/403431](https://docs.devexpress.com/WindowsForms/403431/controls-and-libraries/messages-notifications-and-dialogs/custom-folder-browsers) |
| Flyout Dialog (dimmed overlay dialog) | ModalLayer | have | | [DX/114568](https://docs.devexpress.com/WindowsForms/114568/controls-and-libraries/messages-notifications-and-dialogs/flyout-dialog) |
| Flyout Panel (slides from an edge, beak) | Callout, AnchoredOverlayLayer | have | | [DX/114578](https://docs.devexpress.com/WindowsForms/114578/controls-and-libraries/messages-notifications-and-dialogs/flyout-panel) |
| HTML/CSS templates for message boxes and alerts | form documents | have | | [DX/404058](https://docs.devexpress.com/WindowsForms/404058/controls-and-libraries/messages-notifications-and-dialogs/html-and-css-templates-in-messages-and-dialogs) |

### 1.10 Layout

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Layout Control (auto-aligned labels, groups, tabs) | FormLayout, GridPanel, FlexPanel | have | | [DX/3407](https://docs.devexpress.com/WindowsForms/3407/controls-and-libraries/form-layout-managers/layout-and-data-layout-controls/layout-control/layout-control) |
| Layout Control runtime customisation (users drag items, hide, save) | none | take | M | [P/layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| Flow layout mode | FlowLayoutPanelCompat, FlexPanel | have | | [DX/17548](https://docs.devexpress.com/WindowsForms/17548/controls-and-libraries/form-layout-managers/layout-and-data-layout-controls/flow-layout) |
| Data Layout Control (generates an edit form from a data source) | partial: `<bind>` elements in form documents | take | M | [DX/4921](https://docs.devexpress.com/WindowsForms/4921/controls-and-libraries/form-layout-managers/layout-and-data-layout-controls/data-layout-control) |
| Stack and Table panels | FlexPanel, TableLayoutPanelCompat, GridPanel | have | | [DX/402917](https://docs.devexpress.com/WindowsForms/402917/controls-and-libraries/form-layout-managers/simple-layout-containers/stack-and-table-panels) |
| Responsive breakpoints (Blazor Layout Breakpoint) | ResponsiveTrackPanel | have | | [Blazor](https://docs.devexpress.com/Blazor/405072/components/layout/layout-breakpoint) |
| Tabbed MDI Manager | DockWorkspace (tabbed documents) | have | | [DX/5486](https://docs.devexpress.com/WindowsForms/5486/controls-and-libraries/form-layout-managers/tabbed-mdi-manager) |

### 1.11 Application UI Manager and docking

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Docking Library (dock, float, auto-hide, tabbed panels, hints) | DockSite, DockWorkspace, DockNavigator | have | | [P/docking](https://www.devexpress.com/products/net/controls/winforms/docking/) |
| Document Manager, Tabbed View (tab groups, float, pin) | DockWorkspace | have | | [DX/11355](https://docs.devexpress.com/WindowsForms/11355/controls-and-libraries/application-ui-manager/views/tabbed-view) |
| Native MDI View | DockWorkspace MDI presentation | have | | [DX/11356](https://docs.devexpress.com/WindowsForms/11356/controls-and-libraries/application-ui-manager/views/native-mdi-view) |
| WindowsUI View (tile container, page groups, flyouts) | none | take | L | [DX/11854](https://docs.devexpress.com/WindowsForms/11854/controls-and-libraries/application-ui-manager/views/windowsui-view) |
| Widget View (documents as resizable cards, maximise) | none | take | M | [DX/16239](https://docs.devexpress.com/WindowsForms/16239/controls-and-libraries/application-ui-manager/views/widget-view) |
| Deferred load of documents | page and document views load on show | have | | [DX/11619](https://docs.devexpress.com/WindowsForms/11619/controls-and-libraries/application-ui-manager/deferred-load/deferred-load) |
| Document selector (Ctrl+Tab switcher with previews) | DockNavigator | have | | [DX/11363](https://docs.devexpress.com/WindowsForms/11363/controls-and-libraries/application-ui-manager/interaction-with-dock-panels/interaction-with-dock-panels) |

### 1.12 Data visualisation

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Charts: line, spline, step, area, bar, scatter, point | PlotView (lines, stairs, stems, bars, areas, scatter) | have | | [P/chart](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Charts: pie, doughnut, nested doughnut, funnel, pyramid | none | take | M | [P/chart](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Charts: financial (candlestick, OHLC, indicators, trend lines) | none | take | M | [Sample apps: Finance and Trading](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Charts: range bar, range area, Gantt-style bars, bubble, box plot, error bars | partial: histogram | take | M | [P/chart](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Charts: polar and radar | none | take | S | [Blazor Polar Chart](https://docs.devexpress.com/Blazor/401769/components/charts/get-started-with-charts) |
| Charts: 3D bars, pies, areas | Plot3DView (surfaces) | have | | [P/chart](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Chart crosshair, tooltips, zoom, scroll, selection | PlotView gestures, query, box zoom | have | | [P/chart](https://www.devexpress.com/products/net/controls/winforms/chart/) |
| Chart Designer (run-time chart editing dialog) | none | take | M | [DX/8117](https://docs.devexpress.com/WindowsForms/8117/controls-and-libraries/chart-control) |
| Gauges: circular, linear | RadialGauge, LinearGauge, Dial | have | | [P/gauges](https://www.devexpress.com/products/net/controls/winforms/gauges/) |
| Gauges: digital (seven and fourteen segment, matrix) | SevenSegmentDisplay | have | | [P/gauges](https://www.devexpress.com/products/net/controls/winforms/gauges/) |
| Gauges: state indicator | IndicatorLamp | have | | [P/gauges](https://www.devexpress.com/products/net/controls/winforms/gauges/) |
| Gauge Designer (presets gallery, element editor) | none | take | S | [DX/18218](https://docs.devexpress.com/WindowsForms/18218/controls-and-libraries/gauges) |
| Diagram Control (shapes, connectors, auto layout, org charts) | NodeCanvas, node graph (nodes and links, no shapes library) | take | L | [P/diagrams](https://www.devexpress.com/products/net/controls/winforms/diagrams/) |
| Map Control (tile providers, vector layers, routes, bubbles) | none | take | L | [P/map](https://www.devexpress.com/products/net/controls/winforms/map/) |
| TreeMap | none | take | S | [P/treemap](https://www.devexpress.com/products/net/controls/winforms/treemap/) |
| Sunburst | none | take | S | [P/sunburst](https://www.devexpress.com/products/net/controls/winforms/sunburst/) |
| Sankey Diagram | none | take | M | [Sankey](https://www.devexpress.com/winforms/sankey-diagram/) |
| Heatmap Control | PlotHeatmapSeries | have | | [HeatMap](https://www.devexpress.com/winforms/heatmap/) |
| Range selector over a chart (Blazor Range Selector) | RangeControl, SweepScope | have | | [Blazor](https://docs.devexpress.com/Blazor/405266/components/range-selector) |

### 1.13 Office-inspired

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Rich Text Editor (Word-like: pages, tables, styles, headers, mail merge) | RichTextBox (WinForms parity), ContentTextEditor | take | XL | [P/rich_editor](https://www.devexpress.com/products/net/controls/winforms/rich_editor/) |
| Spreadsheet (formulas, charts in sheets, XLSX) | none | take | XL | [P/spreadsheet](https://www.devexpress.com/products/net/controls/winforms/spreadsheet/) |
| Scheduler (day, week, month, timeline, agenda, Gantt views, recurrence) | none | take | L | [P/scheduler](https://www.devexpress.com/products/net/controls/winforms/scheduler/) |
| Spell Checker (as-you-type, dictionaries) | none (the platform spell services are unused) | take | M | [P/spell_checker](https://www.devexpress.com/products/net/controls/winforms/spell_checker/) |
| Office File API (DOCX, XLSX, PDF processing without UI) | none | leave | a document-processing library, not a toolkit; take only what the editors above need |

### 1.14 Reporting and analytics

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| PDF Viewer (view, search, forms, annotations) | none | take | L | [P/pdf-viewer](https://www.devexpress.com/products/net/controls/winforms/pdf-viewer/) |
| Printing and exporting (print preview, PDF/XLSX export of any control) | partial: help printing | take | L | [P/printing](https://www.devexpress.com/products/net/controls/winforms/printing/) |
| Reporting: Report Viewer (banded documents) | none | take | XL | [Reporting](https://www.devexpress.com/subscriptions/reporting/) |
| BI Dashboard (viewer and end-user designer) | none | leave | a separate BI product; revisit after reporting |

### 1.15 Cross-cutting technologies

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| Application skins (vector and raster), palettes | themes, user themes, surface materials | have | | [P/skins](https://www.devexpress.com/products/net/controls/winforms/skins/) |
| Skin selector gallery and palette gallery (runtime theme switch) | partial: theme switching; no ready ribbon gallery item | take | S | [P/skins](https://www.devexpress.com/products/net/controls/winforms/skins/) |
| HTML and CSS desktop UI | form documents (XML + CSS) | have | | [HTML & CSS](https://www.devexpress.com/winforms/html-css/) |
| DirectX hardware acceleration | our own renderer and compositor | have | | [DirectX](https://www.devexpress.com/winforms/directx/) |
| MVVM framework (commands, services, fluent binding) | commands, binding, BindingSource | have | | [DX/113955](https://docs.devexpress.com/WindowsForms/113955/cross-platform-app-development/winforms-mvvm) |
| Accessibility (UI Automation, screen readers) | semantics (accessibility tree) | have | | [DX/404293](https://docs.devexpress.com/WindowsForms/404293/build-an-application/accessibility) |
| High DPI and per-monitor scaling | scaling (library-wide) | have | | [DX/119441](https://docs.devexpress.com/WindowsForms/119441/common-features/graphics-performance-and-high-dpi/directx-hardware-acceleration) |
| AI Chat control | chat stress demonstrator (a chat view under load) | take | M | [DX/405218](https://docs.devexpress.com/WindowsForms/405218/ai-powered-extensions/ai-chat-control) |
| AI Assistant, Smart Paste, Smart Autocomplete, Smart Search, Semantic Search, Explain Formula, Image Description | none | take | M | [DX/405151](https://docs.devexpress.com/WindowsForms/405151/ai-powered-extensions) |
| Coded UI test support | MCP automation (study G1 to G6) | have | | [Coded UI](https://www.devexpress.com/products/net/controls/winforms/coded-ui/) |
| Localization (resource strings, satellite assemblies) | localizable text, locale files | have | | [Localization](https://docs.devexpress.com/GeneralInformation/404608/localization/localization) |
| Icon library (thousands of SVG icons, context images) | stock icon generator | take | M | [DX/16864](https://docs.devexpress.com/WindowsForms/16864/devexpress-icon-library/image-gallery-and-context-dependent-images) |

## 2. Notable WPF and Blazor items with no WinForms counterpart

| DevExpress | Ours | Mark | Size | Best public page |
|---|---|---|---|---|
| WPF ThemedWindow | Window with custom chrome | have | | [ThemedWindow](https://docs.devexpress.com/WPF/DevExpress.Xpf.Core.ThemedWindow) |
| WPF LoadingDecorator (wait indicator over any content) | ModalLayer, Spinner | have | | [LoadingDecorator](https://docs.devexpress.com/WPF/DevExpress.Xpf.Core.LoadingDecorator) |
| WPF Badge | Badge | have | | [Badge](https://docs.devexpress.com/WPF/DevExpress.Xpf.Core.Badge) |
| Blazor Carousel | none | take | S | [Carousel](https://docs.devexpress.com/Blazor/405183/components/layout/carousel) |
| Blazor Drawer (side drawer over or beside content) | none | take | S | [Drawer](https://docs.devexpress.com/Blazor/404362/components/layout/drawer) |
| Blazor HTML Editor (WYSIWYG HTML) | none | leave | the Rich Text Editor take covers rich editing; HTML output is a web concern |
| Blazor Pager | none | take | S | [Pager](https://docs.devexpress.com/Blazor/405054/components/utility-controls/pager) |
| Blazor Button Group, Split Button | ChoiceGroup (button style), DropDownButton | have | | [Button Group](https://docs.devexpress.com/Blazor/405195/components/utility-controls/button-group) |
| Blazor Bar Gauge (concentric bars) | none | take | S | [Bar Gauge](https://docs.devexpress.com/Blazor/404946/components/bar-gauge) |
| Blazor Color Palette | ColorPicker | have | | [Color Palette](https://docs.devexpress.com/Blazor/405229/components/data-editors/color-palette) |
| Blazor Upload, File Input | file dialogs and drag | leave | web upload widgets; the desktop has file dialogs and drop |
| Blazor Grid Layout | GridPanel | have | | [Grid Layout](https://docs.devexpress.com/Blazor/405071/components/layout/grid-layout) |

## 3. Demo applications

| DevExpress demo | What it shows | Ours | Mark | Size | Best public page |
|---|---|---|---|---|---|
| Outlook-Inspired Application (mail, calendar, contacts, tasks) | Ribbon, grid views, scheduler, navigation bar, layout | file manager demoboard and Gallery cover parts | take | L | [Demos](https://www.devexpress.com/Support/Demos/) |
| Outlook-Inspired Tasks module | grid with detail panel, Office Compact View | MasterDetailView in the Gallery | take | M | [Sample apps](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Outlook-Inspired Calendar module | scheduler views, range control | none | take | M (after Scheduler) | same |
| Outlook-Inspired Contacts module | layout view cards, field layout | none | take | M | same |
| Dental Clinic Application | scheduler, modern UX, HTML templates | none | take | L (after Scheduler) | [Demos](https://www.devexpress.com/Support/Demos/) |
| Stock Market Trader | financial charts, real-time grid, HTML templates | chat stress shows real-time load; no trading screen | take | M | [Demos](https://www.devexpress.com/Support/Demos/) |
| HTML and CSS Templates demo | templated grid rows, Explorer view | form documents in the Gallery | take | S | same |
| Touch-Enabled Hybrid Application | tiles, large touch targets, flyouts | none | take | M (with touch, 3.1) | same |
| Fluent Mail Client | Fluent form, acrylic, hamburger accordion | window backdrop lab | take | M | same |
| Excel-inspired Spreadsheet | spreadsheet with ribbon, charts | none | take | (with Spreadsheet) | [Sample apps](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Excel-inspired Analytics | pivot grid with linked chart | none | take | (with Pivot Grid) | same |
| Word-inspired Applications | rich text editor with ribbon | none | take | (with Rich Text) | same |
| Microsoft CRM-inspired Applications | dashboards of grids and pie, funnel, bar charts | none | take | M | same |
| CRUD Applications | form layout, masked editors, grid, ribbon | Gallery bound values, form documents | have | | same |
| Reporting demo (viewer and end-user designer) | banded reports | none | take | (with Reporting) | [Reporting video](https://www.youtube.com/watch?v=oaXK-XhwF8g) |
| Visio-inspired Applications | diagram editor with ribbon | node graph demonstrator | take | M (with Diagram) | [Sample apps](https://www.devexpress.com/winforms/sample-apps-winforms.xml) |
| Touch-First Tablet Applications | tile control, button panel | none | take | (with Hybrid) | same |
| Finance and Trading Applications | financial charting, grid, doughnut | none | take | (with Stock Market Trader) | same |
| Adobe-inspired Applications (PDF) | PDF viewer, forms | none | take | (with PDF Viewer) | same |
| Browser-like UI | tabbed form, dock panels | none | take | S (with Tabbed Form) | same |
| File Explorer-like UI | Explorer view, accordion, breadcrumb, ribbon | file manager demoboard | have | | same |
| GIS Applications | map layers, routes | none | take | (with Map) | same |
| Visual Studio-inspired Docking | dock, float, auto-hide | DockWorkspace pages in the Gallery | have | | same |
| WinForms Dashboard demos (sales overview) | BI dashboard | none | leave | BI dashboard left (1.14) | [Demos](https://www.devexpress.com/Support/Demos/) |
| WinForms AI demos (AI rich edit) | AI assistant in editors | none | take | (with AI extensions) | same |
| XAF Employee Management | generated CRUD application framework | none | leave | XAF is an application framework, not a toolkit item |
| Per-control Main Demos in the Demo Center (Grid, Editors, Charts and so on) | one browsable module per feature | the Gallery | have | | [Demos](https://www.devexpress.com/Support/Demos/) |
| Demo Center (browse, run, open the source of each demo) | launcher with search and a source button | the Gallery, without a "show the code" button | take | S | [DX/14962](https://docs.devexpress.com/WindowsForms/14962/whats-installed/demo-applications) |

## 4. Design-time and productivity tools

| DevExpress tool | What it does | Ours | Mark | Size | Best public page |
|---|---|---|---|---|---|
| WinForms Skin Editor | edits skins and exports a skin library | theme editor (T1, malkuth-tools) | have | | [Skin Editor](https://docs.devexpress.com/SkinEditor/1630/winforms-skin-editor) |
| WPF Theme Designer (XAML editing, live preview) | standalone theme designer | theme editor (T1) | have | | [Theme Designer](https://docs.devexpress.com/WpfThemeDesigner/118707/wpf-theme-designer) |
| Palette editor and custom palettes (vector skins) | swatch editing per skin | partial: theme editor; `theme_check` | take | S | [P/skins](https://www.devexpress.com/products/net/controls/winforms/skins/) |
| Project Converter | upgrades DevExpress references between versions | none | leave | a version-upgrade tool for their own assemblies; our ABI and version policy does this job (G7 covers converting WinForms projects) |
| Project Settings page (project-wide skin, font, DPI options) | one page of app-wide defaults | partial: build profile, theme defaults | take | S | [DX/120180](https://docs.devexpress.com/WindowsForms/120180/whats-installed/project-settings-page) |
| Template Gallery and Cross-IDE Template Kit | new-project and new-item templates (Outlook-style, Ribbon app, Tabbed MDI) | none | take | M | [Template Kit](https://docs.devexpress.com/WindowsForms/405275/whats-installed/project-template-kit) |
| WinForms UI Templates | ready screens (login, CRUD, dashboards) to paste into apps | none | take | M | [UI Templates](https://www.devexpress.com/ui-templates/winforms/) |
| Instant Layout Assistant (VS extension to arrange controls in place) | quick layout at design time | WYSIWYG editor (T2) | take | (with T2) | [DX/116111](https://docs.devexpress.com/WindowsForms/116111/whats-installed/layout-assistant-extension) |
| Layout Converter (turns a plain form into a Layout Control) | migrates absolute layouts | WinForms converter (G7) | take | (with G7) | [P/layout](https://www.devexpress.com/products/net/controls/winforms/layout/) |
| Image Gallery and image picker (choose icons at design time) | browse and insert icons | stock icon generator | take | S | [DX/16864](https://docs.devexpress.com/WindowsForms/16864/devexpress-icon-library/image-gallery-and-context-dependent-images) |
| SVG Icon Builder | builds SVG icons in DevExpress style | stock icon generator | have | | [Store page](https://www.microsoft.com/en-us/store/p/svg-icon-builder/9mxbbwvknrvr) |
| Bar Manager and Ribbon designers | design toolbars and ribbons by drag | WYSIWYG editor (T2) | take | (with T2) | [DX/5353](https://docs.devexpress.com/WindowsForms/5353/controls-and-libraries/ribbon-bars-and-menu/bars/bar-manager-designer) |
| Grid Designer, Document Manager designer, Layout customisation | per-control design dialogs | WYSIWYG editor (T2) | take | (with T2) | [P/grid](https://www.devexpress.com/products/net/controls/winforms/grid/) |
| HTML and CSS Template Editor and template gallery | edit templates with live preview; reusable templates | form compiler; T2 | take | S | [HTML & CSS](https://www.devexpress.com/winforms/html-css/) |
| Visual Studio, Rider and VS Code Report Designers | design reports in the IDE | none | take | (with Reporting) | [Reporting](https://www.devexpress.com/subscriptions/reporting/) |
| End-User Report Designer | users design reports in the app | none | take | (with Reporting) | same |
| Dashboard Designer (VS and end-user) | BI dashboards | none | leave | BI dashboard left (1.14) | [P winforms](https://www.devexpress.com/products/net/controls/winforms/) |
| Data Source Wizard | binds controls to databases and services | binding | leave | data access belongs to the application; GUI.Forms binds to objects |
| Scaffolding Wizard | generates CRUD views from a data model | none | take | M | [P winforms](https://www.devexpress.com/products/net/controls/winforms/) |
| Online Localization Service and UI Localization Client | translators edit UI strings, with in-app capture | localizable text, locale files | take | M | [Localization service](https://localization.devexpress.com/) |
| Documentation MCP Server | AI assistants query the docs | MCP study (G-stages); help book | take | S | [Docs MCP](https://docs.devexpress.com/GeneralInformation/405551/help-resources/dev-express-documentation-mcp-server-configure-an-ai-powered-assistant) |
| F1 help on API in the designer and code | context help | API reference search, help book | have | | [DX/117844](https://docs.devexpress.com/WindowsForms/117844/whats-installed/getting-help-on-api) |
| Toolbox integration (controls on the VS toolbox) | IDE toolbox | none | leave | Visual Studio integration is after 1.0 and belongs to the Windows coordinator |
| CodeRush | IDE refactoring | none | leave | an IDE product, outside a UI toolkit |
| Unified Component Installer | installs the suite | none | leave | ours ships as a source SDK |

## 5. Counts

Take 129, have 128, leave 16 (counted from the Mark column of sections 1 to 4).

## 6. Ranked shortlist for the owner's pass

Ranked by what a GUI.Forms user would miss soonest, weighted by how much
they lift what we already have.

1. **Data grid views and features (1.1):** grouping with summaries, find
   panel, auto-filter row and Excel-style filters, conditional formatting,
   column chooser, banded and card or layout views. The grid is the centre of
   most business apps.
2. **Scheduler (1.13)** with its Outlook-style calendar module: day, week,
   month, timeline and agenda views, recurrence.
3. **Printing and export (1.14):** print preview and PDF/XLSX export of
   grids, charts and documents. Also the base for reporting and PDF viewing.
4. **Chart families (1.12):** pie, doughnut, funnel, financial
   (candlestick, OHLC), range, bubble, box plot, polar and radar, plus a
   chart designer.
5. **Diagram control (1.12):** a shapes library, connectors, auto layout and
   org charts on top of NodeCanvas and the node graph.
6. **Rich lookup and drop-down editors (1.3):** GridLookUpEdit,
   SearchLookUpEdit, CheckedComboBoxEdit, PopupContainerEdit, ButtonEdit,
   CalcEdit, RatingControl.
7. **Gantt control (1.2).**
8. **Accordion and tile navigation (1.6):** Accordion Control, Office
   Navigation Bar, Tile Bar and Tile Control.
9. **Tabbed Form (1.8)** and the Browser-like UI demo.
10. **Vertical Grid (1.2).**
11. **Outlook-Inspired showcase (3):** one demo application that brings 1, 2
    and 8 together.
12. **Spell checker (1.13).**
13. **Ribbon extras (1.7):** simplified ribbon, mini toolbar, search menu,
    runtime customisation.
14. **Template kit and UI templates (4)**, delivered with T2.
15. **PDF viewer (1.14).**
16. **Pivot grid, spreadsheet and Word-level rich text (1.2, 1.13):** the XL
    products, each its own programme after 1.0.
