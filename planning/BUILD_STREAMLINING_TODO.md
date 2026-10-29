# Build streamlining: to do next time (owner direction, 2026-10-29)

Status: recorded, not started. The goal is to minimise time to build (TTB) and
round-trip time at every stage, locally and remotely, and to stop endangering
the free GitHub Actions allowance.

## Where we are (2026-10-29)

- A full GUI.Forms build is 1736 ninja steps.
- Locally (the M4 through `m4ws`, ccache) a rebuild takes a few seconds once
  the cache is warm; a cold full build was 271 s and a cached one 24 s
  (2026-10-28).
- Remotely, every runner bootstraps itself, and one checkpoint PR starts the
  native jobs on four platforms, the sanitizer job, the gates and any
  path-filtered workflows. On 2026-10-29 the queue reached GitHub's
  concurrent-runner limit, and the required state-machine gate waited for a
  runner.
- The first trims landed in #452: codec fuzz and differential are weekly and
  manual only, and the path-filtered workflows no longer run again on main.

## Direction

1. **Fewer steps.** Reduce the 1736 steps further than we do now.
2. **Dependencies on their own repositories.** Refactor GUI.Forms so that the
   in-house Skia (stx_skia) and FreeType/HarfBuzz live in their own
   repositories, each with its own cache. GUI.Forms pulls them into its build,
   and so does anything else that builds with them, so they are never rebuilt
   as part of GUI.Forms.
3. **Better caching for the remaining files.** Tests that can fail are staged
   off safely, so that under no circumstances can they pollute or break cache
   reuse. The cache, and the record of successful builds, only ever improves on
   the main line.
4. **Branches reuse main's cache.** Branches import and reuse main's cache
   wherever they can. They do not run large builds, and they run only the
   tests their change needs.
5. **Sanitizers off by default.** Sanitizers do not run by default, and never on
   main.
6. **Remote TTB close to local.** The aim is near-local times remotely, given
   that runners must bootstrap. Cut the round trip at every stage.
7. **Template integration.** Fold the whole setup into the template's GitHub
   setup (template_harness), so every build system behaves correctly and
   efficiently wherever the work happens.
8. **A far faster way to use the remote workers for tests.** Tests live in a
   standalone testing repository. A test run there changes only the files it
   needs and runs only the tests it asks for.

## To reconcile when this starts

- **Sanitizer rules.** Item 5 conflicts with the current written rules:
  - gui_forms `AGENTS.md` makes the `sanitizers` CI job run the whole suite
    and requires `ci_build.py --sanitize` before pushing C++ and before stage
    gates;
  - the checkpoint verify (template_harness `docs/workflow.md`) runs the
    sanitized suite;
  - the sanitizer job is a required check.

  Decide where sanitizers run instead (on demand, on a schedule, at stage
  gates only), then update those documents and the required checks together.
- **Required check names.** The required checks are engine-gate, docs,
  house-style, the four native builds, Sanitizers and the state-machine gate.
  Branch protection must change together with the workflows, by the owner.
- **Reference pins.** stx_skia's reference profile and the E4 ISA runs (RunPod
  for x86-64, the Windows host) need a home once stx_skia moves to its own
  repository.
- **Where the checkpoint verify builds.** It should build against the
  published dependency caches too, so the M4 stops rebuilding them.
