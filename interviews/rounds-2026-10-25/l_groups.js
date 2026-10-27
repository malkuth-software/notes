const GROUPS = [
  { key: "A", title: "The shape of layout", blurb: "Stage L adds real dynamic layout: flex, grid and flow, the CSS models, decided in your words as “an engine task”. Layout stays out of themes. The plan is in planning/stages/L-layout/PLAN.md (PR #322).", questions: [
    { id: "L-Q1", q: "One container per layout model, or one container with a mode?",
      ctx: "",
      opts: [
        { t: "Three typed containers over one shared solver: FlexLayoutPanel, GridLayoutPanel, BlockFlowPanel", w: "Each has its own child properties and a typed API (a grid child has a row and column, a flex child has grow and shrink), and form documents name the container kind.", rec: true },
        { t: "One LayoutPanel with a display mode", w: "Closer to CSS's display property, but every child then carries every mode's properties." },
        { t: "Extend FlowLayoutPanel and TableLayoutPanel in place", w: "No new containers, but those two keep their WinForms limits." } ] },
    { id: "L-Q2", q: "What happens to WinForms' FlowLayoutPanel and TableLayoutPanel?",
      ctx: "",
      opts: [
        { t: "Keep their behaviour; rebuild their insides on the new solvers wherever their existing tests pass unchanged", w: "One layout engine underneath, with WinForms behaviour preserved.", rec: true },
        { t: "Leave them untouched", w: "Two layout engines to maintain." },
        { t: "Deprecate them in favour of the new containers", w: "" } ] },
    { id: "L-Q7", q: "Right-to-left?",
      ctx: "",
      opts: [
        { t: "An inherited layout direction on every control", w: "Used by the new containers first, then by the existing containers and Dock/Anchor in phase L6, so a whole form flips at once.", rec: true },
        { t: "A property on each container only", w: "" } ] },
    { id: "L-Q8", q: "Display scale and pixel snapping?",
      ctx: "",
      opts: [
        { t: "Layout is independent of display scale; edges are snapped to pixels only when painting", w: "A form lays out identically at 100% and 150%; only the final edges round to device pixels.", rec: true },
        { t: "Snap to device pixels during layout", w: "Crisper sizes at each scale, but layout then differs between scales." } ] },
    { id: "L-Q14", q: "Units in layout values?",
      ctx: "For themes (T3) you chose em relative to the text-role size. Layout can follow the same rule or keep it simpler.",
      opts: [
        { t: "px, %, fr, the content keywords and linear calc(); em resolved to px when importing; min(), max(), clamp() later", w: "Simplest first version; an imported HTML layout's em becomes a fixed size.", rec: true },
        { t: "Also em, relative to the box's text-role size", w: "Layouts grow with large text, matching your T3 choice for themes. More work in the solver." },
        { t: "Also min(), max() and clamp() now", w: "" } ] },
    { id: "L-Q10", q: "May CI download Chrome for Testing as the layout oracle?",
      ctx: "The gate compares our layout boxes with Chromium's. Chromium is a test oracle only, never used by GUI.Forms itself.",
      opts: [
        { t: "Approve chrome-headless-shell-linux64.zip 155.0.8059.39 (124,203,329 bytes) from storage.googleapis.com/chrome-for-testing-public, for CI only", w: "Pinned by version and hash; only a Linux job uses it, to regenerate the checked-in references.", rec: true },
        { t: "Also approve the Windows package (124,737,213 bytes) for local runs on this machine", w: "" },
        { t: "Neither yet: L0 lands the tools and fixtures without references", w: "" } ] },
    { id: "L-Q11", q: "Where do the reference boxes come from on each run?",
      ctx: "",
      opts: [
        { t: "Checked into the repository; every host compares against them without Chromium; a Linux job regenerates and checks them, not required", w: "Fast and offline on every host; the Chromium job catches drift.", rec: true },
        { t: "Chromium on every CI host on every run", w: "Always fresh, but slow, and a download on every run." },
        { t: "Checked in, with the regenerating job required", w: "" } ] } ] },

  { key: "B", title: "CSS fidelity calls (I'd accept the recommendations)", blurb: "How closely each container follows CSS. The plan's recommendations favour CSS behaviour where HTML import (stage H) needs it, and GUI.Forms' ownership rules where they differ.", questions: [
    { id: "L-Q3", q: "Flex items' automatic minimum size?",
      ctx: "",
      opts: [
        { t: "As CSS: an item's minimum is its content's minimum unless min-width or min-height says otherwise", w: "", rec: true },
        { t: "A GUI.Forms rule: minimum zero", w: "" } ] },
    { id: "L-Q4", q: "Block margin collapsing?",
      ctx: "",
      opts: [
        { t: "CSS 2.1 rules, without floats", w: "Siblings, parent with first and last child, and empty blocks; needed for faithful HTML import.", rec: true },
        { t: "Siblings only", w: "" },
        { t: "None; the importer folds margins", w: "" } ] },
    { id: "L-Q5", q: "Floats?",
      ctx: "",
      opts: [
        { t: "Left out of L; HTML import reports them as unsupported", w: "", rec: true },
        { t: "Included in phase L5", w: "" } ] },
    { id: "L-Q6", q: "What does an absolutely positioned child position against?",
      ctx: "",
      opts: [
        { t: "Its direct parent container", w: "A parent owns its children's slots; the difference from CSS is recorded where an imported source relies on a farther ancestor.", rec: true },
        { t: "The nearest positioned ancestor, as CSS", w: "Lets a control place a grandchild." } ] },
    { id: "L-Q9", q: "Fixture format and tolerance?",
      ctx: "",
      opts: [
        { t: "HTML fragments read by both Chromium and our XML reader; 0.05 px per edge for boxes, 0.51 px per edge and 1.0 px baseline for text", w: "", rec: true },
        { t: "The same, with one tolerance of 0.51 px for everything", w: "" },
        { t: "Wait for form documents and write fixtures as those", w: "" } ] },
    { id: "L-Q12", q: "Virtualisation for huge item counts?",
      ctx: "",
      opts: [
        { t: "The solvers lay out records; item systems realise only visible items; no new virtualising container", w: "", rec: true },
        { t: "Add a virtualising repeater container", w: "" } ] },
    { id: "L-Q13", q: "The docking split solver?",
      ctx: "",
      opts: [
        { t: "Keep it as it is", w: "A stored ratio with proportional fallback is not flex.", rec: true },
        { t: "Replace it with a two-item flex line", w: "" } ] },
    { id: "L-Q15", q: "Grid naming?",
      ctx: "",
      opts: [
        { t: "grid-template-areas and grid-area in phase L4; named lines left out", w: "", rec: true },
        { t: "Neither", w: "" },
        { t: "Both", w: "" } ] } ] }
];
