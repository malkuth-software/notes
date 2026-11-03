# C7: the GUI.Forms C ABI 1.0 audit record (0.5 baseline)

Recorded 2026-11-02 by the C ABI track (stage C, C7), gui_forms branch
claude/c-abi-c6 at 7b66e3c1 on main 855b0f0e (checkpoint 12). Host: Windows 11 x86-64,
CLANG64 toolchain (pinned LLVM), build tree native-windows-x64.

## Baseline

- manifests/abi/baseline/gf_abi-0.5.json: ABI 1.0, 62 records (17 tables,
  122 slots), 207 constants, 1 export (gf_abi_get_table); four targets agree
  (x86_64-w64-windows-gnu, x86_64-pc-windows-msvc, x86_64-unknown-linux-gnu, aarch64-apple-darwin).
- manifests/abi/audit_policy.json: mode report, that baseline, an empty ledger.

## Gate (C7: 0 findings against the baseline, seeded breaks 100%)

| check | result |
| --- | --- |
| tools/abi_audit.py check (gui_forms_abi_audit) | 0 findings against the baseline |
| seeded breaks (gui_forms_abi_audit_fixtures) | 28 of 28 detected; 6 legal cases not reported |
| a slot removed from the baseline (manual seed) | reported as an unrecorded break |
| exports (gui_forms_abi_exports) | the DLL and the MSVC import library export exactly gf_abi_get_table |
| layout probe | current; compiled by clang (C11, C++) and by MSVC cl (C11, /W4 /WX) |
| ctest -L c_abi | 30/30 plain, 30/30 ASan+UBSan |

## Consumers proven on this host

- C11 (clang), the conformance kit and the installed-SDK reference consumer.
- MSVC: the layout probe, the reference consumer and the C++ wrappers' test,
  against gui_forms_abi1.lib (gui_forms_abi_msvc_consumer).
- C++20: include/gui_forms/c/gf_abi.hpp, generated, 1:1 slot map checked.
- C# (.NET 10.0.403 SDK, runtime 10.0.14): the generated interop's layout
  self-test, negotiation, ownership, refusal and callback thunks with shadow
  frames (gui_forms_abi_csharp), built offline.

## Not yet recorded

- macOS and Linux: exports, probe and audit run in CI's native builds; the C#
  test needs a .NET 10 SDK on those runners (a download, not admitted yet).
- TSan and the product consumers are C8.
