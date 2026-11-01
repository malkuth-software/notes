#!/bin/sh
set -u
llvm=$(ls -d /opt/homebrew/opt/llvm/bin 2>/dev/null || true); [ -n "$llvm" ] && PATH="$llvm:$PATH"
root="$(pwd -P)/.."
for src in "$root/stx-skia-negative-src" "$root/stx-skia-positive-src"; do
  cp .hp/column_cut.cpp "$src/tests/column_cut.cpp"
  grep -q column_cut "$src/CMakeLists.txt" || printf '\nadd_executable(stx_skia_column_cut tests/column_cut.cpp)\ntarget_link_libraries(stx_skia_column_cut PRIVATE stx_skia)\n' >> "$src/CMakeLists.txt"
done
for t in negative positive; do cmake -S "$root/stx-skia-$t-src" -B "$root/.build/stx-skia-$t" > /dev/null 2>&1; cmake --build "$root/.build/stx-skia-$t" -j 8 --target stx_skia_column_cut > "$root/stx-col.log" 2>&1 || { tail -20 "$root/stx-col.log"; exit 1; }; echo "== $t"; "$root/.build/stx-skia-$t/stx_skia_column_cut"; done
