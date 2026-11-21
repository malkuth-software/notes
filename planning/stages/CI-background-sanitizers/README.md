# CI: sanitizers in the background, never merged

Written by the coordinator, 2026-11-21, from the owner's direction of the
same day. Covers STXlib and GUI.Forms.

## 1. The owner's direction (2026-11-21)

- Address-sanitizer runs have not happened for a while because those
  builds took very long.
- The owner has a free 30-day trial of **Namespace** (namespace.so), whose
  approach is very different. We do **not** refactor our work for their
  system. We use only their CI runner instances, **at most one per
  architecture at a time**, and run sanitizer CI there.
- Tests and programs already moved into their own repositories so they can
  run "chilled" in the background and not block progress. That remains the
  right approach: waiting on three builds is not acceptable.
- **The plan:** tighten STXlib, get it building and pinned with all changes
  made, then run its sanitizers in the background as a **second pull
  request** that is never merged; it exists only to be run. This is the
  "sanitizers off by default" plan. GUI.Forms does the same once its coding
  is done and its main pull request is merged and pinned.

## 2. How it works

- **The main pull request** builds and lands as today (compile-only CI where
  the owner set it, the local gates). It is merged and becomes the pin.
- **The sanitizer pull request** is opened from a branch on the merged pin
  whose only change is a workflow that builds the sanitized tree and runs
  the whole suite under ASan, UBSan and LSan (TSan where planned). It is
  never merged. Rerunning it after a later pin means updating its branch to
  the new pin.
- **Runners:** the workflow's jobs run on Namespace's runners, selected by
  their labels (`runs-on` set to the Namespace runner profile for each
  architecture). Each architecture's job has a GitHub Actions
  `concurrency` group, so at most one runs per architecture at a time; a
  newer run waits or replaces the older one.
- **Nothing else changes:** the same `tools/ci_build.py --sanitize` and the
  same test runners (for GUI.Forms, `gui_forms_tests`' `tools/ci_tests.py
  --kind sanitize` against the sanitize test cache); no Namespace-specific
  build system, cache or configuration in the repositories beyond the
  `runs-on` labels.
- **Results** go back as the pull request's checks; failures are filed as
  defects against the pin (memory errors and undefined behaviour are
  defects, house style section 1).

## 3. Owner actions

- Install Namespace's GitHub app on the `malkuth-software` organisation for
  the repositories involved (STXlib, gui_forms, gui_forms_tests) and create a
  runner profile per architecture (Linux x86-64 and Arm64, and macOS Arm64
  if it is offered and wanted).
- The trial lasts 30 days; decide before it ends whether to keep it.

## 4. Order

STXlib first (after it is tightened, building and pinned), then GUI.Forms
after its main pull request is merged and pinned. Placement: order of
operations step 5 (organise the builds) sets it up; it runs in the
background from then on. The final sanitizer runs before 1.0 (step 6's
close) use it.
