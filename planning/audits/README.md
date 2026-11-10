# Audits

Date: 2026-11-10. This folder holds the external audits of the GUI.Forms
code base, their triages against current main, and the owner's decisions on
each batch. The audit texts are kept verbatim; the triages are the
coordinator desk's copies (projects `desks/coordinator/`, where they were
written).

| File | What it is | Read against |
|---|---|---|
| `CODEBASE_AUDIT_2026-11-09.md` | External subjective audit, about 90 findings (A control core to J documents) | an old head, bd4b60bf |
| `AUDIT_TRIAGE_2026-11-10.md` | Triage of that audit: each finding re-read on main, a verdict, a size and eight batches | main ae189ff4b |
| `AUDIT_ADDENDUM_2026-11-10.md` | The audit's addendum: the sealed C ABI (K, N), correctness, parity, performance (S) | main |
| `AUDIT_ADDENDUM_TRIAGE_2026-11-10.md` | Triage of the addendum, with options a to f | main def3b2df6 |
| `STALE_TRACES_2026-11-09.md` | Sweep for stale traces in the planning records (what was routed, fixed directly, not covered) | notes and the desk, 2026-11-09 |

Every triage verdict is a recommendation. Only the rows below record what
the owner decided.

## Owner decisions per batch

### The first audit (`AUDIT_TRIAGE_2026-11-10.md`)

| Batch or item | Owner decision | Where it went | Status 2026-11-10 |
|---|---|---|---|
| G2 security: the Windows WM_COPYDATA automation channel was on for every C ABI window | The owner is not sure it is a security problem. Make automation default-off with a minimal change (no capture confinement, no sender checks); move automation toward an MCP integration later and do not extend WM_COPYDATA | `claude/automation-off-by-default` 619b33c9 (w6): on only for `GF_WINDOW_RUN_AUTOMATION` or `GUI_FORMS_AUTOMATION=1`. MCP: `planning/POST_0.5_BACKLOG.md` "Automation via MCP", `planning/GAPS.md` | coded and tested; in cp27 (PR #480, open) |
| Batches 1-8 (event/tree/lifecycle defects; locale-independent parsing; shaping and glyph caches; paint whole-tree work; pointer bookkeeping; host wake and idle; small cleanups; docs) | Not ruled yet | | open |
| Larger items: leaf control inheritance F1/F2/F9, font face identity D8, registry scope H3 and `c_api.h` H1, unused dirty kinds A18, large-file splits B10/G1, display-list encoding C2/C3, document-scale editing F8, build structure and fuzzing I1/I3 | Not ruled yet; each needs an owner decision before sizing | `planning/GAPS.md` (D8 as font face identity) | open |

### The addendum (`AUDIT_ADDENDUM_TRIAGE_2026-11-10.md`)

The owner accepted the recommended order on 2026-11-10: a, then N48 and N61,
then the rest of b; e next.

| Option | Owner decision | Branch | Status 2026-11-10 |
|---|---|---|---|
| a. Seal batch: N20, N34, N71, N14, N27, N10, N24, N17, N21, N25, N26, N29, N9, N5; docs K4, K5, K8, N16 | Accepted, first | `claude/abi-seal-fixes` 5deb8e2b (w4) | coded; in cp27 (PR #480). N9 needs a ThreadSanitizer run on the Mac or Linux (Mac handoff item 5) |
| b. Hang and loss: N48, N2, N42, N45, N43, N70, N52, N113 | Accepted, N48 first | `claude/audit-hang-parity` 1d57cab05 | coded; in cp27 (PR #480) |
| N70 default | **Fail fast** (2026-11-10). An unhandled managed callback failure under `windows.run` or `show_modal` ends the process by default; `UnhandledCallbackPolicy.continue_running` stays available as an opt-in only. SDR#'s author prefers fail fast too, so the swap-in does not opt out | in `claude/audit-hang-parity` (`Callbacks.cs`) | done |
| c. Parity: N61 first | N61 accepted with b | `claude/audit-hang-parity` (the Windows host stops advertising `text_composition` and places the IME windows at the caret) | coded; in cp27 |
| c. the rest: N62 (horizontal wheel), N66, N68, N75; a manual Japanese input test on macOS for N105 and N106 | Not ruled yet | | open |
| d. Tree-cost batch | Not ruled yet | | open |
| e. Per-frame cost, same pixels: S1, N56 with S4, N90 with N1, S5, N50 and N51 | Accepted as next | | not started |
| f. Finish the read | Not ruled yet | | open |
| Owner-decision rows: K10 painter table, N32 dialog owner, N94 incremental list models, N96 warnings and hardening, N110 sub-chunks for composites, S9 test link structure | S9: the shared test precompiled header was accepted 2026-11-10 (on PR and merge-group builds; scheduled builds without it), landed in #479. The rest not ruled | | S9 done; rest open |
