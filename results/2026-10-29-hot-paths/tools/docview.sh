#!/bin/sh
# SCRATCH: document_view 100 runs in one process (patched copy in ../hp-src), recorded with xctrace.
set -eu
here=$(pwd -P); copy="$here/../hp-src"; b="$here/../.build/native-macos-arm64/gui-forms-hp-fill"; o="$here/../.build/native-macos-arm64/hp-out"
sed -i '' 's|        for (std::size_t index = 0; index < sizes.size(); ++index) { measure(sizes\[index\]); }|        for (int run = 0; run < 100; ++run) for (std::size_t index = 0; index < sizes.size(); ++index) { measure(sizes[index]); }|' "$copy/benchmarks/document_view_benchmark.cpp"
grep -c "run < 100" "$copy/benchmarks/document_view_benchmark.cpp"
cmake --build "$b" --target gui_forms_document_view_benchmark > /dev/null
rm -rf "$o/document_view.trace" "$o/document_view-sym.trace" "$b/gui_forms_document_view_benchmark.dSYM"
xcrun xctrace record --template 'Time Profiler' --output "$o/document_view.trace" --target-stdout /dev/null --launch -- "$b/gui_forms_document_view_benchmark" > /dev/null
dsymutil "$b/gui_forms_document_view_benchmark" -o "$b/gui_forms_document_view_benchmark.dSYM" 2>/dev/null
xcrun xctrace symbolicate --input "$o/document_view.trace" --output "$o/document_view-sym.trace" --dsym "$b/gui_forms_document_view_benchmark.dSYM" > /dev/null 2>&1 || true
python3 -B .hp/xt_summary.py --binary "$b/gui_forms_document_view_benchmark" "$o/document_view-sym.trace" --top 10
