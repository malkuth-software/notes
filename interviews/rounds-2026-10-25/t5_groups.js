const GROUPS = [
  { key: "A", title: "What applications and themes can ask for", blurb: "T5 lets a window show through to the desktop: plain transparency, or the system's frosted materials (Windows Mica and Acrylic, macOS visual effect views). The engine half already exists: glass inside a window blurs the window's own background. These questions decide what reaches the window system.", questions: [
    { id: "T5-Q1", q: "How does a theme name a backdrop?",
      ctx: "Each system has its own materials with its own names.",
      opts: [
        { t: "A small portable set: opaque, transparent, window_material, transient_material", w: "A theme asks for a purpose (a window's main surface, a short-lived popup), and each host maps it to its own material: Mica or Acrylic on Windows, a visual effect material on macOS. Themes stay portable, and a host without a material falls back.", rec: true },
        { t: "Platform materials by name", w: "Mica, Acrylic and the NSVisualEffectMaterial names, each reported as a capability. Exact control, but a theme then has to be written per platform." },
        { t: "Only opaque and transparent", w: "No system materials at all; simplest." } ] },
    { id: "T5-Q3", q: "Who asks for a backdrop?",
      ctx: "",
      opts: [
        { t: "Both: the application allows a window to be non-opaque, and the theme chooses the backdrop", w: "A theme can't make a program's window see-through unless the program allowed it, and a program doesn't hard-code a material.", rec: true },
        { t: "The application alone", w: "Themes can't change backdrops." },
        { t: "The theme alone, for any window", w: "A theme could make any program's windows transparent, whether or not the program expects it." } ] },
    { id: "T5-Q5", q: "What happens when a backdrop isn't available?",
      ctx: "For example Mica on Windows 10, or no compositor running on X11.",
      opts: [
        { t: "A declared fallback chain ending at opaque, drawn by GUI.Forms", w: "Up to three entries; by default the preferred backdrop, then opaque. Transparent is used only when the chain lists it. GUI.Forms paints the opaque fallback itself, so the result looks the same everywhere instead of depending on each system's fallback colour.", rec: true },
        { t: "Always straight to opaque", w: "Simpler, no chain." },
        { t: "Leave it to the system's fallback colour", w: "Mica's solid colour, AppKit's opaque material. Looks native, but differs per system." } ] },
    { id: "T5-Q6", q: "Clicks over fully clear pixels?",
      ctx: "",
      opts: [
        { t: "The whole window receives input; no click-through in T5", w: "Predictable, and the same on every host.", rec: true },
        { t: "Offer click-through over clear pixels as a further capability", w: "For overlays and odd-shaped windows; differs a lot between hosts." } ] },
    { id: "T5-Q7", q: "Reading the system's accessibility settings?",
      ctx: "High contrast and reduced transparency should turn backdrops off. T4 later adds the dark preference.",
      opts: [
        { t: "T5 builds a general preference signal: high contrast and reduced transparency", w: "Fed into the presentation settings every window already has; T4 extends it. macOS 'increase contrast' is reported but doesn't switch on high contrast, because it's a milder setting.", rec: true },
        { t: "As above, and macOS 'increase contrast' counts as high contrast", w: "" },
        { t: "T5 reads only reduced transparency; high contrast waits for T4", w: "" } ] },
    { id: "T5-Q16", q: "In-window glass when the user asks for reduced transparency?",
      ctx: "Glass and reflections inside a window come from the engine (E9.3), not the window system.",
      opts: [
        { t: "Glass flattened to its tint over the background colour; reflections omitted", w: "Opaque and unblurred, but still recognisably the same design.", rec: true },
        { t: "Unchanged; only the behind-window backdrop is withdrawn", w: "" },
        { t: "Glass replaced by the part's built-in recipe", w: "" } ] },
    { id: "T5-Q14", q: "What does a reflection reflect?",
      ctx: "",
      opts: [
        { t: "The part's own engine raster only: face, material, glow; not its text", w: "Cheap and cached like the rest of the engine; reflected text tends to look wrong anyway.", rec: true },
        { t: "The part's whole content, text included", w: "Needs the background replay for every reflection." } ] },
    { id: "T5-Q15", q: "How far can a reflection reach?",
      ctx: "",
      opts: [
        { t: "8 logical units, the same cap as blur and glow", w: "", rec: true },
        { t: "16 logical units", w: "" },
        { t: "Only the part's granted overflow, no further cap", w: "" } ] } ] },

  { key: "B", title: "Platform and engineering calls (I'd accept the recommendations)", blurb: "Smaller choices with a clear best answer. Change any you disagree with.", questions: [
    { id: "T5-Q2", q: "Undocumented Windows interfaces?",
      ctx: "Windows 10's accent policy and Windows 11 21H2's attribute 1029 give blur on older Windows, but they aren't documented.",
      opts: [
        { t: "Documented interfaces only", w: "Windows 10 gets transparent; system materials from Windows 11 22H2.", rec: true },
        { t: "Admit them behind version checks", w: "More reach, but they can change or vanish with an update." } ] },
    { id: "T5-Q4", q: "When can a window become non-opaque?",
      ctx: "",
      opts: [
        { t: "Declared when the window is created; only the theme's grant changes at run time", w: "Some systems need a different native window for transparency (an X11 visual, Windows styles).", rec: true },
        { t: "At any time, recreating the native window where needed", w: "" } ] },
    { id: "T5-Q8", q: "How does a theme see whether a backdrop was granted?",
      ctx: "",
      opts: [
        { t: "Through T3's @media, per window", w: "Including CSS's own prefers-reduced-transparency and a GUI.Forms feature for the granted backdrop. A window-level theme record holds the backdrop asked for.", rec: true },
        { t: "A new environment state atom", w: "A vocabulary change." },
        { t: "Both", w: "" } ] },
    { id: "T5-Q9", q: "X11 libraries?",
      ctx: "",
      opts: [
        { t: "Link libXfixes and libXrender", w: "Both ship with every X desktop; two more runtime dependencies in the .deb.", rec: true },
        { t: "No new libraries", w: "Poll the compositor and assume the visual's alpha layout." } ] },
    { id: "T5-Q10", q: "Blur behind on X11?",
      ctx: "",
      opts: [
        { t: "None in T5: X11 offers transparent only", w: "", rec: true },
        { t: "KWin's blur hint when KWin is detected", w: "" } ] },
    { id: "T5-Q11", q: "macOS 26's new glass effect view?",
      ctx: "",
      opts: [
        { t: "Not in T5", w: "Revisit when the deployment target makes it worthwhile.", rec: true },
        { t: "Map window_material to it on macOS 26 and later", w: "" } ] },
    { id: "T5-Q12", q: "A compositor in Linux CI?",
      ctx: "",
      opts: [
        { t: "Install xcompmgr in the Linux CI job; the gate test starts and stops it under Xvfb", w: "Both states recorded on every run.", rec: true },
        { t: "Record the with-compositor state only on a desktop machine", w: "" } ] },
    { id: "T5-Q13", q: "Where does the background replay live?",
      ctx: "Full-fidelity glass needs the window's background re-rendered, including text.",
      opts: [
        { t: "A renderer service every renderer implements, reported in its capabilities", w: "", rec: true },
        { t: "A host capability bit", w: "" } ] } ] }
];
