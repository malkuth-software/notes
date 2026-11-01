# Stage W: resume plan (2026-10-29)

Stage W paused here at the owner's direction once the RichTextBox
follow-ups were finished. This page says where the work stands and how to
pick it up again. The coverage table in [README.md](README.md) is the
long-term record; the rows that are stale are listed below.

## Where the code stands

The markdown stack is finished and verified, and it folds as one topic from
its top branch. It is local to the w2 workspace
(`~/Developer/gui_forms_workers/w2/gui_forms`), sits on `origin/main`
8cde98df (checkpoint 3), and goes to checkpoint 5.

| Step | Branch | Head | What it holds |
|---|---|---|---|
| (i) | `claude/w-markdown-framework` | c0e1b79c | MD4C vendored; `gui_forms/text/markdown` (parse with source ranges, plain text, RTF conversion) |
| (ii) | `claude/w-text-editing-engine` | ae42ffd5 | `detail::TextEditingEngine` under `TextBox` |
| (iii) | `claude/w-rich-text-markdown` | bcfc7ed4 | `RichTextBox` over markdown with the control bar; `RichTextModel` removed; follow-ups below |

The follow-ups on (iii), after its first commit 4530dada:
- 1883393d: a test for a multi-paragraph document. It is typed with Enter, the paragraph wraps, Up walks the rows, pasted paragraphs keep their line feeds, the caret scrolls into view, and the paste is one undo step.
- b7c30b80: the porting guide drops its stale "Not yet available" list.
- ecd5d446: RichTextBox draws each glyph of a selected row once.
- 6b71b2ef: TextBox does the same, for one line or many. `tests/support/clip_tracking_painter.hpp` checks that no glyph is drawn twice.
- bcfc7ed4: the control bar shows on, off and mixed. `RichTextFormatState` is new. A mixed button paints its checked face at half strength and has the `indeterminate` item state and the mixed semantic state; the Heading button reads "Mixed".

List markers are dimmed like every other marker. Measured in the screenshot, the darkest pixel of "- " is (130, 139, 149) and that of "##" is (118, 128, 138); body text is about (30, 42, 53).

Checkpoint 5 (#449) holds the markdown stack, and its three branches are deleted. The tooltip build is in the next section.

## The tooltip build (2026-10-31)

Steps 3 to 10 below are done on `claude/w-tooltips` in w2, built on `origin/main` 2fe87fd9, and it goes into the next checkpoint:
- 1586c8c3, steps 3 to 7: `Control::tool_tip_text` (bindable, UTF-8 checked, published as the semantic field `tool_tip`), the presenter in `src/core/window/tool_tips/`, the window and program switches, the window's `ToolTipSettings`, ToolTip forwarding, ErrorProvider setting its glyph's text, strip-item and tab-header tooltips, RibbonToolTip blocking and title fallback, and the field hints.
- 2fc0ef95, steps 8 to 10: `detail::CutOffTextRevealer` in ListView (details and list views) and TreeView, the porting-guide rows, the docstrings and the regenerated schema.

Verified on the M4: 34 touched tests pass plain and under `--sanitize`; the state-machine gate (`--changed-from origin/main`) reports no F1 or F2; house style, lifetime rules, `docs --check` and the registry and schema checks are clean.

**Owner decisions (accepted 2026-10-31, as built):**
- Cut-off text appears at once when the pointer arrives, with no delay.
- A typed-text field shows its hint on any focus, by click or by Tab. A set placeholder wins, the editable ComboBox included.
- A tooltip is inherited through ancestors, so a composite's parts show its text.
- Keyboard focus shows the bubble only while the focus cue is visible.
- The pointer must rest for the whole delay; moving restarts it.
- After a press, nothing shows until the pointer leaves the thing.
- The settings are window-wide and shared by every `ToolTip` of the window. ErrorProvider's private 250 ms delay is gone.
- `ToolTip::remove_tool_tip` takes a non-const `Control&`.
- `Window::show_tool_tip` ignores the switches.
- A RibbonToolTip mapping on a whole control blocks the plain tooltip on all its parts.
- `SemanticNode::tool_tip` is a new semantic field, so the schema export changes.

The mockup for the first two is `~/Developer/gui_forms_workers/mockups/cut-off-reveal-and-field-hints.html`.

## Verified only on the M4 (macOS arm64)

Every build and test above ran on the M4 through `m4ws`. Windows and Linux were not built.
- On (iii)'s head: 51 tests ran plain and under `--sanitize` and all pass. The regex is in `w2/dev/rtb_regex2.txt`: the TextBox set, the gallery, `rich_text_box`, `markdown`, `markdown_fuzz` and `application_native`. The state-machine gate (`--changed-from origin/main`) reports no F1 or F2.
- (i) alone passes 10 tests, plain and sanitized. (ii) alone passes 50, plain and sanitized.
- The Gallery traces were recorded again for text-and-input, states-and-diagnostics (1300 controls) and typography-and-inspection (#1333). The TextBox fix moved no trace, because no trace paints a selection.
- The screenshots are `w2/dev/rtb_bold.png` and `w2/dev/rtb_mixed.png`. They came from a throwaway macOS program in `w2/dev/capture/`, which is not committed: a main file plus a CMake snippet placed after the seam lab target. It was built with a `--gallery` configure and run with `--capture` over ssh.

## The tooltip design, as settled by the owner (2026-10-28)

**Who has a tooltip**
- Tooltips belong to controls and to stable authored parts only, such as tab headers and toolbar or strip items. Data items never have one.
- `Control` gets plain tooltip text, a `std::string` that the translation system will wrap later. Empty text never shows.
- A `TabPage`'s tooltip is its control tooltip, shown over its tab header.
- Strip items keep theirs.

**What is not provided**
- No `tool_tip_text` on `ListViewItem` or `TreeViewItem`.
- None of these Windows Forms members: `ListViewItem.ToolTipText`, `ListView.ShowItemToolTips`, `TreeNode.ToolTipText`, `TreeView.ShowNodeToolTips`, or `TabControl.ShowToolTips`. A page's empty tooltip simply never shows.
- Add a row to `docs/user/for-windows-forms-developers.md` saying three things: GUI.Forms does not provide those members, data items have no tooltips, and the list reveals cut-off text. Do not mention the owner's conversion program.

**Cut-off item text**
- ListView rows in the details and list views, and TreeView nodes, reveal their full text in place on hover, expanded over the row at the item's own position.
- It happens only when the text is actually cut off, and there is nothing for the program to set.
- This is not a tooltip. It has no presenter content and does not obey the tooltip switches. It may share the hover timing.

**Presenter and switches**
- There is one presenter per window, with a window switch and a program switch.
- Find items through the existing part and semantic hit testing (realized items, part geometry, stable IDs, semantic virtual children). Do not add a new `Control` virtual.

**Keyboard focus**
- Focus shows the tooltip as a bubble below the focused thing.
- Typed-text controls are the exception: `TextBox`, the `ComboBox` edit field, `NumericUpDown`, `MaskedTextBox`, `RichTextBox` and the like. Their tooltip shows as light-grey hint text inside the empty field, like placeholder text. A placeholder that has been set wins.

**Existing components**
- The `ToolTip` component forwards to all of this. It stays the Windows Forms way to set text and timing.
- `RibbonToolTip` reads the plain text from the thing's tooltip.

The design sent earlier (2026-10-28) still guides the build where the owner did not change it: Window timing, the forwarding `ToolTip`, and `RibbonToolTip` reading the plain text. Its per-item fields and the `Show*ToolTips` switches are withdrawn.

## Resume steps

1. **Rebase.** After checkpoint 5 lands, rebase onto `origin/main`. Delete the three markdown-stack branches once the coordinator names them as contained.
2. **Branch.** Create `claude/w-tooltips` from `origin/main`. RichTextBox is then on main, so its hint text can be wired in.
3. **Control tooltip and switches.** Add `Control::tool_tip_text` and its setter: a bindable text property, UTF-8 checked. Add the window switch (`std::optional<bool>`, which follows the program when unset), the program switch and the timing.
4. **The presenter.** Build the per-window presenter from the overlay, timer and placement code in `src/controls/tool_tip/tool_tip.cpp`.
   - Window hover tracking and focus changes feed it.
   - Strip items and tab headers are found by part and semantic hit testing.
5. **ToolTip.** Make it forward: `set_tool_tip` writes the target's tooltip, and its timing goes to the window. Keep `visibility_changed`.
6. **RibbonToolTip.** When it has no title, it reads the plain tooltip.
7. **Hint text.** Show the in-field hint in the typed-text controls on keyboard focus, only when the field is empty and no placeholder is set.
8. **Cut-off text.** Build the reveal in ListView (details and list views) and TreeView: hover over cut-off text only, outside the tooltip switches.
9. **Porting guide.** Add the row described above.
10. **Records.** Update the gallery, docstrings, registry and schema. Verify on the M4: the touched tests plain and `--sanitize`, the gate, and the checkers.

## Other remaining W work

The coverage rows in [README.md](README.md) were brought up to date on 2026-10-31: `RichTextBox`, `Splitter`, the dialog components and `ListView` are done, the `TabControl` row records the header tooltip, and the totals are recounted. What stays open:
- **Partial by design:** the public bases `TextBoxBase`, `ListControl` and `UpDownBase`; `ToolStripControlHost` with arbitrary content; a public `ToolStripDropDown`; and the `DataGridView` editing controls as public types.
- **Docking:** P8's navigation preset, and D6 to D8.
- **TabControl:** `Multiline` wrapping is deferred by the owner.

Also deferred:
- RichTextBox parses synchronously. Moving the parse off the UI thread above 64 KiB was deferred.
- RichTextBox's caret is revealed on paint, as TextBox's is.
