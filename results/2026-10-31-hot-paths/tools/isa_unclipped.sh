#!/bin/sh
# SCRATCH: build isa_unclipped against the old AAA (negative copy) and the new AAA (positive copy).
set -u
llvm=$(ls -d /opt/homebrew/opt/llvm/bin 2>/dev/null || true); [ -n "$llvm" ] && PATH="$llvm:$PATH"
here=$(pwd -P); root="$here/.."
pos="$root/stx-skia-positive-src"; mkdir -p "$pos"
rsync -a --delete --exclude=/build "$here/stx_skia/" "$pos/"
for src in "$root/stx-skia-negative-src" "$pos"; do
  cp "$here/.hp/isa_unclipped.cpp" "$src/tests/isa_unclipped.cpp"
  grep -q isa_unclipped "$src/CMakeLists.txt" || printf '\nadd_executable(stx_skia_isa_unclipped tests/isa_unclipped.cpp)\ntarget_link_libraries(stx_skia_isa_unclipped PRIVATE stx_skia)\n' >> "$src/CMakeLists.txt"
done
neg="$root/.build/stx-skia-negative"; pb="$root/.build/stx-skia-positive"
[ -f "$pb/CMakeCache.txt" ] || cmake -S "$pos" -B "$pb" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ > "$root/stx-pos.log" 2>&1
cmake -S "$root/stx-skia-negative-src" -B "$neg" > /dev/null 2>&1
for t in "$neg" "$pb"; do cmake --build "$t" -j 8 --target stx_skia_isa_unclipped >> "$root/stx-pos.log" 2>&1 || { tail -20 "$root/stx-pos.log"; exit 1; }; done
echo "old AAA:"; "$neg/stx_skia_isa_unclipped"
echo "new AAA:"; "$pb/stx_skia_isa_unclipped"
