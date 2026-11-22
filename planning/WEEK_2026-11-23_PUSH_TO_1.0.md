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

Both come from the owner's Codex sessions on this machine (found
2026-11-18). Codex wrote them as mathematics, not in the house style.
**They are not folded as written** (owner, 2026-11-19): E3 and E4 become a
native-style implementation in STXlib, and everything else imported from
the `sinc` and `pow` studies gets a house-style check-over first. Codex's
branch and patch are the reference the native code is measured against
(same results, then speed).

| # | Item | Where it is | Notes |
|---|---|---|---|
| E1 | The CONV squircle for buttons | `~/Documents/Codex/2026-11-17/i/outputs/`: the tester (`conv-squircle.html`), the source and tests (`conv-squircle-source.zip`), and three derivations (`CONV-math.md`, `CONV-circle-square-math.md`, `CONV-flat-smoothstep-math.md`) | CONV here is Compact Ordered Nodal Variation (stx_conv's mathematics), not convolution. The owner's last direction is the **flat smoothstep family**: exact straight sections whose anchors slide along the sides (0 % is a circle with four marked points, 100 % a square), all turning in the corners, convex, with C4 joins. Port it to C++ as an engine shape primitive and measure it against the current rounded rectangle for cost and look |
| E2 | Control outline rounding | the same family | outlines and focus rings follow the button shape |
| E3 | Normalised sinc, half-turn sine and cosine | STXlib, local branch `codex/pi-trig-and-interval-study` at `f341b2c` (in `~/Developer/stxlib`): `sinpi`, `cospi`, `sincospi`, `sincpi`, `sinc`, `sinc_radians`, `cosm1`, `cosm1pi`, for `float` and `double` | measured on the M4: the double `sincospi` pair 3.87 ns against Apple's 2.04 ns, largest observed error 0.501493 ULP over 172,288 inputs (MPFR); reimplemented in STXlib's native style with `f341b2c` as the reference, then GUI.Forms takes it with the stx pin |
| E4 | The faster `pow` | `~/Documents/Codex/2026-11-18/e/outputs/stxlib-pow-grid.patch`, with `pow-investigation.md` and `stxlib-pow-study.zip` beside it (not yet on a branch) | approved for use (owner, 2026-11-18): keep x^y = 2^(y log2 x) and compute the small remainder of log2 x more carefully; 10.96 to 8.57 ns a call on the M4, analytical budget 0.509425 ULP, largest measured 0.501712 ULP over 2.45 million pairs, no fast-math. Reimplemented in STXlib's native style against the patch as reference, then the stx pin |
| — | Gaussian | `f341b2c` adds `gaussian_interval` in the same commit as E3 | rejected (owner, 2026-11-18): it needs a 140 KiB table and produces tiny numbers, and STXlib blocks subnormals. The native implementation leaves it out, and with it `gaussian.cpp`, `gaussian_table.inc`, `gaussian_tail_terms.inc`, `tools/math/gen_gaussian_table.py`, and its entries in `dispatch.cpp`, the headers and the tests |

The owner's hash research runs in the same Codex instance and is not this
week's GUI.Forms work. Its latest output is
`~/Documents/Codex/2026-11-18/e/outputs/chunked-exact-certificates.zip`
(with `chunked-exact-certificates.md`): a Python reference that appends one
reversible certificate to a file of any length, chunked in 1 MiB pieces with
repeated chunks stored once; the certificate determines the payload exactly,
so two different payloads cannot share one, at the price of a variable-length
certificate. Earlier stages are `exact-self-certification.md` and
`self-hashing-research.md` in the same folder.

### 1.4 The tools

| # | Item | State |
|---|---|---|
| T1 | Theme editor | started (malkuth-tools `claude/theme-editor`, on SwiftEdit's CodeEditor) |
| T2 | WYSIWYG editor (stage D) | open |
| T3 | WinForms reflector | stage R, the SDR# reflection (owner, 2026-11-18) |

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

### 1.6 `stx_filesign`: self-addressed signed files (STXlib)

Astra's specification "STX Release Seal v1" (STXlib `main` at `ff91916`,
`docs/release-seal/`) gives files whose last 64 bytes are the file's own
hash, signature included, signed by the publisher with Ed25519 or with
quantum-resistant LMS. The plan is in STXlib's notes (`stxlib` branch,
`planning/stages/FS-filesign.md`), with the owner's rulings of 2026-11-19.

This week:
- **FS0, the primitives**, owner-guided with a Fable instance, each on its
  own (`planning/stages/FS0-crypto-primitives.md`): SHA-256, SHA-512,
  SHA3-512, HMAC, Ed25519, LMS, a strict DER reader, CMS, X.509 path
  validation;
- **FS1**, the carrier reader and raw hash, once SHA-256 and SHA3-512 exist,
  to the first acceptance test on the published vectors;
- **FS-R**, the research into signing without a capacity limit (three
  levels, rotation under a persistent fundamental key, a public rotation
  log), as a research thread (`planning/stages/FS-R-unbounded-signing.md`).

It runs in STXlib beside the GUI.Forms work and does not gate GUI.Forms 1.0.
The malkuthsoft.com pages for the open specification (owner) come with FS6.

### 1.7 Last

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

Now a stage of its own (owner, 2026-11-19): `planning/stages/TC-touch/README.md`,
and roadmap row 2.22. GUI.Forms is meant for embedded and industrial use
(from a yacht to a plant), often on hardware with no OS touch layer. The
week starts TC0, the study: how each OS exposes touch and suppresses false
touches, capacitive against resistive, and what GUI.Forms must do itself on
the appliance. The owner has bought a 10.1-inch capacitive and a 10.1-inch
resistive Waveshare panel for the Pi 4, which also gives the appliance a
screen; when they arrive, record traces from both. The control changes
(larger controls, touch areas larger than the visible control, a press on
lifting the finger unless it is deliberately dragged off) follow in TC1-TC3.

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

### 3.4 CONV resizing: memory, and a true 2D field

**Superseded 2026-11-21** by the owner's canonical-CONV direction: a
dedicated stx_conv worker under the STXlib coordinator (STXlib notes,
`planning/stages/CV-conv-canon.md`), which takes Astra's 2026-11-20
reformulation (streaming, source and destination as the only whole rasters;
63 to 68 MB peaks at four megapixels against 220 to 540 MB) and the questions
below.

The owner asked ChatGPT about Plan Paint's CONV resize and found two
gaps worth research. Both are confirmed in the code:

- **Memory.** `plan-paint/src/conv.cpp` (`resize_image`) turns the 8-bit
  image into premultiplied `double`s (32 bytes a pixel against 4), resizes
  one axis into a whole intermediate image of doubles, resizes the other
  axis into a whole output image of doubles, then converts back. For a
  4000 x 3000 image enlarged to 8000 x 6000, the peak is about 2.9 GB
  (input 384 MB, intermediate 768 MB, output 1.5 GB, result 192 MB) for a
  48 MB picture, about 60 times its size. GUI.Forms' own interpolation
  (`src/core/interpolation/interpolation.cpp`, used by the Skia map scale)
  works on `double` lines too, and builds a profile and workspace for every
  line it evaluates instead of reusing them (house style section 1).
- **One dimension at a time.** Both resize separably: a pass of 1D CONV
  lines down the columns, then along the rows (the order depends on whether
  the height shrinks). The variation limiting that makes CONV what it is
  then acts along each axis alone, so an edge at 45 degrees is treated
  differently from a vertical one, and the result depends on the pass order.
  A true 2D field would hold the nodes on the lattice with currents and
  their admission in two dimensions.

The study should:
1. measure today's peak memory and time on a fixed set of images and sizes
   (the perf-lab rule: measure before claiming);
2. design a streaming resize: CONV's support is compact, so the second pass
   needs only a few rows of the first; work in strips, from 8-bit to 8-bit
   with premultiplication in registers, with `float` or fixed point where the
   error budget allows, and reuse plans and workspaces; the target is a peak
   of source plus destination plus a few strips;
3. work out the mathematics of a 2D CONV field (nodes, currents and
   admission on the lattice, from the CONV paper's line construction), and
   what it costs against the separable form;
4. compare the two on diagonal-edge and rotated test charts (orientation
   dependence, ringing and overshoot, pass-order dependence), keeping the
   current separable resize as the reference;
5. say what moves into stx_conv, GUI.Forms' interpolation and Plan Paint.

It suits the same kind of research thread that produced the squircle.

### 3.5 Dynamic interfaces

Folded into the MCP study (section 18.2). Its discussion item, better
animation of ribbon transforms, goes to the ribbon stage.

## 4. Order

The owner's order of operations to 1.0 (2026-11-19) governs; it is set out
in `planning/ROADMAP_TO_1.0.md`, "Order of operations to 1.0". This week
follows it from step 1:

1. **Plan the path to 1.0:** this page and the roadmap section (done when the
   owner accepts them).
2. **Clean up** old branches, local and remote, and free space from unneeded
   virtual environments; remote deletions listed for the owner's approval.
3. **The forgotten items** from the Shadow harvest (section 2.1).
4. **Documentation review** (section 1.2) and the site's in-progress items.
5. **Organise and delegate** the next builds and work to STXlib and GUI.Forms
   (sections 1.1, 1.3, 1.4, 1.6, and FS0 under the owner).
6. **Implement and integrate** everything outstanding (section 2.2's gaps
   included), with the framework reviews (section 3.2) running alongside.
7. **MCP** (section 1.5).
8. **Everything else:** touch (3.1), the CONV resize research (3.4), building
   for Apple without a Mac (3.3), the rest of `stx_filesign`.
9. **Declare 1.0.**
10. **Documentation to match**, its review and clean-up, integrity, embedded
    documentation, and pages showing GUI.Forms in many contexts.

How far the week gets is set by the work, not by the calendar; Thursday is
Thanksgiving. The final sanitizer runs (section 1.7) belong to step 6's
close, before 1.0 is declared.

## 5. Questions for the owner

None. Open questions inside a step go to that step's dedicated workers
(owner, 2026-11-19), for example touch's OS on the Pi 4 (TC brief, section
8).
