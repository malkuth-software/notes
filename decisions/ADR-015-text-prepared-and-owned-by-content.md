# ADR-015: Text is prepared ahead of painting and owned by its content

Status: accepted (owner, 2026-10-31); not yet implemented for the stock
controls

Date: 2026-10-31

## Context

The hot-paths measurement of 2026-10-31 (notes branch
`claude/hot-paths-2026-10-31`, `results/2026-10-31-hot-paths/README.md`)
found the Gallery's animation laboratory at 20.6 % CPU while animating, half of
it re-shaping and re-rasterizing text that had not changed. Stock controls keep
shaped runs and glyph coverage in caches attached to recorded display
commands. When an animating control's display chunk is re-recorded, the static
text recorded in the same chunk loses its caches and is shaped and rasterized
again every frame.

SwiftEdit already avoids this through GUI.Forms' own prepared-text model
(`include/gui_forms/prepared_text/`: `PreparedTextSession`,
`PreparedTextKey`). Its text is shaped ahead of painting into retained
geometry keyed by everything that affects it; painting only draws that
geometry, and caret scrolling reuses it. Its storage is explicitly budgeted
(swiftedit `docs/PREPARED_SHAPING_STORAGE_DECISION.md`, 2026-10-03: 8 MiB of
retained shaped payload, 16 MiB of controlled workspace, ceilings of 16,384
runs and 65,536 glyphs, every allocation checked before it is made, no
partial publication). Its focused idle editor paints only at caret-blink
deadlines and damages only the caret (swiftedit
`docs/performance/2026-10-01-mac-idle/README.md`: 9 paints, deadlines and
wakes in 5 s, 342 px each; 0 with focus cleared).

## Decision

1. **Shaped text belongs to its content, not to the drawing record.** Stock
   controls move onto the prepared-text model. Shaped runs and glyph coverage
   are prepared ahead of painting, keyed by what affects them (text, face
   identity and generation, typography, scale and subpixel phase, wrapping,
   tabs, raster profile), and owned by the control or its window. Rebuilding
   a display chunk never re-shapes or re-rasterizes text whose key is
   unchanged.
2. **The storage is budgeted and accounted.** SwiftEdit's numbers are the
   starting point (8 MiB retained, 16 MiB workspace per window, the run and
   glyph ceilings), with checked accounting and no partial publication, until
   GUI.Forms measurements justify different ones.
3. **Animation damages only what moves.** An animating control damages only
   the area that changed, on the deadlines it needs, rather than repainting
   its whole face every tick. How a control reports that a frame changed
   nothing is decided separately (the `on_frame` question, under research
   2026-10-31).
4. **Documentation carries the model.** The user and developer documentation
   describes the model (prepared text, its keys and budgets, damage-limited
   animation on deadlines) as how GUI.Forms programs are expected to behave,
   not as an optimization detail.

## Consequences

- The animation laboratory's static text, and any text re-recorded by
  scrolling, re-layout or theme switches, stops being re-shaped.
- Stock-control text paths change, so pixel identity must be checked: the
  prepared path must draw the same pixels as today's path.
- Memory for shaped text becomes visible and bounded instead of riding on
  display-command lifetimes.
