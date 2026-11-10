# W: Windows Forms control coverage, 0.4 to 0.6 (w5)

Workspace `D:\gui_forms_w5\gui_forms` (Git Bash `/d/gui_forms_w5/gui_forms`). Scratch files in
`D:\gui_forms_w5\dev\`. Task (coordinator, 2026-10-25): survey every WinForms control in the
catalogue (done/partial/missing), record DK and RB phases, recommend the next batch; then build the
batch, one PR per control with auto-merge, BRIEF's new-control checklist in full each time.
Carry on between controls; stop and report only for an owner decision that cannot be taken
conservatively. No agents.

## Decisions in force

- Survey states: done = counterpart covers core use + tests + Gallery; partial = counterpart lacks
  a named core part; missing = none. The catalogue `status` column is stale (not remapped since
  2026-10-14); the survey reads the code.
- Batch (survey, 2026-10-25), in order: 1 `OpenFileDialog` with `CommonDialog`/`FileDialog`;
  2 `SaveFileDialog`; 3 `FolderBrowserDialog`; 4 `ColorDialog`; 5 `DataGridViewImageColumn` and
  `DataGridViewLinkColumn`.
- The dialogs are components over the existing host service (`HostServices::show_dialog`,
  `HostOpenFileDialogRequest` etc.), template `NotifyIcon` (`include/gui_forms/components/`,
  `src/controls/notify_icon/`). No in-window fallback where a host refuses (conservative; an
  owner question if wanted).
- Held for owner decisions (reported, not built): `RichTextBox` scope (matrix row 42), `Splitter`
  (row 45 facade), `MessageBox` facade (TaskDialog or host message dialog).

## State (2026-10-25, end of the batch's build)

Every batch control is built and verified locally. PRs and branches:

- Survey #319 (`claude/w-coverage-survey`), auto-merge on.
- 1 `OpenFileDialog` #327 (`claude/w-open-file-dialog`), auto-merge on. First CI failed only the Linux
  AT-SPI gallery list (tests/linux_accessibility_gallery_client.py NONVISUAL); fixed a36ed713; branch
  updated server-side. Worktree of this branch: `D:\gui_forms_w5\dev\wt_open`.
- 2 `SaveFileDialog` #330 (`claude/w-save-file-dialog`, base `claude/w-open-file-dialog`), ready,
  auto-merge NOT on (a feature-branch base could fold it into #327). When #327 merges and #330 shows
  base main: `gh pr merge 330 --auto --merge`.
- 3 `FolderBrowserDialog`: branch `claude/w-folder-browser-dialog` pushed (b6e3d399, on #330's head
  defe83b1), NO PR yet (it would be a third layer). PR text `D:\gui_forms_w5\dev\pr_folder.md`.
- 4 `ColorDialog`: branch `claude/w-color-dialog` pushed (31a89bf2, on defe83b1, a sibling of 3),
  NO PR yet. PR text `D:\gui_forms_w5\dev\pr_color.md`.
- 5a `DataGridViewImageColumn` #335 (`claude/w-dgv-image-column`, from main), auto-merge on.
- 5b `DataGridViewLinkColumn`: HELD, vocabulary gap (no link family/role in the by-role paint path;
  LinkLabel paints directly with style().link/visited_link). Reported to the coordinator.

Next steps, in order:
1. When #327 has merged: open the PRs for 3 and 4 with base `claude/w-save-file-dialog`
   (`gh pr create --base claude/w-save-file-dialog --head <branch> --body-file <text>`), ready, no
   auto-merge yet.
2. When #330 has merged (3 and 4 retarget to main): `gh pr merge <n> --auto --merge` on both. The
   second of 3 and 4 to land conflicts with the first in the shared lists (CMake, gui_forms.hpp,
   manifests/controls.json, the six nonvisual lists, schema_export_tests, the Gallery page and its
   test) and the three display digests: merge main, keep both entries, re-run
   `sh /d/gui_forms_w5/dev/redigest.sh <dir>` (compares with dev/trace_save) and
   `python dev/set_digests.py <controls> <runtime id> page=0x...` (controls 1230, RuntimeId #1255 with
   both), regenerate the registry/docs/schema, build and test the touched tests.
3. A CI failure on any of these: read the failed job's log and fix on that branch.

Decisions taken conservatively (reported): no public CommonDialog/FileDialog bases (house style 7;
shared `file_dialog_types.hpp` + private `src/controls/file_dialog/file_dialog_support.hpp`); host
dialogs only, no in-window fallback; file_ok cancel re-shows the dialog; SaveFileDialog uses
default_ext only (no filter index in the host request) with .NET's HasExtension rule; the four
dialogs are registry category "Dialogs and popups" (WinForms toolbox Dialogs tab), which keeps
dialogs-and-host-services in its Gallery nav group; FolderBrowserDialog description -> host title,
selected_path else initial_directory is where it starts, no root folder/new-folder toggle;
ColorDialog keeps a title, allow_alpha off by default with alpha forced opaque, no custom colors;
image cells: ImageId values, normal/stretch/zoom, image_description, content click, no edit.

## How a component dialog is wired (checklist as applied)

Header under `include/gui_forms/components/<name>/`, source `src/controls/<name>/`, both in
CMakeLists (sources after notify_icon.cpp; test after gui_forms_notify_icon_tests, linking
gui_forms_controls and gui_forms_host_headless); `gui_forms.hpp` include; `friend class <Type>;` in
window.hpp if it observes the window lifetime; registry record in Components beside notify_icon, then
control_registry emit-cpp/render/readme-table/check; add the id to the nonvisual lists in
tests/gallery_pages/anatomy_coverage_tests.cpp, schema_export_tests.cpp (construct + record_component),
windows_accessibility_{msaa,uia}_gallery_tests.cpp, tests/support/stock_control_fixture.cpp
omit_components, tests/linux_accessibility_gallery_client.py NONVISUAL; Gallery specimen on dialogs-and-host-services (the matching path button); docstring;
generate_library_docs.py and --check; write_schema.sh, control_schema check and render; display trace
digests: GUI_FORMS_GALLERY_TRACE_PRINT=1 gui_forms_gallery_pages_tests.exe --display-trace prints them
(each new registry line on a page adds one control: states-and-diagnostics and typography-and-inspection
change too).

## Environment lessons

- Bash heredocs in this tool collapse `\\` to `\`: never write backslashes through a heredoc (a
  Python fix wrote a NUL byte into a source). Use the Write/Edit tools, or chr(92) in scripts.

- Files git has checked out are CRLF in the working copy (autocrlf): edit scripts must use
  Path.read_text/write_text (universal newlines), never read_bytes with LF-only patterns.
- The Gallery groups a page under the registry category its records name most often; adding
  records to a page can move it in the navigation (every page's display digest then changes).
- build.sh rebuilds most of the tree after a gui_forms.hpp or widely included header change;
  run it in the background and wait with an until-loop on its output file.

## PRs

| Control | Branch | PR | State |
|---|---|---|---|
| survey | claude/w-coverage-survey | #319 | auto-merge on |
| OpenFileDialog | claude/w-open-file-dialog | #327 | auto-merge on |
| SaveFileDialog | claude/w-save-file-dialog | #330 | base #327; auto-merge after #327 lands |
| FolderBrowserDialog | claude/w-folder-browser-dialog | none yet | pushed b6e3d399; PR after #327 lands |
| ColorDialog | claude/w-color-dialog | none yet | pushed 31a89bf2; PR after #327 lands |
| DataGridViewImageColumn | claude/w-dgv-image-column | #335 | auto-merge on |
| DataGridViewLinkColumn | none | none | held: vocabulary gap |
| ThreadExceptionDialog | claude/w-thread-exception-dialog | #341 | auto-merge on (batch 2, item 1) |
| owner decisions + rich text brief (docs) | claude/w-owner-decisions-rich-text | #342 | auto-merge on; worktree dev/wt_docs |
| TabControl tab images | claude/w-tab-control-images | #343 | auto-merge on (batch 2, item 2) |

## Batch 2 (coordinator, 2026-10-25): items needing no owner call

Owner answers pending on an interview page (via the coordinator): RichTextBox scope, Splitter, the
MessageBox facade, a link role for DataGridViewLinkColumn. Do not build those.

1. ThreadExceptionDialog #341 (from main d0f3cae3): component over an owned TaskDialog; closed
   reports cancel (Continue/Escape/revoked) or abort (Quit); describe_exception(exception_ptr);
   warning report = Continue only. Nonvisual lists: inserted after notify_icon (not at the end) so
   it conflicts only in the counts with the file-dialog PRs. Digests on main: 1227 controls, #1252.
2. TabControl tab images (ImageList + TabPage image_key/image_index, delegating to PageStrip's
   image_list/tab_icon): next.
3. Then ListView depth taken conservatively: item tooltips, label editing, insertion mark (drop
   role), groups (caption + divider, non-collapsible, non-virtual), tile view.
Owner question raised instead of building: owner drawing (ListView, TabControl and the list family).

## Owner answers, 2026-10-25 (W interview round 1; D:/gui_forms_workers/interviews/w-interview-answers.json)

Recorded as decisions 1 to 4 of 2026-10-25 in the W README (#342).
- W-Q1 RichTextBox: an inline markdown-like run model; RTF through a standalone library (proposed
  `stx_rtf`) built by a SwiftEdit worker. The brief is planning/stages/W-winforms-controls/RICH_TEXT_BRIEF.md
  (#342). Do NOT build the parser. Build the run model only after the brief lands AND the coordinator
  confirms.
- W-Q2 Splitter: build a thin facade over the dock layout.
- W-Q3 MessageBox.show: build a facade over the host's message dialog (HostMessageDialogRequest).
- W-Q4 link role: its own vocabulary PR first (PartRole or role family "link"; states normal, hot,
  visited, disabled). Built-in themes supply it. LinkLabel moves onto it with goldens unchanged.
  Then DataGridViewLinkColumn. There is no vocabulary "version/digest" in code (checked: only the
  schema's vocabulary lists), so the PR states what changes: the schema vocabulary list, the
  design-language vocabulary table, and the theme recipes.
Order after #343: link role PR, then MessageBox facade, Splitter, link column, then ListView depth.

## Owner answers, 2026-10-26 (via the coordinator)

Recorded as decisions 5 and 6 of 2026-10-26 in the W README (pushed to #342, commit 3bf88c3b).
- W-Q5 link role: two families, `link` and `visited_link`, chosen by the link's visited flag;
  hover keeps the normal link colour. Vocabulary PR first (LinkLabel goldens unchanged), then
  DataGridViewLinkColumn on top. Template for adding a family: commit 5c4ef397 (tool_tip family,
  RB-38): theme_types enum + control_visual_role_name, theme.cpp professional recipes, schema
  vocabulary "families" list (tools/control_schema.py, manifests/control-schema.json,
  schema_export_tests), design language table, part_painting tests.
- W-Q6 owner drawing: content-only. A per-item draw event gets our Painter and the content rect;
  plate, selection and focus stay by role. One cross-control seam PR first, then ListView and
  TabControl adopt it, later the ListBox family.
- RT-Q1: the library is `stx_rtf2md`; RICH_TEXT_BRIEF.md renamed and its questions answered
  (attribute span with sup, sub, colour, background, size; code page 1252 only) in #342.

SwiftEdit's three stx_rtf2md seam refinements (code block = one unstyled run with LF; list item
`ordinal` 0..999999999 in place of ordered_start, consumer may renumber; fallback span `[x]{b}`,
keys b, i, s): no objection from W (not replied, as asked). The run model must accept LF only in
code-block runs and may renumber ordinals on display.

## State (2026-10-26)

PRs open: #319 survey, #327 OpenFileDialog, #335 DGV image column, #341 ThreadExceptionDialog,
#342 docs, #343 TabControl images, #346 Splitter (895f33be, from main 8d1f2be0) all auto-merge on.
#330 SaveFileDialog base claude/w-open-file-dialog, auto-merge OFF until retargeted to main.
Branches with no PR: claude/w-folder-browser-dialog (b6e3d399), claude/w-color-dialog (31a89bf2);
bodies dev/pr_folder.md, dev/pr_color.md; open them (base claude/w-save-file-dialog) after #327.

Splitter lesson: a registry record with a category can move its page to another catalogue group
(the page goes under its most-named category; ties keep the current one). Every page's digest
then changes through the side navigation. Choose the gallery page so no group moves.

Opened 2026-10-26, all from main with auto-merge on, CI pending:
- #347 link role (claude/w-link-role, a9bdf6d5): link + visited_link families appended after
  tool_tip; fill_link_role (clear plates, compatibility link colours, disabled_text); material and
  glass recipe choices extended and asserted; LinkLabel paints from the family unless set_style
  override (outside high contrast). No display trace moved.
- #348 owner-draw seam (claude/w-owner-draw-seam, 0292a4f0): gui_forms/theme/owner_draw
  (ItemDrawMode, ItemContentPaint{painter&, index, column, stable_id, content, states, font, text,
  draw_default}, raise_item_content_paint with a save/clip/restore scope, item_content_text);
  tests/owner_draw_tests.cpp; design language 8.2 item 4.

Also opened: #352 insertion mark (main, auto on), #354 owner-draw adoption (base #348, auto
OFF until retargeted to main after #348 merges).
Merges 2026-10-26: #327 took main (ThreadExceptionDialog #341) at 8d7efcfa (three digests
re-recorded; nonvisual list sizes 21); #330 took #327 at fe130e9e (sizes 22, digests again);
#354 took #348's update (tab images #343 meet owner draw in PageStrip::paint_tab).
Conflict recipe for stacked dialog branches: keep both sides in CMake/includes/nonvisual lists,
bump the std::array sizes, take --theirs for generated files (catalog, schema, rst, controls.json
then re-add the record), regenerate, re-record the three digests (states count, dialogs page,
typography RuntimeId comment).

#357 ListView label editing (claude/w-list-view-label-edit, a954796a), base #352 (retarget to
main + auto-merge after #352 merges). Editor made lazily on first edit (no gallery digest moves);
model changes cancel AFTER the change (event index = position when edit began); press in view
commits then re-handles the press; scrolling keeps the edit (editor at no size, as DGV). Slow
second click NOT done: host reports no double-click interval (out of domain).
Header rule: public Event accessors must be in the same order as the interactions table
(control_schema check fails otherwise).
Env: never wrap `cmake --build` in `timeout` (ccache then loses USERPROFILE).
Rule (BRIEF, 2026-10-26): before every commit run `git diff --cached --stat`; files not edited or
whole-file changes mean CRLF staging (toolchain_env.sh shells) -> reset and restage from a fresh
shell. Checked a954796a, 8d7efcfa, fe130e9e, 97cd9be2: no CRLF introduced.
Coordinator 2026-10-26: main no longer requires up-to-date branches; T3 is moving ~40
Theme::resolve sites in src/controls (approved). Overlaps #347/#348/#354: whichever lands
second rebases; don't fight it.

Coordinator 2026-10-26 (later): T5 is adding the double-click interval as a reported host
setting (all hosts; XSETTINGS default on X11). When the coordinator sends its field name, add the
slow-second-click label edit to ListView (timer from that interval; a double click cancels it).
Strict up-to-date is off on main: merge/rebase only for real conflicts or to retarget stacks.

Next, in order:
1. (done #352) 2. (done #354) 5a. (done #357 label editing; slow click waits on T5's field)
   Slow click: field is PresentationSettings::double_click_milliseconds (std::uint32_t) via
   Window::presentation_settings(); double_click_origin = host or stated 400 ms default;
   headless fixed 500 ms. In #358 (after T5's #331). Build once #358 is on main.
3. DataGridViewLinkColumn once #335 (image column) and #347 merge (it overlaps both).
4. MessageBox.show facade, stacked on #330 once #330 is main-based.
5. ListView label editing (DataGridView begin_text_edit is the model: hosted TextBox,
   before/after events with cancel), groups, tile view, item tooltips.
6. RichTextBox run model: coordinator GO-AHEAD given 2026-10-26 (no further confirmation). Build
   it on stx_rtf2md's document.hpp (#349, RT1, stacked on #344) once #349 is on main, as its own
   PR, after the link-role (#347) and link-column PRs.

## PAUSED for org move (2026-10-26)

Nothing in progress; the working tree in D:\gui_forms_w5\gui_forms is clean on
claude/w-list-view-label-edit. Every branch is pushed. After the move the remote is
github.com/malkuth-software/gui_forms, so run `git remote set-url origin` first.

Merged: #327 OpenFileDialog, #335 DGV image column, #342 docs, #347 link role,
#348 owner-draw seam, #349 RT1 (stx_rtf2md document.hpp).

Open PRs and branch heads (origin):
- #352 claude/w-list-view-insertion-mark 53ae71b9 -> main (auto on)
- #346 claude/w-splitter 3ba8b355 -> main (auto on)
- #330 claude/w-save-file-dialog fe130e9e -> claude/w-open-file-dialog (MERGED base):
  retarget to main, then auto-merge
- #354 claude/w-owner-draw-adopt 97cd9be2 -> claude/w-owner-draw-seam (MERGED base):
  retarget to main, then auto-merge
- #357 claude/w-list-view-label-edit a954796a -> #352: retarget after #352 merges
Branches without a PR (base claude/w-save-file-dialog; bodies dev/pr_folder.md, dev/pr_color.md):
- claude/w-folder-browser-dialog b6e3d399
- claude/w-color-dialog 31a89bf2
Open them once #330 is main-based or merged.

Next, on resume:
1. Retarget #330 and #354 to main and enable auto-merge; open the FolderBrowserDialog and
   ColorDialog PRs.
2. DataGridViewLinkColumn, from main (#335 and #347 are both in).
3. RichTextBox run model on #349's document.hpp (go-ahead given), after the link column.
4. MessageBox.show facade, stacked on #330 or main.
5. The slow-second-click label edit, once #358 is on main
   (PresentationSettings::double_click_milliseconds).
6. ListView groups, tile view and item tooltips; the ListBox family's owner-draw adoption;
   an owner-draw Gallery specimen.
Watch for: the T3 resolve-move PR touching link_label.cpp and owner_draw.cpp. Whichever lands
second rebases.

## PAUSED for the night (2026-10-26)

The PAUSED-for-org-move notes above still hold. Since then the remote has moved to
malkuth-software/gui_forms (origin already repointed), and the coordinator has retargeted #330
and #354 to main.

In flight, NOT committed: in D:/gui_forms_w5/gui_forms, on claude/w-list-view-insertion-mark
(HEAD 53ae71b9, #352), a merge of origin/main is in progress.
- Conflict: tests/gallery_pages/display_trace_tests.cpp, resolved by taking main's side (--theirs).
- Still unmarked (git add not yet run): the typography-and-inspection digest and the "#N rather than" comment need re-recording. #352's
  own value was 0x1f63ef79c50899a0; main's was taken.
- demo/gallery_catalog.inc was regenerated.
- The build and the trace test were stopped before they finished.

Resume:
1. Rebuild; run gui_forms_gallery_display_trace_tests and dump the traces; record the changed digests
   and comment; run list_view, gallery_pages and anatomy plus the sanitizer; `git diff --cached --stat`;
   commit the merge; push.
2. Do the same for #346 (claude/w-splitter, 3ba8b355), which is DIRTY against main.
3. #357: merge the updated #352 branch in, or retarget it to main once #352 merges.
4. Then the PAUSED org-move list, from item 1's PR openings (FolderBrowserDialog and ColorDialog).
   #330 and #354 are already retargeted, so only enable auto-merge on them.
5. New backlog item, last: a plain letter in an open dropdown menu activates the matching
   mnemonic, as native Windows menus do. Today only Alt+letter works, via Window::dispatch_dialog_key.
   - Where: src/controls/menu/context_menu/context_menu.cpp, the MenuPanel and MenuRow on_key handlers.
   - Approach: handle unmodified text input. A unique match activates (or opens a submenu);
     several matches cycle focus. Use MenuRow::mnemonic_matches and is_mnemonic.

## LAND AND FREEZE (2026-10-27, D:/gui_forms_workers/LAND_AND_FREEZE.md)

Start nothing new. Land every finished branch, then write FROZEN here.

Landing (state as of this entry):
- #352 insertion mark: in the merge queue.
- #346 Splitter: base main, auto-merge on.
- #357 label editing: base #352's branch; retarget to main once #352 merges, then auto-merge.
- #354 owner-draw adoption: was failing house-style (control_schema: list_view, tab_control and
  page_strip lacked the draw_item_content interaction). Fix: add it first in each control's
  interaction table, merge origin/main, regenerate manifests/control-schema.json. Done in the
  worktree D:/gui_forms_w5/dev/wt_main (branch claude/w-owner-draw-adopt, build .build/main-trace).
- #385 FolderBrowserDialog: base main, auto-merge on.
- ColorDialog (claude/w-color-dialog): merged with claude/w-folder-browser-dialog (stacked on #385);
  its PR goes against that branch, then is retargeted to main once #385 merges.

PARKED (pushed, no PR):
- MessageBox.show: claude/w-message-box at 7ce48f0e. PR #377 is closed with a parking comment.
  The PR body is in D:/gui_forms_w5/dev/pr_message_box.md.

PARKED (not started, no branch): the link role and link column; the plain-letter menu mnemonic
fix (notes under "PAUSED for the night"); the slow-second-click label edit (builds on #358's
PresentationSettings::double_click_milliseconds); the RichTextBox run model.

Update: ColorDialog is PR #388 (base claude/w-folder-browser-dialog, head ae8f5671). #354 is fixed
at df52b6d2 (merge of main plus the interaction-table fix; sanitizer clean); auto-merge is on.
#346 fix at b2d97949: a targetless splitter no longer turns hot (macOS AX client metrics_view
mismatch, one paint behind). Reported to the coordinator: MetricsView's live semantic value goes
stale after paint-only changes (out of domain).
#352 MERGED. #357 retargeted to main by GitHub and is in the merge queue.
Correction: #346's macOS failure is the MetricsView one-paint lag fixed by #350 (not the splitter hover;
b2d97949 stands as a docstring-conformance fix; PR body corrected). Next: when #350 merges, merge
origin/main into claude/w-splitter and push. #354 re-merged main after #352 (31b61049; sanitizer clean).
Merge queue OFF (owner fold). #385 closed as included in #388 (ColorDialog + FolderBrowserDialog, base
main, 1bbb8b30, auto on). #346 merged main at 2b64ce23 (1239 controls, #1272); merge main again
once #350 lands. #354 at 31b61049, #357 base main, auto on.
#354 MERGED. #357 merged main (owner drawing) at 5ea4013f, sanitizer clean, auto on.
Open: #346 (2b64ce23), #357 (5ea4013f), #388 (1bbb8b30); all base main, auto on.
#388 MERGED. #346 merged main (#388) at d54e5553 (1241 controls, #1274).
#357 MERGED.
#346 MERGED. #402 (Splitter switch default, F1 gate fix, 08ecb775 amended) open on main, auto on.

## FROZEN (2026-10-27)

No W PRs are open. Landed on main during LAND AND FREEZE:
- #330 SaveFileDialog
- #352 ListView insertion mark
- #354 ListView and TabControl content-only owner drawing (W-Q6), with the draw_item_content
  interaction export
- #357 ListView label editing
- #388 FolderBrowserDialog and ColorDialog (#385 closed as included)
- #346 Splitter
- #402 Splitter switch default (state-machine gate F1)

PARKED:
- MessageBox.show: claude/w-message-box at 7ce48f0e, pushed, no PR (#377 closed with a parking
  comment; body in D:/gui_forms_w5/dev/pr_message_box.md).
- Not started: the link role and link column; plain-letter mnemonics in open dropdown menus (notes
  under "PAUSED for the night"); the slow-second-click label edit (on #358's
  PresentationSettings::double_click_milliseconds); the RichTextBox run model.

Leftovers: the worktree D:/gui_forms_w5/dev/wt_main (on claude/w-splitter-switch-default) and
the build D:/gui_forms_w5/.build/main-trace can be removed; local merged branches can be deleted.

## Phase D step D3 (2026-10-27): example_app and reusable workflows

Clone D:/gui_forms_w5/template_harness, local branch `example-app` (not pushed; the coordinator folds it in).
Commits on d3925fa: dfeea67 example_app (+ projects/example_app.toml), 23e0a07 .github/workflows/harness-{native,house-style,state-machines}.yml + example.yml, e62c224 README rows.
Decisions in force: owner option 1 (GUI.Forms releases publish the SDK; the standby makes release.yml change). Reusable workflows live in .github/workflows/ (GitHub requirement), not workflows/. harness.lock format assumed `[harness] repository, commit`. windows_toolchain.py restore gets GITHUB_REPOSITORY overridden to the project's toolchain repository in-workflow.
Verified: harness tests (237 OK), example_app tests (9 OK), house style + lifetime rules over example_app/src,tests clean. actionlint absent; no YAML parser locally.
Next: when the coordinator forwards the first SDK release tag and asset names, adapt sdk_asset/fetch if the layout differs, run `python example_app/fetch_sdk.py pin TAG`, ASK before downloading the windows-x64 SDK (file, source, size), then build/test example_app on Windows with the pinned toolchain (ci_build.py from D:/gui_forms_w5, cache outside the tree).
2026-10-27 update: D3 folded as template_harness PR #1 (branch example-app, held until the SDK is pinned). Clone reset to origin/example-app 7309064; local commits on top: cd9cd57 (pin from SHA256SUMS.txt, SDK layout per GUI.Forms PR #406, URLError handling), 6a6e622 (windows_toolchain restore reads project toolchain_repository, GITHUB_REPOSITORY fallback; workflow override dropped). Coordinator answers: harness.lock shape right, .github/workflows/harness-*.yml accepted, keep python-tests. PR #1 run 40459602133: YAML valid; python-tests, house-style pass; all four native jobs pass every setup step (Windows toolchain restored, macOS runtimes built) and fail only at "gui_forms.lock pins no SDK archive" (expected). Next: coordinator pushes; on the SDK tag, `fetch_sdk.py pin TAG`, ask before the ~33.8 MB windows-x64 download, then build on Windows.
2026-10-27: coordinator pushed both commits and merged harness main; origin/example-app is 1137c74, clone reset to it. ON HOLD until the coordinator sends the SDK release tag.
2026-10-27: owner direction: source SDK (gui-forms-sdk-<version>-source.tar.gz, top dir gui-forms-sdk-<version>/, SDK_MANIFEST.json 'build' -> tools/build_sdk.py --target --prefix --jobs --work; network_inputs listed). Local commit dd42a0d on 1137c74 reworks lock (schema 2, [source]), fetch_sdk (fetch+verify+unpack+build into <cache>/gui-forms-<version>-<target>/, build record = archive digest+target+toolchain identity; host-only target; --allow-network-inputs), ci_build, workflows (install_cache_paths/key_files, HARNESS_CACHE=.harness-cache, HARNESS_TOOLCHAIN_IDENTITY). Not pushed; waiting for coordinator push and the final names/tag. Download nothing without asking.
2026-10-27: checked PR #407 against the contract; local commit (network_inputs filtered by target platform) on dd42a0d. Holding for the tag.
2026-10-27: pinned v0.2.1308 (7d0c6a5, local). Windows: fetched 56.3 MB source, built GUI.Forms (541 steps, ~6 min cold, ccache 17/534 hits), install 90 MB/499 files at D:/gui_forms_w5/.harness-cache; example 6 steps, both tests pass; reuse run: no download, no build. No fixes needed. Waiting for coordinator push / CI.

## D3 CLOSED (2026-10-27)
D3 is done and merged (template_harness PR #1: example_app on the v0.2.1308 source SDK, reusable workflows harness-native/house-style/state-machines + example.yml, windows_toolchain restore from project toolchain repository). No background work running. Open items (none blocking):
- macOS local fetch over-reports the llvm-runtimes-source network input when GUI_FORMS_LLVM_RUNTIME is supplied by the harness (CI passes --allow-network-inputs, so no effect there).
- Cross-target builds are refused by design until per-target toolchains exist (fetch_sdk --target is the parameter).
Local leftovers outside any checkout: D:/gui_forms_w5/.harness-cache (source, install, work tree) and D:/gui_forms_w5/.build/native-windows-x64/example-app; safe to delete.

## SDK + pinned compiler cache, then local lockstep (2026-10-27)
Owner re-prioritised: local builds first, cloud pins later (PHASE_D_DESIGN.md "Local lockstep before cloud pins").
- gui_forms PR #410 (SDK_MANIFEST revision + compiler_cache; package_sdk --compiler-cache only for from_native main pushes) MERGED.
- template_harness local branch `sdk-cache` (NOT to be pushed yet; the cloud switchover waits): a96ef74 (lock schema 3, pin fills [compiler_cache], fetch in CI layout + digest-checked cache restore + hit report), 3795a95 (CRLF + LANG, below). Base f73a6f9.
- Hit-rate finding: CI's Windows ccache keys need CRLF sources (runner git core.autocrlf=true; no .gitattributes in gui_forms except amiri OFL.txt -text and third_party *.patch eol=lf) AND LANG=en_US.UTF-8 (MSYS2 login shell; ccache hashes LANG/LC_*). Objects are byte-identical either way. LF+no LANG: 3/526 hits; CRLF+LANG: 525/526 (99.8%); run 12:10-12:15 incl. CRLF rewrite of 4912 files, vs ~11 min cold. Other platforms' LANG unmeasured.
- Possible later owner decision: a gui_forms .gitattributes (`* -text`) would make every checkout byte-exact LF and drop the CRLF emulation (every Windows working tree would need a re-checkout).
- Lockstep: worktree D:/gui_forms_w5/th_lockstep, branch `lockstep` off origin/main 4437b47: harness/lockstep.py + test_lockstep.py (5 tests, hermetic, stable x6). Layout: root D:/malkuth; workspace gui_forms_lockstep/ holds the clone gui_forms/ and .build/native-<target>/ (CI layout, CCACHE_BASEDIR=workspace, clone with core.autocrlf=true on Windows, LANG=en_US.UTF-8); cache/ccache; cache/lockstep/gui-forms-<commit>-<target>/ + current-<target>.json + .lockstep.lock.
- Next: example_app/ci_build.py --gui-forms-prefix; docs/adopting.md "Local lockstep builds"; init with --seed (D:/gui_forms_w5/gui_forms for the text stack .git dirs, the SDK tree for miniaudio); measure cold / one-file rebuild / example_app against prefix; commit; report record field names to the coordinator.
- 2026-10-27 lockstep DONE locally. Commit 869f762 is on template_harness branch `lockstep`, in worktree D:/gui_forms_w5/th_lockstep, off origin/main 4437b47, for the coordinator to push. It contains:
  - harness/lockstep.py and its tests;
  - example_app/ci_build.py --gui-forms-prefix;
  - docs/adopting.md "Local lockstep builds";
  - docs/contract.md (a provisional row);
  - README and CHANGELOG entries.
- The tree is initialised and built: D:/malkuth/gui_forms_lockstep/gui_forms at 79f18fc8.
  - Seeds: the text stack from D:/gui_forms_w5/gui_forms; miniaudio (with LF restored) from D:/gui_forms_w5/seed.
  - Its current-windows-x64.json points at cache/lockstep/gui-forms-79f18fc8…-windows-x64.
- Measured on Windows with --jobs 8:
  - cold build: 11.5 min, 0/526 hits;
  - one-file change: 15 s, 1 compile plus 4 links;
  - example_app against the prefix: 16 s fresh (configure, 6 steps, both tests pass);
  - a matching build is a no-op in about 1 s.
- Record field names were sent to the coordinator. The measurement branch was deleted and its install removed.
- Open:
  - a text-stack pin bump makes fetch_text_stack.sh fetch over the network;
  - other platforms' LANG is unmeasured;
  - the .gitattributes owner option (above);
  - pinned_cache_revisions.json should list the example's pinned commit when the cloud pin lands.
- 2026-10-27: lockstep 869f762 is on template_harness main (5e3ce6a). The owner keeps the CRLF copying, so sdk-cache stays local until the cloud switchover.
  - Commit b66b283 on branch `lockstep` (worktree th_lockstep, on 5e3ce6a, for the coordinator to push): build always runs `--features all`; the record gains a `features` field, which is part of the key; a commit without `--features` is refused; 6 tests.
  - As the coordinator's delegate, I advanced the lockstep this once: 79f18fc8 -> gui_forms main 8e288666. The build took 24 s (12 compiles, 1 hit).
  - The install exports Application, Audio, TextMasks and AudioLoopTransport; a consumer's find_package with all four components configures.
  - The older install, 79f18fc8, is kept by design.
- 2026-10-27: b66b283 (features) is on template_harness main (17f5c98).
  - New local commit 4462fd5 on `lockstep` (th_lockstep), for the coordinator to push. init and advance now place template_harness at the commit in the clone's harness.lock, using lock.py checkout, beside the clone at gui_forms_lockstep/template_harness. build and prefix refuse when it is missing or at another commit. The record gains a `harness` field, and a matching build refreshes an older record without rebuilding. 8 tests, passing 3 runs in a row.
  - Applied to D:/malkuth: init adopted the coordinator's hand-placed harness clone at 523c2a7. build reported built=already and rewrote the record, which now carries harness 523c2a7.
- 2026-10-27: prepared the house-style A/B workspaces at D:/malkuth/ab/A and D:/malkuth/ab/B. They are identical apart from HOUSE_STYLE.md: A has the most-first variant, B the least-first.
  - Contents: CMakeLists.txt, src/main.cpp (an empty window), tests/notes_tests.cpp, build.sh (lockstep record, pinned toolchain, ccache, LANG), check.sh, TASK.md (the notes app).
  - Both build green: 2/2 tests in about 10 s and 6 s. check.sh is clean on both and exits 1 on a planted finding.
  - The proof build/ trees were removed. The agents are the coordinator's to start.
- 2026-10-27: the A/B agents are running. HOLD the lockstep advance until the coordinator says both have finished, even after gui_forms #415 (FreeType/HarfBuzz export) merges. After that: advance --to main, build, verify the GUIForms::FreeType/HarfBuzz targets, report the record.
- 2026-10-27: the hold is lifted and #415 has merged. I advanced the lockstep from 8e288666 to ec23f9d8 (harness still 523c2a7); the build took 8 s with no compiles. The install exports GUIForms::FreeType and HarfBuzz (GUIFormsTextStackTargets), a consumer configures with all six optional components, and a FreeType+HarfBuzz probe links and runs. D:/malkuth/ab was left untouched.

## RESUMED by worker w2 (2026-10-28, laptop + M4)

Workspace ~/Developer/gui_forms_workers/w2/gui_forms; build/test only via
`~/Developer/gui_forms_workers/bin/m4ws w2 <cmd>` (add --gallery for gallery tests).
OWNER RULE (2026-10-28): workers do not push or open PRs. Commit on local branches, pass
locally on the M4 (touched tests plain + --sanitize, checkers), report branch/head/what ran.
The coordinator folds local branches (template_harness checkpoint tool). Never auto-merge.
Order (owner 2026-10-27, all before v0.5.0): MessageBox, link role + link column, menu letter
mnemonics, slow-click label edit, RichTextBox run model (stx_rtf2md), ListView/TabControl gaps.
DG and DataLayout wait for owner interviews: do not start.

- MessageBox DONE locally: branch claude/w-message-box (local; the origin branch of that name
  is the stale parked 7ce48f0e) at b7ca4433, on L8b's 139ebc3c.
  - 1f668ba5 = cherry-pick of 7ce48f0e (CMake conflict: kept ColorDialog/FolderBrowser and
    MessageBox; planning/ README dropped, now in notes).
  - b7ca4433: MessageBoxResult is a variant (MessageBoxClosed | MessageBoxRefused), factories
    closed()/refused(), queries shown()/result()/error() (house style section 2); accepted answer
    without a message payload -> refused(backend_failure); API docs regenerated.
  - Verified: message_box, gallery_pages, gallery_display_trace, gallery_interaction,
    doc_examples plain and --sanitize (all pass); house style, lifetime, control_schema check,
    control_registry check, generate_library_docs --check.
  - No registry entry (a function facade, no type), as the parked branch had it.
  - Notes: local branch claude/w-message-box-notes at 0a853da (W README rows MessageBox and
    ThreadExceptionDialog), in worktree ~/Developer/gui_forms_workers/w2/notes. Not pushed.
- Next: the link role and DataGridViewLinkColumn (link role #347 is already on main; check what
  is missing), on top of claude/w-message-box.
- 2026-10-28 coordinator: claude/w-message-box b7ca4433 + notes 0a853da go into checkpoint 2.
  Once checkpoint 1 (L8b) lands on main, rebase the W work onto main.
- DataGridViewLinkColumn DONE locally: branch claude/w-link-column at a082e908 (on b7ca4433).
  - DataGridViewColumnKind::link; column link_text, track_visited_state (default true);
    link_visited/set_link_visited (row position + column; kept per row id in visited_links_,
    cleared/erased with rows). Click on the cell_link part, Enter, Space and the semantic
    press activate it (content-click event); visited set before the event when tracked. Hand
    cursor over cell_link. Semantics value "visited"/"unvisited".
  - Paint: cell_link part, role content, family link or visited_link; cell system custom
    state "visited" (index 0). Gallery tree-and-object-views gains a "History" link column.
  - Gap reported (decision pending): no underline (no underline mark; link families' border
    is clear). Options sent: 1 PartMark::underline (recommended), 2 visible link border plus a
    divider part, 3 no underline. LinkBehavior not supported.
  - Verified: data_grid_view, gallery_pages, gallery_display_trace, gallery_interaction,
    doc_examples, anatomy plain and --sanitize; house style, lifetime, control_schema
    check/render --check, control_registry check, generate_library_docs --check.
    Schema rewrite: run the built gallery_pages_tests on the M4 with
    GUI_FORMS_CONTROL_SCHEMA=$PWD/manifests/control-schema.json GUI_FORMS_CONTROL_SCHEMA_WRITE=1,
    then scp the file back (laptop tree is authoritative).
  - Notes: local branch claude/w-link-column-notes at dcc028d (W README rows: link and image
    columns done).
- Next: menu letter mnemonics (plain letter in an open dropdown; notes under "PAUSED for the
  night"), on a local branch on top of a082e908 (or on main after the rebase).
- 2026-10-28 coordinator: links stay without underline until the owner decides -> owner decided
  the same day: option 1, PartMark::underline (recorded in notes W README decision 7).
- Plain-letter menu mnemonics DONE locally (claude/w-menu-mnemonics). ContextMenu::Impl::
  type_mnemonic(depth, focused, char): one enabled match activates (submenu opens + focuses
  first), several cycle focus after the focused row, none -> unhandled. MenuPanel and MenuRow
  on_text_input (typed_character: one scalar, not composing). ContextMenu docstring updated.
  Test test_menu_plain_letter_mnemonics. Not done: Win32's first-letter fallback for items
  without '&' (not asked).
- Underline DONE locally (claude/w-link-underline): PartMark::underline appended before count
  (one-unit draw_line at box bottom - 0.5, recipe.glyph). LinkLabel paints its underline part
  via paint_part_from_recipe (recipe.glyph = text colour, so a set_style override still
  applies; same stroke as before, traces unchanged). DGV: CellParts::cell_link_underline
  (glyph role, 15 cell parts), cell_link returns CellLink{text, underline}; baseline from the
  content role's line-box formula. Tests: docking_vocabulary marks(), data_grid_view
  link_cells. Schema rewritten (marks + part).
- 2026-10-28 L8b gate fix: claude/l8b-text-corpus 317a6f51 (layout_corpus_tests make_kind:
  default + text_block throws). W rebased onto it with --update-refs: message-box e2afa726,
  link-column a07405fe, menu-mnemonics 221021e4, link-underline 9a50086b.
- Gate over the W tip (--changed-from origin/main) reports 6 pre-existing F1s pulled into scope
  by changed headers: appkit_host_services.mm:434,448,595; tests macos_compositor_tests.mm:346,
  macos_exposure_tests.mm:129, macos_message_dialog_key_tests.mm:99. Asked the coordinator.
- Notes: claude/w-link-underline-notes 137fb32 (on claude/w-link-column-notes dcc028d).
- Next: the slow-click label edit (PresentationSettings::double_click_milliseconds).
- 2026-10-28 coordinator: the six macOS F1s are option 2 (the standby sweeps the macOS tree);
  leave appkit_host_services.mm and the macos_*_tests.mm files alone.
- Slow-click label edit DONE locally: claude/w-slow-click-edit fac083a3 (on 9a50086b).
  ListView: PendingLabelClick{index, model revision}, label_click_timer_ (schedule_ui_timer,
  one-shot via 24 h interval, deadline now + presentation_settings().double_click_milliseconds),
  LabelClickElapsed functor (weak_ptr). Armed on a single primary unmodified press on the
  text part of the item focused+selected before the press while the view had focus; dropped
  by any press, a double click, key down, wheel, focus loss, set_label_edit(false),
  begin_label_edit; elapsed re-checks revision/focus/selection. Test slow_click_label_editing.
  Verified: list_view, gallery_pages, gallery_interaction, doc_examples, anatomy plain and
  --sanitize; house style, lifetime, schema check, docs check; gate --changed-from 9a50086b
  shows only the standby's two macOS test F1s.
  Notes: claude/w-slow-click-notes (W README ListView row), on 137fb32.
- Next: the RichTextBox run model (reads stx_rtf2md), then the ListView/TabControl gaps.
- 2026-10-28 coordinator: macOS F1s fixed on the standby's branch (in the checkpoint).
- RichTextBox RUN MODEL (milestone 1 of 2; the control is milestone 2): branch
  claude/w-rich-text-run-model on fac083a3. Decisions taken within the brief (report them):
  - Public type gui_forms::RichTextModel in include/gui_forms/text/rich_text/rich_text.hpp
    (in gui_forms/text.hpp), sources src/core/text/rich_text/{rich_text_model,rich_text_document}.cpp
    in gui_forms_core. stx_rtf2md linked PRIVATE (no stx type public; stx_conv precedent) and
    find_dependency(stx_rtf2md CONFIG) added to cmake/GUIFormsConfig.cmake.in.
  - Storage: a TextStore; paragraphs separated by LF, line breaks U+2028 (a code block's lines
    too); sparse style spans whose TextStyleId indexes an interned RichRunStyle table (compacted
    when full). One RichParagraphFormat per paragraph (variant: body, heading, bulleted,
    numbered, code_block; quote depth; alignment except code). Superscript/subscript one
    RichBaseline enum.
  - Edits: replace(range, text, style). Joining paragraphs keeps the first's format; a typed LF
    copies the current paragraph's format. Text refuses C0 but tab/LF, DEL, NEL, U+2029, U+FEFF.
  - Queries: style_at, insertion_style, uniform_style (nullopt = mixed), attribute_state
    (off/on/mixed); setters for attributes, colour, background, size step, link.
  - Load: load_markdown / load_rtf -> RichTextLoadResult (loaded{losses} | failed{error, offset});
    failure leaves the model unchanged. Save: save_markdown(std::string&) / save_rtf(vector<byte>&)
    -> RichTextSaveResult (report | failed); the builder fits normal form (drops empty paragraphs
    and trailing line breaks, heading breaks -> space, heading size steps and code styles dropped,
    list levels lowered) and counts each in RichTextSaveReport; check_normal_form failure = defect.
  - Test: tests/rich_text_model_tests.cpp (gui_forms_rich_text_model_tests).
  - docs/reference/inventory.json (tools/api_reference.py) is stale on this branch and was already
    stale before it (main regenerates it rarely); not regenerated.
- RUN MODEL committed (cd65787d) and verified: rich_text_model, text_store, doc_examples plain
  and --sanitize; checkers, docs check; gate showed nothing in this code.
- 2026-10-28 REBASED onto origin/main 39b5b77f (checkpoint 1, #445) with --update-refs, all local:
  message-box 3a7ffd63 (2da0fb0d below), link-column 4594d718, menu-mnemonics c0298b02,
  link-underline f88954e7, slow-click-edit 28cf090f, rich-text-run-model f22a0e3d.
  Notes rebased onto origin/gui_forms 6bc80b6: message-box-notes 0f7ef66, link-column-notes
  018735d, link-underline-notes 2c9332b, slow-click-notes a2df496. Local claude/l8b-text-corpus
  and notes claude/l8b-text-record deleted (contained).
  After the rebase: rich_text_model, text_store, menu_controls, list_view, data_grid_view,
  docking_vocabulary, message_box, link_label (9 tests) plain and --sanitize pass; gate
  --changed-from origin/main: no F1/F2 (3 pre-existing advisory C2s in data_grid_view*.cpp);
  house style, lifetime, docs, registry, schema checks clean.
- Next: RichTextBox control (milestone 2: TextBox engine over RichTextModel, painting by role,
  rtf/markdown properties), then the ListView/TabControl gaps.
- 2026-10-28 coordinator: milestone 2 (RichTextBox control) on a local branch on f22a0e3d.
  Checkpoint 2 folds the W chain from its top branch: keep every branch passing on its own.
- RichTextBox CONTROL (milestone 2), branch claude/w-rich-text-box on f22a0e3d. DONE 2026-10-28,
  head 1b67c7a0; notes branch claude/w-rich-text-box-notes on a2df496, head 151bb47 (W README
  RichTextBox row partial; totals 68 done, 19 partial, 6 missing).
  Decisions taken within the brief (report them):
  - "On the TextBox engine's paths": TextBox is a final monolith, so RichTextBox is its own
    Panel control that mirrors TextBox's multiline key table, caret blink, history policy
    (128 steps / 8 MiB, snapshots of the whole model), IME replacement ranges, clipboard and
    semantic text layout; the shared word rule moved to input_control_utilities.hpp
    (previous/next_word_boundary(const TextStore&, Utf8Offset)), TextBox delegates to it.
    No TextBox engine extraction (would touch TextBox goldens); a refactor is an option later.
  - Model additions: RichStyledRun + RichTextModel::styled_runs(ScalarRange, vector&),
    text_store(), free admitted_rich_text(utf8) (CRLF/CR/U+2029 -> LF, drops refused chars).
  - Positions are UTF-8 offsets (TextSelection) as TextBox's; selected_range() gives scalars.
  - Layout: per paragraph, cells per grapheme measured by prefix within chunks (a chunk ends
    after a space, at a tab/line break, or at 64 graphemes); wrap at the last space, else at
    a grapheme; spaces hang; tabs every four spaces of the run font from the row start.
    Rows take the tallest run. Size steps scale by RTF's table (16..64 half-points / 24);
    headings bold at steps +3,+2,+1,0,0,-1; sup/sub 0.7 size, rise 0.4 / drop 0.2; code in
    FontRole::monospace; indents: quote 1.2 em, list (level+1)*1.8 em, code block 0.6 em.
  - Paint by role: editor text, link role text + underline in link glyph colour (owner
    decision), selection role; list bullet/ordinal in the indent, quote rules and code-block
    shade in editor muted_text; run colour/background as given.
  - Keys: TextBox's multiline set (Cmd+Left/Right and Cmd+Backspace to the VISUAL LINE ends,
    not the text ends), Enter = paragraph, Shift+Enter = U+2028, Ctrl+B/I/U toggle,
    Ctrl+L/E/R align. Typing style (empty-selection format changes) until the caret moves.
  - link_clicked on a press+release on the same character without a drag; hand cursor over
    links. Selection-format setters work on a read-only box (programmatic), keys do not.
  - maximum_bytes 1 MiB for set_text (throws length_error) and loads (limit_exceeded).
  - Declined (docstring Notes): OLE/embedded objects, zoom beyond size steps, RTF-only members.
  - Registry: rich_text_box, Text editing, provisional, gallery text-and-input (new group
    "gallery.text.rich" at y 1114). Test: tests/rich_text_box_tests.cpp.
  - Three gallery display traces re-recorded (text-and-input, states-and-diagnostics,
    typography-and-inspection), with a comment in display_trace_tests.cpp; schema rewritten.
  - Layout cache keys on content_revision_ (bumped in changed()), not model revision: a loaded
    model brings its own revision count, which can repeat.
  - Verified on the M4: rich_text, text_box, token_box, gallery_pages, gallery_display_trace,
    gallery_interaction, doc_examples, anatomy, semantic_text, combo_box, domain_up_down,
    numeric_up_down, text_store (17 tests) plain and --sanitize with --gallery pass; gate
    --changed-from f22a0e3d: no F1/F2 (advisory C2s only); house style, lifetime, docs,
    registry, emit-cpp, schema check/render, test_control_schema clean.
- Next: the ListView and TabControl gaps, on a local branch on 1b67c7a0 (chain top).

## Queued by the owner (2026-10-28)

- After the ListView and TabControl gaps: one shared editing engine under TextBox and RichTextBox (keys, history, input method, clipboard, semantics), replacing the copy RichTextBox took from TextBox. TextBox is final and single-piece today, so this is a refactor with both controls' tests and the gallery traces unchanged.

## ListView and TabControl gaps (2026-10-28), branch claude/w-list-view-tab-control-gaps on 1b67c7a0

- Gaps found: ListView groups, tile view, item tooltips (docstring "Not supported yet");
  TabControl images and owner draw are DONE (#343, #354; the W README row is stale); TabControl
  still lacks Selecting/Deselecting (cancelable) and Selected/Deselected events, ShowToolTips with
  TabPage tool_tip_text, and Multiline.
- Commit 1 DONE: 0c6816b0 tile view (ListViewView::tile, tile_size; empty = automatic: large icon
  + 168 units text, max(icon, 3 lines) tall; sub-item lines for columns 1.. with non-empty text,
  as many as fit, block centred; owner draw raises per sub-item line). list_view test passes plain.
- Commit 2 IN PROGRESS: groups (batch-2 plan: caption + rule, non-collapsible, non-virtual, not in
  the list view, empty groups hidden, default group for items with no or unknown group FIRST, as
  Windows Forms' DefaultGroup). Design: display order cached by model_revision_; a per-layout group
  table (first position, count, first line) with header lines of their own extent; top_item_ holds
  a display position plus top_header_; ungrouped arithmetic kept exactly (display traces).
- Item tooltips (ListView ShowItemToolTips, TabControl ShowToolTips, also TreeView) need a shared
  per-item tooltip seam: a decision for the coordinator, not built.
- Then: TabControl selecting/deselecting/selected/deselected events. Multiline: ask (overflow
  scrolling is the chosen behaviour).

- **Superseded 2026-10-28 (owner):** the shared editing engine item becomes RichTextBox on our own markdown rules. The owner relicenses SwiftEdit's markdown engine and RTF converter to MIT for GUI.Forms as a reduced inline editor framework all programs can use; SwiftEdit keeps its PolyForm copy for systemwide/CLI use. Design note to the coordinator first. Reference clone: ~/Developer/swiftedit.

## Owner decision 2026-10-28 (replaces the queued shared-editing-engine item)
- GUI.Forms rich text = our own markdown rules. SwiftEdit's markdown engine and RTF converter are
  relicensed to MIT for GUI.Forms as a reduced inline-editor framework; read-only reference clone
  ~/Developer/swiftedit (src/markdown.*, markdown_preparation.*, markdown_view.*, rtf_import.*,
  editor.*, document*). Do NOT change the swiftedit repo.
- Steps: finish the ListView groups commit cleanly; then send the coordinator a DESIGN NOTE (no
  code): which pieces move and in what reduced form, where they live (MIT headers, house style,
  public framework); RichTextBox as markdown editing plus a small control bar; RTF in and out via
  the converter and stx_rtf2md; what replaces RichTextModel (f22a0e3d, in checkpoint #446) and
  1b67c7a0; shared editing for TextBox and RichTextBox; tests. Then keep going on the gaps.

## Owner decisions on the markdown framework (2026-10-28, via the coordinator)
Design note sent 2026-10-28 (copy: scratchpad markdown_design_note.txt; content summarised here).
1. Parser Option A: vendor MD4C under third_party/md4c (MIT, intake note, pinned). stx_rtf2md's
   MarkdownReader stays the normative reader for conversion; a cross-check test over the dialect
   corpus keeps the presentation parser and MarkdownReader agreeing.
2. Markers always visible and dimmed (muted role) in version 1; caret, selection, IME and undo map
   1:1 to the source.
3. text() = plain text (markers removed); markdown() = the source.
Rest of the design stands: module gui_forms/text/markdown/ (namespace gui_forms::markdown, MIT, house
style): MarkdownDocument with source ranges (content + markers), reduced block set (paragraph,
heading, list item, quote, code, rule, links, dialect attribute spans/blocks; tables and task lists
as source), preparation off-thread only above 64 KiB, MarkdownPresentation (from markdown_view's
layout), rtf_conversion.hpp (import_rtf from SwiftEdit with loss phrases; export_rtf via
MarkdownReader -> Document -> RtfWriter). RichTextBox = markdown source editing + control bar
(Bold Ctrl+B, Italic Ctrl+I, Heading drop-down body/1/2/3, bullets, numbers, Link Ctrl+K, inline
code), link_clicked on Ctrl/Cmd+click. RichTextModel (main, #446) removed in step (iii); local
1b67c7a0 dropped (keep the word-boundary rule, gallery group, docstring notes).
Order, one local branch each, each passing alone, report at each milestone:
 (i) gui_forms::markdown with rtf_conversion and tests (round trips, loss kinds, source-range
     coverage, cross-check, fuzz entry);
 (ii) TextEditingEngine extracted under TextBox, TextBox tests and ALL gallery traces byte-identical;
 (iii) RichTextBox over engine + presentation with the control bar, removing RichTextModel.
Do not change ~/Developer/swiftedit.
Sequence now: finish TabControl events commit (clean, local) -> start (i).

## Rebase after checkpoint 2 (2026-10-28)
- Main f8326690 (#446). Deleted contained local branches: w-link-column, w-link-underline,
  w-menu-mnemonics, w-message-box, w-rich-text-run-model, w-slow-click-edit; notes: link-column,
  link-underline, message-box, slow-click notes.
- claude/w-rich-text-box rebased onto origin/main: 7283337f (to be dropped by step (iii)).
- claude/w-list-view-tab-control-gaps rebased DIRECTLY onto origin/main (no RichTextBox dependency):
  9cfdcb40 tile view, 21289b75 groups. Verified on main: list_view, gallery_pages, display_trace,
  interaction, anatomy, doc_examples, schema_export, part_painting plain + --sanitize pass; gate
  --changed-from origin/main no F1/F2; checkers clean.
- Notes: claude/w-rich-text-box-notes rebased onto origin/gui_forms e1af0f7 (row to be rewritten
  for markdown); claude/w-list-view-gaps-notes cfc6e1f (ListView row: groups + tile; TabControl row
  corrected: images/owner draw done; events, tooltips, multiline open).
- TabControl events design: Deselecting(old) then Selecting(new), both cancellable, before any
  change (user, keys, strip, programmatic); then Deselected, Selected, selected_index_changed.
  Not raised for add_page's first selection or remove_page's fallback.

## ListView/TabControl gaps milestone DONE (2026-10-28)
- Branch claude/w-list-view-tab-control-gaps on origin/main f8326690: 9cfdcb40 tile view,
  21289b75 groups, 79686409 TabControl deselecting/selecting/deselected/selected (cancellable
  first, then after-events, then selected_index_changed; not for add_page's first page or the
  removal fallback). Schema rewritten (interactions). Verified: tab_control, page_strip, page_set,
  gallery_pages, display_trace, interaction, anatomy, doc_examples, schema_export, list_view
  (14 tests) plain + --sanitize; gate --changed-from origin/main no F1/F2; checkers clean.
- Notes claude/w-list-view-gaps-notes 0c97c7b (ListView + TabControl rows).
- claude/w-rich-text-box 7283337f on main: plain 17-test set passes (sanitizer not rerun after the
  rebase; branch to be dropped in step (iii)).
- Open questions sent: per-item tooltip seam (ListView ShowItemToolTips, TabControl ShowToolTips,
  TreeView), TabControl Multiline (overflow scrolling is current behaviour).
- NEXT: step (i) gui_forms::markdown + rtf_conversion on a new branch from origin/main
  (claude/w-markdown-framework).

## Owner decisions on tooltips and Multiline (2026-10-28, via the coordinator)
1. One per-item tooltip seam: a Control virtual giving the tooltip text at a point, queried by
   ToolTip. ListView (ShowItemToolTips), TabControl (ShowToolTips) and TreeView (ShowNodeToolTips)
   adopt it. Minimal public shape (new public API on Control): SEND THE SIGNATURE to the
   coordinator before building it out.
2. TabControl Multiline wrapping DEFERRED by the owner; record it as such in the W README
   (notes repo, TabControl row).
- ORDER: markdown step (i) first, then the tooltip seam, then steps (ii) and (iii). Local
  branches only.

## Step (i) started (2026-10-28): claude/w-markdown-framework from origin/main f8326690
- MD4C v0.6.0 7fc1815a vendored from SwiftEdit's copy. Blob SHA-1 verified equal to upstream
  tree (gh api): LICENSE.md 3baef6a3, src/md4c.c e9f87ba8, src/md4c.h 796fd677,
  src/entity.c 1b7ab64e, src/entity.h 2ee99c8f.
- Step (i) content (claude/w-markdown-framework): third_party/md4c (PROVENANCE.md, LICENSE.md),
  third_party/MD4C_INTAKE_2026-10-28.md, README paragraph, THIRD_PARTY_NOTICES row,
  packaging/licenses.json MD4C.txt, SDK install MD4C-LICENSE.txt. CMake: OBJECT lib gui_forms_md4c
  into gui_forms_core via $<TARGET_OBJECTS>, md4c include dir only on markdown.cpp (no export change).
- Public API (namespace gui_forms, Markdown-prefixed names, NOT gui_forms::markdown; precedent
  for flat names: RichTextModel; report to coordinator):
  include/gui_forms/text/markdown/markdown.hpp: MarkdownDocument{blocks,spans,links},
  MarkdownBlock{kind paragraph|heading|list_item|code_block|rule, level, quote_depth, marker,
  ordinal (as written), alignment, source (lines), first_span, span_count}, MarkdownSpan{kind
  text|line_break, text, source Utf8Range, style, link index}, MarkdownLimits, MarkdownParseResult
  (parsed | failed{error invalid_utf8|limit_exceeded|cancelled, offset}), parse_markdown(source,
  document, limits, stop_token), markdown_plain_text.
  rtf_conversion.hpp: MarkdownLoss (25 kinds = stx LossKind), MarkdownLossReport, describe_loss,
  RtfBodySize/RtfBodyRule, RtfConversionError, RtfConversionResult (imported|exported|failed),
  looks_like_rtf, import_rtf(span<const byte>, string&), export_rtf(string_view, vector<byte>&).
- Parser decisions: flags STRIKETHROUGH|NOHTML only. Markup = source bytes no span covers. Soft/
  hard breaks and code-block line feeds map to the line ending; MD4C static strings map to an
  empty range at the cursor. First paragraph of an LI becomes the list_item; later ones are
  paragraphs. Ordinal read from the marker before the first content byte (fallback: counter).
  Code block's trailing line feed dropped. Block source = lines of first..last content byte
  (fences and setext underline excluded; content-less blocks empty at previous block end).
- Tests: tests/markdown_tests.cpp (parse, source ranges, cross-check vs MarkdownReader over the
  4 corpus .md, RTF import == corpus .md, md->rtf->md identity, losses, failures, doc examples),
  fuzz/markdown_fuzz.cpp (standalone 50k mutations). Docstrings MarkdownDocument.rst and
  RtfConversionResult.rst (examples asserted in markdown_tests).

## Step (i) milestone DONE (2026-10-28)
- claude/w-markdown-framework 1b10e2e1 on origin/main f8326690 (local only, not pushed).
- Verified on the M4: gui_forms_markdown_tests, gui_forms_markdown_fuzz_tests,
  gui_forms_rich_text_model_tests, gui_forms_doc_examples_tests, stx_rtf2md_* (6) = 10 tests,
  plain + --sanitize pass; gate --changed-from origin/main no F1/F2; house style, lifetime,
  generate_library_docs --check, control_registry check, control_schema check clean.
- Notes: claude/w-list-view-gaps-notes 3f2a8a3 adds "Multiline deferred by the owner".
- Pre-existing gap noticed, not changed: stx_rtf2md (in gui_forms_core) has no
  THIRD_PARTY_NOTICES row (in-repo MIT, like stx_conv which has one).
- NEXT: tooltip seam. Signature proposal sent to the coordinator; wait for its answer before
  building it out. Then step (ii).

## Coordinator, 2026-10-28: step (ii) now; tooltip seam waits on the owner
- Signature tool_tip_text_at(Point) const -> optional<string> accepted; which ToolTip asks is
  with the owner (options: any mapping ToolTip, set_tool_tip(target, "") registers without
  fallback [recommended]; or a window-owned implicit ToolTip). Build it when the answer comes.
- stx_rtf2md THIRD_PARTY_NOTICES row added: claude/w-markdown-framework 3e1ffabe.
- Step (ii) branch claude/w-text-editing-engine from origin/main f8326690 (uncommitted while
  building). Design: src/controls/panel/text_editing/text_editing_engine.{hpp,cpp}
  detail::TextEditingEngine (store, selection, history 128/8 MiB, filter, maximum_length,
  read_only, multiline flag, composing, clipboard, editing keys, text input, word rule) and
  detail::TextEditingHost (require_mutable, restoring, text_changed, select, navigating,
  user_edit, host_services, password_protected). TextBox::EditingHost (defined in text_box.cpp,
  not a header: the docs parser misreads `class TextBox::EditingHost` in a header as a second
  TextBox) forwards to the box. ~TextBox moved to text_box.cpp. Layout, paint, multiline line
  keys, Tab/Enter/Escape, auto-complete and blink stay in TextBox. manual.json Snapshot review
  repointed to the engine header.

## Step (ii) milestone DONE (2026-10-28)
- claude/w-text-editing-engine 2ecbfc3d on origin/main f8326690 (local only).
- M4: 39 TextBox-related tests (regex in w2/dev/engine_tests_regex.txt) plain, 7 Gallery tests
  (--gallery: pages, display_trace, interaction, policies, AX client gallery) plain, all 47
  --sanitize: pass (traces byte-identical). Gate no F1/F2 (C2 at text_box.cpp visual_status
  chain pre-exists on main). House style, lifetime, docs --check, registry, schema clean.

## Owner decision 2026-10-28: tooltips belong to the thing (replaces tool_tip_text_at)
- Control and each item have plain tooltip text (empty never shows); one presenter per window;
  window and program switches; ToolTip stays as the WinForms way to set text/timing;
  RibbonToolTip reads plain text from the thing; text goes through translation later.
- Design sent to the coordinator (2026-10-28): Control::tool_tip_text, ListViewItem/
  TreeViewItem::tool_tip_text fields, SemanticNode/SemanticDescriptor::tool_tip, presenter finds
  items via realized_item_count/fill_realized_item_geometry/realized_item_stable_id then the
  semantic_virtual_children node; Window::set_tool_tips_shown(optional<bool>),
  Application::set_tool_tips_shown(bool), Window ToolTipTiming; ToolTip forwards; RibbonToolTip
  falls back to tool_tip_text and suppresses the plain presenter for what it covers.
  Open: A TabPage tip header-only (rec), B keyboard-focus tips on (rec), C keep Show*ToolTips
  switches default false (rec). WAIT for confirmation before building.
- NEXT: step (iii) RichTextBox over the engine + presentation + control bar, on a new local
  branch from claude/w-text-editing-engine (needs step (i) too: base on both or wait for
  checkpoint); drop claude/w-rich-text-box 7283337f when done.

## Tooltip design SETTLED by the owner (2026-10-28), build AFTER step (iii) on its own local branch
- Tooltips belong to controls and stable authored parts (tab headers, toolbar/strip items) only,
  never to data items: NO tool_tip_text on ListViewItem/TreeViewItem; ListViewItem.ToolTipText,
  ShowItemToolTips, TreeNode.ToolTipText, ShowNodeToolTips NOT provided; TabControl ShowToolTips
  not needed either (a page's empty tip never shows) -- treat it like the item switches.
- A TabPage's tooltip (Control::tool_tip_text) shows over its tab header.
- Cut-off data item text (ListView details/list rows, TreeView nodes) is revealed in place on hover:
  full text expanded over the row at the item's own position, only when actually truncated,
  nothing for the program to set. Not a tooltip: no presenter content, no tooltip switches (may
  share hover timing).
- docs/user/for-windows-forms-developers.md row: GUI.Forms does not provide those members; data
  items have no tooltips; cut-off text is revealed by the list. Do NOT mention the owner's
  conversion program.
- One presenter per window; window and program switches; keyboard focus shows a bubble below the
  thing, EXCEPT typed-text controls (TextBox, ComboBox edit, NumericUpDown, MaskedTextBox,
  RichTextBox, similar): their tooltip shows as light-grey hint text inside the EMPTY field, like
  placeholder text; a set placeholder wins.
- ToolTip component forwards (WinForms-compatible way to set text/timing); RibbonToolTip reads
  plain text from the thing's tooltip.

## Step (iii) in progress (2026-10-28): claude/w-rich-text-markdown
- Stack after checkpoint 3 (origin/main 8cde98df): (i) claude/w-markdown-framework c0e1b79c,
  (ii) claude/w-text-editing-engine ae42ffd5, (iii) claude/w-rich-text-markdown (WIP b29b2e5d +
  uncommitted). claude/w-list-view-tab-control-gaps deleted.
- (iii) content: RichTextModel removed (header, document/model cpp, tests, CMake); RichTextBox
  (include/.../rich_text_box/rich_text_box.hpp; src rich_text_box{,_layout,_bar,_format}.cpp)
  over detail::TextEditingEngine. The engine gains maximum_bytes (in (iii), not (ii)); RTB's
  engine stays non-multiline to avoid the 4096-byte line bound, maximum_bytes = 1 MiB.
  tests/rich_text_box_tests.cpp; docstring RichTextBox.rst; gallery group gallery.text.rich at
  {24,1114,930,270} on text-and-input; registry/catalog/README/generated docs; porting guide
  drops RichTextBox from "Not yet available".
- Parsing is synchronous everywhere (the "off-thread above 64 KiB" idea deferred; tell coordinator).
- (iii) COMMITTED: claude/w-rich-text-markdown 4530dada on (ii) ae42ffd5. claude/w-rich-text-box
  7283337f deleted. M4: 50 tests (regex w2/dev/rtb_regex.txt: the TextBox set + gallery +
  rich_text_box + markdown + markdown_fuzz) plain and --sanitize pass; gate no F1/F2 (one F1 in
  invoke_bar_button fixed with a leaving fallback; my C2s given a final else); house style,
  lifetime, docs --check + test_generate_library_docs, registry (+ emit-cpp --check), schema
  (+ render --check) clean. Schema regenerated with GUI_FORMS_CONTROL_SCHEMA_WRITE=1 on the M4
  and copied back (m4ws's rsync overwrites M4-side writes; copy back in the same command).
  Digests: states-and-diagnostics 0x9714ea3dd4f5beed (1300 controls), text-and-input
  0xe6fb680f7424950e, typography-and-inspection 0x6a952bfe1afec2c7 (#1333).
- Not visually inspected (no page-selecting capture in the gallery program).
- Porting guide "Not yet available" also lists DataGridView, MaskedTextBox (and maybe others)
  that now exist: stale, untouched; tell the coordinator.
- Stack re-verified alone on origin/main 8cde98df (w2/dev/verify_stack.sh): (i) c0e1b79c 10 tests
  (markdown, markdown_fuzz, rich_text_model, doc_examples, stx_rtf2md x6) plain + sanitize, gate
  clean; (ii) ae42ffd5 50 tests (w2/dev/stack_ii_regex.txt) plain + sanitize, gate clean.

## Step (iii) milestone DONE (2026-10-28); reported to the coordinator
- Stack: (i) claude/w-markdown-framework c0e1b79c -> (ii) claude/w-text-editing-engine ae42ffd5
  -> (iii) claude/w-rich-text-markdown 4530dada, on origin/main 8cde98df. Local only.
- NEXT: tooltip work on its own local branch claude/w-tooltips from (iii)'s head or from
  origin/main (decide by overlap: the hint-text path touches TextBox and RichTextBox), per the
  settled owner design above.

## Coordinator follow-ups on (iii) (2026-10-29), all on claude/w-rich-text-markdown
- 1883393d tests: multi-paragraph document (Enter between lines, heading/paragraph/list, the
  paragraph wraps to >= 8 visual lines, Up walks rows, 30 pasted paragraphs keep line feeds,
  the caret scrolls into view on paint (reveal happens in paint, as TextBox), Ctrl+Home back,
  paste = one undo step). The engine's multiline flag only selects TextBox's 4096-byte line
  validation; it does not filter line feeds.
- b7c30b80 porting guide: the whole stale "Not yet available" type list removed (all nine exist).
- ecd5d446 selected rows draw each glyph once (three clips). TextBox still double-draws
  selected text (text_box.cpp ~687, ~823): heavier selected glyphs; not changed, reported.
- Screenshots: w2/dev/rtb_bold.png, w2/dev/rtb_mixed.png, made by a throwaway macOS program
  (w2/dev/capture/: main + CMake snippet inserted after the seam lab target, built with a
  --gallery configure, run with --capture over ssh; not committed).
- Verified: rich_text_box, gallery_pages, display_trace plain; + doc_examples under sanitize;
  gate clean.

## PAUSED (2026-10-29, owner direction) -- RESUME POINTER
- Resume from the notes plan: planning/stages/W-winforms-controls/RESUME_PLAN.md on notes branch
  claude/w-resume-plan 98b33d7 (w2 notes worktree, from origin/gui_forms f05639a).
- Markdown stack (local, w2 workspace, on origin/main 8cde98df), folding into checkpoint 5:
  (i) claude/w-markdown-framework c0e1b79c -> (ii) claude/w-text-editing-engine ae42ffd5 ->
  (iii) claude/w-rich-text-markdown bcfc7ed4.
- Follow-ups after 4530dada: 1883393d multi-paragraph test; b7c30b80 porting guide stale list;
  ecd5d446 RTB selected glyphs once; 6b71b2ef TextBox selected glyphs once (+ tests/support/
  clip_tracking_painter.hpp); bcfc7ed4 bar mixed state (RichTextFormatState; indeterminate item
  state, SemanticState::mixed, checked face at half strength; Heading "Mixed").
- Verified on the M4 at bcfc7ed4: 51 tests (w2/dev/rtb_regex2.txt) plain + --sanitize, gate clean,
  checkers clean; schema regenerated (bar_button gains indeterminate).
- List markers are dimmed (pixel check: "- " darkest (130,139,149), "##" (118,128,138), text ~(30,42,53)).
- Screenshots (mixed state shown): w2/dev/rtb_bold.png, w2/dev/rtb_mixed.png.
- No tooltip branch exists; the tooltip build was not started. Next: the resume steps in the plan.

## RESUMED (2026-10-31): tooltip build, claude/w-tooltips from origin/main 2fe87fd9
- Checkpoint 5 (#449) contains the markdown stack as 9e301b63 (checked: every stack file equals
  main except later dash-rule renames from #454 and main's own edits). Deleted local
  claude/w-markdown-framework, claude/w-text-editing-engine, claude/w-rich-text-markdown.
- #454: RichTextBox bar semantic IDs are bulleted-list / numbered-list. SK2 (#450): rasters are
  premultiplied BGRA8; read pixels as BGRA in new pixel tests.
- Following RESUME_PLAN.md steps 3..10 in order.

### Milestone 1 (steps 3-6) state, 2026-10-31 (uncommitted until the plain + sanitizer run passes)
- Core: Control::tool_tip_text (bindable, UTF-8 checked, event tool_tip_text_changed, declared
  interaction), ToolTipPresentation {bubble, field_hint, owner_item}, publishes_item_tool_tips;
  SemanticNode::tool_tip (17th semantic field, schema regenerated); Application and Window
  switches; Window ToolTipSettings; one detail::ToolTipPresenter per window in
  src/core/window/tool_tips/ (state Idle/Waiting/Settled/Showing; resolve on region entry;
  keyboard only with the focus cue; press settles until the pointer leaves).
- ToolTip is a forwarder (set_tool_tip writes the text; settings are the window's);
  ErrorProvider sets its glyph's text. ToolStripItemSpec::tool_tip_text; TabPage text shows over
  its header. RibbonToolTip covers what it maps and takes the plain text as an empty title; its
  layer is now RibbonToolTipLayer (src/controls/ribbon_tool_tip/ribbon_tool_tip_layer/) so the
  docs' ToolTipLayer resolves to the core one. manual.json ToolTipBubble entry rewritten.
- Tests: tooltip_tests (12), ribbon_tool_tip_tests (+plain_text_and_covering), overlay_shadow
  (the bubble paints its lines itself). Docs regenerated; docs/registry/schema checks clean.
- Next: commit milestone 1, then step 7 (hint text), 8 (cut-off reveal), 9 (porting row), 10.
- COMMITTED 1586c8c3 on claude/w-tooltips: steps 3-7 (field hints included: Control::field_hint_text
  walks self then enclosing field_hint controls; TextBox/RichTextBox paint it when focused and
  empty without a placeholder; NumericUpDown/DomainUpDown/MaskedTextBox and an editable ComboBox
  are field_hint). Touched tests (23) plain and --sanitize passed on the M4 before the hint test.
- Step 8 in progress (uncommitted): detail::CutOffTextRevealer in
  src/controls/panel/cut_off_text/ (popup layer, input-transparent face, themed or plain paint,
  not exposed to semantics); ListView (details any column, list view; not owner-draw) and
  TreeView reveal on pointer move, conceal on leave/press/scroll/selection/items. Shown at once,
  no delay (owner decision offered). tests/cut_off_text_tests.cpp (new target).
- Step 9 done in the working tree: two rows in docs/user/for-windows-forms-developers.md.
- Docstrings: Control/Window/ListView/TreeView/text controls updated; regenerate docs after step 8.

### TOOLTIP BUILD DONE (2026-10-31): claude/w-tooltips head 2fc0ef95 (on origin/main 2fe87fd9)
- 1586c8c3 steps 3-7; 2fc0ef95 steps 8-10 (cut-off reveal, porting rows, docstrings, docs regen).
- M4: 34 touched tests plain and --sanitize pass; state-machine gate --changed-from origin/main
  passes (no F1/F2); house style, lifetime rules, docs --check, control_registry check/emit-cpp
  --check, control_schema check/render --check all clean. control-schema.json regenerated.
- Mockup for the two open choices: ~/Developer/gui_forms_workers/mockups/cut-off-reveal-and-field-hints.html
- Open owner decisions (reported): reveal timing (built: at once); hint on any focus (built) vs
  keyboard focus only; plus the design calls listed in the report.
- Not done: editable ComboBox placeholder is not forwarded to its edit field (pre-existing);
  RESUME_PLAN "Other remaining W work" rows untouched.
- Next: wait for the coordinator's decisions/checkpoint; rebase when told.

## RESUME POINTER (2026-10-31, owner stop at checkpoint 7, gui_forms #456)
Workspace ~/Developer/gui_forms_workers/w2. All branches are local and the working trees are clean.
- gui_forms `claude/w-tooltips` 2fc0ef95, VERIFIED: steps 3-10 of the resume plan. The owner accepted every
  recommendation: reveal at once, hint on any focus, and 3(a)-(i) as built. Goes into the next checkpoint.
- gui_forms `claude/w-combo-placeholder` 6198108f (on w-tooltips), VERIFIED: the editable ComboBox passes its
  placeholder to its edit field. On the M4: 8 tests (tooltip, combo_box_filter, input_controls, basic_controls,
  gallery_pages, gallery_interaction, doc_examples, collection_controls) pass plain and with --sanitize; the gate
  has no F1/F2; house style, lifetime, docs --check and schema check are clean.
- gui_forms `claude/w-dock-documents` 78b09623 (on combo-placeholder), PARTIAL, NOT VERIFIED (never compiled):
  DK phase D6 document closing on DockWorkspace (request_close, close_all_documents, close_other_documents,
  answer_close, the document_closing event with cancel and defer, DockClosePrompt each|combined,
  DockCloseOutcome). The floating-window close goes through a request; DockSite's close command calls
  request_close. My decision (not yet reported): a cancel at any prompt closes nothing, so the whole request is
  one transaction at the end.
- notes `claude/w-tooltip-decisions` 7cf71df in w2/notes_w (from origin/gui_forms 88e9ee2): 4f2aba5 records
  the tooltip build and the owner's decisions in the W RESUME_PLAN; 7cf71df recounts the W coverage
  (79 done / 12 partial / 2 missing).
- Next steps, in order:
  1. Build w-dock-documents on the M4 and fix compile errors.
  2. tests/dock_site_tests.cpp and the workspace tests: each and combined prompts; cancel leaves the layout
     unchanged; defer then answer_close(true/false); busy; close-all and close-others order; a floating window
     with a cancelled prompt stays; activation after a close follows MRU.
  3. Add Close All / Close Others to the document position menu, or report that as a choice.
  4. Docstrings (DockWorkspace), regenerate docs, regenerate the schema (DockWorkspace interactions gained
     document_closing).
  5. Run the touched tests plain and with --sanitize, and the gate; report the cancel-closes-nothing decision
     to the coordinator.
  6. Then D7 and D8 per notes planning/stages/DK-docking/PLAN.md.

## D6 DONE (2026-10-31): claude/w-dock-documents 7eb0728c (on w-combo-placeholder 6198108f)
- Amended the partial 78b09623 into 7eb0728c: D6 API (request_close, close_all_documents,
  close_other_documents, answer_close, document_closing, close_prompt each|combined, close_pending,
  pending_close, DockCloseOutcome). Documents closed meanwhile are not asked about
  (drop_unplaced_close_documents). close_floating_site returns the outcome; with no main window a
  cancelled/pending prompt cancels the host close. Tests in dock_site_tests (4 new) and
  dock_floating_tests (1 new). Docstring, docs/api, control-schema.json + schema.md regenerated.
- VERIFIED on the M4: dock|gallery_pages|doc_examples (9 tests) plain + --sanitize; gate no F1/F2;
  style, lifetime, docs --check, registry check, schema check/render --check clean.
- Reported to the coordinator as options: (A) cancel-closes-nothing transaction (built);
  (B) Close All / Close Others in the document position menu (not built).
- Next: the coordinator's answers; then the W resume plan: CommonDialog and FileDialog (coverage
  table "missing"), then D7/D8 per DK PLAN.md.
- Tools (w2/dev/tools/): verify.py [--gallery] [--only build,san,gate,style,docs,schema] [--tests R]
  runs each check via m4run, one line each; default regex in tests_regex.txt.
- Generated files (docs/api, schema JSON/md) are written on the M4 and must be scp'd back from
  m4mini:Developer/Projects/gui_forms_workers/w2/gui_forms before the next sync. Schema JSON:
  run ../.build/native-macos-arm64/gui-forms/gui_forms_gallery_pages_tests with
  GUI_FORMS_CONTROL_SCHEMA=$PWD/manifests/control-schema.json GUI_FORMS_CONTROL_SCHEMA_WRITE=1
  (needs a --gallery build), then control_schema.py render.

## Owner decisions on D6 (2026-10-31, via the coordinator)
- (1) Kept: a cancel at any prompt, or answer_close(false), closes nothing; one transaction after the last prompt.
- (2) Add Close All Documents / Close Other Documents to the document position menu (two new
  DockPositionCommand values, schema regenerated), as a commit on claude/w-dock-documents.
- TODO: record both in notes (w2/notes_w, claude/w-tooltip-decisions, DK README decisions / W RESUME_PLAN).

## State 2026-10-31 23:35 (BLOCKED on the M4 lock)
- claude/w-dock-documents: menu commit IN THE WORKING TREE, uncommitted and not built yet
  (DockPositionCommand close_all_documents/close_other_documents; DockSite menu rows for documents;
  DockWorkspace::closable_documents now public; DockSite/DockWorkspace docstrings; test
  document_menu_close_commands in dock_site_tests). Still to do: build, verify (dev/tools/verify.py
  --gallery, regex dock|gallery_pages|doc_examples), regenerate docs/api and the schema (scp back), commit.
- claude/w-file-dialog-members (on 7eb0728c) 3a WIP commit "File dialogs: filter_index": filter_index on
  Open/SaveFileDialog (0-based), HostOpen/SaveFileDialogRequest::initial_filter,
  HostPathDialogResult::chosen_filter (validated in HostServices), Windows nFilterIndex, Linux combo,
  macOS accessory NSPopUpButton (GuiFormsFileFilterSwitch); save adds the chosen filter's extension.
  Plain build: open/save/host_protocol/doc_examples pass. macos_host_close_tests fails only on its
  scheduled-wake check because the M4 screen is locked (its 5 dialog requests completed). Rebase onto
  w-dock-documents after the menu commit; docs regen; sanitizer; then report and update README rows
  (CommonDialog/FileDialog: by design, members on the concrete dialogs; Win32-only rows excluded).
- BLOCKER: my stopped run (M4WS_WAIT_IDLE=30, macos_host_close) left remote zsh pid 3911 on m4mini
  holding .m4ws.lock while it waits for the screen to unlock; it blocks w1 and me. Killing it was
  refused by the permission system; asked the coordinator.
- Lesson (corrected by the coordinator): activated GUI tests (macos_host_close) DO need
  M4WS_WAIT_IDLE, which makes m4ws wait for an idle, unlocked screen. The fault was m4ws taking the
  lock before those waits; the coordinator changed m4ws so the waits come before the lock. The rule:
  never stop an m4ws run locally while it waits on the M4 (the remote side keeps going); let it finish
  or ask the coordinator. pid 3911 is with the owner (kill it or unlock the screen).

## MILESTONE 2026-11-01: stack rebased on origin/main 6856347b (checkpoint 7, #457), VERIFIED at the top
- claude/w-tooltips 98230ee7 (64b43413, d7b25dc1, + 98230ee7 display-trace digests for 4 pages: diffed
  command by command against main's traces; all from the tooltip build)
- claude/w-combo-placeholder 5fdfcf7d; claude/w-dock-documents 8f3f2e2f (c1c4dd8f D6, 8f3f2e2f menu
  Close All / Close Other Documents, owner decision 2)
- claude/w-file-dialog-members (top) with the filter_index commit (see git log).
- M4: 33 tests plain + --sanitize (regex in dev/tools/tests_regex.txt), gate, style, docs, schema clean.
- notes claude/w-tooltip-decisions: f00e372 DK decisions 56/57; e5c1d88 W coverage (CommonDialog,
  FileDialog done by design; 81/12/0).
- Next: D7 (persistence integration, named layouts) per DK PLAN.md; open FileDialog settings
  (ShowHiddenFiles, DereferenceLinks, Check*/ValidateNames) only if asked.
- Trace digests: GUI_FORMS_GALLERY_TRACE_PRINT=1 ../.build/.../gui_forms_gallery_pages_tests --display-trace;
  dump with GUI_FORMS_GALLERY_TRACE_DIR under ../.build/traces (outside the synced tree).

## D7 persistence DONE (2026-11-01): claude/w-dock-persistence (on ec1de12a, folded into checkpoint 8)
- DockWorkspace save_layout/load_layout (live windows), named layouts (DockNamedLayout; save/apply/
  remove/find/set). Tests in dock_site_tests (4) and dock_floating_tests (1). Verified: 9 tests plain +
  --sanitize, gate, style, docs, schema.
- Open (to the coordinator as options): toolbar dragging on the docking drag session (DK decision 36,
  K2-1). ToolStripContainer/ToolStripPanel have no strip dragging at all; DockSite's drag session is
  private to DockSite.
- Next after the answer: the toolbar part, then D8 (Gallery docking page, accessibility, look).

## MILESTONE 2026-11-01: D8 Gallery page + toolbar dragging (K2-1 option 1)
- claude/w-dock-gallery eaacec9f (on bf1ddf75): Gallery page "docking-workspace"
  (src/controls/gallery/pages/docking-workspace.cpp, tests/gallery_pages/docking-workspace_tests.cpp);
  the group is 930 wide like the other pages (a 1180-wide group put its controls off the test window);
  digest 0x67168c2a206614e4 in display_trace_tests (array now 31). Registry: DockSite/DockWorkspace
  gallery_page = docking-workspace.
- claude/w-toolstrip-drag 87831263 (on eaacec9f): owner chose option 1. Shared mechanics in
  src/controls/drag_gesture (threshold, Ctrl suppression, Escape, stamp shrink/settle curve, grab);
  DockSite uses them (no behaviour change; dock tests pass). ToolStripContainer: grip drag, DragLayer,
  targets = shown panels' bands (empty band 12 units) x rows, strip_moved event, move_strip,
  cancel_drag, drag queries, parts drag_stamp/drag_stamp_image/drop_marker; ToolStripPanel::insert.
  Window releases capture BEFORE dispatching a primary up: the capture-release check is deferred a turn
  (CaptureCheck), as in DockSite. set_child_index counts from the last child (WinForms order).
- Verified both: plain + --sanitize (tool_strip|dock|gallery_pages|doc_examples, 11 tests), gate,
  style, docs, schema. notes cd3e60e: DK decision 58.
- Still open in D8: the accessibility check of a docking fixture (stage A) and the look (T4/T2).
  Toolbar follow-ups not built: several strips per row, a keyboard strip move, window-deactivate cancel.
- Rebase onto origin/main when the coordinator says checkpoint 8 (#458) has landed.

## MILESTONE 2026-11-01 (later): D8 accessibility + look
- claude/w-dock-look 9dd9561a (on 87831263): Gallery --page <id> option (macOS/Linux/Windows mains,
  demo/program_options.hpp); docking page opens on first tabs, shorter panel copy; ALL 31 display-trace
  digests recorded again (the new nav row moves every sidebar; containers-and-focus lost its DockSite/
  DockWorkspace header lines). eaacec9f and 87831263 alone FAIL gui_forms_gallery_display_trace_tests
  (my verify regex 'gallery_pages' missed it; use 'gallery' from now on).
- Accessibility: macos_accessibility (3 tests) pass on the M4; Gallery AX client 31 pages, 3183 nodes,
  0 mismatches. Windows/Linux wait for hosts.
- Look: reference image + findings in notes efcf97e results/2026-11-01-dk-d8-gallery (narrow tool stack
  clips its selected tab at both ends and its caption without an ellipsis).
- Capture recipe: build target gui_forms_gallery; app ../.build/native-macos-arm64/gui-forms/
  "GUI.Forms Gallery.app/Contents/MacOS/GUI.Forms Gallery" --page ID --capture PATH (M4WS_WAIT_IDLE=30).
- Next: toolbar follow-ups (several strips per row, keyboard move) on their own branch; rebase on #458.

## REBASED 2026-11-01 onto origin/main e1262237 (checkpoint 8, #458)
- claude/w-dock-persistence 00253637, claude/w-dock-gallery 9ae7465c, claude/w-toolstrip-drag 074dba87,
  claude/w-dock-look ecaf2ee5 (top). Deleted contained: w-tooltips, w-combo-placeholder,
  w-dock-documents, w-file-dialog-members (their files identical on main; checkpoint re-created commits).
- verify.py --gallery-wide (brief rule "Gallery-wide tests"): adds gallery|doc_examples|binding, builds
  the gallery, M4WS_WAIT_IDLE=30 for build/san (AX Gallery client). Ran at ecaf2ee5 with
  --tests 'tool_strip|dock': 19 tests plain + san, gate, style, docs, schema clean.
- Look finding (narrow tool stack tab cut without ellipsis) is routed to T4 by the coordinator.
- Next: toolbar follow-ups on a new branch from ecaf2ee5: several strips per row, keyboard strip move.

## 2026-11-01: toolbar follow-up 1 done: shared rows
- claude/w-toolstrip-rows e054da20 (on ecaf2ee5): ToolStrip::content_length; ToolStripPanel row model
  (continuing_ RuntimeIds; join_row, rows, row_count; insert = new row; remove fixes row starts);
  row layout (each but last = content_length, last = rest); container targets: middle half of a row
  joins it, outer quarters new row; move_strip_into_row; ToolStripMove from/to_position.
  verify --gallery-wide --tests tool_strip: 13 tests plain + san, gate, style, docs, schema clean.
- Open, sent to the coordinator as options: the keyboard strip move chord/mode.

## MILESTONE 2026-11-01: toolbar follow-up 2 done: keyboard strip move (DK-59, owner option 1)
- claude/w-toolstrip-keyboard 50bb4e96 (on e054da20): move_panel chord with focus in a strip ->
  begin_keyboard_move; key stops 2r = new row at r, 2r+1 = join row r (+ position); arrows across/along
  by edge orientation, Tab/Shift+Tab panels, Enter drops, Escape cancels; pointer ignored meanwhile.
  Focus return: restore set_focusable BEFORE request_focus (unfocusable container let go of the focus);
  FocusReturn via begin_invoke only when the immediate request is refused; cancelled on dispose.
  Test keyboard_move() in tests/tool_strip_container_tests.cpp. Docstring + generated API doc updated.
  verify --gallery-wide --tests tool_strip: 13 tests plain + san, gate, style, docs, schema clean.
- Notes: notes_w claude/w-tooltip-decisions 4a3c3f3 records DK decision 59.
- Stack (top first): w-toolstrip-keyboard 50bb4e96, w-toolstrip-rows e054da20, w-dock-look ecaf2ee5,
  w-toolstrip-drag 074dba87, w-dock-gallery 9ae7465c, w-dock-persistence 00253637.
- Next: wait for the coordinator (toolbar follow-ups complete; rafting outside a container stays unsupported).

## 2026-11-01: after D8, W plan has nothing ready without an owner decision
- Coordinator: stack w-dock-persistence..w-toolstrip-keyboard 50bb4e96 goes into checkpoint 9.
- The 12 partial rows (README coverage): TextBoxBase, ListControl, UpDownBase, ToolStripControlHost,
  ToolStripDropDown + DropDownMenu, the two DataGridView editing controls, ToolStripPanelRow (partial by
  design, RESUME_PLAN); Form (Window, MDI excluded); TabControl (Multiline deferred by the owner);
  DataGridView (provisional; breadth per gap catalogue B19). P8 navigation preset waits on P4 asked again.
- notes_w 32af14d updates the ToolStripContainer and ToolStripPanelRow rows (no count change).
- Options sent to the coordinator: (1, recommended) DataGridView breadth audit + next batch;
  (2) P4 mockups for the navigation preset; (3) lift a partial-by-design item (arbitrary hosted content:
  ToolStripControlHost + public ToolStripDropDown); (4) TabControl Multiline. Waiting for the answer.

## 2026-11-01: DataGridView breadth (owner accepted option 1): audit done, batch 1 proposed
- gui_forms branch claude/w-dgv-breadth from origin/main e1262237 (empty so far).
- Audit: notes_w de13574 planning/stages/W-winforms-controls/DATAGRIDVIEW_AUDIT.md (513 rows: 97 covered,
  23 errors/validation, 38 row editing/dirty, 58 cell/row events, 78 styles, 70 sizing/order, 8 sort,
  18 tooltips/menus, 21 geometry, 3 clipboard, 96 host plumbing). Classifier dev/tools/dgv_audit.py.
- Proposed batch 1 (waiting for the coordinator): editing lifecycle = data errors + validation + dirty
  state + new row/delete rows + cell/row enter/leave; decisions B1-1..B1-3 sent. Batch 2 styles (theme), batch 3 sort/sizing/order.

## RESUME POINTER (2026-11-01, owner wind-down; final for this phase)
Workspace ~/Developer/gui_forms_workers/w2. All branches local, working trees clean (gui_forms; notes_w).
- gui_forms `claude/w-dgv-breadth` 4714c3cd (on origin/main e1262237), VERIFIED: DataGridView batch 1, the
  editing lifecycle (owner decisions B1-1..B1-3). verify --gallery-wide --tests
  'data_grid_view|part_painting|docking_vocabulary' (15 tests incl. gallery|doc_examples|binding) plain + san,
  gate, style, docs, schema clean. Goes into checkpoint 9.
  Worker choices to surface: allow_user_to_add_rows / allow_user_to_delete_rows default OFF (WinForms: on);
  edit_on_enter begins edits only for text cells; marks pencil/asterisk/error are provisional (look -> T4).
- Docking/toolbar stack for checkpoint 9 (top first): w-toolstrip-keyboard 50bb4e96, w-toolstrip-rows e054da20,
  w-dock-look ecaf2ee5, w-toolstrip-drag 074dba87, w-dock-gallery 9ae7465c, w-dock-persistence 00253637.
- notes_w `claude/w-tooltip-decisions` 30703a7: DK-59 (4a3c3f3), strip rows (32af14d), DATAGRIDVIEW_AUDIT.md
  (de13574), batch 1 decisions (0cbec1a) and as built (30703a7).
- Next (when the phase resumes): rebase onto the checkpoint; DataGridView batch 2 = styles and formatting
  (needs an owner decision on mapping cell styles to theme roles, with T3/T4), batch 3 = sort modes,
  SortCompare, auto sizing, column reorder, clipboard; batch 1 leftovers: virtual new row via a provider call,
  CellErrorTextNeeded/RowErrorTextNeeded, row validation on grid focus loss, error tooltips.
- Tools: dev/tools/verify.py (--gallery-wide), dev/tools/dgv_audit.py. Schema JSON needs a --gallery build
  first (ci_build --gallery), then GUI_FORMS_CONTROL_SCHEMA_WRITE=1 gallery_pages_tests; scp generated files back.

## 2026-11-01 (Windows host): resumed on D:/gui_forms_w2; DataGridView defaults on
- Host: this Windows x86-64 machine (native Win32 host, MSAA/UIA tests run here; no M4). Build in
  D:/gui_forms_w2 (env.sh, build.sh, toolchain_env.sh); sanitizer /d/malkuth/projects/tools/sanitize.sh.
  template_harness cloned at the harness.lock pin 53af66c (D:/gui_forms_w2/template_harness).
- One-step check: `sh /d/gui_forms_w2/dev/tools/verify.sh '<ctest regex>' [base-ref]` = build.sh + touched
  tests, sanitize.sh, house style, lifetime, docs (regenerates; lists changed files, since the clone's
  autocrlf makes --check report every file), registry, schema, state-machine gate (HARNESS_PROJECT =
  template_harness/projects/gui_forms.toml, env.sh for ninja). Logs dev/logs/, summary dev/logs/summary.txt.
  Gallery-wide regex on Windows: 'gallery_pages|gallery_display|gallery_interaction|gallery_catalog|
  doc_examples|binding_tests|windows_accessibility_tests|msaa_gallery' (NOT uia_gallery: #459, standby's).
  A first build from stale takes ~36 min; incremental is minutes.
- Owner decision 2026-11-01: allow_user_to_add_rows / allow_user_to_delete_rows default ON (WinForms).
  gui_forms claude/w-dgv-defaults dacbfe62 on origin/checkpoint/2026-11-01-4 (b94c9a0a): defaults on, new
  row ID at construction; tests make_grid/bound_rows set the new row off explicitly; docstring + generated
  page. Gallery grid is virtual (unchanged). verify: 11 tests plain + san, all checks clean.
  The notes audit (DATAGRIDVIEW_AUDIT.md "both off by default, a worker choice") is now stale: fold the
  owner's decision there when notes are next edited.
- T4's style mapping is ready (notes claude/t4-plan a49e76e, STYLE_MAPPING.md; copy dev/style_mapping.md):
  batch 2 can start (colours stored and resolved, painting waits on T4-1).
- Next: batch 2 (styles) on claude/w-dgv-styles from dacbfe62; then batch 3 (sort modes, SortCompare,
  sizing, column order, clipboard); then MessageBox and the remaining W rows.

## 2026-11-01: DataGridView batch 2 (styles) done, on T4's style mapping
- gui_forms claude/w-dgv-styles e8421d99 (on claude/w-dgv-defaults dacbfe62): DataGridViewCellStyle (all
  optional), inheritance chains (cell, row, alternating odd, rows, column, grid; headers own/headers'/grid),
  row/cell styles by row ID (follow new-row rekey), font/alignment (9 places)/padding in the one geometry fn
  (cell_layout = formatted_cell + parts + content_box; cell_parts wraps it), format F0..F12 (Binding's set),
  null_value/data_source_null_value, cell_formatting (mutable Event, raised from const reads) and
  cell_parsing, unchanged edits store nothing, border_style/cell_border_style/header border styles,
  enable_headers_visual_styles, grid/background colour stored, owner draw (draw_mode, draw_item_content).
  Column `alignment` field removed -> default_cell_style (Gallery + tests moved). Colours not painted (T4-1).
  Paint stops when a handler changes structure (structure_revision_ bumped in structure_changed()).
- verify: 12 tests plain + san (data_grid_view, gallery-wide, owner_draw, windows accessibility), checks clean;
  schema rewritten (interactions in header declaration order: draw_item_content first).
- Windows notes: schema JSON rewrite = ctest -R gallery_pages_tests with GUI_FORMS_CONTROL_SCHEMA_WRITE=1 and
  GUI_FORMS_CONTROL_SCHEMA=<manifests/control-schema.json>, then control_schema.py render. A build can leave
  CRLF-only "modified" files (stx_rtf2md corpus, font_metrics_differential): git checkout them before add.
  docs/reference/inventory.json still names DataGridViewCellAlignment (regenerated only by the
  gui_forms_api_reference target; not checked).
- Deferred from batch 2: wrapped text drawing (2b: share TextBlock's greedy breaker), style changed events,
  per-row header cell styles, Custom border styles, RowPrePaint/RowPostPaint, formats beyond F0..F12
  (needs a shared public formatter in core binding: request to coordinator).
- Next: batch 3 (sort modes + SortCompare + Sorted/SortedColumn/SortOrder, auto sizing, column order,
  clipboard) on claude/w-dgv-sort from e8421d99; then MessageBox and the remaining W rows.

## 2026-11-01: batch 3 (sort + clipboard) and T4-1 colour painting; rebased on main 3e0c8d5a
- Branches (local only, gui_forms never pushed by me; coordinator folds):
  - On main 3e0c8d5a: claude/w-dgv-defaults 16782003 -> claude/w-dgv-styles 444602bb ->
    claude/w-dgv-sort 89f3b705.
  - On T4's claude/t4-themes 7f8b0b76 (fetched from D:/gui_forms_w1/gui_forms; local ref `t4-themes`):
    claude/w-dgv-colours = the same three commits cherry-picked (0a9a529e, c225c783, fb5d73ef) + colours
    810f8d31. This is the branch to fold; the main-based stack is its T4-free twin.
- Batch 3: DataGridViewColumnSortMode (column field optional; empty = kind default: text automatic, others
  not_sortable, as WinForms), automatic header click sorts unbound rows asc then toggles (after
  column_header_click; pointer and semantic press), sort_compare (DataGridViewSortCompareEvent, handled +
  sort_result; descending reverses the sign) via a stable bottom-up merge sort over indices (safe for
  inconsistent comparators; std::stable_sort kept when no handler), sort(DataGridViewRowComparer&),
  sorted_column/sort_order/sorted event, require_not_sorting() in every row/column/value mutator (logic_error
  while comparisons run), reset_row_state ends the sort (automatic glyph cleared).
  Clipboard: DataGridViewClipboardCopyMode (default auto header text), clipboard_content() (tab-separated,
  "\r\n" lines, empty fields for unselected columns; row headers for always, or auto in full_row_select;
  column headers for always; top-left field empty), Ctrl+C / Command+C / Ctrl+Insert via host_services().
- T4-1 colours: authored_colours(style, selected) -> parts: cell_selection bg, cell_content fg (not links),
  cell_button_label fg, cell rules fg = grid_color; headers (only with enable_headers_visual_styles off):
  header face bg, label + glyph fg, selection colours while selected. Unset selection colour = theme's
  selection look (not the unselected colour). background_color: well authored bg, then the well painted
  again (no authored colour) clipped to the cells' extent so the theme ground stays under cells (WinForms).
  Owner-draw ItemContentPaint.text stays the theme's text colour (handlers read inherited_cell_style).
- verify (both branches): 12 tests plain + ASan/UBSan, house style, lifetime, docs, registry, schema, gate
  clean. Gallery digests unchanged (Gallery grid is virtual: no automatic sort).
- New rule (BRIEF "Waking the coordinator"): never end a turn while my build/test runs; wait in the
  foreground (chained 10-minute waits); message only at milestones.
- Next: batch 2b (wrapped text), auto sizing and column order (DisplayIndex, user reorder); then MessageBox
  and the remaining W rows. Open request: a shared public number formatter (formats beyond F0..F12).

## 2026-11-02: wrapped text, column auto sizing, column order; rebased on main e22ea9c9 (checkpoint 10)
- Branches (local, on main e22ea9c9; colours folded into checkpoint 10): claude/w-dgv-wrap 662de265 ->
  claude/w-dgv-sizing 3f8b12a2 -> claude/w-dgv-order 03431f83 (one commit each; fold as one stack).
- 2b wrap: TextBlock's greedy breaker moved to src/controls/text_block/line_wrapping.hpp
  (wrap_lines_greedy<Line, Measure>, ControlRunWidth; + mandatory breaks at line ends; TextBlock output
  unchanged, text_block tests pass). Text cells with wrap_mode: lines at content width, block placed by
  alignment (top-clamped), cut at the cell bottom; content part = lines' box. Headers/other kinds: one line.
- Auto sizing: DataGridViewAutoSizeColumnMode (column, not_set->grid) / ...ColumnsMode (grid, default none),
  fill_weight; applied in arrange (apply_auto_sizes: cell/header modes only when auto_size_stale_ or text
  scale changed; displayed modes every pass; fill last via distribute_fill_width with minimums), no
  column_width_changed for automatic widths (deviation); auto-sized columns not user-resizable (fill too:
  deviation, WinForms moves weights). auto_resize_column(s) one-time + event. preferred_cell_width /
  preferred_column_header_width public. Value-change sites now call values_changed() (adds arrange when a
  column sizes by cells); structure_changed marks stale.
- Column order: DataGridViewDisplayIndex wrapper; display_order_/display_indices_; set/column_display_index,
  column_at_display_index, column_display_index_changed (each moved column, new order),
  allow_user_to_order_columns (default off; header drag after 4 units with capture; new grid part
  column_drop_indicator, role drop; release = move, no click). Display order drives layout, hit test,
  paint, realized items, arrows/Tab/Home/End, range selection, semantic column_index, clipboard fields.
  Lesson: the window releases capture before delivering the up, so "capture lost" handling must skip up.
- verify: 15 tests plain + ASan/UBSan (incl. anatomy, text_block), all checkers + gate clean; schema rewritten
  (column_display_index_changed, column_drop_indicator).
- Open decision for coordinator: per-row heights (AutoSizeRowsMode, row resizing, Row.Height) - rows are
  uniform today (row_height_, first_row_ scrolling by index).
- Next: MessageBox and the remaining W rows; per-row heights per the decision.

## 2026-11-02: owner decision on row heights; MessageBox found already on main
- Owner (via coordinator): option 1, real per-row heights for every row source including virtual rows,
  after MessageBox and the remaining W rows. Million-row grid must stay fast: indexed height structure
  (Fenwick / prefix-sum tree), offset->row and row->offset logarithmic, no linear walks in scrolling,
  hit testing or painting; measure on the million-row fixture. Wrap/sizing/order stack -> checkpoint 11.
- MessageBox: show_message_box is on main since checkpoint 2 (b7ca4433; tests message_box_tests); the
  2026-10-27 order list (MessageBox, link column, mnemonics, slow-click edit, RTB run model, ListView/
  TabControl gaps) is all done. ROADMAP_TO_0.5 "Left: MessageBox..." and W README row 361 are stale.
- MessageBox breadth gaps vs WinForms: button sets AbortRetryIgnore and CancelTryContinue (DialogResult
  abort/ignore/try_again/continue), MessageBoxDefaultButton.Button4, help button + HelpRequested,
  MessageBoxOptions RightAlign/RtlReading (DefaultDesktopOnly/ServiceNotification: Windows-session only).
  These touch HostMessageButtons and all three adapters (Win32 testable here; AppKit and X11 not buildable
  on this host). Asked the coordinator which to do (options below in the report).

## 2026-11-02: per-row heights (owner option 1), branch claude/w-dgv-row-heights (on w-dgv-order 03431f83)
- Coordinator order: (1) per-row heights, (2) DataGridView batch-1 leftovers, (3) MessageBox gaps (AppKit/X11
  compiled by checkpoint CI). Main still e22ea9c9 (checkpoint 11 pending a CI fix).
- ada4c429 index: public DataGridViewRowHeights (data_grid_view_row_heights.hpp/.cpp): default height + Fenwick
  tree of differences, nothing stored while uniform; height O(1), top/position_at/set O(log n), reset O(n+k).
  Grid: own heights by row ID (own_row_heights_), mutable index rebuilt when stale (rows_changed at the 17
  row-changing sites; count/default mismatch), set_row_height_at updates in place. row_height_at /
  set_row_height_at [4,4096] / reset_row_height_at, row_height_changed. Geometry, visible rows, fully visible,
  maximum first row (first_row_ending_at), scroll_into_view (also fixed: columns in display order), hit test,
  semantics read the index. Scrolling stays row-granular (WinForms).
- 51283e91 user row resizing: allow_user_to_resize_rows (default on), row_grip part, HitZone::row_divider,
  captured row state, resize cursors (rows and columns).
- fea3aa92 AutoSizeRowsMode (7 modes) + auto_resize_row(s) + preferred_cell_height /
  preferred_row_header_height; applied in arrange after column widths (apply_row_auto_sizes): all modes
  re-measure on stale (values, rows, structure, scale, column widths), displayed modes measure from the first
  row until the viewport is full each pass; automatic heights override own ones, no event, no user resize.
- Million-row fixture (Windows, build.sh tree), 2000 rounds scroll+layout+hover hit+paint: ~335 us uniform,
  ~338 us with 1000 own heights; displayed_cells ~596 us; 1000 set_row_height_at ~9 ms (first allocates
  16 MB); default-height change + rebuild + layout ~11 ms.
- Verified at fea3aa92: 15 tests (data_grid_view, text_block, anatomy, gallery set, doc_examples, binding,
  windows_accessibility, msaa_gallery, owner_draw) plain + ASan/UBSan; style, lifetime, docs, registry, schema
  (row_grip, captured, row_height_changed rewritten), gate clean. Stack: ada4c429 -> 51283e91 -> fea3aa92.
- Next: batch-1 leftovers (virtual new row via a provider call, CellErrorTextNeeded/RowErrorTextNeeded, row
  validation on grid focus loss, error tooltips), then the MessageBox gaps.

## 2026-11-02: batch-1 leftovers (part), branch claude/w-dgv-batch1-rest (on fea3aa92, which goes into checkpoint 12)
- b5e7e6a4: cell_error_text_needed / row_error_text_needed (virtual and bound rows; held_*_error_text without
  the event); validation when focus leaves the grid+editor (window focused_control_changed subscription in
  on_attachment_committed; skipped while a combo choice edit/menu is open; cancel does not hold focus,
  deviation). Verified: 15 tests plain + ASan/UBSan, all checks + gate clean.
- Coordinator question (rebuild cost): single-row height changes are already O(log n) in place
  (set_row_height_at, reset, user drag, displayed auto modes). The ~11 ms was set_row_height (default change).
  Full rebuild runs only on row-structure changes (insert/remove/sort/reorder/replace) and only while some row
  has its own height: a Fenwick tree cannot shift positions.
- Open decisions sent: (a) virtual new row API, (d) error/cell tool tips vs the tooltip rule ("a list of data
  items never publishes item tooltips").
- Next: MessageBox gaps (AbortRetryIgnore, CancelTryContinue, Button4+help/HelpRequested, RightAlign/RtlReading).
- Owner answers 2026-11-02: A option 1 (provider add_row), B option 1 (error glyph tooltips only), focus-loss
  cancel without holding focus = recorded divergence (compat manifest), no Fenwick rebuild work.
- e0663762 (on b5e7e6a4): provider can_add_rows (default false) + add_row(values) -> optional ID (default
  refuses); rekey to the given ID, edit-mode insert by the grid, window mode reported by the app; find_row /
  event_position know the virtual new row. Error glyphs publish image nodes with tool_tip (cell-error./row-error.
  prefixes); set_publishes_item_tool_tips(true); exception recorded at Control::publishes_item_tool_tips.
  Side effect: over cells/headers the grid's own tool_tip_text no longer shows (presenter ends at a node without
  a tooltip). Verified: 16 tests (+tool_tip) plain + ASan/UBSan, checks + gate clean.
- Compat manifest: none exists in gui_forms or notes (searched); divergence recorded in the DataGridView
  docstring and here; asked the coordinator where the manifest lives.
- Next: MessageBox gaps.

## 2026-11-02 (evening): divergences manifest, tooltip fallback, rebase onto 17f94617, MessageBox part 1
- claude/w-winforms-divergences c010442c (on main 17f94617, its own commit so others can add entries):
  manifests/winforms-divergences.json (control, member, winforms, gui_forms, reason, source owner|coordinator,
  date), tools/winforms_divergences.py check/render, tools/test_winforms_divergences.py (house-style job in
  native.yml), docs/controls/generated/winforms-divergences.md (toctree in docs/user/controls.md). Entries:
  DataGridView focus-loss cancel (owner), CheckBox back_color fills the square (T4, coordinator).
- DGV stack rebased onto c010442c (old heads superseded): ada4c429->a692da4f, 51283e91->8dc4029b,
  w-dgv-row-heights=1e317bf6, b5e7e6a4->4654ea91, w-dgv-batch1-rest=989e2ace,
  w-dgv-tooltip-fallback=f202eaaa (presenter: part tooltip overrides the control's own only where it exists;
  disabled/covered parts still nothing; DGV + strip tests; ToolTip/DataGridView docstrings; docstring points at
  the manifest). Verified at f202eaaa: 10 tests (DGV, tooltip, tool_strip, tab_control, gallery_pages) plain +
  ASan/UBSan, all checks, divergences, gate clean.
- claude/w-message-box-gaps f8ce1bf7 (on main): abort_retry_ignore / cancel_try_continue, choices abort/ignore/
  try_again/continue_, button4 (first button until help), MessageBoxOptions right_align/rtl_reading (WinForms
  values; MB_RIGHT/MB_RTLREADING; X11 right-aligns; macOS none), Win32/AppKit/X11, gallery 6 cases. Verified
  before the rebase (message_box, host_protocol, gallery_pages plain + ASan/UBSan, checks, gate); rebased clean,
  not rebuilt since. AppKit/X11 compile only in checkpoint CI.
- Open decision (to send): help button + HelpRequested route. Options: 1 MessageBoxRequest.help_button, host
  reports presses by request id, show_message_box raises a new Window::help_requested (WinForms raises it on the
  owner form) while the box stays open (recommended); 2 route through the window's HelpProvider for the root;
  3 a named help target object in the request. helpFilePath overloads not admitted (help is policy-free).

RESUME POINTER: w2 on claude/w-dgv-tooltip-fallback f202eaaa, clean. DataGridView work is done (anchor met).
Next session: MessageBox help button after the decision above (on w-message-box-gaps; rebuild + verify
'message_box|host_protocol|gallery_pages'), then FD (form documents) moves to this track (owner plan).

## 2026-11-03: MessageBox help (owner: option 1 + provider fallback), branch claude/w-message-box-gaps
- Rebased onto 855b0f0e (checkpoint 12 has the DGV stack + divergences manifest): 590a977b (button sets,
  options), 715fb281 help: MessageBoxRequest::display_help_button; Window::help_requested
  (Event<HelpRequestEvent&>) + Window::request_help(keyboard) -> window handlers, then HelpProviders in creation
  order (detail::WindowHelpFallback registered by each provider, removed on dispose) until handled; provider
  fills its namespace + root mapping. Host: HostMessageDialogRequest::help_button, HostDialogChoice::help (default
  only), HostDialogHelpEvent, HostServices::dialog_help_requested/publish_dialog_help (open dialogs only).
  show_message_box subscribes detail::MessageBoxHelpRoute (weak window lifetime; handler exception held and
  rethrown after the box closes). Win32 MB_HELP + WM_HELP at the owner (F1 with or without button) +
  MB_DEFBUTTON4; AppKit showsHelp + GuiFormsAlertHelp delegate (no F1 on macOS); X11 Help button + F1 accelerator.
- Verified: message_box, guidance_provider, host_protocol, doc_examples, gallery_pages plain + ASan/UBSan;
  checks, divergences, gate clean. C ABI catalogs (abi_event_catalog, emit-abi) are current: they cover
  components' append_interactions only, so Window events do not enter them (C ABI track to extend if wanted).
  No divergence found.
- Next: FD (form documents), brief at notes/planning/stages/FD-form-documents/README.md.

## 2026-11-03: FD (form documents) moves to this track: findings and draft plan (awaiting decisions)
Brief: notes/planning/stages/FD-form-documents/README.md (owner decisions 2026-10-13: CSS-family syntax shared with
T3, one parser and round-trip machinery; Gallery DML migrates; gate accepted).
Findings:
- T3 front end is in src/core/theme_language/ (internal): tokenizer, ThemeSyntaxTree (caller-owned node table,
  NodeIndex/TokenIndex/ByteOffset types), parse_theme_sheet(form = sheet|inline_style), write_theme_sheet (gap-
  copying byte-exact writer; "a later editor replaces one node's bytes"). The parser nests at most 4 deep (sheet,
  at-rule, rule, declaration): no nested rules, so forms need a third ThemeSheetForm with object rules nested up to
  maximum_nesting_depth (32).
- demo/gallery.dml (92 lines, `<type> <id> { prop: value; children }`) is hand-transcribed into demo/gallery_dml.hpp
  (41 NodeSpec rows); there is no DML parser. Most Gallery pages are imperative C++ (src/controls/gallery/pages).
- Runtime schema: Control::property_value / set_property_value(name, BindingValue) / reset_property with
  PropertyDescriptor (kind, enumeration choices, writable, serialization); manifests/control-schema.json (115 types)
  is its projection. static_tree's ControlFactory (type name -> creator) exists but is minimal.
Draft phases: FD1 form grammar in the T3 parser (form_document form: object rules `<type> <stable-id> {}` nested,
declarations = properties in kebab-case, `on-<interaction>: <handler>;`, inline `style {}` block in T3 inline
syntax), byte-exact round trip on a corpus. FD2 document model + validation against the run-time registry before
anything is built (exact diagnostics file/line/node). FD3 run-time builder (transactional, for tests and D's preview).
FD4 editor: set/reset one property, add/remove/move one object, touching only those bytes. FD5 C++ generator (tool
built on the same parser) whose output compiles against the installed SDK and passes house style. FD6 Gallery:
gallery.dml migrates first, then each page's structure moves into documents with behaviour left in named handlers.
Decisions sent to the coordinator (see report): object prelude, order semantics, handler binding, generator
language, run-time loader, extension, public surface.
- Coordinator 2026-11-03: the C ABI worker stacks claude/c-abi-window-events on 715fb281 (window.hpp, window.cpp). Leave those two files alone until checkpoint 13 is folded; ask the coordinator first if FD needs them.

## 2026-11-03: strip merging (owner: notes 5c52a20, DK-62, matrix row 46), PARKED for FD1
Branch claude/w-strip-merge 0e1ebf38 on origin/main 855b0f0e, WIP, NOT BUILT. Done: MergeAction enum (context_menu.hpp),
merge_action + merge_index on MenuItemSpec/ToolStripItemSpec/MenuStripItemSpec; allow_merge on ToolStrip and MenuStrip;
private apply_items(items, merged) (merged relaxes the "menu items need a non-empty menu" rule; validate_specs gets
allow_empty_submenus); prune_empty_submenus applied in ToolStrip::show_drop_down, overflow_menu, MenuStrip::open;
src/controls/tool_strip/strip_merge.hpp history types (MergeStep{moved|removed, item_id, target/source path+index,
removed_top/removed_item}, StripMerge{weak source, steps}, ToolStrip/MenuStripMergeHistory); friend detail::StripMergeAccess.
To do: strip_merge.cpp (CMake near tool_strip.cpp) engine templated on Top; public header
include/gui_forms/controls/tool_strip/tool_strip_manager.hpp: merge_strips(source, target) -> bool (throws
invalid_argument on self), revert_merge(target), revert_merge(target, source) (pops newer merges, re-merges them in
order). Algorithm (WinForms recollection): FindMatch = case-insensitive displayed-text match (empty matches empty), else
target[merge_index] if in range; MatchOnly recurses when source has a non-empty menu and target holds a menu; Replace/Remove
remove the match (Replace moves source to its index); Insert needs only merge_index (min(index,count)), no match; Append
moves to end; walk keeps index when the source shrank; returns true when any step happened. Sync item_checked into
spec.checked on both strips before applying. Work on copies; apply target first (invalid_argument -> false, nothing
changed), then source, rollback on failure. Dead source on revert: its items are dropped from target. Divergences to
record: ASCII-only case folding; same kind only; ID collision refuses; hosted controls recreated; empty submenus hidden;
recursion only into items already holding a menu. Tests: new tests/strip_merge_tests.cpp.

## 2026-11-03: FD decisions (owner accepted all six recommendations); FD1 started
(1) header `<type> <stable-id> { ... }`; (2) document order = child order (later on top), tab order from tab-index;
(3) handlers `on-<interaction>: name;`, generated C++ templated over the owner class, Delegate::bind, no virtual interface;
(4) C++ generator on the same parser, shipped in the SDK; (5) internal run-time loader alongside generation;
(6) `.form` files; parser and model internal until D.
- FD1 DONE, VERIFIED: claude/fd1-form-grammar 165bb6cb on origin/main 855b0f0e. ThemeSheetForm::form_document; node kinds
  object_rule (content = prelude: type ident, trivia, stable ID run checked by valid_stable_id) and style_block (T3 inline
  style inside `style { }`); objects nest iteratively on the explicit stack (32); writer depth bound = stack + 3; no new
  diagnostic codes (E201/E203/E206/E202/E503/E504 reused, so the T3 conformance corpus is untouched). Tests
  tests/form_language/form_syntax_tests.cpp + corpus/*.form (gallery.form = gallery.dml with // comments as /* */);
  theme sheet fuzzer parses each input as a form too. Verified form_syntax, theme_syntax, conformance, inline_style,
  sheet_fuzz plain + ASan/UBSan; checks, divergences, gate clean.
- Next: FD2 (document model + validation against the run-time registry: types, properties, handler names, one style
  block, tab-index, exact diagnostics), then FD3 loader, FD4 editor, FD5 generator, FD6 Gallery. Strip merging is parked
  on claude/w-strip-merge 0e1ebf38 (see the entry above) and resumes after FD.
- FD2 DONE, VERIFIED: claude/fd2-form-model 397739a3 (on fd1 165bb6cb). src/core/form_language/form_model/: FormSchema
  (FormSchemaType: registered name, holds_children = is a Control, PropertyDescriptors, interaction names) filled by
  describe_form_type(name, prototype); build_form_model(source, tokens, lines, tree, schema, model, diagnostics) ->
  model_built | refused (scratch model, caller's replaced only on success). Kebab document names -> registered snake names;
  values per kind (bool, ints, number, string, point/size 2, rect 4, insets 1/2/4 CSS order, #hex colour, enum / flags
  idents; font/image/object/collection E999), then convert_property_value. New codes E801-E810 (COVERAGE code-elsewhere
  rows -> form_model_tests). Tests tests/form_language/form_model_tests.cpp (real UserControl/GroupBox/CheckBox/Button/
  Command prototypes). Verified form_syntax, form_model, theme_conformance, theme_syntax plain + ASan/UBSan; checks clean.
- OPEN (sent to coordinator): the root object's type. Window is not a registry type, so `form` (title, size,
  accept/cancel button) is not expressible; FD2 admits any registry type as root.
- Next: FD3 run-time loader (transactional build via the ABI catalog's create_catalog_type or a factory), FD4 editor.
- FD3 DONE, VERIFIED: claude/fd3-form-loader 9c8a57b2 (on fd2 397739a3). src/core/form_language/form_loader/:
  load_form(source, tokens, lines, tree, schema, model, LoadedForm&, diagnostics) -> loaded | refused (E811
  form_load_refused per refusal; scratch result, caller's unchanged on failure). FormSchemaType::create
  (FormObjectCreator fn pointer); templates create_form_control/create_form_component, form_control_type<T>(name),
  form_component_type<T>(name). Objects made in document order, controls add_child'ed in order, then properties set in
  document order; handlers returned (LoadedForm::handlers) for the caller / FD5 to connect.
  Owner accepted FD option 2: reserved `form` root (form_root_type(): text, client_size, accept_button, cancel_button;
  only the root; exactly one control child; button IDs must name document objects), loader returns LoadedWindow and
  root = the form's control. Window interactions to come from the C ABI worker's Window::append_interactions; window
  hold still on. Verified form_syntax/model/loader + conformance plain + ASan/UBSan; checks clean.
- Next: FD4 editor (set/reset one property, add/remove/move one object, touching only those bytes), then FD5 generator.
- FD4 DONE, VERIFIED: claude/fd4-form-editor 090834e3 (on fd3 9c8a57b2). src/core/form_language/form_editor/: FormDocument
  (file, text, tokens, lines, tree, model); open_form_document, set/reset_form_property, add/remove/move_form_object ->
  FormEditOutcome edited|unchanged|object_missing|not_admitted|refused. One splice from the syntax tree, layout follows
  neighbours (one-line blocks inline, else sibling indent, document's CRLF/LF; removal takes its line when alone); the
  edited text is re-read (parse + model) and replaces the document only when valid. Moves keep the object's bytes
  verbatim (inner indentation not re-indented). Tests assert every edit changes only its own bytes.
  Lesson: never start a verify run while the previous one is still running (shared summary.txt interleaves); wait for
  the background task's own completion notice.
- Next: FD5 C++ generator (tool on the same parser, shipped in the SDK; output templated over the owner class, handlers
  via Delegate::bind, house style), then FD6 Gallery migration.
- Coordinator order (mandatory): test runs that need the desktop foreground (Gallery pages/interaction, UIA clients,
  window focus/capture tests) run under `python -B D:/malkuth/projects/tools/foreground_lock.py -- <command>`.
  dev/tools/verify.sh now wraps the plain and sanitizer runs in the lock when the regex names gallery, uia, window,
  message_box or host_. The FD tests are headless and run without it; no recent Gallery failure to rerun.

## RESUME POINTER (2026-11-03, reboot stop)
Workspace D:/gui_forms_w2 (clone D:/gui_forms_w2/gui_forms). All local, working tree clean, nothing uncommitted.
- claude/fd1-form-grammar 165bb6cb, claude/fd2-form-model 397739a3, claude/fd3-form-loader 9c8a57b2: VERIFIED, accepted
  for checkpoint 14 (stacked: fd1 <- fd2 <- fd3).
- claude/fd4-form-editor 090834e3 (on fd3): VERIFIED (form_syntax/model/loader/editor plain + ASan/UBSan, checks, gate);
  reported, acceptance not yet seen.
- claude/w-strip-merge 0e1ebf38 (on origin/main 855b0f0e): WIP, NOT BUILT, parked (see the strip-merging entry above).
- claude/w-message-box-gaps 715fb281: in checkpoint 13 flow; window.hpp/.cpp hold still applies.
- Next: FD5 C++ generator (tool on the same parser, SDK-shipped; generated code templated over the owner class, handlers
  via Delegate::bind, house style; compiles against the installed SDK), then FD6 Gallery migration, then strip merging.
- Foreground lock: verify.sh wraps gallery/uia/window/message_box/host_ runs in foreground_lock.py.

## 2026-11-02 (after the reboot): MessageBox Sphinx fix
- claude/w-message-box-gaps 92499203 (on 715fb281): docs/library/docstrings/Window.rst had "`HelpProvider`s"
  (no end-string); now a cpp:class reference. Docs regenerated; sphinx-build -W --keep-going (D:/gui_forms_int/.venv-docs)
  passes. The UIA ASan use-after-free is the hot-paths fix, not worked around here.
- Sphinx locally: D:/gui_forms_int/.venv-docs/Scripts/python.exe -m sphinx -W --keep-going -b html docs <out>.
  Docstring sources are docs/library/docstrings/*.rst, not the headers.
- Next: FD5 C++ generator (nothing of FD5 was committed before the reboot; start from fd4 090834e3).

## 2026-11-02: FD5 the C++ generator (branch claude/fd5-form-generator on fd4 090834e3), IN VERIFY
- src/core/form_language/form_generator/: generate_form_cpp(source, tokens, lines, tree, schema, model, options,
  generated, diagnostics) -> generated|refused (E812 form_generation_refused; scratch string). form_member_name (stable
  ID with '.'/'-' -> '_'), form_identifier. FormSchemaType gains cpp_type + header.
- Generated header: `template <typename Owner> class <Name> final` with one shared_ptr member per object (document
  order), initialize_component(Owner&) builds a scratch instance (make in document order + add_child, then
  set_property_value per property in document order, then handlers) and moves it in; handlers via
  Delegate<Args...>::bind<Owner, &Owner::h>(owner) with Args deduced from the event accessor (HandlerDelegate
  partial specialization over decltype), tokens held by the form (connections_). Form root: window_title,
  window_client_size, attach_buttons(Window&). root_control(). Style blocks not generated (loader does not apply them).
- Choice made (to report): properties through set_property_value + BindingValue (same path as the loader; no setter
  metadata exists in the registry); typed setters need a setter column first.
- tools/form_compiler/: form_compiler.cpp (gui_forms_form_compiler <doc> --class N --output H [--namespace] [--check];
  writes only on change; file:line:col diagnostics), form_catalog.{hpp,cpp} (registry_form_schema: every registry
  type makeable alone + form root), form_compiler_catalog.hpp generated by `control_registry.py emit-forms`
  (test + native.yml check added). CMake option GUI_FORMS_BUILD_FORM_COMPILER (ON unless cross), installed to bin.
- Tests: tests/form_language/form_generator_tests.cpp (generated == loader tree and values, handlers fire,
  reinit releases connections, golden current, refusals located, options throw); golden
  tests/form_language/generated/sample_dialog.form -> sample_dialog_form.hpp; ctest gui_forms_form_compiler_tests
  (--check) and gui_forms_form_compiler_refusal_tests (E812 regex).
- FD5 DONE, VERIFIED: claude/fd5-form-generator e5652ca5 (on fd4 090834e3). Local Windows: form_*|theme_syntax|
  theme_conformance (9 tests incl. form_generator, form_compiler, form_compiler_refusal) plain + ASan/UBSan; house
  style, lifetime, docs, registry (+ emit-forms --check, test_control_registry), schema, divergences, gate (no F1/F2);
  test_workflow_pipelines/test_build_sdk/test_package_sdk pass.
- Reported decision: generated properties use set_property_value (loader's path) vs typed setters (needs a setter
  column in the registry). Handlers' signatures must match the event's arguments exactly (Delegate, no prefix rule).

## RESUME POINTER (2026-11-03)
Workspace D:/gui_forms_w2 (clone D:/gui_forms_w2/gui_forms), on claude/fd5-form-generator e5652ca5, tree clean.
- Stack: fd1 165bb6cb <- fd2 397739a3 <- fd3 9c8a57b2 <- fd4 090834e3 <- fd5 e5652ca5, all VERIFIED.
- claude/w-message-box-gaps 92499203: Sphinx fix on 715fb281 (UIA ASan UAF is the hot-paths fix).
- claude/w-strip-merge 0e1ebf38: WIP, NOT BUILT, parked (see strip-merging entry).
- Next: FD6 Gallery migration (demo/gallery.dml -> a .form; each Gallery page's structure into documents, behaviour
  in named handlers; gate: generated C++ for every page compiles and passes its tests), then strip merging.
- Window hold: window.hpp/.cpp untouched until checkpoint 14 is folded.

## 2026-11-03: FD6 survey and plan (decisions sent to the coordinator)
Coordinator: w-message-box-gaps 92499203 and FD5 e5652ca5 accepted for checkpoint 14; hot-paths UAF fix verified;
property path stays option 1 (set_property_value) until the owner rules.
Findings:
- demo/gallery.dml (41 nodes) feeds dml::gallery_nodes (demo/gallery_dml.hpp) and the bespoke GalleryControl kinds in
  src/controls/gallery_controls.cpp (1155 lines: own layout kinds flex-row/form-grid/stack, flags, paint planes,
  materials, page mounts). Its kinds (command-strip, backplane, text-input, slider, row, instrument, diagnostics) are
  not registry types.
- 29 pages (8181 lines) in src/controls/gallery/pages/: 269 make_control (81 Button), 467 add_at (absolute bounds on
  Surface = ScaledPanel), 196 label(...) helpers (font size/weight + colour), 95 group(...) (LayoutGroup), 43
  set_items, 297 on(...) many with bound values (on(event, *this, &P::m, index)), 81 loops.
- Language gaps: font values (E999; FontSpec role/size/weight/italic/letter_spacing); item collections are not even
  run-time properties (DomainUpDown/ListBox/ComboBox set_items has no PropertyDescriptor); application types (
  Gallery's LayoutGroup, GalleryLifecycleCard) need a qualified cpp_type outside namespace gui_forms; handlers cannot
  carry bound values.
Plan: FD6.1 language completions (fonts; text item collections as schema properties + `items: "AM" "FM";`;
application types with qualified C++ names; a CMake helper gui_forms_add_form() running the compiler at build time,
exported for SDK consumers). FD6.2 gallery.dml -> demo/gallery.form. FD6.3 pages one by one (structure -> documents
generated at build time; loops expanded; data-driven content stays in code), smallest first; gate per page: generated
C++ compiles, gallery_pages/interaction tests pass, page screenshot pixel-equal.
Decisions asked: (1) DML kinds -> stock registry types, retiring gallery_dml.hpp + GalleryControl (look of 4 pages
changes) [recommended] vs Gallery-private form types keeping the look; (2) bound handler values: one named handler
per object [recommended now] vs `on-x: name(2);` through gui_forms::on (owner must be a Component, changes FD-3);
(3) font syntax `font: control 12 700 italic;` (role size weight [italic], CSS-like order) [recommended];
(4) item collections: add `items` PropertyDescriptors (text collection) to the item controls (T1 schema change) and
`items: "a" "b";` [recommended].

RESUME POINTER (2026-11-03): w2 on claude/fd5-form-generator e5652ca5, clean. Waiting on FD6 decisions (1)-(4) above;
decision-free FD6.1 work to start meanwhile: qualified application types in FormSchemaType/generator, and the CMake
gui_forms_add_form() helper. Then strip merging after FD.

## 2026-11-03: FD6.1 part 1 committed; owner rulings on FD6
- claude/fd6-application-types 34a1e22e (on fd5 e5652ca5), VERIFIED (form_*|theme_* plain + ASan/UBSan, all checks,
  gate): qualified cpp_type; compiler --type "<name>,<C++ class>,<base>,<header>" (add_application_type);
  cmake/GUIFormsForms.cmake gui_forms_add_form(target doc CLASS NAMESPACE OUTPUT TYPES) using GUIForms::form_compiler;
  own export set GUIFormsFormCompilerTargets; GUIFormsConfig includes the helper. SDK consumer checked locally
  (install + find_package + gui_forms_add_form + run). tests form_application_type_tests.
- OWNER RULINGS: (1) per Gallery-only kind: equivalent registered control -> use it; else extend a registered control
  or add a real registered control (API, house style, schema, tests, docs); record each verdict (equivalent /
  extension / new) here BEFORE converting and send the list; then retire GalleryControl + gallery_dml.hpp.
  (2) one named handler per object. (3) `font: control 12 700 italic;`. (4) `items` text-list property in the
  run-time schema, `items: "AM" "FM";`. Property setting: option 1 stays; owner wants more detail.
- QUEUED (after FD6.1, before page conversion): design study on typed property keys ->
  D:/malkuth/projects/desks/coordinator/PROPERTY_KEYS_STUDY.md. Approaches (a) typed keys + operators ([] or ->*),
  (b) macro layer, (c) our own pre-pass (moc-like, build-time, CMake helper). NO C++26 reflection. The `?` in
  `(.button)?text` is a placeholder for a dot substitute: propose candidate spellings per approach. Cover key types
  and storage, dirtiness/events (update-scope batching), hooks, reads under house style, binding + C ABI schema,
  migration (keys wrap getters/setters, then fields), measured costs on Button/TextBox/ListBox; for (c) grammar,
  #line diagnostics, build/SDK, IDE/debugger, clang plugin. Numbered owner decisions, recommendation first.
- Next: FD6.1 part 2 (font values, items property), then the study, then the kinds verdict list, then pages.

## 2026-11-03: FD6.1 part 2 built; property study written; Gallery kinds verdicts
- FD6.1 part 2 UNCOMMITTED on claude/fd6-application-types (on lab commit 4abe3e16): form-language `font: <role>
  <size> <weight> [italic]` (form_model font_value, generator FontSpec literal), text collections (`items: "AM" "FM";`,
  text_collection_value; generator make_property_collection literal), `items` registered on ListBox and DomainUpDown
  (src/controls/panel/text_items_property.hpp; CheckedListBox inherits), maximum_value_tokens 256; tests
  form_generator_tests test_fonts_and_items + golden sample_dialog_form.hpp, form_model_tests E999 case now `image`.
  form_generator_tests and form_model_tests pass. PENDING: schema regen (gallery_pages_tests with
  GUI_FORMS_CONTROL_SCHEMA_WRITE=1 under the foreground lock; background job waiting since the lock is held by
  D:\gui_forms_int checkpoint verify since 2026-11-02T23:52), then control_schema.py render, verify.sh
  'form_|theme_conformance|theme_syntax|list_box|domain|combo|input_controls|gallery_pages', commit.
- Property study DONE: D:/malkuth/projects/desks/coordinator/PROPERTY_CAPABILITIES_STUDY.md (projects 929f549,
  14c1a59; revised for the owner's correction: ONE shared dirty structure, no per-control changed bits). Lab
  tools/property_capability_lab.cpp committed 4abe3e16 (`--dirty` runs only the dirty section). Findings: spelling
  costs nothing (setter 12.4 / capability 13.3 / key 13.3 ns); per-instance registrations ~20 KB heap per control;
  static table lookup 17 ns vs set_property_value 126-267 ns; today's attached invalidate 70-568 ns paint, 741-1414 ns
  measure (eager ancestor walk, damage, getenv per layout mark = defect, window.cpp is on hold); recommended A2
  per-kind bitsets with summary level per Window, UI-thread only, no atomics.
- Gallery-only kinds verdicts (owner ruling 1; SENT to the coordinator, waiting before converting):
  form -> form root (equivalent); command-strip -> tool_strip (equivalent; toggle = check-on-click item, status
  text = label item); backplane -> material_panel (equivalent; fresco becomes a SurfaceMaterial); panel, user-control,
  label, link-label, button, checkbox (three-state), radio -> the registered controls (equivalent); group ->
  group_box (equivalent); text-input -> text_box (equivalent, placeholder_text); slider -> track_bar, progress ->
  progress_bar (equivalent); list + row -> list_box with `items` and selected index (equivalent; per-row stable ids
  become item identities, drag hover = allow_drop); diagnostics -> metrics_view (equivalent); layouts flex-row /
  flex-column -> flex_panel, form-grid -> grid_panel, stack -> block_panel (equivalent); instrument (animated 30 Hz
  sweep trace over a value) -> NO registered equivalent (instrument_rack edits modules, live_surface publishes
  frames): recommend it stays the Gallery's custom-control exhibit as an FD6.1 application type
  (gallery::Instrument : user_control), which conflicts with the letter of ruling 1 -> asked.
- RESUME POINTER (2026-11-03): w2 on claude/fd6-application-types 4abe3e16 + FD6.1 part 2 uncommitted. Next: when
  the foreground lock frees, schema regen + verify + commit part 2; then on the coordinator's word FD6.2
  (gallery.dml -> demo/gallery.form, retire GalleryControl + gallery_dml.hpp) and FD6.3 pages; strip merging when idle.

## 2026-11-03: foreground tests deferred (coordinator); strip merging resumed in a worktree
- FOREGROUND TESTS PENDING (run when hot paths' private desktop is ready): gallery_pages_tests (incl. the schema export
  byte-check of FD6.1 part 2: manifests/control-schema.json was edited by hand to add `items` to ListBox,
  CheckedListBox and DomainUpDown in the generator's format; regenerate with GUI_FORMS_CONTROL_SCHEMA_WRITE=1 and
  confirm no diff), gallery_interaction_tests, any uia/msaa/focus/capture tests touched by FD6.2+.
- Property study extended (projects 72b21be): subtree marks, container-declared escalation, content regions with
  lazy pending sets and redraw_all folding (DataGridView worked example), surface-root tables; decisions 9-11.
- Strip merging: worktree D:/gui_forms_w2/strip (branch claude/w-strip-merge, on main 855b0f0e), own build dir
  strip/.build. Written: include/gui_forms/controls/tool_strip/tool_strip_manager.hpp (merge_strips, revert_merge x2,
  ToolStrip and MenuStrip overloads), src/controls/tool_strip/strip_merge.cpp, tests/strip_merge_tests.cpp, CMake,
  menu_controls.hpp umbrella, six divergences (source "coordinator": NEED the coordinator's acceptance).

## 2026-11-03: FD6.1 part 2 and strip merging VERIFIED; deferred foreground tests run
- claude/fd6-application-types 1febc847 (FD6.1 part 2, on lab 4abe3e16 on 34a1e22e): verify.sh plain + ASan/UBSan,
  all checks, gate clean; gallery_pages (schema export byte-matches the hand-added `items` entries) and
  gallery_interaction passed on the private desktop (run_on_desktop.py), plain and sanitizer.
- claude/w-strip-merge a0931013 (be907b0f + compliant switch/docs, on main 855b0f0e): merge_strips/revert_merge;
  strip_merge, tool_strip, context_menu tests plain + sanitizer; house style, lifetime, docs (API rst regenerated),
  registry, schema, divergences, gate (no F1/F2); gallery pages/interaction plain + sanitizer on the private
  desktop. Six divergences carry source "coordinator": need the coordinator's acceptance before folding.
- No foreground tests pending. Strip worktree removed; w2 build dir shared again (.build/strip is a spare).
- RESUME POINTER (2026-11-03): w2 clean on claude/w-strip-merge a0931013. Waiting on: coordinator's word on the
  Gallery kinds verdicts (instrument = application type?) before FD6.2; owner decisions 1-11 of
  PROPERTY_CAPABILITIES_STUDY.md. Next: FD6.2 (gallery.dml -> demo/gallery.form, retire GalleryControl +
  gallery_dml.hpp) on claude/fd6-application-types 1febc847.

## 2026-11-03: owner rulings; FD6.2 under way (branch claude/fd6-gallery-form on 1febc847)
- Owner rulings (coordinator, 2026-11-03): (1) PROPERTY_CAPABILITIES_STUDY.md ACCEPTED, all 11 decisions as
  recommended. (2) instrument: BOTH -- keep gallery::Instrument : user_control as the Gallery's custom-control
  exhibit, AND later ingest a real instrument control into the instruments package (gauges, knob, seven-segment,
  sparkline beside the charts) with its sweep as an effect program once effect programs fold (may go to another
  worker; tell the coordinator if saturated). (3) ASCII case-folding divergence ACCEPTED: restored on
  claude/w-strip-merge 83695883 with source "owner" (check + render --check OK). Never mark entries "coordinator"
  before the coordinator has seen them.
- Order, all for 0.5: finish FD6.2 and FD6.3 -> PROPERTY CAPABILITIES MIGRATION per the study (family by family;
  generated code writes capabilities once they exist; setters stay as forwarders until the 0.5 settlement; PLAN
  THE BATCHES IN THIS TRACK FIRST) -> window.hpp/.cpp work incl. the getenv fix in Window::mark_dirty after
  checkpoint 14 folds.
- FD6.2 commits so far: 3938e122 GroupBox caption is the `text` property (schema regenerated); 84f9a045
  gallery::Instrument as an application type (own commit, cheap to change).
- FD6.2 mapping (tell the coordinator): command strip -> panel + buttons + picture-box + label (not tool_strip:
  tests address the buttons by ID); diagnostics -> group_box + labels (not metrics_view); extra application types
  gallery::LifecycleCard (user_control) and gallery::CollectionList (list_box, typed copy drop target).
- In progress (uncommitted): demo/gallery.form, demo/gallery_parts.{hpp,cpp}, gallery_model.cpp without dml; next
  rewrite gallery_context.hpp / gallery_tree.hpp / gallery_controls.cpp around GalleryDocument<GalleryContext>,
  gallery_pages mounts (page, size), make_gallery sets the status badge on the picture box, delete gallery.dml,
  gallery_dml.hpp, gallery/control/gallery_control.hpp; adapt gallery_interaction_tests; verify.sh + gallery tests
  on the private desktop.

## 2026-11-03: FD6.2 DONE (claude/fd6-gallery-form 09034d31); MessageBox trace fix; migration batch plan
- FD6.2 commits on 1febc847: 3938e122 GroupBox `text`; 84f9a045 gallery::Instrument; 0d4e64df the form document
  (demo/gallery.form -> GalleryDocument<GalleryContext>; gallery.dml, gallery_dml.hpp, GalleryControl retired;
  LifecycleCard + CollectionList in demo/gallery_parts.*; manual.json GalleryControl entry removed); 09034d31 every
  Gallery display digest re-recorded (diffed vs main 855b0f0e page by page: command strip themed panel + picture box,
  group boxes, TextBox at 12 px (was 13), ListBox rows, descriptions; counts on states/typography; note in the table).
  Verified: verify.sh ('form_|basic_controls|list_box|collection_controls') plain + sanitizer, all checks, gate clean;
  gallery pages, display trace, interaction plain + sanitizer on the private desktop; abi_registry_lifetime plain +
  sanitizer; MSAA + UIA gallery clients plain under foreground_lock (sanitizer run queued, see below).
- Finding (defect, for the migration's Control family): Panel ignores Control::back_color when themed (its material
  overdraws the ambient fill; set_background is a separate override). gallery.form no longer sets back-color.
- tools/run_windows_wine_smoke.sh clicks `gallery.collection.gamma` by stable ID: now a ListBox item ID (realized
  item), unverified here (Wine).
- claude/w-message-box-gaps 2716d8ee (on 92499203): gallery_display_trace re-recorded for dialogs-and-host-services,
  states-and-diagnostics, typography-and-inspection (six message buttons, two controls more); labels shortened to
  "Abort/Retry/Ignore" etc. because the dotted ones overflowed the 138 px buttons (negative text x). Diffed vs main;
  pages + display trace plain + sanitizer on the private desktop.
- claude/w-strip-merge 83695883: ASCII case-folding divergence restored, source owner.
- Tools: dev/tools/trace_at.sh <ref> <name> (checkout, build pages test, traces to dev/traces/<name>, digest table).

### PROPERTY CAPABILITIES MIGRATION: batch plan (study accepted, all 11 decisions; after FD6.3)
Each batch: one branch commit series; reads switched by a per-family script (include src demo tests examples
file_manager_demoboard); setters kept as one-line forwarders; equal-value writes do nothing; change events at the
write; schema regenerated from the tables and byte-checked; verify.sh + Gallery tests on the private desktop; sizeof
and per-control cost re-measured with tools/property_capability_lab.cpp.
- M0 (window hold: after checkpoint 14 folds) shared dirty structure: per-Window control table (dense index,
  generation-checked, attach/detach), A2 per-kind bitsets + summary, ancestor walk and damage moved to the flush,
  settle-before-read; subtree bitset + container-declared escalation (nearest escalating ancestor cached at attach);
  UI-thread only, other-thread writes rejected and counted. Measure the Gallery and 1k/10k/100k. Same window batch,
  own commit: the getenv fix in Window::mark_dirty.
- M1 PropertyCell + capability bases (CRTP on the declaring class, standard-layout static_asserts) + the per-type
  constexpr table; schema export from the tables, byte-equal to control-schema.json; no property moved.
- M2 Control's family (enabled, visible, bounds, text, font, back_color/fore_color incl. the Panel defect,
  tab_index, cursor, tool_tip_text).
- M3 ButtonBase and leaves (checked, check_state, three_state, default button, visual style, images).
- M4.. one family per change: ranges (value/min/max/changes); text boxes; list controls (items, selection);
  labels and link labels; panels/containers (border, background, material); pickers/calendars; strips and menus;
  docking; ribbon; components/dialogs; the rest.
- M-R content regions (decision 10), first user DataGridView's cell region; redraw_all folds regions.
- M-G the generator writes capabilities (decision 7); corpus and Gallery regenerated.
- M-S at the 0.5 settlement: setters removed, C ABI schema regenerated from the tables (with the C ABI worker).
- The ingested instruments-package control (owner ruling 2) is NOT in this plan: recommend another worker.

RESUME POINTER (2026-11-03): w2 on claude/fd6-gallery-form 09034d31, clean. Pending: sanitizer MSAA/UIA gallery
batch (foreground_lock -> sanitize.sh 'accessibility_(msaa|uia)_gallery', output dev/fg_san.out). Next: FD6.3, pages
one by one smallest first (animation-laboratory 96 lines first), gate per page = display trace digest unchanged.

## 2026-11-03: accepted 2716d8ee + FD6.2 09034d31; sanitizer UIA gallery passed; FD6.3 decision asked
- Sanitizer MSAA gallery passed (381 s), UIA gallery passed (1999 s, holds the foreground lock ~33 min: batch it).
- Instruments-package control went to the C ABI worker (SweepScope). Queue: FD6.3 -> migration M0..M-S (getenv own
  commit after cp14).
- FD6.3 survey (animation-laboratory, the smallest page): branch claude/fd6-pages created on 09034d31 (no commits).
  Gaps before a page can be a document with its digest unchanged: ScaledPanel/ScaledGroupBox design_size (a ctor
  argument, no property); Panel::set_background is not back_color (the Panel defect); ScaledGroupBox has no `font`
  property; font syntax has no letter spacing (group captions use 0.24); BasicControlStyle (page_style) has no property;
  CheckBox indicator_style; Command/Value bindings; data content (easing tracks) stays in code. Also the document root
  must BE the shell's page body (gallery.body.<page>, ScaledPanel 1000x660, canvas background), or every page gains a
  control and its trace moves.
- Asked the coordinator (options): (1) close the gaps now with today's PropertyDescriptors (design_size, background =
  back_color for panels, font on group boxes, letter spacing `font: control 12 700 spacing 0.24`, indicator-style,
  style stays code), then pages, generated root = page body [recommended]; (2) migration M0-M2 first, pages after with
  capabilities; (3) pages as structure-only documents, styling by ID in code.
RESUME POINTER (2026-11-03): w2 on claude/fd6-pages = 09034d31, clean. Waiting on the FD6.3 option.

## 2026-11-03: owner corrections for FD6.3; defect sweep DONE (milestone, before fixes)
- Owner: digests are change detectors, not the gate. Per page: same intent (semantic snapshot / tree, interactions),
  every visual difference explained (before/after PNG pairs in D:/gui_forms_w2/dev/fd6_review/<page>/ + one line),
  regressions fixed, digests re-recorded with the note; one local review page (plain HTML, not published).
- Coordinator decision: OPTION 1 revised (design_size, panel background = back_color, font on group boxes,
  `font: control 12 700 spacing 0.24;`, indicator_style; page style + bindings stay in code; binding syntax is a
  language follow-up; generated root may be the page body). Order: sweep list (milestone) -> fix -> gaps -> pages
  smallest first. Do NOT run sanitized UIA Gallery myself; plain UIA once per batch.
- claude/fd6-pages b149dc4f: WindowsHostOptions::scale_override (+ Gallery --scale), frame metrics follow the
  monitor. Tools: dev/tools/sweep.sh <dir> [scales] (every page via `GUI.Forms Gallery.exe --page --scale --capture`
  on the private desktop), dev/tools/sweep_page.py (writes dev/fd6_review/index.html; defect data inside the script).
- Sweep: 62 captures in dev/fd6_review/sweep/<page>_{100,200}.png; 21 defects D1-D21 in index.html (crops at both
  scales, likely fix place). 200% inspected closely on basics and control-spectrum only: same defects, none
  scale-only.
RESUME POINTER (2026-11-03): w2 on claude/fd6-pages b149dc4f, clean. Waiting on the owner's priorities for D1-D21
(and their own additions). Then fix, then the option-1 gaps, then pages smallest first.

## 2026-11-03: owner priorities; framework defect fixes + option-1 gaps (batch 1)
- Owner priorities: (1) framework D4 D5 D6 D9 D11 D12 D13 D19 (+ framework causes found on typography);
  (2) shell D1 D2 D3 D20; (3) page defects as each page converts, typography-and-inspection first.
- Typography page swept in depth: D22-D25, D27 (framework), D28 (page). index.html has before/after pairs for
  fixed defects (dev/fd6_review/after/ = captures with the fixes; FIXED dict in dev/tools/sweep_page.py).
- FIXED (framework): D4 GroupBox fills only inside its frame, frame stroked in 3 clipped pieces around the caption;
  D5 LinkLabel underline = measured advance (window text metrics); D6 ButtonBase aligned_text_block centres an
  overflowing text block; D11 styled accent Button plain while disabled; D22/D27 TypographySpecimen line boxes +
  baselines only with the adornment switch, footer = first baseline; D23-D25 VisualInspectorView tracking 2 decimals,
  elide_text_end (moved from ObjectView to basic_control_rendering), rows fit above the legend.
- TRIED AND REVERTED (diagnosis wrong, after-captures showed it): D13 themed TabPage painting nothing (notches stay);
  D12 borderless TextBox without well border (removed the up-downs' only frame; the ComboBox inner border stayed).
  Both still open, with D9 (CorrespondenceView) and D19.
- Option-1 gaps DONE: font `[italic] [spacing <n>]` (valid_font_spec checked); GroupBox `font` property; ScaledPanel/
  ScaledGroupBox `design_size` property, children without add_at slots placed by their own bounds in design
  coordinates; CheckBox/RadioButton `indicator_style` (shared enum, button_base/choice_indicator_property.hpp);
  panel background = back_color: Panel's face takes its own back_color (outside high contrast) when no
  set_background, fixing the "themed Panel overdraws back_color" defect. NO separate `background` property (tried:
  back_color is ambient, so pages converting set_background -> back-color may recolour children's grounds; judge
  per page, explained in the pairs). Test: form_generator_tests test_page_descriptors.
- Digests re-recorded (all 31, note in display_trace_tests.cpp); manifests/control-schema.json + generated docs.

## 2026-11-03 (after the shutdown): batch 1 recovery
- The uncommitted 22 files + choice_indicator_property.hpp on claude/fd6-pages b149dc4f are exactly batch 1 above
  (framework D4 D5 D6 D11 D22-D25 D27 fixes + option-1 gaps + re-recorded digests); tracked part equals
  refs/wip/w2-2026-11-03 b8560754. The pre-shutdown verify passed (50 plain / 52 san, all checks) except one gate F1
  (button_base.cpp aligned_text_block switch without default): fixed with a reasoned default.
- Re-verify running: verify.sh (headless regex, log dev/logs/verify_b1.out) then dev/tools/fg_gallery.sh (pages,
  display trace, interaction, MSAA plain + san, UIA plain, on the private desktop; dev/logs/fg_b1.out).
- Then: commit batch 1 (framework fixes / option-1 gaps / digests as separate commits where clean); report milestone.
- Tools: dev/tools/fg_gallery.sh = the Gallery-wide foreground set on the private desktop.
- BATCH 1 COMMITTED, VERIFIED on claude/fd6-pages 998e6b8b (on b149dc4f): 5e50e03d scale_override docs;
  171c59f5 framework fixes D4 D5 D6 D11 D22-D25 D27; 540d7cea option-1 gaps (+ Panel back_color, schema);
  998e6b8b digests re-recorded. verify.sh: 52 plain / 54 ASan+UBSan, style, lifetime, docs, registry, schema,
  divergences, gate (no F1/F2) clean; fg_gallery.sh on the private desktop: pages, display trace, interaction,
  policy, MSAA plain + san, UIA plain: all pass. WIP fully committed; refs/wip/w2-2026-11-03 can be dropped.
  dev/tools/hunks.py: stage chosen hunks (git diff F | hunks.py 1 2 > p; git apply --cached p).

RESUME POINTER (2026-11-03): w2 clean on claude/fd6-pages 998e6b8b (batch 1 reported as a milestone). Next:
remaining framework defects D9 (CorrespondenceView/VisualInspectorView overlap), D12 (ComboBox inner border; first
diagnosis wrong), D13 (TabPage notches; first diagnosis wrong), D19; then shell D1 D2 D3 D20; then pages smallest
first with typography-and-inspection first among pages (D28), each with before/after pairs in dev/fd6_review.
After FD6: property-capabilities migration M0..M-S (batch plan above; M0 waits for checkpoint 14 to merge: window
hold). Verify: verify.sh '<headless regex>' then dev/tools/fg_gallery.sh.

## 2026-11-03: framework defects D9 D12 D13 D19 DONE; shell D1 D2 D3 D20 in progress
- Coordinator: recovery accepted (fd6-pages 998e6b8b -> checkpoint 15), wip ref dropped. Order: framework D9 D12 D13 D19,
  shell D1 D2 D3 D20, then pages (typography-and-inspection first); every fix on the review page.
- claude/fd6-pages ecabc7c4, VERIFIED (59 plain / 61 san, all checks, gate; fg_gallery plain + san, UIA plain):
  4cafccb8 D9 CorrespondenceView card_layout (CorrespondenceCardLayout/Fonts; the plain names clashed in the docs
  tool's manual review) + Gallery ledger expanded 124; 55827b0a D12 borderless TextBox draws no frame, up-downs inset
  field + spinner inside their border; cc86af86 D13 TabControl face = header band (the square face frame made the
  notches; TabPage border default none); 4c00466a D19 rack fields 11, status 9.5, children inside the module border;
  ecabc7c4 five digests re-recorded.
- Diagnosis lessons: D13's notches came from TabControl's face (stroke_rounded_rect radius 0 over the whole control),
  found from the display trace (dev/traces/<name>/<page>.trace: ops in 1/64 units after translate); D12's first
  attempt failed because up-downs relied on the editor's frame.
- Shell (uncommitted, src/controls/gallery_pages.cpp): header pitch 16 (D1); each page in a ScrollableControl
  `<surface>.viewport` at its design size, unscaled (D2: ScaledPanel squeezed tall pages vertically); body height =
  content extent (D3); sidebar to the window bottom (D20). Tests: support.hpp select_page/catalog use
  effectively_visible; click() scrolls the target into the page viewport.
- Tools: dev/tools/cap.sh <dir> <page>... (100/200 captures), png_crop.py (crop+enlarge without PIL), redigest.py
  <table> [note] (apply a trace table to display_trace_tests.cpp), trace_at.sh <ref> <name>.

## 2026-11-03: shell DONE; typography-and-inspection converted (FD6.3 pages begin)
- Shell committed and verified: 67cf3bbf D1 D2 D3 D20, fc7d3b6e all 31 digests re-recorded (verify.sh + fg_gallery.sh
  plain + san, UIA plain: all pass). Review page FIXED has D1 D2 D3 D20 D9 D12 D13 D19.
- typography-and-inspection is a form document: demo/pages/typography-and-inspection.form, compiled by
  gui_forms_add_form(gui_forms_gallery_model ... CLASS TypographyDocument NAMESPACE gui_forms::gallery::pages);
  TypographyPage owns `TypographyDocument<TypographyPage> document_`, moves the root's children into the shell body
  (remove_child + add_child: placed by their bounds in design coordinates) and keeps Command/Values/bindings/
  ChoiceGroup items/page_style/paper background in code.
- Pattern lesson: do NOT use back-color on a page group in a form: back_color is ambient, so the group's Buttons and
  ChoiceGroup take it instead of their themed faces. Set Panel::set_background(paper) in code.
- Schema properties added (option 1): TypographySpecimen text/font/caption/wrap_text, VisualInspectorView and
  VisualInspectorOverlay target_stable_id, TrackBar tick_frequency, Label text_wrapping (manifest regenerated via
  write_schema.sh on the private desktop, then tools/control_schema.py render).
- Trace diff vs the previous: only D28 (standard centred button) and the inspector REQUESTED row (real bounds instead
  of zero). Digest re-recorded with note dev/msg/note_typo.txt; review page FIXED["D28"].

## 2026-11-03: checkpoint 14 merged (main feacf6d4); rebase + owner colour direction
- Rebased the post-FD6.1 commits (3938e122..d0979421, 17) with `git rebase --onto origin/main 1febc847` (cp14 squashed
  FD1..FD6.1 as 4bde249d). Backup ref: backup/fd6-pages-pre-cp14. Conflicts: CMakeLists (main's BUILD_INTERFACE link +
  my gui_forms_add_form block; form include dir as BUILD_INTERFACE), button.cpp (main T4 phase 4 removed the classic
  override paint; kept main's themed path + D11's `accent && effectively_enabled()`), display_trace_tests.cpp notes
  (kept both; digests re-recorded at the end). Main now requires GUI_FORMS_ENABLE_SKIA=ON for the Win32 host:
  build.sh switched to ON and the plain tree reconfigured.
- Owner direction (via coordinator): inline colour (form style blocks, back/fore colours) should not govern colour;
  colour belongs to themes with light/dark variants. For FD6.3 pages prefer theme rules (RuleSelector by stable_id /
  class / role exists in engine/theme_rules) over inline colour. The Gallery loads no program theme rules today, and a
  .form cannot reference a theme: request to coordinator = a theme reference from a .form (or a Gallery program-rule
  sheet the pages add to). typography-and-inspection still has inline fore-color on 4 labels and code paper background
  on 3 groups: to move into theme rules once that exists.

## 2026-11-03: M0 directions received (window hold LIFTED with checkpoint 14)
- M0 = the shared dirty structure, with the getenv fix as its own commit; coordinate window_lifecycle.cpp/window.cpp
  overlap with hot paths through the coordinator.
- Hot paths is fixing resize in core: Window::resize marks arrange + hit_test only (arrange_dirty_recursive damages old
  and new bounds; paint_recursive rebuilds a resized chunk); damage only the exposed strip; the Windows raster keeps its
  pixels. Pending owner decision: per-control ResizeRedraw flag (WinForms ControlStyles.ResizeRedraw).
- M0 must carry: per-control bounds-change damage, exposed-strip damage, ResizeRedraw escalation (fits the study's
  container-declared escalation).
- OWNER: resize invalidation is PER AXIS (vertical top-down, horizontal left-right). Containers/panels register per axis
  whether their contents respond (fill, flex, stretch, anchor); on a resize along an axis only those containers re-lay
  out; non-filling children are invalidated only as children of such a container or by touching a moving edge (children
  never register). Corner drag = both axes. A reallocation that keeps the partition signals nothing elsewhere (the
  collapsing ribbon). Put per-axis registration in M0's control table; agree the interface with hot paths via the
  coordinator.
- M0 sequencing: no M0 code yet. Wait for the C ABI worker's S1.2 head (SurfaceRoot holds the control table, dirty
  structure and flush); M0's dirty structure and per-axis registrations become SurfaceRoot members (property-study
  decision 11). Coordinator forwards the head.

RESUME POINTER (2026-11-03, pre-reboot): w2 clean on claude/fd6-pages f33059cb, rebased onto main feacf6d4
(backup ref backup/fd6-pages-pre-cp14 = pre-rebase d0979421). Commits since main: GroupBox caption .. FD6.2, D4-D27,
option 1, D9 D12 D13 D19, shell D1 D2 D3 D20, typography-and-inspection form (D28), digests re-recorded on cp14.
VERIFIED on f33059cb: verify.sh (16 plain + 16 ASan/UBSan, style, lifetime, docs, registry, schema, gate, divergences)
and plain private-desktop gallery set (6/6). NOT YET: the sanitizer-tree gallery set (the earlier failure was a stale
sanitizer exe, since rebuilt at 20:57) and the input_desktop MSAA/UIA gallery tests under foreground_lock (the MSAA
gallery test is now labelled input_desktop; dev/tools/fg_gallery.sh was rewritten to split by label but never ran).
Next: run `sh /d/gui_forms_w2/dev/tools/fg_gallery.sh`; if green, send the coordinator f33059cb for checkpoint 15;
then the next page conversion (smallest first; colour via theme rules per the owner, pending a .form theme reference
request to the coordinator); M0 waits for S1.2's head. build.sh now configures GUI_FORMS_ENABLE_SKIA=ON (main needs it).

## 2026-11-04 (after the reboot): resumed; theme reference design (sent to the coordinator)
- Resumed on claude/fd6-pages f33059cb (clean). Both trees rebuilt (relink only); fg_gallery.sh running.
- Note for the coordinator's queue: D9 D12 D13 D19 and the shell D1 D2 D3 D20 are already DONE and verified (see the
  2026-11-03 sections); the defect queue continues with the pages, typography first (D28 done with its conversion).
- THEME REFERENCE DESIGN (proposal; typography keeps its inline colours until approved). Facts found:
  themes are packages (theme.json + sheets, as bytes; compile_theme_package onto the built-in professional theme;
  applied by Window/ApplicationResources::set_theme); the shared syntax tree already parses `@import "<name>";`
  (SyntaxNodeKind::import_rule), and the form model counts only object rules at the top level, so a top-level
  `@import` in a .form is silently skipped today (a defect to report either way). Selectors by stable ID exist.
  The repo has no theme.json package and the Gallery sets no theme.
  Options:
  1. (recommended) `@import "<package>";` before the root, at most one: the theme package the document's
     stable-id/class rules live in. The document never applies a theme (a page shares its window); it declares a
     dependency. FormModel::theme_package; the generator emits `static constexpr std::string_view theme_package`
     on the class; the loader reports it; gui_forms_add_form(... THEME <dir>) checks at build time that the package
     exists and that its stable-id selectors name objects of the document (unknown = warning). Any other top-level
     item becomes form_root_invalid instead of skipped. Gallery side: demo/themes/gallery/ (theme.json + one sheet
     per converted page, light and dark variants per T4), embedded as bytes, compiled at start-up and set on
     ApplicationResources; pages' inline fore-color / code backgrounds move there, keyed by stable ID.
  2. `theme: "<package>";` as a root property: rejected-leaning; a theme is not a property of the root control and
     every root type would need a non-control property.
  3. No language change: only the Gallery theme package; forms address nothing. Smallest, but a .form previewed or
     loaded alone shows no colour and nothing checks that the rules still name live IDs.
- 2026-11-04: f33059cb VERIFIED in full (fg set plain + san 4/4 each; input_desktop MSAA+UIA plain 2/2, MSAA san 1/1)
  and sent for checkpoint 15 with the theme-reference options. LESSON: never wrap `ctest -L input_desktop` in
  foreground_lock.py (each test's launcher takes the lock; it was not re-entrant: deadlock 21:23). fg_gallery.sh and
  verify.sh no longer wrap anything; verify.sh takes headless regexes only. `fg_gallery.sh lock-only` = input_desktop set.
- Coordinator ruling: D28's line-box stroke (TypographySpecimen) is diagnostic inspection content (paint census 6 -> 7,
  recorded by the standby). New direct draws in ordinary controls go through theme/anatomy; a diagnostic-instrument
  exemption must be stated in the commit message.
- In progress: animation-laboratory as a form (demo/pages/animation-laboratory.form; EasingPreview gains a `title`
  schema property; tracks stay in code).
- 2026-11-04 OWNER CORRECTION (theme reference): a PROGRAM invokes a theme, not a form; NO form-language change, no
  `@import`. The Gallery ships demo/themes/gallery/ (theme.json + sheets, light + dark variants in T4's color-scheme
  syntax from claude/t4-variants, API via the coordinator), compiles it at start-up with
  ApplicationResources::compile_theme_package and selects it with ApplicationResources::set_theme; inline colours
  (typography fore-colours, group paper backgrounds, ...) move into its rules keyed by stable ID or class. Keep a
  PROGRAM-level build check: a CMake helper (gui_forms_add_theme(... FORMS <list>) or an option on gui_forms_add_form)
  that checks the package's stable-id selectors name objects of the program's .form documents. Start with typography.
- claude/fd6-pages-2 (on f33059cb), VERIFIED: 2d4a0c09 EasingPreview `title` schema property; b421fbdb
  animation-laboratory as a form (trace unchanged). Collections page next (needs ListBox selection_mode, ComboBox
  placeholder_text/drop_down_style/drop_down_filter schema properties), after the theme package work.
- 2026-11-04 OWNER RULINGS (theme package): (1) classes accepted: `classes` schema property on Control
  (`classes: "page-heading";`), Gallery theme keyed by role classes; G3: Label text and Panel/ScaledGroupBox face read
  the theme's rule facts; plus CSS backslash escapes in the selector tokenizer so `#gallery\.typography\.heading` works
  (theme-language code: coordinate with T4 via the coordinator); gui_forms_add_theme(... FORMS ...) build check stays.
  (2) Owner's style MODEL (build toward): any expression anywhere, but on the control itself discouraged; the active
  ruleset is compiled from all sources in layers: library themes; project theme; LOCAL FORM RULES (a .form's own rules
  for its objects; where a visual editor writes); persistent LOCAL CODE RULES (C++/C# custom colour = a legitimate
  inline class); true per-instance inline. Visual editor: GENERIC (theme/class) vs SPECIFIC FORM editing, never
  confused; per-instance form edits = geometry/behaviour; for appearance it asks the scope (instance / kind / class /
  everything) and stores at that layer. .form gains a form-local rules section LATER: design its place only, don't
  build. T4 writes the layer order + cascade spec.
  PLAN: (a) finish collections-and-popups (ListBox selection_mode, ComboBox placeholder_text/drop_down_style/
  drop_down_filter); (b) `classes` property; (c) G3 Label/Panel faces from rule facts; (d) tokenizer escapes (after
  T4 OK via coordinator); (e) gui_forms_add_theme + demo/themes/gallery (light; dark via T4 syntax when it lands);
  (f) typography + animation colours -> classes; (g) design note: place of the form-local rules section.
- 2026-11-04 OWNER SIMPLIFICATION (supersedes form-local rules): a .form is ONTOLOGICAL (types, IDs, nesting, geometry/
  layout, text content, items, classes, handlers, enabled/visible); NO theme items. The project theme styles it.
  Themes live in exactly two places: the project theme (on the library themes) and truly inline code. Theme visual
  editing = a special generic Gallery-miniature .form. Do NOT design form-local rules.
  Sent proposal: REFUSE kind color/font properties (new error form_presentation_property pointing to the project
  theme) rather than remove; staged: colour after G3 + Gallery theme, font after T4 admits font-* (today a W107
  left-out family). Asked: TypographySpecimen.font exempt as content (flag); keep variant enums (visual/indicator/
  border style) in .form. Awaiting answer before changing the language.
- T4 answers: `classes` property binds Control::classes()/set_classes (no new store); facts: `color` (text),
  `background`/`background-color` (faces); dark: `:root { color-scheme: light dark; }` + `@media
  (prefers-color-scheme: dark) { ... }`, prefer `role(ink-on-paper)` colour roles; escapes are on T4's branch (I don't
  touch the tokenizer); cascade: built-in < library packs < project theme < inline code. G3 Label: foreground() =
  effective_theme().resolve(*this, panel, visual_context()).recipe().text (paint_part already applies rules).

## 2026-11-04: M0 started on S1.2 (claude/subsurface-s1 d1f5aa8e from D:/gui_forms_w5; fetched as w5/subsurface-s1)
PER-AXIS RESIZE REGISTRATION INTERFACE (proposal for hot paths, via the coordinator). All SurfaceRoot members, in the
M0 control table (dense index per attached control, generation-checked):
- `enum class ResizeAxes : std::uint8_t { none = 0, horizontal = 1, vertical = 2, both = 3 };` (flags; include/gui_forms/
  layout/resize_axes/resize_axes.hpp, renderer-neutral).
- Containers register, children never do. A container's response = the axes of ITS OWN size along which its children's
  layout changes: `virtual ResizeAxes Control::layout_resize_response() const` (default: OR over children of the
  dock/anchor demand: dock fill = both, top/bottom = horizontal, left/right = vertical; anchor with right (or left+right)
  = horizontal, bottom (or top+bottom) = vertical; a child with auto/percent size along an axis = that axis). Layout
  panels (flex, grid, table, flow, ScaledPanel, ribbon) override with their engine's answer.
- `SurfaceRoot::resize_response(const Control&) const noexcept -> ResizeAxes`: the cached registration (table slot).
  Refreshed in the flush's bookkeeping when a child's layout slot changes (mark_child_layout_slot), a child is added/
  removed, or the container calls `SurfaceRoot::refresh_resize_response(Control&)` after changing its own layout mode.
- `SurfaceRoot::note_bounds_changed(Control&, Rect old_bounds, Rect new_bounds)` (called by arrange when a control's
  arranged size changes; and by Window::resize for the root): changed = axes whose extent changed. If
  resize_response & changed: mark arrange on the control (its children re-lay out); else no relayout of its children.
  Damage: ResizeRedraw (ControlStyles::resize_redraw, pending owner decision on the default) -> whole new bounds;
  otherwise only the exposed strips (new minus old, right and bottom; left/top when the origin moved) plus the
  children touching a moving edge (anchored to it). A corner drag = both axes. A reallocation that keeps the partition
  (sizes swap inside a container whose total is unchanged, e.g. the collapsing ribbon) touches only the swapped
  children; nothing is signalled to the container's ancestors.
- Counters (SurfaceRootMetrics): resize_relayouts, resize_strip_damage_area, resize_redraw_escalations.
- resize_redraw default (owner via coordinator): ON for stock controls whose paint depends on their own size (borders, materials, gradients, centred content); OFF for plain containers and the root.
- claude/fd6-pages-2 now c1484f8b (VERIFIED, same runs): + d46e257c ListBox/ComboBox schema properties, c1484f8b
  collections-and-popups form (trace unchanged). Next on FD6.3 after M0 steps: classes + G3 + Gallery theme.
- claude/m0-dirty-table (on w5/subsurface-s1 d1f5aa8e): 35c3a970 getenv read once (VERIFIED 10+10, all checks).
  M0.1 IN THE TREE (verifying): include/gui_forms/layout/resize_axes/resize_axes.hpp (ResizeAxes flags);
  include/gui_forms/surface_root/control_table/control_table.hpp + src/core/surface_root/control_table/control_table.cpp
  (detail::ControlSlot index+generation, DirtyKind layout|paint|other, DirtyBitset A2 with summary, ControlTable
  assign/release(LIFO reuse, generation++)/touch/take/resize_response); Control::surface_slot_ (+ surface_slot()
  inspection getter); SurfaceRoot::control_table_ + control_table() inspection, settle_dirty() (M0.1: drains only;
  called at ensure_layout start and both paint() overloads); attach_subtree_state lists first then assigns (rollback
  releases), detach releases; touches in mark_dirty, mark_paint_dirty, mark_child_layout_slot, mark_subtree_dirty (root
  only). tests/control_table_tests.cpp (bitset order, slots/generation/reuse, window attach/detach/re-attach/drain).
  NEXT M0.2: move the eager bookkeeping (ancestor subtree flags, suspended-layout notes, layout_dirty_/hit_test_dirty_,
  semantic generation, damage + metrics) from mark_dirty into settle_dirty; settle before every read of settled state
  (subtree_dirty(), metrics, hit_test, semantic snapshot, take_damage, paint_pending...). Then M0.3 subtree bitset +
  container escalation; M0.4 per-axis registrations + note_bounds_changed; measure Gallery + 1k/10k/100k.
- AGREED with hot paths (via coordinator): (1) escalation through `[[nodiscard]] virtual bool
  Control::paint_depends_on_size() const noexcept;` default has_style(ControlStyles::resize_redraw); Panel override:
  true with border or scroll bars; false with explicit background or style; else false when the theme material is
  size-invariant, else true; note_bounds_changed's full-bounds escalation calls it. (2) resize_redraw ON by default
  for EVERY control (custom too); containers and the root answer OFF via the override. (3) strip damage (material
  resize band, old+new for moved origins, ScrollableControl viewport rule) = hot paths' code inside
  note_bounds_changed's damage branch: leave a clear seam (one private function, e.g.
  SurfaceRoot::damage_resized(Control&, Rect old, Rect new, bool escalate)). (4) counters: resize_relayouts mine;
  resize_strip_damage_area + resize_redraw_escalations theirs. (5) host-side expose/WM_SIZE stay in hosts.
- 2026-11-04 NEW BUILD RULES (owner): build only touched targets, run those tests PLAIN; Gallery pages + display trace
  on the private desktop when pages change. NO sanitizer tree, NO full suite (the standby's batch fold verify runs
  everything). Heavy runs wrapped in `python -B D:/malkuth/projects/tools/build_slot.py -- <cmd>` (max 2 at once).
  cp16 batch holds fd6-pages-2 c1484f8b and getenv 35c3a970. M0.1 and pages continue for cp17.
- OWNER ANSWERS (.form presentation): (1) REFUSE colour + font properties in .form with form_presentation_property
  pointing to the project theme; staged: colour after G3 + the Gallery theme, font after T4's fonts. (2)
  TypographySpecimen.font exempt as content (descriptor flag). (3) visual/indicator/border style enums stay in .form
  (ontological). Not before cp16 folds; it is the next item after M0.1.
- Tools: dev/tools/build_tests.sh '<regex>' builds gui_forms_controls + the matching test targets under build_slot and
  runs them plain; verify.sh now uses it (no sanitizer); fg_gallery.sh plain tree only (+ input_desktop plain).
- M0.1 COMMITTED b8d17f0d on claude/m0-dirty-table (on 35c3a970 on S1.2), VERIFIED plain (13 tests + all checks).
  Sent for cp17. Next on M0: M0.2 (lazy bookkeeping) when M0 resumes; per-axis seam damage_resized for hot paths.
  Meanwhile pages continue on claude/fd6-pages-2 (inline colour until classes/G3, held until cp16 folds).
- fd6-pages-2: + images-and-drawing form (trace unchanged), VERIFIED plain. build_tests.sh filters test names to ninja targets.
- fd6-pages-2: + range schema props + ranges-and-progress form (trace unchanged), VERIFIED plain. Note: run write_schema BEFORE fg_gallery when properties change.

## RESUME POINTER (2026-11-04, owner pause after the cp16 landing phase)
Workspace D:/gui_forms_w2 (clone D:/gui_forms_w2/gui_forms). All branches are local and the trees are clean.
- `claude/fd6-pages-2` 49768897, VERIFIED plain (touched targets; Gallery pages + display trace on the private
  desktop, rerun after write_schema). c1484f8b is in cp16. New for cp17 and NOT yet reported to the coordinator:
  059c6e06 (images-and-drawing form), 04670b7a (TrackBar/ProgressBar visual_style, ScrollBar large_change
  schema properties), 49768897 (ranges-and-progress form). Converted forms so far: typography, animation,
  collections, images, ranges.
- `claude/m0-dirty-table` b8d17f0d (M0.1: ControlTable, ControlSlot, DirtyBitset, settle_dirty drains only),
  reported for cp17; on 35c3a970 (getenv fix, in cp16) on S1.2 d1f5aa8e.
- `claude/fd6-pages` f33059cb is in cp15.
- Next, in order:
  1. Report the three page heads above for cp17. Once cp16 folds, rebase and delete the contained branches.
  2. After cp16: the form_presentation_property refusal, staged (colour once G3 and the Gallery theme land,
     font after T4); the `classes` schema property; G3 (Label foreground and the Panel/ScaledGroupBox face
     read the rules); the demo/themes/gallery package with gui_forms_add_theme(... FORMS ...), typography
     first; the TypographySpecimen.font content flag.
  3. M0.2 to M0.4 as in the 2026-11-04 M0 section, then measure the Gallery and 1k/10k/100k.
  4. Continue the page conversions, smallest first; next is retained-layout-panels (186 lines). Run
     write_schema.sh BEFORE fg_gallery.sh whenever schema properties change.
- Build rules: touched targets plain only (dev/tools/build_tests.sh via verify.sh), under build_slot.py.
  No sanitizer tree and no full suite. Never wrap `ctest -L input_desktop` in foreground_lock.py.

## RESUME POINTER (2026-11-05, pre-restart; supersedes the one above)
Workspace D:/gui_forms_w2, branch claude/m0-settle (local only, clean):
- 7293d367 M0.2 (in cp17). 6ce0568d M0.3 (cp18; keep on top of 7293d367). 6775fceb lab at 100k (write 26-29 ns at
  every depth). b617c369 WIP M1 + docs commit after it: VERIFIED (42 tests plain + all checks), so "WIP" is in name
  only; reword or squash before reporting. The M1 head is not yet reported to the coordinator.
- M1 as built and the P-8 options and family split plan are in PROPERTY_CAPABILITIES_STUDY.md sections 11-13 (UNCOMMITTED in projects: the file also holds w8's uncommitted section 10).
- Owner decision: reduced M0.4 (option 1). layout_resize_response is cached in the table and used only for damage
  (damage_resized) and the resize_relayouts counter; arrange() is never skipped. Order: report M1, then reduced M0.4,
  then the Gallery measurement, then M2 (Control family + the Panel back_color-when-themed defect, plus the getter
  switch script).
- fd6-pages-2 and m0-dirty-table have been deleted (contained in cp16, merged to main).

## RESUME 2026-11-05 (after restart): M1 landable, reduced M0.4 in verify
- OWNER P-8 = 1b, recorded in study section 12 (projects 94eaff3): coerce_<property>(Value&) before the store on the
  few coercible properties (Text, Font, ... listed per family), compared again after coercion; on_<property>_changed()
  after store+touch where WinForms has OnXChanged, base raises the event. Build into capability bases in M2/M3.
- claude/m0-settle: M1 reworded (fadf3b9c, tree equal to b617c369) + docs 0fdab0d7 (= old 2ffd4b5e tree). Reported
  landable to the coordinator.
- Reduced M0.4 IN THE TREE (verifying): virtual Control::layout_resize_response() (dock/anchor default, RTL mirrored);
  overrides answering both with children: FlexPanel, GridPanel, FlowLayoutPanelCompat, TableLayoutPanelCompat,
  ScaledPanel, ScrollableControl with auto_scroll. Asked in arrange_dirty_recursive when the arranged size changes
  (no stale-tracking hooks: arrange is never skipped, so it costs one children pass arrange already made), stored via
  ControlTable::set_resize_response; MetricsSnapshot::resize_relayouts counts relaid axes after the first arrange;
  damage_resized gains `ResizeAxes relaid` (unused by the policy: children damage themselves). Test:
  control_table_tests test_resize_registrations. Edit script dev/tools/m04_edit.py.
- Known gap: other controls that place children by their own arrange (ribbon, strips, docking, split containers)
  still answer the dock/anchor default; they get overrides as their families migrate.
- Next: commit M0.4, Gallery measurement, then M2 (Control family + Panel back_color-when-themed + getter script), M3.
- M0.4 VERIFIED abc63a4a (14 touched tests plain, all checks, gate clean after the dock switch moved to the compliant
  child_resize_demand form). 82bd7497 Gallery dirty lab + capability lab fix (lab_property_table).
- MEASURED (Windows, plain Release): invalidate depth 1/8/32 = 17.8/17.6/18.2 ns paint, 17.2/17.4/17.6 ns measure
  (study baseline before M0.2: 70/161/568 and 741/897/1414). Flat N=100k all marked: 203 ns/mark, flush 19.0 ms;
  N=100k D=1000: 46.6 ns, flush 64 us. Gallery (31 pages, ~1400 attached controls, depth 10): paint sweep of every
  control + settle 363-649 us, measure sweep 231-356 us, one write + settle 0.2-0.4 us, three resize steps 193-359 us
  with 27-171 arranged and 15 resize_relayouts. Logs: dev/logs/gallery_dirty.txt, lab_dirty.txt. The paint sweep
  grows with page order (pages stay attached after selection); repeating one page's sweep 40x does not grow.
- Build scripts export USERPROFILE/PROCESSOR_ARCHITECTURE/GUI_FORMS_BUILD_PRIORITY=critical after env.sh (slot 0).
- Study section 14 (G1-G14 conventions + G7 Component owner) recorded and pushed.
- M2 IN PROGRESS in worktree D:/gui_forms_w2/m2wt (branch claude/m2-control-family on abc63a4a; rebase onto 82bd7497):
  M2a = PropertyEntry thunks take Component&, Component::property_table virtual, free entry operations
  (src/core/binding/property_table/property_table.cpp, needs CMake), override_property_entries, Control branches use
  them. Still to do in M2a: PropertyView, probe test G4 convention + thread test, CMake source. Then M2b = Control's
  capabilities (assign shape study 14) + getter switch (projects tools/property_getter_switch.py) + Panel back_color.

## STATE 2026-11-05 (later): M2 a-c verified, M2d in the tree
- Stack in D:/gui_forms_w2/gui_forms on origin/main ebb9fbf4: 4c107777 M2a, f39ad211 M2b, b47cf3b6 M2c (no
  forwarders: set_x/x() deleted, every caller rewritten to cell writes/reads; -Werror=self-assign and
  self-assign-field beside dangling-gsl; DockSite cursor self-assign fixed). Verified at b47cf3b6: 115 headless tests.
- Decisions in force (study section 15): read() returns the declared type; type() returns PropertyType;
  as_type(PropertyType) -> optional<BindingValue> through convert_to_property_type (= convert_property_value);
  as_type(types::x) total pairs only (requires-constrained, so a partial pair does not compile); tags in
  `namespace types` (stock ones in binding/property_type, enum tags beside their enums); no == or conversion on cells.
- M2d (uncommitted, building): .type() -> .read() everywhere (compiler-driven loop dev/tools/m2_read_loop.sh),
  property_type.hpp/.cpp + type_tag.hpp, enum traits for Control (control_capabilities.hpp, defined in control.cpp;
  cursor as cursor_kind with inherit=0), test_property_type in tests/property_capability_tests.cpp. Edit scripts
  dev/tools/m2d_apply.py, m2d_test_apply.py.
- Next: verify M2d (m2_build.sh all, m2_test.sh headless, docs/inventory regenerate, house style, gate), commit,
  report; then the codemod tools/property_codemod.py + tools/property_families/control.json (tell families-B), M3
  ButtonBase through it (report at once), then ScrollableControl G9.
- Never run native/foreground tests on the real desktop (exclude windows_*, cursor_interaction, accessibility, msaa, uia).
- 2026-11-05 later: M2d VERIFIED 84ec483f; codemod 68b00859 (tools/property_codemod.py check/generate/rewrite/audit/
  tests; property_cell_entry thunks; control.json + shared.json); M3 ButtonBase VERIFIED cbe8202c (482 headless tests,
  schema byte-equal). Both reported. Proposed extra config keys (base, includes, table_source, test, prepare,
  written_effects, traits, sample, serialization) await the coordinator's yes. Hook parameter is `incoming`.
- Codemod run recipe (from gui_forms): generate CONFIG; audit CONFIG (fix findings); rewrite --log D:/gui_forms_w2/build.log
  --build-dir D:/gui_forms_w2/.build/native-windows-x64/gui-forms --build-command "sh /d/gui_forms_w2/dev/tools/m2_build.sh all 0"
  CONFIG; fix docstrings/manual.json stale methods; tests; sh dev/tools/m2d_verify.sh.
- Next: ScrollableControl G9 (auto_scroll etc., on_auto_scroll_changed/coerce_auto_scroll).

## RESUME POINTER (2026-11-05, reboot)
- Stack in D:/gui_forms_w2/gui_forms on origin/main ebb9fbf4: 4c107777 M2a, f39ad211 M2b, b47cf3b6 M2c, 84ec483f M2d
  (cp18), 68b00859 codemod, cbe8202c M3 ButtonBase (cp19 candidates, reported), then the G9 WIP commit (HEAD, verified: 482/482, docs, registry, house style clean).
- G9 state: builds clean (whole tree, -Werror=self-assign); docs regenerated after manual.json/docstring fixes; the
  verification (sh /d/gui_forms_w2/dev/tools/m2d_verify.sh -> dev/logs/g9_verify.txt) passed after the reboot.
  Commit reworded to "G9: ..."; reported.
- Then justify_self/align_self (not started): enum class SelfAlignment : std::uint8_t { automatic, start, center,
  end, stretch } beside DockStyle in control_layout_styles.hpp with tags types::self_alignment; two cells default
  automatic in tools/property_families/control.json (regenerate control_capabilities.hpp); hand descriptors and table
  entries in control.cpp (category Layout, bindable, effects like margin: measure|arrange on the parent); traits in
  control_property_traits; own commit with regenerated control-schema.json (owner-approved addition). After the
  cp18 rebase (classes moved into the table) control_property_count becomes 25.
- Rebase onto cp18 when told: expect the owner's overlay ownership/~Control table-slot change in M0 table code.
- Study: 15.2 extra keys accepted and recorded (606b69d); 15.4 child-property review (5638095, amended fc1ab36:
  RuntimeId keys, child_leaving). Codemod gains "child": true when the effects lane's first child table lands.
- Never run native/foreground tests on the real desktop.

## RESUME POINTER (2026-11-05, Mac resume; w2 = ~/Developer/gui_forms_workers/w2)
- Branch claude/m2-control-family (local, from backup ref) REBASED onto cp18 head dfa4f9b5 (cp18 holds M2a-M2d as
  6c56bf23..77732184). Stack: 53ca3d63 codemod, 678be61a M3, 194d169c G9 (docs regenerated per commit for cp18's
  :cpp:member: property links; M3 conflict in anchored-overlays test resolved to expanded_state.read()).
- Linux/other-only callers fixed (folded into M3/G9): src/host/linux/services/linux_dialogs.cpp Button font cell,
  tests/linux_accessibility_gallery_session.hpp text.read(), examples/reference/multiline_scrollbars.cpp
  auto_scroll cell. NOT migrated (not built by CI's main jobs; reported): file_manager_demoboard has ~108 M2-era
  setter calls plus set_expanded_state/expanded_state() (M3).
- All targets build on the M4 at G9 (ninja -k 0, rc 0).
- justify_self/align_self IN THE TREE (uncommitted): SelfAlignment in control_layout_styles.hpp, traits,
  control.json (+regenerated capabilities), control.cpp descriptors/table/hooks (bindable=false like the other
  Layout properties; effects as margin), control_property_count 25, test_self_alignment in
  property_capability_tests, Control.rst. GridPanel still reads GridItemLayout (not wired; documented).
  Edit script dev/tools/selfalign_edit.py. Next: schema render (pull manifests/control-schema.json back by scp
  from m4mini:Developer/Projects/gui_forms_workers/w2/gui_forms/), docs regen, verify, commit, report.
- Tools: dev/tools/verify.py (steps all, codemod, build, san, gate, style, docs, schema).
- 2026-11-05 later (Mac): justify_self/align_self COMMITTED b8da9ad2 on G9 194d169c (branch
  claude/m2-control-family, on cp18 dfa4f9b5). Verified at b8da9ad2: all targets build; 32 touched tests plain +
  sanitized (gallery-wide incl. AX client); style, docs, registry, schema, codemod clean. Gate: 2 F1 findings in
  tests/macos_message_dialog_key_tests.mm:225/236 (not touched by this stack; pre-existing vs origin/main=cp17).
  Digests re-recorded: states-and-diagnostics 0xfeac8e386e2c0970, values-and-spinners 0x097e47bc26908f3b.
  Schema rewrite recipe: GUI_FORMS_CONTROL_SCHEMA_WRITE=1 ctest -R ^gui_forms_gallery_pages_tests$ then
  control_schema.py render, scp files back. Trace digests: GUI_FORMS_GALLERY_TRACE_PRINT=1
  gui_forms_gallery_pages_tests --display-trace.
- Reported to the coordinator. Next: rebase when cp18 lands (expect no conflict: stack sits on dfa4f9b5); then
  GridPanel reading justify_self/align_self is the effects lane's (attached properties / spans) unless reassigned.
- 2026-11-05 (cp18 merged, main 4ab9a107; cp18b = origin/checkpoint/2026-11-05-3 54f8d101, demoboard M2 migration):
  stack REBASED onto 54f8d101: d426d2b3 codemod, ad48f590 M3 (now also migrates file_manager_demoboard incl. the
  installed consumer, + its missed accessible_description read), 7751be80 G9, ba3b287a justify/align (HEAD).
  Programs configuration (gui_forms/.programs_build.sh, untracked + .git/info/exclude; copy in dev/tools/
  programs_build.sh; tree .build/native-macos-arm64/gui-forms-programs on the M4) builds clean with ninja -k 0;
  demoboard tests 2/2. Reported. Next: rebase when cp18b lands.

## RESUME POINTER (2026-11-05, app restart)
- w2 clean on claude/m2-control-family ba3b287a (local only), on cp18b 54f8d101, which is now merged into main
  9e669353. Stack: d426d2b3 codemod, ad48f590 M3 (+demoboard), 7751be80 G9, ba3b287a justify_self/align_self;
  all reported to the coordinator.
- Next: rebase onto origin/main when the coordinator says so (expect no conflict); then propose the in-place
  collections shape (mine to propose, to the coordinator as numbered options with a recommendation).

## RESUME POINTER (2026-11-05, owner pause during P-5)
- cp19 holds my ba3b287a stack. New branch claude/p5-data-grid-view (local) off ba3b287a: 87d51273 "P-5:
  DataGridView's properties on capabilities, through the codemod" (35 cells, table null like M3 so schema
  unchanged; hand hooks beside the old setters; types moved to data_grid_view_types.hpp; config
  tools/property_families/data_grid_view.json built by dev/tools/dgv_config.py, hooks by dev/tools/dgv_hooks.py).
  All targets build on the M4 (ninja -k 0 clean); docs --check, house style, lifetime clean locally.
  Verification run (dev/verify_p5.txt: touched + gallery-wide plain/sanitized, gate, docs, schema, codemod, then
  programs build) was in flight at the pause; see that file for its result.
- Rewrite recipe on the Mac: rewrite --log ../dev/rw.log --build-dir ../.build/native-macos-arm64/gui-forms
  --build-command "sh ../dev/tools/rw_build.sh" (the script maps the /Volumes/Storage tree's paths).
- Next: finish P-5 verification (fix findings), report to the coordinator; rebase onto main once cp19 merges; then
  the in-place collections proposal. Draft options: (1) whole-value cell only, add/remove as copy+write;
  (2) a collection cell with named in-place edits (assign/insert/erase/replace/clear) through one hook pair
  taking a CollectionEdit variant, so controls remap selection and damage only the changed region (RECOMMENDED);
  (3) a scoped edit() guard with diff on commit (rejected: no edit accessor, throwing commit in a destructor);
  (4) status quo methods plus a read-only collection table kind. Per-element fields (column width) as views
  keyed by stable identity (column name / RowId), not index.
- Pause result at 87d51273: plain 17/17 touched + gallery-wide; programs build clean, demoboard 2/2; SANITIZED
  16/17: gui_forms_macos_accessibility_client_gallery_tests failed "the app wrote no semantic dump" (17.5 s;
  passed plain). Undiagnosed, not accepted as a flake: first step on resume is rerun it alone sanitized with
  M4WS_WAIT_IDLE=180 and read dev/logs/v-san-1105-191254.log. gate/docs/schema/codemod steps not yet run on the M4.
- Coordinator 2026-11-05: the sanitized AX gallery client failure is the known drive issue (sanitize trees on the
  external drive block launched apps; launched-app macOS tests run plain only, and it passes plain), not P-5.
  On resume (Sunday): run only gate, schema and codemod (verify.py --only gate,schema,codemod), then report P-5
  87d51273. Paused until then.

## RESUME POINTER (2026-11-07, P-5 reported)
- claude/p5-data-grid-view c2501119 on origin/main 00c78e6c (cp19 merged). Verified: all targets build (M4,
  ninja -k 0); touched tests plain (17 incl. gallery-wide at d10d48ce, 3 at c2501119); gate, schema, codemod,
  house style, docs check clean. Sanitized launched-app tests excluded (verify.py san step now passes
  --skip-labels 'area_host|area_a11y|gallery_accessibility|foreground').
- In-place collections proposal sent with the P-5 report (options 1-4, recommendation 2). Next: the
  coordinator's answer; then implement the chosen shape on one family (ListBox items) as the pilot.

## DESIGN (owner accepted 2026-11-07): in-place collections (cp21, after the Panel family)
- API (seal shape): CollectionCell: items.read() (span), items[i]; items = v; insert(i, x); erase(i, n);
  replace(i, x); clear(). One hook pair prepare_edit(cell, CollectionEdit&) / written(cell, const CollectionEdit&);
  CollectionEdit = variant {assign, insert{index,count}, erase{index,count}, replace{index}} behind named
  factories and queries; a refused edit throws from prepare_edit and changes nothing.
- Mechanism (owner): each edit feeds a per-collection dirty structure settled at flush like DirtyBitset:
  replace(i) sets bit i (two-level bitset, DirtyBitset's summary/leaf shape); insert/erase lower a structural
  watermark (every index >= i dirty, shifted); assign/clear set it to 0. Edits in one update scope coalesce
  (lowest watermark wins, bits OR). At flush the control intersects with its viewport: a structural edit above
  the viewport repaints the visible rows, below it only the scroll extent changes.
- Selection/current by stable ID where the collection has one, else remapped through the edit description.
  Variable-height rows: Fenwick/prefix sums of heights updated from i in O(log n). Visible window + watermark +
  bits = the content region's redraw payload (M-R/S7).
- Per-element fields: views keyed by stable identity (column_width[name] = 40), never index.
- Pilot: ListBox items, then DataGridView rows. Report milestones.

## RESUME POINTER (2026-11-07, collections pilot in progress)
- claude/codemod-initializer-fix d5f4b26a on origin/main 00c78e6c (worktree w2/codemod-fix): delete_members
  now ends a field with a statement scanner (braces/quotes/comments aware; TokenBox delimiters_{",;"} defect),
  generate refuses a header whose class body still names a migrated field, `selftest` runs the fixture
  tools/property_codemod_fixtures/field_initializers{,.expected}.hpp. verify.py codemod step runs selftest.
- claude/collections-pilot 8c08863a on a894e1ca (Panel family), milestone 1 (ListBox) committed and verified:
  34 tests plain gallery-wide, 32 sanitized, gate, style, docs, schema; programs tree + demoboard 2/2. Core binding/collection_cell (CollectionEdit factories +
  remap, CollectionDirty = DirtyBitset bits + watermark, CollectionCell with prepare_edit/written via the
  capability, take_dirty protected on the capability); controls/panel/text_items_capability (TextItemsCapability,
  GUI_FORMS_LOCALIZABLE(items)); ListBox: items cell replaces items()/set_items/add_item/remove_item/clear_items;
  prepare_edit builds edit_selection_ (only allocation), written swaps it in and remaps active/anchor/hover,
  invalidates arrange|semantics (+measure when structural, +paint when selection/top moved); ListBox::arrange
  settles: settle_items_damage intersects with [top, top+visible+1) and invalidate(Rect)s rows (in_layout_
  settles immediately). CheckedListBox: on_items_changing reserves, on_items_changed (noexcept) applies states.
  Localizable plugin collects args of member calls on annotated fields. api_docs_rst CELL_FIELD knows
  CollectionCell. Docs regenerated (ListBox/CheckedListBox rst, manual.json entries removed).
  New test tests/collection_cell_tests.cpp. Callers rewritten by dev/tools/collection_rewrite.py from build errors.
- Deviation to report: prepare_edit takes `const CollectionEdit&` plus `std::span<Element> incoming` (values
  coercible, shape not); no append (insert(size, x)); `items = {}` is ambiguous (use clear()).
- claude/collections-pilot 561ee6a2: + append(x)/append(vector) (coordinator: accepted shape, append added).
- claude/collections-dgv ff6d4410 = collections-pilot 561ee6a2 + merge of claude/p5-data-grid-view (44b2aa7c)
  + DGV rows on the cell (milestone 2): DataGridViewRowsCapability (data_grid_view_rows/), hooks in
  src/controls/data_grid_view/data_grid_view_rows.cpp, settle in arrange (whole when window/viewport/offset moved
  or sized_by_cells), owner ops (row_elements/mark_row_replaced/mark_all_rows/store_rows) for cell values, columns,
  sort; cell reserves before prepare_edit and refuses reentrant edits (revision). Verified: 23 tests plain
  gallery-wide, 21 sanitized, all targets, gate, style, docs, schema, programs + demoboard 2/2.
- Follow-ups noted: set_cell_value still calls values_changed (whole paint) for unbound rows; the row mark is
  recorded but values_changed's paint wins; ComboBox/DomainUpDown items not yet on the cell.
- Next: GroupBox/ScaledPanel/ScaledGroupBox families.

## RESUME POINTER (2026-11-07, Panel family reported)
- claude/p4-panel-family a894e1ca on origin/main 00c78e6c (independent of P-5 c2501119): Panel border_style
  (table entry replaces the registration) + visual_role cells; background/style stay methods (override getters,
  as M3). BorderStyle -> controls/panel/border_style/border_style.hpp. Touches token_box.cpp and list_view.cpp
  (families B's) with one-line border_style writes. Verified: 33 tests plain (gallery-wide), 31 sanitized
  (launched-app labels skipped), gate, docs, schema, codemod, programs build + demoboard 2/2.
- LESSON: the plain M4 tree lacks the demo labs (vsync_lab, visual_inspector_lab, correspondence); build the
  gallery tree (ci_build --gallery) or the programs tree before calling a codemod rewrite complete.
- Next: the collections pilot (DESIGN above): ListBox items, then DataGridView rows, unless the coordinator
  wants GroupBox/ScaledPanel/ScaledGroupBox first.

## RESUME POINTER (2026-11-08, collections milestone 3)
- Owner decision (via coordinator): property families (GroupBox/ScaledPanel/ScaledGroupBox) go to a code-only
  coder; do NOT start them. W keeps collections. FD-6's binding build waits for the app/window study's cp21 plan.
- claude/collections-dgv 37449333 (on ff6d4410): DataGridView::row_values_changed() (parts.cpp): a value stored
  into an unbound row (store_value marks it replaced) invalidates arrange|semantics only, unless sized_by_cells
  or not unbound (then values_changed); used by set_cell_value (not the new row), commit_edit (stored), the
  combo-cell choice and the check toggle. Test in collection_cell_tests (damage height <= one row). Verified:
  16 tests plain gallery-wide, 14 sanitized, style.
- claude/collections-combo a1389f38 (on 37449333), verified 43 plain gallery-wide / 41 sanitized, gate, style, docs, schema, programs build + demoboard 2/2: ComboBox and DomainUpDown derive TextItemsCapability; items()/
  set_items/add_item/remove_item_at/clear_items removed. ComboBox prepare_edit validates UTF-8, remaps the
  selection (edit_selection_) and copies the selected text; written rebuilds the auto-complete source, then
  (editor) refresh rows + match text, or (list style) swaps the selection, raising index, text, items changes.
  DomainUpDown written sorts (TextItemsCapability::sort_items: mark_all + std::sort, no allocation) and
  re-matches. Callers (tests, gallery pages, demoboard, linux_dialogs) rewritten by collection_rewrite.py
  (extended for ComboBox/DomainUpDown, add_item -> append, remove_item_at -> erase). Docs: ComboBox/DomainUpDown
  rst, manual.json entries, docs/user/localizable-literals.md. Test test_choice_items_follow_edits.
- Next: report; await coordinator (FD-6 binding waits for the cp21 plan).

## RESUME POINTER (2026-11-08, Part A steps 1-2 done; supersedes the one above)
- Task: cp21 Part A of desks/coordinator/APP_WINDOW_RULES_DESIGN.md (Application instance).
- Branch claude/app-create in w2/gui_forms, rebased on origin/main d722217c, head b8027bbd:
  2dced0b9 gate + cp20 baseline; 46130205 step 1 (create/current/close, joined windows);
  be3ed7ec T4 theme base API (w1 3c149281, linearized by the rebase); ece835b3 create on T4's API + docs;
  b8027bbd step 2 (application presentation base, PresentationOverrides copy-on-write per window,
  host reports routed to the application, Application::set_theme reaching joined windows,
  creation gate --joined cases + baseline).
- Verified: touched tests plain (29) + sanitized (25), programs tree (gallery, demoboard + its tests),
  creation gate check all six cases ok (allocations equal; bare_window_joined 64 < 66), docs check,
  style/lifetime, state-machine gate --changed-from origin/main clean.
- Decisions in force: run(Window&) lands with Part B (coordinator); C ABI table is w5's (stacked on e5a99786,
  now ece835b3 after rebase - tell w5 via coordinator).
- M4 helper scripts (untracked, excluded): .creation_record.sh, .creation_check.sh, .programs_build.sh, .step2_final.sh.
- Next: whatever the coordinator assigns (rest of Part A / Part B).

## RESUME POINTER (2026-11-08, Part C foundations committed; supersedes the one above)
- Task (coordinator): Part C foundations = APP_WINDOW_RULES_DESIGN C.2 + FD-6 section 5: per-window
  InteractionState and shared gesture recognizer; no rule engine yet; creation gate flat.
- Branch claude/interaction-state (on claude/app-create b8027bbd), see git log for head.
  Files: include/gui_forms/window/interaction/{interaction_types,interaction_snapshot}.hpp,
  src/core/window/interaction/{interaction_state.hpp,.cpp,window_interaction.cpp},
  GestureSettings in window/presentation/input_settings.hpp (+ PresentationSettings::gestures,
  HostInputSettingsEvent::gestures), Control::on_interaction / accepts_drag_gestures,
  dispatch_pointer split into route_pointer + recognizer step, tests/interaction_state_tests.cpp.
- Decisions taken (reported to the coordinator): click = primary release over the pressed control
  (slop governs double-click and hold only; drag only on controls accepting drags, so Buttons keep
  clicking after wandering); hold does not cancel click; secondary: press, release, context_request
  (edge reported by host, default release); middle: press/release only; one gesture at a time.
- Verified: whole plain tree builds; 56 tests plain + 51 sanitized incl. gallery-wide; gate, style,
  docs, schema ok; creation gate all six cases ok (allocations unchanged).
- Next (cp22/23 per plan): remaining click-detecting controls onto the recognizer (cp25), rule engine (cp24).
  Hosts do not yet report slop/drag/hold (Windows sibling: SM_CXDOUBLECLK, SM_CXDRAG).

## RESUME POINTER (2026-11-08, mouse-event rework; supersedes the one above)

Owner's mouse-event rule replaced the recognizer. claude/interaction-state (base fdde9f5b) carries the
rework, uncommitted until the M4 run passes: Control's WinForms events (lazy MouseEvents record),
host click counts, HostContextRequestEvent/HostHoverEvent, PointerSettings, protocol 11,
KeyCommand::context_menu removed, macOS menuForEvent/Control-click/AX show-menu, Linux
linux_input_rules (click count, emulated context keys) + XSettings distance/drag threshold, Windows
WM_CONTEXTMENU/WM_MOUSEHOVER/metrics. Tests: tests/mouse_events_tests.cpp,
tests/linux_input_rules_tests.cpp. Notes rewritten (uncommitted, coordinator commits):
desks/coordinator/APP_WINDOW_RULES_DESIGN.md Part C (C.7 now 8.5 U, total 29.5) and
FD6_BINDINGS_DESIGN.md R5/§5/Q2/Q4/plan. Next: verify (plain+sanitized touched tests, gate,
docs, schema, creation check), commit, report. Linux code needs the Linux CI job; Windows code the
Windows sibling.
Update: committed d78199a0 on claude/interaction-state, registered in INTEGRATION_QUEUE as unverified
(owner: nothing waits on builds). Next: whatever the integrator returns.
Update: ef378c38 (Click on any button; ButtonBase primary only) and 38a6eaa1 (ObjectView/CorrespondenceView/
PageStrip menus from the host's context request) on claude/interaction-state, queued unverified. w8 rebases
its rule hooks over ef378c38. Next: integrator returns, then the rule engine (cp24) when assigned.
Update (2026-11-08): 1f007c58 on claude/interaction-state (DoubleClick needs standard_click too), queued.
P-4 remaining families: none left for w2. w6's 9b947eec (M-S3..M-S7 + C-8) covers every family on W's
list (buttons, ranges, EasingPreview, layout panels, VisualInspectorView/Overlay). The classes still
without a config have no properties (Toast: content and action fixed at construction; UserControl;
SubSurfaceHost: set_content is an ownership method; CommandBar: items is a replace-list collection;
H/VScrollBar: nothing of their own). No claude/p4-remaining branch was made.

## RESUME POINTER (2026-11-08, cp22 items P-5 and R-8; supersedes the ones above)
- claude/p5-content-regions c8737231 on 7e317d4e (w8 ms8 tip; merges cleanly with w6 9b947eec): core ContentRegion
  (include/gui_forms/control/content_region/), SurfaceRoot gathers at the end of ensure_layout, Control::redraw_all
  folds the subtree, detach folds; DataGridView cell_changed notes cells (no arrange), scrolls are redraw_all.
  Tests: control_table test_content_regions, collection_cell test_grid_cell_region. Queued unverified.
- claude/r8-resize-damage 890f5e11 on 7e317d4e: ScrollableControl clears resize_redraw (bar-strip repaint on
  resize), TabControl/UserControl set it again; resize_damage_equivalence scrolling-list case. Queued unverified.
  Finding: the Gallery's full-window paint per size step is its ScaledPanel shell (proportional relayout of every
  child), not invalidation; changing it is a Gallery layout choice (offered to the coordinator).
- claude/interaction-state 1f007c58 (mouse events) queued; P-4 remaining families: none (w6 9b947eec covers them).
- 2026-11-08: e629ad91 on claude/r8-resize-damage (Gallery shell docked/anchored), queued. Next item (Gallery
  init rewrite on Parts A/B/K, FD-6, rules, FD-5) is WAITING on a base decision sent to the coordinator: no branch
  has e629ad91 + w7 c2b46eb4 (A+B+K) + w8 rules + w6 rule migration. Entry points: demo/gallery.cpp make_gallery,
  src/host/*/gallery_main.*, src/controls/gallery_pages.cpp build_gallery_shell.

## RESUME POINTER (2026-11-08, Gallery init rewrite; supersedes the ones above)
- Branch claude/gallery-init @ c64cdee2 (on e629ad91) in w2: the Gallery initialization rewrite, queued unverified. Written against w7 c2b46eb4 (A/B/K, FD-6 a3756eaa), w8 389ff85e (rules), w6 67ad80bb (migration); does not compile on its own base.
- Decisions in force: no load_window (coordinator: stays internal until stage D); generated Window subclass + generated page document classes only; model owned through the root Tag (PageContext -> GalleryContext); page plug-in point pages::page_builders.
- Next: when the integrator announces the cp22 candidate, rebase claude/gallery-init onto it (rebase notes in the queue entry). Do not merge other heads.
- Earlier heads still queued: claude/interaction-state 1f007c58, claude/p5-content-regions c8737231, claude/r8-resize-damage e629ad91.

## Gallery stack (2026-11-08)

- Branch claude/gallery-stack @ 5b6f0235 on 37451ea6 (batch9 base), workspace ~/Developer/gui_forms_workers/w2/gui_forms. It holds my rewrite, w8's panes and w7's spinner/ring plus fixes. Registered in INTEGRATION_QUEUE; I own the Gallery stack and the integrator drops 96562665..13ee18cb.
- Decision in force: the Gallery uses generated Window subclasses and generated page classes only (no load_window until stage D). The form pages are a separate document (demo/gallery_form_pages.form) initialized from the model in make_gallery, and each control is attached once.
- Open: the value-editors rack projection failure (base) and the display-trace digests to re-pin at the checkpoint; schema.md to regenerate at the fold.
- Next: rebase when the coordinator lands the checkpoint; then sanitizer and checkers on the stack's touched tests if asked.
- 2026-11-08 owner bugs fixed on claude/gallery-stack @ 532be98d (registered):
  - anchor references for hidden children, the root of the unreachable page ends;
  - precise wheel deltas;
  - the input log view.
  Stack then rebased onto batch9 4d020751 with T4 5b99547a merged: head 8d4a3d75, then shell grounds as theme classes, T4 2d8b20e7 merged, w8 0373969c cherry-picked: head e86819ad; owner .app rebuilt from 80b1d61a.
