#!/bin/sh
# SCRATCH (AAA clip rows): stx_skia's profiles (candidate, baseline, compare, sanitize), then a
# negative control: the candidate's AA corpus test against the unchanged SkScan_AAAPath.cpp.
set -u
here=$(pwd -P); root="$here/.."
llvm=$(ls -d /opt/homebrew/opt/llvm/bin 2>/dev/null || true)
[ -n "$llvm" ] && PATH="$llvm:$PATH"
export STX_SKIA_BUILD_ROOT="$root/.build/stx-skia-full" BUILD_JOBS=8
cd "$here/stx_skia"
case "${1:-all}" in
all|profiles)
    sh tools/build_native.sh candidate > "$root/stx-full-candidate.log" 2>&1; echo "candidate exit $?"; grep -E "tests passed|tests failed|FAIL" "$root/stx-full-candidate.log" | tail -5
    STX_SKIA_REFERENCE_ROOT="$root/stx-skia-reference" sh tools/build_native.sh baseline > "$root/stx-full-baseline.log" 2>&1; echo "baseline exit $?"; grep -E "tests passed|tests failed" "$root/stx-full-baseline.log" | tail -3
    python3 -B tools/compare_profiles.py "$STX_SKIA_BUILD_ROOT/baseline" "$STX_SKIA_BUILD_ROOT/candidate" > "$root/stx-full-compare.log" 2>&1; echo "compare exit $?"; tail -15 "$root/stx-full-compare.log";;
esac
case "${1:-all}" in
all|sanitize)
    sh tools/build_native.sh candidate-sanitize > "$root/stx-full-sanitize.log" 2>&1; echo "sanitize exit $?"; grep -E "tests passed|tests failed|ERROR|runtime error" "$root/stx-full-sanitize.log" | tail -5;;
esac
case "${1:-all}" in
all|negative)
    neg="$root/.build/stx-skia-negative"
    rm -rf "$neg" "$root/stx-skia-negative-src"; mkdir -p "$root/stx-skia-negative-src"
    rsync -a --exclude=/build "$here/stx_skia/" "$root/stx-skia-negative-src/"
    git -C "$here" show origin/main:stx_skia/skia/src/core/SkScan_AAAPath.cpp > "$root/stx-skia-negative-src/skia/src/core/SkScan_AAAPath.cpp"
    cmake -S "$root/stx-skia-negative-src" -B "$neg" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ > "$root/stx-neg.log" 2>&1
    cmake --build "$neg" -j 8 --target stx_skia_aa_scan_converter_tests >> "$root/stx-neg.log" 2>&1
    "$neg/stx_skia_aa_scan_converter_tests" > "$root/stx-neg-run.log" 2>&1; echo "negative control exit $? (expected 1)"; grep -c "differs" "$root/stx-neg-run.log"; grep "canvas" "$root/stx-neg-run.log" | head -4; tail -2 "$root/stx-neg-run.log";;
esac
