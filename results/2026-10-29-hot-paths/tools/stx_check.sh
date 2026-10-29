#!/bin/sh
# SCRATCH (AAA clip rows): build the AA scan-converter corpus test in the candidate and the
# reference profiles of stx_skia, run both, compare their outputs.
# Usage: sh .hp/stx_check.sh [sanitize]
set -eu
here=$(pwd -P); root="$here/.."
reference="$root/stx-skia-reference"
if [ ! -d "$reference/skia" ]; then
    git clone --filter=blob:none --no-checkout https://github.com/malkuth-software/stx_skia "$reference" > "$root/stx-ref-clone.log" 2>&1
    git -C "$reference" sparse-checkout set baseline include skia src >> "$root/stx-ref-clone.log" 2>&1
    git -C "$reference" checkout -q c20afa3c619d736e1588a33d952a26aaa53caced >> "$root/stx-ref-clone.log" 2>&1
fi
llvm=$(ls -d /opt/homebrew/opt/llvm/bin 2>/dev/null || true)
[ -n "$llvm" ] && PATH="$llvm:$PATH"
cd "$here/stx_skia"
out="$root/.build/stx-skia-aaa"
target=stx_skia_aa_scan_converter_tests
for profile in ${1:-candidate baseline}; do
    case "$profile" in
        baseline) flags="-DSTX_SKIA_BASELINE=ON -DSTX_SKIA_SANITIZE=OFF -DSTX_SKIA_REFERENCE_ROOT=$reference" ;;
        candidate) flags="-DSTX_SKIA_BASELINE=OFF -DSTX_SKIA_SANITIZE=OFF" ;;
        sanitize) flags="-DSTX_SKIA_BASELINE=OFF -DSTX_SKIA_SANITIZE=ON" ;;
    esac
    tree="$out/$profile"
    cmake -S . -B "$tree" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ $flags > "$out-$profile-configure.log" 2>&1 || { tail -20 "$out-$profile-configure.log"; exit 1; }
    cmake --build "$tree" -j 8 --target $target > "$out-$profile-build.log" 2>&1 || { grep -E "error" "$out-$profile-build.log" | head -20; exit 1; }
    if [ "$profile" = sanitize ]; then
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$tree/$target" && echo "sanitize exit 0"
    else
        "$tree/$target" "$tree/aa-scan-converters.raw" "$tree/aa-scan-clip-rows.raw"; echo "$profile exit $?"
    fi
done
if [ -f "$out/baseline/aa-scan-converters.raw" ] && [ -f "$out/candidate/aa-scan-converters.raw" ]; then
    cmp "$out/baseline/aa-scan-converters.raw" "$out/candidate/aa-scan-converters.raw" && echo "aa-scan-converters.raw MATCHES"
    cmp -s "$out/baseline/aa-scan-clip-rows.raw" "$out/candidate/aa-scan-clip-rows.raw" && echo "aa-scan-clip-rows.raw MATCHES (should differ)" || echo "aa-scan-clip-rows.raw differs (intended)"
fi
