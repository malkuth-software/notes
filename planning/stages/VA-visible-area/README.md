# VA: the visible-area guarantee

Written by the coordinator, 2026-11-21, from the owner's direction of the
same day. Status: a design study; nothing below is decided beyond the
owner's direction.

## 1. Why (the owner's example)

The owner played a casual web game in a browser window that was not full
screen. The bottom tenth of the game was off the screen, and the game had
no way of knowing. The owner had to zoom the browser out and then centre
the content vertically by hand. Televisions, monitors and browsers have some
notion of a viewport and a minimum visible extent, but nothing guarantees
that content **is on the screen**.

## 2. The owner's direction (2026-11-21)

- GUI.Forms should be able to promise application designers that content is
  always dynamically appropriate to the area it renders into: never
  invisibly stretched, never extending off screen or underneath other
  controls or sections of the program.
- Responsive layout is not enough. The game's problem needed a **contain**
  operation: scale by `min(w_view / w_content, h_view / h_content)` and centre
  in the leftover space. "The contain operation as an interface guarantee."
- It might be **fully compositional**: each root makes the same promise
  relative to its viewport.
- The point is not to force content onto the screen. It is that nothing can
  ever be too wide or too tall by itself, as content; the engine knows when
  content is too wide or too tall, and can fit it (scale it, not change its
  dimensions) so that images, videos and the like are guaranteed to fit.
- **The architectural question:** should the guarantee be met by a final
  rendering transform, or should visibility be a constraint propagated
  backward into layout?

## 3. What GUI.Forms already has

- Real dynamic layout (stage L): FlexPanel, GridPanel, BlockPanel,
  positioning, LayoutDirection; Windows Forms anchors and docking.
- Scrolling containers, smooth scrolling, scroll copies.
- Owned overlays (menus, drop-downs, tooltips) placed against the window and
  the screen; Window and Popup placement against the desktop work area.
- Retained rendering with transforms in the painter (scales, rotations), and
  the theme engine's part geometry.
- Hosts that report the window's size and scale, and the desktop's work area
  (`desktop_coordinates.hpp`).

What is missing is a stated invariant, checked by the engine, that a
subtree fits its viewport, and a defined response when it does not.

## 4. The two architectures

**A. A final rendering transform.** Layout runs as today at the content's
natural size. If the laid-out subtree exceeds its viewport, the renderer
applies a uniform scale `s = min(1, w_view / w_content, h_view / h_content)`
and a centring translation. Input is mapped back through the inverse.
- For: simple; exact for the game case; works for content that has a fixed
  design size (games, video, images, documents shown whole, a fixed-size
  instrument panel); no change to layout's contract.
- Against: text and controls shrink below legible or touchable sizes; it
  hides a layout problem instead of solving it; hit areas and accessibility
  bounds must follow the transform (they can: the semantic bounds are already
  computed by the window).

**B. A constraint propagated backward into layout.** The viewport's size
becomes an upper bound every subtree receives; each control either fits
(wraps, shrinks within its minimums, scrolls) or reports that it cannot,
and its parent decides (scroll, contain-scale, elide, reflow).
- For: content stays at legible sizes where it can; the engine knows exactly
  which subtree overflowed and why (it can report it, in metrics and to the
  MCP inspection of the MCP study's section 18); it composes, since each
  container passes its own bound down.
- Against: every container needs a defined overflow policy; a subtree with a
  fixed design size (a game canvas) still needs A at its own level.

**Likely answer, for the study to test:** both, as one model. Layout
propagates the bound (B) and records overflow; each viewport-owning node
(a window, a scroll viewport, a media or canvas control, any root) has an
**overflow policy**: `scroll`, `contain` (A, uniform scale and centre),
`cover`, `clip` (only by explicit choice) or `reflow`. The guarantee is then
stated as: **no content is ever outside its viewport unless the policy
explicitly allows it, and every overflow is known to the engine.** `contain`
is the default for content with a fixed design size, and is what the game
needed.

## 5. What to work out

1. The invariant precisely: which nodes own viewports; what "fits" means with
   insets, safe areas (notches, overscan on televisions), the window band and
   owned overlays.
2. The overflow policies and their defaults per control kind (images, video,
   canvases, plots and documents default to `contain`; panels to `reflow` then
   `scroll`).
3. Composition: nested viewports each keep the promise relative to their own
   viewport; nested `contain` scales multiply; minimum legible scale and
   minimum touch size (touch stage TC) bound how far `contain` may shrink
   before it switches to `scroll`.
4. Input and accessibility under a contain transform: hit testing, touch
   areas, semantic bounds and captures map through it.
5. Reporting: overflow as structured metrics, visible in the invalidation
   overlay and to MCP.
6. The host side: the window's size and the screen's work area; a window
   larger than the screen (the browser case) is itself an overflow the
   window manager or host can answer with `contain`.
7. The public API and the C ABI fields this adds, which must land before the
   1.0 ABI freeze.

## 6. Placement

Order of operations step 6 (implement and resolve outstanding subjects):
the study first, then the API before the ABI freeze. It shares ground with
touch (TC: minimum touch sizes) and with the MCP study's inspection
(overflow reports).
