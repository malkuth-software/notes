# Week of 2026-11-23: the push to 1.0

Date: 2026-11-18. Written by the coordinator (this session took the role
on 2026-11-18) for next week. Thanksgiving is Thursday 2026-11-26.
Status: a plan for the owner to adjust; nothing here is decided beyond
what it cites.

Sources:
- the owner's roadmap notes of 2026-11-18 (the list below follows them);
- the Shadow harvest, `projects` repository, `owner_record/
  shadow_harvest_2026-11/` (merged 2026-11-17, 043aab1): every decision and
  open question gathered on the Windows machine;
- `planning/ROADMAP_TO_1.0.md` (status 2026-11-13);
- `planning/studies/MCP_AUTOMATION_STUDY.md` (revision 3, with the answers
  of 2026-11-18).

How the week runs (the owner's working rules from the harvest): code
first, verify later; go far rather than hunt and peck; coders write, the
integrator batches and builds; the final sanitizer runs come last, after
everything else has landed.

## 1. To develop, test and integrate

### 1.1 Repositories and records

| # | Item | What done looks like | Notes |
|---|---|---|---|
| R1 | Test apps and programs to their new repository | every test program, probe and test-only tool lives in `malkuth-software/gui_forms_tests`; this tree keeps the library, packaging and the installed reference examples (the `AGENTS.md` rule of 2026-11-17) | the split is under way (`tests-split` worktree; `gui_forms_tests` updated 2026-11-18); the week finishes what is left and removes the old copies |
| R2 | BFFT: split the owner's experiments from the library; bring the library in house as a repository under malkuth-software | `malkuth-software/bfft` holds the library alone, with its history; the experiments stay in the owner's experiments repository; stx_auxlib's BFFT tiers point at it | owner creates the repository; then the line count of what we ship includes it (code-statistics tool, queued with #483 group 5) |
| R3 | Milestone records rewritten | the stage index, `ROADMAP_TO_1.0.md` and `docs/dev/roadmap.md` say what is true after #483 and the product releases | one writer, after R1 |

### 1.2 Documentation and the public face

| # | Item | What done looks like |
|---|---|---|
| D1 | A final read of the documentation for anything out of date | every page checked against the code at the week's head; stale pages fixed or removed |
| D2 | Screenshots for everything visual, as small previews | each control and feature page carries a small preview; site screenshots use the windows-professional theme (standing rule from the harvest) |
| D3 | `[src]` links | each documented class, function and page links to its source file at a pinned commit |
| D4 | The GitHub README: roadmap, versioning and previews cleaned up | the README states the current version scheme (0.6 package numbers, 1.0 next), the roadmap image matches, previews are current |

D2 can be produced with the MCP capture once it exists; this week they are
made with the existing capture tools (`Window::capture_window`).

### 1.3 Engine imports from Codex

| # | Item | Notes |
|---|---|---|
| E1 | The squircle convolution algorithm for buttons | for the button shape; measured against the current rounded rectangle for cost and look |
| E2 | Control outline rounding | the same family, for outlines |
| E3 | Normalised sinc, cos and the like | only if fast (the math policy: fast first, under 1 ulp where it costs nothing) |
| — | Gaussian | skipped (owner: not worth it) |

The source of these imports is not in the repositories this session can
see (question 1).

### 1.4 The tools

| # | Item | State |
|---|---|---|
| T1 | Theme editor | started (malkuth-tools `claude/theme-editor`, on SwiftEdit's CodeEditor) |
| T2 | WYSIWYG editor (stage D) | open |
| T3 | WinForms reflector | question 2: the reflection stage R, the mirrored-mode launcher (roadmap 5.5, proof of concept on SDR#), or both |

These are Block 5 tools (closed source, `malkuth-tools`). The roadmap
places them after 1.0 (backend); the owner's list brings them into the
week, so they run beside the library work, not in its integration batches.

### 1.5 MCP

As in the study (revision 3 with the 2026-11-18 answers): `stx_mcp` in
STXlib (X1) and the automation core on the headless host (G1) can both
start; the wiring, control and safety, the border glint and the ABI follow.
New from the owner's notes (study section 18): automatic semantic
instrumentation (event propagation, layout reasons, resources, state
summaries) and dynamic interfaces through structural patching (export,
validate, apply and revert form documents; a build toggle to write them
out), as stage G6.

### 1.6 Last

| # | Item |
|---|---|
| S1 | Final sanitizer runs of the whole suite (`gui_forms_tests`, sanitize kind), after everything else is done and dusted |

## 2. Gaps, audits and discussions

### 2.1 Unfinished business from the Shadow harvest

Open questions found on Shadow that are not yet written into GUI.Forms'
planning:

| Item | From | What is needed |
|---|---|---|
| **MDI** | 2026-11-03: "i want further discussion on 5: MDI" | a discussion; planned with the docking work (DK). The owner's notes ask the same |
| **KeyTips** | 2026-10-14 "we wont implement these keytips" against stage A's later "KeyTips as access keys" | confirm the A item means only publishing mnemonics as access keys to assistive technology (the likely reading, which keeps both) |
| **Repeated-item anatomy** (list rows, tree nodes, grid cells) | 2026-10-13: "havin g hard time deciding 1/2" | check whether the Mac work settled it (VirtualRowsView and the grid engine suggest it did); if so, record it; if not, a short explanation and a choice |
| **The control ID question** | 2026-10-13 | same: check `GUI_FORMS_INTERVIEW_LEDGER`; stable IDs are now used everywhere, so it is likely settled |
| **Lamps as a radio choice** | 2026-11-01 | how the radio form of indicator lamps (empty slots, click picks one) is scoped, since a lamp itself never acts |
| **Brushed metal and carbon fibre** | 2026-11-01 | texture algorithms for the skeudoindustrial theme; a study item |
| **Frameless borders as effects** | 2026-11-09: textures, strobe or steady colour carrying meaning | ties directly to the MCP border glint (study 11.1); do them together |
| **The tooltip defect** | SDR# review: "the tooltip issue is generally bothering me" | a defect to find and fix, not a decision |
| **Menus** | the owner's note "(menus?)" | audit menu behaviour against the harvest and the RB decisions (Backstage, no KeyTips, baked ribbon arrangement) |
| **Memory re-sync** | harvest `insights.md` | 11 coordinator memory files differ between Shadow and the mirror; re-sync before Shadow is retired |

### 2.2 Known gaps, for one discussion

- Outbound drag (dragging out of a GUI.Forms window into other programs),
  and inbound drag on Windows.
- Host autocomplete (the platform's text completion and suggestion
  services) and the other host services we do not use yet.
- The rest of `planning/GAPS.md`, reviewed in the same sitting.

Output: one gap catalogue, each item marked do now, before 1.0, after 1.0
or never, with the owner's call.

## 3. Studies (code-free, in parallel)

### 3.1 Touch, for industrial interfaces

The owner's aim: natively touch-friendly behaviour, reliable beyond
consumer devices: multitouch, time touched, probabilistic analysis of
where a touch was meant, possibly dwell time and capacitance from the
drivers. The owner has no touch device.

The study should cover:
- **What each OS delivers.** Windows: pointer input messages with contact
  area, pressure and the digitiser's confidence bit; Linux: evdev's
  multitouch protocol (slots, contact width and height, pressure), through
  XInput 2.2 touch events on X11; macOS: no touch screens, only trackpad
  touches. What HID digitisers report (contact size, pressure, confidence)
  and what needs a vendor SDK (raw capacitance).
- **Industrial practice.** Gloved and wet operation, palm and accidental
  touch rejection, confirm on release, dwell-to-activate for critical
  actions, large targets, two-handed or hold-to-confirm for dangerous ones,
  and the usability engineering standard for such equipment (IEC 62366).
- **Probabilistic targeting.** Treating a touch as a probability over
  nearby controls (contact shape, history, the control's importance) instead
  of a point, with a refusal when two targets are too close to call.
- **Testing without a touch device.** Synthetic touch through GUI.Forms'
  portable input records (and the MCP `pointer` tool once touch is a pointer
  kind); Windows' touch injection; a virtual multitouch device on Linux
  (uinput); recorded touch traces replayed in `gui_forms_tests`. This
  removes the hardware problem for most of the work; a cheap USB touch
  monitor would cover the rest.
- **What GUI.Forms changes.** A touch pointer kind in `PointerEvent`,
  gestures (pan, pinch, long press) as portable events, touch target sizing
  in the theme, and per-control touch behaviour.

Embedded devices and touch-screen controls (the owner's last item) belong
to this study: the controls an industrial panel needs (large steppers,
hold-to-confirm buttons, jog wheels) and the surface for a device with no
window manager (GUI.Forms' own window manager, `src/host/surface/`, already
targets STXlib's framebuffer appliance).

### 3.2 DevExpress and Telerik: the great roundup

"We will take it all": every control, demo and tool we think we should
implement ourselves, and nothing we should not. Under the clean-room rule
already used for BUI and Blender: behaviour and appearance from public
demos and documentation only, no code read. Output: an inventory mapped
against the control completeness matrix and the WinForms catalogue, each
item marked take, already have, or leave, with the demos that show it best;
then the owner's pass.

### 3.3 Building for Apple without Apple

The monster step: today building for macOS needs Apple's SDK, which means
building on a Mac, because there is no clean-room header set. The owner has
an outbound e-mail to the EFF and a possible workaround. The study should:
- set out the workaround beside the no-MinGW precedent on Windows (API
  headers generated from openly licensed metadata, our own runtime);
- list what a macOS build actually needs from the SDK (the frameworks
  GUI.Forms' host uses: AppKit, CoreGraphics, CoreText, IOSurface, and the
  system libraries) and how much of it could be described from public
  documentation;
- note the linker, code signing and notarisation steps, which have their own
  dependence on Apple's tools;
- wait for the EFF's answer before any code. No legal conclusion is drawn
  here.

### 3.4 Dynamic interfaces

Folded into the MCP study (section 18.2). Its discussion item, better
animation of ribbon transforms, goes to the ribbon stage.

## 4. Suggested order

- **Monday and Tuesday:** coders start R1, D1-D4, E1-E3, the MCP X1 and
  G1, and the tool work; the studies (touch, roundup, Apple) start; the gap
  discussion is prepared from 2.1 and 2.2.
- **Wednesday:** the integrator's first batch; the owner's gap discussion.
- **Thursday:** Thanksgiving.
- **Friday:** the second batch, the records rewrite (R3), then the final
  sanitizer runs (S1).

## 5. Questions for the owner

1. **The Codex imports** (squircle convolution, outline rounding,
   normalised sinc and cos): where are they? Neither the Codex workspace on
   this machine nor the repositories show them.
2. **The WinForms reflector:** stage R (SDR# reflection), the mirrored-mode
   launcher, or both?
