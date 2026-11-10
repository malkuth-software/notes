# Owner rulings, 2026-11-09 and 2026-11-10

An index of every owner ruling from these two days and the record that now
holds each one. The record named in the last column is authoritative; this
page only points. Rulings before 2026-11-09 are in the stage briefs,
`planning/ROADMAP_TO_1.0.md` and the projects repository's
`desks/coordinator/archive/STATE_LOG_2026-11.md`.

| Date | Ruling | Recorded in |
|---|---|---|
| 11-09 | Theming is not done until the look integration closes | `planning/ROADMAP_TO_1.0.md` row 2.10 |
| 11-09 | Look ledger N1-N7: rail #93ADD0 (Plan Paint keeps #9BB5D8); tabs +3 (24, selected 28); minimized popup and tab-row QAT are a follow-up RB item; the help "?" part reserved and wired with the help system; default theme overtaken by cp24; per-field cascade confirmed; Downloads screenshots are references only | `planning/DESIGN_LANGUAGE_SPECIFICATION.md` §13; `planning/stages/T4-default-theme/LOOK_INTEGRATION_LEDGER.md` §4 |
| 11-09 | Tab changes: no lip at the selected-tab/plane edge; a smooth faded shadow from each tab into the next; every tab tapered on the right edge; tab colours only for File and contextual tabs. They apply to the stock ribbon too | `planning/DESIGN_LANGUAGE_SPECIFICATION.md` §13 |
| 11-09 | Help system: emulate CHM with Plan Paint's paper look; study rulings 1-7 and A/B; help is records in the language files, not a separate content set; Q14 (section 9's three questions) answered | `studies/HELP_SYSTEM_STUDY.md` §0 and §11 |
| 11-09 | Exhibits: E2, E4, E9, E14, E1, E8, E15, T1, T2 approved, E13 waits; scope now mockups only (E2, E8, E9, E4) | `planning/EXHIBITS_SELECTION.md` |
| 11-09 | SDR# may run through the swap-in on the interactive desktop with network access (SDR# runs only; suites stay on private desktops) | `planning/stages/S-csharp-swap-in/README.md` "Owner rulings" |
| 11-09 | Timing tests: no engineering of determinism into nondeterministic tests; an injected clock when cheap, else the non-gating `timing` label; one quick pass at most | `planning/stages/README.md` "Lessons already paid for" (addendum) |
| 11-09 | Large-list stress demonstrator and the theme editor's completion hints (backlog, after 1.0-critical work) | `planning/POST_0.5_BACKLOG.md` |
| 11-10 | Shared test precompiled header accepted (PR and merge-group builds; scheduled builds without it) | `planning/audits/README.md` (S9); landed in #479 |
| 11-10 | Audit: G2 automation default-off, minimal; the addendum's order a, then N48 and N61, then b, then e | `planning/audits/README.md` |
| 11-10 | Automation moves toward an MCP integration later; WM_COPYDATA is not extended | `planning/POST_0.5_BACKLOG.md` "Automation via MCP"; `planning/stages/C-c-abi/README.md` "Owner rulings" |
| 11-10 | N70: unhandled callback failures under an event loop fail fast by default; continue_running is an opt-in only; the swap-in does not opt out | `planning/stages/S-csharp-swap-in/README.md` and `planning/stages/C-c-abi/README.md` "Owner rulings"; `planning/audits/README.md` |
| 11-10 | The swap-in emulates how WinForms and SDR# controls look (through the theme), never their internal mechanics (no WndProc/WM_PAINT self-repaint emulation) | `planning/stages/S-csharp-swap-in/README.md` "Owner rulings" |
| 11-10 | SDR# zip: runs in place; launcher and facade ship with no licence (third-party notices kept); no contact line; the owner tests it before handing it to Youssef Touil | `planning/stages/S-csharp-swap-in/README.md` "Owner rulings" |
| 11-10 | windows-professional direction: a blend of Windows 2000, Office 2003 and Office 2007, touched toward Windows 10 (crisper, flatter large surfaces) while keeping depth and pop | `planning/DESIGN_LANGUAGE_SPECIFICATION.md` §13 |
| 11-10 | Site: the three-systems triptych may be emulated | `planning/ROADMAP_TO_1.0.md` "Status 2026-11-10" |
| 11-10 | Site images use windows-professional | `planning/DESIGN_LANGUAGE_SPECIFICATION.md` §13; `planning/ROADMAP_TO_1.0.md` row 4.10 |
| 11-10 | Themes may name any bundled font (`font-family: Selawik`), never a host font (option a) | `planning/DESIGN_LANGUAGE_SPECIFICATION.md` §13 |
| 11-10 | Dial and overlay design-language parts: arc, needle, beak, scrim (option a) | `planning/DESIGN_LANGUAGE_SPECIFICATION.md` §13 |
| 11-10 | Overlay and band fix go to main as one small checkpoint; products pin to main (no combined side branch). Later superseded by the owner plan: every worker finishes coding, one combined build, then push (cp27) | projects STATE.md |
| 11-10 | PlaySuite, SwiftEdit and Plan Paint parked (commit and stop); the tile hover blink is the owner's to take up later | `planning/GAPS.md` P5; `planning/ROADMAP_TO_1.0.md` rows 4.1-4.3 |
| 11-10 | Business card name turns fully red when the invalidation overlay is on | `planning/ROADMAP_TO_1.0.md` "Status 2026-11-10" |
