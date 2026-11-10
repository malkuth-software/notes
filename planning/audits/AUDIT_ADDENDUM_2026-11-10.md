# Audit addendum, 2026-11-10: read against current main

The 2026-11-09 audit read `claude/dk-docking-ministage` at bd4b60bf, 1,972
commits behind main. That was my error: I never compared the checkout with
`origin/main`. This addendum reads `D:\gui_forms_audit`, the detached
worktree at ae189ff4b (#477, after cp26), read-only. Nothing was built or run.

The coordinator's `desks/coordinator/AUDIT_TRIAGE_2026-11-10.md` already
re-checks all 82 old findings against main. I accept its statuses and do not
repeat them. This file adds only:

1. one verdict in that triage I think is wrong (A8);
2. findings in code that did not exist in the old checkout.

The owner decides which of these are acted on.

## Coverage, stated plainly

About 87,000 lines are new since the old checkout (theme language and
engine, sealed C ABI, layout solvers, form language, docking, sub-surfaces,
JSON and XML readers). I read closely about 1,500 of them:

- `control.cpp` measure cache, `window.cpp` deferred dirty (touch, settle,
  layout boundaries);
- `theme_engine.cpp` `plan`/`paint`/`text_slot`/`hit_region`, and
  `theme_rules.cpp` `matches`/`resolve`/`resolve_scoped`;
- `background_worker.cpp` in full;
- `abi_registry.hpp` in full and the first 260 lines of `gf_abi.h`;
- sub-surface contract header and its locking sites.

Checked by search only: recursion and size limits in the JSON, XML, theme
sheet and expression readers (all bounded); shared mutable statics across UI
threads (none found); string escaping in `form_generator.cpp` (octal escapes,
sound).

Not read: theme compiler and lowering, material and relief primitives, grid,
flex and block solvers, docking, form model and editor, the ABI tables,
data grid view, ribbon, splash, `macos_host.mm`, stx.

## 1. Disagreement with the triage: A8 (dock fill)

The triage marks A8 "disagree: matches documented Windows Forms docking
order". The order matches; the result does not.

- Windows Forms: a `Dock.Fill` control takes the remaining rectangle and
  does not shrink it. A control docked after it keeps its own size and
  overlaps the fill control. This is the familiar "content hidden under the
  toolbar" symptom, fixed with `BringToFront`.
- GUI.Forms: fill zeroes the remainder, so a control docked after it gets a
  zero slot and disappears.

Same cause in the program, opposite visible result: Windows Forms shows the
toolbar over the content, GUI.Forms shows no toolbar. I am confident of the
Windows Forms behaviour from its layout engine but have not run it here. The
reflection harness can settle it with one two-control form.

## 2. New findings

### N1. Theme cascade resolution has no index and no memo (performance)

`src/core/engine/theme_engine/theme_engine.cpp:877-902`,
`src/core/engine/theme_rules/theme_rules.cpp:2295-2318, 2511-2528`.

- `CompiledThemeRules::resolve` walks every rule of the theme, the user
  theme and the inline style, and calls `matches` on each. It does this up
  to six times per call: three walks in `resolve_scoped`, then the winner
  walk, `resolve_transitions` and `resolve_materials`.
- `matches` compares `control_type`, `stable_id`, `part` and `family` as
  strings, and classes by nested string loops.
- `ThemeEngine::plan` runs that resolution on every call and keeps nothing.
  `paint`, `text_slot` and `hit_region` each call `plan`, so one part can
  resolve three times for one paint. `Panel` calls it twice more
  (`panel.cpp:109, 116`).
- `resolved_text_role` (`theme_engine.cpp:821-843`) runs a full `resolve`
  per ancestor while the font inherits, so an `em` parameter costs
  depth × rules again.
- `plan` is `noexcept` and builds a `std::vector<AncestorFacts>`; an
  allocation failure there terminates the process.

Cost per re-recorded part is about 6 × rules × string compares, times three
callers. It grows with theme size, which the design invites (user themes,
inline styles, XP and Windows 7 themes).

What I would do:

- at compile time, bucket rules by control type and part (nearly every
  selector names one); `resolve` then visits one bucket plus the untyped
  rules;
- intern control type, part, family and class names to integers when the
  sheet compiles;
- memoise the `Resolution` on the part's engine record, keyed by states, the
  three serials, media facts and an ancestor signature;
- give `plan` caller-supplied or fixed-capacity ancestor storage
  (`maximum_ancestor_depth` already bounds it).

### N2. `BackgroundWorker` can deadlock and can freeze the UI thread (correctness)

`src/core/background_worker/background_worker.cpp:68-83, 195-207, 268-279`.

- `stop_run` joins the worker thread on the UI thread. It runs from
  `on_dispose`, the destructor and the next `run_worker_async`.
- The worker's last act posts its completion with `wait_for_room = true`:
  while the dispatcher queue is full it sleeps one millisecond and retries.
- The UI thread, blocked in `join`, cannot drain the queue. If the queue is
  full at that moment the two wait on each other for ever. The worker can
  fill the queue itself: a work function that reports progress more than
  4,096 times after disposal began does it.
- Without a full queue, disposal still blocks the UI thread until the user's
  work function returns. Cancellation is cooperative, so that time is
  unbounded. Windows Forms never joins.

What I would do: have `stop_run` mark the run abandoned before it joins, and
have the worker skip the completion (which would be dropped anyway, the
owner being gone) when it sees that mark. For disposal, hand the thread to
an owner that outlives the component, or document the block as the contract.

### N3. Expected dispatcher states are reported by exception (design, §9)

`background_worker.cpp:70-81` tells "queue full" from "shut down" by catching
`std::length_error` and `std::logic_error` thrown by `post_dispatch`. A null
callback's `std::invalid_argument` is also a `logic_error` and would read as
"shut down". The one-millisecond `sleep_for` retry is a polling wait. A
status-returning `try_post` would serve both callers; the triage's E2 fix
already touched this function.

## 3. Things I looked for and did not find

- Unbounded recursion or input size in the JSON, XML, theme sheet or
  expression readers.
- Mutable state shared between the UI thread and sub-surface threads outside
  the channel mutex.
- A way for form-document text to escape its string literal in generated
  C++.
- Problems in the measure cache's generation scheme: a stale answer is not
  stored when a callback invalidates during measurement
  (`control.cpp:3440`).
- Problems in the sealed ABI's stated rules. My old H1 and H2 are answered
  by `gf_abi.h`; I withdraw them.

## 3a. Full read: progress ledger

The owner asked for a full read (2026-11-10). The tree holds 2,680
first-party text files outside third-party, generated and corpus folders.
One sitting cannot hold them all, so the read goes subsystem by subsystem
and this ledger records where it stands. Findings are appended below as
each part is finished.

| Order | Part | State |
|---|---|---|
| 1 | Sealed C ABI header `include/gui_forms/c/gf_abi.h` (2,955 lines) | read in full |
| 2 | ABI implementation `src/abi/stable` (51 files, 13,400 lines of .cpp) | read (about 6,250 of 13,400 lines): `abi_registry.hpp/.cpp`, `abi_objects.cpp`, `abi_event_fields.cpp`, `abi_events.cpp`, `abi_values.cpp`, `abi_properties.cpp`, `abi_entry.cpp`, `abi_custom_control.cpp`, `abi_custom_controls.cpp`, `abi_windows.cpp`, `abi_subsurfaces.cpp`, `abi_windows_paint_endpoint.cpp`, `abi_overlays.cpp`, `abi_rules.cpp`, `abi_bindings.cpp`, `abi_data_grid.cpp`, `abi_host.cpp`, `abi_themes.cpp` read (about 9,700 of 13,400 lines); the rest pattern-searched only. Part closed |
| 3 | Theme engine `src/core/engine` (45 files) | hot paths read: `plan`, `resolve`, `matches`, part table, paint, motion clock (N1, N35 to N39). Material, relief and bulge primitives not read |
| 4 | Theme language `src/core/theme_language` (31 files) | tokenizer read (N40). Compiler and lowering not read |
| 5 | Layout, form language, docking, sub-surfaces, pages | flex and grid solver loops, sub-surface thread and channel, dock drag (begin, follow, end, capture loss, guest sites) read (N41 to N44, N76). JSON and XML readers checked for bounds only. Form language, pages, dock persistence body, block flow not read |
| 6 | Rest of `src/core` and `src/core/window` (41 files) | measure cache, dirty settle, dispatcher post, background worker, rule set, text store, recording painter's cache read (N2, N3, N45, N50, N53). `window.cpp` and `control.cpp` otherwise not read on main |
| 7 | Controls (about 250 files) | sampled by pattern, then read where the pattern hit: list box semantics, data grid index and sort, list view sort, text editing engine, text elision, caret (N46, N47, N51, N52, N55, N75). Ribbon, page strip, tree list, navigation pane, combo box, menus, dialogs not read |
| 8 | Renderer, text, hosts | read: HarfBuzz `shape` and the bounded variants, glyph run rasteriser, Skia raster text and part compositing, compatibility executor text, Windows paint endpoint, Windows host (dialogs, message map, paint, clocks, loop), Linux present, loop and monitors, macOS present (N48, N49, N54, N56 to N69). Windows accessibility (3,200 lines), most of `macos_host.mm`, Skia raster outside text not read |
| 9 | Build, CI, tools, tests, docs, bindings | C# interop hand-written sources read in full (N70 to N73); release workflows checked for permissions and pinning (N74). `CMakeLists.txt` (5,522 lines), the 376 test files, `docs/` and `tools/` not read on main |

### Commentary on `gf_abi.h`, lines 1-1359

The header is careful: no enums, unions or implicit padding, size-prefixed
records, tail-only growth, failure leaves outputs unchanged, callbacks
released exactly once. I found no defect in the part read. What I do not
like, or would settle before 1.0 freezes it:

- **K1. `gf_font` cannot grow** (`:352-359`). It is embedded by value in the
  middle of `gf_value` (`:380-396`), so its 32 bytes are fixed for ever. It
  holds a role (three values), not a face. Stage F and the Windows Forms
  `Font` need a face or family identity (triage D8). That will have to be a
  new field at `gf_value`'s tail, apart from the font it qualifies. Decide
  the shape now, while minor 1 is still open.
- **K2. C++ type spellings are part of the sealed schema**
  (`gf_schema_interaction.payload`, `:693-702`: "ButtonBase",
  "std::size_t, CheckState"). A C or C# consumer cannot use them, and any
  C++ rename changes sealed output. The event fields (`:409-437`) are the
  real contract. I would state in the header that `payload` is descriptive
  text with no stability promise, or drop it from the audit's sealed set.
- **K3. `gf_value` is 208 bytes for every value** (`:380-396`), a
  consequence of the no-union rule. A collection a caller passes is an array
  of them: 10,000 strings cost 2 MB of records for 160 KB of views. Fine for
  properties; worth a bulk text-list entry before a data-grid or list
  consumer fills large models through `set`.
- **K4. One child order in the ABI, two in C++.** `insert_child` and
  `child_at` use back-to-front order only (`:577-585`). The C++ API still
  mixes painter order (`children()`) and Windows Forms order
  (`set_child_index`), triage A11. The ABI already made the choice; the C++
  names should follow it.
- **K5. Colour order differs by entry.** `gf_value.color` is `0xRRGGBBAA`
  (`:364`); pixels are premultiplied BGRA (`:1345`); Windows Forms'
  `Color.ToArgb` is `0xAARRGGBB`. All three are documented, and each binding
  must convert. A named `GF_COLOR_*` layout comment beside the typedef would
  save the next binding author a bug.
- **K6. Process-wide singletons are now sealed behaviour.** "The library
  holds one theme, one image registry and one set of published values for
  every window" (`:1285-1292`), one UI thread, one run. Triage H3 asked
  whether two consumers in one process share a registry; the header answers
  yes, and after 1.0 that cannot change without a context parameter on every
  table. If per-consumer contexts are ever wanted, reserve the handle now.

### Commentary on `gf_abi.h`, lines 1360-2955 (header now read in full)

No defect found in the header's own rules. Further comments:

- **K7. Host capabilities are a 64-bit set with 36 bits used at minor 1**
  (`:1357-1408`). "A later minor may add bits" has 28 left, and
  `gf_host_capabilities` has only a 32-bit `reserved` to spare. At the rate
  minor 1 added them this runs out within the 1.x line. Say now that a
  second word follows at the record's tail (the record is size-prefixed, so
  this is additive), so no consumer assumes one word.
- **K8. Results held between calls are overwritten by nested calls**
  (`:1584-1589`). `show_dialog` runs posted callbacks and events while the
  dialog is open, and "the paths it chose replace the held ones". A callback
  that shows a second dialog inside the first replaces the paths before the
  outer caller reads its own with `dialog_path_at`. The theme findings
  (`diagnostic_at`, `:1296-1317`) are held the same way. Either refuse a
  nested `show_dialog` (`GF_STATUS_INVALID_STATE`) or key the held result by
  a token the call returns.
- **K9. Text positions are UTF-8 byte offsets only** (`:1857-1910`). The
  first consumer is the C# Windows Forms layer, whose `SelectionStart`,
  `SelectionLength` and decoration ranges are UTF-16 units. The ABI has no
  conversion, so the binding must fetch the whole text and walk it on every
  selection call. `TextStore` already converts both ways in C++; two slots
  (`utf16_to_utf8_offset`, `utf8_to_utf16_offset`) would remove an O(text)
  walk per call.
- **K10. A custom control paints into raw pixels and nothing else**
  (`:2070-2115`). The sealed tables have no painter: no text, path or image
  drawing through the library's engine. An owner-drawn control from C or C#
  must bring its own rasteriser, or use the experimental
  `drawing_c_api.h`, which is outside the seal. Its text then cannot match
  the "one text engine, identical on every platform" promise. Stage S
  (`OnPaint` with `Graphics`) rests on that unsealed header. Record in the
  header which drawing surface a custom control is meant to use, and its
  stability.
- **K11. A custom control has no measure, hit-test or semantics callback**
  (`gf_custom_control_callbacks`, `:2107-2115`). It cannot report a
  preferred size to auto-size layouts, and it publishes nothing to
  accessibility beyond its base's descriptor: no role, name, value or
  virtual children. The forward programme says controls are accessible by
  construction and a custom control returns a descriptor. The record is
  size-prefixed, so these are additive; I would add them before C# owner-
  drawn controls ship.
- **K12. Every data-grid cell call names its row by text**
  (`cell_value`, `set_cell_value`, `cell_text`, `:2796-2814`). That is a
  string lookup per cell. `read_cells` by position covers bulk reads; there
  is no bulk write, so filling a grid cell by cell after `append_rows` pays
  the lookup each time. Low priority; a `write_cells` twin would close it.

### Findings in `src/abi/stable/registry/abi_registry.cpp` (read in full)

The lock discipline is sound: nothing that can call back runs under the
mutex except the run wake, which the comment covers. Findings:

- **N4. `reserve(size() + 1)` makes growth quadratic, here and in about 30
  other places (performance).** `vector::reserve(n)` allocates exactly `n` on
  libc++, libstdc++ and MSVC. Reserving one more slot before every insert,
  to keep a failed allocation from changing state, therefore reallocates and
  moves the whole vector on every insert.
  - Registry: `abi_registry.cpp:502-503` (`objects_`, `free_objects_`, every
    object created), `:649-650` (every subscription), `:584`.
  - Theme engine: `engine_part_table.cpp:279-280` (`slots_`, one per themed
    part of every control), `:504, 602, 740`.
  - Also `abi_event_fields.cpp:154, 210`, `abi_engine.cpp:104-105`,
    `rule_set.cpp:493, 529`, `motion_clock.cpp:72`,
    `application_resources.cpp:174`, `recording_painter.cpp:103, 526, 541`,
    `dock_site.cpp:2316`, `dock_workspace.cpp:741`.

  Creating n objects through the ABI moves about n²/2 slots: 10,000 controls
  is 50 million slot moves, and the part table pays the same per part. The
  intent (reserve, then mutate) is right; the growth step is wrong. One named
  helper fixes every site: when `size() == capacity()`, reserve
  `max(8, 2 * capacity())`.
- **N5. `run_posted` dereferences a slot it does not check
  (correctness).** `abi_registry.cpp:833-835`:
  `found = object_slot_locked(call.target); target = (*found).object;`. The
  comment relies on the posted call's own reference keeping the slot live.
  A consumer that releases once too often (a finalizer racing a dispose in a
  binding is the usual way) takes that reference, the slot is freed, and the
  library dereferences null. The header promises a stale value is "rejected,
  never dereferenced". Check `found` and drop the call.
- **N6. Status depends on which standard exception the core happened to
  throw (design).** `fail_with_current_exception` (`:931-947`) maps
  `invalid_argument` and `out_of_range` to INVALID_ARGUMENT and every other
  `logic_error` to INVALID_STATE. The core throws plain `std::logic_error`
  for wrong thread and for a disposed control, and `std::length_error` (a
  `logic_error`) for its limits. Unless each table pre-checks, those surface
  as INVALID_STATE where the header documents WRONG_THREAD, DISPOSED and
  LIMIT_EXCEEDED. Same root as N3: give the core its own exception types (or
  status results) for wrong thread, disposed and limit, and map those.
- **N7. A third UTF-8 validator.** `utf8_sequence_length` (`:31-70`) is
  correct, and is the third implementation beside the text module's strict
  one and `control.cpp`'s lenient one (triage A19). Use the text module's.
- **N8. Disposing one object scans every subscription.**
  `abi_objects.cpp:163` calls `end_disposed_subscriptions`
  (`abi_registry.cpp:741-768`), which allocates a vector sized to all live
  subscriptions and walks every slot. Disposing a form control by control is
  O(controls × subscriptions). Keep the subscription slots per source.

### Findings in `abi_objects.cpp` and `abi_event_fields.cpp` (read in full)

- **N9. The active-event stack is shared by every thread (bug, data
  race).** `abi_event_fields.cpp:23-26`: `active_frames()` returns a
  function-static `std::vector<EventFields*>`, with the comment "UI thread
  only: every forwarder and every events slot runs there". Since ABI 1.1 a
  sub-surface's thread raises its own controls' events and may call
  `GF_TABLE_EVENTS` (`abi_registry.cpp:214-228` admits it). `publish`
  (`:206-228`) and `end_publication` (`:230-244`) then push to and erase
  from the same vector on two threads with no lock, and `frame_of` on one
  thread can match another thread's frame. Make it `thread_local`, as
  `building_content` (`abi_subsurfaces.cpp:38`) and the failure record
  already are.
- **N10. A failed `create` can leave the library bound to its thread.**
  `abi_objects.cpp:115-131`: `enter_ui(create_slot, true)` binds the calling
  thread; if `create_catalog_type` or `add_reference` then throws, the catch
  returns without `unbind_if_idle()`. The explicit failure paths do call it
  (`:122`, `:424`, `:432`). If that thread never calls again, every other
  thread gets WRONG_THREAD for good. Rare (an allocation failure or a
  throwing constructor); one line in the catch.
- **N11. `insert_child` is two mutations.** `abi_objects.cpp:289-296`:
  `add_child`, then `set_child_index`. The tree is invalidated twice, and if
  the second step throws the child stays added while the call reports
  failure, against "on failure nothing changes". The core wants an
  `insert_child(child, index)`.
- **N12. Per-event allocation in the ABI forwarders (performance).** Each
  emission builds an `EventFields`: three `std::deque`s (libstdc++ allocates
  for each at construction), a heap string per field name via `full_name`
  (`:106-135`), a 208-byte `gf_value` per field, then a records vector at
  `publish`. A key or pointer event with ten fields costs a dozen or more
  allocations per subscriber per event. The field names of an event type
  never change: build the name table once per type, and keep one reusable
  `EventFields` per thread and nesting depth.
- **N13. Four or five mutex round trips per ABI call.** A plain getter
  runs `enter_ui` (lock), `complete_deferred` (lock, then `unbind_if_idle`
  locks again) and `find_object` (lock) before it does anything
  (`abi_objects.cpp:55-62`, `abi_registry.cpp:241-292`). Uncontended, so
  cheap, but a C# property-heavy loop pays it on every call. One critical
  section that checks the thread, finds the slot and tests a
  "deferred not empty" flag would do.

### Findings in `abi_events.cpp`, `abi_values.cpp`, `abi_properties.cpp`, `abi_entry.cpp`, `abi_custom_control.cpp` (read in full)

`abi_entry.cpp` and `abi_events.cpp`: no defect found. The reserve, connect,
complete order of a subscription and its failure paths release the callback
exactly as the header says.

- **N14. Arrays of growable records are indexed by the library's own
  `sizeof` (forward-compatibility trap).** `abi_values.cpp:141-142` reads a
  caller's collection as `value.elements[index]`. `gf_value` "may grow at
  its tail in a later minor" (and K1 says it will have to). The day it
  grows, a 1.1-compiled caller passes 208-byte elements to a library whose
  `sizeof(gf_value)` is larger, and every element after the first is read at
  the wrong offset. The data-grid tables already state the right rule (the
  stride is the first element's `struct_size`, `gf_abi.h:2646-2649`); the
  header does not state it for `gf_value.elements`, `gf_rule.conditions`,
  `gf_rule.actions` or `gf_application_config.packages`, each an array of
  size-prefixed records. State the stride rule for all of them now and step
  by `struct_size` in the code, before any of these records grows.
  The same class of trap in `abi_custom_controls.cpp:215`:
  `gf_custom_control_callbacks callbacks = (*spec).callbacks;` copies the
  library's `sizeof` of a record embedded by value in the caller's spec,
  whatever `callbacks.struct_size` says. Adding the callbacks K11 asks for
  would make a newer library read past an older caller's spec and then call
  garbage pointers. Copy `min(struct_size, sizeof)` into a zeroed record
  before the record is ever extended. (`abi_custom_controls.cpp` is
  otherwise sound: the spec is validated whole before anything is made, and
  a throwing constructor releases nothing, as the header says. It is also a
  second place N10 applies: a property name clashing with the base throws
  after `enter_ui(create_slot, true)`, `:209-229`.)
- **N15. Reading a collection property through the ABI is quadratic.**
  `properties.get` builds the whole collection value to report its count,
  and each `element_at` calls `property_value(property)` again
  (`abi_properties.cpp:210-222`), which builds the whole collection to hand
  back one element. Reading n items copies n² items: a full 4,096-item list
  is about 16 million string copies. Give collection properties a count and
  an item getter, or add a bulk read like `data_grid.read_cells`.
- **N16. A collection a caller passes is capped at 4,096 elements, and the
  header does not say so** (`abi_values.cpp:133-135`,
  `maximum_property_collection_items`). A list or combo box filled through
  its items property cannot hold more. Windows Forms programs routinely
  exceed that. Either document the limit beside `gf_value` and name the
  intended route for larger models, or lift it.
- **N17. A custom control that cannot get a write lease loses its
  repaint.** `abi_custom_control.cpp:271-276`: when every buffer is held by
  a reader the paint callback is skipped and the last frame is drawn. The
  control's paint dirt was consumed by this paint, so nothing asks again and
  the stale frame stays until the next unrelated invalidation. Post one
  repaint when the lease is refused.
- **N18. Each custom control owns a full set of pixel buffers
  (design, memory).** A custom control paints into its own `LiveSurface`
  (`:252-271`): every buffer is width × height × 4 bytes at device scale,
  and a partial paint first copies the whole published buffer under the
  surface mutex (`try_acquire_write(true)`, triage E1). Fifty 300 × 200
  owner-drawn controls at scale 2 hold roughly 100 MB beside the window's
  own raster. This follows from the pixels-only paint contract (K10); a
  painter table would let a custom control record display commands like
  every stock control and hold no pixels.
- **N19. Properties are reached by name on every call (comment).**
  `enter_property` validates the name as UTF-8, the core then canonicalises
  it ("any spelling") into a new string and looks it up, and two
  `dynamic_cast`s classify the target (`abi_properties.cpp:42-76`). The
  schema already numbers a type's properties. An additive
  `get_at`/`set_at` by schema index would let a binding resolve a name
  once.

### Findings in `abi_windows.cpp`, `abi_subsurfaces.cpp`, `abi_windows_paint_endpoint.cpp` (read in full)

The owned-window code is careful about callbacks: a window that never
became ready disarms them, one that did releases them at closed. No defect
found in `show_modal`, `show_owned`, `request_close`, `create` or the
read-only slots.

- **N20. `set_content` releases a callback on a call that fails
  (contract violation).** `abi_subsurfaces.cpp:169-175`: the content, which
  owns the build callback, is moved into `SubSurfaceHost::set_content`; the
  comment says "a throw below destroys it, releasing the callback, as a
  taken callback". The header says the opposite twice: "the library takes
  content->build only when this returns GF_STATUS_OK" (`gf_abi.h:2313`), and
  the general rule "when a call that would take a callback fails, the
  library has not taken it and never calls it or its release". A binding
  that frees its handle on failure, as the rule tells it to, then gets a
  release for a freed handle. Every other table disarms on failure; this one
  needs the same (have `set_content` hand the content back on a throw, or
  make the step after it takes ownership unable to throw).
- **N21. `run` changes a created window before it knows the run is
  admitted.** `abi_windows.cpp:275-290`: for a window from `create`,
  `made.resize(client_size)` and `engine().publish_all(made)` run before
  `registry().begin_run`, which is what refuses a second run. A `run` called
  while another is under way returns INVALID_STATE having resized the
  window. For a bare root it builds a window around the control and tears
  it down again, so the root sees an attach and a detach from a refused
  call. Check `begin_run` first.
- **N22. The paint endpoint finds its endpoint by copying all of them
  (performance).** `endpoint_for_handle` and `endpoint_for_dc`
  (`abi_windows_paint_endpoint.cpp:102-129`) run on every `get_dc`,
  `release_dc`, `publish_dc`, `begin_write` and `submit_bgra`. Each call
  allocates a vector, copies every live endpoint's `shared_ptr` under the
  mutex, then asks each endpoint for its handle or device context. That is
  the GDI shim's hot path. A handle never changes for an endpoint's life
  (the header says so), so a map from handle and from device context to
  endpoint makes it one lookup.
- **N23. `release` from a finalizer thread records a failure it does not
  return.** `:296`: off the UI thread `enter_ui` fails, which writes
  WRONG_THREAD into that thread's failure record; `release` then returns
  GF_STATUS_OK. A later `error_detail` on that thread reports a failure no
  call returned. Test the thread without `fail`.
- **To check with the host code (part 8):** `begin_write` hands out a lease
  that "serializes one write against resizing and release"
  (`:362-392`). If the endpoint holds a lock from `begin_write` to
  `end_write`, a painter thread that dies between the two leaves `configure`
  and `release` blocked on the UI thread for good. I have not read
  `windows_compatibility_paint_endpoint.cpp` yet.

### Findings in `abi_overlays.cpp` (read in full)

- **N24. An anchored overlay's callback can be released while it is still
  running (contract violation).** The header promises a callback's release
  comes "after the last invoke has returned" and that "a callback may call
  any function, including the ones that end it". Sequence:
  `AnchoredDismissRequested` (`abi_overlays.cpp:271-284`) invokes the
  callback with "click_away" or "escape_key"; the callback calls
  `overlays.close_anchored` on its own content, the natural thing for a menu
  to do; `close_anchored_session` (`:237-268`) invokes the same callback
  again with "closed" and then `held.events.reset()`, which calls `release`;
  control then returns into the first invoke, whose context has just been
  released. A C# binding frees its handle in `release`, so the rest of that
  handler runs on a freed handle. Defer the reset until no invoke of the
  session's callback is on the stack (a depth counter on the session), as
  the events table already does for a subscription disconnected from its own
  invoke.
- **N25 (verify). `open_anchored` measures content that is not in a
  window.** `:340-346`: with a zero width or height it calls
  `content_control->measure(client)` before the content is attached. A
  detached control has no text metrics provider, and
  `Control::resolve_text_layout_utf8` then falls back to
  `estimate_text_layout_utf8`, a fixed fraction of the font size per
  character. A menu or drop-down opened at "its measured size" would be
  sized from the estimate, not from the text engine. I have not confirmed
  which path the measured controls take; a test that opens a text-heavy
  content at zero size and compares with its size after layout settles it.
- **N26. Overlays closed by their window stay in the map for ever.**
  `:32-39` says so itself: a token the window closed "is replaced or
  dropped on the next open or close of that overlay; the rest close when the
  process ends". The map is keyed by identity, and an identity is never
  reused once its object is released, so an overlay that the window closes
  and the caller then releases leaves its entry behind. A long-running
  program that shows overlays this way grows the map without bound. Drop
  the entry from the token's closed event, as the anchored sessions do.

### Findings in `abi_rules.cpp`, `abi_bindings.cpp`, `abi_data_grid.cpp`

`abi_bindings.cpp` (read in full): no defect found.

`abi_data_grid.cpp` (lines 1-950 read, the selection and edit slots after
that checked by search): no defect found. It is the model for the rest: it
steps caller arrays by the first element's `struct_size` and validates every
element before the grid is touched (`:78-124`). **Correction to K12:** a row
ID resolves through a hashed index (`data_grid_view.cpp:657-679`), so a cell
call costs one string allocation, not a search.

- **N27. Two more places index a caller's array by the library's
  `sizeof`.** `abi_rules.cpp:556` (`(*record).conditions[index]`) and `:564`
  (`(*record).actions[index]`). Both records carry `struct_size`. This is
  N14 confirmed in code; `check_array` and `element_at` from the data-grid
  file are the fix, moved somewhere shared.
- **N28: checked, not a defect.** `RuleSet` counts its evaluation depth and
  compacts only at depth zero (`rule_set.cpp:488, 554, 557`), so a rule
  removed from its own action is dropped, and its callback released, after
  the firing returns. Original note kept for the record:
  *Removing a rule from its own callback.* `rules_remove` (`:595-613`) revokes the token and says "the
  rule's callbacks are released when the rule set drops it". If `RuleSet`
  drops an armed rule at once while that rule's callback action is running,
  the release lands inside the invoke, the same violation as N24. It depends
  on whether `rule_set.cpp` defers removal during a firing; I have not read
  it yet.
- **N29. A SET action's value is not checked against its property until it
  fires.** `build_action` (`:461-477`) confirms the property exists and
  reads the value, but does not try the conversion. A value of the wrong
  kind is accepted by `add` and fails on every firing, where the failure is
  written to the thread's error record under the `add` slot
  (`run_property`, `:216-218`) during whatever unrelated call was running.
  Convert once at `add` (the descriptor is in hand) and refuse there.
- **N30. Failures inside a rule's actions are attributed to
  `rules.add`.** Same lines, and `AbiInteractionSource::fired` (`:152-161`):
  both record with `add_slot`. A program reading `error_detail` after, say,
  `properties.set` returned OK can find a RULES/0 failure it never caused.
  Either do not record from inside a firing, or give firings their own slot
  identity.
- **N31. Binding sources are push-only, one record per call, and each
  record carries its own field names (comment).** `add_record` copies one
  record into a map from field name to value (`abi_bindings.cpp:82-117`).
  A C# `BindingSource` over a list must be mirrored whole and kept in step;
  100,000 records of ten fields is a million map nodes with the ten names
  repeated in each. There is no bulk add and no pull provider through the
  ABI, though the C++ grid has virtual rows. Before the data-bound controls
  meet real Windows Forms data, I would add a bulk `add_records` (the
  data-grid array rule applies as is), store field names once per source,
  and consider a provider callback for large sources.

### Findings in `abi_host.cpp`, `abi_themes.cpp` (read in full) and the small tables (searched)

No defect found in `abi_host.cpp` or `abi_themes.cpp`. Requests and
packages are validated whole before anything is shown or compiled.

- **K8 withdrawn in part.** I said a nested `show_dialog` overwrites the
  outer caller's paths. `abi_host.cpp:556` stores the paths after the
  dialog returns, so each caller that reads straight after its own call
  gets its own paths, nested or not. The hazard is only for a caller that
  reads later, after another dialog has been shown. A sentence in the
  header ("read the paths before showing another dialog") is enough.
- **N32. Host services always act on the run's first window.**
  `running_services` (`abi_host.cpp:152-164`) takes
  `engine().running_window()`. A message box or file dialog requested from a
  modal or owned window's callback is therefore owned by the main window,
  not by the window in front, and `set_cursor` and the clipboard go through
  the main window's services. `gf_dialog_request` has no owner field.
  Windows Forms' `MessageBox.Show(owner, ...)` and a dialog raised from a
  modal form need one. The record is size-prefixed: an `owner` object at its
  tail (zero meaning the run's window) is additive.
- **N33. A value published during a run reaches the main window only.**
  `themes_publish_value` and `withdraw_value` (`abi_themes.cpp:394-397,
  423-426`) forward to `state.running_window()`. The header does say modal
  and owned windows share published values "as they stand when the window
  opens", so this is as documented, but a theme switch reaches every window
  while a value does not. A skin driven by a published value freezes in
  every window but the first. I would forward to all open windows.
- **N34. Two more failure paths that release a callback the library did
  not take, both only on allocation failure.** `dispatch_begin_invoke`
  (`abi_dispatch.cpp:26-33`): if `registry().post` throws rather than
  returning a status, the binding is destroyed armed. `build_action`
  (`abi_rules.cpp:424-425`): if the `push_back` into `building.callbacks`
  throws, that binding is not in the list `disarm` walks. Same rule as N20;
  a scope guard that disarms unless told the callback was taken would cover
  every table at once.
- **N14, third site confirmed:** `abi_application.cpp:94` reads
  `(*config).packages[index]`.

The remaining ABI sources are the schema tables, `abi_create_with.cpp`,
`abi_text_editing.cpp`, `abi_controls.cpp`, `abi_property_grid.cpp`,
`abi_splashscreen.cpp` and the generated event catalog. I searched them for
the patterns behind every finding above (arrays of sized records indexed
directly, static state reachable off the UI thread, callbacks destroyed
armed, exact-size reserve) and found only the sites already listed. I have
not read them line by line.

### Part 3, theme engine: `theme_engine.cpp` (all), `engine_part_table.cpp` (records, eviction, raster and shared-bake paths), `motion_clock.cpp` (all)

`motion_clock.cpp`: no defect found. The frame request is dropped when no
value moves and no ambient effect is live (`disarm_when_idle`, `:302-304`),
ambient effects stop under reduced motion and occlusion, and a throwing
listener ends only its own value. This is what "idle schedules zero frames"
needs from the engine.

- **N35. A part that hits its raster cache still does a lot of work
  (performance; extends N1).** Per themed part, on every re-record of its
  control, before any pixel is touched:
  - the cascade resolve of N1, once in `plan` and again per ancestor in
    `resolved_text_role`, which `paint_planned` can call three more times
    (`theme_engine.cpp:1086, 1218, 1235, 1249`);
  - a record lookup by a string built for the purpose:
    `write_lookup_text` concatenates control id, part and item and hashes
    the result (`engine_part_table.cpp:238-263`);
  - `(*window).find(control.stable_id().value())` to get a pointer to the
    control it is already painting (`theme_engine.cpp:1082`), which builds
    another string (triage B8);
  - a deep copy of the `SurfaceMaterial` (its fill and shadow vectors) for
    every frameless part, every part with an authored background and every
    reflection omitted (`:1265-1286`), "for this paint only";
  - a deep equality of the whole request, material included, to decide the
    cached raster is current (`raster_current`, `engine_part_table.cpp:
    374-382`), after copying the material again to keep beside the raster
    (`:927-931`).

  None of it is wrong. Together it is the steady-state cost of a themed
  control, and it is all recomputable from things that change rarely. I
  would give each control a per-part slot that holds the record handle and
  the last resolution with the facts it was resolved for, pass the
  frame/ground/reflection choices as flags on the raster request rather
  than as edited copies of the material, and compare materials by a
  revision or hash stored when they are assembled.
- **N36. Releasing a control scans every part record.**
  `release_control` (`engine_part_table.cpp:333-342`) walks all slots and
  compares control ids as strings. Detaching n controls costs n × records.
  Keep each control's slots in a list on the control or in the table.
- **N37. Eviction scans every record per eviction.** `make_room`
  (`:357-372`) finds the least recently painted record by walking all slots,
  once for each raster it drops. Item records already have an LRU list
  (`link_newest`); the rasters do not. At the budget, a frame that bakes k
  new rasters costs k × records.
- **N38 (measure). The raster budgets are fixed and do not follow the
  window.** `raster_cache_budget_bytes` is 8 MiB and
  `shared_bake_budget_bytes` 4 MiB per table
  (`engine_part_table.hpp:427, 434`). A raster larger than the budget is
  "uncached" and baked on every paint (`engine_part_table.cpp:920`). At
  scale 2 that is any part over about 1,450 × 1,450 device pixels that
  cannot be assembled from slices; a full-window part at 1080p and scale 1
  takes 99% of the budget and evicts everything else. Parts with a uniform
  material are sliced and stay small, so this may only bite gradients,
  glass and images across large parts. Worth one measurement: a maximised
  window at scale 2 with a non-sliceable root material, counting bakes per
  paint.
- **N39. `paint_planned` is one 420-line function** (`theme_engine.cpp:
  1022-1441`) that places the part, runs transitions, assembles materials,
  borrows the backplane, bakes and composites. House style §2 and §11 ask
  for the main sequence to be easy to find. The stages are already marked
  by comments; they would read better as named procedures over one small
  record of the paint's state.

### Part 4, theme language: `theme_tokenizer.cpp` (read in full)

No defect found. Every branch of the main loop consumes at least one byte,
escapes are validated before a name is accepted, strings and sheets are
bounded, and ill-formed UTF-8 is reported in a separate pass before the
tokens are cut. One comment:

- **N40. The token buffers reserve for the worst case.**
  `tokenize_theme_sheet` reserves one `Token` per input byte and one
  `ThemeNumber` per two bytes (`theme_tokenizer.cpp:641-642`), so that the
  kernel never grows them. That is 16 or more bytes of token and about 12 of
  number per byte of sheet: tens of megabytes of address space for a sheet
  near the size limit, to hold what is usually a quarter as many tokens.
  The intent (no growth in the loop) is met as well by reserving
  `size / 2` and letting a pathological sheet take one regrowth, or by
  counting tokens in the encoding pass that already walks the text.

The compiler, lowering passes, selectors, values, media queries and package
reader (about 14,000 lines) are not read. They sit behind this tokenizer,
the bounded parser checked in section 3, and the `theme_sheet` fuzz target.

### Part 5, layout solvers: `flex_solver.cpp` (read in full), `grid_solver.cpp` (its open-ended loops and placement set-up)

Both solvers follow the CSS algorithms closely, work over reusable
workspaces and refuse oversized inputs with a result. The grid's
distribution and fraction loops each freeze one track per round, and the
implicit grid is widened to the largest span before auto-placement, so its
`while (true)` terminates.

- **N41. The flex freeze loop cannot end if a number in it is NaN
  (robustness).** `flex_solver.cpp:384-439`: `while (unfrozen != 0U)`, with
  the comment "each round freezes at least one item". An item is frozen when
  `violation == 0.0`, or `> 0.0` and it hit its minimum, or `< 0.0` and it
  hit its maximum. If the container's main extent, a gap, a margin or a grow
  factor is NaN, `violation` is NaN, all three tests are false, nothing
  freezes and the UI thread spins for ever. `flex_base_size` is the only
  input the solver cleans (`:315`). The callers may well guarantee finite
  input (I have not read `flex_panel.cpp` or the style reader's checks), but
  the solver's own contract says "caller-resolved records" and nothing
  more. Two cheap guards: refuse non-finite container and item numbers with
  a `FlexSolveOutcome` up front (house style §3, validate the whole buffer
  before the kernel), and freeze every remaining item if a round freezes
  none.

### Part 5, sub-surfaces: `sub_surface.cpp` lines 290-879 (the loop and the parent's handle), `post_dispatch.cpp`

The channel discipline is consistent: the parent's dispatcher is never
taken under the channel lock, replaced objects are destroyed after the lock
is left, and a failed sub-surface reports through a posted notice.

- **N42. Stopping a sub-surface joins its thread from the UI thread, with
  no way out (same class as N2).** `SubSurfaceThread::stop`
  (`sub_surface.cpp:833-852`) sets `stop_requested` and joins. The loop
  sees the flag only in `wait`, between turns (`:409-423`), so the join
  lasts as long as whatever user code the sub-surface is running: a build
  callback, an event handler, a paint. If that code is itself waiting on the
  UI thread (a synchronous `invoke` onto the parent's dispatcher, which
  `Window::invoke` offers to any worker), the two threads wait on each other
  for ever. `stop` runs when the host leaves its window, when its content is
  replaced, and from the destructor. At the least, `stop` should cancel the
  parent-bound synchronous invokes the sub-surface has outstanding before it
  joins, and the header should say a sub-surface must not block on its
  parent.
- **N43. The sub-surface's input queue is unbounded.** `post_input`
  (`:752-765`) appends to `channel.inputs` with no limit and no coalescing.
  The window's own deferred input is bounded and merges consecutive pointer
  moves. A sub-surface stuck in a slow handler while the pointer moves over
  it accumulates every move, then replays them all. Apply the same bound and
  the same coalescing here.
- **N44. A semantic snapshot is built on every semantic change, listener
  or not (performance).** `publish_semantics` (`:625-653`) builds
  `root.semantic_snapshot()`, a walk of the whole sub-surface tree with
  string copies, whenever the semantic generation moved, and only then looks
  at whether anyone listens. Sub-surfaces exist for busy content: a readout
  that changes its text every frame rebuilds the snapshot every frame on the
  sub-surface's thread whether or not an accessibility client is attached.
  Build it only when a listener is set, and coalesce to one per notice
  taken.
- **Triage E1 matters more here.** `publish` (`:655-682`) takes its lease
  with `try_acquire_write(true)`, which copies the whole previous frame
  under the surface mutex before the changed area is copied in. Every frame
  of every sub-surface pays a full-surface copy while holding the lock the
  window's presenter needs. The sub-surface already keeps its own complete
  framebuffer, so it could ask for the lease without preservation and copy
  the area it needs since the buffer it was given last held.
- **N45. A throwing wake leaves the dispatcher asleep.**
  `post_dispatch.cpp`: `wake_pending` is set to true and then
  `(*state).wake()` is called under the lock. If the wake throws (a queued
  block or a posted message that cannot allocate), the exception leaves,
  `wake_pending` stays true, and every later post is counted as coalesced
  and never wakes the host. Clear the flag on the way out of a throwing
  wake.

### Part 7 and 8, first pass: item controls, text, the Windows paint endpoint

- **N41, lowered.** `FlexPanel` admits only finite authored values
  (`flex_panel.cpp:128-148`) and replaces an unbounded extent with a
  practical one (`:675-676`). A NaN can then reach the freeze loop only from
  a child's own measure. Low; one `isfinite` on the measured base would
  close it.
- **N46. `ListBox` publishes every item as a semantic node.**
  `list_box.cpp:483-521` reserves and builds one `SemanticNode` per item,
  visible or not, and copies each item's text twice (name and value). The
  data grid, `ObjectView`, `ListView` and `TreeListView` build only the
  visible window. A 100,000-item list box costs 200,000 string copies per
  semantic snapshot. Use the same windowed form as the others, with
  `set_size` carrying the total.
- **N46a.** `ListBox::on_semantic_child_action` (`:527-533`) finds an item
  by a linear search over `item_stable_ids_`. Same list, same scale.
- **N47. A bound data grid re-indexes every row on every structural
  change.** `on_source_list_changed` (`data_grid_view.cpp:1516-1544`) calls
  `rebuild_row_index` (`:639-649`), which builds a new hash map of every
  row's ID text. Appending n records one at a time to a `BindingSource` is
  n rebuilds: quadratic, with a string per row each time. The unbound path
  already has `index_rows_from(first)`. An `item_added` at the end needs one
  insertion; an insertion or removal in the middle needs the rows after it
  only.
- **N48. `end_device_context_write` can leave the endpoint's mutex locked
  for ever.** This answers the question left open under the ABI part:
  yes, `state_mutex` is held from `begin_write` to `end_write`
  (`windows_compatibility_paint_endpoint.cpp:330-356`), across the foreign
  GDI operation. `end_write` returns false without unlocking when the device
  context does not match (`:347-350`). A shim that passes a different
  handle to the end call, or never reaches it (a structured exception in
  the GDI call between the two), leaves the mutex held by a thread that
  will lock it again on its next `begin_write`: undefined behaviour on
  `std::mutex`, in practice a hang of that thread and of the publisher.
  Unlock on every path where `write_active` and `write_thread` say this
  thread holds it, and treat the mismatch as an error after unlocking.
- **N49. One publisher thread per compatibility endpoint.** `create`
  starts a `std::thread` for each endpoint (`:122`). A WinForms-compatible
  form with a hundred owner-drawn controls has a hundred threads, each with
  its own pending-frame buffer the size of the control. One shared
  publisher with a queue of endpoints that asked would do the same work.
- **N49a.** Each asynchronous publish copies the frame twice and walks it
  a third time for alpha (`:216-253`). One pass that writes
  `pixel | 0xff000000` from the DIB into the pending buffer replaces the
  `memcpy` and the byte loop; the byte-at-a-time alpha loop
  (`:196-198`, `:239-241`) does not vectorise as written.
- **N50. `TextStore::replace` rebuilds everything on each edit.**
  `text_store.cpp:214-283`: every edit builds a whole new string, runs the
  grapheme, line and scalar analysis over all of it, and re-validates the
  removed range. `grapheme_boundaries_` holds one `size_t` per grapheme:
  eight bytes of index per character of text. With the default limit of
  16 MiB (`text_types.hpp:96`), one keystroke at the limit copies 16 MiB and
  rebuilds a 128 MiB vector. `TextBox` caps multiline text at 1 MiB, so the
  worst case there is 1 MiB copied and an 8 MiB index rebuilt per key.
  Re-analyse from the paragraph before the edit to the paragraph after it
  and splice the three index vectors; store boundaries as 32-bit offsets.
- **N51. `TextEditingEngine::replace` adds three more whole-text passes
  per key.** `text_editing_engine.cpp:117-173`: a candidate copy for
  multiline validation (`:142-144`), two `scalar_index` walks from the start
  of the text (`:148-149`, and `TextStore::scalar_index` and `utf16_offset`
  are linear from offset 0 every call, `text_store.cpp:376-397`), and a
  full-text snapshot for undo (`:160`). Count the removed range's scalars
  directly; validate only the inserted text plus its neighbours.
- **N52. Undo stores the whole text per edit.** Each keystroke pushes a
  full copy (`:48-52`, `:160-165`), bounded at 128 entries or 8 MiB
  (`text_editing_engine.hpp:168-169`). In a 1 MiB document the history is
  eight keystrokes deep, and typing a word discards everything before it
  without notice. There is no coalescing of consecutive typing either: one
  undo removes one character. Store the edit (range, removed text, inserted
  text) and merge adjacent typing into one entry.
- **N53. UTF-16 conversions are linear from the start.** `utf16_offset`,
  `utf8_offset(Utf16Offset)` and `scalar_index` walk from byte 0. The C#
  sibling and UI Automation's text pattern both speak UTF-16; a text-pattern
  client that walks a document by ranges makes this quadratic. A per-line
  cumulative UTF-16 count next to `line_starts_` makes each query one
  binary search plus one line.
- **N54. Shaping validation builds a `TextStore` per request, twice.**
  `text_shaping_validation.cpp:50-52` builds a store (string copy, three
  vectors) to answer two boundary questions, and `validate_glyph_run`
  (`:74-91`) validates the request again and builds a second one. The
  HarfBuzz engine builds its own for the same text
  (`harfbuzz_font_engine.cpp:898`). If validation runs on the shaping path
  in release builds this is three analyses of every shaped string; I found
  no caller of either function under `src` outside the file, so check
  whether they are test-only before acting.
- **N55.** `elide_text_end` (`basic_control_rendering.cpp:152-175`) drops
  one grapheme at a time and measures (shapes) the candidate each time: a
  long label in a narrow cell shapes its text once per removed grapheme. A
  binary search over the grapheme count needs about ten measures instead
  of hundreds; cumulative advances from one shape need one.

### Part 8: renderer, text, the Windows host

- **N56. Text has no cache keyed by what it is.** `SkiaRaster::
  measure_text_utf8` (`skia_raster.cpp:1765-1770`) shapes on every call.
  `draw_text_utf8` (`:1734-1743`) shapes and rasterises on every call. The
  only memory is `ShapedTextMemo`, which hangs off one recorded display
  command and is inherited by ordinal when the same control is recorded
  again with the same text at the same position in its list
  (`recording_painter.cpp:19-35`, `:62-71`). Consequences: a label is shaped
  at least twice before it first appears (measure, then draw); scrolling a
  list by one row shifts every text command's ordinal, so every visible row
  is shaped and rasterised again; the same word in a hundred rows is a
  hundred shapes. `HarfBuzzFontEngine::shape` is not cheap: it builds a
  `TextStore` (string copy, grapheme analysis, three vectors), a candidate
  vector, runs bidi, and allocates five more vectors
  (`harfbuzz_font_engine.cpp:876-984`), for "OK" as for a paragraph.
  Two changes, either alone worth having: (1) one bounded table in the
  raster keyed by (text, font, face count) holding the shaped run and its
  coverage per phase, shared by measure and draw; (2) a fast path in
  `shape` for text that is all ASCII with one covering face: no store, no
  bidi, one segment.
- **N57. No glyph cache below the run.** `GlyphRunRasterizer`
  (`glyph_run_raster.cpp:172-288`) loads each distinct glyph once per run,
  then copies the outline and calls `FT_Render_Glyph` for every occurrence
  at its own 1/64 pixel phase. Nothing survives the run. A glyph bitmap
  cache keyed by (face, size, glyph, phase quantised to a quarter pixel)
  would make most text a blit. Quantising changes pixels, so it is an owner
  decision against the engine being its own reference; the measurement to
  take first is the share of paint time in `FT_Render_Glyph` on a full
  data grid.
- **N58. `ShapedTextMemo` is trusted on length alone (latent).**
  `draw_retained_text_utf8` (`skia_raster.cpp:1541-1546`) accepts the memo
  when engine, face count, font and `text.size()` match; the text itself is
  not compared. The recorder never hands a memo to a different text
  (`same_cached_drawing` compares the strings), so nothing is wrong today.
  The method is a public virtual of `Painter` (`painter.hpp:159-164`),
  and a second caller that reuses a cache for "12:01" then "12:02" would
  draw the first. Keep a hash of the text in the memo.
- **N59.** Every recorded text command allocates a `RetainedDrawCache`
  with `make_shared` and copies its text (`recording_painter.cpp:303-316`),
  including for controls recorded once and thrown away. With N56's table
  the per-command cache is not needed.
- **N60. The compatibility executor's text wrap is quadratic and breaks
  inside clusters.** `SkiaExecutor::measure_string`
  (`skia_executor.cpp:880-923`) shapes the whole remaining line for each
  output line, then shapes the fitted prefix again; a paragraph that wraps
  to n lines is about n²/2 line-lengths of shaping. It advances per code
  point, so it can break between a base letter and its combining mark, and
  the comment says left-to-right only. The core has
  `fill_line_break_opportunities_utf8` and grapheme boundaries; shape once,
  walk cumulative advances, break at an opportunity.
- **N61. The Windows host has no input-method support.** No `WM_IME_*`
  message is handled anywhere under `src/host/windows`, and nothing calls
  the IMM or TSF. Committed text still arrives as `WM_CHAR`, but there is
  no inline composition and the candidate window is not placed at the
  caret. The macOS host implements `NSTextInputClient` and the Linux host
  has XIM, and `TextEditingEngine` already has a composing state. For
  Chinese, Japanese and Korean users on Windows this is the difference
  between usable and not; the guardrails say the three hosts are equal.
- **N62. Also absent from the Windows host:** `WM_MOUSEHWHEEL` (horizontal
  wheel and two-finger horizontal scroll on precision touchpads),
  `WM_XBUTTON*` (back and forward buttons), `WM_POINTER*` or `WM_TOUCH`
  (touch and pen arrive only as synthesised mouse), `WM_DISPLAYCHANGE`.
  Check each against what the host capability report says it offers.
- **N63. Animation frames ride `WM_TIMER`.** `collect_damage`
  (`windows_host.cpp:2064-2071`) kills and sets a `SetTimer` for the
  model's next wake after every paint. `WM_TIMER` has a floor near 10 ms,
  a granularity of 15.6 ms unless the process raised the timer resolution,
  and the lowest message priority, so a theme transition runs at an uneven
  40 to 64 frames a second while the queue is busy. The host already has a
  high-resolution waitable timer phased to the display for live surfaces
  (`arm_live_clock`, `:2091-2128`). Use that clock for any wake nearer than
  about 50 ms, and keep `SetTimer` for the long ones.
- **N64.** `arm_live_clock` asks `DwmGetCompositionTimingInfo` with a null
  window, which reports the primary display. A window on a second display
  with a different refresh rate is phased to the wrong one. Low.
- **N65. Native file dialogs use the pre-Vista API.**
  `native_open_dialog` (`:226-275`) uses `GetOpenFileNameW` with a
  65,536-character buffer: a large multiple selection fails with
  `FNERR_BUFFERTOOSMALL` and is reported as a backend failure.
  `native_folder_dialog` (`:323-350`) uses `SHBrowseForFolderW` and
  `SHGetPathFromIDListW`, which is limited to `MAX_PATH` whatever the
  buffer. `utf8_from_wide` returns an empty string for a file name with an
  unpaired surrogate (legal on NTFS), and that empty path is returned as
  accepted. `IFileOpenDialog` has none of the three limits and gives the
  folder picker the same dialog. Report an unconvertible path as a failure,
  not as an empty path.
- **Not defects, noted:** the message loop's bounded turn (32 messages or
  4 ms, then present, `:3342-3371`) is right; `WM_COPYDATA` automation is
  gated by `options_.automation_enabled` (`:2779`); the class has no
  background brush, so there is no erase flicker.

### Part 8: the Linux and macOS hosts

- **N66. The Linux host reports one monitor at scale 1.0.**
  `x11_windowing.cpp:427-433` publishes a single `HostMonitor`
  ("x11-screen", the whole X screen) with `scale = 1.0`, and pointer
  positions are used as logical coordinates unscaled
  (`linux_host.cpp:1221-1235`). On a HiDPI Linux desktop the interface is
  drawn at half size, and on two monitors the "monitor" is the union of
  both, so anything that centres on or clamps to a monitor (dialogs,
  overlays, maximise) straddles the gap. `Xft.dpi` from the resource
  database gives the scale; RandR gives the monitors.
- **N67. Presentation sends pixels through the X socket.**
  `present_raster_rectangles` (`linux_host.cpp:334-391`) converts into a
  client buffer and calls `XPutImage`. A full 4K frame is 33 MB written to
  a Unix socket and read by the server. MIT-SHM (`XShmPutImage`) removes
  both copies on a local display; keep `XPutImage` as the remote fallback.
  The `XCreateImage` per present is small beside that.
- **N68. The X event drain is unbounded and does not merge motion.**
  `Runtime::step` (`:1623-1628`) dispatches until `XPending` is zero before
  any window updates. The Windows host bounds its turn to 32 messages or
  4 ms for exactly this reason. A slow pointer handler under a fast mouse
  never reaches `update`. Bound the turn and collapse consecutive
  `MotionNotify` for one window to the last.
- **N69.** The wheel is core buttons 4 to 7 at a fixed 40 units
  (`:1237-1244`): no XInput2, so no smooth touchpad scrolling and no touch.
  The live clock is a 16 ms poll timeout (`:1651-1652`), not a display
  phase. Both are honest X11 limits; they should appear in the capability
  report if they do not already.
- **macOS present (`macos_host.mm:3231-3287`): no defect found.** The
  `CGImage` wraps the raster without a copy and is drawn and released
  inside `drawRect:`; the copy blend mode and the byte order are right.

### Part 9: the C# sibling and the release workflow

- **N70. An exception in a managed handler is swallowed for as long as the
  run loop runs.** `Callbacks.invoke` (`bindings/csharp/GuiForms.Interop/
  Callbacks.cs`) catches every exception and parks it in a thread-static
  list; `Abi.check` rethrows the oldest on the next successful call on that
  thread. Inside `windows.run` the next call that returns to managed code
  is the end of the program. A click handler that throws does nothing
  visible, the list grows without bound, and when `run` returns the
  program gets the first failure with the rest in `Data`. WinForms raises
  `Application.ThreadException` at once. The thunk should raise a managed
  unhandled-callback event (default: fail fast with the exception) when it
  is the outermost managed frame, and park only when a managed caller is
  below it on the stack to receive the rethrow. Also: when `check` sees a
  non-OK status it throws that and leaves the parked failures to attach
  themselves to an unrelated later call.
- **N71. The binding's callback rule and the library's behaviour
  disagree, and the result is a double free.** The binding says "a
  to_native whose callback the library refused must be freed by the caller
  with free_unregistered" because "it never calls release then". N20 and
  N34 above are places where the library does call release on a failure
  path. On those paths the managed side frees a `GCHandle` the library
  already freed: `InvalidOperationException` at best, and if the slot was
  reused in between, another callback's delegate is freed and its next
  invoke dereferences a dead handle. This raises N20 and N34 from tidiness
  to correctness. Fix the library side (one rule: a non-OK status means the
  callback was not taken and release was not called) and add an ABI test
  that counts release calls per failure path.
- **N72. Every ABI call takes a lock and copies seven tables.**
  `Abi.objects`, `Abi.properties` and the rest each call `tables()`, which
  locks, then builds a `Tables` struct holding all seven tables by value
  and returns one of them by value (`Abi.cs`). A table is a struct of
  function pointers; a property write copies several hundred bytes under a
  process-wide lock to reach one pointer. Negotiate once in a static
  constructor (or `Lazy`), keep the tables in `static readonly` fields and
  return them by `ref readonly`.
- **N73.** `ObjectHandle` stores a `gf_object` in `SafeHandle`'s `IntPtr`
  (`ObjectHandle.cs`). If `gf_object` is 64 bits (slot and generation) a
  32-bit process truncates it. Either state that the sibling is 64-bit
  only and fail at load, or hold the value in a field and use the base
  handle only as the liveness flag.
- **N74.** `release.yml` and `publish-release.yml` hold `contents: write`
  and reference every action by tag (`actions/checkout@v4`,
  `msys2/setup-msys2@v2`, `actions/download-artifact@v4`). A moved tag on
  a third-party action runs in the job that publishes releases. Pin by
  commit SHA in the two release workflows at least. (The triage may already
  carry this from the first audit; listed so it is not lost.)

### Last items

- **N75. The caret blinks at a fixed 530 ms.** `text_box.cpp:28` and
  `rich_text_box.cpp:27` each define the interval; no host setting feeds
  it. A user who turned caret blinking off in the system (Windows:
  `GetCaretBlinkTime` returns `INFINITE`; GTK: `gtk-cursor-blink`; macOS:
  the insertion point blink defaults) still gets a blinking caret. Add the
  blink period, with "never" as a value, to the host's input settings
  beside the hover time it already reads, and keep one copy of the caret
  code for both controls.
- **N76 (low).** `DockSite::end_drag` sets the phase to `settling`, then
  runs the drop transaction and returns early when the site lost its
  workspace during it (`dock_site_drag.cpp:514-533`). `place_settle_target`
  also returns early without a workspace (`:565`), so the session never
  leaves `settling`, the drag layer stays shown and the site refuses new
  drags. Call `finish_settling` on that early return. The rest of the drag
  code (self kept alive, liveness checked after every re-entrant call,
  weak link to the guest site, capture loss decided a turn later, the tree
  generation checked before the drop) I found no fault in.
- **Looked for, not found:** unbounded recursion or input size in the JSON
  and XML readers (both iterative, with depth, value, member, string and
  input limits; the dock layout reader caps depth at 256 and floating
  sites); a perpetual redraw from the search field's busy strip (it
  enlists with the motion clock only while painting and withdraws).

### Second pass: `src/core/window/window.cpp` (read in full)

The input, focus, validation, drag and lifetime code here is careful: every
handler call is followed by a liveness check, lists are copied before user
code runs, and deferred input is bounded and merged. What follows is cost,
not correctness.

- **N77. Every attach and every disposal walks the whole tree.**
  `attach_subtree` (`window.cpp:2672`) and `dispose_subtree` (`:2752`) end
  with `recompute_subtree_dirty(root_)` (`:4320-4327`), which visits every
  control in the window. Adding n children one at a time to an attached
  parent is n walks of a growing tree: quadratic. Ten thousand rows added
  to a live panel is fifty million visits. The attached subtree's summary
  is already known (`mark_subtree_dirty` follows); only its ancestors' flags
  need the new bits, which is a walk up, not a walk of everything.
- **N78. `contains_control` searches downward.** The helper at `:64-77`
  answers "is candidate inside root" by a depth-first search of root's
  whole subtree. `Control::contains` (a walk up the parent chain) exists
  and is used two functions later (`region_holds`). The downward form is
  called from `focus_allowed_by_active_scope` (every focus request inside a
  modal scope), `revoke_interaction_for_subtree` (six times per detach or
  disable), `begin_focus_scope` (twice per overlay), and
  `collect_mnemonic_candidates` (`:961`), where it runs once per control
  visited: one Alt+letter is quadratic in the scope's size. Replace the
  body with the upward walk.
- **N79. A control is found by runtime identity with a tree search.**
  `find_runtime_control` (`:3223-3240`) searches the whole window. Every
  accessibility call that names a control goes through it
  (`perform_semantic_action`, `semantic_text_layout`,
  `set_semantic_text_selection`). A screen reader walking n nodes makes
  this quadratic. The control table already gives each attached control a
  slot; a map from runtime identity to slot beside it makes this constant.
- **N80. Subtree damage is added control by control.** `add_subtree_damage`
  (`:3655-3660`) adds each descendant's bounds to every paint plane.
  Detaching a panel of 5,000 rows is 15,000 region insertions, each of
  which may compact the region. The root's bounds usually contain the
  rest: add them once and descend only into children whose visual bounds
  leave their parent's.
- **N81.** A pointer move hit-tests twice: once for hover
  (`dispatch_pointer`, `:1678`) and once for the route when nothing holds
  the capture (`route_pointer`, `:1741`); a release hit-tests a third time
  (`:1876`). Each test runs `ensure_layout` and builds the overlay list.
  Pass the hover result down.
- **N82.** `SurfaceRoot::find` (`:655-659`) and `unregister_subtree`
  (`:2951-2956`) build a `std::string` per lookup; the map needs a
  transparent hash and comparator. `settle_indices_.reserve(slot_count())`
  on every attach (`:2540`) is the N4 pattern.
- **N83. Absolute bounds are recomputed from the root for every control
  visited.** `absolute_bounds_of` (`:4609-4617`) walks the parent chain,
  and `Control::parent()` locks a `weak_ptr` at each step (two atomic
  operations per ancestor). `paint_recursive` calls it once and
  `visual_bounds_of` once more (`:4467`, `:4473`); `hit_test_recursive`
  once (`:3905`); `find_paint_cover` twice; `paint_damage_bounds_of` twice.
  All of these are descents that already hold the parent's origin. A paint
  of n controls at depth d does about 2·n·d weak locks to learn what one
  addition per level would give. Pass the parent's absolute origin down the
  recursion.
- **N84. Each tree walk copies each node's child list.**
  `measure_dirty_recursive` (`:3961`), `arrange_dirty_recursive` (`:4088`),
  `paint_recursive` (`:4548`), `hit_test_recursive` (`:3919`) and
  `append_semantic_nodes` (`:213`) copy `children_` (a vector of
  `shared_ptr`) before visiting, so that a callback may change the list.
  For a panel with 5,000 rows, every pointer move over it allocates 40 KB
  and does 10,000 atomic reference-count operations, twice (N81); every
  paint that reaches it does the same. Hit testing then scans the children
  linearly from the last. A child-list revision counter on the control,
  checked after each callback (restart or stop if it moved), gives the same
  safety with no copy. For hit testing in a scrolling list, the container
  could answer "which child is at this point" itself (it knows its rows are
  sorted by position).
- **N85. A layout flush walks the whole tree twice after it has done its
  work.** `ensure_layout` (`:3846-3855`) calls `recompute_subtree_dirty`
  and `commit_layout_requests_recursive` on the root and every overlay
  after every pass. Changing one label in a window of 10,000 controls
  visits 20,000 nodes besides the few the dirty flags led to. The paint
  descent already keeps its summary as it returns
  (`summarize_subtree_paint`, `:4442-4448`); measure and arrange can do the
  same, and the suspended-layout commit can keep a list of the controls
  with `layout_deferred_` set.
- **N86.** `update_display_cache_metrics` (`:4577-4585`) counts display
  chunks by walking the whole tree, on every detach and disposal
  (`:2705`, `:2753`). Keep the count where chunks are created and dropped.

### Second pass: `src/core/control/control/control.cpp` (read in full)

- **N87. Inherited values are pulled up the tree on every read.**
  `effectively_visible` (`control.cpp:3079-3090`), `effectively_enabled`
  (`:3092-3102`), `chosen_theme` (`:1206-1211`, recursive),
  `effective_layout_direction` (`:3482-3491`), `ambient_back_color` and
  `ambient_fore_color` (`:1340-1356`), `effective_cursor` (`:3066-3077`)
  and `on_ceremonial_surface` (`:2966-2973`) each walk to the root, locking
  a `weak_ptr` per step. They are read constantly: `eligible` (both
  visibility walks) runs twice per level in `hit_test_recursive` and once
  per control on every input route; `base_anatomy_states` reads
  `effectively_enabled` for every painted control; `resolved_text_font`
  calls `effective_theme` before it can even test its own cache
  (`:1111-1121`). With N83 this makes the per-control cost of paint, hit
  test and routing proportional to depth. Every one of these values
  changes only through a write that already walks the subtree
  (`invalidate_subtree` on `visible`, `back_color`, `layout_direction`;
  `set_theme_override`). Store the effective value on each control during
  that walk, and at attach; reads become a field load.
- **N88. `clear_children` is quadratic, with a whole-tree walk per
  child.** It removes from the back (`:2191-2193`), and `remove_child`
  finds its child by scanning from the front, twice (`:2119-2123`,
  `:2132-2136`), then runs `detach_subtree` (overlay search, focus-scope
  search, six `contains_control` searches, per-descendant damage, part
  record scan N36) and finally `recompute_subtree_dirty` on the window root
  (`:2146`). Emptying a panel of n rows is n² child comparisons plus n
  walks of the window. A bulk form (detach all, one damage rectangle, one
  summary) is what a list reset needs.
- **N89. Dock and anchor layout measures children outside the measure
  cache.** `Control::arrange` calls `(*child).measure(...)` for each
  docked child (`:3521`) and `get_preferred_size` for each automatically
  sized anchored child (`:3567`), and `Control::measure` calls
  `get_preferred_size` on every child of an automatically sized control
  (`:3372`). None goes through `measure_for_layout`, so the cache the
  window filled a moment earlier in `measure_dirty_recursive` is not used,
  and each level of nested automatic sizing measures its whole subtree
  again: about n times depth measures per layout instead of n.
- **Triage A2 still stands** (`:1790`, `:1835`, `:1865`, `:1898` copy the
  whole `PropertyRegistration`), and every property call by name builds a
  canonical `std::string` first (`registered_name`, `:1757`, `:1779`).

### Second pass: theme resolution per part, part painting, theme language

- **N90. Each painted part resolves its recipe from nothing.**
  `Theme::resolve` (`theme.cpp:1955-2027`), reached from
  `paint_part_with_built_in_recipe` (`part_painting.cpp:904-914`) for every
  part of every control being recorded: builds `PartFacts`; fills a fresh
  vector of ancestor facts by walking up the tree (`:1559-1575`) when any
  rule reads ancestors; runs the cascade (N1, no index, no memo); for an
  inherited text colour runs a further full cascade per ancestor
  (`inherited_text`, `:1674-1697`); allocates `scaled`; may run the cascade
  a second time for the em (`:2016`); then copies the whole
  `ControlVisualRecipe`, whose material holds vectors (`:2020`). A data
  grid that re-records on scroll does this for every visible cell part.
  The inputs are few (type, id, classes, part, state set, media, three rule
  set serials, ancestor chain). A per-control table of resolved recipes
  keyed by (part, states), cleared on `Dirty::style` and when a serial
  moves, makes the common repaint a lookup. This is the same memo N1 asks
  for, placed one level up where it also saves the copy.
- **N91. A pointed part is painted ten times.** `paint_pointed_part`
  (`part_painting.cpp:796-821`) paints the whole part once clipped to its
  body and once for each of nine strips of the nose, each a full
  `paint_part` (resolution, material, text). A breadcrumb of eight pointed
  items is eighty part paints. Resolve once and replay the material under
  the ten clips, or give the painter a convex-polygon clip.
- **N92.** `paint_text` (`:603-637`) and `paint_text_part_at` (`:889-902`)
  measure the text when the caller did not, then draw it: two shapes of the
  same string in one call (N56).
- **Looked for, not found:** unbounded nesting or recursion in the theme
  language. The sheet parser and the value parser use fixed arrays of
  `maximum_nesting_depth` and count rules, declarations and imports
  (`theme_sheet_parser.cpp:139-154`, `:238`, `:301`, `:416`, `:929`;
  `theme_value.cpp:401`, `:509-512`, `:624`). The 3,774-line compiler and
  the lowering files run at theme load, not per frame; I checked their
  limits by search and did not read them line by line.

### Second pass: item controls

Checked by search for whole-model loops inside paint, measure, arrange,
hit-test and pointer functions in `list_view`, `tree_list_view`,
`list_box`, `combo_box`, `object_view`, `page_strip`, `ribbon`,
`context_menu` and `data_grid_view_parts`, then read where it mattered.
Painting is windowed to the visible range everywhere except N46. The tree
list's structure (children in one array with per-node ranges, an explicit
stack for the preorder, a row index per node) is good.

- **N93 (low).** The data grid's realized-item queries each recompute
  `grid_layout()` (`data_grid_view_parts.cpp:1921-1987`), and
  `realized_cells` (`:2006-2026`) asks per cell, so one inspection of a
  40 by 30 view computes the layout 1,200 times. Pass the layout in.
- **N94. `TreeListView` and `ListView` can only replace the whole model.**
  The headers offer `set_nodes` (`tree_list_view.hpp:159`) and `set_items`
  (`list_view.hpp:208`) and nothing that adds or removes one. `set_nodes`
  validates every string, rebuilds the ID map, the structure, the order
  and the rows (`tree_list_view.cpp:398-439`). A node marked `expandable`
  whose children are loaded when it opens (a file tree) costs a full
  rebuild of everything already shown, each time. `ListView` has the
  virtual source for large lists; the tree has no equivalent. Add
  `insert_children(node, span)` and `remove_subtree(node)` that splice the
  arrays, or a virtual tree source.
- `ListView::select_only` clears the whole selection array with
  `std::fill` on every plain click (`list_view.cpp:477-483`): linear in the
  item count per click, 100 KB touched for 100,000 items. Clear only what
  was selected when the count is small.

### Second pass: accessibility, build, and corrections from re-reading

- **N95. With an assistive client attached, any repaint invalidates the
  semantic tree.** `SemanticTreeKey` (`windows_semantic_source.cpp:231-275`)
  includes `content_revision` and `display_generation`, which move on every
  paint touch. A blinking caret or a spinner therefore makes the next UI
  Automation query rebuild the whole tree (`semantic_snapshot`, a walk with
  string copies). The semantic generation, the focus, capture and press
  identities, the client size, scale and title are already in the key and
  are what the tree depends on; bounds change only with layout. Replace
  the two paint counters with the layout pass sequence. The rest of the
  adapter is sound: nothing is built unless a client is observed, and the
  tree is reused while the key holds.
- **N96. The build sets no general warning level and no hardening flags.**
  `CMakeLists.txt` has no `-Wall`, `-Wextra`, `-Wconversion`, `-Wshadow`,
  `/W3` or `/W4` anywhere (searched), only a handful of `-Werror=` for
  particular diagnostics (`:16`, `:498`, `:5494-5495`). The shipped shared
  library gets no stack protector, control-flow guard, CET or
  `_FORTIFY_SOURCE`, and no link-time optimisation. Dependencies are pinned
  by commit (`cmake/Audio.cmake:15`, `cmake/Stx.cmake:61`), which is right.
- **Documentation against code:** the generated API pages describe
  `SurfaceRoot::text_input_caret` as "the box an input method places its
  windows by" (`docs/api/generated/gui_forms.SurfaceRoot.rst:482`). Only
  the macOS host reads it; nothing under `src/host/windows` or
  `src/host/linux` does. See N61.

**Corrections after re-reading the cited lines:**

- **N10 narrowed.** `objects.create` does unbind the thread when the type
  cannot be made (`abi_objects.cpp:119-121`). The thread stays bound only
  when `create_catalog_type` throws.
- **N34 narrowed.** `dispatch.begin_invoke` disarms the binding when the
  post is refused (`abi_dispatch.cpp:27-31`). The callback is released on a
  failing call only when `post` throws.
- **N20 confirmed, and it is deliberate.** `abi_subsurfaces.cpp:173-175`
  says a throw from `set_content` "destroys it, releasing the callback, as
  a taken callback" while the slot returns a failure status. The header
  says a call that fails has "not taken it and never calls it or its
  release" (`gf_abi.h:286`). The two cannot both hold.
- **N71 therefore stands but is rare:** the double free needs one of those
  throwing paths (out of memory, or an internal error inside
  `set_content`). Still worth closing before the seal is relied on, because
  the header's rule is what every binding will be written against.
- **N5 lowered.** `run_posted` dereferences the slot it looks up
  (`abi_registry.cpp:833-835`) on the stated invariant that a posted call
  keeps its target's identity live. A defensive check costs nothing; I
  have no input that breaks the invariant.
- **N54 withdrawn as a cost.** `validate_shaping_request` and
  `validate_glyph_run` are called only from `tests/text_shaping_tests.cpp`.
- **N25 confirmed by reading.** `overlays.open_anchored` measures the
  content while it is detached (`abi_overlays.cpp:342-346`), and a detached
  control measures text with `estimate_text_layout_utf8`
  (`control.cpp:1176-1181`), not the font engine. An overlay given a zero
  extent is sized from estimated text widths.

### Third pass: UI Automation events, tool strip, Skia raster

- **N97. Every semantic change tells assistive clients the whole tree
  changed.** `raise_automation_events` (`windows_automation.cpp:1439-1455`)
  raises `StructureChanged(ChildrenInvalidated)` on the root
  unconditionally, before it compares anything. A progress label that
  changes its text makes Narrator or NVDA drop what it cached and read the
  tree again (and each read pays N79 and N95). The function already builds
  the sorted before and after lists; raise the structure event only when
  the set of element identities differs, and on the nearest common parent.
  The MSAA path does the same with `EVENT_OBJECT_REORDER` on the client
  (`windows_accessibility.cpp:761`). Provider lifetime, COM threading
  (`UseComThreading` on an apartment thread) and teardown
  (`UiaReturnRawElementProvider(hwnd, 0, 0, nullptr)`, `:682`) are right.
- **N98. `ToolStrip` recomputes its layout, shaping every label, on every
  query.** `layout()` (`tool_strip.cpp:435-437`) is not kept; it allocates
  three vectors and calls `item_size` per item, which shapes the item's
  text (`:426`, uncached per N56). It is called from paint, from every
  pointer move's hit test (`:706`, `:752`, `:797`), from semantics, and per
  item from the realized-item geometry (`:1307`), which makes one
  inspection quadratic. `StatusStrip` is the same class. `PageStrip` keeps
  its layout under a key (`page_strip.cpp:722-732`); do the same here.
  `ListView`, `TreeListView` and `NavigationPane` also recompute `layout()`
  per call (24, 19 and 18 sites) but theirs is arithmetic.
- **N99.** `SkiaRaster::synchronize_images` (`skia_raster.cpp:886-987`)
  copies every image's pixels into the raster (`SkData::MakeWithCopy`,
  `:947`, `:964`), and each window's raster has its own map, so an image
  costs its size once per window on top of the registry's copy. Any change
  to the registry (one live image updated per frame) walks every image and
  builds a set of all identities. Share immutable pixel storage between
  the registry and the rasters, and keep a list of changed identities.
  The rest of what I read in the raster (frame begin and end, damage
  clipping, the scroll copy's overlap handling, the retained shadow mask,
  the inset shadow) I found no fault in.

### Third pass: macOS and Linux accessibility, form language, tests

- **N100. Each accessibility element copies its node's whole subtree.**
  On macOS `GUIFormsAccessibilityElement` keeps `SemanticNode _node` and
  assigns `_node = node` in `initWithNode:` and `updateWithNode:`
  (`macos_host.mm:1341`, `:1363`). A `SemanticNode` owns its children, so
  the root element copies the entire tree, each child its subtree, and so
  on: n times depth node copies, strings included, on every semantic
  update. On Linux `sync` does `value.node = node;
  value.node.children.clear();` (`linux_accessibility.cpp:647`): the same
  deep copy, made and then thrown away. Copy the node's own fields only (a
  `SemanticNode` without children, or move the children out first).
- **N101. macOS text attributes re-read the text layout on every call.**
  The element keeps one `SemanticTextLayout` "read again by every text
  attribute" (`macos_host.mm:1089-1091`), and the helpers under it count
  UTF-16 units from the start of the value (`:1159-1169`), find the line
  by a linear scan (`:1219-1226`) and the caret box by scanning every
  grapheme (`:1232-1256`). VoiceOver reading a document a character at a
  time asks several attributes per character: each is a whole layout plus
  linear scans, so reading n characters is quadratic. Keep the layout
  until the node's value or the layout pass changes; see N53.
- **N102 (style).** `linux_accessibility.cpp` packs several statements on
  one line throughout (`:647` holds six assignments; `:1049` two calls).
  The spelling check passes it, but section 2 of the house style asks for
  one recognisable operation per expression, written in execution order,
  and this file is the hardest in the tree to review for that reason.
- **N103. The Linux host builds the semantic tree whether or not an
  assistive client exists.** `NativeWindow::update` calls
  `accessibility.update()` every step (`linux_host.cpp:1342`), and
  `update` (`linux_accessibility.cpp:679-705`) takes a full
  `semantic_snapshot` and syncs every node whenever the semantic generation
  moved. The Windows adapter does nothing until a client is observed. On a
  desktop where the accessibility bus is running but no screen reader is
  (the common case), every semantic change costs a tree walk and N100.
  Gate on the bus's `org.a11y.Status` properties (`IsEnabled`,
  `ScreenReaderEnabled`) or on the first incoming query.
- **N104 (tests).** Thirteen test files sleep. Most poll in 1 ms steps,
  which is fine. Three wait a fixed time and then assert:
  `tests/dispatcher_tests.cpp:848` (50 ms),
  `tests/gallery_pages/animation-laboratory_tests.cpp:39` (110 ms),
  `tests/sub_surface_tests.cpp:202` (200 ms). Under a loaded runner these
  are the ones that will fail without a defect. Drive them from the
  injected clock or poll with a deadline.
- **Looked for, not found:** unbounded input in the form language. Values
  are capped at 256 tokens, grid spans are bounded, and nesting is the
  parser's `maximum_nesting_depth` (`form_model.cpp:136`, `:257-258`,
  `:404-405`).

### Fourth pass: the rest of the macOS host (input, timers, text input)

Read: the key, pointer, wheel, drag and text-input methods, the damage
collection, display link, wake timer and shutdown paths, the accessibility
element and its text attributes. Not read line by line: the 340 lines of
private fixture diagnostics at the top, the window and termination
delegates, and `hostJSON`.

- **N105. An input method cannot commit or cancel on macOS, by reading.**
  `keyDown:` sends the key to the model and calls `interpretKeyEvents:`
  only when the model did not handle it (`macos_host.mm:3554-3558`). While
  composing, `TextBox` marks Enter and Escape handled so that they do not
  reach the accept or cancel button (`text_box.cpp:1075-1082`,
  `:1096-1102`), and the editing engine handles the arrow keys and
  Backspace itself (`text_editing_engine.cpp:345-397`). In every one of
  those cases the host then skips `interpretKeyEvents:`, so the input
  method never receives the key that commits, cancels, moves within or
  shortens the composition. While `hasMarkedText` is true the key must go
  to `interpretKeyEvents:` first. **Unproven until tried with a Japanese
  or Chinese input source; I could not run it.**
- **N106. Marked text is inserted again on each update, by reading.**
  `setMarkedText:` sends a `TextInputEvent` with `composing = true`
  (`macos_host.mm:3621-3640`). `handle_text_input`
  (`text_editing_engine.cpp:402-425`) replaces the current selection with
  the event's text and leaves the caret after it; it keeps a `composing_`
  flag but no marked range. The next update of the same composition ("k",
  then "ka", then "か") therefore replaces an empty selection: the text
  accumulates instead of being replaced, unless the input method passes an
  explicit replacement range, which AppKit usually does not for marked
  text. Each update also pushes a whole-text undo entry (N52). The engine
  needs the marked range: replace it on each update, clear it on commit.
  **Unproven for the same reason as N105.** If both hold, the macOS input
  method support that N61 compares Windows against is itself incomplete.
- **N107.** Timed wakes on macOS are a dispatch timer rounded down to whole
  milliseconds (`armWakeTimer`, `macos_host.mm:2741-2752`); the display
  link runs only for live surfaces (`:2557-2567`). Theme animation frames
  are therefore not phased to the display, and a deadline up to a
  millisecond in the future fires early and is armed again. The macOS form
  of N63.
- **N108 (low).** `physical_key_for` (`:501-614`) has no entry for key
  code 10 (the extra key on ISO keyboards), 93 to 95 and 102 and 104 (JIS
  keys), or F18 to F20. Those keys reach controls as physical key 0, so a
  mnemonic or accelerator on them cannot fire.
- Each pointer event runs a further `hit_test` for the cursor
  (`updateCursorAtPoint:`, `:3420-3429`): the third per move (N81). Every
  semantic change posts `NSAccessibilityLayoutChangedNotification`
  (`:2535-2536`), client or not; see N97.
- **Not defects:** the Objective-C exception guard on the wake source
  (`:2766-2774`), the modal-safe main-thread work (`GUIFormsMainWork`), the
  bounded drag pasteboard reads (`:640-700`), and the element table keyed
  by control identity and virtual child (`:1782-1832`).

### Fourth pass: UI Automation text, ribbon, tools, and the public docs

- **N109. UI Automation text ranges rebuild everything per call.** Every
  `ITextRangeProvider` method calls `read_text`
  (`windows_text_automation.cpp:51-61`): a snapshot lookup, a search for
  the element, and `semantic_text_layout`, which lays the whole text out
  again. `GetAttributeValue(IsReadOnly)` does all of that to read one state
  bit (`:494-500`). `boundary_after`, `boundary_before` and
  `enclosing_unit` scan the boundary list from the start (`:93-128`), and
  `move_across` calls them once per unit moved (`:133-149`), so
  `Move(Character, n)` is n scans. The boundaries are sorted: use binary
  search, and keep the layout while the node's value and the layout pass
  are unchanged. The Windows form of N101. Lifetime and reference counting
  in this file are right. `FindText` with `ignoreCase` folds ASCII only
  (`:214-218`, stated in a comment): a search for "é" will not find "É".
- **N110. A composite control is one display chunk, so any hover re-records
  all of it.** `Ribbon::on_paint` (`ribbon.cpp:1167-1249`) paints the tab
  rail, every tab and every group and command of the selected tab into one
  recording. `command_state_changed` limits the damage to one command's
  bounds (`:787-789`), and hot tracking invalidates paint, but the chunk is
  recorded whole either way: every part is resolved (N90) and every label
  measured and drawn. The same holds for `ToolStrip`, `ListView`,
  `TreeListView` and the data grid outside its cell region. The grid's
  content regions (`cells_on_region`) are the mechanism that avoids this;
  the others do not use it. Either give these controls item-level
  sub-chunks, or make part resolution cheap enough (N90, N56) that the
  whole re-record does not matter. The ribbon's own code (the scaling
  ladder kept under a content generation, the latch with hysteresis, the
  stop model) I found no fault in.
- **Tools.** `tools/profile_hot_paths.py` and the fourteen programs under
  `benchmarks/` already give a way to take every "measure first" in the
  triage; there is a paint workload, a text raster probe and a creation
  benchmark, but no workload with several thousand attached controls,
  which is what N77, N83, N84, N85 and N88 need.

**The public docs against the findings (corrections to what I wrote):**

- **N61 is a known and documented gap, and the defect is the capability
  bit.** `docs/user/hosts-and-capabilities.md:277-279` says the Windows
  adapter "advertises `text_composition`, but its current input path
  forwards committed `WM_CHAR` text rather than a TSF preedit session".
  `windows_host.cpp:3238` sets `HostCapability::text_composition`. The
  guardrails say platform differences are capability-reported; a
  capability reported and not provided is the thing to fix first (clear
  the bit), whatever is decided about implementing composition.
- **N57 is a recorded decision, not an oversight.**
  `docs/user/text-engine.md:36-38`: "Rasterized glyph runs are stored with
  the retained display chunk that drew them and released with it. There is
  no global glyph cache or eviction policy." N56 stands as a cost of that
  decision (measure, shape again on every re-record at a new ordinal), and
  what I propose there is a keyed table of shaped runs, which the decision
  does not rule out. A glyph bitmap cache would reverse the decision and
  is the owner's call.
- **N66:** the capability table gives X11 `scale_notifications: no`, which
  is honest about changes at run time; that the scale is always 1.0 is not
  stated anywhere I found.
- N69 and the lack of a Wayland adapter are documented (`:290-292`).

- **N111 (low).** The context menu's rows use fixed logical offsets and
  sizes (`context_menu.cpp:362-363`, `:385-388`, `:408`, `:432`: the mark
  at 7 by 6, the label at x 30, the chevron 18 from the right) while the
  label's font goes through `effective_font` and so follows the text
  scale. The ribbon and the tool strip multiply every such constant by
  `effective_text_scale()`. At a text scale of 2 the menu's text doubles
  and its marks and gutters do not. The menu's lifetime handling (close
  guarded against re-entry, focus scope ended first, the overlay's
  revocation observed, rows holding references into a snapshot that
  outlives them) I found no fault in.

### Fifth pass: page strip, theme compiler, Skia groups and images, macOS draw, RTF, fuzzing

- **N112 (low).** `PageStrip` keeps its layout under a key, but the key
  includes the scroll revision (`page_strip.cpp:349-368`), and any key
  change runs `measure_tabs` (`:729`), which shapes every tab's label
  (`:412`). Scrolling the row by one tab measures all of them again. Keep
  the natural widths under the set and settings revisions and the scale,
  apart from the placement. The rest of the strip (water-filling shrink,
  frozen widths, the two overflow policies) I found no fault in.
- **N113. One enumeration in the theme compiler has no step bound.**
  `lower_rule_palette` (`theme_compiler.cpp:1600-1670`) walks every
  combination of variants of the colour roles a rule reads
  (`next_palette_choice`, `:1672-1685`): the product of the variant counts.
  Each combination that no window can meet is skipped cheaply, but it is
  still visited. The variable path beside it is bounded twice
  (`maximum_variable_combinations` 128, `maximum_variable_steps` 4,096,
  `theme_custom_properties.hpp:27-30`); this one is bounded only by the
  byte budget, which a skipped combination does not spend. There are 41
  colour roles and a rule may hold 64 declarations. A rule reading 25
  roles that each have a light and a dark variant is 33 million visits; 30
  roles is a billion, at theme load, and a user theme is a file the user
  supplies. Count the visits against the same step limit and report
  `too_many_variable_combinations`. The existing theme sheet fuzz target
  would find this as a timeout if its corpus reached it.
- **N114 (low).** `import_rtf` (`rtf_conversion.cpp:215-250`) runs the
  reader outside its `try`, so an allocation failure while reading pasted
  RTF leaves as an exception, while the same failure in the writer is
  returned as `limit_exceeded`. `error_of` throws `std::logic_error` for
  two statuses it does not expect (`:167-176`), also uncaught. The input is
  the clipboard. Put the whole conversion in the `try` and return a
  failure for the unexpected statuses. The parsing itself is the stx
  library's, with limits and an effort bound.
- **N115. Fuzzing covers six parsers and misses four.** `fuzz/` has
  targets for JSON, XML, Markdown, PNG, deflate and inflate, and the theme
  sheet. There is none for the form language loader, the dock layout
  restore, the theme package manifest, or the RTF import, each of which
  reads a file or the clipboard. (The triage of the first audit already
  asks for one on the automation command parser.)
- **N116 (design).** `drawRetainedRect:` (`macos_host.mm:2896-3397`) is
  about 500 lines that decide, with some twenty flags, between copying,
  painting, live-only frames, scroll copies and prepared frames. I read
  the first 330 lines and the presentation tail and found no fault, but I
  could not hold all of it in my head, and neither will the next person to
  change it. It is the macOS counterpart of N39: split it by decision
  (what is reusable, what is damaged, what is drawn, what is presented).
- **Looked for, not found:** in `skia_raster.cpp`, the groups
  (`:1796-1901`: depth-limited with a named refusal, the layer's clip
  state saved and restored, the composite through the shared blend), the
  live surface draw (`:1957-2020`: the byte-copy path only under a
  full-coverage rectangular clip, borrowed pixels released before the
  call returns) and the image region and pattern draws. In the controls,
  every dereference of a control's window that I checked had its null
  test on a preceding line (92 sites listed, a sample of six read).

### Sixth pass: what could be done faster for the same result

Read for this pass: the pixel compositor (`text_composite.cpp`, all of
it), the display command record and replay, the damage region, the
default theme's build, the ribbon's placement (`ribbon_scaling.cpp`
380-697), the layout functions of the navigation pane, list view and
combo box, the second half of the UI Automation text provider, the form
loader (all of it), and the build file's library and executable
structure. "Same result" below means bit-for-bit the same pixels or the
same observable behaviour; where a change would alter a pixel I say so.

- **S1. The pixel compositor does its general-case arithmetic on every
  pixel.** `composite_part_raster` (`text_composite.cpp:403-493`) is the
  loop every cached part, every baked material and every paint group
  passes through, and `composite_text_coverage` (`:182-245`) the loop
  every glyph passes through. Per pixel they do the following, none of
  which varies over the interior of a blit:
  - `prepared_clip_coverage` (`:135-141`): two `span_coverage` calls and a
    64-bit multiply and divide, plus the rounded-corner test. For every
    pixel wholly inside the clip the answer is 256.
  - The blend mode is tested inside the loop (`blend_part_pixel`,
    `:329-364`, four branches; `target.opaque` twice more). The house
    style's own section 3 says to choose the mode before the loop.
  - Four `weighted_channel` calls, each a 64-bit divide by 65,280
    (`:264-267`), then three or four divides by 255
    (`destination_share`). About eight integer divisions per pixel.
  - In the text loop, one 64-bit divide by 65,280 for the alpha (`:232`)
    and three or four divides by 255 (`blend_channel`, `:166-169`).
  - In the scaled loop, `sample_position` divides twice per pixel
    (`:371-376`) and `texel_channel` bounds-tests each of sixteen fetches
    (`:379-388`).

  All of it can go without changing one output bit:
  1. Split each row into its edge pixels and its interior run. The
     interior's clip coverage is 256 by construction (compute the row's
     vertical coverage once, and the first and last fully covered
     columns); only edge pixels and rounded corners take the present
     path.
  2. Choose the blend routine once per call, and the opaque and
     non-opaque variants once per call.
  3. In the interior the weight is one number for the whole call
     (opacity times 256). `weighted_channel(c, w)` then depends only on
     the byte `c`: fill a 256-entry table once per call (256 divisions)
     and index it per pixel. At full opacity the table is the identity,
     so a source pixel with alpha 255 is copied as a word and a run of
     them is a `memcpy`; alpha 0 is already skipped.
  4. Replace each divide by 255 with the exact identity
     `(t + 128 + ((t + 128) >> 8)) >> 8` for `t = x` where the code now
     computes `(x + 127) / 255`, over the range these loops produce
     (0 to 65,025); it gives the same quotient and is what makes the
     loop vectorise. This must be proved over the whole range by an
     exhaustive test before use (65,026 values; a one-line test).
  5. For text, the alpha `(text * colour_alpha * 256 + 32640) / 65280`
     in the interior depends only on the coverage byte: another
     256-entry table per call.
  6. In the scaled loop, step `sample_position` exactly with a quotient
     and remainder accumulator instead of dividing per pixel, and fetch
     without bounds tests for the columns whose two texels are inside
     the source.

  The existing byte-for-byte comparison test between this compositor and
  the Windows DIB compositor, and the cross-platform identity hashes, are
  exactly the guard this change needs. I expect this to be the largest
  single gain in paint time, because it is the inner loop under almost
  everything; `benchmarks/paint_profile_workload.cpp` and
  `tools/profile_hot_paths.py` will say. One thing to check first: if
  the toolchain builds libc++ in a hardening mode, each `span` index in
  these loops is also bounds-checked (I found no hardening setting in
  the build files, so this depends on the toolchain default).
- **S2. Every display command is a 300-byte record.** `DisplayCommand`
  (`display_command.hpp:56-90`) holds, for every operation including
  `save`, `restore`, `translate` and `clip_rect`: two points, a rectangle,
  a colour, a `FontSpec`, an image identity, three doubles, a
  `std::string`, a `std::vector`, and five `shared_ptr`s. Recording
  zero-fills and moves one per call; destroying a chunk runs seven
  destructors per command; replay walks records most of whose bytes are
  unused. A window of 2,000 controls at ten commands each holds about
  6 MB of display lists. Same result: a tagged record per operation
  (the common ones fit in 48 bytes) in one byte stream per chunk, with
  text, gradient stops and shared objects in side arrays indexed from
  the stream.
- **S3. The default theme is compiled from text at every process start.**
  `gui_forms_theme_build` (`gui_forms_theme.cpp:521-553`) tokenises,
  parses and compiles the floor sheet and the light sheet, evaluates the
  floor (about 100 KB of recipes, `:404-432`), derives the dark variant
  and the high-contrast recipes (`theme.cpp:1496-1555`), on first use.
  The inputs are compile-time constants, and the build already records
  its own duration (`build_time`). Compile the two sheets at build time
  into static data, as the form compiler already does for forms; the
  five familiar themes (`familiar_themes.cpp:1825-1841`) the same. This
  also shortens every one of the 407 test processes that touches a
  control (S9). `gui_forms_theme_definition()` (`:539-542`) returns a
  copy of the 100 KB definition on each call; return a reference.
- **S4. Line height is found by shaping the string "Mg".** Eleven sites
  in nine files call `resolve_text_layout_utf8("Mg", font)` and read the
  height (`list_view.cpp:1149-1152`, `page_strip.cpp:300`,
  `tool_strip.cpp:357`, and others). Each is a full `shape` (N56). The
  list view's is inside `layout()`, which runs on every pointer move.
  The face already has ascent, descent and line gap: add a metrics query
  that reads them, kept per face and size. If the shaped height can
  differ from the face metrics (fallback faces), keep one shaped value
  per font under a key.
- **S5. Controls recompute their layout on every query and return it by
  value.** Generalises N98. `layout()` is called per paint, per pointer
  move, per hit test and per realized-item query: 24 sites in
  `list_view.cpp`, 22 in `page_strip.cpp`, 19 in `tree_list_view.cpp`, 18
  in `navigation_pane.cpp`, 11 each in `tool_strip.cpp` and `ribbon.cpp`.
  Only `PageStrip` keeps the result under a key
  (`page_strip.cpp:370-374`). The ribbon's (`ribbon_scaling.cpp:605-629`)
  measures no text but builds a vector per group plus six more per call;
  the navigation pane's builds a `shown` vector and walks every page
  (`navigation_pane.cpp:631-640`); the tool strip's shapes every label.
  Give each the page strip's arrangement: a key of what the layout
  depends on, a kept result, a `const&` return.
- **S6. `publish_change` builds a `std::function` to call it at once.**
  `Control::publish_change(const void*, std::function<void()>)`
  (`control.cpp:3303-3322`) is the path of every property change
  notification and every focus, capture and availability publication.
  Outside initialisation it calls the function and returns. The callers
  pass function objects holding shared pointers and strings
  (`FocusChangePublication`, `PointerCapturePublication` with its
  `std::string`), which do not fit a `std::function`'s inline buffer, so
  each publication allocates and frees. Test `initialization_depth_`
  first and call the object directly; build the `std::function` only
  when the publication has to be kept.
- **S7. A form loaded at run time sets every property by name.**
  `FormLoader::set_property` (`form_loader.cpp:148-179`) calls
  `set_property_value(descriptor.name, value)`, which builds the
  canonical name string, looks in the registry map and then scans the
  type's property table (`control.cpp:1777-1803`), per property per
  control. The model already resolved the property against the schema
  (`form_property_descriptor`); carry the table entry in the model and
  write through it. The loader does build the tree detached and attach
  it once, which is right (N77 does not apply to it).
- **S8. `document_range`, `caret_range` and `GetEnclosingElement` lay the
  text out to return a number they do not need it for**
  (`windows_text_automation.cpp:835-853`, `:547-563`): each calls
  `read_text`, which runs the full layout, then uses only the value's
  size, the caret offset, or nothing. Split `read_text` into the lookup
  and the layout. Part of N109.
- **S9. The test suite links 439 executables against static libraries
  and starts 407 processes.** `CMakeLists.txt` defines 439 executables;
  the core and the controls are `STATIC` (`:233`, `:772`), so each test
  links its own copy, and there is no unity build, no precompiled header
  and no compiler launcher set in the build files (the shared ccache is
  outside them). `control.hpp` alone is 1,930 lines with 39 includes and
  is reached from nearly every test. Same tests, faster: build the core
  and controls once as shared objects for the test configuration (or as
  object libraries linked into a handful of per-area test executables
  that take a test name), and add a precompiled header of the dozen
  headers every translation unit includes. With S3 this also removes a
  theme compile from each test process.
- **Also seen, smaller:**
  - `DamageRegion::add` restarts its merge scan after every merge
    (`damage_region.cpp:15-32`); bounded by `maximum_rectangles`, so it
    stays small, but N80's per-descendant adds multiply it by three
    planes.
  - `GlyphRunRasterizer` divides by 255 per pixel when combining glyphs
    (`glyph_run_raster.cpp:281`); same identity as S1.4.
  - `Theme::resolve` allocates `scaled` with room for every recipe field
    on each authored resolution (`theme.cpp:2000-2001`); a fixed array
    of `recipe_field_count` on the stack does the same.

**Order I would take them in, by expected gain for the effort:** S1 (one
file, guarded by existing tests), N56 with S4 (text measured and shaped
once), N90 with N1 (recipes resolved once), S5 (layouts kept), then the
tree-walk group (N77, N83, N84, N85, N87, N88), then S2, S3 and S9. The
first four are per-pixel, per-string and per-part costs that every frame
pays; the tree-walk group only shows in large windows; S2, S3 and S9 are
memory, start-up and build time.

## 4. Where this stands, and options for the owner

The triage of every finding is in `AUDIT_ADDENDUM_TRIAGE_2026-11-10.md`
beside this file; its last sections list what was not read and the
options. The text below is the earlier summary and is superseded by it.

Read against `D:\gui_forms_audit` at `ae189ff4b`. Nothing was built or run.
The ledger in section 3a says what was read and what was not; about a third
of the tree by lines was read, chosen for the sealed surface first and then
for the code that runs per frame, per keystroke or per item.

The findings sort into five groups. Each is an option, not an assignment.

a. **Sealed ABI before the seal is relied on** (permanent if missed):
   N71 with N20 and N34 (callback release on failure, a double free from
   C#), N9 (data race on `active_frames`), N5 (null dereference), N14 and
   N27 (arrays of growable records indexed by the library's `sizeof`), N10,
   N17, N24, N32, N33, and the header comments K1, K2, K9 to K11.
b. **Correctness outside the ABI:** N48 (endpoint mutex left locked), N2
   (background worker deadlock), N42 (sub-surface stop joins from the UI
   thread), N45 (dispatcher left asleep), N70 (managed exceptions
   swallowed), N65 (dialog limits), N76.
c. **Platform parity:** N61 (no input method on Windows), N62, N66 (Linux
   scale and monitors), N68, N69, N75.
d. **Speed, in the order I would measure them:** N56 (text shaped and
   rasterised with no keyed cache), N50 to N52 (whole-text work per
   keystroke), N1 and N35 (cascade and cache-hit cost), N63 (animation on
   `WM_TIMER`), N67 (X socket present), N47, N46, N4 (quadratic `reserve`),
   N72 (lock and copy per C# call), N57, N60, N49.
e. **Finish the read.** Not read on main: `window.cpp` and `control.cpp`
   outside the measure cache and dirty settle, the theme compiler and the
   material primitives, the form language, most controls (ribbon, page
   strip, tree list, navigation pane, menus, dialogs), Windows
   accessibility, most of the macOS host, the build files, the tests and
   the public docs.

I recommend (a) first, because it is the only group that cannot be fixed
after 1.0, then N48 and N61 from (b) and (c), then a measurement of N56
before any other speed work: if text is the cost I think it is, several of
the other speed items shrink beside it.
