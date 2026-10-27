const GROUPS = [
  { key: "A", title: "Rich text library (stx_rtf) — SwiftEdit's plan, PR #344", blurb: "The RTF-to-markdown library inside gui_forms that RichTextBox and SwiftEdit share. RT1 (model and markdown) waits on questions 1, 2 and 6.", questions: [
    { id: "RT-Q1", q: "The library's name?", ctx: "",
      opts: [ { t: "stx_rtf", w: "Matches the stx_* family; RTF is the format it adds.", rec: true }, { t: "stx_richtext", w: "" } ] },
    { id: "RT-Q2", q: "How does the markdown spell underline, colour, size and alignment?", ctx: "Plain markdown has no underline, colour, size or alignment.",
      opts: [ { t: "Attribute spans [text]{u color=#c00000} and block lines {align=center}", w: "One rule for every attribute; other markdown renderers show plain text.", rec: true }, { t: "HTML-like tags such as <u>", w: "Familiar, but the dialect otherwise excludes HTML." } ] },
    { id: "RT-Q3", q: "Code pages in the first version?", ctx: "Word, WordPad and TextEdit write Unicode escapes for non-Latin text, so most files never need a code page.",
      opts: [ { t: "1252 only; another code page is refused only when a byte actually needs it", w: "", rec: true }, { t: "All single-byte Windows code pages 1250 to 1258, from generated tables", w: "" } ] },
    { id: "RT-Q4", q: "Test corpus?", ctx: "",
      opts: [ { t: "First-party authored files now; real saves from Word or TextEdit later, when you ask", w: "", rec: true }, { t: "You save a small set of documents from programs on your machines now", w: "" } ] },
    { id: "RT-Q5", q: "What does SwiftEdit do with RTF?", ctx: "SwiftEdit's objectives of 2026-09-30 exclude an RTF editor.",
      opts: [ { t: "Import only: an RTF file opens as a new untitled Markdown document", w: "Keeps SwiftEdit's exclusion; RichTextBox still writes RTF.", rec: true }, { t: "Import, and Save As RTF", w: "" } ] },
    { id: "RT-Q6", q: "Superscript, subscript and background colour?", ctx: "Beyond W's original list.",
      opts: [ { t: "Admit them", w: "", rec: true }, { t: "Leave them out; imports count them as dropped", w: "" } ] } ] },

  { key: "B", title: "Windows Forms controls (W)", blurb: "Two calls that hold the link column and owner drawing.", questions: [
    { id: "W-Q5", q: "How is a visited link represented in the paint vocabulary?", ctx: "The vocabulary picks a look by control family and by state (normal, hover, pressed, disabled and so on). It has no visited state.",
      opts: [ { t: "Two families, link and visited_link; the link's visited flag picks which one", w: "Nothing else in the vocabulary changes; themes name the two families. Hover keeps the normal link colour, as today.", rec: true },
              { t: "A new visited state in every control family", w: "More literal, but every family and theme grows for a state only links use, and it touches T3's selectors." },
              { t: "One link family that reuses selected as visited", w: "Smallest, but misleading." } ] },
    { id: "W-Q6", q: "Owner drawing (WinForms OwnerDraw) for ListView, TabControl, ListBox, ComboBox, CheckedListBox?", ctx: "Nothing like it exists yet.",
      opts: [ { t: "The program draws only the item's content, with our painter; plate, selection and focus stay theme-drawn", w: "Themes and accessibility keep working; designed once as a shared seam, in its own PR before any control adopts it.", rec: true },
              { t: "Full WinForms owner draw, selection and focus included", w: "Closest to WinForms; themes can't style those items." },
              { t: "No owner draw; customise through themes and item templates", w: "" } ] } ] },

  { key: "C", title: "Theme language (T3) confirmations", blurb: "Choices the T3 worker made conservatively while building P3 to P6; it has built them as recorded and will change any you reverse.", questions: [
    { id: "T3-C1", q: "Limits from :is() expansion and lowering?", ctx: "E510: at most 32 simple selectors in one expanded selector. E513: a lowered sheet of at most 16 MiB (estimated per record, string and instruction).",
      opts: [ { t: "Keep both limits; exceeding either is an error and the theme refuses to load", w: "", rec: true }, { t: "Widen them", w: "" } ] },
    { id: "T3-C2", q: "Cascade level of an expanded selector?", ctx: "button:is(.a, #b) expands into two selectors with different specificities.",
      opts: [ { t: "Each expanded selector keeps the specificity :is() gives it (its most specific argument), as in CSS", w: "", rec: true }, { t: "Each expanded selector uses its own argument's specificity", w: "Differs from CSS." } ] },
    { id: "T3-C3", q: "Two new warnings, W108 and W109 (budgets)?", ctx: "They warn when a sheet approaches its rule and byte budgets, appended to the W1xx list.",
      opts: [ { t: "Add them", w: "", rec: true }, { t: "No budget warnings; only the errors at the limit", w: "" } ] },
    { id: "T3-C4", q: "A sheet that declares a newer engine than the running one?", ctx: "",
      opts: [ { t: "Load it at the running version: what this engine understands applies, and the rest is warned", w: "Matches your round-2 answer that unknown keywords warn under a newer engine.", rec: true }, { t: "Refuse the whole sheet and fall back to the previous theme", w: "" } ] },
    { id: "T3-C5", q: "A material's default look?", ctx: "When a rule sets material fields without naming a recipe.",
      opts: [ { t: "The part's own family recipe (engine version 0.3)", w: "", rec: true }, { t: "A neutral material recipe", w: "" } ] },
    { id: "T3-C6", q: "background-color and background-image in different rules?", ctx: "",
      opts: [ { t: "Fill layers cascade as one field: the later rule's fill wins outright", w: "Predictable; the engine's fill is one layered value.", rec: true }, { t: "Combine them as CSS does (colour under image)", w: "" } ] },
    { id: "T3-C7", q: "A rule with material fields but no primitive?", ctx: "",
      opts: [ { t: "Selects the material primitive implicitly", w: "", rec: true }, { t: "An error: the rule must name its primitive", w: "" } ] } ] }
];
