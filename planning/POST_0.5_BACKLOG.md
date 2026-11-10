# Post-0.5 backlog (owner, 2026-11-08)

Right after the seal: rewrite the roadmap and decide how to stagger the next block toward 1.0. Each item below gets a full dissection: what we want from it, how we implement it, and how we test it comprehensively.

| Item | Source | Already noted? |
|---|---|---|
| Declarative dragging | https://declarative-dragging.github.io/ | **No.** Only drag mechanics are noted: CONTROL_SUPERSET_STUDY's reorder lists, autoscroll while dragging, PageView drag, drag-to-scrub. |
| Node-based engine in our own style (blueprint grid/texture background, magnetic curved links, double-click to edit) | https://github.com/ppmpreetham/BUI (to finish studying) | **Partly.** memory trajectory-to-1-0 ("node graph engine: blueprint grids and textures, magnetic curved links, double-click to edit"), CONTROL_SUPERSET_STUDY row 55 plus the node-graph demonstrator, EXHIBITS_PLAN. BUI itself isn't studied. |
| Dear ImGui study: which controls we want, which UI effects to emulate | imgui_demo.cpp | **Partly.** CONTROL_SUPERSET_STUDY lists it as a source and an "ImGui-class widgets" demonstrator, with drag-to-scrub numeric as P2. No effects study yet. |
| Blender-style percent/value bars for settings (the pieces needed for them) | Blender | **No.** |
| Declarative plotting/animation equivalent to ImPlot (sine waves, animation, 2D plots), plus the primitives needed to assemble others (which we won't make ourselves) | ImPlot | **Partly.** CONTROL_SUPERSET_STUDY: charts and plots (row 49), an ImPlot-class demonstrator, sparkline P1 as the first slice of D4 plotting, gauges. A declarative primitive set isn't designed yet. |

Also pending after 0.5:
- the theme designer discussion (owner);
- the tools' licence class (still open);
- the Mac handoff checks (Cmd-Q, logout, resize, C8 macOS rows);
- the control-level active border (ACTIVE_BORDER_STUDY);
- Splash refinements.

## The tools we will ship (owner, 2026-11-08)
1. **Theme editor.** The owner will describe how it should work.
2. **Visual WYSIWYG designer,** with in-line theme override capability.
3. **Visual Studio integration and package,** for VS and C#.
4. **WinForms "mirrored mode" launcher.** Windows only. It launches an existing WinForms program with GUI.Forms in place of WinForms: calls to WinForms are routed to us, and anything unsupported is captured and logged.
   - For us, it means running free or demo WinForms programs to dogfood their interfaces and gather unsupported logic without disassembling anything.
   - For users, it means a swap-in framework for some programs that needs no compiling, but isn't guaranteed to work everywhere.
   - Feasibility is still to be studied. Candidate mechanisms: a replacement System.Windows.Forms facade assembly with binding redirects (.NET Framework), DOTNET_STARTUP_HOOKS or an assembly-load hook (.NET), or method interception. Related: memory winforms-compat-boundary and dotnet-debug-visibility.
   - The tools' licence class is still open.

## Window band (owner, 2026-11-08)
- No resize-edge role: frameless means frameless.
- Effects and textures on the band come after 0.5, because texturing lets it serve the many pet projects that use such borders now. Strobe or steady colour carries other meanings. For 0.5 the band has the built-in flash and steady colour only.

## Theme editor (owner, 2026-11-08)
It is two things.
- **A screencap utensil:** captures all controls, or specific pages of them, for reference work, in an intelligent and aesthetically correct layout.
- **A visual editor for humans,** with three facilities:
  1. Click a control and instantly see which rules affect it, highlighted.
  2. A Compile button above the interactive, highlighted rule text, which says whether the sheet is legal.
  3. The usual save, undo, redo, search and find on a small bar.

It is a new app on GUI.Forms, built on top of SwiftEdit.

## Product revisions that follow (owner, 2026-11-08)
- **SwiftEdit:** revised to the new standard and the new interface design. It picks up syntax highlighting, at least for CSS.
- **Plan Paint:**
  - Redone to demonstrate the canonical ribbon, staying as close to its current layout as possible but with the new soft, large bubble buttons.
  - More robust file handling. The owner couldn't open a `.PNG`, which may be case-sensitive extension matching.
  - Use the new stx_conv algorithm, which is faster.
- **PlaySuite:** migrated forward. It demonstrates custom skinning through themes and shows whether anything is missing to make it look exactly as it does now.

## WYSIWYG designer (owner, 2026-11-08)
- Drag and drop onto a canvas, with PCB-editor-like semantic control and Photoshop-style controls.
- Click to set the floor plan (the window), then place things on it.
- Flex and grid layouts show their grid while being adjusted, with snap to grid.
- In-line edits: right-click to set a rule. On save, it asks whether to move the rule into a `.css`; if not, it gives a C++ inline block to insert in your code.

## C ABI band declaration
C and C# programs can't declare a window band yet, because the windows table has no window presentation. That additive ABI work comes between 0.5 and 1.0, with the C# work (owner, 2026-11-08).

## Swap-in launcher: SDR# as the first real target (owner, 2026-11-08)
- SDRSharp uses custom controls, so they need custom mappings in a shim (spectrum/waterfall, gradient sliders and the rest).
- Use the latest release build. The owner can provide an older debug build if needed, but our SDRSharp notes should be enough (notes planning/stages/R-sdrsharp-reflection/, FORWARD_PROGRAMME R).
- It ships as a custom zip the owner can give Youssef Touil to demo GUI.Forms on SDR# on his own computer.
- Downloading the SDR# release needs the owner's approval at the time (file, source, size).
- It depends on the C# WinForms facade (S) covering SDR#'s used surface, plus the shim layer for its custom controls.

## Node canvas: records (2026-11-08)
Our own apps already draw items as records on one canvas control:
- PlaySuite's `Table` keeps 104 `Sprite` structs and child Buttons only for chrome;
- the puzzle and maze views work the same way;
- Plan Paint's spirograph is plain structs drawn by `Editor`.

So the node canvas uses records too, following the owner's suspicion and the study's recommendation. Green-red indicator lights as a style for node ports is an owner idea. GUI.Forms has no status-lamp part yet: one gets built as a theme part (port state on, off or error).
- **Owner, 2026-11-08: Plan Paint has a simple node engine, but no intelligent wiring.** The node canvas needs automatic link routing:
  - links that lay out their own paths around nodes (obstacle-avoiding connector routing, orthogonal or curved);
  - re-routing live while nodes are dragged;
  - nudging parallel links apart, crossing minimisation and bundling;
  - ports chosen by direction.

  That is the "dynamic intelligent flow/wiring" the owner described. The node-canvas design must include it; the precedent is libavoid-style connector routing.

## Large-list stress demonstrator (owner, 2026-11-09)

Inspired by https://x.com/ppmpreetham/status/2118388897130655885, a BUI demo: "10 Million messages,
280 MB. 1% CPU." It scrolls a chat list with usernames, avatars, timestamps and generated messages of
100-1000 characters. Build our own version as a GUI.Forms stress demonstrator:
- a virtualized list of 10M variable-height rich rows (avatar image, name, timestamp, wrapped text);
  smooth scrolling and jumps anywhere with bounded memory and near-idle CPU at rest;
- the measured figures recorded as the workload (house rule: nothing is "fast" without a recorded workload);
- later, fast search over the corpus without lag or crashing (a separate problem; that demo may not have had it).
Engine angle: our "really fast, really large scrolling and searchable text" features. The Pareto-optimal
B-trees we have lack branchless comparison, cache-optimized layout and SIMD. When optimizing, study
Sergey Slotin's *Algorithms for Modern Hardware* (algorithmica.org): Eytzinger and S-tree/B+ layouts,
branchless search, SIMD string and integer comparison. Clean-room as usual.
Timing: after the 1.0-critical work.
- Startup bar (same demo, owner 2026-11-09): tiny files open instantly, giant files within 100 ms of
  first paint. That demo virtualizes scrolling with Fenwick trees (prefix sums of row heights). Our
  likely approach: a height-indexed B-tree, with each node storing its subtree's total height and row
  count. That gives offset-to-row and row-to-offset in O(log n), O(log n) height updates when a row is
  re-measured, cheap bulk insert and delete (chat appends, edits), and estimated heights for rows not
  yet measured, refined lazily, so startup needs no full measuring pass. It shares the node layout
  work (branchless, cache-friendly, SIMD) with the search B-tree. Record startup and scroll figures
  against the 100 ms bar.

## Theme editor: completion hints (owner, 2026-11-09, later)

IntelliSense-like completion in the theme editor's code pane: as the author types, it suggests
completions, mostly property names, then the valid values after the `:` (enum keywords, palette
roles including app-declared roles, units, part names and states). Source the suggestions from the
theme language's own schema and the compiler's part/role tables (no separate hand-kept list), and
show them in a popup list anchored at the caret (relies on the menu/popup system fixes). Likely a
reusable SwiftEdit code-editor capability (completion provider interface), so SwiftEdit's other
languages can use it later. Timing: later.

## Automation via MCP (owner, 2026-11-10)

Move GUI.Forms automation (today the Windows WM_COPYDATA channel: snapshot, capture, click, key, text) toward an MCP integration, as Claude Desktop exposes one: a GUI.Forms MCP server that test harnesses and AI agents talk to, with explicit opt-in. Do not extend the WM_COPYDATA channel further. Timing: later; needs a short study (transport, auth, the scope of commands, macOS/Linux parity).
