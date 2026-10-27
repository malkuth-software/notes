# GUI.Forms 1.0 consolidation survey

Date: 2026-10-26. Status: **survey and CANDIDATE sequence only**. Nothing
here is decided. It records what the later 1.0 consolidation must move, so
the work starts from evidence rather than memory. Pin surveyed: GUI.Forms
`2fe26d8d` (0.2.x).

## GIVEN

The owner (2026-10-26): once consolidation toward 1.0 begins, this session
refactors or rewrites the File Manager interface code and SwiftEdit's
dedicated file-picker code onto the new GUI.Forms paradigms and capability
exposure. The dependency pin and build-method work is recorded separately in
`../../planning/GUI_FORMS_0_2_CONSUMPTION_2026-10-26.md`.

The owner, the same day: Web.Forms is deprecated, and a better authoring
route will roll out toward GUI.Forms 1.0. Much of what the File Manager
interface depends on, and in turn SwiftEdit and other downstream consumers,
moves forward later as GUI.Forms moves toward 1.0. The generated units below
are therefore carried, not repaired: their known defects are recorded and
leave with the generator.

## What is being moved (OBSERVED at the pin)

| Area | Today |
|---|---|
| Authoring | ADR-016: `ui/boards/file_manager/file_manager.wf.{html,css}` (179 and 684 lines) compiled by the deprecated Web.Forms into five generated C++ units (17,662 lines) against `web_forms/capabilities/gui_forms_observed_002.json`. Sanitized, `make_native_form()` needs a 636,760-byte frame and `make_native_button_state_recipes()` 502,232 bytes (gui_forms `tools/stack_frames.py`, limit 262,144) |
| Composition | `src/application.cpp`, 5,093 lines: controls, commands, menus, styles, subscriptions and workers in one translation unit |
| Styling | hand-written `BasicControlStyle` colour records (`Color::rgba` literals) and `SurfaceMaterial`/`MaterialFillLayer` values in C++ |
| Layout | Web.Forms-generated containers; two `FlowLayoutPanel` hosts (settings pages) and one `TableLayoutPanel` (criteria actions) written by hand |
| Callbacks | 139 `std::bind_front` subscriptions in `src/`; 28 non-owning `gui_forms::Delegate` bindings, mostly in the picker view |
| Picker | `src/document_picker{,_view}.cpp` (769-line view), installed as the `FileManagerDocumentPicker` 1.0 package; SwiftEdit consumes `DocumentPickerView` and `DocumentPickerResult` |
| Text boxes | five edit-session boxes opt in to Enter/Escape since GUI.Forms 0.2 (search, rename, picker path, name filter, file name) |

The most-used toolkit vocabulary in `src/`: `StableId` (56), `Command` (55),
`Control` (42), `Color` (34), `Label` (30), `DropDownButton` (29),
`Delegate` (28), `Button` (28), `TextBox` (24), `ContextMenu` (21),
`ObjectGlyph` (17), `InstrumentModuleSpec` (14), `ObjectViewItem` (13). No
use of `HostImage`, pixel-format types, `DibPainter` or `CoreGraphicsRaster`
was found, so stage SK's announced changes to those types appear to touch
File Manager only through the SDK, not its sources (not yet compiled
against SK).

## The GUI.Forms paradigms it moves onto (status at the pin)

| Stage | Gives File Manager | Status |
|---|---|---|
| T1 anatomy and schema | addressable parts and states of every control | complete |
| T2 engine | layered materials, state machines, settling motion, zero idle frames | sealed |
| T3 theme language | CSS-syntax themes compiled fail-closed; replaces hand-written colour records | in progress (P5 lowering merged) |
| T4 default theme | the GUI.Forms look as theme data | not started |
| L layout | `FlexLayoutPanel`, `GridLayoutPanel`, `BlockFlowPanel`; Flow/Table rebased onto the solvers | L1 measure contract and L2 value parser merged |
| FD form documents | CSS-family form documents with stable IDs and named handlers; disposable generated C++ | not started; format decided |
| H HTML import | real static HTML/CSS onto L and T3; "throw out the earlier conversion" | earlier attempt discarded |
| C stable C ABI | 1.0 requires C++ wrappers to be RAII conveniences over the C tables | experimental, not installed |
| SK universal Skia | one painter and one pixel format everywhere | owner-paused |
| W file dialogs | host-native `OpenFileDialog` and `SaveFileDialog` components | merged |

## Owner questions this work cannot answer alone

1. **Which authoring route replaces Web.Forms.** Web.Forms is deprecated
   (GIVEN above); ADR-016 still names it, so the replacement needs its own
   decision record superseding that part of ADR-016. The candidates at the
   pin are GUI.Forms' form documents (FD) and HTML import (H), neither
   started; the owner's "better one" may be either or neither.
2. **Picker and host dialogs.** GUI.Forms now ships host-native file
   dialogs. Does SwiftEdit keep File Manager's embedded picker (the
   File Manager browser projection) for 1.0, use the host dialogs, or offer
   both? The picker package's 1.0 contract version is independent of this.
3. **Panel topology.** GUI.Forms retired ADR-013's reservation of
   persistent side panels to File Manager (2026-10-13); File Manager's
   `AGENTS.md` still states it. Should File Manager's records follow?
4. **Keyboard policy.** The five opted-in boxes preserve pre-0.2 behaviour.
   Should Enter in the picker's file-name box reach an accept button
   instead, as GUI.Forms now does by default?

## CANDIDATE sequence (for comparison, not selected)

1. Split `application.cpp` along its existing seams (shell, commands and
   menus, object view, inspector, search, transfers, settings) without
   behaviour change, with the current 15 tests as the contract.
2. Replace `std::bind_front` subscriptions with named non-owning delegates
   where a function and context suffice (house style section 8).
3. Move colour and material literals to a File Manager theme sheet once T3
   loads themes; keep the C++ values as the built-in fallback until T4.
4. Move the three hand-written Flow/Table hosts onto L's containers after
   L's oracle gate passes.
5. Re-author the shell on the Web.Forms replacement (question 1); this
   retires the oversized generated frames and the Windows sanitized tree's
   8 MB main-thread reservation with them.
6. Re-expose the picker per the answer to question 2, over the C ABI's
   RAII wrappers once stage C installs them.

Each step needs its own measured gate and source review against
`../../planning/PROGRAMMING_HOUSE_STYLE.md`.
