# Verify PR #350 on the M4 (macOS accessibility value refresh)

From the GUI.Forms coordinating session, 2026-10-26. This is a read-and-run task: change no code and push nothing.

**The bug you found:** in the fixture app, the `metrics_view` value macOS published was one paint behind the semantic tree (41 against 42).

**The cause found:** the defect is in MetricsView, not the host cache. MetricsView computed its accessible value from the live counters on every query. Those counters change with every paint without advancing the semantic generation. The macOS host rebuilds its tree as soon as an invalidation arrives, before the paint in `drawRect:`, and the paint then moves the counter on by one.

**The fix (#350, branch `claude/macos-ax-value-refresh`, head 1c7a6d9e):** MetricsView publishes the text it last painted, which is what the card shows. A paint that changes that text marks the semantics dirty, which advances the generation. The host is unchanged.

## Steps

In `~/Developer/CodexBuilds/gui_forms-t5`, or any clean clone:

1. `git fetch origin && git checkout --detach origin/claude/macos-ax-value-refresh` (expect 1c7a6d9e or later).
2. Build the way you did for the bench run: `python3 gui_forms/tools/ci_build.py --jobs 8`. If the tree is already built, rebuild just the test: `cmake --build .build/native-macos-arm64/gui-forms --target gui_forms_macos_accessibility_client_tests`.
3. Run it five times: `for i in 1 2 3 4 5; do ctest --test-dir .build/native-macos-arm64/gui-forms -R '^gui_forms_macos_accessibility_client_tests$' --output-on-failure; done`.
4. Optional but valuable: confirm on `origin/main` that the test still fails, so the fix is what changed.
5. Optional: run the bisect that shows what exposed it, `git bisect start origin/main addcf238` and then `git bisect run` with the same ctest command. The suspects are 2dd53556 (the DockSite live region) and c2cf549b (text-range windows rebuild early).

## Pass criteria

- All five runs pass, and the `metrics_view` value matches the semantic tree (no 41/42 offset).
- Report the main-versus-#350 result, the bisect's first bad commit if you ran it, and the macOS version.
