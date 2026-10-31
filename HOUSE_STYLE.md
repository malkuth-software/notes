**Code style — orthodox, ultra-transparent**

_The sections are ordered by importance, most important first (the owner's 2026-10-27 ranking)._

**1. Memory use**

Give data an owner and a lifetime. Use ordinary objects, dot access, indexing, and typed references whether storage is local or on the heap. Allocation changes storage location; it need not change the computation's vocabulary.

Tie cleanup to the owner's lifetime. Transfer ownership explicitly and leave the former owner empty. Use shared ownership for genuinely shared lifetimes; prevent cycles and keep observational links non-owning. Borrowing grants access without ownership.

State whether inputs and outputs may share storage. Different names and read-only access do not establish disjointness. Keep borrows valid through use: reallocation, replacement, release, and destruction can invalidate them. Removing an element ends its lifetime even if its bytes remain. A moved allocation can retain its address under a new owner. Retaining a borrow across deferred work or mutation requires a lifetime guarantee.

Synchronize shared access at its boundary. Give concurrent computations separate mutable workspaces unless sharing is intentional. Ordinary access then operates within established ownership and mutation rights.

Check size multiplication, shape products, capacity growth, and alignment before use. Match allocation and release. Initialize every region that will be read, including padding; clear only where the next operation needs it. Define empty-state behavior. Publish replacement storage only after initialization succeeds.

Memory errors and undefined behaviour are defects wherever they occur, tests included. Check for them mechanically: run the whole suite under address and undefined-behaviour sanitizers, and treat a finding as a failure.

Keep raw addresses at allocation, borrowing, foreign-call, device, and low-level kernel boundaries. State the extent and lifetime of borrowed memory. Keep size, capacity, access, and release controls on the object so callers need not coordinate them separately.

Pass read-only references for observation and mutable references for modification or output. Work directly: `processor.process(input, output)`, `frame.samples[i]`, `image.clear()`. Bind an owning handle to a typed reference for repeated access within its valid lifetime.

Use contiguous, length-bearing arrays. Index live elements directly; use dot-accessed controls for size, filling, clearing, reservation, and release. Indexing never silently grows storage. A fixed shape can live on the heap; runtime sizing need not imply growth during processing.

A function that returns a borrow of an argument refuses temporaries: delete its rvalue overload, so a borrow of a value that ends with the statement cannot compile. Declare a borrower after what it borrows, so it is destroyed first; an object that reports into another, including from its destructor, must not outlive it.

Keep small temporaries local. Put persistent state and reusable workspaces on their owner. Allocate substantial or runtime-sized storage before repeated processing, then reuse it. Shared scratch fields create dependencies between calls; use them deliberately.

Use standard growable collections for growth and ordinary strings for text. Use specialized heap arrays for substantial buffers edited in place or for specific alignment and allocation needs.

Name storage operations precisely. Fill edits live elements. Reserve preserves values while increasing capacity. Reset-size replaces contents, even at the same size. Clear ends element lifetimes and states whether capacity remains. Release frees storage. Name large copies; moving an owner transfers storage without copying contents.

If allocation failure is reported solely by a Boolean, keep element construction and relocation from escaping through another failure mechanism. Preserve old contents on failed replacement when promised; preparing a replacement may require both allocations at once.

**2. Failure and foreign interfaces**

Assert internal preconditions; validate external inputs. Disabled assertions cannot reject invalid sizes or stale identities. Return complete success or failure states so callers need not infer validity from unrelated fields.

Use explicit results for expected alternatives. Use exceptions at operation or subsystem failure boundaries that permit them. Keep routine hot-path decisions ordinary control flow. Clean up on every exit. State set for the length of an operation (a re-entrancy mark, a depth count, a suspended notification, a held lock) belongs to a named scope object that restores it on every exit, an exception included, even where the code between cannot currently throw; never set and clear it by hand.

Content can exhaust memory; the interface's own structures are not designed for routine exhaustion recovery. Bound every allocation that input sizes (a file, a document, an image, a message) by a stated limit, set by the format or by the caller, and check the input's declared or measured size against it before allocating. Turn a failed content allocation into the operation's named failure at its boundary; the program stays stable and closable.

Specify whether failure leaves output unchanged, empty, partial, or invalid. Validate before mutation where old contents must survive. Prepare replacements before committing and account for peak storage.

Foreign interfaces specify types, counts, ownership, destruction, errors, callback context, and identity validity. Keep language-specific layouts and ownership behind the interface. Contain exceptions where they cannot cross. Use thin wrappers to restore native resource conventions.

Give each state a shape that holds only that state. A result is its value or its failure, never both. Store the states in a `std::variant` whose alternatives each carry their own state's data, so a contradictory state cannot be represented. Build the record only through named factories, one per state, and read it only through named queries, which may use `std::holds_alternative` and `std::get_if`; `std::visit` with a lambda stays out (§11). A flag beside the value it qualifies, a status beside a payload, and fields that belong to an inactive phase can disagree; do not declare them. Whatever the shape cannot exclude goes to the state machine's one fallback.

A switch over a closed set still has a default, and a state machine still says what an unexpected input does in each state. Send those cases, including ones believed impossible, to one defined fallback: a named failure, a reset, or an input ignored for a stated reason. Do not add checks along the way to compensate. A switch that names every enumerator, leaves from each case, and reaches its fallback directly after the switch also qualifies; omitting `default` there keeps the compiler's warning for a new enumerator.

Take a foreign entry point's type from its declaration, through `decltype` of the declared function or a static assertion against it; never retype it by hand. A call through a mismatched function type is undefined behaviour even where it happens to work.

**3. Types and values**

Name fields so callers need not memorize positions. Group related data in records. Use enums for alternatives and named factories when several fields must agree. Prefer a shape that cannot hold contradictory values to a check that rejects them (state shapes: §2). Initialize independent options directly. Initialize each variable explicitly.

Distinguish types whose confusion causes bugs: byte offsets and character positions, identities and indices, samples and seconds. Specify conversion rules, including rounding and encoding. Wrap scalars when the distinction matters.

An identity is never reused. A counter that issues identities checks for exhaustion before it issues one: at its maximum it refuses with a named failure, never wraps. A counter read from storage is input: check it against the identities read beside it and against the maximum before adopting it.

State array shape, active length, axes, and layout. Distinguish capacity from live length and maximum bounds from active regions. Fixed dimensions represent real bounds. Choose index types that support the required arithmetic; account for negative differences and signed/unsigned conversions.

Choose numerical precision once. Use doubles throughout a computation specified in doubles. Keep storage reusable without adding precision, layout, execution, and allocator parameters to every algorithm.

Spell the type. Use descriptive shorthand. Keep types visible at their declarations; put generic machinery behind named interfaces.

**4. Objects and authority**

Let parents own children and reverse links observe without extending lifetime. A detached object needs a surviving owner or independent lifetime. Cross authority boundaries through named operations or capabilities that specify read, modify, retain, and release rights.

Define the fallback when authority disappears. Reject stale identities, stop revoked callbacks, honor cancellation boundaries, or use an independently owned snapshot. A fallback cannot grant access to a dead owner.

An object holds related state, invariants, and authorized operations. Compose independent capabilities. Use inheritance for a behavioral relationship; define ownership separately.

Construct valid state and establish ownership. Keep plain configuration records as data. Name substantial setup, external interaction, and recoverable acquisition when callers need control over them. Distinguish invocation, connection, ownership, and disposal.

**5. Callbacks and execution**

Name asynchronous work, completion, cancellation, and shutdown. Assign mutable state to an execution context. Share immutable plans; keep mutable workspaces separate unless synchronized. Mark independent parallel work and specify reduction order when results depend on it.

Specify event order, reentrancy, and revocation. Define connection and disconnection during emission. A snapshot can preserve registration order, skip handlers disconnected before their turn, and defer new handlers. Keep the active target alive if it can dispose of itself during invocation.

Hold a lock only around the state it guards. Under a lock, call nothing whose locking you do not know: collect the work and do it after releasing. When holding the lock is the point (a revoked callback must not still be running), call only a target documented to signal without taking your locks or calling back, and state the lock order.

Adopt configuration at a defined processing boundary. Rebuild dependent state when inputs change. Synchronize cross-thread publication; copying fields at cycle start is not synchronization.

Give callbacks named targets and explicit context. Binding specifies invocation; subscription controls connection; ownership controls lifetime. Keep these responsibilities visible.

Use a non-owning delegate when a function and context pointer suffice. Binding need not allocate. Use an owning callable when state must be retained. Account separately for event storage, snapshots, and retained state.

**6. Procedures and data**

Identify inputs, outputs, and permitted mutation. Use `in`, `out`, and `in_out` where available; otherwise use types, names, and a short contract. Mark read-only inputs.

A procedure whose output grows with its input (a transformed buffer, an encoded image, a serialized document) writes into storage its caller supplies and states the size it needs, so the caller sees and can reuse the allocation. Return a fresh value when it is small and bounded, or say why the caller need not control it. State allocation, retained arguments, and permitted overlap so callers can compose operations safely.

In-place operations may use scratch storage. Preserve any input still needed before overwriting it. Require a separate destination where the operation depends on that separation.

Name intermediate forms: windowed samples, spectrum, threshold, mask, reconstructed output. Show the stages between them. Retain repetition that exposes distinct stages; extract repeated work with the same contract.

**7. Admitted abstractions**

Use abstractions that name useful operations and reduce what callers must remember. Keep their mechanisms inspectable.

Use explicit alternatives, borrowed spans, typed identities, and automatic resource owners. Localize virtual dispatch, reflection, and type erasure to boundaries requiring runtime variation. Avoid universal callable frameworks, private standard libraries, allocator hierarchies, and speculative portability layers.

Use standard minimum, maximum, clamp, copy, search, accumulation, inner product, and visitation when they express the work. Name predicates. Write a loop when traversal details matter. An inner product needs no private vocabulary of arithmetic wrappers.

Use templates and compile-time execution to implement declared choices. Name generic operations and their requirements. Require explicit participation in lifecycle and registration protocols; a coincidentally named member must not select ownership or construction behavior.

Choose sorting by count, order, duplicates, stability, move cost, comparator cost, and scratch space. Keep standard sort where it fits; specialize justified cases. Keep house search to named lower-bound, upper-bound, and exact-membership operations over explicit intervals. Use priority queues for required priority ordering and updates.

Precompute stable tables and reusable plans when saved work justifies storage. State normalization and tolerances. Keep quantities, stages, boundaries, and results recognizable from the mathematics.

**8. Repeated kernels**

_In order of priority (owner, 2026-10-31). Kernels are judged by how little they decide and check per element; the bookkeeping the other sections ask for belongs at the kernel's boundary, not in its interior._

No decisions inside the loop. A loop body that decides per element is not finished. Before any other optimization, find the formulation in which the decision does not arise: change what the loop iterates over, split the domain into regions of uniform treatment, precompute the case structure, regroup the work, or change the representation. Evaluating both alternatives and selecting is a fallback that pays for work the right formulation never does, not the answer.

Break the local paradigm, then restore exactness. The fast form need not resemble the reference algorithm or its paper. Derive it, then put back the local corrections it needs (edges, tails, exceptional values, the residual a reformulation leaves) outside the interior, and prove the result against the reference. Keep the reference and the comparison permanently; they are what make a radical reformulation safe.

Expect several waves. A kernel improves by successive reformulations, each measured on the same workload against the same reference. Keep the negative results.

Bookkeeping lives outside the loop. Establish every fact the interior relies on before it starts: shape, extent, capacity, alignment, overlap, and value ranges. The interior then needs no guards. A guard that cannot be moved out marks an unfinished formulation, not a check to preserve. Correctness is carried by the reference comparison, not by per-element defence. `const` does not guarantee non-aliasing; restricted-pointer declarations state an established no-overlap fact and apply wherever it is true.

Then the mechanics. Lay data out in lane groups, with the contiguous dimension innermost. Choose mode, format, direction, and implementation before the loop, one named kernel per distinct operation. Keep allocation, growth, logging, locking, callbacks, and error construction out of the interior. Read fixed parameters once and accumulate locally. State the numerical freedoms taken (reassociation, partial accumulators, fused operations, reciprocal substitution, NaN and signed-zero behaviour) and hold them to the reference's error bound; never blanket fast-math.

**9. Statements and expressions**

Write in execution order. Use assignments, conditionals, counted loops, early exits, and named calls. Show traversal when indices, order, boundaries, or writes explain the algorithm.

Use operators with familiar meanings: complex arithmetic, indexing, value comparisons. Do not hide scheduling, ownership changes, or an allocating pipeline behind punctuation. Whitespace does not change an operator's meaning. Make meaningful conversions explicit.

Keep each expression to one recognizable operation. Keep formulas intact; name useful intermediate quantities. Avoid tiny arithmetic wrappers and expressions that combine allocation, conversion, decisions, and writes.

Break chains that hide intermediate types, evaluation, retained state, or allocation. Name the intermediate value or extract a procedure. State the procedure's effects; a name alone does not explain them.

Calculate, then return. Return a named value, literal status, or direct field or element reference. Keep side effects in separate statements so their order is visible.

Name executable behavior. No lambdas, implicit captures, anonymous callbacks, or comprehensions. Use named functions, methods, functors, or listeners. Give retained state named fields so its dependencies and lifetime can be inspected.

**10. Source layout and comments**

Explain non-obvious dimensions, padding, normalization, lifetime, overlap, disposal order, and boundaries. State why a surprising operation exists. Preserve useful mathematical correspondence and remove stale comments. Keep vocabulary steady through the procedure.

Give substantial types, storage owners, and state machines meaningful source boundaries. Keep related definitions together. Put data and helper contracts before their orchestration where practical. Make the main processing sequence easy to find.

Use descriptive boundary names and short conventional mathematical names locally. Keep formatting consistent. Omit accessors, wrappers, and indirection that add no invariant or useful operation.

**11. C++ spelling**

Mark read-only inputs `const`. Use explicit constructors and conversions where implicit conversion changes meaning; reject unintended narrowing. Delete copying for unique owners and define moved-from state. Mark consequential results `[[nodiscard]]`. Use `noexcept` only when dependencies cannot throw. Use `static_assert` for compile-time requirements.

Fix the language mode and permitted subset.

| Form | Rule |
|---|---|
| Types | Spell local, iterator, parameter, and return types. Use descriptive aliases. No `auto`, `decltype(auto)`, or trailing returns. |
| Member access | Use dot. For pointers, write `(*pointer).member`. |
| Anonymous execution | No lambdas, including captureless predicates and trampolines. Name functions, functors, visitors, and listeners. |
| Other exclusions | No structured bindings, coroutines, or ranges/views pipelines in the default subset. |
| Type machinery | Keep `decltype(expression)` in named private traits, outside ordinary declarations and public signatures. |
| Compile-time work | Admit templates, `constexpr`, `consteval`, `if constexpr`, packs, and folds. Require explicit protocol opt-in; member detection must not choose architecture. |
| States and borrows | Admit `std::span`, `std::optional`, and `std::variant`. State lifetime and alternative semantics. Keep a result's or state's `std::variant` behind named factories and named queries (§2); read it with `std::holds_alternative` and `std::get_if`, and name any visitor. |
| Ownership | Admit `std::unique_ptr`. Use `std::shared_ptr` and `std::weak_ptr` for shared lifetime and observation. |
| Callables | Admit `std::function` for owning callables. Name targets; account for copies, allocation, and retained state. |
| Type erasure | No general `std::any` architecture. Confine inert-metadata compatibility fields to their existing contract. |
| Collections | Use `std::vector` for growth, standard strings for text, fixed arrays for real bounds, and heap arrays for explicit buffer ownership. |
| Comparisons | Write required comparisons. No defaulted comparisons or defaulted spaceship. Preserve meaningful value semantics; omit unused operator families. |
| Initialization | Use designated initializers for independent options with valid defaults; named factories for semantic result states. |
| Library operations | Admit `std::complex<double>`, ordinary algorithms, `accumulate`, `inner_product`, `has_single_bit`, string tests, membership tests, and named-predicate erasure. Representation casts require valid representation rules. |
| Exceptions and RTTI | Use them at failure, adapter, and dispatch boundaries that need them. |

A stricter generated-code profile uses named listeners, typed identities, deterministic initialization and connection order, immutable fixed tables, and bounded views. It excludes `std::vector` and capture-backed callables; use a named owner for required runtime-sized storage. Keep these restrictions scoped to that profile.

**Addendum: simple habits at storage boundaries**

- Before growing or replacing storage, finish reading any input that might refer to it. A read-only pointer or span can still point into the object being changed; copy the needed input first when overlap is allowed.
- Treat an open stream and a closed or failed stream as different states. Check the live handle before I/O, and change byte counts only for bytes actually transferred.
- Find the extent of the work, the input's size or the visible area, and check it against a limit before allocating for it. Check the whole size calculation, including padding and conversion to a smaller type; an unrepresentable size is a failure, not an empty result.
- If a call can return owned storage, adopt it before checking its success flag; failure can still leave storage to release.
- Try the awkward boundary case for each such operation: input from its own storage, a closed stream, a huge extent with a tiny visible clip, or a size near the integer limit. A check that reports a failure must also make its caller or build step fail, even when output is copied to a log.
