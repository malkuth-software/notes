const GROUPS = [
  { key: "A", title: "What a theme says", blurb: "T2's engine already holds themes as rules: a selector, a cascade level, and declarations that name an engine primitive and set its parameters (fills, shadows, radii, shape, filters, motion). T3 is the text a person writes and the parser that turns it into those rules. These questions decide how that text reads.", questions: [
    { id: "T3-A1", q: "Property names: CSS's, the engine's, or both?",
      ctx: "The engine's parameters do not have CSS names. A rounded, shadowed button face is the material primitive with a fill, an outer shadow and corner radii. CSS_COVERAGE.md maps every CSS visual property the engine can carry onto those parameters.",
      opts: [
        { t: "CSS names, lowered onto engine parameters, plus engine names for what CSS lacks", w: "A theme writes background, border-radius, box-shadow, filter, transition and the rest the way any CSS author would. The parser lowers each onto the engine's parameters. Things CSS has no word for (shape: pill, finish, the glow placements, procedural materials) use engine-named properties. Anyone who knows CSS can read a theme, and nothing the engine can do is out of reach.", rec: true },
        { t: "Engine names only", w: "One property per engine parameter, so the text and the engine match exactly and the theme designer has nothing to translate. But a CSS author has to learn a new vocabulary, which loses the main reason for CSS syntax." },
        { t: "CSS names only", w: "Most familiar, but engine capabilities without a CSS property (finish, glow placement, procedural materials, shape) could not be written at all." } ] },
    { id: "T3-A2", q: "Shorthands such as background, border and transition?",
      ctx: "CSS lets one property set several: `border: 1px solid red`, `transition: scale 90ms ease`. The parser must also give back identical bytes for an untouched file (the round-trip rule).",
      opts: [
        { t: "Accept the common shorthands", w: "background, border, border-radius, box-shadow lists, transition and animation. Each expands to longhands inside the parser; the original text is kept for the round trip, so a hand-written shorthand survives untouched.", rec: true },
        { t: "Longhands only", w: "Simpler parser and one way to write each thing, but themes get long and read unlike ordinary CSS." } ] },
    { id: "T3-A3", q: "Which units?",
      ctx: "Themes set no layout (stage L owns layout), so units only size effects: radii, shadow offsets and blurs, keylines, translate distances.",
      opts: [
        { t: "Logical pixels, plus percentages where CSS defines them", w: "px means one logical unit, scaled by the engine for each display, as everywhere else in GUI.Forms. Percentages only where CSS gives them a meaning (gradient stops, radii). No em, rem or viewport units, because they tie effects to layout.", rec: true },
        { t: "Also em, relative to the part's text role size", w: "Lets an effect follow the text size, for example a focus ring that grows with large text. One more dependency between text and effects." } ] },
    { id: "T3-A4", q: "Colours: literal values, colour roles, or both?",
      ctx: "GUI.Forms colours come in relational roles (surface, ink, accent and their pairs), and high contrast replaces the roles. A literal like #3a6ea5 bypasses that.",
      opts: [
        { t: "Both, roles first", w: "role(accent) and its pairs are the normal way, so high contrast and dark variants keep working. Literals (hex, rgb(), hsl(), color-mix()) are allowed for decoration, and the diagnostics flag a literal used as text or edge colour, since forced high contrast cannot remap it.", rec: true },
        { t: "Roles only", w: "Every colour stays remappable, which is the safest for accessibility, but a theme cannot pick an exact decorative tint without adding a role." },
        { t: "Literals only", w: "Plain CSS, but high contrast and dark variants have nothing to remap." } ] },
    { id: "T3-A5", q: "Can a theme define its own variables?",
      ctx: "The scenario writes var(--face-light) and sets --glow. The engine's inputs (--pointer-x, --pointer-y, application values such as --level) are custom properties too, and a theme must never set those.",
      opts: [
        { t: "Yes, with the engine's inputs in a reserved namespace", w: "Themes may define --anything for their own reuse (a palette, a radius used everywhere). Engine inputs keep their names and are read-only: a theme that tries to set one gets a diagnostic and the declaration is dropped.", rec: true },
        { t: "No theme variables", w: "Only engine inputs exist as custom properties. Simpler, but every repeated value is written out, and palettes cannot be shared across rules." } ] } ] },

  { key: "B", title: "Selectors and errors", blurb: "Already decided: control type, stable ID (#id), class, ::part(name), :state(name) and the standard state pseudo-classes, with no combinators. The cascade is the engine's (built-in recipe, theme sheet, class rule, stable-ID rule, inline style, forced high contrast; within a level the more specific selector, then the later rule).", questions: [
    { id: "T3-B1", q: "Any selector forms beyond the decided set?",
      ctx: ":is() and :not() let one rule cover several states or exclude one, without combinators.",
      opts: [
        { t: "None in the first version", w: "The decided set covers every rule the default and Windows-familiar themes need. :is() and :not() can come later without breaking anything.", rec: true },
        { t: ":is() only", w: "Shortens rules that repeat across states, for example button:is(:hover, :focus)::part(face)." },
        { t: ":is() and :not()", w: "Most expressive; :not() makes specificity and the theme designer's view harder to reason about." } ] },
    { id: "T3-B2", q: "What does an invalid value drop?",
      ctx: "The brief says an invalid rule is dropped, not fatal. CSS itself drops only the bad declaration and keeps the rest of the rule.",
      opts: [
        { t: "Only the bad declaration", w: "As CSS does: one mistyped value loses one property, and the rest of the rule still applies. Each drop is a diagnostic naming the file, line and property.", rec: true },
        { t: "The whole rule", w: "Stricter, so a half-applied rule never shows. But one typo then removes a whole part's look." } ] },
    { id: "T3-B3", q: "A theme written for a newer engine?",
      ctx: "A theme declares the engine version it needs. On an older engine it may use a property or primitive that engine does not know.",
      opts: [
        { t: "Unknown names are dropped with a diagnostic; the rest applies", w: "A part keeps every declaration its engine understands, and falls back to the built-in recipe only where no primitive survives. Tools warn when a theme asks for a newer engine than the one running.", rec: true },
        { t: "Any part using an unknown name falls back whole", w: "A part never shows a partial look, but a theme meant for a newer engine shows mostly built-in recipes on an older one." } ] } ] },

  { key: "C", title: "States and motion", blurb: "The engine already runs transitions, settling primitives, in-between frames and the presentational state machine. These questions decide how a theme writes them.", questions: [
    { id: "T3-C1", q: "Can a theme declare states of its own?",
      ctx: "Today a theme keys its looks on the states controls publish (hover, pressed, checked and so on). A pop, for example, could want a short-lived 'risen' state between pressed and rest. T2 rules that theme states never become observable to the program and never delay behaviour.",
      opts: [
        { t: "Not in the first version: transitions and keyframes on published states only", w: "CSS transitions and @keyframes, keyed on the published states, cover the scenario (pop, sheen, glow). A theme-declared state machine can come later as its own rule kind.", rec: true },
        { t: "Yes, through an @-rule that declares presentational states", w: "More expressive motion now, at the cost of a second, non-CSS construct in the language and in the theme designer." } ] },
    { id: "T3-C2", q: "How are follow(), spring() and decay() written?",
      ctx: "They are the engine's settling primitives, for momentum and pop.",
      opts: [
        { t: "As timing functions in transition", w: "transition: scale spring(0.6, 12) reads like any easing curve, and lowers onto the engine's settling primitive. Durations stay in time, since they are what the user sees.", rec: true },
        { t: "As their own property", w: "motion: spring(...) beside transition. Keeps them apart from CSS easing, but a CSS author will not find them where they expect." } ] },
    { id: "T3-C3", q: "Reduced motion?",
      ctx: "The engine already turns motion primitives off when the system asks for reduced motion (a T2 gate).",
      opts: [
        { t: "Automatic, plus @media (prefers-reduced-motion) for themes that want a different calm look", w: "Every theme is safe without doing anything, and a theme can still choose, say, a cross-fade instead of a pop.", rec: true },
        { t: "Automatic only", w: "Nothing to write, and no theme can get it wrong, but no theme can choose its calmer look either." } ] } ] },

  { key: "D", title: "Variants and packs", blurb: "Already decided: a theme collection is immutable data with a manifest, a coverage table and content hashes; it can be swapped while a program runs; external packs need a content hash, not a signature.", questions: [
    { id: "T3-D1", q: "Light, dark, high contrast and display scale: one sheet or several?",
      ctx: "Dark is an admitted variant of the default theme; high contrast and display scale apply to every theme.",
      opts: [
        { t: "One sheet, with @media blocks", w: "@media (prefers-color-scheme: dark), (forced-colors: active) and (resolution) inside one sheet, as on the web. Shared rules are written once.", rec: true },
        { t: "Separate sheets per variant, chosen by the manifest", w: "Each variant reads on its own, but rules shared between variants are repeated or need imports." } ] },
    { id: "T3-D2", q: "Can a theme build on another?",
      ctx: "Plan Paint builds its look as overrides of windows-professional. A Windows-familiar theme may want the default theme's rules with a different surface.",
      opts: [
        { t: "Yes: @import of another installed theme by name", w: "A theme names a base theme and overrides what differs. The import is resolved within installed, hash-checked packs only: no paths, no network.", rec: true },
        { t: "No: every pack is complete", w: "No dependency between packs, but every variant repeats its whole base." } ] },
    { id: "T3-D3", q: "Where do the manifest and the engine version live?",
      ctx: "The manifest holds identity, version, compatible engine versions, coverage and content hashes.",
      opts: [
        { t: "A JSON manifest beside the CSS sheets", w: "Like the repository's other manifests: tools read it without parsing CSS, and the hashes cover the sheets themselves.", rec: true },
        { t: "An @theme block inside the main sheet", w: "One file per theme, but the hashes can no longer cover the file that declares them." } ] } ] },

  { key: "E", title: "Inline styles, limits and the parser", blurb: "How styles reach a single control, how hostile input is bounded, and who may call the parser.", questions: [
    { id: "T3-E1", q: "Where do inline styles come from?",
      ctx: "Your first idea was theming 'both by inline effects as well as by the use of a theme style sheet'. The cascade already has an inline level, just under forced high contrast.",
      opts: [
        { t: "Two sources, one parser: a C++ call on the control, and a style attribute in form documents", w: "control.set_style(\"::part(face) { ... }\") for programs, and style=\"...\" in FD documents, both parsed by the same T3 parser into the inline level.", rec: true },
        { t: "Form documents only", w: "Keeps styling out of program code, so a theme always wins outside forms; programs cannot style one control without a document." } ] },
    { id: "T3-E2", q: "Accept these limits for hostile input?",
      ctx: "Every theme is treated as hostile input: bounded sizes and counts, no recursion depth that input controls, and a fuzz target beside the PNG one. The proposed limits, each counted in effort or size, never in time:",
      opts: [
        { t: "1 MiB per sheet, 10,000 rules, 64 declarations per rule, nesting 32 deep, 4 KiB per string, 16 imports, expressions within T2's operation budget", w: "Generous for any real theme; the default theme is far under each. Hitting a limit drops what is past it, with a diagnostic, never a crash.", rec: true },
        { t: "Accept, with changes in the note", w: "Name the limit to change." } ] },
    { id: "T3-E3", q: "Is the parser public API?",
      ctx: "The theme designer (V) and interface builder (D) need to parse, edit and re-serialize theme text without clobbering hand edits.",
      opts: [
        { t: "Internal now, with a documented seam; public when V starts", w: "The parse, round-trip and structured-edit API settles against real use in V before it becomes a promise.", rec: true },
        { t: "Public from the start", w: "Tools outside the repository can use it now, but its shape is frozen before anything real uses it." } ] } ] }
];
