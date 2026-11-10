# GUI.Forms codebase audit, 2026-11-09

Auditor's notes in shorthand. Branch `claude/dk-docking-ministage` at `bd4b60bf`.
Nothing was built or run except `tools/check_house_style.py` (965 files, 0
findings). Every item is from reading the source.

Tags: **bug** (defect as written), **verify** (suspected defect, needs a test),
**perf**, **design** (works, but I think the approach is wrong), **docs**.

## Coverage

- Read in full: `control.hpp`, `control.cpp`, `window.hpp` (members),
  `window.cpp`, `window_presentation.cpp`, `window_scheduler.cpp`, dispatcher
  (state, post, operation, window, control), `event.hpp`, `component.*`,
  live surface (surface, lease, state, wake), `atomic_pool.c`, `worker.cpp`,
  display list (command, chunk, replay), `damage_region.cpp`, `dirty.hpp`,
  `painter.hpp`, `label.cpp`, clipboard DIB and wire decoders, the fuzz target,
  the house style, forward programme, architecture and roadmap pages.
- Sampled: `skia_raster.cpp` (image sync, clips, text), `glyph_run_raster.cpp`,
  `windows_host.cpp` (outline, paint, run loop, automation), `linux_host.cpp`
  (present, pump), list box, tree view, flow layout, text store API, theme
  resolve, C ABI header and registry entry, CMake, CI workflow outline.
- Not reviewed: body of `macos_host.mm`, `stx_vorbis`, binding, property grid,
  object view, most other controls, C# tools, demoboard, test bodies,
  generated docs.

## Ten things I would change first

1. Layout never caches a measurement (B1).
2. No shaping cache and no glyph cache below the per-command memo (D1).
3. Every control builds its own property registry (A1).
4. Three geometry rasterisers and two PNG decoders (D5).
5. Paint does whole-tree work per frame regardless of damage (C1, C2, C4, A10).
6. Leaf controls inherit from Panel and ScrollableControl (F1).
7. Automation channel is always on for C ABI consumers (G2).
8. Windows and Linux tick at 16 ms while a live surface is merely registered (E4).
9. `std::getenv` on the layout invalidation path (B3).
10. `contains_control` walks down instead of up (B6).

---

## A. Control and the retained tree

- **A1 perf/design** `src/core/control/control/control.cpp:294-537`,
  `include/gui_forms/control/control/control.hpp:874` — each Control
  constructor inserts about 20 `std::map<std::string, BindableProperty>`
  nodes, each with descriptor strings, a default `BindingValue` and two or
  three `std::function`s. Label adds four (`label.cpp:40-111`); Panel and
  ScrollableControl add more. Contradicts "the anatomy describes the type,
  never the instance" and "one registry". Use one static table per type:
  `{descriptor, get(const Control&), set(Control&, const BindingValue&)}`; the
  instance holds nothing. This also removes the `RegisteredPropertyGetter/
  Setter/Connector` `if constexpr` chains (`control.hpp:965-1172`) from the
  public header.
- **A2 perf** `control.cpp:923, 959, 985, 1014` — `set_property_value`,
  `reset_property`, `should_serialize_property` and `property_value_origin`
  each copy the whole `PropertyRegistration` (strings, standard-values vector,
  five callables). Copy only the callable used.
- **A4 perf** `include/gui_forms/event/event/event.hpp:78` — every `Event`
  allocates its shared `State` in the constructor. Control has 13 events
  (`control.hpp:940-952`), so 13 heap allocations before any subscriber.
  Allocate on first subscribe; emit on a null state is a no-op.
- **A5 perf** `event.hpp:304, 321` — `compact()` runs `remove_if` over all
  slots on every connect and after every outermost emit, even when nothing
  disconnected. n subscribes cost O(n²). Keep a pending-disconnect count.
- **A6 bug** `control.cpp:2757-2759` — `on_dispose` disconnects 3 of 13
  events. The other ten keep subscribers' owning callables alive until the
  last `Ptr` drops and can still emit: `window.cpp:1234` publishes
  `pointer_observed_` after `on_pointer` with no `is_alive` check.
- **A7 perf** `control.cpp:574-581, 1850-1861, 1863-1891, 2714-2723`,
  `window.cpp:2847-2855` — every "effective" query walks ancestors through
  `weak_ptr::lock()` (atomic increment and decrement per level).
  `eligible_for_input` is three walks. Keep a raw observing parent pointer
  maintained by add/remove (the parent owns the child, so it is valid while
  linked); keep the `weak_ptr` only for `parent()` returning `Ptr`.
- **A8 design** `control.cpp:2228-2230` — `DockStyle::fill` zeroes the
  remaining rectangle, so docked siblings processed after it get a zero slot
  and vanish. Windows Forms leaves the remainder alone (later siblings
  overlap the fill control). Match it or record the divergence in
  `WINFORMS_BEHAVIOR_GAP_CATALOGUE.md`.
- **A9 bug** `control.cpp:1188-1208` — reparenting removes the child from its
  previous parent before `attach_subtree` can fail (duplicate stable ID,
  throwing `on_attached_to_window`). On failure the child is orphaned, not
  returned. Validate the stable-ID set first (house style §9).
- **A10 perf** `control.cpp:1245` — `remove_child` recomputes subtree dirt for
  the whole window tree; `clear_children` (`:1290`) is then
  O(children × tree). Same whole-tree pass at `window.cpp:1768` (every
  attach), `:1847` (dispose), `:2399, 2405` (every layout pass), and
  `update_display_cache_metrics` at `:1799, 1848` and
  `window_presentation.cpp:404` (every paint). Subtree dirt is already OR-ed
  upward when marked; clear it on the way back up and keep the cache-entry
  count as a counter.
- **A11 design** `control.hpp:371` vs `:375-378` — `children()` is painter
  order (index 0 is backmost); `child_index`/`set_child_index` use Windows
  Forms order (0 is topmost). Two orders in one API.
- **A12** `control.cpp:1251` — `set_child_index` lacks the
  lifecycle-notification guard that add/remove have, and invalidates when the
  position did not change.
- **A13 style** `control.hpp:463-465` — `visual_context(bool, bool, bool,
  bool, bool)`. Five positional flags (§1). Take a record.
- **A14 docs** `control.hpp:886-888` — comment says atomic `shared_ptr` free
  functions; the code is a `std::mutex` per control.
- **A15 design** `control.cpp:2036-2050` — `invalidate_declared(none)` throws
  in debug (after the state is already mutated) and silently goes
  conservative in release. Behaviour should not depend on `NDEBUG`.
- **A16** `control.cpp:1033-1040` — `data_bindings() const` does a
  `const_cast`, lazy creation and a UI-thread check.
- **A17 design** `control.hpp:244-263` — 17 `ControlStyles` flags; only
  `selectable` is read (`window.cpp:861`). Inert compatibility state in the
  core; it belongs to the facade.
- **A18 design** `include/gui_forms/dirty.hpp:15-18` — `Dirty::text`,
  `style`, `resource` and `accessibility` are never tested anywhere, only
  OR-ed in. Consume them or delete them. `invalidation::bounds`,
  `visual_tree` and `visibility` are the same constant.
- **A19** `control.cpp:169-200` — a private lenient UTF-8 decoder (accepts
  overlongs and surrogates) beside the strict text module. `:32` literal pi.
- **A20** `++x; if (x == 0) ++x;` on 64-bit counters at 15 or more sites
  (`control.cpp:1533, 1559, 1954, 2011, 2025`; `window.cpp:1796, 1843, 2034,
  2090, 2186, 2220, 2231, 2626`). A 64-bit counter does not wrap. Delete, or
  one helper.

## B. Window: layout, input, structure

- **B1 perf/design** `window.cpp:2521-2524` — the measure pass calls
  `measure()` and discards the result. Containers measure children again in
  their own `measure` and again in `arrange` (`control.cpp:2129, 2180, 2245`;
  `flow_layout_panel.cpp:172-175`, reached from both `measure` at `:324` and
  arrange). Nested auto-size or flow containers of depth d measure a leaf up
  to 2^d times per pass. A leaf is `Label::measure`, which shapes each line
  twice (`label.cpp:338-343`) with no cache underneath. Store the desired
  size and the constraint it was computed for on the control; return it while
  `Dirty::measure` is clear and the constraint is unchanged.
- **B2 perf** `window.cpp:2015-2019` — any layout-dirty mutation marks every
  ancestor measure and arrange up to the root. `Label::set_text` avoids this
  by hand (`label.cpp:139-146`). Generalise: a control whose slot cannot
  change (fixed size under a non-auto-size parent, dock fill) is a layout
  boundary.
- **B3 perf** `window.cpp:2001` — `std::getenv` inside `mark_dirty` on every
  layout invalidation, and `fprintf` prose where the guardrails ask for
  structured counters. Read the flag once. Same pattern at
  `src/abi/drawing_skia_c_api.cpp:101, 235` and
  `src/abi/drawing_platform_windows.cpp:258`.
- **B4 perf** `window.cpp:2469, 2511, 2577, 2763` — every traversal copies
  each node's child vector (an allocation and two atomics per child) to
  survive mutation by callbacks. Paint, hit test and arrange are the hot
  loops. Forbid structural mutation during paint, as lifecycle notification
  already does, and iterate by index.
- **B5 bug** `window.cpp:2041, 2193` — when the damage bounds are empty the
  fallback uses `requested_bounds_`, which is parent-relative, as a window
  rectangle. Remove it; arrange adds the right damage.
- **B6 perf** `window.cpp:45-58` — `contains_control(root, candidate)`
  searches the whole subtree. `Control::contains` (`control.cpp:1610`) walks
  ancestors. Called per candidate in mnemonic collection (`:649, 743`, so
  O(N²)), on every focus request under a scope (`:1128`), five times in
  `revoke_interaction_for_subtree` (`:1867-1887`), and per popup (`:428`).
- **B7 perf** `window.cpp:1223, 1252, 1328` — a pointer move runs two full
  hit tests, a release three; each builds a popup-root vector (`:460`). Reuse
  the first result while `semantic_generation_` is unchanged.
- **B8** `window.cpp:448` — `find` builds a `std::string` per lookup. Use a
  transparent hash.
- **B9 perf** `window.cpp:1990-2048` — `mark_dirty` runs two or three
  separate ancestor loops plus `paint_damage_bounds_of` (two more walks and a
  virtual outset computation with `sin`/`cos` per decoration,
  `control.cpp:2351`).
- **B10 design** `window.hpp:728-886` — about 150 members and 12 friends.
  `paint_recursive` takes six counter out-parameters (`window.hpp:676-685`).
  The same `struct Reset` is defined at `window.cpp:806, 837, 911, 2099` and
  `window_scheduler.cpp:123`.
- **B11** `window.cpp:454-476` — `hit_test` returns null silently when it does
  not converge, and borrows `maximum_semantic_snapshot_passes` as its bound.

## C. Painting

- **C1 perf** `window_presentation.cpp:272-276` — every paint snapshots
  `(Control::Ptr, chunk)` for every control in the tree. Record a checkpoint
  only where a chunk is rebuilt (`window.cpp:2720`).
- **C2 perf** Commands are copied twice per frame: chunk into the candidate
  recorder (`window.cpp:2755`, copying text strings and gradient vectors),
  then candidate into the backend (`window_presentation.cpp:363`). Let the
  candidate hold `(chunk, translate, clip)` references.
- **C3 perf** `src/core/display/command/display_command.hpp:48-70` — one fat
  record (two points, rect, font, three doubles, a vector, a string, two or
  three `shared_ptr`s) for every operation including save and restore. Use a
  tagged compact encoding with side tables.
- **C4 perf** `window.cpp:2691, 2697` and `:2451-2455` — paint and hit test
  compute absolute bounds by walking to the root at every node, twice in
  paint. Both are O(N·depth). Pass the parent origin down.
- **C5 verify** `window.cpp:2714-2721` — `Dirty::paint` is cleared only when
  the control intersects the painted damage. A control invalidated while
  scrolled out of an ancestor viewport is never reached, so its bit stays,
  the root keeps paint in `subtree_dirty_`, `paint_dirty_`
  (`window_presentation.cpp:407`) stays true and the lease never reports
  clean. Hosts key on `take_damage`, so nothing spins; the idle accounting is
  what goes wrong. Test: invalidate a label scrolled out of view, paint,
  assert the lease is clean.
- **C6** `window_presentation.cpp:238-247` — a reentrant paint request damages
  the whole window. Keep the requested rectangles.
- **C7 verify** `window_presentation.cpp:659` — `!popups_.empty()` suspends
  the live lane for every live surface while any popup is open, tooltips
  included. The comment at `:626-630` says uncovered pixels keep advancing
  under a menu. One of the two is stale.
- **C8 perf** `window_presentation.cpp:631-638` — every live tick walks the
  whole tree for overlay rectangles, with an ancestor walk per node. Cache by
  layout generation.
- **C9 perf** `list_box.cpp:347`, `object_view.cpp:1048` — a hover change
  invalidates the whole control. The chunk is re-recorded, the per-command
  text memos die with it, and every visible row is shaped and rasterised
  again (see D1).
- **C10** `window_scheduler.cpp:132-135, 148` — three hash sets and a vector
  copy per poll; one `std::function` copy per timer tick.

## D. Text and rasterisation

- **D1 perf** No shaping cache: `harfbuzz_font_engine.cpp` runs bidi and
  `hb_shape_full` on every call, and `skia_raster.cpp:1139-1148` measures
  through it uncached. No glyph cache: `glyph_run_raster.cpp:150-173` calls
  `FT_Load_Glyph` and `FT_Render_Glyph` for every glyph of every run. The only
  memo belongs to the display command (`skia_raster.cpp:1009-1035`) and is
  lost whenever its chunk is re-recorded. Add a renderer-level LRU keyed on
  (text hash, FontSpec, scale, phase) holding the shaped run and coverage. It
  keeps pixels identical and survives re-recording.
- **D2 perf** `typography_types.hpp:44-52` — `ResolvedTextLayout` carries
  strings and a run vector; `label.cpp:22-33` builds one per wrap probe to
  read a width. Add a plain metrics query.
- **D3 perf** `skia_raster.cpp:538-568` — each image is held twice (registry
  bytes and `SkData::MakeWithCopy`); PNGs decode into a temporary vector and
  are copied again. Decode into `SkData::MakeUninitialized`; share BGRA
  storage.
- **D4 perf** `skia_raster.cpp:481-590` — any registry revision rescans every
  image, and a patch (`window.cpp:274`) re-copies the whole patched bitmap.
  Track a revision and dirty rectangle per image.
- **D5 design** Three geometry rasterisers: Skia (`src/render/skia`, macOS and
  Linux), a hand-written `DibPainter` (`windows_host.cpp` ≈640-2250: spans,
  gradients, shadows, rounded rectangles), and a CoreGraphics adapter
  (`src/render/coregraphics`, 881 lines, host font "Lucida Grande" at
  `coregraphics_raster.cpp:61-65`). Windows decodes PNG through WIC
  (`windows_host.cpp:2102`), Skia through `SkPngDecoder`; the guardrails admit
  one decoder. T2's gate is identical pixels per primitive on three hosts,
  and each new primitive must be written two or three times. Settle on one
  rasteriser before T2. The CoreGraphics adapter appears to serve only
  `layered_material_raster_tests`; delete it if so.
- **D6** `skia_raster.cpp:681-695` — nested rounded clips collapse to bounds
  for text (documented). Text can show in the corner of an inner rounded clip.
- **D7 design** `src/host/macos/application/macos_host.mm:1581` — macOS
  tooltips use `NSFont` "Lucida Grande" or the system font and `NSString`
  layout. Windows paints tooltips with the bundled engine
  (`windows_host.cpp:2828`). A host font on a GUI.Forms surface.
- **D8 design** `paint_types.hpp:76-91` — `FontSpec` has a role and no family
  or face. Stage F and the Windows Forms `Font` need a face identity, and
  `FontSpec` sits in `Painter`, the display list and themes. Decide before
  the 0.5 seal.
- **D9** CRC-32 is implemented twice, a byte at a time
  (`image_registry.cpp:51-72`, `png_writer.cpp:19-58`).

## E. Threads

- **E1 perf** `src/core/live_surface/surface/live_surface.cpp` ≈100-107 —
  `try_acquire_write(preserve)` copies the whole pixel vector while holding
  the state mutex; the UI thread's `snapshot()` waits behind it (33 MB at
  4K). Reserve the slot under the lock, copy outside.
- **E2** `src/core/dispatcher/state/post_dispatch.cpp:66` — the wake handler
  is copied and called outside the lock. A host removing its handler on the
  UI thread races a worker about to call the old copy. Guard it with a shared
  wake state, as `LiveSurfaceIdleWake` does.
- **E3** `dispatcher_state.hpp:23` — one `std::mutex` per posted work only to
  guard the fault pointer.
- **E4 design** Only macOS uses the live-surface idle handshake
  (`macos_host.mm:1245`). Windows (`windows_host.cpp:3037-3046`) and Linux
  (`linux_host.cpp:1271-1272`) wake every 16 ms for as long as a live surface
  is registered, publishing or not. That breaks "idle schedules zero frames".
- **E5** `atomic_pool.c` — read closely; no defect found.
  `gui_forms_atomic_pool_is_worker` reads `threads[]` unlocked, which is safe
  only after create returns. `Worker` (`worker.cpp:89`) builds a whole pool
  per thread.
- **E6** `component.cpp:52-56` — `own_revocable` purges expired entries on
  every add.

## F. Controls

- **F1 design** TextBox, ListBox, ComboBox, TreeView, NumericUpDown,
  DateTimePicker and PropertyGrid derive from Panel, which derives from
  ScrollableControl (`include/gui_forms/controls/panel/*`). They inherit
  scroll state, style overrides, registered properties and public setters
  they should not have. Windows Forms has TextBoxBase, ListControl and
  UpDownBase directly under Control. This is inheritance for reuse (§6) and
  the wrong shape for stages W and S and for the schema. Fix before the seal.
- **F2 design** Include paths encode the base-class chain:
  `gui_forms/controls/scrollable_control/container_control/responsive_track_panel/responsive_track_panel.hpp`.
  Re-parenting a class breaks every consumer's include, and the paths press
  on `MAX_PATH`. Use `gui_forms/controls/<name>.hpp`.
- **F3 perf** `list_box.cpp:425-457` — `semantic_virtual_children` builds a
  node for every item with three strings each. TreeView realises visible rows
  only (`tree_view.cpp:453`). The million-item fixture will stop here.
- **F4 bug** `std::stod`/`std::stoll` parse user text at
  `numeric_up_down.cpp:157, 160, 237`, `scroll_bar.cpp:441`,
  `track_bar.cpp:404`, `split_container.cpp:573`,
  `scrolling_utilities.hpp:32`. They follow `LC_NUMERIC`, which any embedding
  application can change, and they throw for ordinary bad input. Use
  `std::from_chars`.
- **F5 design** Two styling systems: theme recipes and `Panel::style()`
  (`panel.hpp:31`; the `themed` branches in `list_box.cpp:277-321`; 12
  files). T4 removes one; until then each control paints two ways.
- **F6** `label.cpp:339, 351, 354, 401-417` — insets 2.0, 4.0 and 1.0 repeated
  as literals across measure and layout; the list box does the same with 2.0,
  4.0 and 8.0. Name them once.
- **F7 perf** `label.cpp:476-489` — `parts()` runs a full text layout per
  call. The T2 engine will ask per paint.
- **F8 perf** `text_box.cpp:241-330` — undo stores the whole text per edit;
  `TextStore::replace` re-analyses the whole text per keystroke
  (`text_store.cpp:214-285, 501`) and keeps a `size_t` per grapheme. Fine for
  fields, not for multiline documents.
- **F9 design** `CMakeLists.txt:415-472` — CorrespondenceView, ReviewCard,
  InstrumentRack, RackModulePanel, EasingPreview, TypographySpecimen and
  VisualInspectorView compile into the installed Controls library. The
  manifest lists 5 lab and 7 provisional controls. Move them out before the
  ABI seal.
- **F10** `include/gui_forms/detail/algorithm/sort.hpp` — a 309-line private
  merge sort in installed headers with 2 call sites; 21 sites use the
  standard sorts (§7).

## G. Hosts

- **G1** `windows_host.cpp` is 4,301 lines: dialogs (≈124-360), the painter,
  host state, tooltip, automation JSON by `printf` (≈3541-3820), and two
  copies of the message loop (≈4032-4075, ≈4245-4270). `macos_host.mm` is
  3,429 lines; `src/abi/registry/registry.hpp` is a 3,233-line header holding
  an implementation (§11).
- **G2 bug** `src/abi/registry/registry.hpp:1744` — every window hosted
  through the C ABI on Windows sets `automation_enabled = true`. The
  `WM_COPYDATA` channel (click, focus, set value, capture to a caller-chosen
  path at `windows_host.cpp:3573-3578`, JSON to stdout) is live in every ABI
  consumer. Make it a run flag.
- **G3 perf** `windows_host.cpp:3981` — `CS_HREDRAW | CS_VREDRAW` invalidates
  the whole window on every resize. The host keeps a retained raster and adds
  its own resize damage.
- **G4 perf** `linux_host.cpp:215-264` — `XCreateImage` per present, a full
  conversion copy, then `XPutImage` over the socket. Use MIT-SHM with a
  fallback; when the visual is 32-bit BGRX, render in that order and skip the
  conversion.

## H. C ABI (input to stage C)

- **H1** `include/gui_forms/c_api.h:24-50` — 26 version macros. Public
  structs (`gf_scroll_state`, `gf_property_value`) carry no size field, so
  they cannot grow. Size-prefix every struct in the sealed ABI.
- **H2** `gf_scroll_state` mixes current state with last-event fields.
- **H3** `src/abi/error/abi_error.cpp:5-6`, `registry.hpp:3228` — one
  process-wide registry. Two independent consumers loaded together share it;
  the 1.0 conformance gate asks for two independent consumers.

## I. Build, CI, tools

- **I1** `CMakeLists.txt` — 3,112 lines, 185 target definitions, no
  functions. `dib_frame_store.cpp` is listed in five targets (`:1046, 1913,
  1928, 1946, 1965`).
- **I2** 158 tests, 17 timeout or serial properties. A hung test waits for the
  40-minute job limit (`native.yml:79`).
- **I3** One fuzz target (PNG validation). No fuzzing for the clipboard
  decoders, the automation command parser, `stx_vorbis`, or the coming theme
  and form-document parsers. I read both clipboard decoders and found no
  defect.
- **I4** `tools/check_house_style.py:37` — the extension list omits `.inc`
  and `.c`. Eight `.inc` sources are unchecked (clean today);
  `atomic_pool.c` has 91 arrows. Say whether C is exempt.

## J. Documents

- **J1** `docs/dev/roadmap.md:188` lists CSS under "Not planned"; lines 44 and
  81 of the same page plan CSS-syntax themes and CSS import.
- **J2** `docs/dev/architecture.md:26` says the Windows renderer is a DIB
  compositor; `AGENTS.md` says CPU-only Skia behind the drawing vocabulary.
- **J3** `AGENTS.md` still carries "Scope of the first proving slice" (a macOS
  gallery), "Agent ownership during the initial parallel build" and the
  Lucida Grande paragraph; `PROVING_SLICE.md` likewise. They load as binding
  instructions every session.
- **J4** ADRs live in `decisions/` and `docs/dev/adr/`. Numbers 005 and
  007-012 do not exist in either.
- **J5** `FORWARD_PROGRAMME.md` §3A says only Windows has a registered
  accessibility test; `tests/linux_accessibility_fixture.cpp` and its client
  exist. Recheck the statement.
- **J6** Stale comments: `window_presentation.cpp:626-630` (C7),
  `control.hpp:886-888` (A14).
