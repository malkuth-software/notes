# ADR-016: Animation runs on the motion clock and settling timers; active surfaces are retired

Status: accepted (owner, 2026-10-31); not yet implemented

Date: 2026-10-31

## Context

`Window::activate_surface(control, interval, first)` calls the control's
`on_frame(FrameTime)` every interval, and after each call the scheduler
invalidates the control's whole face unless the control already damaged itself
(`src/core/window/scheduler/window_scheduler.cpp`, around lines 239-273).
`on_frame` returns nothing, so a control cannot say "nothing changed", and a
control that forgets to damage only what moved repaints entirely on every
tick. The 2026-10-31 hot-paths measurement traced most animating CPU to this:
ProgressBars as 16 ms surfaces at 48 paints a second, the inspector overlay
repainting the window four times a second, EasingPreview damaging its whole
face.

Earlier decisions point one way:

- The owner, 2026-10-09 (PlaySuite session "waterfall"): "lets be careful
  about falling into a pit where the work to determine what to redraw doesnt
  meet or exceed the amount of work we have to do to begin with", and that it
  is "a problem of who controls timing- the parent program or the child
  runtime". A requested "nothing changed" signal (`LiveSurface::publish_unchanged`)
  was withdrawn as extra counter bookkeeping; no such API exists.
- PlaySuite never uses active surfaces: each view drives a plain timer,
  `advance()` reports whether anything still moves, the view draws only when
  dirty, and the timer stops once settled (its kit contract test requires a
  settled window to have no wake, or one at least 900 ms away).
- gui_forms #201 (T2 E3, the motion clock) moved the docking stamp off its
  active-surface loop; the T2 plan names `activate_surface` and timer loops
  as the pattern being replaced; the 2026-10-31 inspector fix replaced its
  active surfaces with a UI-timer poll.
- ADR-015 point 3: animation damages only what moves.

## Decision

Option 3 of the three considered (owner, 2026-10-31):

1. **Retire `activate_surface`.** Animation runs on control-owned
   `MotionValue`s and the window's motion clock, which holds a frame request
   only while some value is unsettled, or on UI timers that the control stops
   when it settles.
2. **Each control damages what it changes,** explicitly, as TextBox's caret
   already does. The scheduler's implicit whole-face invalidation after a tick
   goes away with active surfaces.
3. **No "changed" return value or unchanged counter** is added to `on_frame`
   (option 1, rejected as the bookkeeping already turned down).

## Consequences

- The stock users (ProgressBar, EasingPreview, the instrument and VSync labs,
  any remaining `activate_surface` callers) migrate to the motion clock or a
  settling timer with explicit damage; a settled window holds no wake.
- `activate_surface` and `on_frame` are public API: their removal goes through
  the deprecation path and the porting guide, and the C ABI and generated
  facade follow.
- The documentation (with ADR-015) describes animation as motion values or
  settling timers that damage only what moves.
- Rectangle damage still re-records the control's whole display chunk;
  ADR-015's prepared text keeps that from re-shaping text.
