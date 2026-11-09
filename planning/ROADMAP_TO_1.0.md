# From 0.5 to 1.0

Date: 2026-11-08, written after the owner declared 0.5 done. Status: a
proposed staggering for owner review; nothing below is decided unless it
cites an owner ruling. `planning/FORWARD_PROGRAMME.md` stays authoritative
for stage scopes and gates, and each stage brief under `planning/stages/` for
its own decisions. `planning/ROADMAP_TO_0.5.md` records what 0.5 sealed.

Sources: the coordinator's `POST_0.5_BACKLOG.md` (the owner's post-0.5
items, 2026-11-08), `TRAJECTORY_TO_1.0.md`, `CONTROL_SUPERSET_STUDY.md`,
`EXHIBITS_PLAN.md`, `ACTIVE_BORDER_STUDY.md`, `SPLASH_NOTES.md` and
`CLOSURE_0.5.md` (projects repository, `desks/coordinator/`), and the stage
index.

## How to read the tables

**Effort (U):** one worker topic of checkpoint size, verified, as in
`CLOSURE_0.5.md`. Rough estimates, not measurements. Verification-bound
work is counted in U too, but it is serial: adding coders does not shorten
it.

**Kind:** **S** a study (documents only, code-free, parallel), **C**
code-first coder work (no build; registered with the integrator), **B** a
build or verification run through the integrator, **O** owner review or
decision, **M** Mac hardware, **W** a Windows host, **V** Visual Studio on
the shadow desktop.

**Study first:** a row marked *(study first)* may not start code until its
study is accepted by the owner. Six studies are required before code:
declarative dragging, BUI, Dear ImGui, ImPlot, Blender's value bars and the
mirrored-mode feasibility.

## The shape

```text
Block 0  Seal tail ─────────────┐ (now, in parallel with Block 1)
Block 1  Studies ───────────────┤ code-free; each gates its Block 2 row
Block 2  Engine and final controls ── needs Block 1 per row
Block 3  ABI additions and C# ──────── band declaration first; S needs R
Block 4  Consumers and demonstrators ─ needs Block 2 pieces, Block 3 ports
         ══ 1.0 (backend) declared ══
Block 5  The tools ─────────────────── needs S, the drag engine, SwiftEdit
         ══ cleanup, documentation, packaging: 1.0 Stable ══
```

The stagger is three waves. **Wave A** (now): Block 0 and every Block 1
study run at once; studies are cheap, code-free and owner-reviewed, and the
seal tail is mostly verification. **Wave B**: Block 2 rows open one by one as
their studies are accepted, and Block 3's band declaration and R start
beside them. **Wave C**: S, the consumer ports and the demonstrators, which
consume Block 2's pieces and stress the drawing ABI before it is sealed.
The tools follow 1.0 (backend), as the owner set on 2026-10-27; the owner
expects the pace to slow in the Visual Studio phase and quicken again
towards the end.

Throughout, coders write in parallel and the integrator verifies in
batches; the floor of every block is its verification chain and owner
review, not its U total.

## Block 0: seal tail (Wave A)

The items the owner ruled after the seal, and the closure list's leftovers.

| # | Item | Depends on | Kind | U | Notes |
|---|---|---|---|---|---|
| 0.1 | Mac handoff: Cmd-Q, logout, the resize check, T5 C8's macOS rows | the seal head on the M4 | M+O+B | 0.5-1 | first Mac session; records under `results/` |
| 0.2 | template_harness example on the 0.5 shape | harness pins on main | C+B | 1-1.5 | harness PR by hand (owner rule) |
| 0.3 | Plan Paint opens `.PNG`: case-insensitive extension matching | none | C+B | 0.25 | a defect; take it now, not with the revision |
| 0.4 | Splash, further: baking on a worker thread; menus as their own Splash kind | none | C+B+O | 1.5-2 | perceptual check by the owner (snappiness, slide) |
| 0.5 | Record refresh: stage index, board, track pointers; this roadmap folded into `docs/dev/roadmap.md` | owner accepts this page | C | 0.5 | docs-only |
| 0.6 | Owner discussions: the theme designer, the tools' licence class | none | O | - | feed Block 5 and question 3, 4 |

**Mac handoff.** What we take: closure of the seal's deferred host rows, so
the 0.5 record is complete on all three hosts. Test: each check is a
recorded run on the M4's GUI session (Cmd-Q and logout end the application
through the lifecycle contract with no lost state; the resize check is the
owner's perceptual verdict; the C8 rows are Light, Dark and Reduce
transparency captures against the compositor capability report).

Block 0 total: about 4-5 U, of which about 2 U is coder work; the rest is
Mac, harness and owner time.

## Block 1: studies (Wave A; all code-free)

All seven run in parallel as documents in the coordinator desk, each ending
in numbered decisions for the owner. **BUI and Blender are GPL** (BUI is
GPL-3.0 and decouples Blender's own widget code), so both studies follow the
clean-room rule of `CONTROL_SUPERSET_STUDY.md` section 1: behaviour and
appearance only, no code read for transcription. BUI is also the closest
public form of Blender's value bars, so 1.2 and 1.5 share a reading.

| # | Study | Source | Feeds | Kind | U |
|---|---|---|---|---|---|
| 1.1 | Declarative dragging *(required)* | declarative-dragging.github.io; Horowitz, Blinn and Heer, UIST 2026 | 2.1, 2.4, DK drag, list reorder | S+O | 0.5-1 |
| 1.2 | BUI node engine *(required)* | github.com/ppmpreetham/BUI (finish reading) | 2.4, D1 | S+O | 1 |
| 1.3 | Dear ImGui: controls and effects *(required)* | imgui_demo.cpp | 2.5, 2.6, D3 | S+O | 1 |
| 1.4 | ImPlot: plot layer and primitives *(required)* | ImPlot demo and API | 2.3, D4, D5 | S+O | 1 |
| 1.5 | Blender's value bars *(required)* | Blender, BUI | 2.2 | S+O | 0.5 |
| 1.6 | Mirrored-mode feasibility *(required)* | .NET hosting, facades, startup hooks | 5.5 | S+W+O | 1-1.5 |
| 1.7 | Drawing ABI inventory (`gd_*` before its seal) | the drawing headers, the above studies | 3.2 | S | 0.5 |

**Declarative dragging.** The paper's model: an author lists the model
states a dragged element may reach; the engine speculatively lays out each
state, finds where the element lands in each, and turns those positions into
drop zones for an animated drag, with composable modifiers for snapping,
floating and transitions. What we would take: one retained drag engine in
which a drag is declared as a set of reachable states over our layout,
instead of hand-written hit geometry per control; DK's docking drag, list and
tab reordering, PageView drag, the node graph's links and the designer's
canvas would all be its clients, and the form language could declare drag
targets. Speculative layout must stay bounded (no per-frame allocation,
reachable states enumerated before the drag starts) to respect the
house style and the creation-cost contract. Test: deterministic drag
traces (pointer script in, drop-zone set and final model state out) for each
client; a keyboard alternative for every drag (DK's requirement and the
accessibility gate); an allocation counter at zero during the drag's
interior; display traces of the animation at fixed clock steps.

**BUI node engine.** What we would take: the node editor's interaction
grammar (typed sockets, coloured links, header menus, frames and reroutes)
to set beside the owner's requirements: blueprint grid and texture
backgrounds in white, blue, green or grey; magnetically attaching links on
soft curves; double-click to edit a node's contents. Implemented in our
style: a retained `NodeCanvas` over the drag engine, pattern and texture
fills as engine primitives (shared with the window band's textures),
links as curve primitives, and the Diagram study (DG, waiting on its
interview) reconciled so there is one canvas, not two. Test: model-level
tests (connect, disconnect, typed-socket refusal, undo), link-snap geometry
tests at fixed positions, display traces of the backgrounds on all hosts
(identical pixels under the own-engine rule), and a large-graph paint
benchmark recorded with its workload.

**Dear ImGui.** What we would take: a ranked list of the controls our
superset still lacks (drag-to-scrub numbers, colour edit and picker
variants, tree nodes with inline widgets, plot lines, tables with frozen
columns, docking previews) and of the UI effects worth emulating (hover and
active fills, drag previews, collapsing headers, tooltips that follow, the
style editor's live retuning), each mapped to an existing control, an
effect program or a new row. Built the retained way: no immediate-mode
control API (`AGENTS.md`). Test: each admitted control enters the Gallery
with its semantic, input and display-trace tests; each effect is an effect
program with its budget counted.

**Blender's value bars.** What we would take: the settings bar that shows a
value as a filled proportion, edits by drag (scrub), click (step), double-click
(type a number) and Ctrl or Shift modifiers (snap, fine), and the pieces it
needs: a fill part, an inline editor swap, unit formatting and a soft range
inside a hard range. Likely a presentation of NumericUpDown or TrackBar plus
the drag-to-scrub mode, not a wholly new control. Test: input scripts for
each gesture with the resulting value, keyboard entry, the UIA, MSAA, AX
and AT-SPI range pattern reading the value, display traces of the fill at
0, 50 and 100 percent in every theme.

**ImPlot.** What we would take: a declarative plot description (series,
axes, legends, markers, a time base for animation) equivalent to ImPlot's
line, scatter, bar, shaded and heatmap plots, animated sine waves and live
2D plots, on our high-speed path; and, separately, the public primitives
users need to assemble plot kinds we will not make (scales and ticks,
polyline and area batches, marker atlases, clipping, hit-testing a series,
a decimation pass). Instruments (W-5's vocabulary, gauges) share the scales.
Test: numeric tests for scales and decimation (proven error bounds), display
traces of each plot kind, an idle check (no frames when the data are still),
and a measured live-plot benchmark (points per frame, frame cost) before any
claim of speed.

**Mirrored-mode feasibility.** What we would learn: whether a WinForms
program can run on GUI.Forms without recompiling, and through which
mechanism: a replacement `System.Windows.Forms` facade with binding
redirects (.NET Framework), `DOTNET_STARTUP_HOOKS` or an assembly-load hook
(.NET), or method interception. The study names what each reaches (strong
names, ReadyToRun, P/Invoke into user32, owner-drawn controls), how
unsupported calls are captured and logged, and the legal position of
launching third-party programs (no disassembly). It includes one probe on a
Windows host with a small free WinForms program. Test of the study: the probe
runs and its log lists every unsupported member it met; the full test plan
is in 5.5.

Block 1 total: about 5.5-6.5 U, all coder or study work, plus owner review.

## Block 2: engine and final controls (Wave B)

| # | Item | Depends on | Kind | U | Parallel with |
|---|---|---|---|---|---|
| 2.1 | Declarative drag engine *(study first: 1.1)* | DK's drag session | C, then B | 3-4 | 2.2, 2.3, 2.5 |
| 2.2 | Value bar (Blender-style) and drag-to-scrub *(study first: 1.5)* | the reduced-motion gate | C, then B | 1-1.5 | all |
| 2.3 | Plot primitives, then the declarative plot layer, sparkline *(study first: 1.4)* | 1.7 | C, then B | 5-6 | all |
| 2.4 | Node canvas: blueprint backgrounds, magnetic curved links, double-click edit *(study first: 1.2, 1.1)* | 2.1, pattern fills | C, then B | 4-5 | 2.3 |
| 2.5 | Final controls: W-1 to W-3, W-5b to W-9, and the superset's P1 rest (callout, wizard and step indicator, gauges, dial, seven-segment readout, lookup combo, in-window modal layer, About) | ImGui ranking (1.3) for any additions | C, then B | 8-12 | all |
| 2.6 | Effects taken from Dear ImGui, as effect programs *(study first: 1.3)* | EP | C, then B | 1-2 | all |
| 2.7 | Active border phase 2: the control border | none | C, then B, O | 2.5 | all |
| 2.8 | Active border phase 3: the window band, host shaping, band textures and effects | pattern fills (2.4) | C, then B, W, M, O | 6.5 | 2.5 |
| 2.9 | DK and RB phases still open (to audit against their plans) | - | C, then B | audit first | all |

Coder work is most of Block 2 and can run four or five rows at a time; the
verification lands in two or three integrator checkpoints. Host shaping
(2.8) needs a Windows 11 host and the Mac; the owner reviews the control
border and the band visually. The band follows the owner's 2026-11-07
answers (fixed reservation, textures on the band only, the focus ring stays
on its own path) and 2026-11-08's (no resize-edge role).

Block 2 total: about 31-40 U, of which about three quarters is coder work.

## Block 3: ABI additions and C# (Wave B into C)

| # | Item | Depends on | Kind | U | Notes |
|---|---|---|---|---|---|
| 3.1 | C ABI band declaration: window presentation in the windows table | 2.8's declaration shape | C+B+O | 0.5-1 | additive; needs a recorded owner decision (sealed interface) |
| 3.2 | Drawing ABI seal (`gd_*`) | 1.7, 2.3, 2.4, Block 4's demonstrators | C+B+O | 1-2 | sealed only after the demonstrators stress it |
| 3.3 | R: SDRSharp reflection rebuilt on the current ABI | T4, F | C+B+W+M | 2-3 | precedes S (forward programme) |
| 3.4 | S: generated C# bindings, the WinForms-shaped package, the corpus | 3.3, C | C+B+W+V | 6-10 | its own repository |
| 3.5 | PlaySuite and SwiftEdit as full ports to the C ABI | ABI 1.1 | C+B | 2-3 each | owner, 2026-11-03 |
| 3.6 | 1.0 conditions: normative traces on Windows, macOS and X11; the member manifest; two-consumer conformance | 3.5 | B+W+M | 2-3 | release-1.0 conditions 2 to 4 |

**C ABI band declaration.** What we take: C and C# programs can declare a
window band, which today only C++ can, because the windows table has no
window presentation. Additive, after the band's C++ shape settles, and
landed with the C# work (owner, 2026-11-08). Test: the breaking-change audit
passes against the sealed baseline with only additions; a C conformance
test and a C# interop test declare, light, flash and undeclare a band and
read the reserved rectangle back.

Block 3 total: about 16-25 U; S is the long pole and is Windows- and
Visual-Studio-bound.

## Block 4: consumers and demonstrators (Wave C)

| # | Item | Depends on | Kind | U |
|---|---|---|---|---|
| 4.1 | SwiftEdit revised to the new standard and interface; syntax highlighting, CSS first | 3.5, a styled-run seam in the text editor | C+B+O | 2-3 |
| 4.2 | Plan Paint: the canonical ribbon with soft large bubble buttons; robust file handling; the new `stx_conv` algorithm | RB, 0.3 | C+B+O | 2.5-3 |
| 4.3 | PlaySuite migrated forward: custom skinning through themes, with a gap list | 3.5 | C+B+O | 2-3 |
| 4.4 | D1 node graph, D3 ImGui-class and D4 ImPlot-class demonstrators | 2.4, 2.5, 2.6, 2.3 | C+B | 4-5 |
| 4.5 | D2 flame graph and profiler viewer | 2.3 | C+B | 2 |
| 4.6 | D5 ImPlot3D-class plotting | owner relicensing of PlaySuite's 3D parts to MIT | O, then C+B | 2-3 |
| 4.7 | File Manager frontend, benchmark demonstrations | per their plans | C+B | per plan |
| 4.8 | Emscripten build | `stx_skia` | C+B | 3-5 |
| 4.9 | First exhibits: E2 oscilloscope, E4 process control, E14 wizard, E9 data entry | 2.3, 2.5 | C+B | 1-2 each |

**SwiftEdit.** What we take: the editor on the 0.5 shape (forms, properties
as capabilities, the `gui-forms` theme) and syntax highlighting, at least
for CSS, which the theme editor needs. Test: tokenizer tests over a CSS
corpus (including our theme files), display traces of highlighted text,
edit-and-rehighlight damage limited to the changed lines.

**Plan Paint.** What we take: the canonical ribbon demonstrated as close to
the current layout as possible with the new bubble buttons; file handling
that accepts any extension case and reports refusals; the faster `stx_conv`
path. Test: open tests for `.png`, `.PNG` and mixed case and for malformed
files; ribbon display traces against the RB plan; a recorded before-and-after
conversion benchmark.

**PlaySuite.** What we take: proof that themes can skin a game frontend to
look exactly as it does now, and the list of anything missing. Test: capture
the current PlaySuite and the migrated one at the same states and diff;
every remaining difference becomes an engine or theme item, or an accepted
difference with a reason.

**1.0 (backend) is declared** when Blocks 2 to 4 are in, the drawing ABI is
sealed and the release-1.0 conditions chosen for it (question 1) are
recorded. Block 4 total: about 18-24 U plus the File Manager and the exhibits.

## Block 5: the tools (after 1.0 backend)

Closed source and nagware (owner, 2026-10-27); the licence class and the
certificates (Authenticode, an Apple Developer ID) come first.

| # | Tool | Depends on | Kind | U |
|---|---|---|---|---|
| 5.0 | Licence class, certificates, the nagware screen and About | owner | O, then C | 0.5-1 |
| 5.1 | Theme editor: the screencap utensil and the visual editor | 4.1, T4 provenance | C+B+O | 5-6 |
| 5.2 | WYSIWYG designer (D) with in-line theme override | 2.1, L, FD, 5.1's rule panel | C+B+O | 8-12 |
| 5.3 | Visual Studio integration and package, for VS and C# | 3.4, 5.2 | C+B+W+V | 3-5 |
| 5.4 | Windows Forms conversion program | 3.4 | C+B+W | 3-4 |
| 5.5 | WinForms mirrored-mode launcher *(study first: 1.6)* | 3.4 | C+B+W | 4-8 if feasible |
| 5.6 | Exhibits continue; E12, the .NET open-source port | 3.4 | C+B+W | per exhibit |

**Theme editor.** It is two things (owner, 2026-11-08), built on SwiftEdit.
A screencap utensil captures every control, or chosen pages of them, laid
out intelligently for reference work; a visual editor lets a person click a
control and see the rules that affect it highlighted, press Compile above
the highlighted rule text to learn whether the sheet is legal, and use
save, undo, redo, search and find on a small bar. What we take from the
engine: T4's provenance for "which rules affect this", T3's compiler
diagnostics for Compile. Test: provenance queries against golden rule sets;
Compile over the T3 conformance corpus (legal and illegal sheets with
expected diagnostics); capture layouts as display traces; editor commands
under scripted input.

**WYSIWYG designer.** Drag and drop onto a canvas with PCB-editor-like
semantic control and Photoshop-style controls: click to set the floor plan
(the window), then place things on it; flex and grid layouts show their grid
while adjusted, with snap; right-click sets a rule, and on save it asks
whether to move the rule into a `.css` or gives a C++ inline block to paste.
What we take: D's gate (hand and designer edits round-trip byte for byte)
plus the drag engine for every canvas gesture. Test: the D round-trip gate;
snap geometry tests in flex and grid; the inline-rule export compiled in a
test consumer; the T3 relocation tool agreeing with the designer's "move to
.css".

**Visual Studio integration.** What we take: a package that installs
GUI.Forms for C++ and C# projects, opens form documents in the designer and
builds the generated code. Test: a clean shadow-desktop VS install creates,
builds and runs a C++ and a C# template project; the package's uninstall
leaves no residue.

**Mirrored-mode launcher.** Windows only. It launches an existing WinForms
program with GUI.Forms in place of WinForms, routes the calls to us, and
captures and logs anything unsupported. For us it dogfoods real interfaces
and gathers unsupported logic without disassembly; for users it is a
no-compile swap-in that is not guaranteed everywhere. Test: a corpus of
free and demo WinForms programs launched under the shadow desktop's
private desktop; per program, a launch verdict, an unsupported-member log
that is complete (cross-checked against a reference run's call trace) and a
visual differential against the real WinForms run; nothing writes outside
its sandbox.

Block 5 total: about 24-36 U, much of it Windows- and VS-bound, so the
slowest block per U.

## Formal 1.0

| # | Item | Kind | U |
|---|---|---|---|
| F.1 | Cleanup round across the tree | C+B | 2-3 |
| F.2 | Documentation and tutorials, after the owner's discussion of a better form | C+O | 2-4 |
| F.3 | Packaging: signing, notarization, SBOM, reproducible builds | C+B+W+M | 2-3 |
| F.4 | The release-1.0 gate record; the owner declares 1.0 | B+O | 0.5 |

After 1.0: touch base with SDRSharp, HTML import (H), a light FreeType fork,
identical floating-point results on every architecture.

## Totals

| Block | U | Coder share |
|---|---|---|
| 0 Seal tail | 4-5 | about half |
| 1 Studies | 5.5-6.5 | all, plus owner review |
| 2 Engine and final controls | 31-40 | about three quarters |
| 3 ABI and C# | 16-25 | about half (Windows- and VS-bound) |
| 4 Consumers and demonstrators | 18-24, plus exhibits | about two thirds |
| 5 Tools | 24-36 | about half |
| Formal 1.0 | 6.5-10.5 | about half |

The coder share can go faster under code-first, verify-later; the rest is
bounded by integrator checkpoints, hosts and owner review.

## Open questions for the owner

1. **What declares 1.0 (backend)?** Recommendation: release-1.0 conditions 1
   to 5 (ABI audit, two consumers, manifest, normative traces, native
   accessibility) and the drawing ABI seal; packaging (condition 6) at
   formal 1.0.
2. **Which two conformance consumers?** The 2026-10-13 ruling names
   PlaySuite and Plan Paint; 2026-11-03 makes PlaySuite and SwiftEdit the C
   ABI ports. Recommendation: PlaySuite and SwiftEdit through the C ABI
   satisfy the condition; Plan Paint stays a C++ consumer.
3. **Is the theme editor V?** V was a timeline-and-stage designer reusing
   Plan Paint's editors. Recommendation: the theme editor is V's first
   form and a tool; timeline animation authoring after 1.0.
4. **The tools' licence class.** Recommendation: one proprietary licence for
   all closed tools, decided before the first tool is distributed, not
   before coding starts.
5. **Clean-room for BUI and Blender (GPL).** Recommendation: behaviour and
   appearance only, as the superset study's rule.
6. **Mirrored mode's target runtime.** Free WinForms programs are mostly
   .NET Framework, while W covers .NET 8+. Recommendation: the study covers
   both; prototype .NET 8+ first, and admit Framework only if the facade
   route proves cheap.
7. **Declarative dragging's reach.** Recommendation: an internal engine for
   every drag in GUI.Forms first, with drag targets in the form language
   once DK and the node canvas use it.
8. **Plots and instruments as packages or base controls?** Recommendation:
   a plots package beside the instruments package (superset decision 2b),
   with the primitives public.
9. **ABI 1.2 for the band declaration.** Recommendation: an additive minor
   version with the C# work, recorded as the first post-seal owner decision.
