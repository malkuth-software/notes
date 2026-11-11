# Borrow-lifetime sanitizer: inventory and design study

Base: `origin/checkpoint/2026-11-11-cp28` @ e39b2822d, branch
`claude/borrow-sanitizer-study` in `w5/gui_forms`. Design study only: no
code under `include/` or `src/` changes. Canonical home: the notes
repository's `gui_forms` branch (`planning/` or `decisions/`); the
coordinator moves this file there.

## 0. The question

GUI.Forms already handles ownership and lifetime with shared owners, weak
observers, unique owners, borrowed `T*`/`T&`, explicit dispose,
generational `ControlTable` slots (`control_at` answers null for a stale
slot) and callback revocation. What remains is a **borrowed view that
becomes invalid while its owner is still alive** (structural invalidation),
and a view that outlives its owner (lifetime invalidation):

```cpp
std::span<const Control::Ptr> children = parent.children();
parent.add_child(other);          // may relocate children_, or shift it
use(children[0]);                 // reads relocated or shifted storage
```

AddressSanitizer reports only memory that is poisoned. It sees this case
only when the old buffer was freed (relocation) and not yet recycled, or
when the read lands between size and capacity of a libc++ vector with
container annotations. It does not see an insertion, removal or reorder
that stays within the capacity: the bytes are mapped and initialized, they
just belong to other elements now. House style section 1 calls that a
defect anyway ("Removing an element ends its lifetime even if its bytes
remain"). A borrow sanitizer ties each escaping view to its owner's
mutation generation and stops the process on the first use after an
invalidating mutation or after the owner's death.

## 1. Inventory

### 1.1 Method

- `dev/tools/borrow_inventory.py` (workspace `w5/dev/tools/`, copied to
  `docs-notes/borrow_inventory.py`) scans `include/` and `src/` for
  functions returning `std::span`, `std::string_view`, `const char*`,
  `const std::vector&`/`const std::string&`, iterators, `T&`/`T*`
  accessors, and members that store a span, string view or back link.
  It found 2062 candidate sites; the full table is
  `docs-notes/borrow_inventory.tsv` (kind, file, line, type, name, uses).
- Use counts come from `dev/tools/borrow_calls.py`: call sites of
  `.name()`/`->name()`, how many bind the result to a named local (the
  escaping form), range-for loops over it and member stores. Counts are by
  name, so a common name (`read`, `items`, `pixels`) sums every class that
  has one; the per-API rows below were checked by reading.
- Classification by reading:
  - **Static-backed views are not at risk** and are excluded from the
    ranking: `property_table()`/`child_property_table()` (134 accessors,
    static `std::array` tables), anatomy declarations
    (`PartDeclaration`, `ControlAnatomy` spans into `constexpr` tables),
    schema strings, and most of the 558 stored `std::string_view` fields
    (constant tables in `.cpp` files and argument records).
  - **Argument records** (a span or view field in a request passed down one
    call: `TextShapingRequest::utf8_text`, `ApplicationThemeConfig::controls`
    "borrowed for the call to create only") are borrows for one call. They
    are at risk only when the callee stores them.
  - **Owner-relative views** (the subject): 89 inline member accessors
    returning a member container as a view (58 `std::span`, 5
    `std::string_view`, 26 `const std::vector&`/`const std::string&`; of the
    58 span accessors only 18 carry the `const&`/`const&& = delete` pair),
    out-of-line ones of the same shape, 43 accessors returning a back link
    (`Window*`, `SurfaceRoot*`, `Control*`, `Application*`), 101 stored
    back-link fields and about 40 stored payload views (event records,
    paint records). About 270 owner-relative sites in all.

### 1.2 Ranked inventory

Risk weighs: public or not, how often the view is bound to a name or
stored (escapes), whether the invalidating mutation is reachable from code
that runs while the view is held (callbacks, layout, event handlers), and
whether ASan would see the failure.

| # | API (file:line) | Owner | Invalidated by | Escapes | Uses | Risk |
|---|---|---|---|---|---|---|
| 1 | `Control::children()` -> `std::span<const Ptr>` (`include/gui_forms/control/control/control.hpp:355`) | the parent `Control` (`children_`, a `std::vector<Ptr>`) | `add_child`, `insert_child`, `remove_child`, `set_child_index_from_top`, `clear_children`, disposal, and any of them from a callback during a walk | returned; bound to a named local at 26 sites (20 in `src/`: window.cpp, help_contents, abi_objects, five layout panels, plot_view x5, page_view, navigation_pane, tool_strip_panel x2, dock_site) | 122 calls (src 58, tests 60) | **highest**: hot, public, walks call out (measure, arrange, semantics) into code that can edit the tree; audit N84 already hit it and added `children_revision()` as a hand-rolled generation |
| 2 | Borrowed `Window*`/`SurfaceRoot*` in `Control`: `window()` (`control.hpp:1172`), `surface_root()` (`:1175`), `attached_window()` (`:418`), `attached_surface_root()` (`:421`); fields `window_`, `surface_root_`, `layout_deferred_listed_in_` (`:1600`, `:1601`, `:1768`) | the attachment (set on attach, cleared on detach) and the `Window` itself | detach, re-parenting to another window, window close or destruction | returned; bound at 49 sites (34 `attached_window`, 15 `window`) | 212 calls | **high**: a handler that closes the window or moves the control while a caller holds the pointer; the rules live in comments only |
| 3 | `CollectionCell<E,C>::read()` -> `std::span<const E>` (`include/gui_forms/binding/collection_cell/collection_cell.hpp:155`); `owner_elements()` (`:295`) | the cell (`elements_`), inside ListBox/ComboBox/DomainUpDown text items, DataGridView rows, plot series columns | `operator=`, `insert`, `erase`, moves, clear; already counted by `revision_` | returned; bound at 10 sites; `abi_data_grid.cpp:793`, `data_grid_view_rows.cpp:124` hold it across loops | `read` 5510 calls by name (mostly `PropertyCell::read`, which returns `const Value&` to an object that stays put) | **high**: one template, public, edits come from user handlers (selection changed -> items edited) |
| 4 | Item-list accessors returning `std::span<const Item>`: `ListView::items()` (`controls/panel/list_view/list_view.hpp:215`), `TreeView` (`tree_view.hpp:52`), `RibbonGallery` (`ribbon_gallery.hpp:69`), `ObjectView` (`:87`), `CorrespondenceView` (`:79`), `ExpandableSections`, `CommandBar`, `ChoiceGroup`, `BackstageView`, `AutoComplete` (`:85`), `TextDecorations` (`:179`), `ControlBindingsCollection` (`:35`); `const std::vector&` siblings `ToolStrip::items()` (`tool_strip.hpp:140`), `LookupComboBox::rows()/columns()/filtered_rows()`, `Ribbon::application_menu()` | the control's item vector | `set_items`, add/remove item, sort, filter | returned; 10 bound locals, 5 range-for | `items` 209 calls (tests 153, demoboard 33) | **high** for consumers (the demoboard iterates and edits), medium inside the library |
| 5 | `TextStore::utf8()` -> `std::string_view` (`include/gui_forms/text/text_store/text_store.hpp:19`) | the text store (`text_`) | every edit (insert, erase, replace, undo) | returned; bound at 10 sites, including rich_text_box_format.cpp:229, :377, :506, :532 and rich_text_box_layout.cpp:195, which hold it across formatting steps | 55 calls | **high**: edits are frequent, in-capacity edits are ASan-silent, and text code keeps byte offsets into the view |
| 6 | Event payload views lent for a raise: `DockDocumentClosing::documents` (`controls/docking/dock_workspace/dock_workspace.hpp:67`), `FileDialogOk::file_names` (`components/file_dialog_types/file_dialog_types.hpp:69`), the ABI's `gf_event` fields and text "lent for the invoke" (`gf_abi.h:479`, `:490`) | the raise | the raise's end; a handler that edits the workspace (closes a document) during the raise | stored in a record the handler can copy and keep | per event | **medium-high**: the lifetime is a comment ("valid only while the event is raised") |
| 7 | `PartPaint::text` stored `std::string_view` (`theme/part_painting/part_painting.hpp:159`), `PartPaint::control_type` (`:247`) | whoever filled the record (a control's text) | the record outliving the paint call, or the text changing during paint | stored in a record | every themed text paint | **medium-high**: twice held a temporary (CorrespondenceView, PropertyList); `-Werror=dangling-gsl` now catches only the temporary form |
| 8 | Back links stored across callbacks in the ABI tables: `AnchoredSession::window` (`src/abi/stable/tables/abi_overlays.cpp:242`), run `model` (`abi_windows.cpp:63`), `Window::previous_in_application_`/`next_in_application_` and `Application::first_window_`/`last_window_` (`window.hpp:334`, `application.hpp:489`), `NativePopupRecord::model_` | the window, by documented detach order | window close before detach; a reordering of teardown | stored | 10 fields | **medium**: correct today by teardown order, unchecked |
| 9 | Pixel views: `PartRaster::pixels()` (`engine/part_raster/part_raster.hpp:37`), `PaintFramebuffer::pixels()` (`paint_framebuffer.hpp:25`), `ImageSnapshot::pixels()` (`drawing/image_snapshot/image_snapshot.hpp:24`), `LiveSurfaceFrame::pixels()` (`live_surface/frame/live_surface_frame.hpp:45`), `LiveSurfaceWriteLease::pixels()` (`write_lease/live_surface_write_lease.hpp:37`), `InvalidationOverlay` pixels (`:115`), `PreparedTextLayout::pixels()` (`:82`) | the raster, framebuffer or lease | resize, reconfigure, the lease's move or end | returned; 65 bound locals | `pixels` 227 calls | **medium**: relocation is a real free (ASan sees it), but a reconfigure at the same size reuses the bytes |
| 10 | `Control::owned_decorations()` (`control.hpp:532`), `Control::inline_style()` (`:621`), `inline_style_rules()` (`:624`) | the control | `set_owned_decorations`, `set_inline_style`, theme change | returned | 8 calls (tests) | low-medium |
| 11 | Diagnostic and record spans: `SurfaceRoot::layout_diagnostics()` (`surface_root/surface_root.hpp:1249`, documented "valid until the next layout pass or clear", `&&` deleted), `SemanticFeedback::records()` (`feedback/semantic_feedback/semantic_feedback.hpp:25`), `InlineStyleResult::diagnostics()` (`theme/theme_compiler/theme_compiler.hpp:158`), `DamageRegion::rectangles()` (`types/damage_region/damage_region.hpp:20`), `ComponentContainer::components()` (`component_container.hpp:19`), `ThemeRules::diagnostics()/published_values()` (`engine/theme_rules/theme_rules.hpp:1644`) | the root, feedback, result or region | the next pass, `emit`, `add`, clear | returned; mostly tests | 300 calls by name, mostly tests | low |
| 12 | Layout workspaces: `GridLayoutWorkspace::items()` (`layout/grid_solver/grid_solver.hpp:190`), Flex and BlockFlow `items()`/`order()` | the workspace, reused per layout | the next solve | returned, `&&` deleted | internal | low (internal, single caller per workspace) |
| 13 | `PartGeometryBuffer::parts()` (`anatomy/types/anatomy_types.hpp:392`) and the other 40 span accessors without a ref qualifier | a value type | the value's end (a temporary) | returned | 458 calls by name | low today: no binding from a temporary found; the `&&` delete is missing (lifetime rule gap, section 2.7) |
| 14 | C ABI, outward | none lent | n/a | n/a | n/a | **none by construction**: `gf_abi.h:21`/`:37`, text out goes into caller buffers; the only lent library storage is schema text, valid until process end (`:1088`) |
| 15 | C ABI, inward lending: caller text "valid until the library's next callback" (custom controls `gf_abi.h:2315`, binding providers `:2561`), `gf_paint_target` pixels lent for the call (`:2272`) | the C or C# caller | the next callback, the call's end | held by the library between callbacks | per provider | **medium**: the library is the borrower; a library path that keeps the text past the next callback reads freed caller memory (C# pinned buffers make this ASan-invisible) |
| 16 | Host adapters | the host | n/a | n/a | n/a | **low**: the macOS view owns its `Window` (`std::unique_ptr<Window> _model`), accessibility elements keep a `__weak` owner, a copied `SemanticNode` and runtime identities; Windows and Linux keep run-scoped raw pointers (`RunOpenState*`, `X11Windowing*`, `NativeWindow*`) that live exactly as long as the run; native entries are being routed through `call_failing_fast` (CP29 row M: 118 AppKit entries not yet routed) |

Precedents already in the tree, which the design reuses:
`Control::children_revision_` (audit N84), `CollectionCell::revision_` with
`require_unchanged_since`, `ControlTable` slot generations, and
`detail::WindowLifetime` (a shared record a window clears when it goes).
Each is a hand-rolled owner generation; none is checked at the point of use.

## 2. Design

### 2.1 Shape: a checked borrow type family

Two modes, chosen for the whole program (section 2.6):

- **Ordinary build:** each borrowed type is an alias of the plain view.
  `BorrowedSpan<T>` is `std::span<const T>`, `BorrowedMutableSpan<T>` is
  `std::span<T>`, `BorrowedText` is `std::string_view`, `BorrowedRef<T>` is
  `T&` (spelled `T*` where null is a state: `BorrowedPointer<T>`). Same
  size, same code, same ABI, zero cost: the public signatures are literally
  what they are today.
- **Checked build:** each is a small class with the plain view's data
  (pointer and size) plus a **stamp**: a pointer to the owner's state
  record, the generation the view was made at, and the creation site.
  Every element access, iteration step, `size()` and conversion checks the
  stamp first.

```cpp
namespace gui_forms {

#if GUI_FORMS_CHECKED_BORROWS
template <typename Element>
class BorrowedSpan final {
public:
    using element_type = const Element;
    // Made only by an owner, from its storage's pointer and live length
    // (house style section 1: state extent and lifetime) and its stamp.
    BorrowedSpan(const Element* data, std::size_t size, detail::BorrowStamp stamp) noexcept;
    [[nodiscard]] std::size_t size() const noexcept;     // checks
    [[nodiscard]] bool empty() const noexcept;           // checks
    [[nodiscard]] const Element& operator[](std::size_t index) const; // checks stamp and bound
    [[nodiscard]] BorrowedIterator<Element> begin() const noexcept;    // checked iterator
    [[nodiscard]] BorrowedIterator<Element> end() const noexcept;
    [[nodiscard]] BorrowedSpan subspan(std::size_t offset, std::size_t count) const;
private:
    const Element* data_;
    std::size_t size_;
    detail::BorrowStamp stamp_;
};
#else
template <typename Element>
using BorrowedSpan = std::span<const Element>;
#endif

// The one explicit way out to a plain view, valid in both modes: checks the
// stamp once (checked build) and returns the untracked view. For passing to
// a callee that takes std::span and finishes before the owner can change.
template <typename Element>
[[nodiscard]] std::span<const Element> unchecked_view(const BorrowedSpan<Element>& view);

}
```

Design points:

- **No implicit conversion to `std::span` in the checked build.** A caller
  that writes `std::span<const Control::Ptr> children = parent.children();`
  compiles in the ordinary build and fails in the checked build: the
  checked tree is what enforces that an escaping borrow is spelled as a
  borrow. Callers spell the owner's named alias (`Control::ChildView`), as
  house style section 3 asks ("spell the type", "descriptive aliases").
- **Checked iterators** carry the stamp pointer; a range-for over a view
  checks on every dereference, so the loop that calls out to a callback
  which edits the owner fails on the next element, not later.
- **Construction from pointer and size only.** The cp28 `-ftime-trace`
  study (`mac_handoff_results/2026-11-11/time-trace-cp28.md`, finding 2)
  measured concept checking at 751 s inclusive, 23% of the build, dominated
  by libc++'s constrained range constructor of `std::span`, instantiated in
  every unit by one-line accessors such as `control.hpp:355`,
  `component_container.hpp:20`, `damage_region.hpp:20`,
  `part_raster.hpp:37`, `surface_root.hpp:1250`. So:
  - the checked class has no range constructor at all;
  - in the ordinary build the accessor body changes from `return
    children_;` to `return BorrowedSpan<Ptr>(children_.data(),
    children_.size());`, which reaches `std::span`'s pointer-and-count
    constructor, not the range constructor;
  - accessors on cold paths move out of line (declared in the header,
    defined in the owner's `.cpp`), so the instantiation happens once.
    Hot accessors (`children()`) stay inline in the ordinary build.
  - Stage 2 measures this with the same `tools/time_trace_report.py` run.
  The conversion is therefore also a compile-time saving, independent of
  the checking.
- **House style.** No `auto`: every view has a spelled alias. No lambdas:
  the check is a named function (`detail::require_current(stamp)`). The
  owner's mutation is a named storage operation (section 2.2). Mutable
  views (`owner_elements()`) are `BorrowedMutableSpan<T>`. Accessors keep
  `const&` with `const&& = delete` (the lifetime rule) in both modes.

### 2.2 The owner state and who bumps the generation

```cpp
namespace gui_forms::detail {
#if GUI_FORMS_CHECKED_BORROWS
struct BorrowOwnerState final {
    std::uint64_t generation{};          // advanced by every structural change
    bool alive{true};                    // false once the owner is destroyed
    std::thread::id owner_thread{};      // the execution context (section 2.4)
    void* last_invalidation_site{};      // return address of the last invalidate
    BorrowInvalidation last_reason{BorrowInvalidation::none};
};
struct BorrowStamp final {
    std::shared_ptr<BorrowOwnerState> owner;  // keeps the record, not the owner
    std::uint64_t generation{};
    void* creation_site{};
};
#endif

// A member of every owner that lends views. Empty in the ordinary build.
class BorrowOwner final {
public:
    // Ends every view lent so far (named storage operation, section 1).
    void invalidate_borrows(BorrowInvalidation reason) noexcept;
    [[nodiscard]] BorrowStamp stamp() const noexcept;   // checked build only
    ~BorrowOwner();  // checked build: alive = false, generation advanced
private:
#if GUI_FORMS_CHECKED_BORROWS
    std::shared_ptr<BorrowOwnerState> state_;
#endif
};
}
```

- **Which mutations bump: every structural change**, not only
  relocations. Insert, erase, reorder, assign, clear, `reserve`/shrink,
  replacement of the whole container, attach and detach for back links.
  Writes to an element's fields in place do not bump: the element keeps
  its identity and the view still names it. Reason: house style section 1
  says an element's lifetime ends at removal even if its bytes remain, and
  a reorder within capacity makes `children[0]` a different control without
  any memory event. "Relocating only" would reproduce what ASan already
  sees and miss the rest.
- **Reuse of the existing counters.** `Control::children_revision_` and
  `CollectionCell::revision_` already advance on exactly these changes.
  In the checked build the owner calls `invalidate_borrows` beside each
  advance (one line at each site); the counters keep their present meaning.
- **Owner death:** the stamp holds a `std::shared_ptr` to the state record
  only. The owner's destructor (through `~BorrowOwner`) clears `alive`.
  The record outlives the owner while any view holds it, so a use after the
  owner's death reads `alive == false` instead of freed memory. A
  `std::weak_ptr` would also work (expired means dead) but would turn the
  diagnosis into "unknown owner" and costs a lock per check; the shared
  record keeps the creation and invalidation sites for the report.
- **Back links** (`window()`, `surface_root()`): the stamp is the
  control's attachment state (bumped on attach and detach), and the
  checked `BorrowedPointer<Window>` also stamps the window's own
  `BorrowOwner` (death). Retention across callbacks is not a borrow at
  all: it goes through a weak handle (section 2.5).
- **Call-scope borrows** (event payloads, lent caller text): the owner is
  a named scope object for the raise or the call; its destructor bumps the
  generation. A handler that copies the payload and keeps it fails on its
  first later use.

### 2.3 Cost in the checked build

One `std::shared_ptr` copy (atomic increment) per view made, one
comparison per access. Views are made per call, not per element, so the
copy is outside the loops (house style section 8). Size of a checked span:
pointer, size, shared pointer (two words), generation, site: 48 bytes on
64-bit. Acceptable for a sanitizer tree, which already runs ASan and UBSan.

### 2.4 Thread rules

- Default: an owner and its views belong to the owner's thread (controls,
  surface roots and collections are UI-thread state; house style section 5,
  one execution context per piece of mutable state). The stamp records the
  thread at creation; a check on another thread fails with
  `BorrowFault::wrong_thread`. The state record needs no atomics.
- Exception: lease types used across threads (`LiveSurfaceFrame`,
  `LiveSurfaceWriteLease`, `ImageSnapshot`) lend views whose owner is an
  immutable shared buffer; their state record uses an atomic generation and
  skips the thread check (`BorrowThreading::any_thread`), chosen per owner
  type at compile time.

### 2.5 Which APIs change shape, and which stop lending

| API | Decision | Why |
|---|---|---|
| `Control::children()` | `Control::ChildView` (`BorrowedSpan<Ptr>`); add `child_count()` and `child_at(index)` returning `Ptr` (a shared copy) for walks that call out | hot path, indices plus `children_revision()` already used by walks; copy-out would allocate per walk |
| `window()`, `surface_root()`, `attached_window()`, `attached_surface_root()` | `BorrowedPointer<Window>`/`BorrowedPointer<SurfaceRoot>`; add a weak `WindowRef` built on `detail::WindowLifetime` for anything kept across a callback | the ABI tables (row 8) keep `Window*` across callbacks today |
| `CollectionCell::read()`, `owner_elements()` | `BorrowedSpan<E>`/`BorrowedMutableSpan<E>` with the cell's `revision_` | one template covers every collection property |
| Item-list `items()` (row 4) | `BorrowedSpan<Item>`; the `const std::vector&` siblings become `BorrowedSpan` too | same shape, public, consumers iterate and edit |
| `TextStore::utf8()` | `BorrowedText` | edits invalidate; text code keeps offsets, which stay valid as numbers |
| Event payload fields (row 6), `PartPaint::text` | `BorrowedSpan`/`BorrowedText` fields made by the raise or paint scope | the lifetime moves from a comment into the type |
| Pixel views (row 9) | `BorrowedMutableSpan<std::byte>` stamped by the raster or lease | stage 3 |
| Diagnostics and records (row 11) | copy-out: `fill_layout_diagnostics(std::vector<LayoutDiagnostic>& destination)` (house style section 6, caller-supplied storage), or keep the documented span with `&&` deleted | test-facing, cold |
| Layout workspaces (row 12) | `BorrowedSpan` stamped per solve | cheap, internal, stage 3 |
| Static tables (`property_table()`, anatomy) | unchanged `std::span` | static storage; nothing to check |
| `ControlTable::control_at` | unchanged | already generational: the model the rest follows |
| C ABI | unchanged outward; in the checked build the library wraps caller-lent text as call-scope borrows, and scribbles and ASan-poisons its own lent scratch (event fields and their text) when the invoke returns (`ASAN_POISON_MEMORY_REGION`) | a C or C# caller that keeps lent text then reads poisoned memory and ASan reports it in the caller's frame |

### 2.6 Selecting the checked mode

- CMake option `GUI_FORMS_CHECKED_BORROWS` (OFF by default), set ON by
  `tools/ci_build.py --sanitize`, so the persistent sanitizer tree on the M4
  (owner ruling 2026-11-11, CP29 plan) and CI's sanitizer job run checked.
  The definition is PUBLIC on the targets and recorded in the installed
  `GUIFormsConfig.cmake`, so a consumer built against a checked SDK is
  checked too.
- Mixing modes is an ODR violation (the types differ). The checked headers
  odr-use an exported marker symbol (`gui_forms_checked_borrows_v1`) that
  only a checked library defines, so a mixed link fails at link time instead
  of misbehaving.
- The stx packages are not involved (no GUI.Forms views cross into them).

### 2.7 Diagnostic path

- A failed check calls `detail::fail_borrow(const BorrowStamp&, BorrowFault)`,
  `[[noreturn]]`, which writes one report to stderr and ends the process
  with `std::abort`, the same contract as `detail::fail_fast`
  (`src/core/failure/fail_fast.hpp`, owner 2026-11-11): nothing is counted,
  nothing continues, nothing unwinds.
- The report names the fault (`structurally_invalidated`, `owner_destroyed`,
  `wrong_thread`, `out_of_range`), the owner's type, the creation site and
  the last invalidation site (both return addresses captured with
  `__builtin_return_address(0)` in the checked accessor and in
  `invalidate_borrows`, then symbolized), and the current stack through the
  stack printer `fail_fast` already uses (`__sanitizer_print_stack_trace`
  under ASan). No signature change is needed for the sites, so the public
  declarations stay identical in both modes.
- Tests: a death-test fixture per fault (the four faults on `children()`,
  `CollectionCell::read()`, a back link and a call-scope payload) under the
  checked tree; the ordinary tree compiles the same tests with the death
  cases skipped.

### 2.8 The lifetime-rule gap the inventory found

`tools/check_lifetime_rules.py` enforces "a function returning a borrow of
an argument deletes its rvalue overload" for free functions. Member
accessors are the same rule with `*this` as the argument: 40 of the 58
inline span accessors (for example `PartGeometryBuffer::parts()`,
`Control::children()`, `DamageRegion::rectangles()`) have no `const&`
qualifier and no deleted `const&&`. No binding from a temporary exists
today (searched: no `f().parts()`-style binding or range-for), but the
checker should cover member accessors returning a view of a member; it is
an S item in stage 1.

### 2.9 Workstream I and the compatibility policy

- Workstream I (CP29 phase 2: F1 bases, F2 flat headers, B10 SurfaceRoot
  split, export annotations) rewrites every public header once. The borrow
  aliases should land **in that pass or before it**, not after: each public
  accessor's return type is written once, and the re-layout's own
  `-ftime-trace` follow-up already moves the span accessors (section 2.1).
  Recommended order: the alias header and `BorrowOwner` (stage 2a) land on
  cp28 main before I starts, so I carries them; the per-API conversions
  ride I's moves of each header.
- Compatibility policy (shape fixed at 1.0, then additions only): in the
  ordinary build the aliases are the plain views, so converting an API is
  source-compatible for every consumer and changes no ABI. The checked
  build is a sanitizer mode, not a product mode, and may tighten after 1.0
  without breaking anyone. What must be fixed before 1.0 is the set of
  public **names** (`BorrowedSpan`, `BorrowedText`, `BorrowedPointer`,
  `unchecked_view`, the per-owner aliases such as `Control::ChildView`) and
  which accessors keep lending versus copying out. The C ABI is unaffected.

## 3. Clang lifetime lint run (cp28)

### 3.1 Setup

- Tree `/Volumes/Storage/gui_forms_workers/w5/.build/lifetime-lint` on the
  M4, configured as `tools/ci_build.py`'s plain macOS tree (Release,
  `cmake/llvm22.cmake`, gallery and tests on) with `CXXFLAGS` and
  `OBJCXXFLAGS` set to `-Xclang -fexperimental-lifetime-safety
  -Wexperimental-lifetime-safety -Wexperimental-lifetime-safety-strict`
  (CMake folds them into the toolchain's flags; the cache shows them in
  `CMAKE_CXX_FLAGS`). The integration tree's stx package at stx.lock's
  commit (1f5c6f54) is linked read-only, so third-party code is not
  analysed. Script: `.tmp_lifetime_lint.py` (untracked, in the workspace).
- `cmake --build --parallel 2 -- -k 0`, no `-Werror` added: 2351 of 2352
  edges, no failed edge, exit 0, 1464 s of wall time while the integrator
  held the other M4 slot. Warnings deduplicated by file, line, column and
  flag (a header warning counts once).
- A probe with the same flags on `/tmp/lt/p.cpp` confirms the analysis is
  live in this compiler: it reports the returned string view of a local
  and the use after scope, and not the span used after `push_back` (even
  with `[[clang::lifetimebound]]` on the accessor), nor the pointer into a
  vector used after `push_back`.

### 3.2 Counts

| Warning kind | Unique findings |
|---|---|
| `-Wexperimental-lifetime-safety-permissive`: "address of stack memory is returned later" | 4 |
| `-Wexperimental-lifetime-safety-permissive`: "object whose reference is captured does not live long enough" | 2 |
| `-Wexperimental-lifetime-safety-strict` | 0 |
| (unrelated, from the tree's existing warning level: `-Wmissing-designated-field-initializers` 720, `-Wmissing-field-initializers` 80, `-Wunnecessary-virtual-specifier` 57, unused-* 66, 9 others) | 928 |

Per directory (lifetime findings only): `src/controls` 4, `src/render` 2;
`include/`, `src/core`, `src/abi`, `src/host` (including the Objective-C++
host), `tests`, `demo` and the demoboard 0.

### 3.3 Every finding

All six are one false-positive shape: a raw pointer or reference taken
through a local `std::unique_ptr` (`get()` or `*`), after which the
`unique_ptr` is moved into a container that owns it from then on. The
analysis models the result as a borrow of the local `unique_ptr` object
rather than of its heap pointee, so the move looks like the end of the
borrowed object's life. The pointee is never freed by the move.

| # | File:line | Shape | Real? |
|---|---|---|---|
| 1 | `src/controls/rules/rule_set/rule_set.cpp:537` | `made.get()` then `taps_.push_back(std::move(made))`, pointer returned | no |
| 2 | `src/controls/guidance/help_provider/help_provider.cpp:118` | `Entry& result = *entry;` then `entries_.emplace(..., std::move(entry))`, reference returned | no |
| 3 | `src/controls/guidance/error_provider/error_provider.cpp:150` | same as 2 | no |
| 4 | `src/controls/ribbon_tool_tip/ribbon_tool_tip.cpp:170` | same as 2 | no |
| 5 | `src/render/skia/raster/skia_raster.cpp:1806` | `memo = replacement.get();` then `(*cache).memo = std::move(replacement);`, used at :1812 | no |
| 6 | `src/render/skia/raster/skia_raster.cpp:2007` | same as 5, used at :2010 | no |

The top-15 list asked for has six entries because the build has six
lifetime findings. Real findings: none.

### 3.4 What this says about the lint

- The LLVM 22 analysis is intra-procedural and covers stack lifetimes. It
  finds nothing in the inventory's subject (views of heap containers
  invalidated by a member call), because it does not model a member call
  such as `add_child` as invalidating `children()`'s result. The design in
  section 2 does not depend on it.
- Its value is as a cheap regression gate for the stack cases (returned
  views of locals, use after scope), which ASan sees only when the path
  runs. The `unique_ptr`-move shape needs a baseline entry, or the six
  sites can take the pointer after the move (`taps_.back().get()`), which
  is also clearer: an S item each, optional.
- Re-run the job at each LLVM pin change; a later analysis may model
  container invalidation and find some of the inventory statically.

## 4. Staged plan

Sizes: S one function, C one file or mechanism, B crosses subsystems.

| Stage | Item | Size |
|---|---|---|
| 1a | Lint tree as an M4 job: the configure in `.tmp_lifetime_lint.py` (separate tree, stx package linked read-only, `ninja -k 0`, deduplicated finding table), report-only, run with the persistent sanitizer tree | C |
| 1b | A suppression baseline in the `tools/warning_baseline.json` style for the false-positive shapes (section 3), then gating: a new finding fails the job | C |
| 1c | `check_lifetime_rules.py` covers member accessors returning a view of a member (`const&` plus `const&& = delete`) and the 40 accessors gain the pair | C |
| 2a | `gui_forms/borrowed.hpp` (alias family, checked classes, checked iterator, `unchecked_view`), `detail::BorrowOwner`, `fail_borrow`, the CMake option, the link-time marker, death tests | B |
| 2b | Top-ranked conversions: `Control::children()` (+ `child_count`/`child_at`, the 26 bound sites), back links (+ `WindowRef`, the ABI tables of row 8), `CollectionCell::read()`/`owner_elements()`, `TextStore::utf8()` | B |
| 2c | Item-list `items()` accessors and their `const std::vector&` siblings; event payload fields and `PartPaint::text` as call-scope borrows | C each, B in all |
| 3a | Pixel views and leases (row 9), layout workspaces (row 12), remaining member views | B |
| 3b | Copy-out for diagnostics and records (row 11) | C |
| 3c | C ABI checked mode: call-scope wrapping of caller-lent text, poisoning of lent scratch after invoke | C |

## 5. Open owner decisions (recommendation first)

1. **Generation policy.**
   - a. (recommended) Every structural change bumps (insert, erase,
     reorder, assign, clear, capacity change, attach/detach), matching house
     style section 1.
   - b. Relocating changes only (capacity change, replacement): fewer
     reports, but in-capacity shifts stay silent, which is the case ASan
     already misses.
2. **Public shape.**
   - a. (recommended) Aliases that are the plain views in the ordinary
     build and checked classes in the sanitizer build, with no implicit
     conversion to `std::span` in the checked build.
   - b. Distinct classes in both modes (always `BorrowedSpan`, zero-cost
     wrapper): stronger typing, but every consumer passing a view to a
     `std::span` parameter changes, and the type is new public vocabulary in
     the ordinary build.
   - c. Index-based APIs everywhere (`child_count`/`child_at` only, no
     views): no borrows left to check, at the price of a shared-pointer copy
     per element and a different idiom from WinForms-style collections.
3. **Timing against workstream I.**
   - a. (recommended) Stage 2a lands before workstream I; conversions ride
     I's header moves.
   - b. After I: every public header is touched twice.
4. **Lint gating.**
   - a. (recommended) Report-only until the false-positive shapes have a
     baseline (stage 1b), then gating on new findings in the M4 job only
     (not every CI runner: the analysis is experimental in LLVM 22 and adds
     compile time).
   - b. Gate immediately with `-Werror=experimental-lifetime-safety*`.
   - c. Keep it a manual study tool.
5. **Owner death detection.**
   - a. (recommended) A shared state record kept by the stamp, with the
     sites for the report.
   - b. A weak pointer per stamp: the same detection, poorer reports.
