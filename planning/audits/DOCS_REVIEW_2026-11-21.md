# GUI.Forms documentation review, 2026-11-21

Step 4 of the order of operations to 1.0 (week plan section 1.2, items D1 to
D4). A review only: no page, generator or image was changed, nothing was
committed and nothing was built.

- **Checkout:** `~/Developer/work/docs-review/gui_forms`, detached at
  `c0e1a31a8` (main after #483 cp32, #484 cp33, #485 and #486).
- **Read:** every hand-written page under `docs/user`, `docs/dev`,
  `docs/building`, `docs/release` (index and unreleased), `docs/wiki`,
  `README.md`, `docs/README.md`, `docs/library/README.md`,
  `docs/library/MIGRATION_FRONTIER.md`, `docs/index.rst`, `docs/conf.py`,
  `.readthedocs.yaml` and `packaging/RELEASE_NOTES.md`. `docs/reference/`
  holds JSON inputs, not pages. Generated pages and docstrings were sampled.
- **Inputs:** week plan 1.2; the two audits of 2026-11-13
  (`DOCS_AUDIT_USER.md`, `DOCS_AUDIT_DEV.md`); `ROADMAP_TO_1.0.md`, "Order of
  operations to 1.0".
- **Generator checks run read-only** (output to the scratchpad, `git status`
  clean afterwards): `generate_library_docs.py --check` PASS;
  `control_registry.py members --check` PASS (157 types);
  `control_registry.py render` identical to `docs/controls/generated/`;
  `control_registry.py readme-table` identical to the README table;
  `control_schema.py render --check` current; `stock_specimens.py --check`
  current; `winforms_divergences.py check` OK. The generated pages match
  their generators. Their stale text comes from header comments and
  docstrings (section 2, "Generated pages").
- **Severity:** *wrong* means the page states something false at main;
  *outdated* means it was true once, or something important is missing;
  *minor* means wording, a small gap or a broken path that misleads little.
- **Not counted as stale** (the owner's instruction): pages the MDI work and
  the `<bind>` element change will touch. They are listed in section 7.

## 0. What is true at main

| Fact | Value at `c0e1a31a8` | Source |
|---|---|---|
| Package version | 0.6.0; tags `v0.6.N` | `CMakeLists.txt:9` |
| C ABI | **1.5** (1.4: content boundary, 2026-11-13; 1.5: window outer band and shaping, pointer location, list view table, failure detail, composites, business card and diagnostic overlay, relative pointer, application font families, presentation capabilities) | `include/gui_forms/c/gf_abi.h:69-91`; commits `1cc8a9d3e`, `caac8d758` |
| Host protocol | **15** (14: locale; 15: relative pointer) | `include/gui_forms/host/types/host_capabilities/host_capabilities.hpp:28-36` |
| Drawing ABI (`gd_*`) | **sealed at 1.0** on 2026-11-13; 1.1 provisional (path flattening, `raster_initialize`) | `include/gui_forms/drawing_c_api.h:32-33,116-124`; `8eee05d42` |
| Controls | 154 in `manifests/controls.json` (70 stable, 83 provisional, 1 lab) | registry |
| Member manifest | 283 types; members: 2,334 supported, 13 deferred (10 of them Window MDI), 367 excluded, 4,141 unclassified | `manifests/members.json` |
| API reference | 215 pages, 2,914 types, 692 enumerations; 181 types with a page; 150 docstrings; 31 registry controls without a docstring | `generate_library_docs.py` run to scratch |
| Tests | none in this tree; they live in `malkuth-software/gui_forms_tests` and build against the test cache. The tree's only `add_test` is the fuzz targets' | `AGENTS.md`, "Public seam"; `cmake/FuzzTargets.cmake:28` |
| Form documents | XML (`urn:gui-forms:form:1`); old syntax refused; one-time converter `tools/convert_form_to_xml.py` | `33e71576f` |
| Fonts | every face compiled into Core as one font pack; no face file is installed or packaged, only licences | `cmake/FontPack.cmake:1-17` |
| Text stack | STXlib (glyphs, textlayout, markdown) since 2026-11-16; FreeType, HarfBuzz, SheenBidi, libunibreak, MD4C gone | `AGENTS.md` |
| STXlib | pinned through `stx.lock` since cp33; row height index on STXlib's `stx_btree`; `stx_auxlib.lock` and `cmake/StxAuxlib.cmake` are gone | `stx.lock:1-10`; `src/controls/collections/row_height_index.hpp:16-18` |
| Public `std::optional` | being replaced by `stxlib::Optional` (D15 V1, census ratchet); e.g. `Window::paint` returns `stxlib::Optional<PaintReceipt>` | `include/gui_forms/surface_root/surface_root.hpp:981` |
| New hosts and targets | browser host (Emscripten, `src/surface/web/`, `GUI_FORMS_ENABLE_WEB_HOST`, `cmake/emscripten.cmake`, landed `a7ea4a525`); Windows MSVC target (`cmake/llvm22-windows-msvc*.cmake`, not in CI); high-integrity builds (`cmake/HighIntegrity.cmake`, `examples/high_integrity`) | `CMakeLists.txt:133,138` |
| Windows host | advertises `text_composition`; no relative pointer, no direct buffers, no first-frame gate, no inbound drop | `src/surface/windows/application/windows_host.cpp:4588-4615` |
| Linux host | publishes `AtkText` with caret and selection; XInput 2 raw motion for relative pointer; no preedit (`XIMPreeditNothing`) | `src/surface/linux/accessibility/linux_accessibility.cpp:808-856`; `linux_host.cpp:1304,2271-2284` |

## 1. The site's "in progress" items

Every page or row that says in progress, not yet, to come, planned, to be
recorded or similar, with its true status at main. Items that are still
true are kept and marked "keep".

| # | Where | It says | True at main | Fix |
|---|---|---|---|---|
| 1.1 | `docs/dev/roadmap.md:9-13` and `docs/_static/roadmap.svg` (`tools/make_roadmap_svg.py:29-80`) | chevrons end at 0.5; "1.0 Backend: In progress" listing "C ABI additions, keys to focus", "Theming and the look integration", "Final controls, help system" | those three landed in cp24 to cp31; 0.6 is the package number and is not shown | Edit `STAGES`: add a 0.6 chevron (current), make 1.0 "next"; regenerate the SVG and the alt text |
| 1.2 | `docs/dev/roadmap.md:90-141` "1.0 (backend): in progress", "Done since the seal" | done list stops at checkpoints 29 to 31 (#482) | cp32 (#483) and cp33 (#484) merged 2026-11-17 and 2026-11-18 | Add what #483 and #484 brought (see 2.A4) |
| 1.3 | `docs/dev/roadmap.md:143-203` "In progress: the Mac side, to the Windows hand-back" | font identity and font pack, content boundary, engine and controls, demonstrators, STXlib swap-in, Mac conditions, consumers, "much of it ... in branches of checkpoints 32 and 33 that wait to be folded" | font family registry `FontFamilyId` (`6ae847879`) and the embedded font pack: landed. Content surfaces, content editor and hosted content on macOS, Linux **and Windows**: landed (C ABI 1.4). Stock icons, curved paths (gd 0.6), host locale on all three hosts, ribbon minimized popup and QAT, menu first-letter jumps, file dialog checks, DataGridView on the shared row engine (`column_strip.hpp`), window band in C ABI **1.5** (not 1.3): landed. Drawing ABI: sealed at 1.0 on 2026-11-13. STXlib: pinned since cp33, text stack and row index moved; D15 conversion still running. Plot3DView (D5): landed. Consumers' rebases and the hand-back: not in this repository | Replace the section with the order of operations (steps 5 to 9) in public wording; move landed streams to "Done" |
| 1.4 | `docs/dev/roadmap.md:236-237` | "Emscripten ... is planned separately" | a browser host landed in cp33 (`src/surface/web/`, `GUI_FORMS_ENABLE_WEB_HOST`) | State what exists (a browser host presenting the CPU surface) and what is left |
| 1.5 | `docs/dev/roadmap.md:241-275` "Tools (after 1.0 backend)" | the tools follow 1.0 (backend); launcher "in progress" | order of operations (owner, 2026-11-19): MCP is step 7, the tools are step 8 "the tools of Block 5 the owner brings forward", 1.0 is declared at step 9 | Rewrite the sequence to match; ask the coordinator whether "1.0 (backend)" and "1.0 Stable" remain two names |
| 1.6 | `docs/dev/roadmap.md:314` milestone "Unicode text ..." | "in progress: preedit on X11 remains" | true (`XIMPreeditNothing`) | keep |
| 1.7 | `docs/dev/roadmap.md:315` "Layout ..." | "DataGridView on the same row engine is in checkpoint 33" | landed | Say done |
| 1.8 | `docs/dev/roadmap.md:317` "Collection, command and data controls" | menu first-letter jumps and DataGridView breadth "in checkpoint 32"; "MDI (with docking) is open" | first two landed; MDI is being coded (section 7) | Say done; leave MDI to the MDI change |
| 1.9 | `docs/dev/roadmap.md:320` "Themes, assets and localization" | "the font family registry and the font pack are in checkpoints 32 and 33" | landed | Say done |
| 1.10 | `docs/dev/roadmap.md:321,327,328` compositor, pressure, atomic raster | "in progress: ... rows of the stage gate are open"; "native wall-clock soak remains"; "opt-in Windows transactional frame store" | not verifiable from the tree; no change seen in #483/#484 | keep; confirm with the coordinator |
| 1.11 | `docs/dev/roadmap.md:322` "Authoring manifest" | member manifest "classifies every property, event and method"; "conformance fixtures are open" | 4,141 of 6,855 members are `unclassified` | State the counts; the 1.0 condition needs every member classified |
| 1.12 | `docs/dev/roadmap.md:323` "Stable C ABI" | "in progress: ABI 1.3 is installed"; "the drawing ABI (`gd_*`, 0.3 on `main`) is sealed after the demonstrators" | ABI 1.5; gd sealed at 1.0, 1.1 provisional | Correct both numbers |
| 1.13 | `docs/dev/roadmap.md:324` "Windows and Linux hosts" | "Windows lacks inbound drag and drop and occlusion; X11 lacks preedit" | true | keep |
| 1.14 | `docs/dev/roadmap.md:325` "Packaging and release" | signing, notarization, SBOM, reproducible builds remain | true (Formal 1.0 F.3) | keep |
| 1.15 | `docs/dev/roadmap.md:408-426` "Open work by area" | documentation: signature-only members and CI screenshots; hosts: "the locale read from the host (checkpoint 32; Windows with the hand-back)"; controls: MDI | locale is read on all three hosts (`8c86ef5b5`, host table row `locale`); documentation items are D2 and D3 of this week | Drop the locale item; point documentation at D2/D3 |
| 1.16 | `docs/wiki/Roadmap.md:12` | "To come: font identity and the font pack, the content boundary, the demonstrators and the drawing ABI seal, the STXlib swap-in, the 1.0 conditions, then the Windows hand-back" | the first four landed; Windows hand-back and conditions open | Rewrite the row from the order of operations |
| 1.17 | `docs/wiki/Roadmap.md:13` | tools "after 1.0 (backend); the launcher is in progress" | see 1.5 | Match 1.5 |
| 1.18 | `docs/user/hosts-and-capabilities.md:57-60` | Windows "not yet" for `relative_pointer`, `raw_pointer_motion`, `pointer_confinement`, `pointer_warp` | true: the Windows host advertises none | keep; say "no (contract in Relative pointer mode)" |
| 1.19 | `docs/user/hosts-and-capabilities.md:61` | Windows `direct_buffers` "not yet" | owner, 2026-11-16: no GPU surfaces; Windows has no direct path by design (`presentation.md:33`) | "no (by design; host scroll only)" |
| 1.20 | `docs/user/relative-pointer.md:90,109-138` | Windows "contract below"; "until then it reports none" | true | keep |
| 1.21 | `docs/user/window-band.md:64,87-111` | Windows 11 native border "not yet (contract below)"; line 87 "does not yet set any border attribute" | border colour not set: true. But the corner preference is implemented (item 1 of the same list), so line 87 contradicts line 89 | Line 87: "sets the corner preference only; the border colour is not set yet" |
| 1.22 | `docs/user/presentation.md:31` | Wayland: "GUI.Forms has no Wayland host yet" | true; but `roadmap.md:442` says a Wayland adapter "is not planned" | Reconcile the two pages ("not planned" or "not yet") with the coordinator |
| 1.23 | `docs/user/presentation.md:191-194` | the Windows host "still shows at once" (first-frame gate) | true: `FirstFrameShowGate` is used only under `src/surface/macos/` and `src/surface/linux/` | keep |
| 1.24 | `docs/user/theme-language.md:939-944` "Not yet built" | pointer-positioned gradient refused (E405); "Procedural backgrounds (`felt()` and the rest) are warnings until the engine registers them" | the engine registers `felt` and eleven other patterns, written `pattern(felt, ...)` (`theme_material_lowering.cpp:251-268`, `theme-language.md:312`) | Keep the gradient line; reword the second to say a bare `felt()` call is a warning and `pattern()` is the form (verify the warning code first) |
| 1.25 | `docs/user/theme-language.md:627` | `popup` rules "compiled and held but not yet applied to native popup" | popups take a backdrop through `NativePopupOptions::backdrop` (`native_popup_window.cpp:102`); whether sheet `popup` rules reach them was not confirmed | Coder to confirm; then fix or keep |
| 1.26 | `docs/user/theme-language.md:659-661` | "In 0.5 the band is a steady colour or the built-in flash only. Effect programs and textures on the band are planned after 0.5" | the band has textures, `cycling`, colour roles and a frequency (`window-band.md:33-50`) | Replace with a link to The window band |
| 1.27 | `docs/user/for-windows-forms-developers.md:181-186` "Not yet available" | host auto-complete sources unsupported; outbound drag and native menu bar unavailable | true | keep |
| 1.28 | `docs/user/input-and-focus.md:251-252` | inbound drop not on Windows; "Outbound drag initiation is not yet available on any host" | true (`typed_drag_source` advertised nowhere) | keep |
| 1.29 | `docs/user/hosts-and-capabilities.md:361,372-373` | "OLE drag/drop and native menu publication are not implemented"; Linux "editable text ranges are not implemented" | first: true. Second: wrong, the Linux bridge implements `AtkText` with caret and selection | Delete the Linux sentence (see 2.B1) |
| 1.30 | `docs/user/factory-bundle.md:142-170` | "Measured on: to be recorded" and an empty table; "Not yet measured" for Windows and Linux | the table is empty at main; the tool it names, `stx_fontpack/tools/bench_loader.py`, is not in the tree | Fill from a run on the M4 or remove the empty table until measured; fix the tool path |
| 1.31 | `docs/user/forms.md:116,223` | E999 `not_yet_built`: `convert <name>` reserved | true (`form_model.cpp:834,1158`) | keep (the `<bind>` change may alter it, section 7) |
| 1.32 | `docs/user/text-engine.md:89-91,176-181` | fallback "does not yet choose by text language"; "Open edges" | true as far as the tree shows | keep |
| 1.33 | `docs/controls/generated/members.md` and every control's API page | members "not yet classified" (4,141); Window MDI rows "deferred ... not built yet" (`docs/api/generated/gui_forms.Window.rst:631-745`) | true; MDI being coded | keep; regenerate after MDI |
| 1.34 | `docs/library/docstrings/Ribbon.rst:350` (page `gui_forms.Ribbon.rst`) | "Not yet: the Quick Access Toolbar, the minimized overlay, ..." | `Ribbon::set_quick_access_toolbar` and the minimized popup exist (`include/gui_forms/controls/ribbon.hpp:317-348,454-458`) | Remove those two from the list; check the rest |
| 1.35 | `docs/library/docstrings/OpenFileDialog.rst:92` and `SaveFileDialog.rst:101` | "Not yet supported: the filter index (the host request has no field for it), ... checking that a typed name exists" | `HostOpenFileDialogRequest::initial_filter` exists and is filled from `filter_index` (`host_types.hpp:346,363`); `check_file_exists` is honoured on every host (`hosts-and-capabilities.md:430-450`) | Rewrite both "Not yet" paragraphs |
| 1.36 | docstrings `DockSite.rst:360`, `RibbonColorPicker.rst:88`, `RibbonFontControl.rst:63`, `ColorDialog.rst:54`, `FolderBrowserDialog.rst:55`, `ThreadExceptionDialog.rst:56` | "Not yet ..." lists | not contradicted by anything found; not individually verified | Writer checks each against its header |
| 1.37 | `docs/api/library.drawing.rst:1543` (from `include/gui_forms/drawing/graphics_path/graphics_path.hpp:109` and `recorder_raster/recorder_raster.hpp:11`) | "Provisional, numbered 0.8 of the drawing seam as a placeholder" | the slot is gd 1.1 provisional | Fix the two header comments (a coder), then regenerate |
| 1.38 | `docs/user/app-fonts.md:77`, `gf_abi.h:1572` | application fonts "C ABI 1.5, provisional" | true (1.5 unreleased) | keep |

## 2. D1: stale content, page by page

### 2.A Project-level pages

**README.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 13-14 | wrong | "the package version is 0.6 (C ABI 1.3)" | C ABI 1.5 | "0.6 (C ABI 1.5, host protocol 15, drawing ABI 1.0)"; say 1.0 is next |
| 26-32 | minor | platforms: macOS, Windows, Linux | a browser host exists (cp33) | Add a row once the coordinator says it is public |
| 98 | outdated | Gallery screenshot | captured 2026-10-11 (`8d39fb980`, `f93b640d0`): old navigation, status line "Host 0.4", not windows-professional | Replace (section 5) |
| 266-270 | minor | one house-style command | `AGENTS.md` lists seven checks | Link the contributor's guide for the full list |

**docs/index.rst**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 33-39 | minor | "native hosts for macOS, Windows and Linux" | plus a browser host | Mention it when public |

**docs/dev/roadmap.md** (beyond the items in section 1)

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 92-96 | minor | "coders write in parallel without building, an integrator folds ..." | the site's own writing rules (`docs/README.md:62-63`) forbid agent or role names and milestone numbers | Describe checkpoints without roles; drop checkpoint numbers from user pages |
| 141 | outdated | "The first build under the 0.6 number is `v0.6.1541`" | still true, but the page names no newer build | Keep; add the latest tag at rewrite time |
| 209-214 | outdated | "programs cannot register fonts with its engine" | owner decision of 2026-11-15: programs load application font families for composites and sub-surfaces (`app-fonts.md`); stock controls and themes still never draw them | Add the exception |
| 379-380 | wrong | "The host protocol versions ... now at version 13" | 15 | 15 |
| 393-394 | wrong | "The C ABI at version 1.3" | 1.5 | 1.5, and gd sealed at 1.0 |
| 395-396 | outdated | done list | lacks content surfaces, hosted content, composites (`.control`), app fonts, relative pointer, font pack, XML forms, business card, presentation shim, first-frame show, browser host | Add |

**docs/dev/architecture.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 14-19 | outdated | six main components plus CAbi and optional ones | also `HostedClient` (`cmake/GUIFormsConfig.cmake.in:188`) and `GUIForms::Module` (`cmake/Module.cmake:13`) | Add both |
| 29-31 | minor | host adapters `macos | windows | linux` | also `src/surface/web` and the window manager `src/host/surface/` | Add |
| 63-65 | wrong | the drawing C ABI's "library is not part of the package until the drawing ABI is sealed" | gd is sealed at 1.0 and ships inside `gui_forms_abi` (`CMakeLists.txt:1090-1094`) | Rewrite |
| 75-82 | wrong | `cmake/check_host_boundaries.cmake`, `cmake/check_drawing_boundaries.cmake` | do not exist; `cmake/` has `check_renderer_boundaries.cmake`, `check_windows_boundaries.cmake`, `CheckCAbiExports.cmake`, `CheckObjectSurface.cmake`, `CheckSystemImports.cmake` | List what exists; boundary tests now run in gui_forms_tests |
| 185-189 | wrong | protocol "at version 13"; C ABI "1.3"; "drawing ABI (`gd_*`, 0.3) stays experimental until it is sealed" | 15; 1.5; sealed 1.0 | Correct |

**packaging/RELEASE_NOTES.md** (included by `docs/release/unreleased.md`)

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 20 and `unreleased.md:3-5` | outdated | "These notes cover checkpoints 24 to 31 (#474 to #482)" | #483 (cp32), #484 (cp33), #485, #486 merged | Add a section for each |
| 22-64 | outdated | breaking changes stop at cp31 | missing breaking changes: form documents are XML (old syntax refused; `tools/convert_form_to_xml.py`); FreeType and HarfBuzz components no longer installed; public signatures moving from `std::optional` to `stxlib::Optional` (D15); fonts no longer read from files (`fonts/` holds licences only); tests moved to gui_forms_tests | Add |
| new | outdated | (absent) | C ABI 1.4 and 1.5; gd sealed at 1.0; host protocol 14 and 15; content surfaces and hosted content; composites; app fonts; relative pointer; business card and diagnostic overlay; presentation shim (`GUI_FORMS_PRESENT`); first-frame show; browser host; high-integrity builds; warnings as errors in CI | Add under "Other changes" |

**docs/user/code-statistics.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 65-149 | outdated | counted at gui_forms `2f06e9a4b` (2026-11-16); includes a `tests` component of 631 C++ and 42 C files | the tests left the tree in cp33 | Run `tools/code_statistics.py` again at the week's head |

### 2.B User guide

**docs/user/hosts-and-capabilities.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 12 | wrong | `protocol_version` "currently 13" | 15 | 15 |
| 20-63 | outdated | capability table: three host columns | no row for `open_url` (advertised by macOS, Windows and Linux); no column for the browser host (`web_host.cpp:186`) | Add the row; add the column when the web host is documented |
| 61 | minor | Windows `direct_buffers` "not yet" | no direct path by design | see 1.19 |
| 312 | wrong | macOS: "Fonts are read from the bundle" | from the font pack in the library | Fix |
| 323-326 | wrong | Windows text uses "the bundled faces in `fonts\` beside the executable, or ... `GUI_FORMS_FONT_DIR`; with an incomplete font pack ..." | faces come from the embedded pack; `GUI_FORMS_FONT_DIR` only in development builds | Fix |
| 372-373 | wrong | Linux: "A text box's text is published whole; editable text ranges are not implemented" | `AtkText` with caret and selection (`linux_accessibility.cpp:808-856`); also contradicts lines 418-420 | Delete |
| 404-406 | outdated | X11: "no XInput2" | XInput 2 raw motion is used for relative pointer mode (`linux_host.cpp:2271-2284`); the wheel still uses core buttons | "The wheel uses core buttons 4 to 7; XInput 2 is used only for relative motion" |

**docs/user/text-engine.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 74 | outdated | Portsmouth Rapids light/italic, Portsmouth, Portsmouth Mono: "Installed with the SDK; not registered by the hosts" | in the pack's interface group (`factory-bundle.md:96`); no face is installed | Fix the row |
| 168-173 | wrong | "Windows does not advertise `HostCapability::text_composition`" | it does (`windows_host.cpp:4598`); inline IMM32 composition, as `hosts-and-capabilities.md:356-361` says | Rewrite the paragraph |

**docs/user/install.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 119-129 | wrong | "Build, test and install": `ctest --test-dir build`, `xvfb-run -a ctest` | the tree registers no tests (only fuzz targets when fuzzers are on) | Drop ctest; point at gui_forms_tests and `building/index.md#tests` |
| 168-177 | outdated | package components table | lacks `HostedClient` | Add the row (`content-surfaces.md:131-136` has the text) |
| 162-195 | minor | (no module component) | `GUI_FORMS_BUILD_MODULE` installs `GUIForms::Module` | Add a row |

**docs/user/consume-the-sdk.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 86-95 | minor | components table | lacks `HostedClient`, `Module` | Add |
| 162-163 | wrong | "`tests/application_text_consumer/` in the repository" | in gui_forms_tests | Name that repository |
| 197-198 | minor | "`gd_get_api_v0`; there is no `gdr_*` export / and keeps libc++ ..." | a broken sentence | Rejoin it |
| 215-217 | wrong | "(`tests/hostile_macros/`)" | in gui_forms_tests | Name that repository |

**docs/user/download.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 66-67 | outdated | "Keep the `fonts/` directory beside the executable" | it holds only licences (`packaging/programs.json:7`) | "The `fonts/` folder holds the fonts' licences" |

**docs/user/debugging.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 81 | wrong | failure detail "C ABI 1.4" | 1.5 (`gf_abi.h:2445-2452`) | 1.5 |
| 1-69 | outdated | invalidation overlay only | 1.5 adds the diagnostic overlay's counters card and its toggle gesture (`gf_abi.h:2454-2466`; C++ `set_diagnostic_overlay`, `toggle_diagnostic_overlay` in `components/business_card/business_card.hpp:128-133`) | Add a section |

**docs/user/painting-and-damage.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 118 | wrong | `Window::paint` "returns `std::optional<PaintReceipt>`" | `stxlib::Optional<PaintReceipt>` (`surface_root.hpp:981`) | Fix; D15 will move more types, so check each spelled type at the week's head |

**docs/user/application-and-windows.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 208-234 | outdated | C ABI section stops at 1.3 | 1.5 adds the window outer band and shaping, pointer location, business card and program identity (`gf_abi.h:79-84`) | Add "Since 1.5"; link The window band |
| (absent) | outdated | no business card or program identity | `application.set_program_identity`, `windows.show_business_card` (owner decision 4a, 2026-11-15); `BusinessCard` control | Add a short section |

**docs/user/c-abi-conformance-kit.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 8 | minor | "GUI.Forms runs the same kit in its own tests" | in gui_forms_tests | Say so |
| 45-66 | outdated | coverage list | lacks the content table (1.4; `content-surfaces.md:257` names `gf_conformance_content.c`) and 1.5's slots | Add |
| 132-135 | minor | the kit "uses pthreads"; MSVC not covered | the kit gained a pthread shim and `Sleep` on the Microsoft target (`89ed444e5`, `2d38cf907`) | Coder confirms; then update |

**docs/user/theme-language.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 6-9 | wrong | conformance corpus at `tests/theme_language/conformance/` | in gui_forms_tests | Name that repository |
| 627, 659-661, 939-944 | | | | see 1.24 to 1.26 |

**docs/user/layout.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 409-410 | wrong | corpus "in `tests/layout_corpus/`" | in gui_forms_tests | Name that repository |

**docs/user/relative-pointer.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 179-181 | minor | "`tests/relative_pointer_example_tests.cpp`" | in gui_forms_tests | Name that repository |

**docs/user/content-surfaces.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 138, 333-335 | minor | `tests/hosted_client_consumer`, `tests/hosted_content_tests.cpp`, `tests/hosted_child.cpp` | in gui_forms_tests | Name that repository |

**docs/user/audio.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 5-7 | wrong | stx_vorbis "lives in this repository under `stx_vorbis/`" | in the stx repository, pinned by `stx.lock` | Fix |
| 145-152 | wrong | `stx_vorbis/README.md` and three `stx_vorbis/docs/*.md` "in the repository" | in stx | Point at stx |

**docs/user/localizable-literals.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 88-93 | wrong | `tools/dump_localizable_literals.py` | not in `tools/` | Find where it went (gui_forms_tests or the plugin) or delete the paragraph |

**docs/user/transparency.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 139-141 | wrong | `tools/t5_gate_record.py` | not in `tools/` | Fix or delete |

**docs/user/effect-verification.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 44-51 | wrong | Linux native and sanitizer jobs verify effects; an `effect-pass` job in `.github/workflows/effect-pass.yml` | no such workflow in `.github/workflows/`; tests run in gui_forms_tests | Describe where the pass's cases run now |

**docs/user/factory-bundle.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 143 | wrong | `stx_fontpack/tools/bench_loader.py` | `stx_fontpack/` has no `tools/` | Fix the path |
| 142-170 | outdated | empty measurement tables | see 1.30 | |

**docs/user/for-windows-forms-developers.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 96-146 | outdated | type correspondence | missing 20 registry controls with a Windows Forms counterpart: `ListView` (stable; the table maps ListView to ObjectView only, line 123), `ToolStrip` (stable; line 140 maps ToolStrip to CommandBar), `StatusStrip`, `ToolStripPanel`, `ToolStripContainer`, `BindingNavigator`, `MaskedTextBox`, `RichTextBox`, `MonthCalendar`, `DomainUpDown`, `DataGridView`, `FontDialog`, `TaskDialog`, `ThreadExceptionDialog`, `FolderBrowserDialog`, `ColorDialog`, `OpenFileDialog`, `SaveFileDialog`, `NotifyIcon`, `Splitter` | Generate the table from the registry's `winforms` field, or add the rows |
| 171-172 | wrong | "the legacy `Splitter`" under "Not provided" | `Splitter` is a registered provisional control (layout) | Remove it from the list |

**docs/user/forms.md, first-window.md, values-commands-and-binding.md, data-binding.md, properties.md, converting-an-app.md, rules.md, mouse-events.md, update-scopes-and-dirtiness.md, styled-text.md, threading.md, lifecycle.md, controls.md, overlays-popups-and-splash.md, themes.md, retained-tree.md, presentation.md, window-band.md, app-fonts.md, content-surfaces.md (apart from paths), index.md**: nothing stale found beyond the rows above and section 7. Forms are already XML on every page.

### 2.C Building, contributing and maintainer pages

**docs/building/index.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 41-45, 314-317 | outdated | Windows build puts "a `fonts/` directory" beside the Gallery | licences only | Say so |
| 49-71 | outdated | options table | lacks `GUI_FORMS_ENABLE_WEB_HOST`, `GUI_FORMS_WARNINGS_AS_ERRORS` (CI turns it on), `GUI_FORMS_HIGH_INTEGRITY` (and `_ALLOW_CLIPBOARD_TEXT`), `GUI_FORMS_BUILD_C_ABI`, `GUI_FORMS_BUILD_MODULE`, `GUI_FORMS_REFERENCE_ARITHMETIC`, `GUI_FORMS_EFFECT_VERIFY_REQUIRED`, `GUI_FORMS_WINDOWS_AUTOMATION_ENVIRONMENT`, `GUI_FORMS_ENABLE_WAYLAND_PRESENT` | Add the user-facing ones; leave probes |
| 89-93 | outdated | stx package contents | lacks `stx_raster3d` and STXlib (`stx.lock:1-6`) | Add |
| 95-100 | wrong | row index from stx_auxlib at `stx_auxlib.lock` (`cmake/StxAuxlib.cmake`); "STXlib ... is planned to take this over" | STXlib's `stx_btree` through the stx package; both files are gone | Rewrite |
| 199-221 | wrong | `tools/run_sanitizer_suite.sh` | not in `tools/` | Delete the section; `ci_build.py --sanitize --test-cache` is the procedure |
| 155-197, 222-231 | outdated | named tests (`gui_forms_owned_event_tests` and others) as if run here | those tests are in gui_forms_tests | Mark as that repository's |
| 245-252 | minor | `abi_tsan.py` "builds only what the tests labelled `c_abi` need" | the labels live in gui_forms_tests | Coder confirms how `abi_tsan.py` finds them now; then fix |
| 268-343 | outdated | Windows: MSYS2 CLANG64 only | an MSVC target exists (`cmake/llvm22-windows-msvc.cmake`, `GUI_FORMS_MSVC_RUNTIME=OWN`), not in CI | One paragraph, marked experimental, if the coordinator wants it public |
| (absent) | outdated | no browser (Emscripten) build | `cmake/emscripten.cmake`, `src/surface/web/`, `web/paint/` | Add a section when public |

**docs/building/fast-builds.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 27-34 | wrong | each native job "runs every native, installed-SDK and development-profile test (on a pull request, `--test-targets-only --changed-from HEAD^1`)" | the native job builds, packs the test cache and runs no tests; "The tests run there, not here" (`.github/workflows/native-platform.yml:352-353`) | Rewrite the loop |
| 105-107, 173-181 | wrong | `cmake/TestPrecompiledHeader.cmake`, `cmake/TestRuntime.cmake`, `GUI_FORMS_TEST_PRECOMPILED_HEADER` | the files are gone with the tests | Delete or move to gui_forms_tests' docs |
| 197 | outdated | builds `gui_forms_owned_event_tests` | a gui_forms_tests program | Fix |

**docs/building/reusing-ci-caches.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 42, 75-110 | wrong | native job's ctest JUnit report, `validated_tests`, programs job copying a ctest receipt | no ctest in this tree's CI | Rewrite against the test-cache flow, or check whether `release_tests.py` still reads receipts and describe that |

**docs/dev/contributing.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 35 | wrong | "Boundary tests run under `ctest`" | in gui_forms_tests | Fix |
| 62 | wrong | house-style command includes `tests` | `AGENTS.md` and README omit it: no `tests/` here | Use AGENTS.md's seven commands |
| 73-139 | outdated | sanitizers "builds and runs the whole suite"; `--tests`, `--skip-labels`, area labels, `ctest -L area_gui`, `change_selection.py` | the suite runs in gui_forms_tests against the sanitize test cache (`AGENTS.md`) | Move the test-running half to gui_forms_tests; keep the tree-building half |
| 166-167 | wrong | "Build and run `ctest`" | no tests here | "Build, then run gui_forms_tests against your test cache" |

**docs/dev/tools.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 40 | outdated | writes `tests/doc_examples_tests.cpp` | writes it into the gui_forms_tests checkout (`tools/tests_checkout.py`; `generate_library_docs.py:807-812`) | Fix |
| 61, 73 | minor | examples run on `tests/...` files | none here | Use paths under `src/` |
| 99-100, 134-135 | outdated | "ctest runs it" (native boundaries, ABI audit) | gui_forms_tests | Fix |
| 144 | wrong | "the C ABI 1.3 class tables" | introduced in 1.3; the ABI is 1.5 | "the class tables (since C ABI 1.3)" |
| 197-200 | wrong | `tools/check_test_registration.py`, `tools/check_test_asserts.py` | not in `tools/` | Remove or point at gui_forms_tests |
| 225-235 | wrong | `ci_build.py` "runs `ctest`"; `--test-targets-only` "the whole suite" | it packs the test cache (`--test-cache`) | Rewrite |
| 251-254, 264 | outdated | `tests/installed_feature_consumer/`, `tests/sdk_consumer/` | gui_forms_tests | Fix |
| 359-361 | outdated | grapheme cases written to `tests/unicode_grapheme_break_cases.hpp` | gui_forms_tests | Fix |
| 364-370 | minor | profile areas name HarfBuzz and FreeType | gone since 2026-11-16 | Name STXlib |
| (absent) | outdated | 31 tools under `tools/` are undocumented, among them `convert_form_to_xml.py`, `verify_form_conversion.py`, `check_std_rules.py`, `std_surface_census.py` (both in AGENTS.md's required checks), `make_roadmap_svg.py`, `readthedocs_build.py`, `factory_bundle_sizes.py`, `stock_icon_codec.py`, `tests_checkout.py`, `consumer_shield.py`, `module_exports.py` | | Add the ones a contributor runs |

**docs/README.md** (maintainer notes)

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 92 | outdated | generator "writes api/ and tests/doc_examples_tests.cpp" | the examples test goes to the gui_forms_tests checkout | Fix |
| 100-104 | wrong | the examples test "builds and runs under `ctest`, so ... fails the native jobs" | it runs in gui_forms_tests | Fix |
| 59-73 | minor | writing rules | no rule for previews or source links (D2, D3) | Add once D2 and D3 land |

**docs/library/README.md**

| Line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| 20, 115-116 | outdated | `tests/doc_examples_tests.cpp` "runs in CI" here | gui_forms_tests | Fix |
| 98-99 | minor | statements must match "the tests under `tests/`" | gui_forms_tests | Fix |
| 11 | minor | `captures/` "kept as evidence. Not published." | see section 3 and section 8 | Update after the coordinator decides |

**docs/library/MIGRATION_FRONTIER.md**: marked as history; nothing to fix.

### 2.D Wiki pages (`docs/wiki/`)

| Page:line | Sev | It says | True at main | Fix |
|---|---|---|---|---|
| Roadmap.md:12-13 | outdated | | | see 1.16, 1.17 |
| Setting-up-a-GUI-Forms-application.md:5 | minor | "the approach sealed in GUI.Forms 0.5" | still the approach; 0.6 now | "the approach GUI.Forms recommends" |
| Themes.md:51 | minor | the `window` and `popup` surfaces "hold only `backdrop`" | `window` rules also take the band properties (`theme-language.md:631-658`) | Add the band |
| Home.md, _Sidebar.md, Application-and-windows.md, Form-documents.md, Overlays-Popups-and-Splash.md, Properties.md, Rules.md | | nothing stale found (Form-documents is in section 7) | | |

### 2.E Generated pages and docstrings (sample)

The generated pages match their generators. The sample (Window, Ribbon,
OpenFileDialog, SaveFileDialog, DockSite, the drawing topic page, the member
summary, by-function and alphabetical indexes) found:

- 1.34 to 1.37 above (docstrings and one header comment);
- 31 registry controls have no docstring, so their pages show signatures and
  header comments only: WindowCapsule, ProgressRing, Spinner, Sparkline,
  PlotView, Plot3DView, PlotAxisItem, PlotSeriesItem, PlotAnnotationItem,
  ValueBar, ColorPicker, CompositeControl, FormLayout, SevenSegmentDisplay,
  GaugeBase, LinearGauge, RadialGauge, Dial, StepIndicator, Wizard,
  WizardPage, Callout, ModalLayer, LookupComboBox, AboutDialog,
  SubSurfaceHost, PlotLinkGroup, HelpBook, HelpBookViewer, HelpBookSystem,
  BusinessCard (outdated);
- `members.md` and each control's correspondence table report 4,141
  unclassified members (true; see 1.11).

## 3. D2: previews

**What carries an image now.** No published page does. Of the 40 user
pages, the 215 API pages (181 type pages, 14 control categories, 14
library topics and indexes) and the generated control indexes, none has an
`image` or `figure` directive. The landing page's four card icons
(`docs/_static/index-images/*.svg`) and the roadmap chevrons are the only
pictures on the site. The README shows one screenshot.

**Existing images and their theme.**

| Image | Where used | What it shows | Theme |
|---|---|---|---|
| `docs/images/gallery-macos.png` (1280x820, 2026-10-11) | README.md:98 | the Gallery's old shell; status line "Portable core · Host 0.4" | the pre-0.5 default look, before the windows-professional theme was redone (cp28) and before `gui-forms` became the default; not today's windows-professional |
| `docs/images/correspondence-macos.png` (1120x720, 2026-10-11) | nothing | Correspondence | same era |
| `docs/library/captures/*` (54 files, October atlas) | nothing (excluded from the site) | control captures and "screen" captures of the old showcase | the old "Windows 7/10 professional lineage" showcase look. 12 `*_screen.png` files are 1259x760; the two opened (`basic_controls_002_screen.png`, `tab_control_screen.png`) are photographs of a whole personal desktop showing the machine's name, notifications and a terminal with a home path. 13 files named `.png` are JPEG data (10 of them `*_screen`, plus `card_collection`, `master_detail_view`, `review_card`) |

**Capture tools at main.**

| Tool | What it produces | Theme control |
|---|---|---|
| `Window::capture_window`, `capture_control` (`include/gui_forms/surface_root/surface_root.hpp:680,701`; `encode_window_capture_png`) | a live window or one control subtree, overlays and band included | the window's theme |
| `OffscreenRaster` + `OffscreenAttachment::capture_control` (`include/gui_forms/capture/offscreen_raster/offscreen_raster.hpp`, `gui_forms/capture.hpp`) | windowless captures at a chosen scale, colour scheme and contrast; no host window | the window's resources (`ThemeBase::windows_professional`) |
| Stock specimen catalogue (`include/gui_forms/controls/stock_specimens.hpp`, `tools/stock_specimens.py`) | one representative, filled instance of every control type in the schema, made by name; `frame` is what a palette captures | as above |
| Gallery headless capture (`demo/gallery_headless_capture.hpp`; options in `demo/program_options.hpp:15-45`) | `--page <id> --capture <png> --headless [--size WxH] [--scale f] [--theme-base windows-professional] [--scheme light|dark] [--reveal <id>] [--open-menu <id>]`: one Gallery page, still, no window | `--theme-base windows-professional` |
| `tools/capture_help_book.sh` | the help-book page, light and dark, 1x and 2x, into `docs/library/captures/` | the Gallery's default (`gui-forms`); add `--theme-base` |
| Windows automation probe `capture [path]`; host `--capture` after the first frame | a live window's framebuffer | the program's |

No program yet walks the specimen catalogue and writes one PNG per control
for the docs, and no generator inserts images. The standing rule (site
screenshots in windows-professional) needs `--theme-base
windows-professional` or `ThemeBase::windows_professional` explicitly,
because `gui-forms` is the default.

**Pages that need a preview.**

- Every control's API page: the 154 registry controls. 13 have no Gallery
  page (`gallery_page` null: Sparkline, PlotView, Plot3DView, PlotAxisItem,
  PlotSeriesItem, PlotAnnotationItem, ValueBar, WindowCapsule,
  PlotLinkGroup, TextBoxBase, ListControl, UpDownBase, BusinessCard), so
  their pictures must come from the specimen catalogue; the three abstract
  bases (TextBoxBase, ListControl, UpDownBase) need none.
- Visual feature pages: `first-window.md` (the finished window),
  `controls.md` (VirtualRowsView, NodeCanvas, IndicatorLamp, a danger
  button), `themes.md` (one small picture per built-in and familiar theme:
  seven), `theme-language.md` (band, effects, finishes), `window-band.md`,
  `overlays-popups-and-splash.md` (menu, tooltip, splashscreen),
  `transparency.md` (glass, reflection), `styled-text.md`, `debugging.md`
  (invalidation overlay, counters card), `content-surfaces.md` (the stale
  veil), `layout.md` (flex, grid spans), `factory-bundle.md` (a specimen
  line per family), `application-and-windows.md` (business card).
- The 37 Gallery pages (`demo/pages/*.form`) can serve the category pages
  (`docs/api/controls.*.rst`) through the headless capture.

## 4. D3: source links

**What is configured.** Nothing. `docs/conf.py:48-52` loads `myst_parser`,
`sphinx_design` and `sphinx_copybutton` only: no `sphinx.ext.linkcode`, no
`sphinx.ext.viewcode` (Python-only anyway), no `extlinks`, no edit-page
button (`html_theme_options` has no `use_edit_page_button`), and
`html_copy_source = False`. Generated type pages print the header as plain
text: ``#include <gui_forms/...>`` (`tools/api_docs_rst.py:553`).

**What the generators already know.** `tools/api_docs_parse.py` records
`header` and `line` for each `TypeRecord` (lines 36-41) and `EnumRecord`
(61-65), and `HeaderScope.header` for namespace-scope functions. `Method` and
`Declaration` (lines 21-32) carry no line; `record_declaration` (412-440) is
where a line would be added.

**How to generate the links at a pinned commit.**

1. *The pin.* Resolve the commit in `conf.py` at build time, not in the
   generated files, so that `generate_library_docs.py --check` stays stable:
   `READTHEDOCS_GIT_COMMIT_HASH` on Read the Docs, `GITHUB_SHA` in the docs
   job, else `git rev-parse HEAD`, else `main`.
2. *Classes, functions and enumerations* (`[source]` beside each
   signature): add `sphinx.ext.linkcode` and a `linkcode_resolve(domain,
   info)`. For the C++ domain `info["names"]` holds the declaration's names;
   map each to `path#Lline` and build
   `https://github.com/malkuth-software/gui_forms/blob/<commit>/<path>#L<line>`.
   The map comes from the generator: it writes
   `docs/api/source_map.json` (qualified name to header and line), which
   `--check` already compares with the rest of `docs/api/` and which Sphinx
   ignores as a non-source file. Confirm on Sphinx 9.1.0 that linkcode reads
   the C++ domain's names as expected (Sphinx was not installed here).
3. *A file link role* for the include line and for hand-written pages: add
   `sphinx.ext.extlinks` with `extlinks = {"src": ("https://github.com/
   malkuth-software/gui_forms/blob/<commit>/%s", "%s")}`. The generator
   writes `` :src:`include/gui_forms/controls/button.hpp` `` instead of the
   plain include line, and hand-written pages write `` {src}`path` `` (MyST)
   where they now name a repository file in backticks.
4. *Each page's own source* (hand-written and generated): a small template
   in `docs/_templates/` (for example `page-source.html`) added to
   `secondary_sidebar_items`, linking
   `blob/<commit>/docs/<pagename><suffix>`; `html_context` passes the
   commit. For generated pages the useful source is the header, so the
   template can prefer a page-level `source` metadata the generator writes.

**Files a coder changes.**

| File | Change |
|---|---|
| `docs/conf.py` | extensions `sphinx.ext.linkcode` and `sphinx.ext.extlinks`; commit resolution; `linkcode_resolve`; `extlinks`; `templates_path`; `html_context` commit; sidebar item |
| `docs/_templates/page-source.html` (new) | per-page source link |
| `tools/api_docs_parse.py` | `line` on `Method`, `Declaration` and namespace-scope functions |
| `tools/api_docs_rst.py` | `:src:` include line (`type_page`, line 553); page `source` metadata |
| `tools/generate_library_docs.py` | write `docs/api/source_map.json`; include it in `check` |
| `tools/control_registry.py` (`render`) | optional: `:src:` links to each control's header in the indexes |
| `tools/test_generate_library_docs.py` | tests for the map and the role |
| `docs/README.md` | writing rule: repository files are written with `{src}` |

No new Python dependency is needed: both extensions ship with Sphinx.

## 5. D4: the GitHub README

| Item | Now | Fix |
|---|---|---|
| Version line (13-17) | "0.5 ... is done, and the package version is 0.6 (C ABI 1.3). Work is now towards 1.0 (backend)" | "The package version is 0.6; 1.0 is next" with C ABI 1.5, host protocol 15 and drawing ABI 1.0 named once; link the roadmap |
| ABI line | 1.3 | 1.5 (wrong now) |
| Roadmap image | none in the README; `docs/_static/roadmap.svg` exists but is stale (1.1) | Regenerate the SVG with a 0.6 chevron and embed it (`![Roadmap](docs/_static/roadmap.svg)`; GitHub renders SVG) |
| Previews | one Gallery screenshot from 2026-10-11 in the old look; `correspondence-macos.png` unused | Replace with a windows-professional capture from the Gallery's headless mode (`--page control-spectrum --capture ... --headless --theme-base windows-professional --scale 2`), shown at a small width; add a strip of two or three page captures, or drop the unused image |
| Control table | 154 rows, identical to `control_registry.py readme-table`; "more than 140" holds | accurate; regenerate only when the registry changes |
| Platforms table | three hosts | add the browser host when the coordinator says it is public |
| Quick start, tests note, links, badges | current (tests in gui_forms_tests; Docs badge from #486) | none |

## 6. Work list for writers

Sizes: S up to two hours, M half a day, L a day or more. Groups A, B and C
touch disjoint files, so three writers can work at once. Group D is code
for coders (not writers); group E needs the coordinator first. Every
writer checks a statement against main before writing it, and regenerates
nothing that a coder owns.

**Group A: project-level pages** (README.md, `docs/dev/roadmap.md`,
`docs/dev/architecture.md`, `docs/wiki/Roadmap.md`, `docs/index.rst`,
`packaging/RELEASE_NOTES.md`, `docs/release/unreleased.md`,
`docs/user/code-statistics.md`, `tools/make_roadmap_svg.py` and
`docs/_static/roadmap.svg`)

1. (L) Rewrite `docs/dev/roadmap.md`'s 1.0 sections from the order of
   operations: done since the seal through #486, what is next, milestones
   (1.2 to 1.15), done list, open work, Wayland wording; no role names or
   checkpoint numbers.
2. (S) Edit `STAGES` in `tools/make_roadmap_svg.py` (a 0.6 chevron, 1.0
   next) and regenerate `roadmap.svg` and the alt text (1.1).
3. (S) `docs/wiki/Roadmap.md` to match item 1 (1.16, 1.17).
4. (M) Release notes for #483 to #486, with the breaking changes (2.A,
   RELEASE_NOTES rows).
5. (S) `docs/dev/architecture.md` (2.A).
6. (S) README version, ABI and platform lines (section 5); the image swap
   waits for D2's capture (item 21).
7. (S) Regenerate `code-statistics.md` with `tools/code_statistics.py`
   (needs the stx and STXlib checkouts; no build).
8. (S) `docs/index.rst` host sentence (when the browser host is public).

**Group B: user guide** (`docs/user/*` except `code-statistics.md`)

9. (M) `hosts-and-capabilities.md`: protocol 15, `open_url` row, fonts on
   macOS and Windows, Linux text ranges, XInput 2, direct buffers (2.B,
   1.19, 1.29).
10. (S) `text-engine.md`: Windows composition, the Portsmouth row.
11. (M) `install.md`, `consume-the-sdk.md`, `download.md`: ctest, components
    (`HostedClient`, `Module`), moved test paths, fonts folder, the broken
    sentence.
12. (M) `debugging.md` (1.5 detail, diagnostic overlay),
    `painting-and-damage.md` (`stxlib::Optional`),
    `application-and-windows.md` (C ABI 1.5, business card),
    `c-abi-conformance-kit.md` (coverage, tests location).
13. (S) `theme-language.md`: corpus path, band paragraph, popup rules, "Not
    yet built" (1.24 to 1.26; 1.25 after a coder confirms).
14. (M) `for-windows-forms-developers.md`: the 20 missing rows and
    `Splitter` (the MDI line waits for the MDI change).
15. (S) Moved test paths in `layout.md`, `relative-pointer.md`,
    `content-surfaces.md`.
16. (S) `audio.md`, `localizable-literals.md`, `transparency.md`,
    `effect-verification.md`, `factory-bundle.md`: missing tools and
    workflows, stx_vorbis location, empty tables (1.30).

**Group C: building, contributor and maintainer pages, docstrings, wiki**
(`docs/building/*`, `docs/dev/contributing.md`, `docs/dev/tools.md`,
`docs/README.md`, `docs/library/*`, `docs/wiki/*` except `Roadmap.md`)

17. (M) `docs/building/index.md`: options, stx package rows, row index,
    sanitizer script, fonts folder, MSVC and browser targets (2.C).
18. (M) `fast-builds.md` and `reusing-ci-caches.md`: the loop and receipts
    without tests in this tree.
19. (M) `contributing.md` and `tools.md`: tests moved, ctest references,
    missing and undocumented tools.
20. (S) `docs/README.md`, `docs/library/README.md`: the examples test's new
    home.
21. (S) Docstrings: `Ribbon.rst`, `OpenFileDialog.rst`, `SaveFileDialog.rst`
    (1.34, 1.35); check the six in 1.36. Then a coder regenerates
    `docs/api/` (`generate_library_docs.py`); docstring examples compile in
    gui_forms_tests, so the writer tells the coordinator which files changed.
22. (L) Docstrings for the 31 controls that lack one (2.E), in batches;
    the instruments (SevenSegmentDisplay, gauges, Dial) and the plots first.
23. (S) Wiki: `Setting-up-a-GUI-Forms-application.md:5`, `Themes.md:51`.

**Group D: code for coders** (not writers)

24. (M) D3 source links (section 4 file list).
25. (L) D2 previews: a small capture program on the M4 that walks the stock
    specimen catalogue with `OffscreenRaster` under
    `ThemeBase::windows_professional` (scale 2, light) and writes
    `docs/_static/previews/<Type>.png` at a small, fixed maximum width; the
    Gallery headless capture for the 37 pages and the feature pages; the
    generator inserts a preview on a type page when the file exists, and
    `--check` covers it. The README image (item 6) uses one of these.
26. (S) Header comments `graphics_path.hpp:109`, `recorder_raster.hpp:11`
    ("0.8" to gd 1.1 provisional), then regenerate (1.37).
27. (S) Confirm `popup` sheet rules on native popups (1.25), the
    conformance kit on the MSVC target (2.B), and how `abi_tsan.py` finds
    the `c_abi` tests (2.C).

**Group E: for the coordinator first**

28. Destructive, needs the owner's approval: `docs/library/captures/` (54
    files): the `*_screen` captures are photographs of a personal desktop
    (the two opened show the machine's name, notifications and a terminal
    with a home path), in a public repository; 13 are JPEG data under
    `.png` names; all predate the theme engine. Proposal: delete the folder
    (or keep only the files a page will use, re-captured), and delete
    `docs/images/correspondence-macos.png` unless the README uses it.
29. Decide whether the browser host, the MSVC target and high-integrity
    builds are documented publicly now (they gate rows in items 6, 8, 9
    and 17).
30. Decide the release names: "1.0 (backend)" and "1.0 Stable", or one 1.0
    after the order of operations (items 1 to 3).

## 7. Pages the two changes in progress will touch (flagged, not counted)

**MDI** (Window MDI members, now deferred):

- `docs/user/for-windows-forms-developers.md:166` ("MDI" under "Not
  provided": outside the library by design);
- `docs/dev/architecture.md:193-194` ("MDI is planned with the docking
  work");
- `docs/dev/roadmap.md:317,423` (MDI open);
- `docs/api/generated/gui_forms.Window.rst:631-745` and
  `docs/controls/generated/members.md:211,288` (deferred and excluded rows,
  from `manifests/members.json`);
- probably a new section in `docs/user/application-and-windows.md` and a
  docstring for the MDI members (`docs/library/docstrings/Window.rst`).

**Form bindings as `<bind>` elements:**

- `docs/user/forms.md:29-33,63-65,112-117,201-228` (attribute bindings, the
  `\bind(` escape, E817, E818, E999, the Bindings section);
- `docs/user/first-window.md:16,128-133`;
- `docs/user/values-commands-and-binding.md:10-12,20-21,97-98,129`;
- `docs/user/data-binding.md:61`;
- `docs/user/properties.md:165` (link to Forms#bindings);
- `docs/user/converting-an-app.md:88-106`;
- `docs/wiki/Form-documents.md:15-16,32-35`;
- `docs/wiki/Setting-up-a-GUI-Forms-application.md:97,111-113,125`;
- docstrings that show a form binding, if any (`CheckBox.rst`,
  `ButtonBase.rst`, `Command.rst`, `RangeControl.rst`, `ChoiceGroup.rst`,
  `ProgressBar.rst`, `VScrollBar.rst` mention `bind(`; most are the C++
  `bind` call, which may not change).

## 8. Counts

| Section | Count |
|---|---|
| 1. In-progress items | 38 rows (13 still true and kept; the rest need a change or a check) |
| 2. D1 findings | 94 table rows (37 wrong, 36 outdated, 21 minor), plus the 31 controls without a docstring (2.E) |
| 3. D2 | 0 pages with a preview; 154 control pages and 13 feature pages need one; 6 capture tools; 3 existing image sets, none in windows-professional |
| 4. D3 | not configured; 8 files to change |
| 5. D4 | 7 items, 3 needing change now (version and ABI line, roadmap image, previews) |
| 6. Work list | 30 items (A 8, B 8, C 7, D 4, E 3) |
