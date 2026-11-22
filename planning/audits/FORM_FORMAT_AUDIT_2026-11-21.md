# Audit: the form document format, XML or CSS (2026-11-21)

Asked by the owner on 2026-11-21: "our notes discussed that form records are
xml (like sdrsharp's layout) but it wound up being css because a worker
decided- hey, make them both css. but we didnt concur on this for reasons of
intelligent design. I want to audit that, too."

Read against `main` c0e1a31a8, the notes (`gui_forms` branch) and the desk.

## 1. What happened

| Date | Record | What it says |
|---|---|---|
| before 2026-10-13 | `demo/gallery.dml` | a provisional form description in a CSS-like block syntax (`form gallery.root { title: "…"; }`) |
| 2026-10-13 | FD stage brief, "Owner decisions, 2026-10-13", decision 1 | "**Syntax: the CSS family, shared with themes.** One parser and one round-trip machinery serve form and theme documents. The provisional Gallery DML's block style already looks like this and evolves into it." |
| 2026-11-03 | FD1 to FD6.1 (4bde249d8) | form documents, the form compiler and application types built on the theme parser, CSS-family |
| 2026-11-15 | desk `DECISIONS_2026-11-13.md` | "Form syntax (owner, 2026-11-15): forms were always meant to be XML; forms and composite controls move to XML before 1.0 (themes stay CSS-family). The existing .form files are converted." |
| 2026-11-15 | 33e71576f, landed in #483 (merged 2026-11-17) | "Forms move to XML": a located read over the bounded XML reader, the model, generator, loader, window loader, form compiler and editor rebuilt on it; the CSS form document removed from the theme parser; all 56 documents converted |

The 2026-10-13 record attributes the CSS choice to the owner. The owner's
own words for that answer are not in the Shadow harvest, so the record
cannot be checked against them; the reasoning written there (one parser for
forms and themes, the DML "already looks like this") is a worker's argument
from convenience.

## 2. Where things stand on `main`

- **Forms are XML.** All 48 `.form` files open with the XML declaration and
  `<form xmlns="urn:gui-forms:form:1">`; each object is an element named by
  its kebab type with its stable ID in `id`, properties are attributes.
- **Composite controls (`.control`) are XML** on the same reader.
- **Dock layouts, locales and help records are XML** (the bounded reader, DK 43).
- **Themes are CSS-family**, by decision.

## 3. What is still CSS inside the XML forms

1. **Layout declarations as CSS strings.** The layout panels take a CSS
   declaration block in an attribute: `style="grid-template-columns: 1fr 2fr; column-gap: 6px"`
   on the container and `item-style` on each child (`docs/user/forms.md`;
   `include/gui_forms/layout/style/layout_style.hpp`: "names and keywords
   are CSS's"). The panels are named `flex-panel`, `grid-panel`,
   `block-panel`. This is the largest piece of CSS left in a form.
2. **Attribute values are read through the theme tokenizer** (lengths,
   colours, keywords, whitespace-separated lists). For simple values this is
   only a token grammar; it shows mostly in the layout strings above.
3. **Bindings are written function-style**, `bind(...)`.

## 4. Stale records

- FD stage brief, decision 1, still says "the CSS family, shared with themes".
- `docs/controls/generated/schema.md` (from `tools/control_schema.py`): "form
  documents, which are CSS-family".
- `tools/convert_form_to_xml.py` and `tools/verify_form_conversion.py`, the
  one-time conversion tools, are still in the tree.

These are fixed whatever the decision below (record amendments and a small
code change for a coder).

## 5. The decision

How layout is written in a form (item 3.1):

- **a) XML-native layout attributes (recommended).** Each layout setting is
  its own attribute on the panel or child (`<grid-panel columns="1fr 2fr" column-gap="6">`,
  `<button column="2" row="1">`), with no CSS declaration strings in forms.
  Forms become XML through and through; themes stay the only CSS. The
  layout engine keeps its CSS semantics underneath, and the layout oracle's
  Chromium fixtures keep their CSS (they are test material, not forms).
- **b) Keep the CSS declaration strings** (status quo): one spelling shared
  with the layout fixtures and the CSS specification, at the cost of a second
  language inside an XML attribute.
- **c) Move layout to the theme sheet:** forms carry structure only and
  layout lives with presentation in CSS. This splits a form's meaning across
  two files.

## 6. Owner, 2026-11-21

"its the structure that needs to be xml, not the description." Layout:
keep the CSS declaration strings (option b), the owner leaning that way for
designers' fluidity. The principle: **structure is XML, description is
CSS.** Layout declarations are description and stay CSS; which objects
exist, their identities, nesting and relationships are structure.

Bindings were called "a bit sketchy": `bind(...)` shares the property's
attribute with literal values, so a literal text that begins with `bind(`
needs an escape (`\bind(`). Under the principle a binding is structure (a
relationship between a property and the model), so it gets an XML form of its own.

**Bindings, owner 2026-11-21: (a) binding elements.**

```xml
<check-box id="settings.auto" text="Save automatically">
  <bind property="checked" to="autosave"/>
</check-box>
<progress-bar id="job.progress">
  <bind property="value" to="progress" mode="one-way"/>
</progress-bar>
```

Literal attributes are always literal; `bind(...)` and its `\\bind(` escape go.
Given to a coder with the stale-record fixes of section 4 (track
`tracks/gui_forms/form_bindings.md` in `projects`).
