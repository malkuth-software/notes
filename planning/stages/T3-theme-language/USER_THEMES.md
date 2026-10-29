# User themes: the `themes/` folder (design note, T3)

Status: design only, for owner review. No code until reviewed.
Owner directions (2026-10-28): user themes follow the user-translations
model; a user theme is a CSS file in `themes/` with its assets in a folder
of the same name; it is a cascade layer over the program's whole result,
overriding it only where allowed; it may override icons; there is no live
editing (interactive theme editing is kept for the owner's commercial
visual tool); and changes in the system's appearance reach a program only
where it opts in.

## 1. What a user theme is

A user theme is a file `themes/<name>.css`, with an optional folder
`themes/<name>/` beside it for its images. It is display only. It changes
how the program's parts are drawn and which icons it shows. It cannot
change what the program does, its layout, its text or its translations.

`<name>` follows P10's name rule, `[a-z0-9][a-z0-9-]{0,63}`: ASCII
lowercase letters, digits and dashes. A file whose stem breaks the rule is
skipped, and the log says so.

There are two `themes/` folders:

- **Beside the program**: read-only, and shipped with it.
- **Per user**: under the platform's configuration folder for the
  program:
  - macOS: `~/Library/Application Support/<program>/themes`;
  - Windows: `%APPDATA%\<program>\themes`;
  - Linux: `$XDG_CONFIG_HOME/<program>/themes`.

  The user may create the per-user folder.

Where both folders hold the same name, the per-user theme wins. That
mirrors user translations.

A user edits a theme file and restarts the program to apply it. The
program does not watch the folder.

## 2. Finding themes at launch

At launch, the application layer reads every `*.css` file in both folders.
The engine's core reads no file system (P10: "nothing is read from a file
system"), so this is the host's job. The scan is bounded:

- at most 64 files per folder;
- each theme's closure is at most 1 MiB of sheet text (P10's limit);
- its asset folder holds at most 256 files and 16 MiB.

Something over a limit is skipped, with a diagnostic.

A CSS file is a theme when it opens with a theme header:

```css
@theme {
  engine: ">=0.3";
  author: "...";
  provenance: "...";
  version: "1.0.0";
}
```

The header uses CSS descriptor syntax, like `@font-face`. The name is the
file stem; the header does not repeat it. A CSS file without the header is
not a theme. It is ignored without a diagnostic, so that a theme's partial
sheets can sit in the folder (see `@import` in section 3).

The candidate themes are parsed and checked at launch, so the theme list
can show only themes that compile. A refused theme appears in the list
greyed out, with its diagnostics.

## 3. Relation to P10's packages

A user theme is a **degenerate package with an implied manifest**:

| Manifest field | Implied value for a user theme |
|---|---|
| `format` | `gui-forms-theme/1` |
| `name` | the file stem |
| `version`, `engine`, `author`, `provenance` | the `@theme` header |
| `sheet` | the CSS file |
| `resources` | every file under `themes/<name>/`, by relative path (P10's path rules) |
| `published_values` | the program's |
| `coverage` | empty |
| `imports` | see below |

The implied package is compiled through P10's entry point. Everything P10
checks still applies, except hash pinning:

- **Integrity checks** (SHA-256 per file, E602; E604 import pins) have
  nothing to check. The implied manifest hashes the bytes it reads, so
  they match by construction. Hashing stays useful only as the theme's
  identity in the installed index (P10: the identity is the hash of the
  manifest bytes). For a user theme, the identity is the hash of the
  implied manifest, which covers the sheet and every asset.
- **Structural checks** all apply:
  - path rules (relative paths, no `..`, no `:`, at most 255 bytes and 8
    segments);
  - every image must decode as the image it claims to be (E601);
  - the closure size limit;
  - the engine bound (E607);
  - every diagnostic of the sheet itself.
- **`@import`** (recommended):
  - A user theme may import the program's own packages by name, pinned
    implicitly to the installed one, since they ship together.
  - It may also import a partial sheet in its own folder, as a path in
    `themes/<name>/`.
  - It may not import another user theme, so no chains run between user
    files.
- **The installed index**: user themes go in a separate `ThemeLibrary`
  from the program's packages, so a user name never shadows a program
  package that another `@import` names.
- **Image ownership** (P10c): each compiled theme owns its images. Not
  selecting a user theme, or replacing it, removes them.

What P10 does not give is new work:

- the folder scan;
- the `@theme` header (a parser at-rule, which is E206 today);
- building the implied manifest;
- the user cascade level (section 4);
- icon overrides (section 6);
- export (section 7);
- the settings entries.

## 4. The cascade layer

Owner decisions (2026-10-28).

### 4.1 The three cases

GUI.Forms' default theme sits at the bottom and is not authoritative: a
user theme replaces it. The default theme is the built-in recipe and the
engine's built-in rules, at CascadeLevel `built_in`. A program's own
sheet, if it has one, is the theme compiled from its package, at the
sheet levels (`theme_sheet`, `class_rule`, `stable_id_rule`,
`important_theme_sheet`). The levels already tell the two apart, so the
default theme needs no new marker.

1. **No program sheet, no inline styles** (the default case). The user
   theme applies in full over the default theme. `!important` never comes
   into play.
2. **A replaceable program sheet.** The developer declares the program's
   sheet replaceable (section 4.3). A selected user theme takes the
   sheet's place whole: the program's sheet is not compiled into the
   cascade, and the user sheet is compiled at the sheet levels, exactly as
   a program sheet is today. Only the program's inline styles, if any,
   keep their order over it (E1, with the user sheet as "the theme").
   This case needs no new level.
3. **An authoritative program sheet** (the default for a program with a
   sheet). The order, weakest first:
   1. user theme, ordinary: it applies only where the program declares
      nothing for that property on that part;
   2. program sheet, ordinary;
   3. program inline, ordinary;
   4. user theme, `!important`: this is how a user theme overrides the
      program;
   5. program sheet, `!important`: overrides inline, unless inline is also
      `!important`;
   6. program inline, `!important`.

   A user theme's `!important` never wins where the program declares
   `!important` for that property on that part, in its sheet or inline.

### 4.2 The levels

P9's CascadeLevel expresses this with two levels inserted. The rest of
the order is unchanged, as are the resolution walk's merge (a third rule
set) and the forced high-contrast filter (`>= important_inline_style`).
Weakest first:

| Level | Status |
|---|---|
| built_in | the default theme |
| **user_theme** | new: user, ordinary |
| theme_sheet, class_rule, stable_id_rule | the program sheet, ordinary |
| inline_style | the program inline, ordinary |
| **important_user_theme** | new: user, `!important` |
| important_theme_sheet | |
| forced_high_contrast | |
| important_inline_style | |

What changes then:

- the enumeration gains two values;
- the manifest's `cascade_levels` list grows (a schema change);
- the walk takes a third set with its own machine-number base;
- the user compilation emits at the two user levels, as an inline
  compilation emits at its two.

P9's logic needs no rework.

Forced high contrast stays where E1 put it, below program inline
`!important`. It sits above every user declaration, so a user theme can
never take away the system's high-contrast colours.

The user sheet's own selector kinds (type, class, stable ID) are ordered
by specificity within its level, not by separate levels as the program
sheet's are. That is enough, because no other sheet shares the user
levels.

### 4.3 Declaring the program sheet replaceable

This is a policy the program sets once, before its first window
(recommended):

```cpp
enum class UserThemePolicy : std::uint8_t {
    // The program's sheet is authoritative: the six-level order.
    layered,
    // A selected user theme takes the program sheet's place.
    replaces_program_sheet,
    // No user themes: the settings entry is absent and --theme is
    // refused.
    refused,
};
ApplicationResources::set_user_theme_policy(UserThemePolicy policy);
```

The default is `layered`. A program without a sheet behaves the same under
`layered` and `replaces_program_sheet` (case 1).

The alternative is a manifest key, `"user_themes": "replace"`, in the
program's `theme.json`. It travels with the sheet, but it cannot express
`refused`, it does not cover a program sheet built in C++, and an unknown
key is E601 today, so adding it needs a format version.

## 5. Selection, the built-in theme and the system appearance

- **One user theme at a time** (recommended), or none. None is the
  default.
  - It is chosen on the program's settings page, which lists the
    discovered themes by name, author and version.
  - It can also be chosen from the command line, as `--theme <name>`.
  - The choice is saved in the program's settings and takes effect at the
    next launch.
- **What the user theme replaces** depends on the policy (section 4): the
  default theme always; the program's sheet only under
  `replaces_program_sheet`. With no user theme selected, nothing
  changes.
- **System appearance**: the user theme's `@media (prefers-color-scheme)`
  and `(forced-colors)` rules follow the window's presentation settings.
  Those settings track the system only where the program opted in (owner
  direction 2026-10-28). By default the program keeps what it chose, and
  the user theme sees that.
- **A program may refuse user themes altogether** with
  `UserThemePolicy::refused` (for a kiosk, say). The settings entry is then
  absent.

## 6. Icon overrides

The program's icons are resources with identifiers: ASCII lowercase
letters, digits and dashes, with `.` between segments, as in the locale
keys (for example `toolbar.save`, `check-mark`).

A user theme overrides one by a file at
`themes/<name>/icons/<identifier>.png` (recommended), for example
`themes/harbour/icons/toolbar.save.png`. The alternative is a mapping in
the sheet, such as `@icon toolbar.save url("icons/disk.png")`, which
allows one file for several icons but is more to write.

The rules for an override:

- **It must decode.** It may be at most 1024 x 1024 pixels and 1 MiB. It
  is drawn into the icon's slot at the slot's size, as the program's own
  icon is.
- **A fully transparent image is refused**, so an override cannot make a
  control unrecognisable.
- **An unknown identifier is a warning.** The file is ignored.
- **The program can mark an icon not overridable**, the icon's
  counterpart of `!important`. Examples are a security indicator and the
  icon of the settings entry that leads back to the built-in theme. An
  override of a marked icon is refused with a warning.
- **Icons are resolved once at launch** through the compiled theme's
  image ownership (P10c).

New program-side API: icons are registered with an identifier and an
overridable flag. That is a small `ApplicationResources` addition, for the
T4 or FD work that draws icons.

## 7. Export

Export writes the program's current themes into a `themes/` folder, as a
starting point for the user. It is reached two ways, like the locale
skeleton:

- the command line, `--export-themes [folder]`, by default the per-user
  `themes/`;
- a settings entry, "Export themes...".

The exported themes are all the program's packages, with their resources
and icons:

- each package becomes `<name>.css` with its assets in `<name>/`;
- every overridable icon is written as `<name>/icons/<identifier>.png`;
- the built-in recipe is C++ data, not a sheet. It is the floor under
  every theme, and nothing is exported for it.

Export never overwrites. An existing file is skipped and listed in the
export's report.

One form is **a skeleton**. The exported sheet keeps the program's
selectors, with every declaration commented out:

```css
/* button { border-radius: 6px; } */
```

It opens with a `@theme` header. The exported theme therefore changes
nothing until the user un-comments and edits a line. A comment inside the
program's own sheet is written as `/* */` text with its `*/` broken (as
`* /`), so that comments never nest.

Recommended form: **a verbatim copy**, with its `!important` declarations
kept. Under the owner's order it changes nothing until edited:

- Under `replaces_program_sheet`, the copy takes the program sheet's place
  with the same declarations.
- Under the layered policy:
  - the copy's ordinary declarations sit below every declaration the
    program makes;
  - its `!important` ones land at `important_user_theme`, below the
    program's own `!important` that they repeat.

Every value is then live and visible to edit. Under the layered policy, a
user overrides a program declaration by adding `!important` to it. The
exported `README.txt` says so, and says that the program's own
`!important` declarations stay the program's.

## 8. Safety

A user theme is display only, and nothing in it can make the program
unusable without a way back that needs no reading.

- **No code and no network.** A user theme has:
  - no scripts;
  - no URLs other than relative paths inside its own folder;
  - no fonts other than the installed set the program already uses.
- **No layout.** The theme language has no layout properties. Scale and
  translation stay within the overflow the control grants (D5-1), so hit
  regions and layout cannot move.
- **The program's essentials stay the program's**, through `!important`
  under the layered policy (section 4) and non-overridable icons (section
  6). Examples are the
  settings entry and its route back to the built-in theme.
- **An engine floor on the user level.** At the user level, an `opacity`
  below 0.5 on any part is clamped to 0.5, with a warning (recommended
  value; owner to confirm). A theme then cannot hide a control outright.
- **Forced high contrast** stands over the user theme's colours (section
  4).
- **Refusal is whole.** A theme that does not compile is not applied (B2,
  RC003). The program starts without it and logs the diagnostics.
- **The way back needs no reading:**
  - `--theme built-in` on the command line, a literal ASCII word that is
    never translated;
  - holding Shift while the program starts, which starts it without a
    user theme for that run (the platform's convention may differ; to
    confirm per host).

  Both are documented in the program's help and in a `README.txt` that
  export writes into `themes/`.

## 9. Decisions for the owner (recommendations first)

1. **The theme marker**:
   - (a) a `@theme { ... }` header (recommended);
   - (b) every parseable CSS file named by the rule is a theme.
2. **The export form**:
   - (a) a verbatim copy (recommended; under the owner's order it changes
     nothing until edited);
   - (b) a skeleton with every declaration commented out.
3. **How many user themes**:
   - (a) one selected user theme (recommended);
   - (b) an ordered stack of them.
4. **`@import` in a user theme**:
   - (a) of the program's packages and its own partial sheets
     (recommended);
   - (b) none.
5. **Icon overrides**:
   - (a) the file convention `<name>/icons/<identifier>.png`
     (recommended);
   - (b) an `@icon` mapping in the sheet.
6. **Declaring the program sheet replaceable**:
   - (a) `ApplicationResources::set_user_theme_policy` (recommended);
   - (b) a `theme.json` key.

   ("Where allowed" itself is decided: the owner's six-level order.)
7. **The opacity floor** at the user level: 0.5, or another value, or
   none.
8. **The same name in both folders**: the per-user theme wins
   (recommended), as with translations.

## 10. Where it lands

- **After P12 and P11** in T3, or as its own stage, once the decisions
  above are taken.
- **The engine part** is small:
  - the `user_theme` and `important_user_theme` levels;
  - a third rule set in the walk;
  - the replaceable-sheet policy;
  - the opacity floor.
- **The rest is application-layer work:**
  - the scan;
  - the implied manifest;
  - the `@theme` at-rule in the parser;
  - the icon registry and its overrides;
  - export;
  - the settings entries.
