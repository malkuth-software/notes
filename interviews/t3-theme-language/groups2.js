const GROUPS = [
  { key: "A", title: "Decisions only you can make", blurb: "These shape what a theme author sees, or add vocabulary atoms. The plan (planning/stages/T3-theme-language/PLAN.md, PR #316) explains each in full.", questions: [
    { id: "T3-2-03", q: "Colour roles: what are they, and where does a theme's palette live?",
      ctx: "Round 1 chose colour roles first. No colour-role atom exists in the code yet; the design language §5.4 proposes a set of pairs (ink on paper, accent on surface and so on).",
      opts: [
        { t: "§5.4's pairs become colour-role atoms; the palette is a :root block in the sheet", w: "A theme writes ink-on-paper: #1b2733 on #ffffff; in :root. role(ink-on-paper) gives the ink in color and the paper in background, and role(ink-on-paper foreground) picks one side anywhere. The palette stays in the sheet the designer edits. Adding the atoms is a one-time vocabulary decision.", rec: true },
        { t: "The same atoms, with the palette in the manifest JSON", w: "The palette sits beside the sheets, not in them; tools read it without parsing CSS, but the designer edits two files." },
        { t: "No new atoms: role() reads only the recipe's existing colours", w: "text, muted-text, glyph, focus-ring, default-ring, accent(n). Nothing new to admit, but themes get only those few names." } ] },
    { id: "T3-2-02", q: "How does a rule carry a material (fills, borders, shadows)?",
      ctx: "Today the material primitive takes only a built-in recipe and a shape, so CSS backgrounds have nothing to land on. This is the biggest engine change T3 needs.",
      opts: [
        { t: "Each material field cascades on its own", w: "As CSS does: a :hover rule that changes only the background leaves the border and shadow from the base rule alone. The engine assembles each part's material from the winning fields, cached by the assembly.", rec: true },
        { t: "A rule carries a whole material", w: "Simpler engine change, but a :hover rule must restate every layer, which surprises every CSS author." },
        { t: "CSS writes into the theme's recipe table by family and state", w: "Reuses today's recipe table, but addresses whole recipe families, not individual parts." } ] },
    { id: "T3-2-04", q: "Can a selector address a part by its role?",
      ctx: "Appearance is bound to part role and state (design language §8.2), and the engine's selector already has a role field, but round 1's selector set has no way to write it. ::part() names a part by its declared name.",
      opts: [
        { t: "Yes, with ::role(name)", w: "::role(face) styles every part whose role is face, whatever its name. Explicit, and kept apart from ::part().", rec: true },
        { t: "::part() matches a name or a role", w: "One form, but ambiguous wherever a part's name is also another part's role." },
        { t: "Not in the first version", w: "Parts are addressed by name only." } ] },
    { id: "T3-2-10", q: "Inline styles: what can they contain, and what is the C++ call named?",
      ctx: "Round 1 admitted inline styles from C++ and from form documents.",
      opts: [
        { t: "Declarations plus optional part blocks; named set_inline_style()", w: "control.set_inline_style(\"background: role(accent); ::part(glyph) { ... }\"): declarations for the control, then blocks for its parts, with the control implied. The control owns the result, so it needs no stable ID and survives theme swaps. The name avoids clashing with the existing WinForms set_style(ControlStyles, bool).", rec: true },
        { t: "The same, named set_style()", w: "What you wrote in round 1; it overloads the WinForms-compatible set_style(ControlStyles, bool)." },
        { t: "Declarations only, applied to every part", w: "Simpler, but one control's parts cannot differ." } ] },
    { id: "T3-2-07", q: "Engine-named properties: plain names or a prefix?",
      ctx: "Properties CSS lacks (shape, finish, glow placement and so on).",
      opts: [
        { t: "Plain names, versioned with the engine", w: "shape: circle, as in the scenario you saw. A keyword the engine lacks is an error, or a warning when the sheet declares a newer engine, so newer themes still load on older engines.", rec: true },
        { t: "A -gf- prefix on each", w: "-gf-shape: circle. Marks what is not CSS, but reads worse and differs from the agreed scenario." } ] },
    { id: "T3-2-01", q: "File extension and media type?",
      ctx: "Left open since 2026-10-13.",
      opts: [
        { t: ".theme.css sheets, theme.json manifest, the dialect named in the manifest", w: "Editors highlight .theme.css as CSS; the inner suffix says what it is; the manifest's format field (\"gui-forms-theme/1\") names the dialect and lists the sheets, so nothing is found by extension. text/css on the wire.", rec: true },
        { t: ".css and text/css", w: "Highlighted everywhere, but looks like browser CSS." },
        { t: ".gftheme and text/x.gui-forms.theme", w: "Unambiguous, but no editor highlights it." } ] },
    { id: "T3-2-18", q: "Transitions declared on one rule, states set on another?",
      ctx: "In CSS, `button { transition: scale 90ms }` animates a scale set by `.primary:hover { scale: 1.05 }`. The engine keeps a transition's states and curve in one record, so it can't yet join them across rules.",
      opts: [
        { t: "First version: a transition covers refinements of its own rule; later, an engine change if themes need more", w: "button { transition } animates button:hover and button:active states. A state value set by an unrelated rule (.primary:hover) changes instantly, with a warning.", rec: true },
        { t: "Engine change now: cascade curves and state values separately and join them per part", w: "Full CSS behaviour from the start; one more engine phase before T3 lands." } ] } ] },

  { key: "B", title: "Technical calls (I'd accept the recommendations)", blurb: "Smaller choices with a clear best answer. Change any you disagree with; otherwise choosing the recommendation for each is fine.", questions: [
    { id: "T3-2-05", q: "Scope of a theme's own variables?",
      ctx: "",
      opts: [
        { t: "Global plus rule-local", w: "Defined in :root and used everywhere; a rule's own --x shadows the global one inside that rule. Full per-part CSS cascading later, only if a real theme needs it. The scenario's --glow is this kind.", rec: true },
        { t: "Global only", w: "Simplest; the scenario's per-rule --glow would not work." },
        { t: "Full CSS cascade per part", w: "Needs the engine to carry deferred values." } ] },
    { id: "T3-2-06", q: "How does a theme declare the application values it reads?",
      ctx: "Values such as --level that a program publishes.",
      opts: [
        { t: "Listed in the manifest", w: "Explicit and checked; var(--level) then reads one.", rec: true },
        { t: "A reserved prefix (--app-level)", w: "No manifest entry, but every name carries the prefix." },
        { t: "Any var() the theme doesn't define", w: "No declaration, but a typo silently becomes an input." } ] },
    { id: "T3-2-08", q: "Classifying cases the round-1 rule didn't name",
      ctx: "Round 1: unknown properties warn, bad values fail.",
      opts: [
        { t: "As the plan proposes", w: "An unknown control, part, state or pseudo-class is a warning that drops the rule (an unknown name). A known primitive on a part role it cannot paint is an error (a bad value). A primitive over the effort budget is a warning, since the author can't fix it and the part falls back. A property refused by decision (clip-path, rotate, layout) is a warning.", rec: true },
        { t: "Change some (note)", w: "Name which." } ] },
    { id: "T3-2-09", q: "Does a theme's !important survive forced high contrast?",
      ctx: "Round 1: high contrast wins on colour; inline !important is honoured for everything else.",
      opts: [
        { t: "No: only inline !important non-colour declarations survive", w: "Your guarantee is to the program designer's inline rules.", rec: true },
        { t: "Theme !important non-colour declarations survive too", w: "" } ] },
    { id: "T3-2-11", q: "Percentage radii?",
      ctx: "The engine draws circular corners only.",
      opts: [
        { t: "A percentage of the shorter side", w: "50% on any box is a pill, a stated difference from CSS's ellipse; shape keywords cover circles and pills exactly.", rec: true },
        { t: "Percentage radii are a bad value", w: "" } ] },
    { id: "T3-2-12", q: "Hash for the manifest's content hashes?",
      ctx: "",
      opts: [
        { t: "SHA-256, first-party, with the standard test vectors", w: "Any tool can produce the manifest's hashes; no new dependency.", rec: true },
        { t: "BLAKE3", w: "Faster, less universal tooling." } ] },
    { id: "T3-2-13", q: "States declared by custom controls?",
      ctx: "Custom controls may declare their own states (T1), but the engine's selector holds only the built-in set.",
      opts: [
        { t: "Defer: a custom state is an unknown name (a warning) for now", w: "Consistent with round 1's 'no theme-declared states' first version.", rec: true },
        { t: "Extend the selector in this stage", w: "" } ] },
    { id: "T3-2-14", q: "The derived limits?",
      ctx: "32 :is() arguments, 1,024 custom properties, 4,096 tokens per substituted value, 64 @keyframes, 1,000 diagnostic records. Each exceeded limit is an error, like round 1's.",
      opts: [
        { t: "Accept as starting values", w: "", rec: true },
        { t: "Change some (note)", w: "" } ] },
    { id: "T3-2-15", q: "Selector lists (a, b { })?",
      ctx: "",
      opts: [
        { t: "Admit them, one rule per selector, each with its own specificity", w: "Every CSS author writes them.", rec: true },
        { t: "Refuse; :is() covers the need", w: "" } ] },
    { id: "T3-2-16", q: "Escapes?",
      ctx: "",
      opts: [
        { t: "Only inside strings; names are plain kebab-case ASCII", w: "Names are registered words; escapes in names only hide them.", rec: true },
        { t: "Full CSS escapes everywhere", w: "" } ] },
    { id: "T3-2-17", q: "ease-in and ease-out?",
      ctx: "The engine has no easing of those names.",
      opts: [
        { t: "Lower to their exact CSS cubic-bezier()", w: "Exact, and no engine change.", rec: true },
        { t: "The nearest built-in easing", w: "" } ] },
    { id: "T3-2-19", q: "Where is @media evaluated?",
      ctx: "",
      opts: [
        { t: "Per window: each rule carries its condition, re-resolved on a change", w: "Windows share a theme but not a display scale.", rec: true },
        { t: "One compiled rule set per environment, swapped", w: "" } ] },
    { id: "T3-2-20", q: "Counting the 16 imports?",
      ctx: "",
      opts: [
        { t: "Over a theme's whole import closure, each installed theme once", w: "Bounds the total work, which is the limit's purpose.", rec: true },
        { t: "Per sheet", w: "" } ] },
    { id: "T3-2-21", q: "Smaller proposals, as one set?",
      ctx: "Named colours other than transparent and currentcolor are not admitted; initial means the built-in recipe's value; isolation is accepted and changes nothing; non-default animation-direction and animation-fill-mode are errors in the first version; the diagnostic log line format; the inline-with-theme warning once per window.",
      opts: [
        { t: "Accept the set", w: "", rec: true },
        { t: "Accept, except items in the note", w: "" } ] } ] }
];
