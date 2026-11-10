# Help system study

Status: study accepted with one correction, nothing built. First written
2026-11-09 for the owner's request: "help system needs to be designed. i
want to more or less emulate windows help, the old kind, chm, but i want to
retain the nice paper style of the help from plan paint."

Revision 1 (2026-11-09) follows the owner's rulings and their correction:
"help is itself merely records part of the translatable text that every
program is supposed to be using already." The first draft treated help as a
separate content set, with its own package, compiler and topic keys. This
revision replaces that: help lives in the program's text records, which are
keyed by stable ID, and it is compiled with the language files. The first
draft's question 5 (the authoring source) is withdrawn.

This is a clean-room study. It describes compiled HTML Help (CHM) only from
public documentation and from how the viewer behaves on screen. No Microsoft
code, file layouts, icons, strings or other assets are copied, and we do not
read or write `.chm` files.

## 0. Owner rulings (2026-11-09)

| # | Question | Ruling |
|---|---|---|
| 1 | The `.helpbook` name | Accepted. Section 5 says what it names now. |
| 2 | Navigation pane | CHM-like tabs. |
| 3 | Presentation | A window plus a docked book, with the window as the default. |
| 4 | Plan Paint's collapsible topics | Kept, as a markdown construct. |
| 5 | Authoring source | Withdrawn. Help is translatable text records. |
| 6 | External links | Passed to the application as an event. |
| 7 | Release placement | The format, the compiler and the C ABI come before 1.0. |
| A | The contents tree | (a) Generated from the program's structure: windows, then panels and groups, then controls. Stand-alone topics are slotted in where they are declared. |
| B | The full help body | (a) One markdown string in our dialect. Topic links are written as stable IDs. |

## 1. What GUI.Forms already has

- **The language-file design** (`TRANSLATION_DESIGN_2026-10-28.md`, settled
  decisions after revision B):
  - one `locales-<tag>.xml` per locale, in CSL locale XML;
  - `<controls>` nested by stable-ID segment, with properties as attributes
    (`text`, `tool-tip`, `accessible-name`, `accessible-description`,
    `placeholder`, `header`, `title`);
  - the built-in language compiled into the program, and a skeleton/template
    export;
  - user files that win over the program's, under the override rules;
  - display text only, with whole-file and per-entry rejection;
  - the untranslatable rescue back to the built-in language.

  Plan Paint's `languages/*.json` packs are the shipping precedent for this:
  a flat file per language, built-in English compiled in, a validating
  loader. They already carry Plan Paint's full in-app help as catalog
  entries, which is the owner's point in practice.
- **StableId:** dot-separated segments, each `[a-z0-9]+(-[a-z0-9]+)*`.
- **HelpProvider** (`components/help_provider/`):
  - It holds authored per-control help metadata: string, keyword,
    navigator, show-help and namespace.
  - It routes F1, mouse and window help requests that no handler took, as
    `HelpRequestEvent`.
  - It is policy-free and opens nothing by itself.
- **RibbonToolTip:** its superset is title, description, shortcut, image,
  disabled reason and `help_topic`. F1 while it shows raises an event that
  the application handles.
- **The C ABI:** `windows.request_help(window, &handled)`.
- **The markdown engine:**
  - It reads the stx_rtf2md dialect: CommonMark 0.31.2 plus
    strikethrough, attribute spans and block attribute lines.
  - Spans keep their source bytes.
  - Parsing never opens a link or an image.
- **Plan Paint's HelpBook** (`src/forms/help.cpp`): the paper look to keep.
- **Plain tooltips:** translatable text on the thing, never on data items,
  one presenter per window.
- **Look-ledger N4:** reserves the ribbon "?" part and the window help card
  for this design.

## 2. The CHM model, and what we keep

| CHM feature | Verdict |
|---|---|
| Three panes: toolbar, navigation, topic, with a splitter | **Keep.** |
| Contents tab (books and pages) | **Keep**, generated from the program's structure (ruling A). Books are paper topic bars with drawn triangles. |
| Index tab: type-ahead keyword list, subkeywords, a chooser for several topics | **Keep.** Keywords come from the records' `keywords` field. The chooser is an inline list. |
| Search tab: ranked full text, titles only, boolean | **Adapt.** v1 has ranked results, AND, quoted phrases and titles-only. NEAR, similar words and search within results are dropped. |
| Favorites tab | **Adapt** into a Bookmarks tab, stored in the application's settings and keyed by stable ID. |
| Toolbar: Hide/Show, Back, Forward, Home, Print, Options, Locate | **Keep.** Locate becomes "Show in contents". **Drop** Stop, Refresh, Internet Options and Jump, which belong to a browser. |
| Sync TOC | **Keep**, automatic by default. |
| Topic links | **Keep**, written as stable IDs (ruling B). Other schemes go to the application (ruling 6). |
| Popup topics | **Keep.** The same paper popup as What's This. |
| Context help (F1) | **Keep.** The focused control's own record, walking up the parents (section 6). |
| What's This? | **Keep** as a mode (Shift+F1, caption "?" where the host offers it). |
| Compiled single file | **Adapt.** It is the help section of each compiled language pack (section 5). |
| Merged files, information types, custom windows, ActiveX, script, HTML | **Drop.** |
| Browser rendering | **Replace** with GUI.Forms text and layout over our markdown. |

## 3. The look: Plan Paint's paper, as theme parts

The values come from `plan-paint/src/forms/help.cpp`. They become the default
values of a `help-book` theme family, so light, dark and our high-contrast
theme can each restate them.

| Part | Plan Paint value | Theme part |
|---|---|---|
| Paper ground | #FFFFD3 cream | `help-book::part(paper)` background |
| Paper edge | 1 px #A09D7D | `help-book::part(paper)` border |
| Margins | 12 px left and top, 28 px right including the scroll bar | `help-book::part(paper)` padding |
| Title | control face 18, black | `help-book::part(title)` |
| Title rule | 1 px #BEB992 | `help-book::part(rule)` |
| Body | control face 18, black, wrapped, 7 px between blocks | `help-book::part(body)` |
| Topic bar | #EEEAA8, no border, square corners, at least 24 px | `help-book::part(topic-bar)` |
| Topic bar, hot | #E6E196 | `help-book::part(topic-bar):hover` |
| Expander | drawn triangle, right or down | `help-book::part(expander)` |
| Close | "×", face 24, upper left | `help-book::part(close)` |

New parts follow the same paper rules:
- **Headings** 1 to 3, with level 1 ruled like the title.
- **Callouts** `note`, `tip` and `caution`: a darker paper with an edge in
  the rule colour and a one-word label. They are written as a block
  attribute line, `{callout=tip}`; the viewer adds the `callout` key to
  what the dialect already reads.
- **Collapsible sections** (ruling 4): a heading followed by the block
  attribute `{collapsible}` folds its section under a topic bar, as Plan
  Paint's topics do. `{collapsible open}` starts it open.
- **Links** in a dark ink, underlined, with hover and visited states.
- **Code** in the monospace face.
- **Navigation pane** on paper one step darker.
- **Toolbar** with flat buttons on the bar colour.
- **Popup** on the paper ground with a small shadow.
- **Search hit** as a marker highlight.

In the dark theme the paper is a warm dark ground with cream ink. High
contrast maps paper to window, ink to text and bars to button face. The tone
is part of the look: second person, short paragraphs, a welcome first. The
authoring guide says so.

## 4. Help is text: the record

Every program already keeps one text store per language:
- the built-in language is compiled in;
- each translation is a language file;
- records are keyed by stable ID.

Help adds three fields to that record, beside `text` and `tool-tip`:

| Field | Meaning | In `locales-<tag>.xml` |
|---|---|---|
| `help-line` | One sentence: the What's This popup, the window help card, the line under a contents entry | an attribute on `<control>` |
| `help` | The full help body: one markdown string (ruling B) | a `<help>` child element whose content is the markdown, so line breaks survive. Control text stays in attributes, so this is not mixed content. |
| `keywords` | Index keywords. `;` separates keywords, and `,` separates a keyword from its subkeyword. | an attribute on `<help>` |

```xml
<control name="main">
  <control name="ribbon">
    <control name="paste" text="Coller" tool-tip="Colle le presse-papiers"
             help-line="Colle le contenu du presse-papiers dans le dessin.">
      <help keywords="presse-papiers; coller, image">
Collez une image copiée ailleurs. Voir [Sélection](main.canvas.selection).
      </help>
    </control>
  </control>
</control>
```

**Stand-alone topics** ("Make your first picture") are ordinary records
with their own stable IDs and the same fields. No control holds those IDs.
A program declares them like messages:
`inline const gf::HelpTopic first_picture{gf::StableId("help.first-picture"),
"Make your first picture", body, gf::HelpPlace::after("main")};`
Their title is the record's `text`. The placement names the record they
follow or nest under in the generated contents.

**Links** (ruling B):
- A link destination that is a valid stable ID is a topic link, for
  example `[Paste](main.ribbon.paste)` or
  `[Shortcuts](main.ribbon.paste#shortcuts)`.
- A link title of `popup` opens the topic as a popup:
  `[Paste](main.ribbon.paste "popup")`.
- Any other destination is offered to the application through an event
  (ruling 6).
- Images are written `![alt](name.png)`. They are PNG only, from the
  program's resources, and are never fetched.

**The contents tree** (ruling A) is generated from the program's
structure:
- windows, then panels and groups, then controls, in stable-ID order as the
  skeleton enumerates them;
- only records with a `help` body get pages;
- a control with only a `help-line` appears as a line on its parent's page;
- stand-alone topics are slotted in at their declared places.

The skeleton export already walks every window, form and hidden page to
enumerate stable-ID paths, so the generator reuses that walk and invents no
second structure.

**Translation and overrides:**
- The template export includes `help-line`, `help` and `keywords`
  automatically, with the built-in text as values.
- Users add or override help by stable ID under the same rules as tooltips:
  a user file wins per record.
- Data items never carry help, just as they never carry tooltips.
- The untranslatable way back covers help. The rescue switches help to the
  built-in language with everything else. The F1 viewer carries the
  untranslatable link at the top that the translation design already
  reserves.
- A record missing in the chosen language falls back per field, exactly as
  text does, and the page marks the fallback.

**HelpProvider** becomes a run-time view of these fields:
- `set_help_string` writes the control's `help-line` and
  `set_help_keyword` writes its `keywords`;
- both are program values, which a language file can translate;
- `help_navigator` and `help_namespace` stay as routing hints.
The WinForms-shaped surface is unchanged, and there is a single source of
help text.

**RibbonToolTip's** title and description are already the command's text
and tooltip records. Its `help_topic` becomes a stable ID, defaulting to the
command's own ID.

**The window help card (N4)** is the window record's `help-line` in the paper
popup, with a "More..." link to its `help` page. The ribbon's reserved "?"
part opens the window's page.

**The greaseboard** comes later. A record may later gain a `tour` field; it
is reserved and unused in v1.

## 5. Compilation, and what `.helpbook` names

The help compiler is a pass of the language-file compiler (the
`gui-forms-locale` tool, which the translation design already defines as the
checker on the shared XML reader). For each language, the pass does five
things:
1. It parses every `help` body with the markdown engine and rejects the
   entry on a fault, as text entries are rejected.
2. It checks that every topic link names a record that exists, either a
   control path from the skeleton or a declared topic, and that every image
   names a PNG resource.
3. It builds the keyword index: sorted, folded with the locale's case
   folding, keyword to stable IDs.
4. It builds the search index (section 7).
5. It records the generated contents tree, which is the same for every
   language except for titles.

**What `.helpbook` names:** no file carries the extension.
- The **helpbook section** is the help portion of a compiled language pack:
  the contents tree, the keyword index, the search index and the link table,
  with a format version.
- The viewer control keeps Plan Paint's name: **HelpBook** (the paper
  topic pane), **HelpBookViewer** (three panes) and **HelpBookWindow**.

The built-in language's helpbook section is built at program build time and
compiled in with the built-in text. A shipped translation's section is built
by the same pass. A user's `locales-<tag>.xml`, which is editable XML, gets
its helpbook section built in memory at load by the same code, limited to
the records it overrides and merged over the shipped section. So the
index is never stale for user overrides.

**Versioning:**
- The section has its own format version. A reader refuses a later major
  version with an explicit result, and the viewer then rebuilds the section
  from the records.
- Unknown sub-sections are skipped.
- The help content has no version of its own: it is the program's text at
  that build.

**Shipping and opening.** Nothing extra ships. Help travels with the
program's text: built in for the built-in language, and in the language
packs for the others. "Opening help" means opening the viewer on the current
language's store.

## 6. APIs

**C++**
- `HelpBookSystem`, one per application, owned by the application object:
  - `show(owner_window, stable_id)` opens the viewer at a record;
  - `show_contents`, `show_index(keyword)` and `show_search(text)` cover
    `HelpNavigator`'s cases;
  - `show_popup(owner, stable_id, anchor)` opens the paper popup;
  - `resolve(control)` returns the stable ID F1 would open.
- F1 resolution reads the focused control's own record, then walks up the
  parents' records to the window, then the window's page, then the
  program's home topic. The first record found with a `help` body opens.
  When the first record found has only a `help-line`, the viewer opens its
  nearest ancestor's page scrolled to that line.
- It is the default handler of `HelpProvider` and `RibbonToolTip` help
  events. The application's handler runs first, and the help system only
  handles what is still unhandled.
- **What's This mode:** `Window::enter_help_mode()`.
  - The help cursor shows and the next click is captured.
  - The control's `help-line` appears in the paper popup, or the first
    paragraph of its `help` body when it has no line.
  - Escape or a click elsewhere cancels.
  - Whether the caption shows "?" is a capability the host reports.
- **Controls:** `HelpBook` (the topic pane alone, Plan Paint's docked case),
  `HelpBookViewer` (three panes, placeable in any window or dock panel) and
  `HelpBookWindow` (the default presentation, reused by `show`).

**C ABI** (a new `help` table, struct_size gated; part of ABI 1.2 with 3.0's
additions):
- `help.show(window, stable_id_utf8)`
- `help.show_navigator(window, navigator, text_utf8)`
- `help.show_popup(window, stable_id_utf8, x, y)`
- `help.resolve(control, buffer, capacity, &length)`
- `help.enter_help_mode(window)`
- `help.declare_topic(application, stable_id_utf8, title_utf8, body_utf8,
  place_utf8)`, for C programs, which cannot use the C++ declaration
- the three fields as ordinary control properties (`help-line`, `help`,
  `keywords`) in the schema, so they come through the existing property
  table with no new setters

Strings are borrowed for the call and results are explicit codes. The
HelpBook controls are in the control schema, so C# and the designer place
them like any control.

## 7. Search

v1 is simple and deterministic:
- Tokenise with the text engine's word breaks and fold case.
- Keep a sorted term table with postings of (record, count, in-title flag).
- Binary-search the terms and intersect postings for AND.
- Check phrases against the parsed body.
- Rank by title hit, then frequency, then contents order.
- Match prefixes as the user types. There is no stemming.
- Work in a bounded budget per keystroke on the UI thread. The target is
  2,000 records with help bodies, and it gets measured before anyone calls
  it fast.

Later, the term table is a customer of the post-0.5 fast-search backlog:
- Eytzinger and S-tree layouts and branchless B-trees, studied from
  *Algorithms for Modern Hardware*;
- the shared search routine for the 10M-row chat demo.

The term table is its own sub-section, so a new layout is a new sub-section
version.

## 8. Accessibility, keyboard, printing

- **Accessibility:**
  - The viewer publishes a normal tree through UIA, AX and AT-SPI: tab
    list, contents tree, index and result lists, and a document with
    headings, links and images (with their alternative text).
  - The popup's text is announced, and focus returns to the control.
  - Entering and leaving help mode is announced.
  - High contrast comes from our palette, and text size follows Options and
    the system scale.
- **Keyboard:**
  - Alt+C, Alt+N, Alt+S and Alt+I go to Contents, Index, Search and
    Bookmarks; F6 cycles the panes.
  - Alt+Left and Alt+Right (and the mouse buttons) move in history, and
    Alt+Home goes home.
  - Ctrl+P prints and Ctrl+F finds in the page.
  - Tab and Enter move through and follow links. Escape closes a popup or
    leaves help mode.
  - Platform equivalents come from the existing accelerator mapping.
- **Printing:**
  - The current page, or a contents book, goes through the existing print
    path.
  - Paper style without the ground fill.
  - Page headers carry the program and page title.
  - Links print their target's title in brackets.

## 9. Open questions for the owner (new, from revision 1)

1. **Where built-in help bodies are written.**
   (a) In code, like other built-in text (`set_help(...)`,
   `gf::HelpTopic`); (b) in a built-in locale file that the build embeds;
   (c) either, with both compiled into the same built-in store.
   *Recommendation: (c),* with (b) suggested for long bodies, since
   multi-paragraph markdown reads badly in C++ literals. Short `help-line`s
   stay in code.
2. **Controls without a help body in the generated contents.**
   (a) Omitted, with their `help-line` shown on the parent's page; (b) every
   control listed.
   *Recommendation: (a).* The contents then shows what someone wrote, not
   every button.
3. **Missing help for a focused control.**
   (a) Walk up the parents to the window page, then home (section 6);
   (b) show "No help for this item" with a link to contents.
   *Recommendation: (a),* which matches how CHM programs usually feel.

## 10. Staged plan for the 1.0 roadmap

Kinds and effort units (U) as in `ROADMAP_TO_1.0.md`.

| Row | Stage | Depends on | Kind | U | Gate |
|---|---|---|---|---|---|
| H1 | Help fields in the text record: `help-line`, `help`, `keywords` in the locale XML schema and limits, the C++ properties (registry localizable flag), template export, user overrides, the way back; HelpProvider as a view onto them | the language-file implementation | C, then B | 1.5 | A template export of the gallery lists the help fields; user override and rescue tests |
| H2 | Stand-alone topic records (`gf::HelpTopic`, placement) and the contents generator from the skeleton walk | H1 | C, then B | 1 | The generated tree for the gallery matches a golden |
| H3 | The helpbook section in the language-file compiler: markdown, link and image validation, keyword index, search term table, contents; the same pass at load for user files | H1, H2 | C, then B | 1.5 | A broken link fails the compile; a hostile-section fuzz target |
| H4 | `HelpBook`: the paper topic pane on theme parts, with collapsible sections, callouts, stable-ID links and PNG images | H3, 2.10 | C, then B, O | 1.5 | Plan Paint's help shows as today's book (screenshot compare) |
| H5 | `HelpBookViewer` and `HelpBookWindow`: tabs, history, bookmarks, sync, toolbar, Options; accessibility on all three hosts | H4 | C, then B, W, M | 2-3 | Keyboard walkthrough and accessibility tree tests |
| H6 | Search v1 | H3, H5 | C, then B | 0.5-1 | Measured on 2,000 records, workload recorded |
| H7 | Help routing: F1 through records, the default handler for HelpProvider and RibbonToolTip, the ribbon "?" and window card (2.11, N4), What's This, the paper popup | H5 | C, then B | 1 | F1, Shift+F1 and "?" gallery interaction tests |
| H8 | Printing of a page and a book | H5, print path | C, then B | 0.5 | Print-preview golden |
| H9 | C ABI `help` table, the help fields in the schema, the C# binding | H7, 3.0 | C+B+O | 0.5 | ABI conformance cases; joins ABI 1.2 |
| H10 | Plan Paint: help records replace `help_content.hpp`, its JSON help entries convert with the planned converter, and its HelpBook comes from the library | H7, the Plan Paint locale migration | C, then B, O | 1 | Plan Paint's help unchanged to the eye in two languages; F1 opens context pages |
| H11 | After 1.0: fast-search layouts, greaseboard tours | the search backlog | S, then C | - | Separate studies |

H1 to H10 come to about 11-13 U, roughly three quarters of it coder work.

## 11. Owner answers to section 9 (2026-11-09, "Q14")

Recorded from the coordinator's log; the study is accepted with these.

1. Built-in help bodies live in an embedded built-in locale file; short
   lines may stay in code.
2. Controls without a help body are left out of the generated contents;
   their help line is shown on the parent's page (a).
3. F1 with no help walks up the parents to the window page, then home (a).

Implementation follows `planning/ROADMAP_TO_1.0.md` rows 2.13-2.20 (H1-H8,
coded on `claude/help-system`, in cp27), 3.7 and 4.12. Open points are in
`planning/GAPS.md` (H1-H5).
