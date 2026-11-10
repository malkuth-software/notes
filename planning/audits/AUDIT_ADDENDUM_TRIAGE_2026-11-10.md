# Triage of the 2026-11-10 audit addendum

Every finding in `AUDIT_ADDENDUM_2026-11-10.md` (K1 to K12, N1 to N96),
read against `D:\gui_forms_audit` at `ae189ff4b` (origin/main, #477).
Nothing was built or run. The 82 findings of the first audit are in the
coordinator's `AUDIT_TRIAGE_2026-11-10.md` and are not repeated here.

**The owner decides which findings are acted on.** Every verdict is a
recommendation; the batches at the end are options, not assignments.
Nothing has been routed to a worker.

Columns, as in the coordinator's triage:

- **Status:** confirmed (the cited lines say it), narrowed (true on fewer
  paths than first written), withdrawn, or unproven (needs a test or a
  measurement before anyone acts).
- **Recommendation:** fix, fix later (before 1.0 or with the stage that owns
  it), measure first, document, or none.
- **Severity:** correctness, sealed (correctness or shape of the sealed
  ABI), parity (one host lacks what the others have), performance, design.
- **Size:** S (one function), C (one file or one mechanism), B (crosses
  subsystems).

## A. Sealed C ABI: header (`include/gui_forms/c/gf_abi.h`)

| Id | Status | Recommendation | Severity | Size |
|---|---|---|---|---|
| K1 | Confirmed. `gf_font` is embedded in `gf_value`, so it cannot grow, and it names no face. | Fix before more consumers bind: reserve fields, or carry a face identity. | sealed | C |
| K2 | Confirmed. `gf_schema_interaction.payload` carries C++ type spellings. | Fix: publish payload names from the ABI's own vocabulary. | sealed | C |
| K3 | Confirmed. `gf_value` is 208 bytes. | None for 1.x; note for a 2.0. | design | — |
| K4 | Confirmed. One child order in the ABI, two in C++. | Document which order the ABI means. | design | S |
| K5 | Confirmed. Three colour layouts. | Document; unify at 2.0. | design | S |
| K6 | Confirmed. Process-wide singletons are sealed in. | None for 1.x. | design | — |
| K7 | Confirmed. 36 of 64 host capability bits used. | None now; plan the second word before bit 56. | design | S |
| K8 | Narrowed. Nested dialogs overwrite stored paths only if read after a later dialog. | Document the read-before-next-dialog rule. | sealed | S |
| K9 | Confirmed. Text offsets are UTF-8 only; C# needs UTF-16. | Fix later: add conversion slots (minor addition). See N53. | sealed | C |
| K10 | Confirmed. A custom control paints raw pixels; no painter table. | Owner decision: is a painter table wanted in 1.x. | design | B |
| K11 | Confirmed. A custom control has no measure, hit-test or semantics callback. | Fix later (minor addition) if custom controls are to be first-class. | sealed | C |
| K12 | Withdrawn in part. Row lookup is hashed; one string allocation per call remains. | None. | performance | — |

## B. Sealed C ABI: implementation (`src/abi/stable`)

| Id | Status | Recommendation | Severity | Size |
|---|---|---|---|---|
| N20 | Confirmed and deliberate: `set_content` releases the callback on a throw while returning failure (`abi_subsurfaces.cpp:173-175`), against `gf_abi.h:286`. | Fix: disarm on every failure path, one rule. | sealed | S |
| N34 | Narrowed: released only when `post` throws (`abi_dispatch.cpp:26-33`); the same shape in `abi_rules.cpp:424-425`. | Fix with N20. | sealed | S |
| N71 | Confirmed, rare: with N20 or N34 the C# side frees the `GCHandle` a second time. | Fix with N20; add a conformance case that counts releases per failing slot. | sealed | S |
| N9 | Unproven. `active_frames()` is one static vector (`abi_event_fields.cpp:23-26`) commented "UI thread only"; a race needs an events slot running on a sub-surface thread. | Run the ABI sub-surface tests under ThreadSanitizer; make it `thread_local` either way. | sealed | S |
| N14 | Confirmed: `value.elements[index]` steps by the library's `sizeof(gf_value)` while checking each element's `struct_size` (`abi_values.cpp:141-142`); the same in `abi_custom_controls.cpp:215`, `abi_application.cpp:94`. | Fix: step by the caller's element size, or state that arrays are of the negotiated size. | sealed | C |
| N27 | Confirmed: rules arrays, same shape (`abi_rules.cpp:556`, `:564`). | Fix with N14. | sealed | S |
| N10 | Narrowed: the thread stays bound only when creation throws (`abi_objects.cpp:115-131`, `abi_custom_controls.cpp:209-229`). | Fix: unbind in the catch. | sealed | S |
| N5 | Lowered: null dereference only if the posted-call invariant breaks (`abi_registry.cpp:833-835`). | Add the check. | correctness | S |
| N6 | Confirmed: status chosen from the C++ exception type. | Fix later: explicit results at the slots that have expected alternatives. | design | C |
| N7 | Confirmed: a third UTF-8 validator. | Fix later: one validator. | design | S |
| N8 | Confirmed: dispose scans all subscriptions. | Fix later: per-object list. | performance | S |
| N11 | Confirmed: `insert_child` is two mutations. | Fix later: one validated mutation. | correctness | S |
| N12 | Confirmed: allocations per event. | Measure first. | performance | C |
| N13 | Confirmed: four to five mutex round trips per call. | Measure first; one lock per call is reachable. | performance | C |
| N15 | Confirmed: reading a collection property element by element is quadratic (`abi_properties.cpp:210-222`). | Fix: bulk read. | performance | S |
| N16 | Confirmed: undocumented 4,096-element cap. | Document in the header. | sealed | S |
| N17 | Confirmed: a custom control's repaint is lost when no write lease is free (`abi_custom_control.cpp:271-276`). | Fix: retry on the next frame. | correctness | S |
| N18 | Confirmed: a pixel buffer per custom control. | None unless custom controls multiply. | performance | — |
| N19 | Confirmed: properties resolved by name on every call. | Fix later: property handles (minor addition). | performance | C |
| N21 | Confirmed: `run` mutates the window before `begin_run` can refuse (`abi_windows.cpp:275-290`). | Fix: validate, then mutate. | correctness | S |
| N22 | Confirmed: endpoint lookup copies every endpoint. | Fix. | performance | S |
| N23 | Confirmed: a release from a finalizer records a failure and returns OK. | Document; it is the only workable answer for a finalizer. | design | S |
| N24 | Confirmed: an anchored overlay's callback can be released during its own invoke (`abi_overlays.cpp:237-284`). | Fix: defer the release as events do. | sealed | S |
| N25 | Confirmed by reading: a zero-extent anchored overlay is sized from estimated text (`abi_overlays.cpp:342-346`, `control.cpp:1176-1181`). | Fix: attach, then measure. | correctness | S |
| N26 | Confirmed: overlay map entries are never erased. | Fix. | correctness | S |
| N28 | Withdrawn: the rule set compacts only at depth 0. | None. | — | — |
| N29 | Confirmed: a SET action's value is not converted when the rule is added. | Fix: convert at add, fail there. | correctness | S |
| N30 | Confirmed: rule-action failures are attributed to `rules.add`. | Fix later. | design | S |
| N31 | Confirmed: binding sources push only; a field-name map per record. | Fix later. | performance | C |
| N32 | Confirmed: host services always use the run's first window; `gf_dialog_request` has no owner. | Fix later: an owner field (minor addition). | sealed | C |
| N33 | Confirmed: `publish_value` reaches the main window only. | Fix with N32. | correctness | S |
| N4 | Confirmed: `reserve(size() + 1)` at about thirty sites makes growth quadratic. | Fix: plain `push_back`, or reserve geometrically. | performance | C |

## C. Correctness outside the ABI

| Id | Status | Recommendation | Severity | Size |
|---|---|---|---|---|
| N48 | Confirmed: `end_write` returns without unlocking on a mismatched device context; the mutex is held across the foreign GDI call (`windows_compatibility_paint_endpoint.cpp:330-356`). | Fix. | correctness | S |
| N2 | Confirmed: `BackgroundWorker` can deadlock or freeze the UI thread (`background_worker.cpp:68-83`, `:268-279`). | Fix. | correctness | C |
| N42 | Confirmed: `SubSurfaceThread::stop` joins from the UI thread while the sub-surface may be waiting on it (`sub_surface.cpp:833-852`). | Fix: cancel outstanding invokes before the join. | correctness | C |
| N45 | Confirmed: a throwing wake leaves `wake_pending` set (`post_dispatch.cpp`). | Fix. | correctness | S |
| N43 | Confirmed: the sub-surface input queue is unbounded and unmerged (`sub_surface.cpp:752-765`). | Fix: the window's bound and merge. | correctness | S |
| N70 | Confirmed: a managed handler's exception is parked until `run` returns (`Callbacks.cs`, `Abi.cs`). | Fix: raise an unhandled-callback event from the thunk. Owner decision on the default (fail fast or continue). | correctness | C |
| N65 | Confirmed: pre-Vista dialogs; 65,536-character buffer; `MAX_PATH` folder paths; an unconvertible path returned as an empty accepted path (`windows_host.cpp:226-350`). | Fix later: `IFileDialog`. The empty-path case is S and can go first. | correctness | C |
| N76 | Confirmed, low: a dock site that loses its workspace during a drop stays in `settling` (`dock_site_drag.cpp:514-533`, `:565`). | Fix. | correctness | S |
| N113 | Confirmed by reading; not run: the palette enumeration in the theme compiler has no step bound (`theme_compiler.cpp:1600-1685`), so a rule reading many varying colour roles can hang theme load. | Fix: count visits against `maximum_variable_steps`. Add a fuzz seed that reaches it. | correctness | S |
| N114 | Confirmed, low: `import_rtf` lets a reader allocation failure and two unexpected statuses escape as exceptions (`rtf_conversion.cpp:167-176`, `:215-222`). | Fix. | correctness | S |
| N115 | Confirmed: no fuzz target for the form loader, dock layout restore, theme manifest or RTF import. | Fix later: four small targets. | design | C |
| N116 | Confirmed: `drawRetainedRect:` is about 500 lines and twenty flags (`macos_host.mm:2896-3397`). | Fix later with N39: split by decision. | design | C |
| N112 | Confirmed, low: scrolling the page strip re-measures every tab label (`page_strip.cpp:349-368`, `:729`). | Fix later, or leave to N56. | performance | S |
| N3 | Confirmed: dispatcher states signalled by exception. | Fix later. | design | C |
| N41 | Lowered: the flex freeze loop cannot end on NaN, but `FlexPanel` admits only finite values. | Add one `isfinite` on the measured base. | correctness | S |
| N58 | Confirmed, latent: `ShapedTextMemo` is accepted on text length alone (`skia_raster.cpp:1541-1546`). | Fix: keep a hash of the text. | correctness | S |
| N73 | Unproven: `ObjectHandle` keeps a `gf_object` in an `IntPtr`; wrong only in a 32-bit process. | Document 64-bit only, or hold the value in a field. | correctness | S |
| N74 | Confirmed: release workflows hold `contents: write` and reference actions by tag. | Fix: pin by commit in the two release workflows. | correctness | S |
| N96 | Confirmed: no general warning level, no hardening flags, no link-time optimisation. | Owner decision: a warning baseline is a broad clean-up; hardening flags on the shipped library are small. | design | B |

## D. Platform parity

| Id | Status | Recommendation | Severity | Size |
|---|---|---|---|---|
| N61 | Confirmed, and already documented as a gap (`docs/user/hosts-and-capabilities.md:277-279`). The defect is that `windows_host.cpp:3238` still reports `HostCapability::text_composition`. | Fix now: clear the capability bit (S). Then place the candidate window at `text_input_caret` (S); inline composition is C. | parity | S, then C |
| N105 | Unproven, by reading: on macOS a key the model handles never reaches `interpretKeyEvents:`, and the text box handles Enter, Escape, arrows and Backspace while composing (`macos_host.mm:3554-3558`, `text_box.cpp:1075-1102`). | Test with a Japanese input source first. If it fails: route keys to the input method while text is marked. | correctness | S |
| N106 | Unproven, by reading: marked text is inserted again on each composition update; the engine keeps no marked range (`text_editing_engine.cpp:402-425`). | Same test. If it fails: keep and replace the marked range. | correctness | C |
| N107 | Confirmed: macOS timed wakes are a millisecond-rounded dispatch timer, not the display link (`macos_host.mm:2728-2756`). | Fix with N63. | parity | S |
| N108 | Confirmed, low: ISO, JIS and F18 to F20 key codes map to no physical key (`macos_host.mm:501-614`). | Fix later. | parity | S |
| N111 | Confirmed, low: context menu geometry ignores the text scale (`context_menu.cpp:362-432`). | Fix later. | parity | S |
| N62 | Confirmed: no `WM_MOUSEHWHEEL`, `WM_XBUTTON*`, `WM_POINTER*`, `WM_TOUCH`, `WM_DISPLAYCHANGE`. | Fix the horizontal wheel now (S); the rest with the capability report. | parity | C |
| N66 | Confirmed: the Linux host reports one monitor at scale 1.0 (`x11_windowing.cpp:427-433`). | Fix: `Xft.dpi` and RandR. | parity | C |
| N68 | Confirmed: the X event drain is unbounded and does not merge motion (`linux_host.cpp:1623-1628`). | Fix: the Windows host's bounded turn. | parity | S |
| N69 | Confirmed: fixed-step wheel, 16 ms poll clock. | Document in the capability report. | parity | S |
| N75 | Confirmed: caret blink fixed at 530 ms in two controls; no host setting. | Fix: a blink period in the host input settings, "never" included. | parity | S |
| N64 | Confirmed, low: the live clock is phased to the primary display. | Fix later. | parity | S |

## E. Performance, structural (cost grows with the size of the window)

| Id | Status | Recommendation | Severity | Size |
|---|---|---|---|---|
| N77 | Confirmed: `attach_subtree`, `dispose_subtree` and `remove_child` each end with a walk of the whole tree (`window.cpp:2672`, `:2752`; `control.cpp:2146`). | Fix: mark ancestors only. | performance | S |
| N88 | Confirmed: `clear_children` is quadratic, with N77 per child (`control.cpp:2191-2193`, `:2119-2136`). | Fix: a bulk detach. | performance | C |
| N85 | Confirmed: every layout flush walks the whole tree twice (`window.cpp:3846-3855`). | Fix: summarise on the way back up, as paint does. | performance | C |
| N84 | Confirmed: each walk copies each node's child list (`window.cpp:3961`, `:4088`, `:4548`, `:3919`, `:213`). | Fix: a child-list revision check in place of the copy. Measure the 5,000-row pointer move first to size the gain. | performance | C |
| N83 | Confirmed: absolute bounds recomputed from the root per control visited (`window.cpp:4609-4617` and callers). | Fix: pass the origin down the descent. | performance | C |
| N87 | Confirmed: inherited values pulled up the tree on every read (`control.cpp:3079-3102`, `:1206-1211`, `:3482-3491`, `:1340-1356`). | Fix: store effective values during the subtree walks that already happen. | performance | B |
| N78 | Confirmed: `contains_control` searches downward; a mnemonic is quadratic (`window.cpp:64-77`, `:961`). | Fix: use `Control::contains`. | performance | S |
| N79 | Confirmed: runtime-identity lookup is a tree search (`window.cpp:3223-3240`). | Fix: a map beside the control table. | performance | S |
| N80 | Confirmed: subtree damage added control by control (`window.cpp:3655-3660`). | Fix. | performance | S |
| N81 | Confirmed: two or three hit tests per pointer event. | Fix. | performance | S |
| N82 | Confirmed: a `std::string` per ID lookup; N4 at `:2540`. | Fix: transparent hashing. | performance | S |
| N86 | Confirmed: display cache metric counted by tree walk on every detach. | Fix: keep a counter. | performance | S |
| N89 | Confirmed: dock and anchor layout measure outside the measure cache (`control.cpp:3521`, `:3567`, `:3372`). | Fix: route through `measure_for_layout`. | performance | C |
| N36 | Confirmed: `release_control` scans every part record per detached control. | Fix: index records by control. | performance | S |
| N46 | Confirmed: `ListBox` publishes every item as a semantic node (`list_box.cpp:483-521`); N46a linear action lookup. | Fix: window it like the other item controls. | performance | S |
| N47 | Confirmed: a bound data grid re-indexes every row per structural change (`data_grid_view.cpp:1516-1544`, `:639-649`). | Fix: index from the changed row. | performance | S |
| N94 | Confirmed: `TreeListView` and `ListView` can only replace the whole model. | Owner decision: incremental splice or a virtual tree source. | design | C |
| N95 | Confirmed: with an assistive client attached, any repaint invalidates the semantic tree (`windows_semantic_source.cpp:231-275`). | Fix: key on layout, not paint. | performance | S |
| N44 | Confirmed: a sub-surface builds a semantic snapshot on every change, listener or not (`sub_surface.cpp:625-653`). | Fix. | performance | S |
| N97 | Confirmed: every semantic change raises `ChildrenInvalidated` on the root (`windows_automation.cpp:1439-1455`). | Fix: raise it only when the identity set changed. Check with Narrator and NVDA. | performance | S |
| N98 | Confirmed: `ToolStrip::layout()` is recomputed, shaping every label, on every pointer move and query (`tool_strip.cpp:435-437`, `:426`). | Fix: keep it under a key, as `PageStrip` does. | performance | S |
| N99 | Confirmed: image pixels copied into each window's raster; the whole registry walked on any change (`skia_raster.cpp:886-987`). | Fix later: shared storage, changed-identity list. | performance | C |
| N100 | Confirmed: each accessibility element deep-copies its node's subtree (`macos_host.mm:1341`, `:1363`; `linux_accessibility.cpp:647`). | Fix: copy the node's own fields. | performance | S |
| N101 | Confirmed by reading; size unproven: macOS text attributes rebuild the layout and scan linearly per call (`macos_host.mm:1089-1256`). | Fix later, with N53. | performance | C |
| N103 | Confirmed: the Linux host builds and syncs the semantic tree with no client attached (`linux_host.cpp:1342`, `linux_accessibility.cpp:679-705`). | Fix: gate on the accessibility bus status or the first query. | performance | S |
| N102 | Confirmed: `linux_accessibility.cpp` packs several statements per line. | Fix later: reformat to the house style (no behaviour change). | design | C |
| N104 | Confirmed: three tests assert after a fixed sleep (`dispatcher_tests.cpp:848`, `animation-laboratory_tests.cpp:39`, `sub_surface_tests.cpp:202`). | Fix: injected clock or a deadline poll. | design | S |

## F. Performance, text and theme (cost per thing drawn)

| Id | Status | Recommendation | Severity | Size |
|---|---|---|---|---|
| N56 | Confirmed by reading; size unproven: no cache keyed by text; measure shapes, draw shapes and rasterises; scrolling re-shapes every row (`skia_raster.cpp:1734-1770`, `recording_painter.cpp:19-71`, `harfbuzz_font_engine.cpp:876-984`). | **Measure first** (share of a data-grid scroll frame in `shape` and `FT_Render_Glyph`). Then a keyed table and an ASCII fast path. | performance | C |
| N57 | Confirmed, and a recorded decision (`docs/user/text-engine.md:36-38`: no global glyph cache). | None unless the N56 measurement makes the owner reopen it. | design | — |
| N109 | Confirmed: every UI Automation text-range call lays the text out again and scans boundaries linearly; `Move` by n is n scans (`windows_text_automation.cpp:51-149`). | Fix later with N101 and N53: keep the layout, binary search. | performance | C |
| N110 | Confirmed: a composite control is one display chunk, so a hover re-records all its parts (`ribbon.cpp:1167-1249`). | Owner decision: item-level sub-chunks or content regions for ribbon, tool strip and list views, or rely on N90 and N56 making the re-record cheap. | design | B |
| N59 | Confirmed: a `make_shared` cache and a text copy per recorded text command. | Folds into N56. | performance | S |
| N92 | Confirmed: `paint_text` measures then draws. | Folds into N56. | performance | S |
| N1 | Confirmed: cascade resolution has no index and no memo (`theme_engine.cpp:877-902`, `theme_rules.cpp:2295-2318`, `:2511-2528`). | Fix with N90. | performance | C |
| N90 | Confirmed: each painted part resolves its recipe from nothing and copies it (`theme.cpp:1955-2027`, `:1674-1697`). | Fix: per-control memo keyed by part and states. | performance | C |
| N35 | Confirmed: cost on a part-cache hit (`theme_engine.cpp:1082-1286`, `engine_part_table.cpp:238-263`). | Measure with N90. | performance | C |
| N37 | Confirmed: eviction scans. | Fix later. | performance | S |
| N38 | Unproven: fixed raster budgets (8 MiB, 4 MiB) may thrash on large or dense windows. | Measure on a 4K window. | performance | S |
| N39 | Confirmed: `paint_planned` is 420 lines. | Fix later: split by phase. | design | C |
| N40 | Confirmed: the tokenizer reserves for the worst case. | None; bounded by input size. | performance | — |
| N91 | Confirmed: a pointed part is painted ten times (`part_painting.cpp:796-821`). | Fix: resolve once, replay under the clips. | performance | S |
| N50 | Confirmed: `TextStore::replace` rebuilds the whole text and its indexes per edit; 8 bytes of index per grapheme (`text_store.cpp:214-283`). | Fix: local re-analysis, 32-bit offsets. | performance | C |
| N51 | Confirmed: three more whole-text passes per key in the editing engine (`text_editing_engine.cpp:117-173`). | Fix. | performance | S |
| N52 | Confirmed: undo stores the whole text per edit, no merging; eight steps deep at 1 MiB. | Fix: store edits, merge typing. This one users will notice. | correctness | C |
| N53 | Confirmed: UTF-16 conversions are linear from the start (`text_store.cpp:335-397`). | Fix with K9. | performance | S |
| N54 | Withdrawn: test-only. | None. | — | — |
| N55 | Confirmed: elision shapes once per removed grapheme. | Fix: binary search or cumulative advances. | performance | S |
| N60 | Confirmed: the compatibility executor's wrap is quadratic and can break inside a cluster (`skia_executor.cpp:880-923`). | Fix. | correctness | S |
| N93 | Confirmed, low: data grid inspection recomputes the layout per item. | Fix later. | performance | S |

## F2. Same result, faster (the sixth pass)

Each of these leaves pixels and behaviour unchanged; the guard that
proves it is named.

| Id | Status | Recommendation | Guard | Size |
|---|---|---|---|---|
| S1 | Confirmed by reading; gain unmeasured: the pixel compositor recomputes clip coverage, tests the blend mode and does about eight integer divisions per pixel (`text_composite.cpp:182-245`, `:403-493`). | Fix first: interior runs, mode chosen per call, per-call tables, exact divide-by-255, word copies of opaque runs. Measure with `paint_profile_workload` before and after. | The byte-for-byte compositor comparison test and the cross-platform identity hashes; an exhaustive test of the division identity. | C |
| S4 | Confirmed: line height found by shaping "Mg" at eleven sites, one inside `ListView::layout()` (`list_view.cpp:1149-1152`). | Fix: a face metrics query, or one shaped value kept per font. | Layout oracle tests. | S |
| S5 | Confirmed: `layout()` recomputed and returned by value per query in six composite controls; only `PageStrip` keeps it. | Fix: keyed, kept layout, `const&` return. Supersedes N98. | Each control's existing tests. | C |
| S6 | Confirmed: `publish_change` builds a `std::function` to call it immediately (`control.cpp:3303-3322`). | Fix: call directly outside initialisation. | Event order tests. | S |
| S2 | Confirmed: every display command is about 300 bytes with seven destructible members (`display_command.hpp:56-90`). | Fix later: tagged records in a byte stream with side arrays. Measure memory on the Gallery first. | Paint identity hashes. | C |
| S3 | Confirmed: the default theme is compiled from text on first use in every process (`gui_forms_theme.cpp:521-553`). | Fix later: compile the built-in sheets at build time. Read the recorded `build_time` first to size it. | Theme provenance and gate tests. | C |
| S7 | Confirmed: the run-time form loader sets each property by name (`form_loader.cpp:148-179`). | Fix later: carry the property table entry in the model. | Form loader tests. | S |
| S8 | Confirmed: three UI Automation text calls lay the text out and do not use the layout. | Fix with N109. | UIA text tests. | S |
| S9 | Confirmed: 439 executables linked against static libraries, no precompiled header or unity build in the build files. | Owner decision: shared libraries or per-area test executables for the test configuration; a precompiled header. | The same test list passes. | B |

## G. Performance, hosts and bindings

| Id | Status | Recommendation | Severity | Size |
|---|---|---|---|---|
| N63 | Confirmed: animation wakes ride `WM_TIMER` (`windows_host.cpp:2064-2071`). | Fix: use the live clock for near wakes. | performance | S |
| N67 | Confirmed: X presentation goes through the socket (`linux_host.cpp:334-391`). | Fix: MIT-SHM with the current path as fallback. | performance | C |
| N49 | Confirmed: one publisher thread per compatibility endpoint; N49a three passes per publish. | Fix later: one shared publisher. | performance | C |
| N72 | Confirmed: every C# ABI call locks and copies seven tables (`Abi.cs`). | Fix. | performance | S |
| N22, N13, N12 | See section B. | | | |

## What I did not read on main

Stated so that nothing above is taken as a clean bill for these:

- `src/core/theme_language/compiler/theme_compiler.cpp`: the rule
  lowering, variation and palette enumeration (560 of 3,774 lines) were
  read, the rest outlined. The material and motion lowering (4,400
  lines), `familiar_themes.cpp`, `form_model.cpp` and the form generator,
  loader and editor: searched for limits and recursion only. They run at
  load, not per frame.
- Most control sources line by line. I searched all the item controls for
  whole-model work in their hot functions and read where the search hit.
  Of the composites, the ribbon's layout, parts and paint (560 lines) and
  the context menu's rows and open and close paths (330 lines) were read;
  page strip, navigation pane, combo box and the dialogs were only
  searched.
- `windows_automation.cpp` beyond its lifetime, threading and event code;
  the second half of `windows_text_automation.cpp`;
  `linux_accessibility.cpp` beyond its update and action paths. In
  `macos_host.mm`: the fixture diagnostics, the window and termination
  delegates and the retained draw (`drawRetainedRect:`, about 500 lines)
  beyond its presentation tail. `skia_raster.cpp` gradients, groups and
  image drawing (about 700 lines).
- `CMakeLists.txt` beyond its flags; the 376 test files (only what they
  cover for N71 and which of them sleep); `tools/` beyond the profiler's
  header; `docs/` beyond the host capability page and the text engine
  page, which I checked against N57, N61, N66 and N69.

By lines, somewhat over half of `src/` and `include/` was read in full or
in the parts that run per frame, per keystroke or per item; the rest was
searched for specific patterns only. Tests, tools and docs were sampled.
The unread remainder is load-time code, declarative tables, test code and
generated documentation; I do not expect it to change the batches below,
but I have not read it and cannot say it is clean.

## Options for the owner

a. **Seal batch** (all S or C, all in `src/abi/stable` and the C# sibling):
   N20, N34, N71, N14, N27, N10, N24, N17, N21, N25, N26, N29, N9 under
   ThreadSanitizer, N5, and the documentation items K4, K5, K8 and N16. I recommend this first: it is the only group that
   cannot be corrected after consumers bind.
b. **Hang and loss batch:** N48, N2, N42, N45, N43, N70, N52, N113.
c. **Parity batch:** N61 (clear the capability bit, then the candidate
   window), N62 (horizontal wheel first), N66, N68, N75; and one manual
   test of a Japanese input source on macOS to settle N105 and N106.
d. **Tree-cost batch** (cheap, independent, each S): N77, N78, N79, N80,
   N81, N82, N86, N36, N46, N47, N95, N97, N98, N100, N103. Then N88, N85, N83, N84, N87 as a
   second step, measured on a window of several thousand controls.
e. **Per-frame cost, same pixels:** S1 first (the pixel compositor); then
   N56 with S4 (measure, then keep shaped runs and line metrics); then
   N90 with N1 (resolve a part's recipe once); then S5 (keep layouts);
   then N50 and N51 (text edits). Each has an existing test that proves
   the output did not change.
f. **Finish the read** of the files listed above.

Recommended order: a, then N48 and N61 from b and c. For speed alone:
S1, then the N56 measurement with S4, then N90, then d's first step.
