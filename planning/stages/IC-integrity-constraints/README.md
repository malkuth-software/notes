# IC: unified behavioural constraints, and an engine that checks them

Written by the coordinator, 2026-11-21, from the owner's direction of the
same day. Covers GUI.Forms and STXlib. Status: a study and plan; nothing
below is decided beyond the owner's direction.

## 1. The owner's direction (2026-11-21)

- The owner spent several rounds with a Mythos unit building an
  optimistically constrained solver to prove the allocator valid, and is
  only about half convinced it amounts to formal verification.
- A recent fuzzing approach, **STITCH** (Harrison Green; the owner's link is
  a post on X, not read here), looks better than our fuzzing. As the owner
  describes it: instead of evaluating all permutations, each block of code is
  permuted independently and the call graph is built bottom up; to avoid
  false crashes from violating an API's usage rules, it adds **extrinsic
  typestate**: metadata attached to the objects passed between blocks, which
  blocks set and read. Without changing the fuzzing engine, blocks can then
  enforce precise constraints (subtyping, state machines, reference counts,
  parent and child ownership, and many more), and the approach generalised to
  hundreds of real projects.
- GUI.Forms and STXlib already constrain behaviour precisely, but not in
  one unified way. **We need not only a house style but unified
  constraints.**
- Study what Fable wrote to evaluate the allocator and what Mythos wrote,
  derive our own reasonable engine, and extend it to the rest of the
  codebase. Some minor code clean-up or refinement may follow. It is part of
  the integrity work: GUI.Forms is one monolithic C++ system and can be made
  highly reliable.

## 2. What exists today

| Kind | Where |
|---|---|
| Spelling and mechanical rules | `tools/check_house_style.py`, `tools/check_lifetime_rules.py` (borrows, looked-up function types), `tools/check_std_rules.py` |
| Declared surfaces | the control registry and schema, `manifests/members.json`, the C ABI class tables and their ledger, export lists |
| State machines | the state-machine gate (`--changed-from origin/main`), the house style's rule that switches keep one fallback |
| Host behaviour | the normative traces (release 1.0 condition 4) |
| Runtime integrity | integrity counters (`integrity_counts`), checked builds, the high-integrity build option, `code_integrity_checker/` |
| Fuzzing | libFuzzer entry points (`fuzz/`, `GUI_FORMS_BUILD_FUZZERS`) for parsers |
| Proofs (STXlib) | the verification programme (`planning/VERIFICATION_PROGRAMME.md` on the STXlib notes), ESBMC proof rows (`proofs/b1/rows/`), and the allocator's concurrency proof: `stx_minmallocpp/dev/concurrency_audit/` ("sxv"): the page protocol as a counter system with an inductive invariant, discharged by z3 on the M4 in about 32 s, plus exploration and sweeps kept as tests |
| Reviews (STXlib) | the allocator's design and reviews (`planning/stx_minmallocpp/DESIGN.md`, `review-logic.md`, `review-compile-style.md`) |
| The Mythos solver | the owner's investigatory runs; unfinished, found no flaw so far; **not to be copied into a repository** (owner), only studied |

Each states constraints its own way: a header comment, a Python check, a
z3 model, a fuzz harness, a state-machine table. Nothing ties a function's
preconditions, an object's typestate and an ownership rule to one
description that every checker reads.

## 3. The plan

**3.1 Study (code-free).**
- Compare the allocator evaluations: the sxv proof (what it proves, its
  stated premises, what it leaves to assumption), the reviews, and the
  Mythos solver's approach; say plainly what each establishes and how far
  each is from formal verification.
- Read the STITCH work at its source (the paper or repository behind the
  post) and set out its block model and extrinsic typestate precisely.

**3.2 One constraint vocabulary.** Constraints written once, next to the
API, in a form every checker reads. Candidates for what it must express:
- preconditions and postconditions on calls (ranges, non-null, thread);
- **typestate**: the states an object passes through and which calls each
  state admits (a window before attach, attached, closing, closed; a
  dispatcher accepting or shut down; a borrowed view valid or released);
- ownership: parent and child, borrower after owner, transfer;
- reference counts and revocation (stale identities rejected, revoked
  callbacks stopped);
- execution context: which thread or dispatcher may call.
GUI.Forms already enumerates its surface (registry, member manifest, ABI
class tables), so the vocabulary can hang off those rather than off free
text; STXlib's headers get the same.

**3.3 Our engine.** A block-based API fuzzer in STITCH's manner, driven by
the vocabulary: small code blocks per API call, permuted independently,
composed bottom up into call graphs, with the typestate kept as metadata on
the objects passed between blocks so that only legal sequences run. Every
crash, sanitizer report or integrity stop it then finds is a real defect,
not a misuse. It runs against the sanitizer builds (see the background
sanitizer brief, `planning/stages/CI-background-sanitizers/README.md`).

**3.4 Proofs where they pay.** Where a component is small and critical (the
allocator's page protocol, the dispatcher's queues, the ratchet of
`stx_filesign`), the same constraints feed a decision procedure, as the sxv
proof does today.

**3.5 Order.** STXlib first (the allocator, containers, files and the
platform layer), then GUI.Forms (control and window lifecycle, the
dispatcher, the C ABI tables, the property capabilities), then the
products' use of the public seams. Minor clean-up where two places state one
constraint differently.

## 4. Placement

Order of operations step 6 (alongside the implementation, as integrity
work), continued in step 10's integrity work. Shares the sanitizer builds
with the background sanitizer runs.
