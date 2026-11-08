# FD form documents gate, 2026-11-08 (DRAFT: measurements pending)

**Status: draft.** Every gate item is listed below with the evidence it
needs and the command that measures it. No run has been made for this
record: GUI.Forms coders run nothing on the M4, and the integrator measures
it once the branches below are folded. Every "pending" line is filled in
from that run, and the record loses "DRAFT" when the last one is filled.

GUI.Forms state to measure: one checkpoint that folds all of these.

- FD-4: all 28 Gallery pages as documents (`claude/effects_lane-fd4-gallery`
  at `e90a7aee`).
- FD-5b: the colour and font refusal, E813 (cp20, `af173593`, `e417548c`).
- FD-6: bindings in the form (w7 `claude/fd6-bindings` at `7d7fe052`,
  folded in batch 5).
- Part B: the window is the form (w7 `claude/window-is-form` at
  `c2b46eb4`).
- The X-3 reference app (template_harness `claude/x3-reference-app` at
  `335f5b8`).

Checkpoint and commit: pending. Host: the M4 (arm64 macOS), Homebrew clang
22.1.8, as for T3. Windows and Linux: pending (CI on the checkpoint PR).

## The gate (stage brief, accepted 2026-10-13)

> a corpus of documents round-trips byte for byte when untouched; a
> scripted edit of one property changes only that property's bytes; every
> Gallery page can be expressed as a document whose generated C++ compiles
> against the installed SDK and passes its tests.

### 1. Untouched documents round-trip byte for byte

- **Corpus.** `tests/form_language/corpus/` holds `gallery.form`,
  `settings_dialog.form` and `spacing_and_comments.form`. The corpus also
  covers the 28 Gallery page documents (`demo/pages/*.form`),
  `demo/gallery.form`, the generated-test documents
  (`tests/form_language/generated/*.form`), the installed example
  (`examples/reference/collection_gallery.form`) and the reference app
  (`example_app/forms/greeter.form`).
- **Test.** `gui_forms_form_syntax_tests` (`test_corpus`,
  `test_line_endings`: LF, CRLF and CR, a byte order mark) and
  `gui_forms_form_editor_tests` (a document read and written untouched).
- **To record:** the number of documents and bytes round-tripped, and
  whether the page documents are in the test's corpus or only parsed by the
  build. If they are only parsed, add them to `test_corpus` before the gate
  and record the change. **Pending.**

### 2. A scripted edit of one property changes only its bytes

- **Test.** `gui_forms_form_editor_tests`:
  - `test_properties`: a value replaced, with the bytes around it equal; an
    added and a removed property; escapes written as the value's bytes;
    FD-6 bindings written by the editor.
  - `test_objects`: an object added, removed or moved, its bytes kept.
  - `test_line_endings`.
- **To record:** the edits the test scripts, and that for each one the
  difference is the property's bytes only (the test's `check_text`).
  **Pending.**

### 3. Every Gallery page is a document whose generated C++ compiles against the installed SDK and passes its tests

- **Pages.** 28 of 28 pages are documents (FD-4). Each page's document is
  generated at build time with `gui_forms_add_form`, and the page tests run
  against the generated classes (`gui_forms_gallery_pages_tests` and each
  page's own test).
- **Installed SDK.** Two consumers generate from an installed SDK:
  - `examples/reference` (`collection_gallery.form`, derived window with
    `MODEL`), configured against the installed prefix by
    `tools/ci_build.py`;
  - the template harness `example_app` (form, model, theme package, rule),
    built against the local checkout's install with
    `harness/lockstep.py prefix`.
- **Open point for the owner (wording, not engineering).** The Gallery pages
  are generated in-tree by the same form compiler the SDK installs
  (`GUIForms::form_compiler`), but they are not configured against the
  installed prefix. Two ways to close the gap:
  1. **(recommended)** Add one installed-consumer test that runs
     `gui_forms_add_form` from the installed prefix over every
     `demo/pages/*.form` and compiles each generated header. It does not
     link the Gallery, so it proves the gate's wording with little build
     cost.
  2. Accept the in-tree generation together with the installed examples as
     meeting the gate, and say so in this record.
- **To record:** the page count, the page tests passing (plain and under
  ASan, UBSan and LSan), and the installed-consumer results. **Pending.**

## Also in the stage brief (not gate items, recorded for the stage)

- **Validation before mutation.** A document naming an unknown control,
  property or state is refused with its code, line and column before
  anything is built (`gui_forms_form_model_tests`, `gui_forms_form_loader_tests`,
  `gui_forms_form_compiler_refusal_tests`). Recent refusals:
  - E812: a derived class that binds without `MODEL`;
  - E813: colour and font properties;
  - E816: `on-` handler references;
  - E817: an unbindable property or a malformed binding;
  - E818: a misplaced binding option;
  - E819: a missing or mistyped source;
  - E999: `convert`, reserved.
- **Hostile input.** The grammar shares T3's tokenizer and limits.
  `test_depth` and `test_declaration_limit` check the depth and declaration
  bounds. **To record:** whether a form-document fuzzer is wanted for the
  seal. T3 fuzzed the shared front end
  (`results/2026-11-01-t3-theme-language-gate`).
- **Generated code** follows house style: the generated headers are checked
  by `tools/check_house_style.py` in the build. **To confirm.**
- **User documentation.** On `claude/effects_lane-rules` at `64b55e13`:
  `docs/user/forms.md`, `properties.md`, `themes.md`,
  `converting-an-app.md`.

## Commands (for the integrator)

```text
python3 -B tools/ci_build.py --gallery --tests 'gui_forms_form_|gui_forms_gallery_pages_tests|gui_forms_form_derived_class_tests'
python3 -B tools/ci_build.py --gallery --sanitize --tests 'gui_forms_form_|gui_forms_gallery_pages_tests|gui_forms_form_derived_class_tests'
python3 -B tools/ci_build.py --gallery            # the installed examples run at the end
python3 -B template_harness/example_app/ci_build.py --gui-forms-prefix "$(python3 template_harness/harness/lockstep.py prefix)"
```

Record each command's exit status, test counts and duration. Keep any
failure as a negative result before fixing it.

## Results

Pending.
