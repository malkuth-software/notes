# Style mapping: DataGridView cell styles and control colours onto the theme

Date: 2026-11-01. Status: accepted (owner, 2026-11-01: T4-1 option 1) for
stage W's DataGridView batch 2 (styles and formatting). Everything but the
colours is usable now; the colours are usable once `PartPaint`'s authored
colours land (T4 branch, reported through the coordinator).

## Principle

The theme owns appearance (design language §1.2, §12.7). A Windows Forms
style property becomes an **application value** only where the application
sets it. Unset properties draw from the theme, so a grid nobody styles looks
like the rest of the theme, in light, dark and high contrast alike.

- The Windows Forms defaults that name system colours (`DefaultCellStyle`'s
  `Window`, `ControlText`, `Highlight` and `HighlightText`, the headers'
  `Control` and `WindowText`, `BackgroundColor`'s `AppWorkspace`) mean "the
  theme", not those colours. In GUI.Forms the default styles are empty. A C#
  facade (stage S) that receives a system colour passes "not set".
- Every property resolves on its own: the first style in the inheritance
  chain that sets it wins, as in Windows Forms (`InheritedStyle`). For a data
  cell: the cell's `Style`, the row's `DefaultCellStyle`, then
  `AlternatingRowsDefaultCellStyle` on odd rows, `RowsDefaultCellStyle`, the
  column's `DefaultCellStyle`, the grid's `DefaultCellStyle`. Header cells
  resolve through their own defaults (`ColumnHeadersDefaultCellStyle`,
  `RowHeadersDefaultCellStyle`) as Windows Forms does. This is behaviour, not
  look; W implements it.
- A style is shared by reference in Windows Forms. In GUI.Forms a style is a
  value; a grid that needs the Windows Forms sharing semantics (changing one
  style object restyles every cell holding it) keeps styles in a table and
  cells refer to an entry. That is W's design choice and does not affect the
  mapping.

## Properties that need no decision

| Windows Forms | GUI.Forms |
|---|---|
| `Font` | `PartPaint::font` of the cell's `cell_content` and the header labels; measurement (batch 3's auto sizing) uses the same font. Applies in every presentation, high contrast included. |
| `Alignment` | Horizontal: `PartPaint::alignment` (start, center, end; mirrored with the layout direction). Vertical: geometry. The cell's one geometry function places the content part's box at the top, middle or bottom of the cell box less its padding. `NotSet` resolves through the chain; the last default is middle-start. |
| `Padding` | Geometry: the content box is the cell box less the padding, in the same geometry function. |
| `WrapMode` | `False`: one line, as today. `True`: the control lays the text out with the text layout engine and paints each line as a `cell_content` part from the cell's geometry function; batch 3's row auto sizing reads the same layout. |
| `Format`, `FormatProvider`, `NullValue`, `DataSourceNullValue`, `Tag` | Behaviour (formatting, parsing, `CellFormatting`, `CellParsing`); no look. |
| `CellPainting`, `RowPrePaint`, `RowPostPaint` | Content-only owner drawing (`theme/owner_draw/owner_draw.hpp`): the handler may draw in the cell's content box and suppress the content parts (`PaintParts` content foreground and content background); background, border, selection, focus and the error mark stay the theme's. |
| `BorderStyle` | `None` publishes no `edge` part; `FixedSingle` and `Fixed3D` publish it. `Fixed3D` is the theme's recess, not a bevel: in the default theme the grid's `well` is already recessed. |
| `CellBorderStyle`, `ColumnHeadersBorderStyle`, `RowHeadersBorderStyle`, `AdvancedCellBorderStyle` | Which `divider` parts exist is geometry: horizontal, vertical, both or none. `Raised` and `Sunken` (and their horizontal and vertical forms) publish the same dividers as `Single`; their look is the theme's divider. Raised and sunken cells are "too windowsy" for the house themes (stage brief, lessons); the Classic theme draws its dividers as bevel pairs. A recorded difference from Windows Forms. |
| `EnableHeadersVisualStyles` | `true` (the default): the header styles' colours are not applied and the theme draws the headers, as Windows Forms ignores them under visual styles. Their font, alignment, padding and wrap mode still apply. `false`: the header colours apply as below. W checks the Windows Forms reference for which header colours visual styles ignore and follows it. |

## The colours (decision T4-1: option 1, accepted)

| Windows Forms | Part (role) | Colour read |
|---|---|---|
| `BackColor` | a new item part `cell_fill` (`fill`, painted only when the cell's inherited style sets a background) | background |
| `ForeColor` | `cell_content` (`content`) and a header's label | foreground, when the cell is not selected |
| `SelectionBackColor` | `cell_selection` (`selection`) | background |
| `SelectionForeColor` | `cell_content` | foreground, when the cell is selected |
| `GridColor` | the grid's `divider` parts | foreground |
| `BackgroundColor` | the grid's `well` | background |

The control chooses which colour it passes (the selected or the ordinary
one), as it already chooses the part's states. How the colour reaches the
part is the decision:

1. **Authored colours on the part (recommended).** `PartPaint` gains an
   optional authored background and foreground, applied by the engine as an
   inline style on that one part would be (R2-10, rule E1): they win over the
   theme's ordinary declarations, lose to its `!important` ones, and are
   dropped under forced high contrast, where the role's colours stand
   (design language §5.8: the host palette wins). Surface roles (`face`,
   `fill`, `selection`, `well`) read the background; text, glyph and divider
   roles read the foreground. A value atom beside `content_color` (W-7), so a
   vocabulary change and the owner's.
2. **Inline styles per cell style.** The grid compiles each distinct style
   into theme-language rules. Selectors cannot name one item instance and
   the vocabulary admits only finite custom states, so the rules cannot be
   matched to the cells that hold the style. Not recommended.
3. **Colour roles only.** A style names a `ColourRole` rather than a colour.
   The most themeable, but a C# program that sets RGB colours (stage S)
   cannot be mapped. Not recommended.
4. **`content_color` as it is.** Background only, through the `fill` part;
   `ForeColor` and `SelectionForeColor` unsupported. `content_color` is data
   and is drawn as given in high contrast too, so an application's
   background would override the host's high-contrast palette, against §5.8.
   Not recommended.

Control-level colours (Windows Forms `Control.BackColor` and `ForeColor`, the
C ABI's `set_control_colors`) need no new atom: they lower to the control's
inline style (`Control::set_inline_style`), which already has the semantics
of option 1. T4 phase 4 moves today's per-control `BasicControlStyle`
colours onto it.

## What W can start now

Everything in the first table, the inheritance chains, `CellFormatting`,
`CellParsing` and the owner-drawing events. The colour properties can be
stored and resolved now; painting them waits on T4-1. The provisional
pencil, asterisk and error marks keep their drawings until T4 phase 4 draws
them for the shipped themes.
