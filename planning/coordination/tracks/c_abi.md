# Track: C ABI (stage C, critical path to 0.5)

Started 2026-11-01 in workspace `D:\gui_forms_w5` (checkout `/d/gui_forms_w5/gui_forms`).
The stx_skia track that held this workspace is paused: leave its branches
(`claude/stx-skia-*`) and the harness branch `sdk-cache` alone.

## Authority and inputs
- Coordinator's task (2026-11-01). Briefs: `D:/malkuth/projects/briefs/BRIEF.md`
  (M4 parts do not apply: the build host is this Windows x86-64 machine),
  notes main `HOUSE_STYLE.md`, notes gui_forms `planning/stages/C-c-abi/README.md`,
  `planning/ROADMAP_TO_0.5.md`, coordinator desk `PLAN_0.5_LEG.md`, `TRAJECTORY_TO_1.0.md`.
- 0.5 gate: (1) an installed, versioned C ABI with an error model; (2) a
  breaking-change audit of its function tables, the C++ wrappers being RAII
  conveniences, not a second contract; (3) PlaySuite and SwiftEdit (after their
  reworks) passing the ownership, thread and callback conformance tests against
  the installed SDK. C# bindings roll out alongside, no demo program.

## Workflow
- Base: `origin/checkpoint/2026-11-01-4` (b94c9a0a) until it merges, then rebase onto main.
  Harness at its pin 53af66c (detached in `/d/gui_forms_w5/template_harness`).
- Commit on local `claude/c-abi-*` branches; never push gui_forms. Push only this file, to projects main.
- Routine check: `sh /d/gui_forms_w5/dev/c_abi/check.sh '<ctest regex>'` (build + touched
  tests plain and `--sanitize` + house style; logs in `dev/c_abi/logs/`).
- Python: `C:/Users/Shadow/AppData/Local/Programs/Python/Python314/python.exe`.
  Clang 22.1.8 (x86_64-w64-windows-gnu): `/d/a/_temp/msys64/clang64/bin/` (also llvm-dlltool,
  llvm-readobj, llvm-nm, llvm-objdump). No .NET SDK on this machine or in CI.

## File map (current experimental 0.x ABI)
- `include/gui_forms/c_api.h` (gf_api_v0, 87 slots, ABI 0.1 to 0.26, one export
  `gf_get_api_v0` plus 14 `gf_windows_paint_endpoint_*_v1` exports on Windows),
  `c_api.hpp` (abi0::Api/Control wrapper), `drawing_c_api.h` (gd_api_v0, 110 slots, separate DLL).
- `src/abi/c_api.cpp` (entry and slot functions), `src/abi/registry/registry.hpp` (3,228 lines:
  generational handles, one mutex, ui_thread per record), `src/abi/error/abi_error.*`
  (thread_local code + message, `translate` catches exceptions).
- Target `gui_forms_c_api` (output `gui_forms_abi0`, SHARED, not installed). Export checks:
  `cmake/check_c_api_exports.cmake` (macOS), `check_linux_c_api_exports.cmake`; none on Windows.
- Tests: `tests/c_api_c11_tests.c` (1,627 lines), `c_api_cpp_tests.cpp`, `abi_registry_lifetime_tests.cpp`.
- C# today: `tools/facade_generator/Program.cs` writes `generated/facade-v1/.../NativeBridge.g.cs`
  (hand-shaped bridge over gf_api_v0); `tools/compat_capture/` measures Forms member use.
- Installed-SDK consumers: `examples/reference/` (built by `tools/ci_build.py` full mode after `cmake --install`).

## Plan (draft 1, 2026-11-01; reported to the coordinator)

### Where the 0.x ABI falls short of the gate
1. Not installed: no CMake component, no pkg-config, the DLL is not in the SDK.
2. One flat table of 87 slots grown by 26 minor numbers; every minor accepted
   by an explicit list; no major; the version is not semantic.
3. Error model: `gf_result` is a C enum used as a return type (its size is
   implementation-defined, `-fshort-enums` changes it); the detail is a
   borrowed pointer into library thread-local storage, valid until the next
   failure; std::bad_alloc becomes INTERNAL; a missing host reports
   UNSUPPORTED_VERSION (version and capability conflated); failure outputs are
   unspecified.
4. Layout: implicit padding (gf_property_value bytes 36-40, others), structs
   larger than 16 bytes passed by value (gf_rect, gf_scroll_axis_state), no
   stated calling convention for callbacks.
5. Exports: Windows exports 15 symbols and nothing checks them; macOS and
   Linux check by hand-written scripts.
6. Threads: retain/release/dispose require the owning UI thread, so a C#
   finalizer or SafeHandle cannot release; callback contexts have no release
   hook ("must outlive the proxy handle"), so a binding cannot free a GCHandle
   deterministically.
7. Surface: no T1 schema enumeration, no generic property or interaction
   access by schema identity, no T2 theme loading or named inputs; many
   facade-specific slots (PropertyGrid, WinForms paint endpoint) sit in the
   portable table; two subscribe variants with different callback types.
8. Audit: none; nothing detects a reordered slot, a changed field or a value change.
9. C++ wrapper: hand-written, negotiates 0.4, covers 6 slots.

### Versioning
- One export: `gf_abi_get_table(table_id, abi_major, abi_minor, table, table_capacity)`.
  The header defines `GF_ABI_MAJOR`, `GF_ABI_MINOR`; the caller passes the
  values it was compiled with and `sizeof` its table.
- Tables per domain (runtime, objects, properties, events, dispatch/window,
  schema, theme, host services, then control families); extension tables
  (Windows compatibility paint) have ids >= 0x100 and report
  UNSUPPORTED_CAPABILITY where the host lacks them, so every platform exports
  the same one symbol.
- Every table begins `{uint32 struct_size; uint32 abi_minor}`. Within a major a
  table only grows at its tail; each slot records the minor that added it.
  Different major: UNSUPPORTED_VERSION. Requested minor above the library's:
  UNSUPPORTED_VERSION. Otherwise the library writes min(capacity, its size)
  bytes and reports its own size and minor.
- Never removed within a major; a deprecated slot stays and returns
  GF_ERROR_REMOVED with detail (compatibility policy: deprecation plus an error).
- The library file carries the major: `gui_forms_abi1.dll`,
  `libgui_forms_abi.so.1`, `libgui_forms_abi.1.dylib`.

### Error model
- `typedef int32_t gf_status`; `GF_OK` 0; failures positive. 1 to 8 keep their
  0.x numbers; new: OUT_OF_MEMORY, UNSUPPORTED_CAPABILITY, NOT_FOUND,
  INVALID_STATE, LIMIT_EXCEEDED, CANCELLED, REMOVED. Codes are append-only;
  a consumer treats any unknown nonzero code as failure.
- Every function returns gf_status. On failure every output is left unchanged
  (BUFFER_TOO_SMALL sets only the required size); the conformance kit checks
  this with sentinel-filled outputs.
- Detail: one record per thread {status, table id, slot index, message},
  overwritten only by the next failure on that thread; copied out by
  `runtime.error_detail(record*, buffer, capacity, required)` into caller
  storage, never borrowed; `runtime.clear_error()`. No mutable state crosses
  threads.
- Exceptions never cross: each slot translates (bad_alloc to OUT_OF_MEMORY).
  A callback must not unwind into the library; the C# thunks catch.

### Function-table layout and ABI-stability rules (checked by the audit)
- Types: fixed-width integers, double, pointers to declared structs, typedef'd
  function pointers with `GF_CALL`. No enum, bool, char signedness, size_t,
  long, wchar_t, bit-fields, unions or flexible arrays. Enumerations are
  `uint32_t` typedefs plus integer-literal macros.
- Objects: `typedef uint64_t gf_object` (generation high, slot low, 0 null), so
  no small-struct-by-value passing rules apply to handles.
- Structs: no implicit padding (explicit `reserved` fields), size a multiple of
  8; extensible inputs begin with `struct_size`; structs over 16 bytes pass by
  pointer.
- Text: UTF-8 `gf_string_view` in (pointer + uint64 length, validated); out into
  caller buffers with capacity and required size. Library-owned pointers only
  for immutable data with library lifetime (schema names), stated per slot.
- Calling convention: `GF_CALL` is the platform C convention (Microsoft x64,
  SysV AMD64, AAPCS64/Apple arm64); `__cdecl` spelled on Windows. 64-bit
  targets only; wasm32 is a later, separate decision.
- Callbacks: function pointer + context + optional context-release function;
  registration returns a token; the library calls release exactly once, on
  the owning thread, after the last invocation (disconnect, sender disposal
  or shutdown); disconnect during emission is defined (skips later turns).
- Threads: every object belongs to its window's UI thread; calls from another
  thread return WRONG_THREAD, except a stated any-thread set (release,
  begin_invoke, runtime table).
- Export allowlist: `manifests/abi/gf_abi_exports.txt`, from which the Linux
  version script, the macOS exported-symbols list and the Windows .def are
  generated; one checker on all three hosts (llvm-nm / llvm-readobj).
- Windows: install an MSVC-format import library (`gui_forms_abi1.lib`, from
  the .def through llvm-dlltool) beside the MinGW one, so Visual Studio
  consumers link.

### Install layout (component `CAbi`, target `GUIForms::CAbi`)
- `include/gui_forms/c/gf_abi.h` (C11, no C++ headers), `gf_abi.hpp` (RAII wrappers).
- `bin/gui_forms_abi1.dll` + `lib/gui_forms_abi1.lib` + `lib/libgui_forms_abi1.dll.a`;
  `lib/libgui_forms_abi.so.1*`; `lib/libgui_forms_abi.1.dylib`.
- `lib/cmake/GUIForms/GUIFormsCAbiTargets.cmake`, `lib/pkgconfig/gui_forms_abi.pc`.
- `share/GUIForms/abi/gf_abi.json` (the audited manifest; generators read it),
  `share/GUIForms/abi/gf_abi_exports.txt`.
- Later: `share/GUIForms/bindings/csharp/` (generated interop source) and
  `share/GUIForms/conformance/` (the conformance kit).

### Breaking-change audit (tools/abi_audit.py)
- `extract`: the pinned clang parses `gf_abi.h` (`-ffreestanding`, AST JSON plus
  `-fdump-record-layouts`) for x86_64-w64-windows-gnu, x86_64-pc-windows-msvc,
  x86_64-unknown-linux-gnu and aarch64-apple-darwin from any host, and writes
  `manifests/abi/gf_abi.json`: constants, typedefs, structs with per-target
  offsets and sizes, tables with slot order, signatures and the minor that
  added each slot, table ids, exports. Deterministic; `--check` fails when stale.
- `check`: the rules above (forbidden types, implicit padding, by-value size,
  layouts equal across targets) and, against `manifests/abi/baselines/<seal>.json`,
  append-only tables, unchanged signatures, fields, offsets, sizes and constant
  values, new slots carrying the current minor.
- `probe`: writes a C11 file of `_Static_assert`s on every size and offset,
  compiled on every CI host by the real compiler.
- `exports`: the built library's exported symbols equal the allowlist.
- Seeded-break fixtures under `tests/abi/audit_fixtures/`: each one a single
  break kind; the audit test requires every one to be reported.

### C# binding generation
- `tools/abi_bindings.py csharp` reads only `gf_abi.json` and writes
  `bindings/csharp/GUIForms.Interop/*.g.cs`: blittable sequential structs,
  tables as structs of `delegate* unmanaged[Cdecl]<...>`, constants, one
  `[LibraryImport]` for `gf_abi_get_table`, `[assembly: DisableRuntimeMarshalling]`
  (AOT-safe), a generated layout self-test against the manifest's offsets.
- A thin hand-written layer: SafeHandle-style object ownership (needs
  any-thread release), `[UnmanagedCallersOnly]` thunks with GCHandle contexts
  freed by the context-release callback, status-to-exception translation.
- The Forms-shaped API is stage S (0.6), not here.
- **Shadow frames in the thunks** (owner, 2026-11-02; desks/coordinator/
  DOTNET_DEBUG_VISIBILITY.md). The library already pushes a frame around every event
  and posted callback (GF_SHADOW_FRAME_KIND_EVENT_CALLBACK / POSTED_CALLBACK), so the
  thunks push nothing native. They use the frames three ways: (1) each generated
  `[UnmanagedCallersOnly]` entry calls a managed method named for its logical frame
  (`GuiForms.Dispatch.Click(Button ok)`, from the frame's kind, subject and name), marked
  `DebuggerStepThrough`/`DebuggerHidden` so the managed stack reads meaningfully; (2) the
  thunk's catch (the no-unwind rule) attaches `diagnostics.shadow_frames` as a snapshot to
  the exception it keeps for re-raise, so frames are read before the callback returns and
  balance is preserved; (3) `DebuggerTypeProxy` views on handles and a `ShadowStack`
  view read the table. C6 test: a C# handler that throws, caught by the thunk, leaves
  depth as it was (the C equivalent is in the kit; the C++ unwinding one in
  tests/shadow_frames_tests.cpp). The VS Concord call-stack filter (VS integration phase)
  reads the same table.

### Phases and gates
- **C1 skeleton with every gate wired.** Header, negotiation, error model and
  the runtime table; new target `gui_forms_abi` installed as `CAbi`; manifest,
  audit, layout probe, export allowlist on three hosts, MSVC import library;
  installed-SDK C11 consumer in `examples/reference`. Gate: the consumer
  negotiates and reads an error detail against the installed SDK; extract is
  byte-identical across two runs and four targets agree; seeded breaks 100%
  detected, 0 findings on the real header; exports equal the allowlist on
  Windows here (macOS and Linux by CI); touched tests clean under ASan/UBSan.
- **C2 objects, tree, events, dispatch, window.** Lifecycle with deferred
  any-thread release, tree, events by interaction identity with context
  release, begin_invoke, run/close; reuse the 0.x registry internals. Gate:
  conformance kit part 1 (ownership, thread, callback) in-tree, plain and
  sanitized; slot coverage 100% (every slot exercised, counted by a tool).
- **C3 T1 schema.** Enumerate control types, parts, states, properties,
  interactions, item systems; create by type id; get/set/reset properties by
  schema identity. Gate: the schema serialized through the ABI equals
  `manifests/control-schema.json` (115 types) exactly.
- **C4 T2 engine and host services.** Load a theme (atomic replace), publish
  named inputs; dialogs, clipboard, tooltips, cursors, capability reports.
  Gate: a theme loaded through the ABI gives the same paint trace as the C++
  path on a fixture; per-host capability matrix test.
- **C5 control families and S coverage.** Field editing, range, scroll, layout
  transactions, property proxies, popups, owner-draw pixel surfaces, the
  Windows compatibility extension table. Gate: the compat_capture member set
  maps to ABI slots or schema properties with 0 unmapped (or each deferral
  listed for the owner).
- **C6 C++ wrappers and C# interop.** Generated header-only RAII wrappers (each
  method calls exactly one slot; generated from the manifest, so no second
  contract); generated C# interop. Gate: wrapper slot map 1:1 checked; C#
  layout self-test green on three CI hosts (needs decision 4).
- **C7 seal and audit.** Baseline committed; CI audit job; audit record under
  notes `results/`. Gate: 0 findings vs baseline, seeded breaks 100%.
- **C8 consumers.** Conformance kit installed; PlaySuite and SwiftEdit run it
  in-process against the installed SDK (product workers); TSan on Linux/macOS.
  Gate: both runs recorded under `results/`, plain and sanitized.
- The 0.x ABI keeps building, uninstalled, until the facade (R) migrates.

### Owner decisions in force (answered 2026-11-01, relayed by the coordinator)
1. **Numbering:** major 1. Nothing is truly sealed until 1.0. ABI 1 is the 0.5
   stability target; the 0.5 audit baseline is a recorded snapshot. Between
   0.5 and 1.0 changes are allowed, but every break is recorded with its
   reason; the audit reports breaks against the baseline and does not block.
   At 1.0 the audit becomes blocking and the major is frozen.
   **Audit mode switch (my design, C2):** `manifests/abi/audit_policy.json`
   holds `mode` (`report` | `blocking`), the baseline path (absent until C7
   records the 0.5 snapshot) and a break ledger (one entry per break: symbol,
   rule, reason, date). `report`: breaks vs the baseline that the ledger names
   print as recorded breaks and pass; a break the ledger does not name fails
   with "record its reason" (the break is allowed, silence is not); stale
   ledger entries are reported. `blocking`: every break fails, the ledger is
   closed, and a change of `GF_ABI_MAJOR` fails. Rule findings on the current
   header (dialect, padding, layout disagreement) fail in both modes. The
   switch to `blocking` is one owner commit at 1.0; `--mode` overrides it for
   the fixture tests only.
2. **One engine per process:** the ABI library links the Application shared
   library.
3. **Consumers at 0.5: the kit only.** PlaySuite and SwiftEdit link the
   installed CAbi and run the conformance kit in their own tests; they move
   wholly onto the new approach after 0.5 is declared complete.
4. **Release from any thread;** destruction deferred to the owning UI thread.
5. **.NET:** the owner will set up Visual Studio (.NET desktop workload, .NET 10
   SDK) and admits a pinned .NET 10 LTS SDK. Today the host has neither: C#
   work is generation only, nothing downloaded. When the pinned SDK is needed,
   tell the coordinator the exact version, file, source URL, size and hash.
6. **Drawing ABI (gd_*):** pending; stays experimental and unchanged until relayed.
7. **C# shipping:** source in the SDK, evolving; NuGet packages finalised and
   shipped at 1.0 (not 0.6).
Consequences for the phases: C6's C# gate is generation plus a layout table
checked by the audit until the SDK is admitted; C8 is the kit only.

## C1 file map (branch `claude/c-abi-c1`)
- `include/gui_forms/c/gf_abi.h`: the contract (dialect rules in its head comment).
- `src/abi/stable/entry/abi_entry.cpp` (export + runtime table),
  `src/abi/stable/failure/abi_failure.*` (per-thread fixed-size failure record).
- `cmake/CAbi.cmake` (target `gui_forms_abi`, allowlist-generated .map/.txt/.def,
  llvm-dlltool MSVC .lib, install, tests), `cmake/gui_forms_abi.pc.in`,
  `cmake/GUIFormsConfig.cmake.in` (CAbi component).
- `manifests/abi/gf_abi.json` (manifest), `manifests/abi/gf_abi_exports.txt` (allowlist).
- `tools/abi_audit.py` extract | check [--baseline] | probe | exports.
- Tests: `tests/abi/gf_abi_runtime_c11_tests.c`, `gf_abi_layout_probe.c` (generated) +
  `_cpp.cpp`, `abi_audit_tests.py` + `audit_fixtures/base.h` (seeded breaks).
  ctest names `gui_forms_abi_runtime_c11_tests`, `_layout_probe_c11|cpp`, `_audit`,
  `_layout_probe_current`, `_audit_fixtures`, `_exports`.
- Installed consumer: `examples/reference/c_abi_consumer.c` (test `c_abi_consumer`).
- Docs: `tools/generate_library_docs.py` C ABI group gains area `c`; `docs/library/manual.json`
  disambiguates `gf_string_view` by declaration_header.
- Workspace scripts: `dev/c_abi/check.sh` (routine check), `dev/c_abi/install_check.sh`
  (full plain build, install to dev/c_abi/sdk, build and run only c_abi_consumer).
- Lesson: the Bash tool unescapes `\n` inside heredocs; write files with backslashes via Write.
- Lesson: generate_library_docs.py on Windows rewrites every page with CRLF; restore the
  files whose `git diff --stat` is empty before committing.

## C2 file map (branch `claude/c-abi-c2`, stacked on c1)
- `gf_abi.h`: Threads/Callbacks contract; gf_object_state, gf_value(_kind), gf_event,
  gf_callback, gf_subscription; tables objects (2), properties (3), events (4),
  dispatch (5), windows (6); gf_run_flags (none at 1.0: a primary window cannot start
  hidden, Application::validate refuses it), gf_run_options.
- `src/abi/stable/registry/abi_registry.*`: `HandleRegistry` (named so: the docs generator
  refuses a second `Registry`, the 0.x one), one mutex; object slots with generation +
  reference count + Control* index (child_at/parent reuse an identity); subscription
  slots (reserve, connect, complete); deferred controls for off-UI-thread last releases;
  posted queue (retains target); run state (window handle, host wake called under the
  lock, cleared at closed). UI thread binds at the first create, unbinds when idle.
  `CallbackBinding` (release in destructor; `disarm` when a call fails) and
  `SubscriptionBinding` (owned by the forwarder in the C++ event; destructor frees the
  identity). Helpers: read_text (UTF-8 validated), write_text, fail_with_current_exception.
- `src/abi/stable/tables/abi_{objects,properties,events,dispatch,windows}.cpp`, one table
  each; `abi_tables.hpp`; entry's find_table serves them.
- Child order in the ABI = `Control::children()` (back to front); insert_child maps to
  WinForms `set_child_index` (top index = count - index).
- `cmake/CAbi.cmake`: links gui_forms_application; skipped (`return()`) without it;
  kit library `gui_forms_abi_conformance_kit`, tests `gui_forms_abi_conformance` (60 s
  timeout; shows a window briefly) and `gui_forms_abi_slot_coverage`.
  `GUIFormsConfig.cmake.in`: CAbi included only with GUIForms::Application.
- Audit: `manifests/abi/audit_policy.json` {format, mode, baseline, breaks[{finding,
  reason, recorded}]}; `check --policy/--mode/--baseline`; `coverage <sources>` matches
  `.<table>.<slot>(` calls. Fixtures 27/27 breaks, 5 legal.
- Kit: `tests/abi/conformance/` (gf_conformance.h public entry `gf_conformance_run(FILE*,
  flags)`; internal.h; kit, ownership, values, callbacks, threads, window, main).
- Lesson: heredocs still unescape backslash-n in Python patch scripts; write patch scripts
  with Write (kept in dev/c_abi/patch_*.py).

## C3 file map (branch `claude/c-abi-c3`, stacked on c2)
- `gf_abi.h`: GF_TABLE_SCHEMA 7, `gf_schema_table` (17 slots: vocabulary_count/at, type_count,
  find_type, type_at, item_system_at, part_at, state_at, custom_state_at, property_at,
  choice_at, interaction_at, engine, primitive_at, primitive_name_at, parameter_at,
  parameter_choice_at); records gf_schema_{type,item_system,part,property,choice,
  interaction,engine,primitive,parameter}; enums gf_schema_{kind,vocabulary (14 lists),
  property_flags,serialization,parameter_kind,primitive_list}; GF_SCHEMA_NONE (all ones).
  Schema text is lent until process end (the stated exception to no-lending). gf_value:
  kinds 6..15, tail records gf_logical_{point,size,rect,insets} (gf_point/size/rect are
  0.x c_api.h names: the coexistence probe fails on a clash; the audit does not check it),
  `color` (0xRRGGBBAA) replaces `reserved`; size 152. objects.type_name -> type_id.
- `src/abi/stable/schema/`: `abi_schema_recipes.hpp/.cpp` (SchemaSample, record_control /
  record_component, `create_type<T>` = make_control when constructible from StableId else
  null, `describe_type<T>` primary static_asserts that; explicit specializations for
  anchored_popup_layer, dock_site, flags_value_editor, page_strip, every component, and
  LiveSurface (service, records nothing)); `abi_schema_catalog.hpp` GENERATED by
  `tools/control_registry.py emit-abi` (+ --check in CI native.yml, ctest
  gui_forms_abi_schema_catalog_current, unittest); `abi_schema.hpp/.cpp` (leaked
  Schema built at first call: vocabularies copied, types copied from one fresh instance
  each, primitives span of builtin registry; catalog_type_count/find_catalog_type
  (registered_name)/create_catalog_type, value_kind_of).
- `tables/abi_schema_table.cpp`: `SchemaCall` binds an unbound library for the call and
  `unbind_if_idle` (now public on HandleRegistry) at its end; output records need
  struct_size >= 1.0 size and get the 1.0 fields after struct_size.
- `abi_objects.cpp`: create by any spelling via the catalog (UNSUPPORTED for components and
  the four controls above); type_id = anatomy().control_id; objects_state switch moved to
  a compliant function (state-machine F1).
- `abi_properties.cpp`: point/size/rectangle/insets/colour/enumeration get and set
  (finite checks); font/image/object/collection UNSUPPORTED until C5.
- `include/gui_forms/theme/part_painting/part_painting.hpp`: `part_value_names` (shared
  by the export test and the ABI vocabulary).
- Gate test `tests/abi/schema/gf_abi_schema_export.cpp` (C++ consumer of gf_abi.h only,
  ctest gui_forms_abi_schema_export, writes build/gf_abi/control-schema.abi.json). Kit part
  `gf_conformance_schema.c`. Slot coverage reads tests/abi/conformance and tests/abi/schema.
- Stable ids must be kebab segments ("gf-abi.schema.sample"), not snake.
- Not in C3 (deferred): events for every schema interaction (still the C2 set) -> C5 with
  the control families; create for components and the four controls -> C4/C5.

## C4 file map (branch `claude/c-abi-c4`, stacked on c3)
- Commit 1 (68346e63): `tools/abi_audit.py` rule `legacy-name`: every name gf_abi.h declares
  is disjoint from the 0.x `gui_forms/c_api.h` (found beside the header's c/ directory);
  fixtures 28/28 breaks, 6 legal.
- Commit 2 (947e75a4, core seam, flag to the coordinator): `ApplicationResources::
  compile_theme_package(package, library, options)` compiles a program package into the
  resources' own registry (the free function needs a registry the windows cannot see).
  Test in `tests/shared_resources_tests.cpp`.
- Commit 3 (38f2621c): `gf_abi.h` GF_TABLE_THEMES 8 (`gf_themes_table`: load, reset,
  install, remove, diagnostic_at, diagnostic_text, theme_id, publish_value, withdraw_value,
  published_value) and GF_TABLE_HOST 9 (`gf_host_table`: capabilities, platform_name,
  set_cursor, read_clipboard_text, write_clipboard_text, show_dialog, dialog_path_at);
  records gf_bytes_view, gf_theme_{file,package,report,diagnostic}, gf_host_capabilities,
  gf_file_filter, gf_dialog_{request,result}; enums gf_theme_{outcome,severity,
  diagnostic_text}, gf_host_capability (uint64 bits = C++ bit positions, mapped by an
  explicit table), gf_pointer_cursor (gf_cursor_kind is a 0.x name), gf_dialog_{kind,
  choice,flags,outcome}, gf_message_{buttons,icon}; limits GF_THEME_PACKAGE_MAXIMUM_FILES
  256 / _BYTES 16 MiB, GF_THEME_MAXIMUM_PUBLISHED_VALUES 256, GF_DIALOG_NO_FILTER.
- `src/abi/stable/engine/abi_engine.*`: `EngineState` (leaked; shared ApplicationResources
  made on first use, which calls `HandleRegistry::hold_binding()` so the UI thread stays
  bound; ThemeLibrary; controls = every schema anatomy; findings; published names/values;
  running Window* from ready to closed; last dialog's paths) and `make_run_window`
  (Window sharing the resources + published values), used by `abi_windows.cpp`.
- `tables/abi_themes.cpp`, `tables/abi_host.cpp`, `tables/abi_records.hpp` (check_record /
  write_record, moved out of abi_schema_table.cpp).
- CMake: `gui_forms_abi_objects` OBJECT library (GF_ABI_BUILD public); the shared library
  is made from its objects. Gates: `gui_forms_abi_theme_trace`
  (`tests/abi/themes/gf_abi_theme_trace.cpp`, links the objects + application statically,
  writes trace-{builtin,abi,cpp}.txt to build/gf_abi) and `gui_forms_abi_capability_matrix`
  (`tests/abi/host/gf_abi_capability_matrix.c`, C only, reads
  `manifests/abi/host_capabilities.json`: rows win32-dib, appkit-macos, linux-x11). Kit
  parts `gf_conformance_themes.c`, `gf_conformance_host.c` (host checks during a run come
  from the window part's ready; the kit never writes the clipboard or shows a dialog).
  Slot coverage also reads tests/abi/host.
- Theme language facts: opacity and other fraction properties refuse input-reading
  expressions (E405); a published value is read by primitive parameters, e.g.
  `#id::part(face) { primitive: bulge; amplitude: clamp(0, var(--level), 1) }`. Stock
  Button/CheckBox paint their classic recipe; a part the engine draws needs `paint_part`.
- Lesson: toolchain_env.sh puts the MSYS2 runtime's usr/bin first; any MSYS2-runtime tool
  (sh, env) started from Git Bash then gets an almost empty environment (no USERPROFILE,
  CC...), so ccache failed a consumer configure run from a shell that had sourced env.sh.
  Fixed in the workspace's env.sh (2026-11-01): sourced once per shell
  (GF_W5_ENV_SOURCED) and, under Git Bash, clang64/bin then Git's /usr/bin go ahead of
  MSYS2's usr/bin. Verified: install_check.sh passes from a sourced shell.
- Toolchains for C6 (coordinator 2026-11-01, installed and usable): Visual Studio
  Community 2026 18.11.3 (installationVersion 18.11.12328.437) at
  C:/Program Files/Microsoft Visual Studio/18/Community, MSVC toolset 14.51.36231,
  Windows SDK 10.0.28000.0, .NET SDK 10.0.403 (runtime 10.0.14; the pin candidate).
  Smoke tests (dev/c_abi/vs_smoke): msvc_consumer.c built by cl /W4 /WX against the
  installed gui_forms_abi1.lib runs ("GUI.Forms 0.2.0, C ABI 1.0, host win32-dib");
  a net10.0 console built offline (empty --source) calls gf_abi_get_table and
  runtime.version through a function pointer. C6 takes: C# bindings compiled and tested
  with that SDK plus the layout self-test, and the MSVC .lib consumer as a ctest. The
  standalone pinned SDK for users without VS comes later; any download is asked first.

## State
- 2026-11-01: baseline at b94c9a0a: c_api|abi_registry tests 5/5 plain and sanitized, style clean.
- 2026-11-01: **C1 done**, local branch `claude/c-abi-c1` at 9f5ea363 (one commit on
  origin/checkpoint/2026-11-01-4; nothing pushed). Gate measured on Windows x86-64:
  12/12 ABI and c_api tests plain and ASan+UBSan; seeded breaks 23/23, legal changes 0/3
  reported; manifest current, four targets agree, 0 findings; DLL and MSVC .lib export
  exactly gf_abi_get_table; installed-SDK c_abi_consumer passes; style, lifetime,
  state-machine and library-docs checks clean. CI only: macOS/Linux exports and probe.
- Plan and decisions 1 to 7 reported to the coordinator; answers recorded above.
- 2026-11-01: **C2 done**, local branch `claude/c-abi-c2` at 1191b97b (one commit on
  c1's 9f5ea363; nothing pushed). Gate measured on Windows x86-64: kit part 1, 215
  checks, 0 failed, plain and ASan+UBSan; repeated 15x plain and 10x sanitized, no
  flake; slot coverage 26/26; touched tests (gui_forms_abi|c_api|application_native)
  15/15 plain and sanitized; audit fixtures 27/27 breaks, 5 legal; DLL still exports
  only gf_abi_get_table; installed-SDK c_abi_consumer passes; style, lifetime,
  library-docs clean; state-machine gate: c-abi area 0 findings. CI only: macOS/Linux.

- 2026-11-01: checkpoint 9 (#459) merged; stack rebased onto origin/main 3e0c8d5a:
  c1 01c6a9c2, c2 89d6d07f, c3 2065dc68 (nothing pushed).
- 2026-11-01: **C3 done**, local branch `claude/c-abi-c3` at 2065dc68. Gate measured on
  Windows x86-64: the schema read through gf_abi.h alone equals
  manifests/control-schema.json byte for byte (115 types, 772570 bytes), plain and
  ASan+UBSan; kit 272 checks 0 failed (create by every schema control: 87 made, 4 + all
  components refused, type ids checked); slot coverage 43/43; touched tests
  (c_api|abi|application_native) 17/17 plain and sanitized; gallery_pages_tests plain and
  sanitized; emit-abi/emit-cpp current; registry unittest 24/24; audit 0 findings;
  installed-SDK consumer passes; style 1500 files 0; lifetime 0; library docs current;
  state-machine gate c-abi 0. CI only: macOS/Linux.

- 2026-11-01: **C4 done**, local branch `claude/c-abi-c4` at 38f2621c (3 commits on c3;
  nothing pushed). Gates on Windows x86-64, plain and ASan+UBSan: theme trace equal ABI vs
  C++ (and sensitive to theme, refusal-atomic, published values live); capability matrix
  win32-dib row equal; kit 356 checks 0 failed; kit + trace + matrix repeated 10x plain and
  5x sanitized; touched tests (c_api|abi|shared_resources|application_native) 20/20 plain
  and sanitized; slot coverage 60/60; audit 0 findings, fixtures 28/28 + 6 legal; style
  1506 files 0; lifetime 0; state-machine gate (changed from origin/main) no F1/F2;
  library docs current; installed-SDK consumer passes. CI only: macOS/Linux rows.


- 2026-11-01: checkpoint 10 (#460) dropped the stack on one conflict
  (tests/gallery_pages/schema_export_tests.cpp: c3's loop over part_value_names vs the DGV
  colours topic's "authored_color"). Rebased c1..c4 onto main e22ea9c9 (checkpoint 10
  merged) with authored_color added to part_value_names (c3); docs/API inventory
  regenerated in a commit on c4 (7e46a767). Branches: c1 fdbfde76, c2 7b6740be,
  c3 c3239830, c4 7e46a767 (c981ab06, 917d5cfa, b2a02db1, 7e46a767); c5 = c4.

## C5 plan (draft, 2026-11-01)
Survey: 1004 interaction declarations across the declaring classes (176 files; each class
declares its own in append_interactions, inherited ones come from the base), 1615 (type,
interaction) pairs, ~150 payload types: scalars (bool, std::string, size_t, int, double,
optional<...>), aggregates of scalars/ids/enums/geometry (PointerEvent, ScrollEvent,
DataGridView*Event, Page*Request, ...), a few with collections (vector, span), objects
(Control*, shared_ptr<TabPage>, Binding*), BindingValue, and owner-draw (ItemContentPaint
holds Painter&). C++ events are typed accessors named as the interaction.
- **C5a events for every interaction.** Generator tools/abi_event_catalog.py (emit and
  --check, like emit-abi): from the append_interactions arrays and the payload structs in
  include/, writes src/abi/stable/events/abi_event_catalog.hpp: one connector per
  (declaring class, interaction) that dynamic_casts the source and subscribes a forwarder
  deduced from the Event's argument types; one field writer per payload struct (members
  flattened, nested structs dotted), one writer-back per mutable payload, enum choice
  names. ABI: gf_event grows `field_count` + `fields` (gf_event_field {name, writable,
  value}); a single scalar payload also stays in `payload` (unchanged); new slots
  events.write_field (validates writability and kind, copies text, applies to the C++
  payload when the callback returns) and events.field_element (lazy collection element:
  a COLLECTION field holds its count). Objects in a payload are lent an identity for the
  callback (retain to keep). Not representable at C5a and listed as deferrals: Painter
  (owner-draw family), std::exception_ptr, maps (FieldMap/ErrorMap).
- **C5b components and the four irregular controls.** Registry holds Component::Ptr
  (controls and components); creation slots for components (owner given) and for
  anchored_popup_layer, dock_site, flags_value_editor, page_strip (their extra object as a
  handle argument).
- **C5c property proxies.** FONT, IMAGE, OBJECT, COLLECTION gf_value forms.
- **C5d families and the compat gate.** Field editing, range, scroll, layout transactions,
  popups, owner-draw pixel surfaces, the Windows compatibility extension table; gate: the
  compat_capture member set maps to ABI slots or schema properties, 0 unmapped (deferrals
  listed for the owner).

- 2026-11-01: **C5a done**, `claude/c-abi-c5` at 38e5d5ee (on c4 7e46a767). Files:
  tools/abi_event_catalog.py (generator, --check; ctest gui_forms_abi_event_catalog_current),
  src/abi/stable/events/abi_event_catalog.hpp (generated: 258 declarations, 138 structs,
  55 enumerations, 7 deferred members), abi_event_fields.{hpp,cpp} (EventFields frame,
  forwarder, scalar writers/readers, lending), src/abi/stable/values/abi_values.{hpp,cpp}
  (gf_value <-> BindingValue, moved from abi_properties.cpp), abi_events.cpp (connector
  lookup by name, most derived first; write_field slot 3, field_element slot 4),
  SubscriptionBinding::emit(gf_event&). ABI: gf_event_field (32 bytes, fixed stride),
  gf_event_field_flags WRITABLE/OPTIONAL/ANY_KIND, gf_event + field_count, fields. Tests:
  kit part gf_conformance_events.c (every interaction of every created type: 1505 on 87
  types, 0 refused; enumeration field via a property change); tests/abi/events/
  gf_abi_event_fields.cpp (static link; raises C++ events: tab selecting cancel write-back,
  lent page identity ends unless retained, list selection collections, optional text
  written and cleared, two-argument Dirty flags names). Verified plain + ASan/UBSan
  (abi|c_api 20/20), kit 369 checks 0 failed, audit 0 (39 records), docs current, style,
  lifetime 0, state-machine rc 0, installed consumer passes.
  Decision taken in-track (reported): payloads as named field lists, not per-payload C
  records (one marshalling shape for C#; nothing sealed before 1.0).

- 2026-11-02: **C5b done**, `claude/c-abi-c5` 12059b59: registry holds Component::Ptr;
  find_object answers controls only (WRONG_HANDLE_KIND for components), find_component any;
  create makes components from nothing or a string id; objects.create_with (slot 12,
  gf_create_arguments 56 bytes: flags COMBINABLE, owner, enumeration + choices, value)
  makes anchored_popup_layer (anchor control), dock_site (dock_workspace, kept alive via
  engine keep_owner), page_strip (page_view/navigation_pane, kept alive), flags_value_editor
  (caller enumeration), color/font/gallery_command (command); window-owned components
  (background_worker, binding*, error/help_provider, image_list, notify_icon,
  ribbon_tool_tip, semantic_feedback, timer, tool_tip) and live_surface refused
  UNSUPPORTED_CAPABILITY. Kit gf_conformance_create.c; events gate now 1589 interactions on
  102 types. Kit 407 checks 0 failed; abi|c_api 20/20 plain + sanitized; audit 0; docs,
  emit-abi, event catalog current; style, lifetime 0; installed consumer passes.
  Open for the coordinator: how window-owned components are made (options sent with the
  milestone report; recommendation: a window handle object).
- 2026-11-02: **Shadow frames done** (owner scope via coordinator), `claude/c-abi-c5`
  5955a979. Core: include/gui_forms/diagnostics/shadow_frames/shadow_frames.hpp +
  src/core/diagnostics/shadow_frames/shadow_frames.cpp (thread_local fixed stack of 64
  112-byte records; kind, flags, interface/function ids, sequence, subject 48 B, name 32 B,
  cut at a UTF-8 boundary with a flag; depth/pushed/dropped counters; ShadowFrameScope RAII,
  so exceptions balance). Push points: every C ABI slot except the runtime and diagnostics
  tables (59, script dev/c_abi/c5/shadow_slots.py), SubscriptionBinding::emit (event
  callback), run_posted (posted callback), Window::dispatch_pointer/key/text/drag,
  ensure_layout (layout pass), paint_rectangles (paint pass). ABI: GF_TABLE_DIAGNOSTICS 10,
  gf_diagnostics_table.shadow_frames(frames, capacity, required, info), gf_shadow_frame
  (112-byte stride), gf_shadow_stack_info (24), kinds 1..6, flags, capacities. Tests:
  tests/shadow_frames_tests.cpp (nesting, overflow 70 deep: 64 recorded + 6 dropped,
  truncation, exception balance, per-thread, window layout/paint/input frames, throwing
  on_pointer leaves depth 0); kit gf_conformance_diagnostics.c (contract, event and posted
  frames, failing call inside a callback balances; callbacks never nest so overflow is not
  reachable through the ABI). Cost (benchmarks/shadow_frames_benchmark.cpp, Release,
  Windows x86-64 clang64, this host, 3 runs, 20M pairs / 20k turns): library_call push+pop
  6.2-12.9 ns, text frame 12.4-37.1 ns (noisy host); renderer-neutral window workload
  (200 controls; pointer move + layout + paint per turn, 84-165 us/turn) opens 3 frames
  per turn, estimated share 0.044-0.068%. The Linux perf hot-paths workloads
  (tools/profile_hot_paths.py) were not run here (Linux only). No off switch: the
  measurement does not demand one. Gates: abi|c_api|shadow_frames 21/21 plain + sanitized;
  core/basic_controls/dispatcher/timer/dock_site/data_grid_view 6/6 plain; kit 433 checks
  0 failed; audit 0 (44 records, 180 constants); docs current (diagnostics area mapped to
  the window page in tools/generate_library_docs.py); style, lifetime 0; state-machine rc 0;
  installed consumer passes.

- 2026-11-02: coordinator chose option 1 (a window handle object). C++ check: the
  C++ API constructs Timer/ToolTip/ErrorProvider/HelpProvider/ImageList/NotifyIcon/
  BackgroundWorker/RibbonToolTip/SemanticFeedback/BindingContext/BindingSource over a
  `Window&` before Application::run(std::unique_ptr<Window>) takes the window; the program
  owns the components, which only observe the window (weak WindowLifetime) and become
  unbound when it is destroyed. The window does not own them. The ABI follows that: owner
  in create_with means "binds to", and neither keeps the other. Binding is not
  window-bound (Binding(Control& target, property, shared_ptr<BindingSource>, member)):
  deferred to C5d with its own argument shape. Reported to the coordinator.
  DONE (feb17b88, below). Was in progress in /d/gui_forms_w5/gui_forms on claude/c-abi-c5:
  src/abi/stable/windows/abi_window_handle.{hpp,cpp} (WindowHandle component owning the
  Window until run), windows.create slot 2, run/request_close take a window or a root,
  create_with on_window recipes (11 types), objects type_id "window" / stable_id root's,
  kit (gf_conformance_create.c windows(), gf_conformance_window.c gf_kit_window_handle),
  patch script dev/c_abi/c5/kit_windows.py already applied. Not yet built: next run
  abi_audit extract/probe, then check.sh 'abi|c_api|shadow_frames', fix, commit.
- 2026-11-02: checkpoint 11 docs job fix: branch `claude/c-abi-docs-cdomain` 9d4acc6c on
  origin/checkpoint/2026-11-02-2 (worktree /d/gui_forms_w5/docsfix): tools/api_docs_rst.py
  renders `ret (GF_CALL* name)(...)` slots as `.. cpp:member:: ret (*name)(...)` (the
  form the drawing C API's slots already use and the docs job accepts); 61 directives
  changed. No Sphinx on this host (not installed); generate --check PASS, unittest OK.
  c5 needs the same tools change cherry-picked when it is next regenerated.

- 2026-11-02: **Window handles done**, `claude/c-abi-c5` feb17b88 (on 50b04d63, the
  docs fix cherry-picked from 9d4acc6c). windows.create slot 2; WindowHandle
  (src/abi/stable/windows/); run/request_close take a window or a root; create_with
  on_window recipes (11 types); header states owner = "binds to" (coordinator accepted).
  Kit: thread handover now runs before create_with/events (the engine's first use binds
  the UI thread for good; that is by design). Events gate 1617 interactions on 113 types;
  kit 468 checks 0 failed (window part included); abi|c_api|shadow_frames 21/21 plain +
  sanitized; audit 0; docs current (0 GF_CALL* directives); style, lifetime 0; sm rc 0;
  installed consumer passes.

- 2026-11-02: checkpoint 11 linux fix: branch `claude/c-abi-install-rpath` 07fc41f6 on
  origin/checkpoint/2026-11-02-2 1a9a89cb (worktree /d/gui_forms_w5/docsfix):
  INSTALL_RPATH $ORIGIN (@loader_path on macOS) on gui_forms_abi and gui_forms_application
  (GUI_FORMS_INSTALLED_LIBRARY_RPATH in CMakeLists.txt). No rpath rules existed before;
  C++ consumers link application and threading directly, the C consumer links only the ABI
  library. Not run on Linux here.
- 2026-11-02: **C5c done**, claude/c-abi-c5 8d519e71 (on feb17b88). FONT = gf_font record
  in gf_value (role 0-2, weight 1-1000, italic 0/1, reserved 0, then valid_font_spec);
  IMAGE = registry id in unsigned_integer, themes.load_image (slot 10, PNG via
  resources().load_png; limits -> LIMIT_EXCEEDED, allocation -> OUT_OF_MEMORY, else
  INVALID_ARGUMENT) and themes.remove_image (slot 11, NOT_FOUND when absent); COLLECTION =
  count in unsigned_integer on output, properties.element_at (slot 3) per element; input
  elements/element_count (max 4096, no nested, no nulls, one kind), built with item type
  "value" then reshaped to the property's default collection (shape_collection). OBJECT
  stays UNSUPPORTED (no schema property has one); event fields fall back to OBJECT only.
  gf_value is 200 bytes. tools/abi_audit.py admits `struct gf_X*` (a record naming itself
  by tag). Kit part gf_conformance_proxies.c (runs after themes). Gates: abi|c_api|
  shadow_frames 21/21 plain and sanitized; audit 0 findings; docs regenerated and current;
  style, lifetime 0; state machines rc 0; registry, emit-abi, event catalog current;
  installed consumer passes.

- 2026-11-02: **gf_value size review** (coordinator's question after C5c). (1) gf_value
  (200 bytes) never crosses by value: every slot, callback and record takes `gf_value*` or
  `const gf_value*`; no record embeds one (gf_event_field holds a pointer); abi_audit's
  by-value rule (records over 16 bytes go by pointer) enforces it and reports 0 findings.
  (2) Decision: the font stays inline. gf_value is a record of every kind's field, not a
  union, so it was already 152 bytes before fonts (rectangle 32 and insets 32 are the
  largest fields); the font adds 32 + elements/count 16. A font pointer would save 24 bytes
  but needs caller-supplied or library-owned storage on output (get, element_at, event
  fields), a second lifetime rule for one kind. Size does not affect the calling
  convention, since it is always by pointer. A union-shaped gf_value (about 40 bytes) would
  be the real lever; not taken: C# marshals overlapping fields poorly and struct_size
  growth keeps old fields at fixed offsets. Revisit only if a caller keeps large gf_value
  arrays (collection input is the one case; 4096 x 200 bytes = 800 KiB worst case).

- 2026-11-02: **C5d gate in place**, claude/c-abi-c5 cd41db30. Reading of the gate: the
  facade (generated/facade-v1) reaches native code only through gf_api_v0 and the
  gf_windows_paint_endpoint_*_v1 exports, so every one of the 1104 required rows of
  surface-manifest-v1.json resolves through those 98 entries; the gate maps the 98.
  manifests/abi/compat-map.json (target slot:/property:/interaction:, or pending <family>,
  or deferred <reason>); tools/abi_compat_map.py checks (ctest gui_forms_abi_compat_map),
  --strict fails while pending. Today 38 carried, 60 pending: diagnostics 2, field_editing 7,
  input 4 (pointer capture, per-control cursor), layout 5, pixel_surfaces 2, popups 4
  (tooltips, attach/detach), property_grid 9, range_and_state 4 (min/max, three-state
  check), scroll 7, windows_compatibility 16 (colours, alignment, appearance, border style,
  12 paint endpoint exports). v0 implementations: src/abi/registry/registry.hpp (adapter
  controls in src/abi/control_adapters: RasterControl, FieldControl, ...).
  **Asked the coordinator** (options): state-like entries (range min/max, check state,
  cursor, auto-scroll settings, colours, text alignment, button appearance, panel border
  style; about 22) become schema properties declared by the owning C++ controls (other
  tracks own them), with slots only for operations (about 38: field editing, scroll state
  and into-view, layout transactions, pointer capture, absolute bounds, popups and
  tooltips, property grid, pixel surfaces, diagnostics) in new family tables 11-15, and the
  paint endpoint in a Windows extension table (id >= 0x100, capability-reported).
  Recommended; alternative is slots for everything.

- 2026-11-02: **C5d families** (coordinator: option 1; colours via T4-1 atoms; go-ahead to
  declare properties in owning controls; tell before touching W's files). Commits on
  claude/c-abi-c5: 10daa665 properties (Control cursor; RangeControl minimum/maximum; CheckBox
  check_state; ButtonBase text_alignment; Button visual_style, flat_border_width; Label
  alignment, vertical_alignment; Panel border_style; ScrollableControl auto_scroll,
  auto_scroll_margin, auto_scroll_min_size, auto_scroll_position; PropertyGrid property_sort;
  EnumerationPropertyGetter/Setter adapters; no default where a derived type differs;
  schema exported through the ABI (gf_abi_schema_export writes it) and copied to
  manifests/control-schema.json; binding_tests TextBox descriptor count 23 -> 29).
  82030d29 tables 11 CONTROLS, 12 TEXT_EDITING, 13 POPUPS, 14 PROPERTY_GRID, themes.load_pixels
  (slot 12); records gf_layout_transaction, gf_scroll_axis, gf_scroll_snapshot (0.x names
  collide); kit part gf_conformance_families.c. Gates: abi|c_api|shadow_frames 22/22 plain and
  sanitized; touched control tests 56/56 (binding, panels, range, scrollable, text boxes,
  inspection, property) plain and sanitized; audit 0 (52 records); docs current; style,
  lifetime 0; state machines rc 0; installed consumer passes. Schema regenerated touches
  generated rows of W's types (data_grid_view gains cursor) but none of W's sources.
  Compat map: 80 carried, 3 deferred (last_host_trace, callback_fault_count,
  field_position_from_point: TextBox has no public hit test), 15 pending awaiting decisions:
  colours (T4-1 atoms exist only on PartPaint and DataGridView; stock controls expose no
  authored colours), custom controls (property_object_define/notify: caller-defined
  properties on a proxy control, the C form of subclassing), the Windows paint endpoint (12
  exports; its 0.x target is the ABI-only RasterControl adapter).

- 2026-11-02: **owner approved all three** (coordinator relay): (1) colours: T4 adds authored
  background/foreground to Control; coordinator tells me when T4's branch is ready, then I
  declare the two properties. (2) custom-control family, mirroring C++ (subclass Control,
  override paint, declare properties; tell the coordinator before adding any new C++
  mechanism). (3) Windows paint endpoint targets the custom-control surface, in a
  Windows-only table id >= 0x100 reported as a host capability. control-schema.json
  conflicts with W are regenerated at fold time.
  **Design in progress (C5d custom controls):** C++ pieces exist: Control::define_bindable_property
  (protected, for subclasses), on_paint(Painter&, Rect), public invalidate, LiveSurface
  (include/gui_forms/live_surface: create(description), try_acquire_write -> lease.pixels(),
  publish(damage)), Painter::draw_live_surface, host::WindowsCompatibilityPaintEndpoint
  (include/gui_forms/platform/windows_compatibility_paint_endpoint: acquire(w,h),
  live_surface(), device_context(), configure, submit_bgra32_premultiplied, touch,
  drain_now, snapshot, begin/end_device_context_write, publish_device_context, release).
  0.x references: src/abi/control_adapters/raster_control (live surface paint path),
  abi_property_object_control (foreign property callbacks), src/abi/paint_endpoint.
  Plan: GF_TABLE_CUSTOM_CONTROLS 15: create(spec, stable_id, *object) where spec names a
  type name, properties (name, kind, flags, category, description, default gf_value) and
  one callback record (context; paint(context, object, gf_paint_target*) filling a lent BGRA
  premultiplied buffer of the control's pixel size; get_property / set_property by name
  returning gf_status; release); notify_changed(object, name); invalidate(object, rect or
  null). C++ side: AbiCustomControl : ScrollableControl (as 0.x RasterControl, dormant
  scrolling) defining each property with define_bindable_property, owning a LiveSurface;
  on_paint leases, calls paint, publishes, draws. Then the Windows table 0x100 over
  WindowsCompatibilityPaintEndpoint bound to a custom control's surface.

- 2026-11-02: **C5d custom controls done**, claude/c-abi-c5 at 2baea2c5 (on 82030d29).
  GF_TABLE_CUSTOM_CONTROLS 15: create(spec, stable_id, *object), notify_changed(object,
  index); controls.invalidate (slot 10, rect or null). Spec: base (SCROLLABLE_CONTROL 0,
  USER_CONTROL 1), flags (FOCUSABLE), property_size stride, gf_custom_property[] (name in any
  spelling, category, description, kind other than NULL/OBJECT/COLLECTION, flags READ_ONLY /
  COMBINABLE / CHANGE_NOTIFICATIONS, default gf_value, enumeration + choices as create_with),
  gf_custom_control_callbacks {context, paint, get_property, set_property, release}.
  C++: src/abi/stable/custom/abi_custom_control.{hpp,cpp}: CustomControl<Base> (explicit
  instantiations for ScrollableControl and UserControl; schema type = base's) owns a
  CustomControlState (callbacks, definitions, one Event<> per notifying property, LiveSurface).
  Properties are defined with define_bindable_property (no new C++ mechanism); a base's name
  is refused (property_descriptor), duplicates refused. on_paint: base paint, then lease the
  surface (preserve unless size changed), call paint with gf_paint_target (pixels, scale =
  window scale, damage), publish, draw_live_surface device-aligned. Note: Window records a
  control's display chunk whole, so on_paint's damage is the whole control in practice.
  release is armed only after construction (refused create calls nothing), runs once at
  on_dispose or destruction. read_enumeration moved to schema/abi_create_with (shared).
  Tests: kit part gf_conformance_custom.c; C++ test tests/abi/custom/gf_abi_custom_paint.cpp
  (ctest gui_forms_abi_custom_paint, window at scale 2). Gates: abi|c_api|shadow_frames 23/23
  plain and sanitized; audit 0 (57 records); docs current; style 0; lifetime 0; state
  machines rc 0; installed consumer passes. Compat map: property_object_define ->
  custom_controls.create, notify_changed -> custom_controls.notify_changed; pending now 13
  (colours 1, windows_compatibility 12).

- 2026-11-02: **C5 done.** Stack rebased onto T4's claude/t4-control-colours a74ac740 (itself
  on main 17f94617, checkpoint 11; main carries C1-C4 squashed as c5e034ea). claude/c-abi-c5 at
  f2107b9f, 12 commits over a74ac740: 3bf70bd2 C5a events, e39cf42c C5b create_with,
  139c6305 shadow frames, 7e37898c windows.create, d1fca05e C5c values, d51a4c70 compat gate,
  c6fa1cc1 properties, 8e249dd2 family tables 11-14, cc9e439a custom controls (15),
  9cff415c Windows paint endpoint (0x100), 539cbf97 bindings (16), f2107b9f back_color /
  fore_color. Windows endpoint: GF_TABLE_WINDOWS_PAINT_ENDPOINT 0x100, built only with the
  Windows host (GF_ABI_HAS_WINDOWS_PAINT_ENDPOINT), reported as
  GF_HOST_CAPABILITY_WINDOWS_PAINT_ENDPOINT (bit 32; win32-dib matrix row); handles cross as
  uint64; acquire binds a custom control's bound surface (CustomControlState::bind_surface,
  presentation wake as 0.x RasterControl); release off the UI thread leaves the last frame.
  Bindings: GF_TABLE_BINDINGS 16 (bind with gf_binding_arguments -> data_bindings().add;
  unbind disposes; read/write_value; binding_source records add/insert/remove/clear,
  set_position/position, current_field/set_current_field); check_value_output and
  write_value_output moved to abi_values. Colours: Control back_color, fore_color nullable
  colour properties (null clears; reset clears). Gates at f2107b9f: abi|c_api|shadow_frames|
  binding|basic_controls|property_grid|inspection|part_painting 29/29 plain after the event
  catalog regeneration (kit + catalog + event fields + custom paint rerun plain and sanitized
  4/4; the sanitized 29 ran 28/29 with the kit failing only on the stale catalog, fixed);
  `tools/abi_compat_map.py --strict` passes (98 entries: 95 carried, 3 deferred); audit 0
  (62 records); schema regenerated through gf_abi_schema_export; docs current; style 0;
  lifetime 0; state machines rc 0; installed consumer passes. Fold note: manifests/control-
  schema.json and docs/controls/generated/schema.md regenerate at fold time (W conflicts).


## C6 file map (branch `claude/c-abi-c6`, on main 855b0f0e = checkpoint 12, which holds C5)
- `ctest -L c_abi`: the label (cmake/CAbi.cmake, end) on the ABI's verification set:
  every gui_forms_abi_* test, the 0.x c_api and registry tests, binding and
  shadow_frames tests. 30 tests here; give folds this label, not a name pattern.
- `tests/abi/msvc/gf_abi_msvc_consumer.cmake` (ctest gui_forms_abi_msvc_consumer): vswhere
  finds vcvars64 at configure; cl /W4 /WX compiles the layout probe (C11), the reference
  consumer (C11) and the C++ wrappers' test (C++20) against gui_forms_abi1.lib; all run.
- `tools/abi_bindings.py csharp|cpp [--check]`: generated from gf_abi.json only.
  csharp -> `bindings/csharp/GuiForms.Interop/Generated/{Aliases,Types,Constants,Exports,Layout}.g.cs`
  (aliases are per assembly: consumers compile Aliases.g.cs too). cpp ->
  `include/gui_forms/c/gf_abi.hpp` (gui_forms::abi::<Name>Table per table, negotiate/adopt,
  one method per slot, UNSUPPORTED_VERSION for an unserved slot; ObjectReference and
  SubscriptionReference from `tools/abi_bindings_owners.hpp.in`). The cpp check reads the
  slot map back from the header (1:1; a seeded swap is reported).
- Hand-written C#: `bindings/csharp/GuiForms.Interop/{NativeLibraryName,Abi,ShadowStack,
  Callbacks,ObjectHandle}.cs`; `bindings/csharp/Directory.Build.props` (GfIntermediateRoot
  keeps obj/ in the build tree). No IsAotCompatible/trim analyzers: they pull ILLink, a
  download.
- Tests: gui_forms_abi_bindings_current, gui_forms_abi_cpp_wrappers_current,
  gui_forms_abi_cpp_wrappers (tests/abi/cpp), gui_forms_abi_csharp
  (tests/abi/csharp/gf_abi_csharp.cmake: dotnet build offline with an empty source, then
  run; present when a .NET 10 SDK is found).
- Deferred from the shadow-frame plan: (1) per-frame named managed dispatch methods
  (GuiForms.Dispatch.Click(Button ok)) belong to stage S's Forms-shaped API; done here:
  (2) catch + snapshot + rethrow, (3) debugger views (ObjectHandleView, ShadowFrame
  display, DebuggerStepThrough thunks).
- CI: the C# test needs a .NET 10 SDK on the macOS/Linux runners (setup-dotnet is a
  download): asked of the coordinator, not done.

- 2026-11-02: **C6 done** on claude/c-abi-c6: de263e43 C6a (label, MSVC consumer, C#
  interop), 87f07802 C6b (C++ wrappers), dd4d3801 C6c (debugger views). Gates: c_abi set
  30/30 plain and 30/30 ASan+UBSan (the C# test loads the sanitized DLL too); style 0;
  lifetime 0; state machines rc 0; library docs regenerated (library.c-abi.rst gains
  gf_abi.hpp); installed consumer passes.

- 2026-11-02: **C7 done** on claude/c-abi-c6 at 7b66e3c1: manifests/abi/baseline/
  gf_abi-0.5.json (ABI 1.0 snapshot) named by audit_policy.json (mode report, empty
  ledger); from here an addition needs GF_ABI_MINOR 1 and a break needs a ledger reason.
  Audit 0 findings vs baseline; seeded breaks 28/28, legal 6 unreported; a manual slot
  removal reported unrecorded; native.yml house-style job checks bindings, slot map,
  strict compat map. Record: notes results/2026-11-02-c7-abi-audit/README.md (written,
  not committed; for the coordinator).

- 2026-11-02: **C8, GUI.Forms side, done** on claude/c-abi-c6 at fcb37541 (on 7b66e3c1,
  which goes into checkpoint 13). The kit builds outside the tests block and installs as
  component CAbiConformance (GUIForms::CAbiConformance, lib/libgui_forms_abi_conformance_kit.a,
  include/gui_forms/c/gf_conformance.h; Threads comes from the config's existing
  find_dependency). Installed example examples/reference/c_abi_conformance.c runs it
  --no-window: 695 checks, 0 failed. Consumer note docs/user/c-abi-conformance-kit.md
  (user-guide toctree). TSan: tools/abi_tsan.py (gui-forms-tsan tree, c_abi label minus
  csharp/msvc, TSAN_OPTIONS halt_on_error) + tools/sanitizer/tsan.supp; procedure in
  docs/building/index.md "ThreadSanitizer". Not run (Windows host): first Linux and macOS
  runs are to be recorded under notes results/. Products' runs (PlaySuite, SwiftEdit) and
  .NET on CI runners are with the owner; not touched.
  Gates: c_abi 30/30 plain and sanitized; installed consumers 2/2; docs, style, lifetime,
  state machines clean.

- 2026-11-02: owner rulings (relayed): (1) .NET in CI is option (a): gui_forms_abi_csharp
  uses a preinstalled SDK of major 10 or later and otherwise skips with a notice
  (SKIP_REGULAR_EXPRESSION "gui_forms_abi_csharp: SKIPPED"); nothing is downloaded.
  (2) PlaySuite and SwiftEdit get a full port to the C ABI, planned separately; C8's gate
  no longer includes product kit runs, so **C8 is the GUI.Forms side (fcb37541), done**.
  TSan: both runs (Linux via RunPod, macOS) go to the owner's Mac side after checkpoint 13.
  Window events approved as recommended (1a, 2, 3A, 4, 5a, 6-8); branch
  claude/c-abi-window-events on W's 715fb281 with claude/c-abi-c6 merged in (934260bd).

## Next step
WINDOW EVENTS DONE (2026-11-02): branch claude/c-abi-window-events (W's 715fb281 + merge of
c-abi-c6 934260bd), head 4b532603. Commits: 40cfffc5 the C# test skips without a .NET 10 SDK
(owner ruling (1), ctest SKIP_REGULAR_EXPRESSION, nothing downloaded); ece5cfb5 Window
declares its seven interactions (static Window::append_interactions, WindowHandle forwards,
catalog HOSTED connectors, Theme field deferred); 104d0c72 ABI 1.1: GF_ABI_MINOR 1,
windows.interaction_count/interaction_at/request_help since 1.1 in their own commit
(gf_schema_interaction moved above the windows table; audit 0 findings, additions legal vs
0.5 baseline); 4b532603 tests (kit part gf_conformance_window_events.c, C++ wrapper 1.0-table
skew check, C# windows negotiation + help thunk). Gates: ABI set 31/31 plain and sanitized
(C# ran, not skipped), style, lifetime, docs, install check 2/2; state machines report only a
pre-existing C3 in host_types.hpp.

NEXT: EFFECT PROGRAMS (owner brief 2026-11-02, replaces the Hermes/QuickJS study). Design
note (a) written at desks/coordinator/EFFECT_PROGRAMS_DESIGN.md (projects repo, includes the
setup amendment, random woven stripes); posted to the coordinator, awaiting go before any
build. Then (b) the vocabulary (draft in the note, from PlaySuite read-only at
D:/malkuth/workspaces/games: shared/render r2d/Canvas, shared/particles, shared/ambient) and
(c) the implementation on a new gui_forms branch: plugin (reuse tools/localizable_plugin),
EffectRegistry, first effects progress_stripes/pulse/laser (replacing T4's 13 legacy calls,
coordinate via the coordinator), progress_woven_stripes, progress_bubbles with steam outsets.

EFFECT PROGRAMS M1 COMMITTED 6f7d1b5b (go 2026-11-02; decisions 1-7 all (a)). Branch
claude/effect-programs from origin/main 855b0f0e in /d/gui_forms_w5/gui_forms.
Built (b) vocabulary v1 (note section 4 updated: Painter-grounded, no paths/rotation/additive)
and (c) core + first effects:
- include/gui_forms/engine/effect/effect.hpp (contract, EffectRandom, bounded_range,
  EffectArray, EffectDeclaration, EffectFrame/Setup, concepts), engine/effect_canvas (metered
  canvas), engine/effect_registry (EffectEntry thunks, EffectRegistry, EffectInstance: 60 Hz
  ticks, cap 4 per frame with skips counted, setup on first paint and geometry change).
- src/core/text/localizable/gui_forms_notes.hpp: note reader generalized by type
  (0x47464c01 localizable, 0x47464c02 effect verification); Windows reads the module of the
  run thunk.
- progress_bar_effects.{hpp,cpp}: progress_stripes, progress_pulse, progress_laser (ports;
  legacy paint_progress_* and progress_luminance_band deleted with their loop-ledger
  entries), progress_woven_stripes (setup + seed), progress_bubbles (steam top outset 14).
- ProgressBar hosts them: set_effect(name) (empty = the style's own), set_effect_seed
  (default effect_default_seed(stable id, name)), effect_instance(), visual_outsets();
  still under reduced motion or animation off; the bar's AnimationTimer drives ticks, its
  interval clamped to [1/fps, 4 ticks] while an effect is attached.
- tests/effect_program_tests.cpp (known answers, checksums, hosting); animation_tests pass.
OPEN: the plugin's effect pass (cannot build here: MSYS2 lacks lib/cmake/llvm; needs the
coordinator: download request for mingw-w64-clang-x86_64-llvm, or build/verify on the M4);
CMake/diagnostic/load warnings when unverified; M2: MotionClock ambient path through engine
parts, effect/effect_input/effect_seed properties, ABI schema listing.

EFFECT PROGRAMS M2 (2026-11-02). Branch claude/effect-programs-m2 in /d/gui_forms_w5/gui_forms,
stacked on claude/effect-pass 24f966f1 (the Mac's pass, on 6f7d1b5b). Commits:
- f6beadbf MotionClock control ambient path (enlist from paint only; parked controls repainted
  on resume), Control::on_ambient_frame, ProgressBar effect/effect_seed/effect_input
  properties (schema + docs regenerated), EffectInstance::queue_input (lesser of grant and
  declaration, 8 a frame), bubbles take pointer presses; the marquee alone keeps the timer.
- bc496678 woven stripes in one pass (setup stores drawing order): pass bound run 3,772,
  setup 4,210; budget back to 6,000 (owner ruling: restructure, don't raise).
- 52c6fef1 verification records match name AND state size; effect_load_diagnostics() with a
  one-line stderr diagnostic for the first unverified effect; MetricsSnapshot
  effect_ambient_frames, effects_unverified; API docs and inventory regenerated.
- e5961478 Windows localizable reader walks every loaded module (K32EnumProcessModules), dump
  script prints UTF-8 (both surfaced when the plugin first built on Windows).
- 1624ca03 plugin CMake, owner option 2: LLVMConfig.cmake + Clang headers + libclang-cpp and
  libLLVM by name; no ClangTargets.cmake; GUI_FORMS_LLVM_CMAKE_DIR. Mac to re-check macOS.
Verified: plain and sanitize trees 43/43 (15 effect_pass, localizable plugin and dump,
effect_program, progress, range_control, animation, motion, metrics, text); house style,
state machines (--changed-from origin/main), loop branches on the touched areas.
LLVM INSTALL RECORD (owner approved): pacman -U --noconfirm
https://repo.msys2.org/mingw/clang64/mingw-w64-clang-x86_64-llvm-22.1.8-3-any.pkg.tar.zst,
signature checked (SigLevel Required), SHA-256
186879878581862f71d29f79f13dd102af6f6722284406b509ac2528896b9cc3. Package list 198 -> 199,
only that package added (/d/gui_forms_w5/dev/c_abi/pacman_before.txt, pacman_after.txt,
pacman_install.log). clang-tools-extra NOT installed and not needed.
Prune (design note section 3, by host): Windows needs only -llvm beyond the compiler; it
uses 10 MB of its 420 MiB, but LLVMExports.cmake demands the 388 MB of static libs.
CI Linux: cannot load the plugin; needs llvm-22-dev and libclang-22-dev in
template_harness [linux_packages.toolkit].
NEXT: ABI listing (gf_schema_engine effect_count, gf_schema_table effect_at /
gf_schema_effect, "since 1.1") on a branch joining claude/c-abi-window-events with M2;
manifest "effects" list; both exporters, audit, slot coverage, kit, C++ wrapper, C#.

ABI EFFECT LISTING (2026-11-03, checkpoint 14). Branch claude/c-abi-effects in
/d/gui_forms_w5/gui_forms = M2 (1624ca03) merged with c-abi-window-events (4b532603), then:
- cc451986 C ABI 1.1: gf_schema_table effect_count/effect_at (slots 17, 18, since 1.1), record
  gf_schema_effect (no 1.0 record grows); manifest engine.effects (both exporters agree; no
  verified flag); control_schema.py _effect_findings; kit effects() in gf_conformance_schema.c;
  gf_abi.json, probe, gf_abi.hpp, C# Generated, schema page, library docs regenerated.
- 18918160 display trace: ranges-and-progress digest (M2's effects). Bisected (private desktop):
  1624ca03 differs only there; 715fb281 (W's MessageBox gaps, via the c6 merge) differs on
  states-and-diagnostics, dialogs-and-host-services, typography-and-inspection: W's to rebaseline.
Verified: abi_audit 0.5 baseline 0 findings; touched set (abi|c_api|registry|binding|
shadow_frames|schema|effect|localizable|doc_examples|progress|range_control|animation|motion)
58/58 plain, 58/58 sanitized; schema 115 OK; render, docs, style, lifetime clean; SM rc 0
(advisory C3 in host_types.hpp HostPathDialogResult, from the window-events/W side). Gallery
set on the private desktop 23/24 plain and sanitized (only display trace, the W pages above).
- Schema rewrite now headless: copy .build/native-windows-x64/gui-forms/gf_abi/
  control-schema.abi.json (the ABI export) to manifests/ (dev/c_abi/effects_listing.sh).
- Foreground: Gallery tests run on the private desktop via D:/malkuth/projects/tools/
  run_on_desktop.py -- <cmd> (dev/c_abi/gallery_desktop.sh); only UIA/MSAA/compositor probe use
  foreground_lock.py. Pending foreground tests: none of mine.
- Lesson: never run git checkout from a shell that sourced env.sh (MSYS2 git; brief "Line
  endings"); here autocrlf=true and status stayed clean, but check with the ucrt64 git.

LLVM PRUNE (owner 2026-11-03). Branch claude/llvm-prune 2527822d on 18918160: the plugin loads
no CMake package; GUI_FORMS_LLVM_ROOT (replaces GUI_FORMS_LLVM_CMAKE_DIR) -> llvm-config.h
(version 22), clang/AST/ASTConsumer.h, Windows import libs; -fno-rtti always; stated LLVM
definitions. tools/localizable_plugin/header_closure.py lists/copies the set. Windows set: 134
llvm + 5 llvm-c headers (2.3 MB, the only files from -llvm), 256 clang + 9 clang-c (9.3 MB,
compiler's -clang), 2 import libs (49 MB, compiler's -clang/-llvm-tools). Proven: standalone
plugin build against dev/c_abi/prune/llvm_min only (ninja deps: 0 headers from elsewhere), loads,
records the probe (dev/c_abi/prune/prove_min.sh); toolkit trees with GUI_FORMS_LLVM_ROOT=llvm_min
(dev/c_abi/prune/prove_toolkit.sh). Design note section 3 updated (host-requirements table).
Mac: re-check macOS and run header_closure.py there at the next handoff.

THREADED SUB-SURFACES NOTE (2026-11-03): desks/coordinator/THREADED_SUBSURFACES_DESIGN.md drafted
(SurfaceRoot/SubSurface/SubSurfaceHost; content epochs and pairing; input sequence numbers;
semantic snapshots for accessibility; stop/join with generations; C ABI GF_TABLE_SUBSURFACES since
1.1; M1-M9; nine owner decisions). Sections 7 (present) and 8 (pacing) PROVISIONAL until hot
paths' VSync + Waterfall lab findings arrive via the coordinator.

SUB-SURFACES PLAN (owner accepted decisions 1-9 as (a), 2026-11-03; starts after the instrument
control and once checkpoint 14 folds; each stage its own branch, plain + sanitized + TSan where
available):
- S1 SurfaceRoot extraction: move the control-root part of Window (control table, dirty structure,
  update scopes and flush, layout, focus within, MotionClock, dispatcher queue, thread identity)
  into SurfaceRoot; Window derives; require_ui_thread -> the root's thread. No behaviour change;
  whole suite equal (the checkpoint verify); coordinate with W's property migration (dirty table
  per surface root, study 3.4/3.5).
- S2 SubSurface + SubSurfaceHost with an owned thread, LiveSurface output, start/stop/join with
  generations; publish tagged with the content epoch (LiveSurface publish gains a u64 tag); the
  pairing filter in take_live_surface_presentations. Tests: start/stop stress, stale messages.
- S3 Input and focus forwarding (sequence numbers, capture, tab_out, unhandled_key, declared
  accelerator pre-filter, IME caret in frame metadata).
- S2 also: LiveSurfacePacer + host display timing record (note section 8); hosts publish presented
  generation per surface after each present (section 7).
- S4 Effects and pacing on a sub-surface (decision 7 confirmed); waterfall lab moves to the
  pacer; M3 floor: >=59.5 presents/s, p99 <=1.15 refresh, 0 frames >1.5 refresh, UI blocked 200 ms.
- S5 Accessibility: semantic snapshots per frame, posted actions, UIA fragment root stitching,
  AppKit/AT-SPI readers; M8.
- S6 C ABI GF_TABLE_SUBSURFACES (since 1.1), schema type subsurface_host, kit + C# test.
- S7 DataGridView cell region as the first real sub-surface (with W); M1, M2, M4-M9.

SWEEPSCOPE (instrument control, 2026-11-03). Branch claude/instrument-sweep-scope 3888a8f5 on
claude/llvm-prune 2527822d. SweepScope (include/gui_forms/controls/instruments/sweep_scope/):
read-only RangeControl, caption + sweep_period (0.1-60 s, number property in seconds), effect
scope_sweep (verified by the effect pass; ambient, 30 Hz, cost 400 fixed + 4/px, 16 B state).
built_in_effect_registry() moved to src/controls/built_in_effect_registry/ (6 effects). Registry
sweep_scope (Information display, provisional), schema 116 types, docstring, Gallery specimen on
ranges-and-progress (below the 820 px trace viewport; digest 0xdd9d5756cdabbaae for the status
text). Verified: touched set 54/54 plain and sanitized, gallery_pages + interaction plain and
sanitized on the private desktop; display trace fails only W's three pages (known). Style, life,
docs, schema, registry clean; SM only the known C3 advisory.
- Vocabulary gaps (report): no plot/graticule/trace part roles (used well, divider, indicator,
  activity); no meter semantic role (used progress_bar).
- Capability-mixin property direction deferred: W's migration base does not exist; today's
  define_bindable_property pattern used; migrate with W's study.
- gallery::Instrument (W's custom-control exhibit) kept as is.
- Lesson: GUI_FORMS_GALLERY_TRACE_PRINT=1 makes the display trace pass (prints, never compares);
  ctest hides the print. Print by running the binary, then compare without it.

## cp14 static-check fixes (2026-11-03, after the shutdown)
- claude/instrument-sweep-scope 76cd6a7b (on 3888a8f5): paint census classes EffectCanvas vocabulary
  calls (on a name declared `EffectCanvas&`, drawing members only) as `effect`, never stock
  (coordinator ruling); baseline: progress_bar_effects 0 stock/16 effect, range_control_rendering 12;
  test_paint_census +1 test. effect_registry I05: name printed to at most 256 bytes. Docs:
  api_docs_rst.clean_signature drops GUI_FORMS_EFFECT_VOCABULARY marks (Sphinx could not parse them);
  effect_fraction docstring reworded (no |x|). Checked in w5 and on the fold (22fbaedd already holds
  it): census ok, integrity 0 raised, generate_library_docs --check PASS, Sphinx -W rc 0.
- Lesson: the Bash tool strips backslashes even in quoted heredocs; write patch scripts with Write.

- cp14 #464 Linux (libstdc++) compile fix: claude/cp14-storage-release c0a82da6 on
  origin/checkpoint/2026-11-03-1 f221ba88 (worktree D:/gui_forms_w5/dev/cp14ck): EffectInstance's
  deleter StorageRelease moved to effect_detail (a nested class with an NSDMI is not default
  constructible until the enclosing class completes; libstdc++'s unique_ptr checks it). libc++
  syntax-checked; no libstdc++ headers installed here. No other unique_ptr-of-nested-NSDMI in the
  effect/sweep code. Lesson: deleters and other types used in member defaults at namespace scope.

## S1 SurfaceRoot extraction (started 2026-11-03, after cp14 merged as main feacf6d4)
- Old branches deleted after a containment check (dev/tools/contained.py: share of each branch's added
  lines present in main; all 94-100%, the gaps explained by later rewrites): c-abi-c1..c6, docs-cdomain,
  effects, install-rpath, window-events, effect-programs(-m2), llvm-prune, instrument-sweep-scope,
  cp14-storage-release. Worktrees docsfix and dev/cp14ck removed.
- Build: main fetches stx_auxlib (FetchContent, a download): w5 is configured with
  -DFETCHCONTENT_SOURCE_DIR_GUI_FORMS_STX_AUXLIB=<int's _deps/gui_forms_stx_auxlib-src> (commit
  08c8132d = stx_auxlib.lock).
- Branch claude/subsurface-s1 on origin/main. Phases, each a commit, suite equal:
  S1.1 thread identity (root_thread_, check_access/verify_access, require_ui_thread renamed
       require_root_thread), dispatcher (src/core/surface_root/dispatcher/), metrics -> SurfaceRoot,
       a base of Window (include/gui_forms/surface_root/surface_root.hpp). Window::shutdown_dispatcher
       = abandon deferred input + SurfaceRoot::close_dispatcher.
  S1.2 control table: root_, stable ids, attach/detach/register, find, find_runtime_control; the
       Control back link gains SurfaceRoot* (window() stays Window*, null-able later in a sub-surface).
  S1.3 dirty structure, damage, update scopes and flush, layout passes.
  S1.4 paint pass, focus within, deferred input, MotionClock (MotionClock takes SurfaceRoot&).
  What stays Window: host session, cursor, presentation declaration/receipts, backdrop/compositor,
  live-surface registrations, tooltips, accelerators, command routing, dialogs, help, key bindings.
- Coordinate with W's capabilities migration (dirty table per surface root) before S1.3.
- Hot paths' resize lessons: ask the coordinator before S2/S3 touch resize.

- S1.1 DONE: claude/subsurface-s1 bea32dd0. Focused tests 72/72 plain; sanitized 70/72, the 2 UIA
  input_desktop tests timed out waiting for W's foreground lock (never ran; rerun when free). Docs
  (SurfaceRoot page, manual.json, docstrings), Sphinx -W, house style, lifetime, integrity, state gate ok.
- Finding: the control root's code is inseparable (attach/detach touch dialogs, popups, focus scopes,
  engine parts, motion clock, damage; input routing calls tooltips and accelerators; paint reads the
  backdrop grant). Only the host cluster (cursor, host session binding/key-map seeding, presentation
  declaration, live-surface registrations and idle wake, compositor state) separates cleanly. Asked the
  coordinator for the S1 cut (options in the milestone report). Lesson: never sed member names across
  src/include; HostSession, ApplicationResources, cursor interaction etc. have their own ui_thread_.
- Effects (coordinator, after T4's phase 7 audit): (a) backdrop sampling (EffectDeclaration
  backdrop_margin + one metered blurred-backplane canvas call), (b) clip_part_shape(). Plan for (b):
  Painter::clip_rounded_rect_corners (default: clip_rounded_rect at the smallest radius), RecordingPainter
  op + replay, Skia SkRRect per-corner; EffectInstance::paint takes the part's PartShape (default
  rectangle). For (a): the host passes the Control so the engine can lend Window::engine_backplane at the
  part's window origin; an effect backplane reader list for damage_backplane_readers (non-ambient effects).
- Effect capability check written in desks/coordinator/EFFECT_PROGRAMS_DESIGN.md section 8a (26c5cc5).

- S1.2 DONE (2026-11-04): claude/subsurface-s1 d1f5aa8e (one commit on S1.1 bea32dd0). Option 1 in place:
  SurfaceRoot = Window minus the host cluster; Control::surface_root() set/cleared with window_. Focused tests
  75/75 plain + 75/75 sanitized (regex in dev/tools/s12_verify.sh call; -LE foreground|input_desktop), style,
  lifetime, integrity, state gate, docs, Sphinx -W pass. Head sent to the coordinator.
- Tools: dev/tools/s12_verify.sh '<regex>' [skip-tests] (plain ctest on build.sh tree + ci_build --sanitize +
  all gates; logs dev/s12/; Sphinx via D:/gui_forms_int/.venv-docs python; SM needs HARNESS_PROJECT=
  template_harness/projects/gui_forms.toml). s12_docs_move.py / s12_rst_move.py: move manual.json and
  docstring entries between types by the docs check's stale lists. Build: sh D:/gui_forms_w5/build.sh (a
  control.hpp change rebuilds ~900 steps; run it in the background).
- S1.2 sanitized UIA (uia_provider, uia_text) at d1f5aa8e: 2/2 under foreground_lock.py
  (dev/tools/uia_sanitized.sh builds the two targets explicitly: ci_build --tests with the label left out
  builds nothing). S1.2 + S2.0 are in the cp16 batch.
- S2.0 DONE f5a780d7 (claude/subsurface-s2): as_window() removed; Control::surface_root_ (table) +
  window_ (presenting Window, null in a sub-surface, from SurfaceRoot::presenting_window_ set by Window's
  ctor); attached_surface_root(); on_detaching_from_window(SurfaceRoot&). 82/82 plain + sanitized.
- S2.1 DONE ff059e69 (for cp17): LiveSurfaceWriteLease::publish(damage, content_tag); frame/snapshot
  content_tag (carried through reconfigure); Window::require_live_surface_content + pairing wait of one
  take tick (only when placed before), then unpaired (LiveSurfacePresentation::paired); metrics
  live_pairing_waits / live_unpaired_presentations. live_surface tests' bare asserts -> require()
  (dev/tools/assert_to_require.py); the fold must drop those 2 files from check_test_asserts.py pending.
- BUILD RULES (owner, 2026-11-04): build touched targets, run their tests plain only, inside
  D:/malkuth/projects/tools/build_slot.py -- <cmd>; no sanitized tree of our own (batch fold runs it).
  dev/tools/s12_verify.sh follows them. LANDING PHASE: no new S2 code until the coordinator says.
- S2.2 design (not started): SubSurface : SurfaceRoot built on its own thread (thread-bound
  ApplicationResources of its own); SubSurfaceContent (virtual build_root(), unique_ptr handed to the thread);
  loop = cv wait (dispatch wake handler signals) -> apply newest configure (epoch, size, scale, framebuffer
  created by the host on the UI thread via create_framebuffer and moved across) -> drain_posted_work ->
  layout -> paint (RecordingPainter/PaintFramebuffer) -> copy damage into a LiveSurface lease ->
  publish(damage, epoch) -> notify_presented. Move Window::resize/set_scale's shared part into SurfaceRoot
  (protected). SubSurfaceHost control: start on attach, configure per device-size change (newest wins, one
  per refresh, schedules its own present: resize rule 3), stop = revoke generation, unregister, post stop,
  join. Controls inside see window() null (W's migration to surface_root later).
RESUME POINTER (2026-11-08, Part S macOS groundwork): claude/abi-application restacked da3c769e on b8027bbd (presentation_source
HOST/DEFAULTS documented). Part S of desks/coordinator/APP_WINDOW_RULES_DESIGN.md rewritten on the owner's settled model (one shared
transient surface; Splash = no input: tooltip, splashscreen with a live minimal text/paint path before the Application; Popup = input
through the owner, warm show <= 1208 ns/40 allocs/6.5 KB; K.2 warm tooltip/warm Popup/cold splashscreen); coordinator commits it.
Plan: shared host surface -> Splash (no input) -> Popup input -> Splash core (1.5 U). Branch claude/splash-macos 0b15828a on b8027bbd (verified: both tests plain, core test sanitized, style,
lifetime, docs; gate's only F1 is W's abi_windows.cpp on the base, fixed on claude/abi-application): include/gui_forms/host/transient/host_transient_surfaces.hpp + src/core/host/transient/host_transient_surfaces.cpp
(HostTransientSurfaces: open/present/place/set_alpha/show/hide/close, fixed storage, validation; kinds tooltip/splashscreen/popup);
src/host/macos/application/macos_transient_surfaces.{hpp,mm} (MacTransientSurfaces: borderless nonactivating NSPanel never key/main,
clear, no AppKit shadow, layer-hosting view, two IOSurface buffers per surface reserved at open, contentsRect crop; Splash kinds
ignoresMouseEvents; tooltip help level, splashscreen floating; popup kind NOT offered until input lands); tests
transient_surfaces_tests.cpp + macos_transient_surfaces_tests.mm (window-server readback of own panel, no Application). The
HostCapability::transient_surfaces flag + GF_HOST_CAPABILITY entry deferred to the Splash core (ABI change via coordinator).
NEXT: Popup input (pointer mapping into owner, outside-click monitor), then Splash core.
Superseded pointer: RESUME POINTER (2026-11-08, cp21 Part A C ABI): claude/abi-application 54fec269 on W's claude/app-create b8027bbd: GF_TABLE_APPLICATION
= 18 (create/close/state, gf_application_config, report via gf_theme_report, E601 past 16 packages, close returns the built-in theme;
src/abi/stable/tables/abi_application.cpp, abi_theme_packages.hpp shared with themes, EngineState adopt/release_application,
HandleRegistry::holds_objects; kit part after gf_kit_threads; C# check). Verified abi_|c_api|application plain 33 + sanitized 31,
style, lifetime, docs, gate. Open: presentation_source HOST vs DEFAULTS is not observable in C++ yet (both only set the start;
host reports override) - raised with the coordinator. NEXT: await coordinator.
Superseded pointer: RESUME POINTER (2026-11-08): S7 complete in cp20 (head claude/subsurface-s7-2 32a41a62). C-4 done: claude/abi-tsan-skip-labels
fd6b3088 on main (259d06ab --skip-labels; fd6b3088 builds the whole tree when selected_test_targets answers None; it built nothing
before). TSan tree on the M4 = symlink .build/native-macos-arm64/gui-forms-tsan -> /Volumes/Storage/gui_forms_workers/w5/...;
run on the S7 stack (branch claude/abi-tsan-on-s7): 28/28 c_abi tests pass, instrumented, no reports. Root-at-origin pre-seal audit
reported to the coordinator (no sealed call assumes it; placement in SurfaceRoot flush + apply_client_resize + host a11y roots).
Window/theme API work is paused for w7's design study. NEXT: await coordinator.
Superseded pointer: RESUME POINTER (2026-11-07, S7 part 2 done): stack on P-5 c2501119 (w2 claude/p5-data-grid-view): 02bd494b S6 -> 1119b36f S7.1
-> fd65393b (DataGridViewCells private: no registry exemption exists; registering would publish a paint-only control to
schema/DML/ABI) -> cdb4db1f harness -> claude/subsurface-s7-2: 1e801c01 S7.2 (cells_on_sub_surface, off by default; region =
SubSurfaceHost first child at the bottom of the paint order, hit-test transparent, unfocusable; snapshot sent from on_paint via
redraw_all; owner_draw keeps cells on UI) -> 8f12b5d1 enter fix (also alone on main: claude/subsurface-enter-move 014113c9; the host
forwarded the window's enter as a move, so each entering move reached the sub-surface twice; cp20 sanitized flake at
sub_surface_tests "both pointer moves"; deterministic check added; 50x sanitized repeat PASS) -> 643acff4 grid measurements.
Verified: data_grid_view|sub_surface|property_family|schema|abi_|control_registry plain+sanitized (skip-labels), all targets, style,
lifetime, docs, gate (the last cells-test fix rerun on data_grid_view|sub_surface only).
Measurements (M4 mac, 600 steps, 100k x 8, 800x600): M1 cells UI 0.62 ms vs region 0.065 ms; grid UI 5.39 ms vs grid region 1.94 ms
(medians); M2/M7 redraw_all->publish 1.59 ms; M5 16.6 ms; M6 LiveSurface 5.76 MB + snapshot 336 KB/224 cells; M7 post 0.003 ms;
M9 idle 0.05 ms, busy 30.1 ms; M8 n/a. M4 FINDING: 1 wait then 599 unpaired: under continuous scroll the region is always one
present tick behind (pacer produces at the vblank deadline; pairing_waited only resets on catch-up, so it waits once). Coordinator chose option 2:
HEAD NOW claude/subsurface-s7-2 (see git log; pairing commit on 643acff4) produces a newly configured epoch at once (epoch_due_ in
SubSurfaceLoop::produce); M4 now 0 waits, 0 unpaired, 0 of 600 behind. Per-epoch waits (option 1) not needed. Follow-ups: provider on the sub-surface thread; stopped/pairing_missed schema; run after the seal; host's
"Sub-surface" pane in the grid's semantic tree; M2 input path.
Superseded pointer: RESUME POINTER (2026-11-05, S6 WIP): claude/subsurface-s6 b9ad0926 (WIP, NOT BUILT: M4 disk full) on claude/subsurface-mac 5639ab72.
S6 so far: registry thread domains (src/abi/stable/registry: ThreadDomain, ObjectSlot.domain/orphaned, SubSurfaceDomain
{thread, deferred, posted, dispatcher, drain_pending}; begin_domain/set_domain_wake/end_domain/drain_domain; enter_ui admits
objects/properties/events/dispatch/schema/controls/text_editing/subsurfaces on a sub-surface thread; find_component refuses
other-thread objects (WRONG_THREAD) and orphaned ones (STALE); release defers to the owner's thread; post queues per domain
(DrainDomain via the sub-surface's WindowDispatcher); disconnect refuses a source of another thread). GF_TABLE_SUBSURFACES 17
(since 1.1; src/abi/stable/tables/abi_subsurfaces.cpp): set_content(host, gf_subsurface_content{struct_size, opaque, build})
/ set_root(root) inside build / state / content_epoch / failure. AbiSubSurfaceContent: begin_domain in build_root,
set_domain_wake in the new SubSurfaceContent::root_started(dispatcher) hook (core), end_domain in its destructor (sub thread).
SubSurfaceHost::failure() added. Kit part tests/abi/conformance/gf_conformance_subsurfaces.c (window flag); C# check
(no window) in tests/abi/csharp Program.cs subsurfaces(). Manifest/probe/C++ and C# bindings/docs regenerated locally
(abi_audit check 0 findings, coverage 133/133, docs --check PASS, style + lifetime clean, all run on the laptop).
Deferred: run (adopting the caller's thread), schema interactions stopped/pairing_missed, registry record finalisation.
NEXT: when the M4 has space: build all, run gui_forms_abi_conformance|abi_|sub_surface plain + sanitized (M4WS_WAIT_IDLE),
SM gate; fix; squash WIP into "C ABI: threaded sub-surfaces (S6)". C# test unverified (no dotnet on the M4: skipped).
Superseded pointer: RESUME POINTER (2026-11-05, rebased on main 4ab9a107 = cp18): claude/subsurface-s3.3 d5da0adf (S3.3 + cp18 setter migration
in sub_surface_tests folded in) -> d9084113 (S4) -> claude/subsurface-s4 1123be6d (S4.6) -> 43e4a75e (test switch defaults;
the dangling-text fixes were already in cp18) -> claude/subsurface-mac 5639ab72 (macOS IME caret + presented reports).
Verified at 5639ab72: 91/91 plain (sub_surface|tool_tip|tooltip|message_box|file_dialog|color_dialog|semantic_gallery|
window_presentation|core|theme_engine|motion|effect_pass|effect_registry|live_surface|theme_media|backdrop|theme_user|caret|
macos_) and all targets (ninja -k 0). NOT verified: sanitized + style/lifetime/docs/gate: M4 disk full (116 MiB free),
reported to the coordinator. Rerun: sh .tmp_mac_verify.sh "'<same regex>'" skip-plain.
Conflicts met: cp18 renamed in-surface popup -> overlay (OverlayToken, open_overlay, OverlayAttachment, overlays_,
(*a).overlay()); Control::tool_tip_text is a property cell (.read(), = value); setters removed (visible = false).
NEXT: S6 = registry per-object thread domains (UI thread or a sub-surface thread), GF_TABLE_SUBSURFACES (since 1.1),
schema type subsurface_host/registry finalisation, kit + C# (no dotnet on the M4: C# unverified locally).
Superseded pointer: RESUME POINTER (2026-11-05, Mac items done): claude/subsurface-mac 81be95fe -> 81b74442 (defect fixes: dangling PartPaint::text
in CorrespondenceView/PropertyList found by ASan in the macOS accessibility tests; macos_message_dialog_key_tests switch defaults,
SM gate F1) -> 356a9da4 (S4.6). 81be95fe = macOS IME caret + presented-frame reports (details below). Verified on the M4: 19/19
plain and sanitized (macos_|correspondence|property_list|gallery_display_trace|gallery_pages), style, lifetime, SM gate.
Windows: unverified (UIA/MSAA reruns, IME positioning), CI or a later Windows host. NEXT: await coordinator; rebase onto
origin/main when cp18 lands (check cp18's own fix of macos_message_dialog_key_tests for a conflict); S6 registry finalisation.
Superseded pointer: RESUME POINTER (2026-11-05, Mac resume): workspace now ~/Developer/gui_forms_workers/w5/gui_forms (laptop; builds on the M4
via bin/m4run w5). Local branches restored from backup/: claude/subsurface-s4 356a9da4 (S4.6 on S4 f2e02872 on S3.3 6211b4eb);
host-anatomy 575b580e still only as backup/claude/subsurface-host-anatomy. New branch claude/subsurface-mac on 356a9da4 (Mac
handoff items): (1) macOS firstRectForCharacterRange reads Window::text_input_caret (test macos_text_input_caret_tests, new);
(2) macOS host reports presented live frames: drawRect keeps per-frame generations (_liveFrameGenerations) and calls
LiveSurface::notify_presented once per shown surface with the CADisplayLink's timestamp/duration as LiveDisplayTiming
(display_phased; source none before the first tick) -> the sub-surface pacer is phased to the display on macOS; display link
test checks presentation_record in active phases. TSan on macOS: the coordinator's C-4 pass (tools/abi_tsan.py), not ours.
Windows-only (UIA/MSAA reruns, Windows IME positioning): unverified, CI or a later Windows host.
Verify: M4WS_WAIT_IDLE=180 bin/m4run w5 -- sh ../dev/tools/mac_verify.sh "'<regex>'" [skip-sanitize] (plain, sanitize,
style, lifetime, SM gate; logs w5/dev/logs/mac/). Linux/X11 host does not report presented frames yet (pacer deadline fallback).
Superseded pointer: RESUME POINTER (2026-11-05, S4.6 done): claude/subsurface-s4 356a9da4 = S4.6 on S4 f2e02872 (D:/gui_forms_w5).
S4.6: SurfaceRoot::raise_alert/withdraw_alert (AlertRequest; outcome.choice), detail::AlertRoute: WindowAlerts
(src/core/window/alerts: host message dialog, one at a time, posted, liveness guard), SubSurfaceAlertRequester
(sub_surface_alerts.*), host receive_alert/relay_alert/AlertRelay; take_dialog_request_id moved to core
(host_services.cpp); posting functors in sub_surface_request_posts.hpp. Tests window_shows_alerts,
sub_surface_forwards_alerts (sub_surface_tests links gui_forms_host_headless). Verified 47/47 plain
(sub_surface|tooltip|message_box|file_dialog|color_dialog|semantic_gallery|window_presentation|core), sub_surface x5,
gates, docs --check, clean Sphinx 0 (dev/s2/run46d.txt). Design 15.1/15.2 updated.
cp18 extras on claude/subsurface-host-anatomy (base ee26abc4): f262c95b anatomy + 575b580e semantic node (pane, focus).
NEXT: await coordinator; rebase onto origin/main when cp18 lands; S6 registry finalisation.
RESUME POINTER (2026-11-05, S4 done): claude/subsurface-s4 f2e02872 on S3.3 6211b4eb (D:/gui_forms_w5).
S4 = section 14 gaps (design note "S4 outcome"): root cause was Window-only lookups (attached_window null in a
sub-surface) in MotionClock, MotionValue, ThemeEngine (plan, window_media_facts(const SurfaceRoot&), private helpers),
PartMotion, theme.cpp recipe facts, EffectInstance::sample_backdrop -> attached_surface_root. Backdrop samples the
sub-surface's own backplane (no cross-surface). Loop polls frame schedule at LiveSurfacePacer::display_time (new).
Layered user theme -> SubSurfaceThread ctor param (ApplicationResources::user_theme_ptr new). Tests
ambient_effect_ticks_off_the_ui_thread, user_theme_and_engine_input_reach_the_sub_surface.
Verified: 33/33 plain (sub_surface|theme_engine|motion|effect_pass|effect_registry|live_surface|theme_media|backdrop|
theme_user|tooltip), sub_surface x5, style/lifetime/integrity/SM, docs --check, clean Sphinx 0 (dev/s2/run4b.txt).
Pre-existing, not ours: gui_forms_effect_program_tests dark-scheme colour roles fails on base (reported).
Also done (coordinator, cp18 unblock): claude/subsurface-host-anatomy f262c95b on s3 ee26abc4: SubSurfaceHost T1
anatomy (region/content/focus), registry record gallery_page live-surfaces, Gallery group, policy admits
SubSurfaceContent; schema/stock anatomies/catalogs regenerated. Lesson: git add -u misses new generated pages.
NEXT: S4.6 raise_alert on the SurfaceRoot request API (Window: native popup or modal overlay; SubSurface: forwarded).
Rebase onto origin/main when coordinator says cp18 landed.
S6 TODO (coordinator 2026-11-05): finalise SubSurfaceHost control registry record (standby provisional 2ae6706c: category Drawing, no Gallery page): category, Gallery presence, schema properties.
RESUME POINTER (2026-11-05, S3.3 done): claude/subsurface-s3.3 6211b4eb on s3 ee26abc4 (D:/gui_forms_w5).
S3.3 = design section 15 (rewritten, owner final): NO popup refusal anywhere; native popups are Window-only (hot paths).
SurfaceRoot::request_tool_tip(owner, ToolTipRequest, weak SurfaceRequestAnswer)/withdraw_tool_tip_request; per-root
detail::ToolTipRoute (Window: ToolTipPresenter; SubSurface: SubSurfaceToolTipRequester, src/core/surface_root/
sub_surface/sub_surface_tool_tips.*); subject resolution shared (tool_tip_subjects.*); Control::tool_tip_items (new
virtual); SubSurfaceRequest channel + host side (stale by epoch/input sequence, declined by switches, offered as
host tooltip parts "sub-surface-request.<id>"). Fixed latent null deref (tooltip API inside a sub-surface).
Verified: 8/8 plain (tooltip|ribbon_tool_tip|sub_surface|core|semantic|focus_scope|focus_region|live_surface|
window_presentation), then tooltip+sub_surface after the stale fix, sub_surface x5, style, lifetime, integrity, SM,
docs --check, Sphinx 0 warnings (dev/s2/run33b.txt, run33c.txt). Scripts dev/tools/s33_*.py.
Noticed (not ours): -Wdangling-assignment-gsl in correspondence_view.cpp:732/738 and property_list.cpp:1062.
NEXT: S4.1-S4.5 (section 14 gaps), then S4.6 raise_alert on the same API (Window: native popup or modal overlay).
Superseded pointer: RESUME POINTER (2026-11-05, after restart): owner: sub-surfaces never show anything beyond their region; no native
popups, no escaping overlays (tooltips); they ASK the parent through a request channel (design section 15, written).
Order now: S3.3 (request channel + tooltips, ~S3.1 size; branch claude/subsurface-s3.3 on s3 ee26abc4), then S4.1-S4.5
(section 14 gaps), then S4.6 (alerts via the channel). NO refusal anywhere (owner, final): native popups
are Window-only; the request API lives on SurfaceRoot (request_tool_tip now, raise_alert in S4.6); Window handles,
SubSurface forwards (design section 15, rewritten).
S3.3 also fixes a latent null deref: SurfaceRoot tooltip API (set_tool_tip_text in a sub-surface) hit a null presenter.
Superseded pointer: RESUME POINTER (2026-11-05, S3.2 done, restart stop): claude/subsurface-s3 S3.1 f337f4b4 -> S3.2 ee26abc4 (D:/gui_forms_w5).
Verified at the stop: 10/10 plain (sub_surface|focus|caret|text_box|pointer|capture), docs regenerated, style,
lifetime, integrity, state-machine gates, docs --check, Sphinx -W (0 warnings) and sub_surface x5 all clean (finished
after the stop report; nothing to rerun; the commands: sh dev/tools/s2_verify.sh x skip-tests; ctest --repeat
until-fail:5 inside build_slot). S3.2: Control::text_input_caret() virtual (local box, default none);
SurfaceRoot::text_input_caret() (window coords: the focused control's caret, or a 1-unit box at its left edge); the
sub-surface loop publishes its caret to the channel at the end of each turn (SubSurfaceThread::text_input_caret) and
SubSurfaceHost overrides text_input_caret with it, so the window's caret includes the host offset with no
synchronous call. Hosts do not read it yet: the macOS host's firstRectForCharacterRange still uses the focused
control's left edge (switch it to Window::text_input_caret on the Mac handoff); the Windows host has no IME
positioning (ImmSetCompositionWindow / candidate window: a host item); TextBox/RichTextBox do not override
text_input_caret yet (controls item). Capture: the host takes the window's capture for a non-primary press it
forwards (pressed_buttons_, took_capture_) and releases it on the last release; primary stays the window's.
NEXT: S4 (the section 14 gaps).
Popups from a sub-surface (for hot paths' native popups, coordinator 2026-11-05): a SubSurface has no host session,
so a native popup opened inside one needs (1) the owner native window = the host control's window; (2) anchor
coordinates translated sub-surface -> window (host's absolute origin + local) -> screen on the parent thread (the
origin is known only there; it could also travel in each configuration); (3) a thread rule: the native popup lives
on the parent UI thread, the popup's content tree on the sub-surface thread. Options: (a) content stays on the
sub-surface thread, rendered into its own LiveSurface composed by the native popup (a mini SubSurfaceHost, input
forwarded the same way); (b) content built on the parent thread by a factory; (c) for 0.5, sub-surface popups stay
in-region overlays (today's behaviour, clipped to the region), (a) after. Recommendation: (c) for 0.5, then (a).
Superseded pointer: RESUME POINTER (2026-11-05, S3.1 input done): claude/subsurface-s3 f337f4b4 on pacer 18eefd16 (stack: s2.1 e537614f -> s2.2
5d144a4f -> s2.3 233f205f -> pacer 18eefd16 -> s3). S3.1 = design section 4 minus IME: SubSurfaceInput {kind pointer|
key|text|focus_enter|focus_leave}, SubSurfaceThread::post_input (sequence 1,2,..; 0 after stop) queues in the channel
(vector swapped whole into the loop; dispatched after the newest configuration, before flush); unhandled keys go
back through set_key_return(parent dispatcher, weak SubSurfaceKeyReturn) as DeliverReturnedKey posts. SurfaceRoot:
focus_wraps_ (false in a sub-surface: Tab at the last stop is unhandled -> returned), last_focus_traversal_ (Tab /
Shift+Tab / none, set by move_focus), route_key tail split into finish_unhandled_key, private resume_unhandled_key
(bubble from the host's parent, accelerators, dialog keys, Tab/F6); friend SubSurfaceHost. Host: focusable tab stop,
on_pointer/on_key/on_text_input forward (positions via point_from_window, enter -> move) and mark handled; keyboard
focus = host focused in an active window -> focus_enter(first|last|restore) / focus_leave (sub-surface active_ follows,
starts inactive); KeyReturn nested target revoked in stop_thread; returned_keys_discarded() when focus moved away.
Stats inputs_received/dispatched/last_input_sequence/keys_returned. Docs: painting-and-damage "Sub-surface input".
Verified: 18/18 plain (sub_surface|live_surface|focus|key|accelerator|dialog|keyboard|traversal|mnemonic), gates.
NEXT: S3.2 IME caret in frame metadata (design section 4) and non-primary-button capture, then S4 (section 14 gaps).
Superseded pointer: RESUME POINTER (2026-11-05, pacer done): claude/subsurface-pacer 18eefd16 on S2.3 233f205f (stack: s2.1 e537614f ->
s2.2 5d144a4f -> s2.3 233f205f -> pacer 18eefd16). Pacer: LiveSurface::notify_presented(generation, LiveDisplayTiming)
/ presentation_record() / connect_presented_wake; LiveSurfacePacer (include/gui_forms/live_surface/pacer/): plan(now)
non-blocking, wait_for_frame(stop_token), idle()/arm(); frame when last frame shown (or a tick passed), deadline at
1.5 refresh (missed_frames), pending(wake_at). Windows host: arm_live_clock stores the phase (live_timing_vblank_/
period_/source_; no interface change to hot paths' clock), present_live_frame_sequence reports each shown surface
frame (transactional-DIB dev path does not report yet). macOS/Linux hosts do not report yet (pacer falls back to its
deadline there): Mac handoff. Sub-surface paces each paint (presented wake + pacing_wake_), stats deadline_wakes/
missed_frames; bug found and fixed: a new-epoch whole frame must not publish before its paint. Verified 10/10 plain
(sub_surface|live_surface|window_presentation|windows_host|vsync|resize) + all gates; sub_surface + pacer tests x10.
Effects thread-safety review written: design section 14 (gaps: backdrop sampling, display-time ticks, user theme
rules, reduced-motion test, input via S3). NEXT: S3 input and focus forwarding (design section 4), then S4.
Superseded pointer: RESUME POINTER (2026-11-05, S2.3 done): stack on cp16: claude/subsurface-s2.1 e537614f (S2.1, cp17 candidate) ->
claude/subsurface-s2.2 5d144a4f (S2.2) -> claude/subsurface-s2.3 233f205f (S2.3). When S2.1 folds with the assert
sweep 7aa159bd: drop the two live_surface entries from tools/check_test_asserts.py pending.
S2.2: SubSurface/SubSurfaceThread/SubSurfaceContent/SubSurfaceConfiguration (include/gui_forms/surface_root/
sub_surface/sub_surface.hpp), SubSurfaceHost (sub_surface_host.hpp; surface_root area, registry/schema type in S6),
src/core/surface_root/sub_surface/; SurfaceRoot::resize_surface/set_surface_scale/release_tree. 34/34 plain + gates.
S2.3: ApplicationResources tracks SurfaceRoot* (windows_ vector), SubSurface attaches to its own resources;
configuration carries theme (application theme snapshot) + PresentationSettings; host follows theme_changed/
presentation_changed (new epoch, framebuffer only on device-size change); SubSurfaceStatistics::loop_turns + idle
test. 29/29 plain + gates (docs regenerated, Sphinx ok); after loop_turns: sub_surface x6, style, docs, Sphinx.
Timing note: sub_surface_tests takes 8-12 s with the machine at 100% CPU (other builds), 1.9 s at high priority;
thread start/stop latency under load, not a spin (idle test).
NEXT: the LiveSurfacePacer + host display timing record (design section 8; section 7 "hosts publish presented
generation per surface after each present"): LiveSurface::pacer(), wait_for_frame(StopToken)->{outcome frame|
deadline|stopped, frame_time, presented_generation, missed_frames}, idle()/armed(); seqlock timing record written by
the host present tick (refresh_period, last_vblank, presented_generation, timing_source); Windows host clock is hot
paths' df04b70c work (coordinate before touching src/host/windows). Then S3 input forwarding, S4 effects.
Effects on a sub-surface not yet audited for threads (S4).
Verify: sh dev/tools/s2_verify.sh '<regex>' [skip-tests|skip-gates] (ctest names filtered to real ninja targets);
build only inside build_slot. LAPSE (2026-11-05, S2.3): one rebuild of gui_forms_sub_surface_tests ran outside
build_slot; never again. Coordinator: pacer builds on hot paths' df04b70c clock (threadpool wait on the phased
clock); clock interface changes go to the coordinator first. Before S4: thread-safety review for effects into the
design. TSan for sub_surface/live_surface on the next Mac handoff.
Superseded pointer: RESUME POINTER (2026-11-04, landing phase): S2.0 f5a780d7 (cp16) and S2.1 ff059e69 (cp17) committed on
claude/subsurface-s2; nothing pending. Next, when allowed: S2.2 per the design above, then the pacer.
Superseded pointer: RESUME POINTER (2026-11-04): S1.2 delivered (d1f5aa8e). Next S2: read THREADED_SUBSURFACES_DESIGN.md S2; asked
the coordinator for hot paths' resize lessons. Pending: the 2 sanitized UIA tests (foreground lock).
Superseded pointer: RESUME POINTER (2026-11-03, reboot stop). Effect items moved to the new effects worker; none was started
here (only design note 8a, projects 26c5cc5). S1.2 (coordinator option 1: SurfaceRoot = today's Window minus
the host cluster, in place) is WIP commit cc32d8c7 on claude/subsurface-s1 (on S1.1 bea32dd0) in D:/gui_forms_w5.
NOT building, NOTHING verified. Two compile errors left: (1) ~SurfaceRoot() = default needs ToolTipPresenter
complete: it was moved out of surface_root.cpp and placed before SurfaceRoot::shutdown_dispatcher in
src/core/window/lifecycle/window_lifecycle.cpp, which includes tool_tip_presenter.hpp; check it now resolves;
(2) src/core/window/update_scope/update_scope.cpp:36 `Window* = SurfaceRoot*`: change the local to
SurfaceRoot* (UpdateScope now holds SurfaceRoot*). Then: fix any remaining `*this`-as-Window& sites in
SurfaceRoot members with as_window(); add Control::surface_root() (SurfaceRoot* set wherever window_ is set);
move Window manual.json/docstring entries to SurfaceRoot until tools/generate_library_docs.py --check passes;
verify (focused tests plain + sanitize.sh, avoid UIA/MSAA regexes; house style, lifetime, integrity, Sphinx,
state gate --changed-from origin/main); squash the WIP into one mechanical S1.2 commit; report the head to the
coordinator at once. Scripts: D:/gui_forms_w5/dev/tools/s12_split.py (already applied), s12_qualify.py (applied).
Also pending: rerun the 2 sanitized UIA tests when the foreground lock is free. Then S2 (ask coordinator for hot
paths' resize lessons first).

### Coordinator rulings for S6 (2026-11-05)
- Caller-thread `run` slot: left out of S6 (decision 9 allows, does not require); revisit after the 1.1 seal.
- Schema interactions `stopped` / `pairing_missed`: left out of S6. FOLLOW-UP: add them to the subsurface_host schema type after S6 lands.
- SubSurfaceHost registry record stays category Drawing, status provisional.
- Order: wait for the coordinator's "space is free" message, then the stack's sanitized pass and checks (skip-plain at 5639ab72), then S6 build/tests/squash as planned.

## RESUME POINTER (2026-11-05, app restart)
- Heads: stack claude/subsurface-mac 5639ab72 (s3.3 d5da0adf, s4 1123be6d) on main 4ab9a107; S6 WIP claude/subsurface-s6 b9ad0926 (on 5639ab72). No uncommitted tracked changes; w5 clone checked out at claude/subsurface-mac.
- Ran at 5639ab72: plain 91/91 and all targets on the M4; style/lifetime/docs --check locally. The sanitized pass + M4 checks (skip-plain) were started after the disk was freed and ABANDONED at the restart (stopped mid-run; the remote side may have completed or left a lock: check `ssh m4mini` for a running ninja/ctest before rerunning).
- Next: cp dev/tools/mac_verify.sh gui_forms/.tmp_mac_verify.sh; `M4WS_WAIT_IDLE=180 bin/m4run w5 -- sh .tmp_mac_verify.sh "'sub_surface|tool_tip|tooltip|message_box|file_dialog|color_dialog|semantic_gallery|window_presentation|core|theme_engine|motion|effect_pass|effect_registry|live_surface|theme_media|backdrop|theme_user|caret|macos_'" skip-plain`; report; then checkout claude/subsurface-s6 and run regex "'abi_|sub_surface|caret|macos_display_link'" mode all; fix, squash WIP into "C ABI: threaded sub-surfaces (S6)", report. M4 has ~10 GB shared: create no new build trees.

## RESUME POINTER (2026-11-05, S6 done; S7 design pending)
- S6 COMMITTED: claude/subsurface-s6 2c87fdb0 on 5639ab72 (stack in cp19; cp19 not yet merged into main 9e669353: rebase S6 onto origin/main once it is). Fix found on the M4: the kit's sub-surface window was 140 px high, under the default minimum_size 150x150 (host refused, INVALID_ARGUMENT); now 160.
- Verified at 2c87fdb0 (macOS arm64): abi_|sub_surface|caret|macos_display_link 26/26 plain and sanitized, all targets, style, lifetime, docs, SM gate. C# not run (no dotnet on the M4). The stack's own sanitized skip-plain pass at 5639ab72 was never rerun (abandoned at the restart); cp19's verify covers it.
- S7 (SS-7, DataGridView cell region as first real sub-surface; M1, M2, M4-M9): DataGridView::on_paint (data_grid_view_parts.cpp:1604) paints well, cells (paint_cell, which runs formatting handlers and returns false if a handler changed the grid), headers, row headers in one pass on the UI thread. Design options sent to the coordinator (see report); waits on W's P-5 and the coordinator's choice.
- Follow-ups: schema interactions stopped/pairing_missed for subsurface_host; `run` caller-thread slot after the seal.

## RESUME POINTER (2026-11-05, S7 part 1 in progress)
- Coordinator ruling: S7 option 1 (snapshot of the visible row window posted in batches; UI thread keeps data, events, editor, headers, scroll bars; redraw-all with a new epoch on scroll/resize/column width; virtual-mode provider on the UI thread; opt-in cells_on_subsurface, off by default). Part 1 now (outside DataGridView's files); wiring into DataGridView's paint/layout after W's P-5 lands (the coordinator sends W's head).
- FOLLOW-UP: virtual-mode provider on the sub-surface thread (decision 3a second half).
- Branch claude/subsurface-s7 on claude/subsurface-s6 2c87fdb0 (uncommitted while the first M4 run goes). Contents:
  - core: SubSurfaceContentUpdate + SubSurfaceContent::apply_update; SubSurfaceConfiguration::update (carried over a coalesced configuration; updates_applied/updates_coalesced statistics); SubSurfaceHost::redraw_all(update) -> epoch (0 while not configured; waits for the first configuration; set_content drops it); every configuration now repaints the root whole (each epoch gets a frame: pairing).
  - include/gui_forms/controls/data_grid_view/data_grid_view_cells.hpp + src/controls/data_grid_view/data_grid_view_cells.cpp (new files in the grid's dirs so the docs page resolves): DataGridViewCellsSnapshot (look, well, cells_extent, cells, shared text + lines buffers), data_grid_view_cells_snapshot_valid, paint_data_grid_view_cell (mirror of DataGridView::paint_cell minus owner draw; at wiring the grid should call it, single source), DataGridViewCells control (anatomy id data_grid_view, not exposed, not focusable), DataGridViewCellsUpdate, DataGridViewCellsContent.
  - tests/data_grid_view_cells_tests.cpp (validation, part order via a text-recording painter, epochs via redraw_all with a text-logging raster); tests/support/fill_raster.hpp (moved from sub_surface_tests).
- Wiring notes for part 2: owner draw keeps cells on the UI thread; the host over the data area must let pointer input reach the grid (input stays on the UI thread); grid publishes cell semantics; M1, M2, M4-M9 measured after wiring.

## RESUME POINTER (2026-11-05, cp19 schema fix + S7 part 1)
- URGENT cp19 fix DONE: claude/abi-child-properties 835c3599 on cp19/trial 0ff5a447 (fetched from w3 as w3/cp19-trial). Schema table slots 19-21 child_property_count / child_property_at / child_choice_at (since 1.1); gf_schema_type unchanged (check_record uses sizeof as the 1.0 size, so growing a record refuses 1.0 callers: never grow an output record). Export test writes child_properties; kit checks grid_panel/button. Verified abi_|schema 22/22 plain + sanitized, coverage 131/131, style, lifetime, docs, gate. For the standby to fold.
- S7 part 1 COMMITTED: claude/subsurface-s7 ed0e93fa on claude/subsurface-s6 2c87fdb0. Verified sub_surface|data_grid_view|abi_conformance 5/5 plain + sanitized, threaded tests x25 per tree, checks, gate. Next: wiring into DataGridView after W's P-5 (coordinator sends W's head); rebase S6/S7 onto main after cp19 merges (S6 will need the schema slots renumbered if both add schema slots: S6 added no schema slots, so no clash expected; GF_TABLE_SUBSURFACES=17 stands).

## RESUME POINTER (2026-11-05, owner pause)
- Nothing uncommitted. Clone on claude/abi-child-properties.
- Finished + verified: claude/abi-child-properties 835c3599 (cp19 schema fix, on cp19/trial 0ff5a447); claude/subsurface-s6 2c87fdb0 (S6); claude/subsurface-s7 ed0e93fa (S7 part 1, on S6). Stack claude/subsurface-mac 5639ab72 is in cp19.
- WIP: none.
- Next: when cp19 merges, rebase S6 and S7 onto origin/main; then S7 part 2 (wire DataGridView: cells_on_subsurface opt-in, snapshot from the visible row window, redraw_all on scroll/resize/column width, the grid's paint_cell calling paint_data_grid_view_cell) after W's P-5 head arrives; then M1, M2, M4-M9.
