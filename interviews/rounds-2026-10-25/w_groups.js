const GROUPS = [
  { key: "A", title: "Controls held for you", blurb: "The W worker has built OpenFileDialog, SaveFileDialog, FolderBrowserDialog, ColorDialog and DataGridViewImageColumn. Four items wait on these answers.", questions: [
    { id: "W-Q1", q: "How much of RichTextBox?",
      ctx: "Windows Forms' RichTextBox is a full RTF editor (128 members). The survey places it as a later package (matrix row 42).",
      opts: [
        { t: "Styled runs over our TextBox engine, with RTF reading and writing for that subset; no embedded objects", w: "Font, size, colour, bold, italic, underline, paragraph alignment, indents and bullets, plus find, selection formatting and undo. A separate package, built after T3 and L land.", rec: true },
        { t: "Full RTF fidelity, including tables, images and embedded objects", w: "Much larger; RTF's long tail is big." },
        { t: "Not now", w: "" } ] },
    { id: "W-Q2", q: "WinForms' old Splitter control?",
      ctx: "Splitter is the pre-2.0 control docked between two docked controls; SplitContainer replaced it, and we have SplitContainer.",
      opts: [
        { t: "A thin migration facade: a Splitter docked between dock children resizes its neighbour, built on our Dock layout", w: "Old Windows Forms code ports without rewriting; small.", rec: true },
        { t: "Don't build it; document the move to SplitContainer", w: "" } ] },
    { id: "W-Q3", q: "What does MessageBox.show use?",
      ctx: "Both exist already: the in-window TaskDialog, drawn by our engine and themed, and the host's own message dialog.",
      opts: [
        { t: "The host's message dialog, as the file and colour dialogs do; TaskDialog stays for themed, richer dialogs", w: "MessageBox looks like each system's own; one rule for every stock dialog.", rec: true },
        { t: "TaskDialog, drawn by our engine", w: "The same look on every host, and it follows the theme." },
        { t: "TaskDialog by default, with the host dialog as an option", w: "" } ] },
    { id: "W-Q4", q: "Links in grid cells: a link role in the paint vocabulary?",
      ctx: "DataGridViewLinkColumn needs to paint a cell as a link. The paint vocabulary has no link role; LinkLabel paints its colours directly. The vocabulary is sealed, so adding to it is your call.",
      opts: [
        { t: "Add a link role with normal, hover, visited and disabled states; LinkLabel moves onto it too", w: "Themes can then style every link in one place. A small vocabulary addition after the seal.", rec: true },
        { t: "The link column paints directly, as LinkLabel does", w: "No vocabulary change, but links can't be themed." },
        { t: "Leave the link column out for now", w: "" } ] } ] }
];
