# ADR-003: Use one deterministic event order and preserve DML assignment order

Status: accepted

Date: 2026-08-04

Owner approval: grand architect selected pragmatic blocker analysis, one
GUI.Forms order, and preserved DML order.

## GIVEN constraints

- GUI.Forms behavior is deterministic and retained.
- Compatibility does not require preserving every WinForms bug or alternate
  historical order.
- DML is authoritative input to retained construction.

## Workloads and failure modes

Observable ordering includes property changes, synchronous events, focus,
capture, activation, layout invalidation, disposal, and designer-style staged
initialization. Multiple context-dependent orders make traces unstable and
application reasoning difficult. Reordering DML assignments can change event,
layout, resource, or initialization behavior even when final values match.

## Candidates

1. Reproduce multiple consumer/platform orders.
2. Specify one deterministic GUI.Forms order and compare captured retired compatibility specimen paths for
   concrete blockers.
3. Select correctness-first order without inspecting the compatibility corpus.

For DML, either preserve authored property-assignment order or permit compiler
reordering after a claimed independence proof.

## Evidence and measurements

**OBSERVED:** existing lifecycle and host traces already use one synchronous,
deterministic order. **OBSERVED:** Forms designer initialization is an ordered
imperative sequence with batching scopes and observable hooks.

## Decision

**DECIDED:** select event candidate 2. GUI.Forms publishes and tests one order.
Captured retired compatibility specimen paths are examined pragmatically; a concrete blocker may change
the specified order through a reviewed compatibility change, but no second
runtime order is introduced.

**DECIDED:** DML preserves authored initialization assignment order. A compiler
may coalesce downstream rendering/layout work at transaction boundaries, but it
may not reorder authored assignments.

## Why the other candidates lost

Candidate 1 undermines determinism and multiplies the conformance surface.
Candidate 3 can miss a known application blocker. DML reordering confuses final-
state equivalence with observable construction equivalence.

## Consequences

- Every control family receives a normative trace, not a set of selectable
  ordering modes.
- Compatibility deviations name the blocker and the chosen invariant.
- DML generators and round-trip tools retain property order.

## Reversal and migration path

Changing a published order requires a versioned trace update and compatibility
analysis. If future declarative optimization is needed, it may optimize work
scheduled by assignments, not the assignment sequence itself.

## Unresolved edges

- The normative order for control families not yet implemented.
- The threshold for classifying an retired compatibility specimen difference as a blocking incompatibility.

## Amendment 2026-11-12: input event order

**DECIDED (owner, 2026-11-12):** input events follow six rules, chosen for
fewer bugs over matching Windows Forms. They replace Windows Forms' order
(MouseUp after Click and MouseClick) as GUI.Forms' one published order.

1. **Physical first, in causal order.** Pointer: preview, MouseDown, any
   MouseMove, MouseUp. Keys: PreviewKeyDown, KeyDown, KeyPress, KeyUp.
2. **Derived events after the completing physical event, on the same target
   only.** Click and MouseClick (or DoubleClick and MouseDoubleClick) follow
   MouseUp, and only on the control the same button pressed; KeyPress
   follows KeyDown.
3. **Handled cancels derived events.** A MouseUp left handled cancels Click,
   MouseClick and the activation (and DoubleClick and MouseDoubleClick); a
   KeyDown left handled, by the window or the control, cancels the KeyPress
   and the key's text, as SuppressKeyPress does. An input method's commit
   while a composition is open is not cancelled. A control that activates
   on its click therefore leaves its own release unhandled.
4. **State is already updated.** At MouseUp the button is up and the capture
   released; MouseCaptureChanged is raised at the end of the sequence.
5. **A broken sequence drops the rest.** When a handler disposes the
   control, opens a modal window or a modal layer or menu (a focus scope),
   or the capture is lost before the release, the remaining derived events
   of that sequence are dropped.
6. **Recorded.** The order of each interaction sequence is published in the
   event catalogue's order data (`manifests/abi/interaction_order.json` in
   gui_forms, written by `tools/abi_event_catalog.py`).

**DECIDED:** the second press of a double-click raises MouseDown, MouseUp,
DoubleClick and MouseDoubleClick only: no Click, no MouseClick and no
activation. A full double-click is therefore down, up, Click, MouseClick,
down, up, DoubleClick, MouseDoubleClick. A control without
standard_double_click (Button) raises a second Click instead.

Keyboard activation (Space or Enter on a button) raises the button's Click
after KeyUp, with no mouse events.

**Consequence:** the C# package's OnMouseUp, OnClick, OnMouseClick,
OnDoubleClick, OnMouseDoubleClick and OnKeyDown documentation, and the
Windows Forms conversion tool, state the difference from Windows Forms. The
normative traces that placed MouseUp after the clicks move under rules 1 and
2.
