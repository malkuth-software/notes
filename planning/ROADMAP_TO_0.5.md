# 0.5: the ABI and API seal

Date: 2026-11-08, from checkpoint 22 (`checkpoint/2026-11-08-cp22`,
1416c6d3). Status: what 0.5 is, what it seals, what remains before the seal
and what moves after it. `planning/FORWARD_PROGRAMME.md` stays authoritative
for scope and gates, and each stage brief under `planning/stages/` for its
own decisions. The item-by-item closure list, with effort units and owners,
is the coordinator's `CLOSURE_0.5.md` (projects repository,
`desks/coordinator/`). The 2026-10-27 version of this page, a remainder list
by release, is in git history.

## What 0.5 is

There are no 0.3 or 0.4 tags (owner, 2026-10-27): their content ships in
0.5, and the next release is v0.5.0. The owner defined 0.5 on 2026-11-03,
and widened it on 2026-11-07 and 2026-11-08, as an internal candidate that
finalises the ABI, the API and the languages, then touches them up and
records the seal. After the seal, a change to a sealed interface needs a
recorded owner decision.

## What is sealed

| Area | Contents | Stages | State at cp22 |
|---|---|---|---|
| The C ABI | version 1.1, installed, with an error model, the breaking-change audit against a recorded baseline, the conformance kit and C# interop; tables for the application (`GF_TABLE_APPLICATION`), rules (`GF_TABLE_RULES`), sub-surfaces and the splashscreen | C | built; the baseline is re-recorded at the seal |
| Properties as capabilities | every property a cell, a view or a collection cell: `(*c).text = "OK"` writes, `.read()` reads, `as_type` converts, child properties such as `(*grid).area[*c]`; setters for properties removed (M-S) | PC, W | in cp22; the residual setter audit (R1) is open |
| The application, windows and rules | `Application::create` and one current application whose theme compiles once; the generated window class bound at construction; window rules over a closed vocabulary | APP_WINDOW_RULES_DESIGN.md (coordinator desk) | in cp22 |
| Window kinds | Window, Popup and Splash: warm tooltip windows, menus and drop-downs on Popups, the splashscreen, the `transient_surfaces` capability | SPLASH_NOTES.md (coordinator desk) | in cp22 |
| The form language | `.form` documents, in-slot bindings, generated window and user-control classes, the refusals | FD | in cp22; the FD-7 gate record is a draft |
| The theme language and effects | selectors, specificity and origins, `:family()`, backdrops, user themes, effect programs | T3, EP | T3 gate recorded 2026-11-01; EP-1 to EP-8 in |
| Theming | the GUI.Forms theme with derived dark and high-contrast variants, theme variants, the four familiar themes, compositor cooperation | T4, T5 | in cp22; the T4 gate record is a draft; T5 C8 rows open |
| Sub-surfaces | threaded regions with their own input, effects and accessibility | SS | S1 to S7 in |
| The creation-cost contract | window creation does no hidden work, enforced by a benchmark gate | K (APP_WINDOW_RULES_DESIGN.md) | in cp22; baselines on the M4 only |
| Windows Forms coverage | every 2026-10-27 row, the small controls, the instruments vocabulary | W | in; W-1 to W-3 and W-5b to W-9 ruled after 0.5 |
| Smooth scrolling | a notched wheel, the scroll bar's arrows and track, and paging glide; precise deltas apply at once; reduced motion is instant; focus scrolls into view | (W) | on `claude/scroll-0.5`, queued for the seal checkpoint |

## Owner rulings, 2026-11-08

- **Smooth scrolling is in 0.5.**
- **The default theme becomes `gui-forms`** (T4 phase 6), and the four
  familiar themes (classic, xp-style, 7-style, macos-style) are approved.
- **`examples/reference` is the reference application.** The
  template_harness example moves onto the 0.5 shape after the seal.
- **The ABI baseline is re-recorded at the seal** (`gf_abi-0.5.json` gains
  the 1.1 additions; `GF_ABI_MINOR` stays 1) and the audit becomes blocking.
- **The Mac checks come after 0.5**: Cmd-Q, logout, the resize check and
  T5 C8's macOS rows.

## Before the seal

In order (CLOSURE_0.5.md section 3 has the detail):

1. Land checkpoint 22: the harness test fix, the harness and stx pins on
   merged commits, the first full CI run and the Windows verification.
2. Measure the gate records: T4 (G1), FD-7 (G2), T5 C8's Windows and X11
   rows (G3) and the creation gate per host (G4).
3. The default-theme switch to `gui-forms`, with traces re-recorded and a
   difference count.
4. The seal checkpoint: the switch, smooth scrolling, the setter audit, the
   documentation and these notes, folded and fully verified.
5. The seal: the ABI baseline re-recorded, the audit blocking, the seal
   record under `results/`, signed by the owner.

## After 0.5

The trajectory (owner, 2026-10-27, amended since) after the seal:

1. **The final controls**, then **1.0 (backend)**: the library, its ABI and
   its C# bindings. S, the C# sibling, belongs here; PlaySuite and SwiftEdit
   become full ports to the C ABI (owner, 2026-11-03).
2. **The closed-source tools**: the Windows Forms conversion program, the
   visual editor, the interface builder (D) and Visual Studio integration.
   Whether the theme designer (V) is one of them is open.
3. **A final cleanup round**, better documentation and tutorials, then
   **1.0 Stable** (`planning/stages/release-1.0/`).

Placed after the seal:

- the Mac checks (above);
- outer-edge effects: the active border, the window band and host shaping
  (ACTIVE_BORDER_STUDY.md, ruled after the seal 2026-11-07);
- Splash, further: baking on a worker thread, menus as their own Splash
  kind;
- rules in form documents (built-in actions only) and public run-time form
  loading;
- the template_harness example on the 0.5 shape;
- the consumer conversions (PlaySuite, Plan Paint, SwiftEdit, the benchmark
  demonstrations, the File Manager frontend) and the capability
  demonstrators (TRAJECTORY_TO_1.0.md);
- the Emscripten build, after `stx_skia`;
- the exhibits, before 1.0 (EXHIBITS_PLAN.md).

After 1.0: HTML import (H), a light FreeType fork, identical floating-point
results on every architecture.

## Superseded

The release names 0.6 Bindings and 0.7 Designers are retired: S moves into
1.0 (backend), D and V into the tools step, H after 1.0. The 2026-10-31
addendum's order (0.5 landing during the consumer refactor, 0.6 as
regression testing and the shadow desktop) is superseded by the seal
definition above; the consumer conversions follow the seal.
