#!/bin/sh
# SCRATCH (F1): build the checker against origin/main (old) and this tree (new); compare hashes; time both.
set -eu
here=$(pwd -P); root="$here/.."; b="$root/.build/native-macos-arm64"
for side in old new; do
    copy="$root/hp-$side"
    mkdir -p "$copy"
    rsync -a --delete --exclude=/.git "$here/" "$copy/"
    if [ "$side" = old ]; then
        git -C "$here" show origin/main:src/render/text/glyph_run/glyph_run_raster.cpp > "$copy/src/render/text/glyph_run/glyph_run_raster.cpp"
    fi
    printf '\nadd_executable(hp_glyph_check .hp/glyph_check.cpp)\ntarget_link_libraries(hp_glyph_check PRIVATE gui_forms_text_engine freetype)\n' >> "$copy/CMakeLists.txt"
    tree="$b/hp-glyph-$side"
    if [ ! -f "$tree/CMakeCache.txt" ]; then
        python3 -B - "$copy" "$tree" <<'PY' > "$root/hp-glyph-$side-configure.log" 2>&1
import subprocess, sys
from pathlib import Path
source = Path(sys.argv[1]); tree = Path(sys.argv[2])
sys.path.insert(0, str(source / 'tools'))
from toolkit_configure import host_options, prepare_host, toolkit_options
prepare_host(source, True)
subprocess.run(['cmake', '-S', str(source), '-B', str(tree), '-G', 'Ninja',
                *toolkit_options(source, tree.parent / (tree.name + '-sdk'), False, False),
                *host_options('macos')], check=True)
PY
    fi
    grep -q "GUI_FORMS_BUILD_TEXT_ENGINE_BENCHMARK:BOOL=ON" "$tree/CMakeCache.txt" || cmake -DGUI_FORMS_BUILD_TEXT_ENGINE_BENCHMARK=ON "$tree" > /dev/null
    cmake --build "$tree" --parallel 8 --target hp_glyph_check gui_forms_text_engine_benchmark > "$root/hp-glyph-$side-build.log" 2>&1 || { tail -30 "$root/hp-glyph-$side-build.log"; exit 1; }
    "$tree/hp_glyph_check" "$here/assets/fonts" > "$b/hp-out/glyph-$side.txt"
done
if cmp -s "$b/hp-out/glyph-old.txt" "$b/hp-out/glyph-new.txt"; then echo "IDENTICAL $(wc -l < "$b/hp-out/glyph-new.txt") lines; $(tail -1 "$b/hp-out/glyph-new.txt")"; else echo DIFFERENT; diff "$b/hp-out/glyph-old.txt" "$b/hp-out/glyph-new.txt" | head -20; fi
for round in 1 2 3; do
    for side in old new; do echo "$side $("$b/hp-glyph-$side/hp_glyph_check" "$here/assets/fonts" time | tr '\n' ' ')"; done
done
for round in 1 2 3; do
    for side in old new; do echo "$side text_engine $("$b/hp-glyph-$side/gui_forms_text_engine_benchmark" "$here/assets/fonts" 15 | grep '^stage')"; done
done
uptime
