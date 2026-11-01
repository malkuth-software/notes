# DK D8: the Gallery docking page, its accessibility check and reference image

GUI.Forms commits: page eaacec9f (claude/w-dock-gallery), toolbar dragging
87831263 (claude/w-toolstrip-drag), look and digests 9dd9561a
(claude/w-dock-look). Host: the M4 (macOS, arm64), 2026-11-01.

## Accessibility (stage A, macOS)

`gui_forms_macos_accessibility_tests`, `..._client_tests` and
`..._client_gallery_tests` pass, run on the M4 with an idle, unlocked
screen (`M4WS_WAIT_IDLE=30`). The Gallery client presses every page's
navigation row through AXUIElement and compares each page's AX tree with
the page's semantic snapshot:

    note gallery pages: 31
    control dock_site instances=2 nodes=9 matched=9
    control dock_navigator instances=1 nodes=8 matched=8
    types=91 instances=1067 nodes=3183 matched=3183 mismatches=0 failures=0

The docking page's stacks are published as panes named by their selected
panel (`gallery.docking.site.stack.N.panel.<id>`), with their caption
buttons as children; each matched. Not run here: Windows (UIA, MSAA) and
Linux (AT-SPI), which wait for those hosts, and the caption buttons'
actions through AX (the Gallery client compares trees and does not press
them).

## Reference image

`docking-workspace.png`: `GUI.Forms Gallery --page docking-workspace
--capture <png>` at the default 1280 x 820, the window's presented
framebuffer (CPU Skia). Solution Explorer and Class View tabbed on the
left, the Toolbox pinned out to the left edge, Properties on the right,
Output and Error List tabbed below, three documents; the page's controls
on the right.

Findings for the look (T4) and motion (T2), not changed here:

- A tool stack narrower than its selected tab shows that tab scrolled and
  cut at both ends ("ution Explo") in the bottom strip, and the caption
  title cut without an ellipsis ("Solution").
- The stamp's shrink and settle, the drop tiles and the toolbar drag's
  insertion line are motion and cannot be captured by `--capture`; their
  frame counts are covered by `dock_drag_tests` (14 settle frames) and the
  strip drag's settle by `tool_strip_container_tests`.
