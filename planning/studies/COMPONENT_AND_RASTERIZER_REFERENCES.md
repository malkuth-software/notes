# Study: component, visual and rasterizer references (Dear ImGui, BUI)

Opened 2026-11-05 by the owner. Not started; nothing below has been read or
measured yet. What we take from these projects: components, visual
capabilities and performance targets. Their programming models are not on
the table; GUI.Forms' retained model is unchanged.

## Sources

- **Dear ImGui** (ocornut/imgui): its component set and its demo. Its
  documentation shape is already used in
  `planning/POLISH_STAGE_PLAN_2026-10-11.md` (section C).
- **BUI** (https://github.com/ppmpreetham/BUI), added by the owner
  2026-11-05. To read for:
  1. its components (like Dear ImGui, many to learn from);
  2. its CSS support;
  3. its image handling;
  4. theme capabilities the owner wants ours to have: rounding, liquid
     filling, and the like;
  5. a published claim (a competitive tweet) that its software rasterizer
     scrolls an intensive stress test smoothly and quickly: a log or chat
     history. **The benchmark is what we want**: reproduce that workload as
     a GUI.Forms benchmark on our CPU renderer and record our numbers
     against it.

## To do when the study starts

- Component inventory from both, mapped against our W rows and controls;
  list what we lack.
- BUI's CSS subset compared with our theme language; BUI's image path
  compared with `src/core/image/` (PNG only on our side).
- Theme effects: rounding and liquid fill as theme-language features or
  effect programs (bounded cost, verified by the effect pass).
- The log/chat scroll benchmark: find the tweet and BUI's own stress test,
  define the workload (line count, line lengths, scroll speed, window size),
  and measure GUI.Forms on the M4 with the workload recorded (no "fast"
  claim without it). Candidate home: beside the document view D1 benchmark
  (`experiments/DOCUMENT_VIEW_D1_*`).
- Licences of both before copying anything (BUI's not yet checked).
