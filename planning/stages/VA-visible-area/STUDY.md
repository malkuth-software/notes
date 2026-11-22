# VA study: the visible-area guarantee

Written by the VA/IC study worker, 2026-11-22, from the brief beside it
(`README.md`) and a reading of GUI.Forms at `main` c7c9f4f0c (the cp35
merge). Code-free: nothing in GUI.Forms was changed. Status: decided 2026-11-22 (section 13): the owner
accepted every recommendation except question 4, where Windows Forms
panels contain by default.

## 1. The problem

The owner's example: a web game in a browser window that was not full
screen lost its bottom tenth off the screen, and nothing in the stack knew.
The owner's direction: GUI.Forms promises application designers that
content is always appropriate to the area it renders into: never invisibly
stretched, never off screen, never under other controls or sections; and
"the contain operation as an interface guarantee": scale by
`min(w_view / w_content, h_view / h_content)`, centre in the leftover space,
possibly compositionally, each root promising the same relative to its
viewport. The aim is not to force content onto the screen but that the
engine always **knows** when content is too wide or too tall and can fit it
by scaling, not by changing its dimensions.

Stated as one invariant (section 4 refines it):

> No content is ever outside its viewport unless the viewport's owner
> explicitly answers that overflow (scroll, contain, cover or clip), and
> every overflow is recorded by the engine.

## 2. Owner record and prior work

- Owner record searched (`owner_record/shadow_harvest_2026-11/*.md`,
  `desks/coordinator/DECISIONS_2026-11-13/16/22.md`, the notes `planning/`
  tree): nothing on VA beyond the brief. No earlier VA study, branch or
  prototype exists.
- Every source the brief names exists and is current: stage L's panels,
  `desktop_coordinates.hpp`, the invalidation overlay
  (`diagnostics/invalidation_overlay/`), the MCP study's section 18
  (18.1 "layout reasons" is where overflow reports belong) and the touch
  brief's themeable minimum target (TC, "a minimum of 9 mm square").
- One piece of prior art the brief does not name: **`ScaledPanel`**
  (section 3.4). It is the nearest thing to `contain` in the tree today and
  it does the opposite of what the owner asks.

## 3. What exists today

### 3.1 Roots already render at their own scale

- `SurfaceRoot` (`surface_root/surface_root.hpp`) keeps a logical size and
  a `scale` (device pixels per logical unit, accepted in (0, 8]);
  `set_surface_scale` re-marks the whole tree and every control records
  afresh at the new scale. Captures at another scale already re-record
  controls at that scale (`transient_recording_scale_`, exhibit gap E2).
- A `SubSurface` is a nested root with its own logical size and scale
  (`SubSurfaceConfiguration{logical_size, scale}`); its frames are copied
  1:1 into the host region ("a copy, never a scale"). Its input is
  forwarded by the host and its semantics published as virtual nodes
  (`sub_surface_host.hpp`, sections 4 and 5 of its design note).
- Consequence: **a contain at a root costs almost nothing new.** Give the
  root `logical_size = content design size` and
  `scale = device_scale × s`; it records crisply at the smaller scale (text
  shaped and hinted at the real pixel size, not a shrunk bitmap), and input
  maps back by one division. Windows, sub-surfaces, native popups and warm
  Popup windows are all roots.

### 3.2 Inside one root, frames are translation only

- The window's walks (hit testing, paint cover, replay, semantic bounds)
  carry a `Point parent_origin` per level: `hit_test_recursive`,
  `find_paint_cover` and their neighbours in `src/core/window/window.cpp`
  (around lines 4777, 5361, 5454, 5508) add the control's placed bounds and
  its `child_viewport_rectangle()` and recurse.
- The painter has **scoped transform groups** (`types/painter/painter.hpp`,
  `PaintTransformGroup`; owner 2026-11-16: "a scoped transform, never a
  free concatenation"), but only inside one control's own paint.
- Content effects (`Control::set_content_effect`) already draw a control
  **and its descendants** into one group composited as a unit, with damage
  grown by the effect's reach. That is the seam a subtree scale would use.
- Parents clip descendants to their client rectangle (`control.hpp`,
  `visual_outsets`), silently.

### 3.3 Layout knows sizes but not overflow

- `measure` / `arrange` and stage L's `LayoutConstraint`
  (`layout/types/layout_types.hpp`: `definite`, `at_most`, `min_content`,
  `max_content` per axis), `minimum_size` / `maximum_size` on every control.
- A structured diagnostics channel already exists:
  `LayoutDiagnosticCode` (`layout/diagnostics/layout_diagnostics.hpp`,
  codes 201 `containing_block_not_parent`, 202 `arrange_outside_slot`),
  kept per window (`SurfaceRoot::layout_diagnostics()`, bounded at 1000,
  later ones counted), and layout counters in `MetricsSnapshot`
  (`layout_boundary_stops`, `layout_measure_clamps`,
  `layout_arrange_refusals`).
- Nothing records "this subtree's minimum content exceeds its viewport".

### 3.4 Overflow answers exist, one per control, each its own way

| Where | Answer |
|---|---|
| `ToolStrip` (`ToolStripOverflow`) | items move to an overflow menu, or are cut off |
| `PageStrip` (`PageStripOverflow`, default `scroll`) | tabs scroll |
| Ribbon (RB), `CommandOverflowPanel`, `NavigationPane` | compaction, overflow area |
| `ScrollableControl::auto_scroll` | scroll (Windows Forms' `AutoScroll`) |
| `PictureBox` (`PictureBoxSizeMode::zoom`) | a leaf contain for one image |
| `RasterCanvas::zoom`, plot views | the control's own view transform |
| `FlexPanel` (`overflow_shift_`) | keeps a reversed axis' overflow reachable |
| `ScaledPanel` (`controls/scaled_panel.hpp`) | **scales child bounds per axis**: `scale_x = w_final / w_design`, `scale_y = h_final / h_design` (`scaled_layout_utilities.hpp:49`) |

`ScaledPanel` stretches non-uniformly and re-lays its children at the
stretched bounds (text is not scaled, only boxes). It is "invisibly
stretched" by design. It stays legitimate as an authored choice, but it is
not the guarantee.

### 3.5 Hosts reproduce the owner's example

- `clamp_to_work_area` (`host/windowing/host_windowing.hpp:154`): "a
  rectangle wider or taller than the work area is aligned with its left or
  top edge. The size never changes." A window taller than the screen loses
  its bottom, exactly the owner's game. `native_popup.cpp:137` uses it for
  popups too.
- The single-surface window manager (`src/host/surface/manager/`) sets each
  monitor's `work_area = frame`.
- The browser host (`src/surface/web/page/web_host.js`) sizes the surface to
  the canvas' bounding box (`getBoundingClientRect`, `ResizeObserver`), not
  to the part of it the browser shows (`visualViewport`); the page shell
  sizes the canvas to 100 % of its container (`web/paint/shell.css`). A page
  that places the canvas partly off screen is not detected.
- No host reports **safe insets** (notches, rounded corners, television
  overscan, a browser toolbar over the canvas). The word does not occur in
  the tree.

### 3.6 The C ABI

- Properties are read and written by name (`gf_properties_table.get/set`),
  so a new property on `Control` adds no ABI entry; its enum reaches the
  bindings through the schema.
- Neither `MetricsSnapshot` nor the layout diagnostics reach the C ABI
  today. Overflow reports therefore need new, appended entries, and so do
  safe insets. All of this must land before the 1.0 ABI freeze.

## 4. The invariant, precisely

**Viewport owners.** A control owns a viewport when it places descendants
inside a rectangle it can answer for: every root (window client area, a
sub-surface, a native popup, a warm Popup window, an overlay), every
`ScrollableControl`, every layout container, every media or canvas control
that shows content of a fixed design size. Its viewport is its
`child_viewport_rectangle()`; at a root, minus the host's safe insets and
minus the window band (the in-surface title band is not client area).

**Fits.** A descendant fits when its painted extent (arranged bounds grown
by `visual_outsets()`), mapped into the owner's frame, lies inside the
viewport. Owned overlays (menus, drop-downs, tooltips) are not "under"
anything: they are their own roots and keep the same promise against the
work area.

**Answers.** When layout cannot make the content fit (its minimum content,
after reflow, exceeds the viewport, or it has a fixed design size larger
than the viewport, or an absolutely placed child lies outside), the owner's
policy answers:

- `scroll`: content keeps its size; the scroll range reaches every part;
- `contain`: one uniform scale `s = clamp(min(w_v / w_c, h_v / h_c),
  minimum_scale, maximum_scale)` and alignment in the leftover (centred by
  default); below `minimum_scale` the policy's stated fallback answers
  (normally `scroll`);
- `cover`: uniform scale to fill, the excess cropped: an explicit, recorded
  crop;
- `clip`: content keeps its size and is cut: allowed only by explicit
  choice, always recorded.

Reflow is not an answer; it is what layout does first (architecture B).
The policy is what happens when reflow is not enough.

**Composition.** Nested owners keep the promise against their own
viewports. Nested contain scales multiply; `minimum_scale` is checked
against the **effective** scale (the product up to the root, times the
device scale for touch and text sizes), so an inner contain cannot escape
the floor by nesting.

**Checked.** After each layout pass the window verifies the arranged
subtrees it laid out (the dirty ones, so the cost follows the change):
every overflow is either answered by its owner's policy or recorded as
unanswered. A healthy program reports zero unanswered overflows; that
counter is the guarantee's evidence.

## 5. The architecture question

The brief asks: a final rendering transform (A), or visibility as a
constraint propagated backward into layout (B)?

**Answer proposed: both, as one model,** split by where they apply:

- B everywhere: the viewport's size is the bound every subtree is laid out
  under (it already is, through `measure(available)` and
  `LayoutConstraint::at_most`); what is new is that a subtree whose minimum
  content exceeds it is **recorded** and handed to its owner's policy
  instead of silently clipped.
- A at viewport owners whose policy is `contain` or `cover`: a uniform
  scale and translation applied to the subtree as a frame, with input,
  hit testing, captures and semantics mapped through its inverse.

What A costs depends on where the owner is:

| Owner | Mechanism | New engine work |
|---|---|---|
| A root (window, sub-surface, popup) | root scale `device × s`, logical size = design size | almost none (3.1) |
| A control inside a root | a **scale frame** in the window's walks: `parent_origin` (a point) becomes a frame `{origin, scale}`, uniform scale and translation only, no rotation, so rectangles stay rectangles | hit testing, cover, replay, damage, semantic bounds, captures, IME and caret rectangles, tooltip and popup anchors; the subtree records at the effective scale |

Restricting the frame to uniform scale plus translation keeps every
existing rectangle computation a rectangle computation, which is why the
in-tree case is tractable at all.

## 6. Rendering a contained subtree

- **Re-record at the effective scale** (recommended): the subtree's
  controls record at `window_scale × s`, as captures at another scale do
  today. Text stays crisp and hinted at its real pixel size; a change of
  `s` re-records the subtree once. For roots this is what already happens.
- **Scale the retained chunks**: cheap, but glyphs and images cached at the
  old scale blur or alias; acceptable only for transient animation of `s`.

## 7. Host side

- **Window size.** A window larger than its work area is itself an
  overflow. Proposed: the host clamps the **size** as well as the position
  (`clamp_to_work_area` keeps its name and gains the size rule, or a sibling
  `fit_to_work_area` is added), and the window's root policy answers inside
  the clamped client area. A `minimum_size` larger than the work area
  yields to it and is recorded (`minimum_size_exceeds_work_area`).
- **Safe insets.** A host capability reporting the window's obscured edges
  (`SafeInsets`, zero on hosts without any): macOS full screen with a
  notch, a television's overscan, the browser's visual viewport against
  the canvas.
- **Browser.** The web host also reads `visualViewport` and reports the part
  of the canvas the browser hides as safe insets, so the owner's game case
  is detected and answered by the root's `contain`.

## 8. Defaults per control kind

| Kind | Default answer |
|---|---|
| Media and fixed-design content: `PictureBox` in zoom mode, video, `RasterCanvas` whole-image view, plot export views, a game or sub-surface with a design size | `contain`, `minimum_scale` 0 (always fits), `maximum_scale` unbounded |
| CSS panels (`FlexPanel`, `GridPanel`, `BlockPanel`) | reflow, then `scroll` |
| Windows Forms panels (absolute, Dock and Anchor) | `scroll` when `auto_scroll` is on; otherwise **`contain`** (owner, question 4: a deliberate difference from Windows Forms, which clips) |
| Strips, ribbon, navigation | their own overflow answers (3.4), reported through the same record |
| Top-level window root | reflow, then `scroll` (question 7) |
| Overlays and menus | `scroll` within the work area |

`PictureBoxSizeMode::zoom` becomes the same `contain` computation, not a
second implementation.

## 9. Input and accessibility under contain

- Pointer, touch and wheel positions map through the inverse frame before
  hit testing and before they reach handlers; handlers keep seeing local,
  unscaled coordinates.
- Captures and drags keep the frame taken at capture time; a frame change
  during a capture re-maps the next event, it does not cancel.
- Semantic bounds published to the accessibility hosts and to MCP are the
  scaled rectangles; a node records its effective scale.
- Touch: TC's minimum target is compared against `target × effective
  scale`; below it the floor applies (question 5).
- Text input: the IME's caret and composition rectangles map through the
  frame.

## 10. Reporting

- A new layout diagnostic, `content_overflow` (203), recorded when an
  overflow first appears or its answer changes (not every pass), naming the
  owner, the content and the viewport extents, the answer and the applied
  scale.
- Metrics counters: `overflow_scrolled`, `overflow_contained`,
  `overflow_covered`, `overflow_clipped`, `overflow_unanswered`.
- The diagnostic overlay outlines overflowing owners (red for unanswered,
  amber for clipped).
- MCP (`inspect`, section 18.1 "layout reasons") reads the records.

## 11. API sketch (illustrative; house style spellings)

```cpp
namespace gui_forms {

// What a viewport owner does when its content, after layout, does not fit.
enum class OverflowAnswer : std::uint8_t {
    scroll = 0,   // content keeps its size; the scroll range reaches all of it
    contain = 1,  // one uniform scale within [minimum_scale, maximum_scale], aligned
    cover = 2,    // one uniform scale filling the viewport; the excess is cropped
    clip = 3,     // content keeps its size and is cut; always recorded
};

// A viewport owner's policy. Independent options with valid defaults.
struct GUI_FORMS_API ViewportFit final {
    OverflowAnswer answer{OverflowAnswer::scroll};
    // Bounds on the effective scale (the product of every contain scale up
    // to the root). Contain below minimum_scale takes below_minimum instead.
    double minimum_scale{0.0};
    double maximum_scale{1.0};
    OverflowAnswer below_minimum{OverflowAnswer::scroll};
    ContentAlignment alignment{ContentAlignment::middle_center};

    friend bool operator==(const ViewportFit& left, const ViewportFit& right) noexcept;
};

// On Control, generated by property_codemod like every property:
//   PropertyCell<ViewportFit, ...> viewport_fit;    default per kind (section 8)
//   PropertyCell<Size, ...> design_size;            0 x 0: the content's own extent

enum class OverflowCause : std::uint8_t {
    minimum_content_exceeds_viewport = 0,
    design_size_exceeds_viewport = 1,
    placed_child_outside_viewport = 2,
    minimum_size_exceeds_work_area = 3,
};

struct GUI_FORMS_API OverflowRecord final {
    std::string owner{};          // stable id of the viewport owner
    OverflowCause cause{OverflowCause::minimum_content_exceeds_viewport};
    Size content_extent{};
    Size viewport_extent{};
    OverflowAnswer answer{OverflowAnswer::scroll};
    double effective_scale{1.0};
    bool answered{};              // false only for an unanswered overflow
};

// Host capability: the edges of the client area the host or device hides.
struct GUI_FORMS_API SafeInsets final {
    double left{};
    double top{};
    double right{};
    double bottom{};
};

// SurfaceRoot:
//   std::span<const OverflowRecord> overflow_records() const& noexcept;   (&& deleted)
//   SafeInsets safe_insets() const noexcept;
//   Rect visible_client_rectangle() const noexcept;   client minus band and safe insets
// MetricsSnapshot: overflow_scrolled, overflow_contained, overflow_covered,
//   overflow_clipped, overflow_unanswered.
// LayoutDiagnosticCode::content_overflow = 203.

} // namespace gui_forms
```

C ABI (appended, `GF_ABI_MINOR` bumped in fold order):
`gf_overflow_record` and `gf_safe_insets` records; in `gf_diagnostics_table`,
`overflow_records(window, records, capacity, required)` (the caller-supplied
storage pattern the table already uses for `shadow_frames`); in
`gf_windows_table`, `safe_insets(window, insets)`. `viewport_fit` and
`design_size` travel through the existing property calls; the enum and the
record kind go into the schema and the member manifest.

The names are proposals (question 9).

## 12. Phases

| Phase | Content | Lands |
|---|---|---|
| VA1 | the API and ABI of section 11; detection and records (diagnostic 203, counters, unanswered = 0 check) for every owner; `clip` and `scroll` reported; contain and cover at **roots** through the root scale; window size clamp | before the ABI freeze |
| VA2 | the in-tree scale frame: hit testing, cover, replay, damage, semantics, captures, IME, anchors; re-record at the effective scale; contain on any control | before 1.0 |
| VA3 | defaults per kind (section 8); `PictureBox` zoom and the canvases onto the shared computation; safe insets on the hosts that have them; the web host's visual viewport | before 1.0 |
| VA4 | diagnostic overlay colours; MCP layout reasons; gate: the owner's game case reproduced (a too-tall root in a too-small browser canvas) failing before and passing after, plus nested-contain, floor and input-mapping tests in gui_forms_tests | gate |

## 13. Decisions (owner, 2026-11-22)

The owner answered: "contain by default on 4. otherwise accepted."

1. **Both as one model**: layout propagates the bound and records overflow;
   each viewport owner's policy (scroll, contain, cover, clip) answers what
   reflow cannot fit.
2. **Contain on any control**, phased: roots first (VA1), the in-tree scale
   frame second (VA2), both before 1.0.
3. **The viewport excludes the window band and host-reported safe insets**
   (zero where a host has none).
4. **Windows Forms panels contain by default** (absolute, Dock and Anchor
   panels without `auto_scroll`): content outside them is scaled to fit,
   not clipped. A deliberate difference from Windows Forms; `clip` remains
   available by explicit choice and is recorded. The Windows Forms
   compatibility notes say so.
5. **Scale range with a floor on the effective scale**: `minimum_scale`,
   `maximum_scale`, and `below_minimum` (normally scroll). Media defaults to
   minimum 0; control content to a floor from the presentation settings,
   bounded below by TC's minimum target.
6. **Re-record at the effective scale**; scaling retained chunks only for
   transient animation of the scale.
7. **Hosts clamp an oversized window's size** to the work area; the root's
   policy answers inside; a minimum size that cannot fit yields and is
   recorded.
8. **Diagnostic 203 `content_overflow`, the counters (unanswered included)
   and the C ABI entries for records and safe insets before the freeze**;
   MCP layout reasons later.
9. **Names as section 11**: `viewport_fit` (`ViewportFit`), `OverflowAnswer`,
   `design_size`, `OverflowRecord`, `OverflowCause`, `SafeInsets`.
10. **`ScaledPanel` stays as an explicit, recorded stretch** and gains an
    aspect setting with a uniform mode.
