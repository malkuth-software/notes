# Study: immediate-mode toolkits as references (Dear ImGui, BUI)

Opened 2026-11-05 by the owner. Not started; nothing below has been read
or measured yet. GUI.Forms stays retained (AGENTS.md hard boundary): these
projects are studied for components, visuals and performance targets, never
for an immediate-mode API.

## Sources

- **Dear ImGui** (ocornut/imgui): the component set and the demo; already
  used for documentation shape in `planning/POLISH_STAGE_PLAN_2026-10-11.md`
  (section C).
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
     a GUI.Forms benchmark (CPU renderer, retained model) and record our
     numbers against it.

## To do when the study starts

- Component inventory from both, mapped against our W rows and controls;
  list what we lack.
- BUI's CSS subset compared with our theme language; BUI's image path
  compared with `src/core/image/` (PNG only on our side).
- Theme effects: rounding and liquid fill expressed as theme-language
  features or effect programs (EP rules: bounded cost, verified).
- The log/chat scroll benchmark: find the tweet and BUI's own stress test,
  define the workload (line count, line lengths, scroll speed, window
  size), and measure GUI.Forms on the M4 with the workload recorded
  (no "fast" claim without it). Candidate home: the Gallery's hot-path
  benches, beside the document view D1 benchmark
  (`experiments/DOCUMENT_VIEW_D1_*`).
- Licences of both before copying anything (BUI's not yet checked).
