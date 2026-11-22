# IC study: unified behavioural constraints and the block fuzzer

Written by the VA/IC study worker, 2026-11-22, from the brief beside it
(`README.md`). Read: GUI.Forms at `main` c7c9f4f0c, STXlib at `main`
1de8f3c, template_harness at 335f5b8, the STXlib planning notes (the
`stxlib` branch of the notes repository, 1496fd4), the allocator's proof
tools recovered from the M4 (section 2.2), the session records of the
sessions that wrote them, and the STITCH paper. Code-free: nothing was
changed in any repository. Status: proposals and questions for the owner.

## 1. The problem

GUI.Forms and STXlib constrain behaviour precisely, but each constraint
lives in its own form: a prose header comment ("std::logic_error off its
thread or once closed"), a Python check, a z3 model, a fuzz harness, a
state-machine table, a JSON property family. No checker can read another's
constraints, and nothing ties a call's preconditions, an object's typestate
and an ownership rule to one description. The owner's direction
(2026-11-21): not only a house style but **unified constraints**; study
what Fable and Mythos wrote for the allocator, derive our own engine, and
extend it to the whole codebase, with a STITCH-style block fuzzer.

## 2. The owner record, and what the brief gets wrong

### 2.1 The owner's stance, from earlier records

- **"The representation is the model"** (2026-11-15, Fable's allocator
  session): do not verify a separate model checked for conformance; trace
  the C++ itself as a graph of entries and exits with bounded cost. The
  same record: the owner intends this tracing for STXlib and GUI.Forms after
  the allocator, as "an engine that is efficient, that we then extend with
  behavior": this stage's own wording, eight days earlier.
- **A proof is deductive** (2026-11-18, Mythos' allocator session): "a
  proof of correctness cannot lie in a proof created by evaluating a finite
  set of possible states, nor in deliberate breaking or sweeping."
  Exploration, sweeps and planted breakage are tests and must be labelled
  as tests. A slow solver means a wrong encoding: change the mathematics.
- **No whole-program solver runs**: an earlier attempt ran for hours; small
  closed graphs and per-node facts instead.
- High-integrity documentation (2026-11-16) is to describe "instrumentation
  that fable used on our allocator" and the Mythos audits, and to say
  plainly that this is high integrity by design, **not formal proof**.

### 2.2 Corrections to the brief

1. **The sxv proof and "the Mythos solver" are the same work.** The brief
   lists them as two rows. The session that wrote `concurrency_audit`
   (sxv, the counter system, the closed model) ran on Mythos 5.2 from
   2026-11-18; it is the "optimistically constrained solver" the owner
   describes. Fable's allocator work is a different set of instruments
   (section 3.2).
2. **The sxv path is stale and the tool is in no repository.** The brief's
   `stx_minmallocpp/dev/concurrency_audit/` exists nowhere. The tool was
   never committed: the 2026-11-21 tidy rescued it from the gone
   `stxlib-minmallocpp` checkout into a thin git bundle on the M4
   (`tidy-bundles-2026-11-21/…stx_minmallocpp_stxlib.bundle`, branch
   `rescue/2026-11-21-stxlib-minmallocpp` 715cd66, path
   `tools/concurrency_audit/`, 21 files, about 7,000 lines; it needs
   STXlib's history to unpack). **That bundle is the only copy.** Given the
   owner's rule that the Mythos solver is not copied into a repository,
   this may be deliberate; either way it should be kept somewhere safer
   than a temporary bundle (question 8).
3. **The STXlib notes.** The brief points at "the STXlib notes"; since
   2026-11-21 they are in the private `stxlib_notes` repository, of which
   this machine has no clone. I read the copies still on the public notes
   repository's `stxlib` branch (`VERIFICATION_PROGRAMME.md`,
   `stx_minmallocpp/DESIGN.md`, `review-logic.md`,
   `review-compile-style.md`); they may be older than the private ones.
4. Everything else the brief names exists.

## 3. The allocator evaluations, and how far each is from formal verification

### 3.1 Mythos: sxv (deductive, relative to named premises)

What it is: a front end for the house dialect (tokenizer, parser,
control-flow graph, `sxv_front.py`) that reads ten step routines of the
page protocol **from the source**, cuts them at every
`STX_MINMALLOCPP_STEP` mark, call and unmarked loop, executes each path
symbolically (64-bit symbolic words, unbounded integer counters, a relaxed
load as an arbitrary value) and sends 2,444 conditions to z3: each path is
an enabled transition of a counter system whose invariant J (7 clauses) is
inductive (285 obligations); each thread's role assertion survives every
other thread's step (417); 21 call-site conditions; and soundness checks
(no condition proved from contradictory premises). All proved, 35 s on the
M4. The key move is mathematical: counters as finite sums with three
elementary laws, and popcount by its algebra, not as a 64-bit circuit
(which never finished).

What it establishes: for any number of threads, pages and blocks and any
interleaving, the page protocol as written never reaches an integrity stop,
never refuses a held block's release, and keeps J. That is a genuine
machine-checked safety proof.

How far from formal verification of the code:
- **Trusted extraction.** The source is read by a hand-written parser of a
  dialect subset, not by the compiler; which helper calls write no page
  word (`OPAQUE_CALLS`) was decided by hand. A misreading would prove the
  wrong program.
- **Assumed premises.** The C++ memory model's coherence, each segment
  between marks being atomic, sequentially consistent operations being
  linearizable; the loop invariant of `collect` stated, not derived; the
  16-bit count treated as an integer (safe below about 61,000 threads mid
  release of one page; the source's static assertion covers 4,096); the
  queue and abandoned-list contracts assumed.
- **Scope.** The page protocol only: not the orphan heap, huge mappings, the
  free-run index, commit units, queues, the release path before the page,
  the hosts, or progress (safety only).

Verdict: formal verification of the **page protocol as extracted**, with an
unverified extractor and named premises; "optimistic" is fair where the
premises cover what is not proved (the queues). It is the strongest
evidence the project has, and its method (dialect front end, path cutting
at marks, algebraic encodings) is reusable.

The closed model beside it (`closed.py`, `explore.cpp`: every state of 2
threads by 1 to 2 slots to a fixed point, the largest 10 million states) is
a test by the owner's definition and the README says so.

### 3.2 Fable: the graph, the checks, the interleaving checker

- **`proofs/stx_minmallocpp_graph.md`**: every entry, exit (condition,
  writes before it, result), leaf host call and cycle with its bound.
  Written by reading, "nothing here has been run". It is the specification
  sxv and the tests both need, but it is not checked against the code.
- **`tools/check_object_surface.py`** (component file
  `tools/object_surface/stx_minmallocpp.toml`): the compiled objects reach
  only the listed host calls. Mechanical and real, on the binaries.
- **`STX_MINMALLOCPP_CHECK`**: entry and exit checks on in the direct tests
  and a Debug build (local facts: ranges, shifts, sums).
- **`tests/stx_minmallocpp_interleave.cpp`**: the allocator compiled with
  each step mark turned into a scheduler yield; every schedule of fixed
  scenarios with 2 threads and up to `preemptions` forced switches, each in
  a fresh process, checking integrity stops, wrongful refusals, deadlock
  and lost pages. Exhaustive within the bound, on the compiled code, under
  sequential consistency: a test, and a good one.

Verdict: shape facts checked mechanically, local facts checked at run time,
concurrency tested exhaustively within a small bound. None of it is a
proof of the protocol; together with sxv it is unusually strong.

### 3.3 The reviews

`review-logic.md` (commit f8cf9c6, by reading: two defects and twelve risks
"proved" as arguments from the code, two "suspected") and
`review-compile-style.md` (by reading). Careful human-style arguments, not
mechanical; valuable, not verification.

### 3.4 The B1 rows

`proofs/b1/`: 24 stx_auxlib functions with properties, bounds, planted
mutants and vacuity probes under ESBMC, CBMC, cpp-verify and GenMC: the
tool evaluation of `VERIFICATION_PROGRAMME.md` (levels P0 to P4). Bounded
model checking is a proof only when every loop is fully unwound or
discharged by induction; the programme already says so.

### 3.5 The lesson

Each instrument needed the same facts and wrote them down separately: the
graph's exit tables, the CHECK macros, sxv's role assertions and call
conditions, the interleaving checker's scenario rules, the review's
statements. The unified vocabulary is the thing all five would have read.

## 4. STITCH, at its source

Green, Brown and Le Goues (Carnegie Mellon), "Automatic, Expressive, and
Scalable Fuzzing with Stitching", arXiv 2602.18689 (2026-02-21), read in
its HTML form. The tool is said to be at `github.com/hgarrereyn/STITCH`
(not verified: the page text was cut there).

- **Code blocks.** A block is native code (a C/C++ function body), typed
  inputs, typed outputs and a space of fuzzable parameters; it describes how
  to call one library function. Preconditions are ordinary code inside the
  block. Specifications are JSON (name, inputs, outputs, code).
- **Static typing between blocks.** Inputs and outputs have types from a
  finite set (canonical object types, with aliases merged); a testcase is
  well formed when references point backward, each output is used at most
  once, and types match.
- **Extrinsic typestate.** Key-value metadata per object and globally,
  persisting across blocks (`FUZZ_GET_ATTR_INT/STR/PTR` and setters). Blocks
  write facts ("this is an array") and later blocks read them. State
  machines, reference counts and ownership are not built in: each block
  encodes them by hand. A failed precondition calls `FUZZ_BAIL`: the run
  stops cleanly there (not a crash) and the testcase is kept up to that
  point.
- **Assembly.** A corpus entry is a whole testcase (a sequence of block
  instances with references to earlier outputs). Mutations: regenerate (a
  random block plus producers for its inputs: bottom up), crossover,
  extend or repair at the bail frontier; parameters mutated by LibAFL's
  byte mutators. LibAFL in fork-server mode.
- **LLMs** set projects up, scan APIs, write and compile-check blocks,
  triage crashes (security, stability, misuse) and patch blocks; hand
  specifications are possible but described as laborious.
- **Results.** Highest coverage on 21 of 33 benchmarks; 30 true bugs
  against 10 for all other tools; 70 % precision after triage; 131 new bugs
  in 102 of 1,365 projects. C and C++ only; no treatment of threads,
  callbacks or global state.

What carries over: the testcase-as-program model, typed object passing,
bail instead of crash, frontier mutations. What does not: an LLM writing
and repairing constraints (our constraints are already precise and should
be declared once, not re-derived per block), and ad hoc typestate in block
code. STITCH's false positives (30 % after triage) are exactly the
constraint mistakes a declared vocabulary removes.

## 5. What GUI.Forms and STXlib already declare

| Carrier | Constraint it holds | Machine-readable? |
|---|---|---|
| Header comments | thread, lifecycle ("off its thread or once closed"), failure contracts ("changing nothing") | no |
| `tools/property_families/*.json` (126 families) | each property's type, default, effects, enumeration; generates capabilities, tables and docs | yes, except validation, which is prose in `comment` ("Throws std::invalid_argument unless both sides are finite and positive") and hand code in `prepare_write` |
| `manifests/members.json` (283 types) | every member, its status, basis and evidence | yes |
| C ABI class tables and ledger | raisers, class ids, segments | yes |
| `check_state_machines.py` (template_harness) | switch fallbacks, contradictory shapes | compiler-backed |
| `check_lifetime_rules.py`, `lifetime_lint.py` | borrow rvalue overloads, looked-up types; clang's lifetime lint | yes |
| `code_integrity_checker/` | review candidates for storage and failure boundaries | heuristic |
| `fuzz/` (17 libFuzzer targets) | parsers and codecs, byte inputs | n/a: no API sequences |
| STXlib `check_object_surface.py` + TOML | reachable host calls per component | yes |
| `STX_MINMALLOCPP_CHECK`, integrity stops | local facts at run time | in code |
| Runtime state enums (24 public `*State` enums, `SubSurfaceState`, …) | an object's lifecycle, observable | yes, but transitions are prose |

The property families are the existing proof of the pattern: one
declaration, several generators. What they lack is a machine-readable
**requires** clause, and nothing like them exists for calls, lifecycles,
threads or ownership.

## 6. Design

### 6.1 The vocabulary

One declaration per component, beside its header, keyed to names that
already exist (registry types, member-manifest members, ABI entries,
STXlib headers). It states:

| Clause | Example |
|---|---|
| `states` | `Window: created, attached, closing, closed` (tied to the public state enum where one exists) |
| `call` + `in` / `moves` | `close(): in attached -> closing`; `attach: in created -> attached` |
| `requires` | `bounds.width in [1, 32768]`, `scale in (0, 8]`, `finite`, `not null` |
| `refuses` | what a call outside its states or ranges does: `logic_error`, `invalid_argument`, `status INVALID_ARGUMENT`, and **what it leaves** (`unchanged`) |
| `thread` | `ui`, `any`, `owner`, `dispatcher` |
| `owns` / `borrows` / `transfers` | parent and child, borrower after owner, `Ptr` moved |
| `identity` | generational; stale refused (named failure) |
| `revokes` | a token whose revocation stops its callback |

Generators read it to write: the header contract comments (as property
comments are generated today), the `prepare_write` and entry checks (so a
stated range and its check cannot differ), the member manifest's
constraint column, the fuzzer's blocks, and proof obligations where a
decision procedure is used.

### 6.2 The engine: a block fuzzer driven by the vocabulary

- **Blocks are generated** from the vocabulary, one per call and legal
  source state, with typed inputs and outputs (handles and values); compound
  blocks (build a form, open a popup with content) are written by hand in
  house style. No LLM in the loop.
- **Typestate is intrinsic to the engine, not to the blocks**: the engine
  keeps each object's declared state as metadata and only schedules a block
  whose `in` states and `requires` hold, so a legal-mode run cannot misuse
  the API. A crash, sanitizer report, integrity stop or refusal in legal
  mode is a defect, in the code or in the vocabulary, never "misuse".
- **Misuse mode** (not in STITCH): the engine deliberately calls blocks out
  of state or out of range and checks the declared refusal: the named
  failure, and the object unchanged (compared through its public state).
  GUI.Forms validates external input, so a misuse that crashes, corrupts or
  is silently accepted is a defect.
- **Oracle per step**: after each block, the object's own reported state
  must equal the engine's predicted state (conformance of code to
  vocabulary), besides the sanitizers (the sanitizer trees), integrity
  stops and checked builds.
- **Testcases are programs**: sequences of block instances with backward
  references; mutations regenerate, cross over, extend and trim at the
  frontier, and mutate parameters; minimised failures are written out as
  ordinary C++ tests for gui_forms_tests.
- **Driver**: libFuzzer (already admitted, `GUI_FORMS_BUILD_FUZZERS`) as
  the coverage and corpus driver, with a custom mutator that edits the
  serialized testcase; the engine itself is ours, in house-style C++ over
  STXlib.
- **Scope limits stated**: one UI thread first (the headless host, the
  dispatcher drained between blocks); threads and sub-surfaces later, with
  the interleaving checker's scheduler as the model.

Results of the fuzzer are **tests**, by the owner's definition, and are
reported as such.

### 6.3 Proofs where they pay

The same clauses become obligations for a decision procedure where a
component is small and critical: the allocator's page protocol (sxv's
domain), the dispatcher's queues, `stx_filesign`'s ratchet. The method is
sxv's: read the source in the house dialect, cut at marked steps, discharge
algebraically. Proof reports follow `VERIFICATION_PROGRAMME.md` section 6
(level, tool, premises, source hash).

## 7. API sketch (illustrative)

A vocabulary file, `include/gui_forms/window/window.ic.toml`:

```toml
component = "Window"
header = "include/gui_forms/window/window.hpp"
states = ["created", "attached", "closing", "closed"]
state_query = "Window::lifecycle"          # does not exist yet (below)

[[call]]
name = "open_native_popup"
in = ["attached"]
thread = "ui"
requires = ["owner.live", "owner.visible", "content.detached_root",
            "bounds.width in [1, 32768]", "bounds.height in [1, 32768]"]
refuses = [{ when = "not owner.live or not content.detached_root", failure = "logic_error", leaves = "unchanged" },
           { when = "bounds out of range", failure = "invalid_argument", leaves = "unchanged" }]
returns = { type = "NativePopupToken", owns = "caller", revokes = "on owner gone" }
```

The engine's block, generated:

```cpp
// Block: Window::open_native_popup, legal mode (generated from window.ic.toml).
BlockOutcome block_window_open_native_popup(BlockContext& context, BlockInputs& inputs) {
    Window& window = inputs.window(0);                 // engine state: attached
    const Control::Ptr& owner = inputs.control(1);     // engine state: live, visible
    const Control::Ptr& content = inputs.control(2);   // engine state: detached root
    const Rect bounds = context.parameter_rect(1.0, 32768.0);
    NativePopupToken token = window.open_native_popup(owner, content, bounds, NativePopupOptions{});
    context.output_token(0, std::move(token));
    return BlockOutcome::completed();
}
```

No STITCH-style metadata reads appear in the block: the engine selected
its inputs because their declared states admitted the call.

The per-step oracle needs every typestate to be observable. `Window` has
no public lifecycle query today (its states live in private flags and
notifications), and neither do several other stateful objects; IC3 adds
one per stateful type, and for the C ABI one per handle kind, which is an
ABI addition and so belongs before the freeze (question 7).

## 8. Phases

| Phase | Content | Repository |
|---|---|---|
| IC1 | the vocabulary's format, parser and generators (comments, checks, manifest column); applied to the allocator, containers and files of STXlib | STXlib, template_harness for the shared tool |
| IC2 | the engine (blocks, testcases, legal and misuse modes, libFuzzer driver, minimiser writing C++ tests), run on STXlib against the sanitizer trees | STXlib tests, then gui_forms_tests |
| IC3 | GUI.Forms: window and control lifecycle, the dispatcher, the property families' `requires` (moved from prose), the C ABI tables | gui_forms, gui_forms_tests |
| IC4 | proofs from the vocabulary where they pay; the existing checkers reading it; clean-up where two places state one constraint differently | both |
| Gate | per component: clauses declared, blocks generated, states and edges reached, defects found and fixed (through `integrity_counts`), and zero legal-mode false positives over a stated run | notes `results/` |

## 9. Questions for the owner

Put in conversation, numbered with lettered options; answers are recorded
here when given.

1. Where constraints are written.
2. The fuzz surface of GUI.Forms (C++ API, C ABI, or both).
3. Misuse mode.
4. The engine's base.
5. Who writes blocks.
6. sxv's status and the deduction layer.
7. Order, against the ABI freeze.
8. Where the STXlib half and the rescued sxv live.
