# GUI.Forms 0.2 consumption and the provider build method

Date: 2026-10-26. Implements accepted ADR-021 at a new pin; no new decision.
This record separates what was given, observed, measured and left unverified.

## Direction

**GIVEN (owner, 2026-10-26):** this session takes on File Manager, moves it
to the newest dependency pins, and revamps the File Manager build to mirror
the GUI.Forms build enhancements. Rewriting the File Manager interface and
SwiftEdit's picker code onto the new GUI.Forms paradigms is later 1.0
consolidation work, not part of this change.

**GIVEN (owner, 2026-10-26):** Web.Forms is deprecated; a better route rolls
out toward GUI.Forms 1.0, and much of what the File Manager interface (and
SwiftEdit and other downstream consumers) depends on moves forward later
with it. Its generated code is carried here, not repaired.
`../frontend/planning/GUI_FORMS_1_0_CONSOLIDATION_SURVEY.md` records what
that later work must move.

## What changed in the provider since the c90eacf pin

**OBSERVED** in GUI.Forms between `c90eacf3` and the new pin (1,300+ commits;
the public headers gained about 23,000 lines and lost 121):

- The repository moved to `malkuth-software/gui_forms`; GitHub redirects
  the former name, which provider manifests still record.
- One pinned LLVM 22.1.8 per platform, checked at configure and by
  `tools/compiler_identity.py`: the published MSYS2 CLANG64 archive on
  Windows (extracted at `D:\a\_temp\msys64`), Homebrew `llvm@22` plus a
  source-built macOS 14 libc++ runtime on macOS (deployment target 14.0), and
  apt.llvm.org `clang-22` on Linux. `cmake/llvm22.cmake` is passed to the
  toolkit and every consumer.
- `tools/ci_build.py` builds the toolkit in a consumer-compatible layout: the
  workspace is the directory above `gui_forms/`, the tree is
  `.build/native-<platform>/gui-forms/`. It also installs the SDK, checks
  installed notices and Windows DLL imports, and builds and tests the
  installed reference examples.
- Cache archives are schema 2; the macOS archive also carries the runtime
  prefix `.build/toolchain/llvm-22.1.8-macos14`.
- Windows programs and DLLs link `-static`; packages reject any GCC or LLVM
  runtime DLL import. HarfBuzz text is on for Windows.
- `GUIForms::Threading` is a shared library that `Application` loads.
- Native CI adds exact-tree reuse and promotion, cross-branch seeds, tree
  receipts, working-set eviction, main-only cache saves and pruning.
- The package config version is 0.2.
- TextBox keyboard policy (`f333fb12`, `4f0be17f`): Enter and Escape pass on
  to containers and the accept or cancel button unless the box has something
  to accept or cancel, or opts in with `set_accepts_enter` /
  `set_accepts_escape`.

## What File Manager now does

- `tools/build_native.py` runs the pinned provider's `ci_build.py` for the
  toolkit, then configures the frontend with `cmake/llvm22.cmake` through
  the provider's configure record (no needless reconfigure). Its platform
  name follows `ci_build.py`, so both use one build directory.
  `--sanitize` builds `frontend-sanitize` with the provider's sanitizer
  profile against the tested Release SDK.
- `tools/fetch_build_inputs.py` accepts schema 2 and either provider name,
  admits the macOS runtime prefix only on macOS, and reports whether the
  `clang` on `PATH` is the binary that filled the cache.
- The frontend requests GUIForms 0.2 and rejects compilers other than Clang
  22.1.x. Windows executables link `-static` and stage
  `$<TARGET_RUNTIME_DLLS>`; `StageWindowsRuntime.cmake` now fails on any
  import that is neither a system DLL nor a staged GUI.Forms DLL. macOS and
  Linux stage Threading beside Application.
- `tools/package_native.py` checks every top-level Windows binary's imports
  with the provider's rule lists and records them in the receipt; Linux runs
  `ldd` over every packaged GUI.Forms library.
- `tools/export_native_sdks.py` ships `cmake/llvm22.cmake` in each SDK
  archive, plus the macOS runtime as `llvm-runtime/`, and configures its
  relocated consumers with them.
- CI uses the provider's runner images (`macos-15`, `ubuntu-24.04`,
  `windows-2022`), its toolchain restore and package installer, its compiler
  identity, eviction (`ci_cache.py evict`) and pruning, and saves caches only
  on main. House style runs the provider's own spelling and lifetime
  checkers over File Manager's first-party C++. Linux runs the frontend
  under the sanitizers.
- The five File Manager text boxes that subscribe `committed()` or
  `cancelled()` opt in to Enter and Escape: search, inline rename, and the
  picker's path, name-filter and file-name boxes. The tested behaviour is
  preserved; any change to that keyboard policy belongs to the 1.0 refactor.
- The checker found four borrow-returning helpers without a deleted
  rvalue overload; each now has one.

## Measurements

**MEASURED** on Shadow (Windows 11, 8 logical processors). Toolchain: the
provider's published CLANG64 tree at `D:\a\_temp\msys64` (`clang.exe`
SHA-256 `071874172edc…`, the cache manifest's compiler). Cache: the restored
`build-847ed638…` Windows archive. Command:
`tools/build_native.py --jobs 6 --test-jobs 3`.

| Step | Result |
|---|---|
| Toolkit compilation through `ci_build.py` | 760 of 763 cacheable compilations hit (99.6%) |
| Toolkit CTest | 222/222 passed, 114 s |
| Installed reference examples | 11/11 passed |
| Frontend CTest, first run | 13/15: the interaction and transfer tests failed on Escape and Enter during inline folder naming |
| Frontend CTest after the opt-ins | 15/15 passed |
| Package | executable imports `libgui_forms_application.dll` only, which imports `libgui_forms_threading.dll`, which imports system DLLs only; five-second startup on a generated empty root passed |

That run covered GUI.Forms `847ed638`. At the final pin, `2fe26d8d`, after
restoring that revision's Windows cache:

| Step | Result |
|---|---|
| Restore | `fetch_build_inputs.py` reported the local `clang` is the cache's compiler |
| Compilation after the pin move | 345 of 355 hit (97.2%); the misses are the changed provider sources |
| Toolkit CTest | 223/223 passed |
| Installed reference examples | 11/11 passed |
| Frontend CTest | 15/15 passed |
| Package | the import check and five-second startup passed |
| Sanitized frontend (`--sanitize`, ASan and UBSan; Windows has no LSan) | 15/15 passed, no sanitizer report | These are single local observations and are not CI timings.
A hit is not test acceptance: every toolkit and frontend test ran.

## Negative results and corrections

- **OBSERVED:** `platform.machine()` is empty in a shell that does not carry
  `PROCESSOR_ARCHITECTURE`; the earlier mapping raised `KeyError`. File
  Manager now derives the name as `ci_build.py` does.
- **OBSERVED, pre-existing:** `native_build_tests.py`'s smoke-reaping test
  patches the shared `subprocess.Popen`, and Python 3.14's first
  `platform.system()` on Windows ran `ver` through the fixture. The test now
  fills platform's cache first. The committed tooling failed the same way.
- **OBSERVED:** MSYS Git without the checkout's line-ending policy reported
  841 changed paths; with `core.autocrlf=true` it reported the 23 real ones,
  as CI already configures.
- **OBSERVED:** the earlier Windows build tree was compiled by another
  toolchain; it was moved aside intact as
  `.build/native-windows-x64.pre-llvm22-2026-10-26`.

## Stack frames under the sanitizers

**MEASURED** with the provider's guard, `gui_forms/tools/stack_frames.py
check` (limit 262,144 bytes), over the sanitized Windows frontend tree:

| Frame | Function | Disposition |
|---:|---|---|
| 636,760 B | Web.Forms-generated `make_native_form()` | carried: deprecated generator |
| 502,232 B | Web.Forms-generated `make_native_button_state_recipes()` | carried: deprecated generator |
| 265,528 B | `file_manager::checksum_sha256()`: a 256 KiB `std::array` read buffer | fixed: owned heap buffer allocated once before the read loop (house style section 5) |

**OBSERVED:** the two generated frames together overflowed Windows' 1 MB main
thread in `___chkstk_ms` before either large test began
(`file_manager_application_interaction_tests`,
`file_manager_application_transfer_tests`). Linux and macOS give main 8 MB,
so CI's Linux sanitizer job does not reach this. The provider fixed its own
case at the root (GUI.Forms #283, `314a64c3`). Here the generator is being
retired, so `build_native.py --sanitize` reserves an 8 MB main-thread stack
on Windows in the sanitized tree only, which is never packaged. The guard is
not a CI gate yet, because it would fail on the generated frames; it becomes
one when the generator is replaced.

## Findings from the first CI runs (PR #61)

**MEASURED** in CI runs 40459593556 and 40466042854, which exercise macOS and
Linux for the first time at this pin. Windows passed throughout.

- **Linux packaging.** The packaged `libgui_forms_application.so.0` had no
  RUNPATH. A library's own dependencies do not use the executable's RUNPATH,
  so `ldd` of the library alone could not find `libgui_forms_threading.so.0`.
  Packaging now sets `$ORIGIN` on each packaged GUI.Forms library with
  `patchelf`, as GUI.Forms' packager does; CI installs the provider's
  `programs` package profile, which adds `patchelf`.
- **macOS: publishing a read-only directory copy.** A probe on `macos-15`:
  `renamex_np(RENAME_EXCL)` within one parent fails with `EACCES` for a
  directory without owner write and succeeds for a writable one. The copy
  applies the source's read-only mode to its staged directory before
  publication, so publication failed. The reset `main` passed this test on
  `macos-26`, built by Apple's toolchain with a macOS 26 minimum; File
  Manager now declares macOS 14. `publish_stage_no_replace` gives such a
  staged directory owner write for the rename only and restores its exact
  mode through a descriptor held on it, which still names it after the
  rename. It changes only stages File Manager created, and only on macOS;
  user renames and moves are unchanged. A copy published with its mode not
  restored reports `publication_mode_failed` and retains the object.
- **macOS: the Details header check.** The sampled rectangle was correct
  (flipped `GUIFormsView`, 1024×681), but only 9 pixels passed the
  per-channel 0.2/0.25/0.3 cut after AppKit converted the CI display's
  "Apple Virtual" colour space to sRGB (18 in the saved capture's
  unconverted values). GUI.Forms 0.2's header text is a lighter navy than
  before. The check now counts pixels below 0.35 on every channel. In the
  unconverted capture that finds 32 in the header and none on an empty header
  strip or empty list. The test also reports its last rectangle, count and
  capture geometry if that stage times out.
- **Sanitizers (Linux): `alloc-dealloc-mismatch`.** The transfer test's
  allocation fixture replaced the throwing `operator new` and the scalar
  `operator delete`, but not the nothrow forms. libstdc++'s `stable_sort` took
  a nothrow buffer from AddressSanitizer's allocator and returned it through
  the fixture's sized delete, which calls `free`. The fixture now replaces
  every throwing and nothrow form, scalar and array, over one `malloc`/`free`
  pair; nothrow allocation returns null for the armed rejection.
- **The `macos-26` control** first failed before its tests: the runtime in
  the provider cache matches only the provider's compiler and SDK. The
  runtime step now rebuilds on a mismatch, as the manifest tool directs.
- **macOS packaging (run 40470922039).** With the fixes above every macOS
  test passed on both images (232, 11 and 16), and packaging then failed:
  the bundled `libc++.1.0.dylib` names `@rpath/libc++abi.1.dylib` and has no
  LC_RPATH of its own. The closure resolves such a name against the
  executable's rpaths, but it rewrites the executable first and deletes its
  build-tree rpath (the LLVM runtime prefix) before it reaches libc++.
  `mac_closure` now records the executable's rpaths before any rewrite, as
  GUI.Forms' packager reads them from its unmodified executable. With the
  macOS failures explained, the `macos-26` control job is removed.

## Not established

- macOS and Linux were not built locally; CI has not run the new workflow yet.
- Exact-tree reuse and promotion are not mirrored. The provider's
  `ci_reuse.py` and `ci_promote.py` name its own workflow and archives; File
  Manager needs its own equivalent over its own tree, which already binds
  both submodule revisions.
- Linux arm64 is not built: the backend publishes no linux-arm64 bundle.
- No interactive GUI, accessibility or daily-root acceptance.

## Reviewed source scope

House-style review covered the changed Python (`build_native.py`,
`fetch_build_inputs.py`, `package_native.py`, `export_native_sdks.py` and the
two test files), both PowerShell scripts, the CMake changes and the C++ edits
above: explicit types, named execution, failure states, ordering, and
borrow lifetimes. The provider checkers report 0 spelling and 0 lifetime
findings over `frontend/` and the Orchestrator C++ client. The Web.Forms
generated units were not reviewed; they belong to a deprecated generator
(above). Remaining known departures in first-party code, deferred to the 1.0
refactor: `std::bind_front` subscriptions
(139 in `frontend/src`) where a non-owning `gui_forms::Delegate` would
suffice (house style section 8), and the 5,000-line `application.cpp`.
