# Triage of the 2026-11-09 codebase audit

The audit (`CODEBASE_AUDIT_2026-11-09.md` beside this file) read an old
checkout: `claude/dk-docking-ministage` at bd4b60bf (2026-10-14), 1972
commits behind main. Each finding below was checked again, read-only, against
origin/main at ae189ff4b (#477, after cp26), in a detached worktree. Line
numbers in the audit no longer match; the status column cites main.

**The owner decides which findings we act on.** Every verdict is a
recommendation, and the batches at the end are options, not assignments.
Nothing has been routed to a worker.

Columns:

- **Status on main:** still true, fixed (with commit where found), changed
  (partly addressed or reshaped), or not applicable.
- **Recommendation:** fix, fix later (before 1.0, or with the stage that
  owns it) or disagree, with the reason.
- **Severity:** security, correctness, performance, design or docs.
- **Size:** S, C or B on the effort scale.

## Security: G2, with the owner's caveat

**G2 is still true on main.** `src/abi/registry/registry.hpp:1778` sets
`options.automation_enabled = true` for every non-headless window opened
through the C ABI on Windows. The demo mains set it only from `--automation`.
`windows_host.cpp:2778` accepts `WM_COPYDATA` messages prefixed
`GUI.Forms.Automation/1 `. The commands are:

- snapshot (JSON to stdout);
- `capture [path]` (writes a PNG to a path the sender chooses);
- close, move, focus, text, key, activate and click-at.

Any process on the same desktop can therefore drive and capture every
C ABI consumer, and can make it write a file wherever its user can write.

Caveat: the owner is not sure this is a security problem. A minimal change
that makes automation default-off is already in progress. The owner's
direction is to move automation towards an MCP integration later. The
coordinator has had this since the first check.

Recommendation: let the default-off change land. It should put automation
behind an explicit run option, as the demos do. Leave the protocol's future
to the MCP direction. Until then, two points belong in the MCP design:

- `capture` should stop accepting an arbitrary path;
- the parser should get a fuzz target (I3).

No other finding is a security item.

## Findings

### A. Control core

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| A1 | Changed. Static `property_table()` spans (control.hpp:753) replaced per-instance records for stock properties; `BindablePropertyMap` and `PropertyRegistration` remain only for foreign or registered properties. | Disagree: the sealed property-capability design already answers this, and the map exists for foreign objects. | performance | core/control | — |
| A2 | Still true: control.cpp:1790, 1835, 1865 copy the whole `PropertyRegistration`. | Fix: borrow by const reference (§5, keep borrows valid through use). | performance | core/control | S |
| A4 | Still true: event.hpp:78 allocates `State` in every `Event` constructor. | Fix: allocate on first connect; most events never get a handler. | performance | core/event | S |
| A5 | Still true: `compact` runs on every connect and after the outermost emit (event.hpp:304, 321). | Fix: compact only when a disconnect during emission left a hole. | performance | core/event | S |
| A6 | Still true: `on_dispose` (control.cpp:4202) disconnects 3 of 14 events; window.cpp:1812 publishes `pointer_observed_` after `on_pointer` without checking that the control is still alive. | Fix: disconnect every event on dispose and check liveness after the callback (§6, §8: stop revoked callbacks). | correctness | core/control, core/window | S |
| A7 | Still true: 18 `parent_.lock()` uses on hot paths. | Fix later: an observing raw back link has to be proven safe under the lifecycle contract (ADR-014) first. | performance | core/control | C |
| A8 | Still true: `DockStyle::fill` takes the rest of the rectangle and leaves zero for later siblings. | Disagree: this matches documented Windows Forms docking order (the compatibility boundary is documented observable behaviour). | design | core/control | — |
| A9 | Still true: control.cpp:2056 `add_child` detaches from the old parent before `attach_subtree`; when the attach fails the child ends up orphaned. | Fix: validate before mutating (§9); on failure leave the child where it was. | correctness | core/control | S |
| A10 | Still true: `recompute_subtree_dirty` (control.cpp:2146; window.cpp:2672, 2752, 3846, 3872) and `update_display_cache_metrics` (window.cpp:2705, 2753, 3357) rescan on every removal. | Fix later: incremental subtree-dirty counts belong with the paint whole-tree batch. | performance | core/window | C |
| A11 | Still true: `children()` returns painter order; `set_child_index` takes Windows Forms order. | Fix: name the two orders in the API (§1, separate confusable quantities). This is a rename, not a behaviour change. | design | core/control | S |
| A12 | Still true: `set_child_index` (control.cpp:2152) has no lifecycle guard and invalidates even when the index does not change. | Fix: add the guard, and return early when the index is unchanged. | correctness | core/control | S |
| A13 | Still true: `visual_context` takes five bools (control.hpp:517). | Fix: a named record (§1). | design | core/control | S |
| A14 | Still true: the comment at control.hpp:1461 contradicts `std::mutex dispatcher_mutex_`. | Fix: correct the comment (§11). | docs | core/control | S |
| A15 | Still true: behaviour at control.cpp:3288 depends on `#ifndef NDEBUG`. | Fix: release and debug must fail the same way (§9); keep only diagnostics conditional. | correctness | core/control | S |
| A16 | Still true: `data_bindings() const` uses `const_cast` (control.cpp:1923). | Fix: make the lazy member `mutable` with a stated rule, or create it eagerly. | design | core/control | S |
| A17 | Changed: `selectable`, `standard_click` and `standard_double_click` are now read (window.cpp:1173, 1891-1893; button_base.cpp:223). Several flags are still never read. | Fix later: before 1.0, wire or remove each unread `ControlStyles` flag, recording divergences. | design | core/control | S |
| A18 | Still true: `Dirty::text`, `style` and `resource` are never read or tested. `accessibility` appears in the sealed event catalogue (`abi_event_catalog.hpp:331`). | Fix later: wire or remove the three internal kinds; keep `accessibility`, because the ABI is sealed and its change needs an owner decision. | design | core | C |
| A19 | Still true: a private lenient UTF-8 decoder (control.cpp:206) and literal pi (:50). | Fix: use the shared strict decoder and the named constant. | correctness | core/control | S |
| A20 | Still true: nine sites in control.cpp and window.cpp hand-roll `++x; if (x == 0) ++x;`. | Fix: one named helper for the non-zero revision. | design | core | S |

### B. Window and layout

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| B1 | Fixed in 674ac833a ("L1: the measure and arrange contract"): `measure_cache_` (capacity 4) with hit and miss metrics. | — | performance | core/window | — |
| B2 | Fixed in 674ac833a: `mark_layout_ancestors` (window.cpp:4134) stops at a layout boundary and records `layout_boundary_stop`. | — | performance | core/window | — |
| B3 | Changed: fixed in `mark_dirty` by 304aa9f05 (window.cpp:44 reads the flag once). Remaining: `getenv` in `drawing_skia_c_api.cpp:99`, and in `drawing_platform_windows.cpp:252` on a failure path. | Fix: read each flag once, at the same seam. | performance | render/drawing | S |
| B4 | Still true: every traversal copies the child vector (window.cpp:3919, 3961, 4088, 4548). | Fix: iterate by index over a stable snapshot only where callbacks can mutate (§3). | performance | core/window | C |
| B5 | Still true: an empty damage rectangle falls back to `requested_bounds_` (window.cpp:3274, 3700). | Fix: treat empty damage as no damage, and test it. | correctness | core/window | S |
| B6 | Still true: `contains_control` walks down from the root (window.cpp:64; called at 644, 961, 980, 1055, 1258). | Fix: walk up the parent chain. | performance | core/window | S |
| B7 | Still true: a pointer move runs several full hit tests (window.cpp:1678, 1741, 1876). | Fix: one hit test per event, shared by its consumers. | performance | core/window | C |
| B8 | Still true: `find` builds a `std::string` per lookup. | Fix: heterogeneous lookup on `string_view`. | performance | core/window | S |
| B9 | Still true: `mark_dirty` still runs several passes, and control.cpp:51-52 still calls sin and cos. | Fix later: with B4 and C4. | performance | core | S |
| B10 | Worse: `surface_root.hpp` has 48 friends, window.hpp 16; six `struct Reset` definitions. | Fix later: split `SurfaceRoot` along its sub-authorities (§6, §11); a refactor sized B, not urgent. | design | core/window | B |
| B11 | Still true: `hit_test` returns `{}` when it does not converge, and borrows `maximum_semantic_snapshot_passes`. | Fix: give hit testing its own explicit result and bound (§9). | correctness | core/window | S |

### C. Paint and presentation

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| C1 | Still true: `collect_paint_checkpoints` (window_presentation.cpp:203) walks the whole tree on every paint. | Fix: collect from the dirty subtree only. | performance | core/presentation | C |
| C2 | Still true as far as read: commands are copied chunk to candidate to backend. Not re-read in depth. | Fix later: references into chunks need a lifetime design first (§5). | performance | core/display | C |
| C3 | Still true: one large `DisplayCommand` record for every operation. | Fix later: a compact encoding is a display-list redesign; measure first. | performance | core/display | B |
| C4 | Still true: `absolute_bounds_of` per node in paint (window.cpp:4467) and hit test (3905). | Fix: pass the parent origin down. | performance | core/window | C |
| C5 | Unverified: the structure is unchanged. | Fix: write the test the audit names (invalidate a scrolled-out label, paint, assert the lease is clean); fix whatever it shows. | correctness | core/presentation | S |
| C6 | Still true: a reentrant paint request damages the whole window (window_presentation.cpp:530-533). | Fix: keep the requested rectangles. | performance | core/presentation | S |
| C7 | Still true: the comment (window_presentation.cpp:969) says uncovered pixels keep advancing, while :999 suspends every live surface while any overlay is open, tooltips included. | Fix: suspend only covered surfaces, or correct the comment; the owner's live-surface intent decides which. | correctness | core/presentation | S |
| C8 | Still true: `collect_visible_overlay_rectangles` runs on every live tick. | Fix later: cache per overlay revision. | performance | core/presentation | S |
| C9 | Still true: hover invalidates the whole control (list_box.cpp:400-401, checked_list_box.cpp:1063). | Fix: invalidate the old and new rows. | performance | controls | S |
| C10 | Still true: window_scheduler.cpp:134 uses an `unordered_set`, a vector copy and a `std::function` copy per tick. | Fix later: small, measured first. | performance | core/scheduler | S |

### D. Text and rendering

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| D1 | Still true: `hb_shape_full` on every shape (harfbuzz_font_engine.cpp:554); `FT_Load_Glyph`/`FT_Render_Glyph` on every run (glyph_run_raster.cpp:207, 237). The text-mask budget is not a glyph cache. | Fix: a shaping cache keyed by text, face, size and features, and a glyph cache keyed by face, glyph, size and subpixel phase, both bounded. Our engine stays the reference, so the cached output must be bit-identical. | performance | render/text | B |
| D2 | Still true: `ResolvedTextLayout` (typography_types.hpp:64) carries per-call copies. | Fix later: with D1. | performance | render/text | C |
| D3 | Changed: skia_raster.cpp:327 uses `MakeUninitialized`; `skia_executor` still uses `SkData::MakeWithCopy`. | Fix: the remaining copy. | performance | render/skia | S |
| D4 | Changed: unchanged images are now skipped by content hash (skia_raster.cpp:908), but each registry revision still walks every image, and a patch re-copies the bitmap. | Fix later: a revision and dirty rectangle per image. | performance | render/skia | C |
| D5 | Fixed: one CPU renderer (stx_skia) on every host; the DIB painter, the CoreGraphics adapter and WIC are gone, and PNG decodes through the shared owned path (14a84171a). `dib_frame_store` remains as the Windows present store only. | — | design | render | — |
| D6 | Still true, and documented: nested rounded clips approximate text clipping (skia_raster.cpp:1097). | Fix later: T2 requires identical pixels, so this needs a rule before T2 closes. | correctness | render/skia | C |
| D7 | Fixed: the macOS host tooltip (NSFont, Lucida Grande) is replaced by the engine-drawn tooltip splash. | — | design | host/macos | — |
| D8 | Still true: `FontSpec` (paint_types.hpp:99) has a role and no face identity. | Fix later, with an owner decision: Stage F and the Windows Forms `Font` need a face; decide the shape before 1.0. | design | types/text | C |
| D9 | Fixed: the image registry uses `stx_zlib::crc32`. A test-local copy remains in `skia_raster_smoke.cpp:32`. | Fix later: point the smoke test at stx_zlib. | design | render/skia | S |

### E. Threads

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| E1 | Still true: `try_acquire_write(preserve)` copies the published pixels under the state mutex (live_surface.cpp:94-97). | Fix: reserve the slot under the lock, copy outside. | performance | core/live_surface | S |
| E2 | Fixed: the wake now runs under the lock, so a host removing its handler waits for one already under way (post_dispatch.cpp:73-80). | — | correctness | core/dispatcher | — |
| E3 | Still true: a `fault_mutex` per `DispatchWork` (dispatcher_state.hpp:24). | Fix later: publish the fault through the operation state. | performance | core/dispatcher | S |
| E4 | Still true: the Linux host samples live surfaces at least every 16 ms while one is registered (linux_host.cpp:1650-1652), and the Windows host's waitable timer does the same; only macOS uses the idle handshake. | Fix: adopt the idle handshake on Windows and Linux; "idle schedules zero frames" is a design-language rule. | performance | host/windows, host/linux | C |
| E5 | Not applicable: the audit found no defect. | — | — | core/threading | — |
| E6 | Still true: `own_revocable` purges expired entries on every add (component.cpp:53). | Fix later: purge on a threshold. | performance | core/component | S |

### F. Controls

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| F1 | Still true: TextBox, ListBox, ComboBox, TreeView, NumericUpDown, DateTimePicker and PropertyGrid derive from `Panel`. | Fix later, with an owner decision: re-base them on Control-level bases (TextBoxBase, ListControl, UpDownBase) before 1.0. This is inheritance for reuse (§6), and it shapes the schema and stages W and S. | design | controls | B |
| F2 | Still true: include paths encode the base-class chain (`controls/panel/text_box/text_box.hpp`). | Fix later, with F1: flat `controls/<name>.hpp` paths before 1.0 (compatibility policy: right by 1.0). | design | include/controls | C |
| F3 | Still true: `semantic_virtual_children` (list_box.cpp:483) builds a node for every item. | Fix: realise the visible rows, as TreeView does. | performance | controls | S |
| F4 | Still true: `std::stod`/`std::stoll` at numeric_up_down.cpp:161, 164, 259; scroll_bar.cpp:456; track_bar.cpp:413; split_container.cpp:527; splitter.cpp:262; scrolling_utilities.hpp:22. They follow `LC_NUMERIC` and throw. | Fix: `std::from_chars` with explicit results (§9). | correctness | controls | S |
| F5 | Still true: `Panel::style()` and the `themed` branches remain (14 files). | Fix later: the theme engine (T4) removes one system. | design | controls/themes | C |
| F6 | Still true: repeated inset literals in label.cpp (11) and list_box. | Fix: named constants. | design | controls | S |
| F7 | Changed: `parts()` (label.cpp:441) reads the published anatomy rather than laying out. | — | performance | controls | — |
| F8 | Still true: undo keeps whole-text snapshots (`text_editing_engine.hpp:139`), and `TextStore` re-analyses the whole text. | Fix later: edit deltas and incremental analysis when multiline documents become a goal. | performance | controls/text | C |
| F9 | Still true: CorrespondenceView, ReviewCard, InstrumentRack, RackModulePanel, EasingPreview, TypographySpecimen and VisualInspectorView compile into the installed controls library. | Fix later: move lab and provisional controls out of the installed library before 1.0. | design | controls/build | C |
| F10 | Still true: `detail/algorithm/sort.hpp` in installed headers, three call sites in data_grid_view. | Fix later: use `std::stable_sort` (§7), unless the private sort exists for the freestanding path; record the reason either way. | design | include/detail | S |

### G. Hosts

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| G1 | Changed: windows_host.cpp is down to 3,541 lines (painter and dialogs moved out); macos_host.mm grew to 4,562; registry.hpp is 3,227 lines of implementation in a header. | Fix later: split macos_host.mm and move registry.hpp's implementation into sources (§11). | design | hosts, abi | B |
| G2 | Still true. See the security section. | Owner's call: default-off change in progress; MCP direction later. | security (owner unsure) | abi, host/windows | S |
| G3 | Fixed: the window classes no longer use `CS_HREDRAW`/`CS_VREDRAW` (windows_host.cpp:3273, 3421). | — | performance | host/windows | — |
| G4 | Still true: `XCreateImage`, a conversion copy and `XPutImage` per present (linux_host.cpp:355-381). | Fix later: MIT-SHM with a fallback; skip the conversion for a BGRX visual. | performance | host/linux | C |

### H. C ABI

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| H1 | Changed: the sealed ABI (`gui_forms/c/gf_abi.h`) uses `struct_size` and one exported table. The 26 version macros and size-less structs are in the experimental 0.x `c_api.h`. | Fix later: retire `c_api.h` before 1.0, or mark it experimental in the docs. | design | abi | S |
| H2 | Changed: the sealed ABI exposes `gf_scroll_snapshot` (gf_abi.h:1774). The mixed `gf_scroll_state` is only in the experimental header. | Disagree for the sealed ABI; it lapses with H1. | design | abi | — |
| H3 | Still true: one process-wide registry (registry.hpp:3222). | Fix later, with an owner decision: state whether two consumers in one process share it. The 1.0 gate's "two independent consumers" can be met by two processes; a per-consumer context would be an additive ABI change. | design | abi | C |

### I. Build, CI, tools

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| I1 | Still true: CMakeLists.txt is 5,522 lines with three functions; `dib_frame_store.cpp` is in four targets. | Fix later: helper functions and one object library for shared sources; this belongs with the remote split. | design | build | C |
| I2 | Fixed: `ci_build.py` and the release workflow pass `ctest --timeout`, and the jobs have their own limits. | — | design | ci | — |
| I3 | Changed: seven fuzz targets now (PNG, deflate, inflate, JSON, XML, Markdown, theme sheet). The clipboard decoders, the automation parser and stx_vorbis are still unfuzzed. | Fix later: add the clipboard decoders and stx_vorbis; leave the automation parser to its MCP successor. | correctness | fuzz | C |
| I4 | Changed: the harness checker now covers `.inc`. `.c` is still excluded; `atomic_pool.c` is vendored, with a LICENSE and provenance. | Disagree: vendored C is exempt; say so in the checker. | docs | tools | S |

### J. Documents

| Id | Status on main | Recommendation | Severity | Area | Size |
|---|---|---|---|---|---|
| J1 | Changed: the roadmap plans CSS-syntax theme sheets (docs/dev/roadmap.md:56, 340), and "Not planned" (:379) still says "a browser engine, CSS or an HTML runtime". | Fix: say "CSS layout or an HTML runtime". | docs | docs | S |
| J2 | Still true: architecture.md:26 calls the Windows renderer a DIB compositor. | Fix: one stx_skia renderer on every host. | docs | docs | S |
| J3 | Still true: AGENTS.md keeps "Scope of the first proving slice", "Agent ownership during the initial parallel build" and the Lucida Grande paragraph; PROVING_SLICE.md moved to the notes repository. | Fix: retire those sections, or mark them historical (owner's file). | docs | AGENTS.md | S |
| J4 | Changed: ADRs are consolidated in notes `decisions/`; numbers 005 and 007-012 are still missing. | Fix: record the missing numbers as never issued or withdrawn. | docs | notes | S |
| J5 | Fixed: FORWARD_PROGRAMME §3A (now in notes) records accessibility verified on all three hosts. | — | docs | notes | — |
| J6 | Still true: the stale comments at C7 and A14. | Fix: with C7 and A14. | docs | core | S |

## Counts

82 findings (the audit's ids run to about 90 with gaps; A3 does not exist).

| Status on main | Count |
|---|---|
| Still true (one of them worse: B10) | 58 |
| Changed (partly addressed or reshaped) | 13 |
| Fixed | 9 |
| Unverified | 1 (C5) |
| Not applicable | 1 (E5) |

| Recommendation | Count |
|---|---|
| Fix | 38 |
| Fix later | 29 |
| Disagree | 4 (A1, A8, H2, I4) |
| Nothing to do (fixed, F7, E5) | 10 |
| Owner's call | 1 (G2) |

One security item (G2), which the owner doubts is one; about 18
correctness items, about 40 performance items, the rest design and docs.

## Batches (options for the owner)

Each option is self-contained. Each pairs with its tests, and none needs an
ABI change. Options 1 to 4 are the recommended first choices, in order.

1. **Event, tree and lifecycle defects** (correctness, about C).
   A6, A9, A12, A15, A16, B5, B11, C5, C7 with J6 and A14.
   These are real defects in disposal, reparenting, child order, damage and
   hit testing, each with a small test.
2. **Locale-independent parsing and strict UTF-8** (correctness, S).
   F4 and A19: `std::from_chars` at the eight sites, and the shared strict
   decoder.
3. **Shaping and glyph caches** (performance, B).
   D1 and D2: bounded shaping and glyph caches with bit-identical output, and
   a measured workload before and after. This is the largest single
   performance item and matters for T2.
4. **Paint whole-tree work** (performance, C to B).
   B4, C1, C4, C6, C9, A10, B9. Paint and hit test stop walking the whole
   tree, the parent origin is passed down, and invalidation covers rows
   rather than whole controls.
5. **Pointer and dirty bookkeeping** (performance, C).
   B6, B7, B8, A2, A4, A5, A20, E6. One hit test per pointer event, an
   upward containment walk, cheaper events, one revision helper.
6. **Host wake and idle** (performance, C).
   E4, with E1 and C8: the idle handshake on Windows and Linux, so that an
   idle window schedules zero frames. G4 (MIT-SHM) can ride along on Linux.
7. **Small cleanups** (design, S each).
   A11, A13, B3, D3, D9, F3, F6, F10, A17: named records and constants, the
   remaining `getenv` and copies, ListBox visible-row semantics.
8. **Docs** (docs, S).
   J1, J2, J3, J4 and the I4 exemption note. These could ship with the next
   checkpoint's docs update.

Larger items that need an owner decision before anyone sizes them:

- **Leaf control inheritance and include layout** (F1, F2, F9; B).
  Re-base the leaf controls off `Panel`, flatten the include paths, and move
  lab controls out of the installed library, all before 1.0.
- **Font face identity** (D8; C). `FontSpec` gains a face before 1.0.
- **Registry scope** (H3; C), and retiring the experimental `c_api.h` (H1).
- **Unused dirty kinds** (A18; C). `accessibility` is in the sealed event
  catalogue.
- **Large-file splits** (B10, G1; B).
- **Display-list encoding** (C2, C3; B), after a measurement.
- **Text editing at document scale** (F8; C), when multiline documents become
  a goal.
- **Build structure and fuzzing** (I1, I3; C).
