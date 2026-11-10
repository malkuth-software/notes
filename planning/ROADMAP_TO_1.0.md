# From 0.5 to 1.0

Date: 2026-11-08, written after the owner declared 0.5 done; updated
2026-11-09 and 2026-11-10 with the decisions and the work since (sections
"Status 2026-11-10" and "Status 2026-11-09", and the **Status** columns).
Open gaps are listed in `planning/GAPS.md`; the owner rulings of these two
days are indexed in `planning/DECISIONS_2026-11.md`. The owner answered the first
version's questions on 2026-11-08; nothing else below is decided unless it
cites an owner ruling. `planning/FORWARD_PROGRAMME.md` stays authoritative
for stage scopes and gates, and each stage brief under `planning/stages/` for
its own decisions. `planning/ROADMAP_TO_0.5.md` records what 0.5 sealed.

Sources: the coordinator's `POST_0.5_BACKLOG.md` (the owner's post-0.5
items, 2026-11-08), `TRAJECTORY_TO_1.0.md`, `CONTROL_SUPERSET_STUDY.md`,
`EXHIBITS_PLAN.md`, `ACTIVE_BORDER_STUDY.md`, `SPLASH_NOTES.md`,
`CLOSURE_0.5.md`, `STATE.md`, `INTEGRATION_QUEUE.md`,
`LOOK_INTEGRATION_LEDGER.md`, `MAC_HANDOFF_2026-11-09.md` and the Block 1
studies (projects repository, `desks/coordinator/`), and the stage index.

## Status 2026-11-10

**Checkpoints.** cp26 (#476), the CI docs fast path (#478), the docs refresh
(#477) and the CI fix (#479: test-targets-only PR jobs, area selection, the
prebuilt stx package, the shared test precompiled header, run cancellation)
merged on 2026-11-09; main is def3b2df6. By the owner's plan every worker
finished coding without separate builds, and the integrator merged every
registered head into one combined tree (fold/bleeding-edge), built it once
with the products, and pushed **cp27 as PR #480**
(checkpoint/2026-11-10-cp27 @ 757db03a7, open, auto-merge on; 416 of 418
locally, the two failures being fixed on top). The bleeding-edge SDK is
installed at `D:/malkuth/cache/lockstep/gui-forms-bleeding-edge-windows-x64`.

**Coded today, in cp27** (all on #480 unless marked):
- **menus and popups** (`claude/menu-system`): popups declare a content
  surface (no theme slab), translucency honours the backdrop grant, menu
  look (icons, widths, hover, separators), the two-pane application menu
  with submenus, tooltip rules, C ABI overlays slots 7-8
  (`set_tool_tips_shown`), ribbon local repaint;
- **the invalidation overlay** (`claude/invalidation-overlay`): areas
  presented again are outlined for 300 ms by cause; environment, C++, C ABI
  and Gallery switches; `docs/user/debugging.md` (the owner's note: the
  business card name turns fully red when the overlay is on);
- **the band fix** (`claude/dib-bottom-band-present`): the PlaySuite bounce
  and SDR#'s re-fit, a StretchDIBits read of bottom-touching bands;
- **pan and scroll copy** (`claude/canvas-pan-perf`): Plan Paint panning
  from 45-361 ms to 0.2-9 ms per step (recorded workload in the queue
  entry), scroll copies on Windows and Linux;
- **the look integration** (row 2.10, `claude/look-integration`): 28 ledger
  rows implemented on the branch; N1-N7 ruled;
- **final controls and the gauge parts** (row 2.5, `claude/final-controls`):
  SevenSegmentDisplay, gauges, Dial, StepIndicator, Wizard, Callout,
  ModalLayer, LookupComboBox, AboutDialog; the arc, needle, beak and scrim
  parts (option a);
- **the help system H1-H8** (rows 2.13-2.20, `claude/help-system`);
- **the audit's seal and hang fixes** (`claude/abi-seal-fixes`,
  `claude/audit-hang-parity`; `planning/audits/README.md`);
- **automation off by default** (`claude/automation-off-by-default`);
- **splash baking** for tooltips (row 0.4, `claude/splash-baking`);
- theme application roles and theme checking (`claude/theme-app-roles`),
  flex panel gaps, the docking C ABI table (`claude/dock-abi-docking`), the
  swap-in's present path and waterfall zero-copy
  (`claude/swapin-sdr-present-path`); and cp26's tail: the drag engine, the
  value bar, the drawing primitives and the plots (rows 2.1-2.3).

**Not in cp27:** the windows-professional redesign and theme font naming
(`claude/professional-theme` 114bddcda, follow-up
`claude/be-professional-theme` 96401f3cd), captures for owner review owed.

**SDR# swap-in.** First proof of concept on 2026-11-09: the real SDR# ran
on GUI.Forms through the .NET 10 swap-in with a live radio. The release
zip for Youssef Touil has its packaging script; it is built after the
combined build and tested by the owner first.

**Site.** malkuthsoft.com was published on 2026-11-10 (hero: four mockup
exhibits; the emulated three-systems triptych; the code section with
WinForms C#, GUI.Forms C# and C++ tabs) and republished with all ten images
in windows-professional. They are recaptured once the lockstep SDK is on
main and the professional redesign lands.

**Products** are parked by the owner (commit and stop), each with a
READY_TO_PUSH.md; their GUI.Forms pins move to the post-cp27 main.

## Status 2026-11-09

**Checkpoints.** cp22 (#473) merged on 2026-11-09 with the 0.5 coder batch;
main is c9b11f0a. cp24 (#474) is part 1 of the next fold: the gui-forms
default theme switch and its fallout (525/525 on Windows; the macOS
accessibility fixture fix c6b78b4b waits on the Mac rerun). cp25 (#475)
merged on 2026-11-09 (main e1fc3bf68): the key and numeric ABI, TextBox
styled runs, the focus stack (modal and owned windows, window properties,
focus) and the host file gaps. cp26 is folding the rest: the data grid ABI,
the ribbon gaps, the look integration and the drag engine stack (value bar,
drawing primitives, plots).

**Coded, folding in cp26** (not on main yet; the integrator verifies):
- the declarative drag engine with the node canvas N0 skeleton (NodeGraph
  records, the orthogonal LinkRouter) (2.1, 2.4);
- the value bar and scrub gesture (2.2);
- the drawing primitives: batch and polyline drawing, conic gradient, the
  new compositing pieces and the `gd_*` additions at gd 0.3 (2.3, 1.7);
- the plots package (2.3).

**C ABI additions** (additive over the sealed 1.1 baseline, driven by the
SDR# swap-in; row 3.0): key interactions and payloads with `windows.key_state`
and the NumericUpDown range as schema properties; modal and owned windows;
window properties (Form chrome); keyboard focus with focus restore and
composite fields (the edit part takes the caret, the composite is the
active control; `focusable` is a schema property); the data grid; TextBox
styled runs.

**SDR# swap-in** (row 5.5, moved forward): facade-v1 runs on `gf_abi` and
covers 927 of 933 members of SDR#'s swapped surface. The launcher's P0 and
P1 are done (preflight, exact-name substitution, stubs, capture). DockPanel
Suite runs as its own code on our facade (option a). The goal is SDR#
working tonight (2026-11-09). It ships as a custom zip for Youssef Touil,
with a GUI.Forms theme package that looks just like SDR#.

**Tools.** All the tools live in the private repository
`malkuth-software/malkuth-tools`: the theme editor, the WYSIWYG designer,
the Visual Studio integration and the swap-in launcher. Binaries come from
the website, not the repository. The tools ship with "MALKUTH SOFTWARE
CONSULTING" in the title bar plus one line along the top. There is no
splash and no nag (owner, 2026-11-09; the logo splash was dropped). The
licence is freeware, single seat, restricted to protect consulting hours;
the wording is open. The website repository `malkuth-software/malkuthsoft.com`
needs screenshots of the template examples.

**Theming is NOT done** (owner, 2026-11-09). The look studies (RIBBON_STUDY,
the scaling study, RB decisions 1-33, the design language, T3, T4, the
docking and PageView studies) are not fully integrated. The look
integration (row 2.10, `LOOK_INTEGRATION_LEDGER.md`) has 87 rows: 56 were
already implemented, 23 are implemented on its branch, and 8 are left (2
partial, 3 missing, 1 contradicted, 1 not audited, 1 decided as §4). Seven
owner questions (N1-N7) are open. The ribbon gaps found by the Plan Paint
port, and Plan Paint's three tab features (the lip, the faded neighbour
shadow with taper, the right folder taper), are in it as theme
capabilities. The visual target is the owner's Plan Paint ribbon reference
screenshot.

**Products.** Plan Paint, PlaySuite and SwiftEdit are each ported by their
own worker; their builds are pending.
- Plan Paint: the .PNG defect (trailing bytes after IEND, not case) is
  fixed; the `stx_conv` line API is adopted; the app is Application-first
  with a theme package; the ribbon is option b (gf::Ribbon with stock
  galleries, its own panels in overlays).
- PlaySuite: `is_key_down`, the playsuite theme package, and a gap list.
- SwiftEdit: the reusable highlighter (CSS, form and inline) and the
  CodeEditor for the theme editor.
- cp25's key and styled-run items are on main (#475), so PlaySuite and
  SwiftEdit can now build against it.

**Mac** (`MAC_HANDOFF_2026-11-09.md`):
- the #474 macOS accessibility rerun;
- the AppKit paths of the focus stack (window properties at attach, modal
  focus return, composite focus) and the Finder open-files hook;
- the standing post-0.5 list: smooth-scroll feel, Cmd-Q, logout, live
  resize, the C8 macOS rows, and the engine_motion fade hash;
- an optional macOS build of the cp25 WIP.

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

| # | Item | Depends on | Kind | U | Notes | Status 2026-11-09 |
|---|---|---|---|---|---|---|
| 0.1 | Mac handoff: Cmd-Q, logout, the resize check, T5 C8's macOS rows | the seal head on the M4 | M+O+B | 0.5-1 | first Mac session; records under `results/` | open: `MAC_HANDOFF_2026-11-09.md` (adds the #474 rerun, the focus stack's AppKit paths, smooth-scroll feel) |
| 0.2 | template_harness example on the 0.5 shape | harness pins on main | C+B | 1-1.5 | harness PR by hand (owner rule) | coded (example-app-0.5 6853272); the harness PR by hand and the 0.5 SDK pin are open |
| 0.3 | Plan Paint opens `.PNG`: case-insensitive extension matching | none | C+B | 0.25 | a defect; take it now, not with the revision | **done**: the cause was trailing bytes after IEND, not case; fixed on the Plan Paint branch |
| 0.4 | Splash, further: baking on a worker thread; menus as their own Splash kind | none | C+B+O | 1.5-2 | perceptual check by the owner (snappiness, slide) | coded: tooltip baking on the run's worker thread (`claude/splash-baking` 703d7ac87, in cp27 #480); menus stay Popups (Part S) |
| 0.5 | Record refresh: stage index, board, track pointers; this roadmap folded into `docs/dev/roadmap.md` | owner accepts this page | C | 0.5 | docs-only | in progress: this page and the coordinator desk refreshed 2026-11-09; `docs/dev/roadmap.md` and the board still to follow |
| 0.6 | Owner discussions: the theme designer, the tools' licence class | none | O | - | feed Block 5 and question 3, 4 | **done** for the theme editor (the owner's POST_0.5 instructions); licence: freeware single seat, wording open |
| 0.7 | cp24 (#474): the gui-forms default theme switch | the Mac rerun | B+M | 0.5 | part 1 of the post-0.5 fold | in CI; Windows 525/525 |
| 0.8 | cp25 fold: the queued C ABI additions, products' prerequisites and Block 2 code | 0.7 | B | 3-4 | integrator, order in `INTEGRATION_QUEUE.md` | merged (#475, e1fc3bf68); cp26 merged (#476) |
| 0.9 | 0.5 release tail: ABI re-baseline and blocking audit, the 0.5 tag and SDK release, docs/wiki publication, roadmap.svg | none | C+B+O | 1 | gates nothing | open |

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

| # | Study | Source | Feeds | Kind | U | Status 2026-11-09 |
|---|---|---|---|---|---|---|
| 1.1 | Declarative dragging *(required)* | declarative-dragging.github.io; Horowitz, Blinn and Heer, UIST 2026 | 2.1, 2.4, DK drag, list reorder | S+O | 0.5-1 | **done** (`DECLARATIVE_DRAGGING_STUDY.md`); recommendations adopted |
| 1.2 | BUI node engine *(required)* | github.com/ppmpreetham/BUI (finish reading) | 2.4, D1 | S+O | 1 | **done** (`NODE_ENGINE_AND_VALUE_BAR_STUDY.md`, clean-room); node canvas = records |
| 1.3 | Dear ImGui: controls and effects *(required)* | imgui_demo.cpp | 2.5, 2.6, D3 | S+O | 1 | **done** (`IMGUI_IMPLOT_STUDY.md`) |
| 1.4 | ImPlot: plot layer and primitives *(required)* | ImPlot demo and API | 2.3, D4, D5 | S+O | 1 | **done** (`IMGUI_IMPLOT_STUDY.md`); batch/polyline drawing and conic gradient accepted before the drawing seal |
| 1.5 | Blender's value bars *(required)* | Blender, BUI | 2.2 | S+O | 0.5 | **done** (with 1.2) |
| 1.6 | Mirrored-mode feasibility *(required)* | .NET hosting, facades, startup hooks | 5.5 | S+W+O | 1-1.5 | **done** (`MIRRORED_MODE_STUDY.md`); P0 GO on .NET 10 (hostfxr TPA swap); R2R composite needs a preflight |
| 1.7 | Drawing ABI inventory (`gd_*` before its seal) | the drawing headers, the above studies | 3.2 | S | 0.5 | partly: the additions are in gd 0.3 (2.3); the full inventory before 3.2 is open |
| 1.8 | Help system: CHM-shaped viewer on Plan Paint paper; help as translatable text records | CHM public documentation and behaviour (clean-room), Plan Paint's help | 2.13-2.20, 3.7, 4.12 | S+O | 0.5 | **done** (`HELP_SYSTEM_STUDY.md`, revision 1); owner rulings 1-7, A(a), B(a) recorded; its questions 1-3 open (question 14 below) |

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

Block 1 total: about 6-7 U, all coder or study work, plus owner review.

## Block 2: engine and final controls (Wave B)

| # | Item | Depends on | Kind | U | Parallel with | Status 2026-11-09 |
|---|---|---|---|---|---|---|
| 2.1 | Declarative drag engine *(study first: 1.1)* | DK's drag session | C, then B | 3-4 | 2.2, 2.3, 2.5 | coded (drag-engine 850bf218: engine core, Splitter and SplitContainer on it, drag announcements); in cp27 (#480) |
| 2.2 | Value bar (Blender-style) and drag-to-scrub *(study first: 1.5)* | the reduced-motion gate | C, then B | 1-1.5 | all | coded (value-bar 1bfa4a94: ValueBar, ScrubGesture); in cp27 (#480) |
| 2.3 | Plot primitives, then the declarative plot layer, sparkline *(study first: 1.4)* | 1.7 | C, then B | 5-6 | all | in cp27 (#480): drawing primitives (32cd5cbd) and the plots package (d05561e4), with the drag engine and value bar (rows 2.1, 2.2). Backlog: a bulk FormSources path so `.form` bind() can feed plot series |
| 2.4 | Node canvas: blueprint backgrounds, magnetic curved links, double-click edit *(study first: 1.2, 1.1)* | 2.1, pattern fills | C, then B | 4-5 | 2.3 | N0 coded (NodeGraph records, LinkRouter, in 850bf218); the canvas, backgrounds and port lamps (a new lamp part; green-red indicator style) open |
| 2.5 | Final controls: W-1 to W-3, W-5b to W-9, and the superset's P1 rest (callout, wizard and step indicator, gauges, dial, seven-segment readout, lookup combo, in-window modal layer, About) | ImGui ranking (1.3) for any additions | C, then B | 8-12 | all | coded (`claude/final-controls` e5e4e2d6b, in cp27 #480), with the arc, needle, beak and scrim parts (option a); deferred W-1, W-2, W-3, W-7, W-8 (`planning/GAPS.md` W1) |
| 2.6 | Effects taken from Dear ImGui, as effect programs *(study first: 1.3)* | EP | C, then B | 1-2 | all | open (study done) |
| 2.7 | Active border phase 2: the control border | none | C, then B, O | 2.5 | all | open |
| 2.8 | Active border phase 3: the window band, host shaping, band textures and effects | pattern fills (2.4) | C, then B, W, M, O | 6.5 | 2.5 | the band itself landed in 0.5 (window-band, no resize edge); host shaping, textures and effects open |
| 2.9 | DK and RB phases still open (to audit against their plans) | - | C, then B | audit first | all | open; RB's look rows are in 2.10 |
| 2.10 | **Theming: the look integration** (owner, 2026-11-09: theming is not done) | the look studies in notes | C, then B, O | 3-5 | all | coded (`claude/look-integration` f528d4f76, in cp27 #480): 28 rows implemented, N1-N7 ruled (`planning/DESIGN_LANGUAGE_SPECIFICATION.md` §13); leftovers in `planning/GAPS.md` T3; the windows-professional redesign follows (`claude/professional-theme`) |
| 2.11 | Ribbon behaviour still "not yet": the minimized ribbon's temporary popup, the QAT on the tab row, the help question mark with a window help card | 2.10; the card through 2.19 (design: 1.8) | C, then B | 1.5-2 | all | open: ruled a follow-up RB item (N3); the help "?" part reserved (N4) |
| 2.12 | Per-tab transitions: key PageStrip and TabControl tab parts by page | T4 | C, then B | 0.5 | all | backlog (cp24 ruling) |
| 2.13 | Help H1: `help-line`, `help` (markdown) and `keywords` fields in the text record; locale XML, template export, user overrides, the way back; HelpProvider as a view onto them *(study: 1.8)* | the language-file implementation | C, then B | 1.5 | all | coded (`claude/help-system` ecf4167f4, in cp27 #480) |
| 2.14 | Help H2: stand-alone topic records and the contents tree generated from the skeleton walk (windows, panels and groups, controls) | 2.13 | C, then B | 1 | all | coded (`claude/help-system`, in cp27 #480) |
| 2.15 | Help H3: the helpbook section in the language-file compiler (link and image validation, keyword and search indexes, contents; same pass at load for user files) | 2.13, 2.14 | C, then B | 1.5 | all | coded (`claude/help-system`, in cp27 #480) |
| 2.16 | Help H4: `HelpBook`, the paper topic pane on theme parts (collapsible sections, callouts, stable-ID links, PNG images) | 2.15, 2.10 | C, then B, O | 1.5 | all | coded (`claude/help-system`, in cp27 #480) |
| 2.17 | Help H5: `HelpBookViewer` and `HelpBookWindow` (CHM tabs, history, bookmarks, sync, toolbar), accessibility on three hosts | 2.16 | C, then B, W, M | 2-3 | all | coded (`claude/help-system`, in cp27 #480) |
| 2.18 | Help H6: search v1 (term table, AND, phrases, titles-only), measured on 2,000 records | 2.15, 2.17 | C, then B | 0.5-1 | all | coded (`claude/help-system`, in cp27 #480); ASCII folding only |
| 2.19 | Help H7: F1 through records, default handler for HelpProvider and RibbonToolTip, ribbon "?" and window card, What's This and the paper popup | 2.17; closes 2.11's card | C, then B | 1 | all | coded (`claude/help-system`, in cp27 #480) |
| 2.20 | Help H8: printing a page and a book | 2.17, print path | C, then B | 0.5 | all | coded as a print layout (`claude/help-system`); device printing deferred (no print path) |

Coder work is most of Block 2 and can run four or five rows at a time; the
verification lands in two or three integrator checkpoints. Host shaping
(2.8) needs a Windows 11 host and the Mac; the owner reviews the control
border and the band visually. The band follows the owner's 2026-11-07
answers (fixed reservation, textures on the band only, the focus ring stays
on its own path) and 2026-11-08's (no resize-edge role).

The help rows (2.13-2.20) follow `HELP_SYSTEM_STUDY.md`; help after 1.0
(fast-search layouts for its term table, greaseboard tours) is H11 there.

Block 2 total: about 40.5-51 U, of which about three quarters is coder work.

## Block 3: ABI additions and C# (Wave B into C)

| # | Item | Depends on | Kind | U | Notes |
|---|---|---|---|---|---|
| 3.0 | C ABI additions for the swap-in: keys and numeric, modal and owned windows, window properties, focus (composites, focus restore, `focusable` in the schema), the data grid, TextBox styled runs | the sealed 1.1 baseline | C+B | 3-4 | **partly on main**: keys and numeric, modal and owned windows, window properties, focus and styled runs (cp25, #475); the data grid (cp26, #476); the docking table GF_TABLE_DOCKING, tooltips shown (overlays 7-8) and the diagnostics overlay switch in cp27 (#480); additive; the audit and the version number need a recorded owner decision (question 10) |
| 3.1 | C ABI band declaration: window presentation in the windows table | 2.8's declaration shape | C+B+O | 0.5-1 | additive; needs a recorded owner decision (sealed interface) |
| 3.2 | Drawing ABI seal (`gd_*`) | 1.7, 2.3, 2.4, Block 4's demonstrators | C+B+O | 1-2 | sealed only after the demonstrators stress it |
| 3.3 | R: SDRSharp reflection rebuilt on the current ABI | T4, F | C+B+W+M | 2-3 | precedes S (forward programme); **in progress through the swap-in** (facade-v1 on `gf_abi`, 927/933 members; 5.5) |
| 3.4 | S: generated C# bindings, the WinForms-shaped package, the corpus | 3.3, C | C+B+W+V | 6-10 | its own repository |
| 3.5 | PlaySuite and SwiftEdit as full ports to the C ABI | ABI 1.1 | C+B | 2-3 each | owner, 2026-11-03; both ported forward by their workers (C++ consumers so far), builds against main (cp25 landed) pending |
| 3.6 | 1.0 conditions: normative traces on Windows, macOS and X11; the member manifest; two-consumer conformance | 3.5 | B+W+M | 2-3 | release-1.0 conditions 2 to 4 |
| 3.7 | Help H9: C ABI `help` table, the help fields in the schema, the C# binding | 2.19, 3.0 | C+B+O | 0.5 | joins ABI 1.2 with 3.0 (question 10) |

**C ABI band declaration.** What we take: C and C# programs can declare a
window band, which today only C++ can, because the windows table has no
window presentation. Additive, after the band's C++ shape settles, and
landed with the C# work (owner, 2026-11-08). Test: the breaking-change audit
passes against the sealed baseline with only additions; a C conformance
test and a C# interop test declare, light, flash and undeclare a band and
read the reserved rectangle back.

Block 3 total: about 16.5-25.5 U; S is the long pole and is Windows- and
Visual-Studio-bound.

## Block 4: consumers and demonstrators (Wave C)

| # | Item | Depends on | Kind | U | Status 2026-11-09 |
|---|---|---|---|---|---|
| 4.1 | SwiftEdit revised to the new standard and interface; syntax highlighting, CSS first | 3.5, a styled-run seam in the text editor | C+B+O | 2-3 | complete (SwiftEdit e411177, theme editor 6dabb58); parked by the owner; pins move to the post-cp27 main |
| 4.2 | Plan Paint: the canonical ribbon with soft large bubble buttons; robust file handling; the new `stx_conv` algorithm | RB, 0.3 | C+B+O | 2.5-3 | complete (claude/png-open-fix fcdf6b6, -look 30e3a11, bb7702a; dialogs-as-windows on claude/combined-build); parked; language packs need a native-speaker check |
| 4.3 | PlaySuite migrated forward: custom skinning through themes, with a gap list | 3.5 | C+B+O | 2-3 | complete (claude/playsuite-cp22 c181871, 25 games pass the play check); parked; tile hover blink open (`planning/GAPS.md` P5) |
| 4.4 | D1 node graph, D3 ImGui-class and D4 ImPlot-class demonstrators | 2.4, 2.5, 2.6, 2.3 | C+B | 4-5 | open |
| 4.5 | D2 flame graph and profiler viewer | 2.3 | C+B | 2 | open |
| 4.6 | D5 ImPlot3D-class plotting | owner relicensing of PlaySuite's 3D parts to MIT | O, then C+B | 2-3 | open |
| 4.7 | File Manager frontend, benchmark demonstrations | per their plans | C+B | per plan | open |
| 4.8 | Emscripten build | `stx_skia` | C+B | 3-5 | open |
| 4.9 | First exhibits: E2 oscilloscope, E4 process control, E14 wizard, E9 data entry | 2.3, 2.5 | C+B | 1-2 each | in progress: mockups E2, E4, E8, E9 (owner scope 2026-11-09, `planning/EXHIBITS_SELECTION.md`); working apps later; 12 GUI.Forms gaps in `planning/GAPS.md` |
| 4.10 | Website: template example screenshots for `malkuth-software/malkuthsoft.com` | 0.2, the examples | C+O | 0.5 | published 2026-11-10 (malkuthsoft.com, all images in windows-professional); recapture against the lockstep SDK after cp27 and the professional redesign |
| 4.11 | GUI.Forms gaps from the product ports: macOS open-files from Finder, Linux dialog filters ignoring case | none | C+B+M | 0.5 | landed (host-file-gaps 1744d23b3, cp25 #475); Mac check in the handoff |
| 4.12 | Help H10: Plan Paint's help as records (replaces `help_content.hpp`, converts its JSON help entries), the library HelpBook, F1 to context pages | 2.19, the Plan Paint locale migration | C, then B, O | 1 | 4.2 | open |

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
recorded. Block 4 total: about 19-25 U plus the File Manager and the exhibits.

## Block 5: the tools (after 1.0 backend)

Closed source, in the private repository `malkuth-software/malkuth-tools`;
binaries come from the website (owner, 2026-11-09). There is no nag screen
and no splash: each tool shows "MALKUTH SOFTWARE CONSULTING" in its title bar
and one line along the top, through one shared helper (owner, 2026-11-09;
this supersedes the 2026-10-27 nagware screen). The licence is freeware,
single seat, restricted to protect consulting hours; its wording is open.
Certificates (Authenticode, an Apple Developer ID) come before the first
distribution. The swap-in launcher (5.5) has moved forward: SDR# is its first
real target now, not after 1.0 (backend).

| # | Tool | Depends on | Kind | U | Status 2026-11-09 |
|---|---|---|---|---|---|
| 5.0 | Licence wording, certificates, the title-bar branding line and About | owner | O, then C | 0.5-1 | branding decided; licence wording open |
| 5.1 | Theme editor: the screencap utensil and the visual editor | 4.1, T4 provenance | C+B+O | 5-6 | started (malkuth-tools claude/theme-editor a53dd6c, on SwiftEdit's CodeEditor); v1 uses the winning-rule query |
| 5.2 | WYSIWYG designer (D) with in-line theme override | 2.1, L, FD, 5.1's rule panel | C+B+O | 8-12 | open |
| 5.3 | Visual Studio integration and package, for VS and C# | 3.4, 5.2 | C+B+W+V | 3-5 | open |
| 5.4 | Windows Forms conversion program | 3.4 | C+B+W | 3-4 | open |
| 5.5 | WinForms mirrored-mode launcher *(study first: 1.6)* | 3.0 (not 3.4 for SDR#) | C+B+W | 4-8 | **in progress**: proof of concept 2026-11-09 (real SDR# with a live radio); rulings: look not mechanics, fail fast (`planning/stages/S-csharp-swap-in/README.md`); the release zip is built after the combined build; open list in `planning/GAPS.md` |
| 5.6 | Exhibits continue; E12, the .NET open-source port | 3.4 | C+B+W | per exhibit | open |

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
| 1 Studies | 6-7 | all, plus owner review |
| 2 Engine and final controls | 40.5-51 | about three quarters |
| 3 ABI and C# | 16.5-25.5 | about half (Windows- and VS-bound) |
| 4 Consumers and demonstrators | 19-25, plus exhibits | about two thirds |
| 5 Tools | 24-36 | about half |
| Formal 1.0 | 6.5-10.5 | about half |

The coder share can go faster under code-first, verify-later; the rest is
bounded by integrator checkpoints, hosts and owner review.

## Open questions for the owner

Answered on 2026-11-08 (kept for the record): 1. 1.0 is when everything is
done; no separate definition. 3. The theme editor is the owner's
POST_0.5 instructions; drop the "V designer" framing. 4. The tools stay in a
private repository; licence: freeware single seat (2026-11-09), wording
open. 5. Clean-room, as recommended; also study which controls BUI and
Blender produce and the compositing their widgets need. 6. .NET 10.0.
7. An internal engine, dogfed on the node engine. 8. Plots as a package.
9. Band declaration as ABI 1.2.

Still open:

2. **Which two conformance consumers?** (below, unchanged)
10. **Versioning the C ABI additions (3.0).** The swap-in additions are
    additive over the sealed 1.1 baseline. Recommendation: they join the
    band declaration in ABI 1.2, recorded as one owner decision, with the
    blocking audit passing on additions only.
11. **The tools' licence wording:** scope (a/b/c) and seat (a/b), and the
    text of the line under the title bar.
12. **The look integration's N1-N7** (`LOOK_INTEGRATION_LEDGER.md`): rail
    colour, tab heights, the ribbon's "not yet" behaviour, the help question
    mark, the default theme base, per-field cascade, which screenshots are
    decisions.
13. **DockPanel Suite under the swap-in:** option a now (its own code on our
    facade). Recommendation: the shim over DK (option b) after SDR# runs.
14. **Help system, revision 1's questions:** answered by the owner
    (2026-11-09), so the study is accepted. Built-in help bodies live in an
    embedded built-in locale file (short help lines may stay in code).
    Controls without a body are left out of the contents, with their help
    line shown on the parent's page. F1 with no help walks up to the window
    page, then home.

The first version's questions, as asked:

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
