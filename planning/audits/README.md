# Audits

Date: 2026-11-10, rulings brought up to date on 2026-11-22. This folder holds the external audits of the GUI.Forms
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
| `DOCS_REVIEW_2026-11-21.md` | Review of the documentation before 1.0 (week plan items D1 to D4); nothing changed | main c0e1a31a8 |
| `FORM_FORMAT_AUDIT_2026-11-21.md` | The form document format, XML or CSS (asked by the owner) | main c0e1a31a8 |
| `DEVEXPRESS_ROUNDUP_2026-11-22.md` | DevExpress controls, demos and tools to match (week plan item 3.2) | 2026-11-22 |
| `STALE_TRACES_2026-11-09.md` | Sweep for stale traces in the planning records (what was routed, fixed directly, not covered) | notes and the desk, 2026-11-09 |

Two later records live only on the desk (projects `desks/coordinator/`):

- `AUDIT_COMPLETION_2026-11-11.md` reads what the addendum's triage left
  unread. Its findings are P1 to P20, read on the cp28 assembly head.
- `AUDIT_TO_1_0_2026-11-15.md` is the plan of what remains to 1.0 (main
  96d3696). It holds no new findings or rulings.

Every triage verdict is a recommendation. Only the rows below record what
the owner decided.

## Owner decisions

### The ruling that covers both audits (owner, 2026-11-10)

After cp27 folded, cp28 was to fix everything still open from the audit
(82 findings, A to J) and its addendum (K1-K12, N1-N116, S1-S9). It was also
to settle everything that had to be right before the ABI and API were called
stable. The owner split this into two checkpoints:

- **cp28 (PR #481):** workstreams A to H.
- **cp29 (PR #482, with cp30 and cp31):** workstream I, the re-layout.

The per-item record is section 2 of the desk's `CP28_PLAN.md`; its section
8 holds the owner's decisions. Every batch the 2026-11-10 table left
"not ruled yet" falls under this ruling:

- the first audit's batches 1 to 8;
- the addendum's options c (the rest), d and f;
- the owner-decision rows.

### The larger items (cp28 decisions D1 to D13, owner, 2026-11-10)

| Item | Decision | Where it went |
|---|---|---|
| D1: leaf inheritance F1, F2, F9 | Accepted: public TextBoxBase, ListControl and UpDownBase; flat `controls/<name>.hpp` with no forwarding headers; lab controls out of the installed library | cp29 workstream I (#482; the bases in 1da03e3f4) |
| D2: font face identity D8 (with K1) | Accepted: `FontSpec::family`, an interned family id in `gf_font.reserved` | the font family registry, 6ae847879 (#483) |
| D3: registry scope H3 | Accepted: one registry per process, stated in the header | `gf_abi.h:326` |
| D4: unused dirty kinds A18 | Accepted: remove text, style and resource; keep and wire accessibility | `include/gui_forms/dirty.hpp` |
| D5: experimental `c_api.h` H1 | Accepted: retired with the facade generator and the 0.x tests | cp28 (#481) |
| D6: live surfaces while an overlay is open, C7 | Option a: suspend only the covered pixels | cp28 (#481) |
| D7: dock fill A8 | Accepted, amended: the reflection-harness result governs only the Windows Forms compatibility layer, and GUI.Forms' own docking records the divergence | divergence documented in 426c1a32a (#483) |
| D8: painter table K10 | Option b: a painter table over the gd_* vocabulary in 1.x, after the gd seal; cp28 still adds K11's measure, hit-test and semantics callbacks | 1.x |
| D9: ABI 1.2 (N32) | Accepted | cp28 (#481) |
| D10: incremental list models N94 | Option b: a virtual tree source now, over a height-indexed B-tree | `tree_list_view.hpp:115`; the row index is STXlib's RowHeightIndex (6dc5cd60b) |
| D11: warnings and hardening N96, S9 rest | Accepted | cp28 (#481); warnings are failures since a34c2a8e8 (#484) |
| D12: item-level chunks for composites N110 | Accepted, measured after the frame-cost work | no commit naming N110 found on main |
| D13: the conformance consumers | PlaySuite, SwiftEdit and Plan Paint, through the C ABI | the conformance step |
| N66: X11 HiDPI | Option b: monitors only in cp28, full X11 scaling in cp29 | #481, #482 |

### The audit completion (owner, 2026-11-11)

| Findings | Decision | Where it went |
|---|---|---|
| P2 to P10, P12 to P20 | Accepted as recommended, in cp28. Unproven findings are tested first, and the "later" items are included, per "everything by cp28" | 124db503b (#481) |
| P1 (universal exception boundaries: efficiency and performance), P11 (image downsampling) | Held for a detailed breakdown | no later ruling found |

### Earlier rulings (2026-11-10), with their outcome

| Item | Owner decision | Outcome |
|---|---|---|
| G2: the Windows WM_COPYDATA automation channel was on for every C ABI window | Automation is off by default, through a minimal change; later moves toward MCP | in cp27 (#480): on only for `GF_WINDOW_RUN_AUTOMATION` or `GUI_FORMS_AUTOMATION=1` |
| Addendum option a: the seal batch (N20, N34, N71, N14, N27, N10, N24, N17, N21, N25, N26, N29, N9, N5; docs K4, K5, K8, N16) | Accepted, first | in cp27 (#480, f88f58843). N9's ThreadSanitizer proof passed on the cp29 block (2026-11-11); a run after the tests split is `planning/GAPS.md` P2 |
| Addendum option b: hang and loss (N48, N2, N42, N45, N43, N70, N52, N113) | Accepted, N48 first | in cp27 (#480) |
| N70 default | Fail fast. An unhandled managed callback failure under `windows.run` or `show_modal` ends the process; `UnhandledCallbackPolicy.continue_running` is an opt-in | in cp27 (#480) |
| Addendum option c: N61 | Accepted with b | in cp27 (#480) |
| Addendum option e: per-frame cost (S1, N56 with S4, N90 with N1, S5, N50, N51) | Accepted as next | cp28 (#481) |
| S9: the shared test precompiled header | Accepted | #479; the tests have since moved to gui_forms_tests (#484) |

Later decisions that change what these audits named are in
`planning/DECISIONS_2026-11.md` and on the desk.
