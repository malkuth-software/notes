#!/bin/sh
# SCRATCH (B1 prototype): build .hp/band_proto.cpp in a copy of this tree and run it.
# Usage: sh .hp/band_check.sh cases | hold <threads> <bands> <seconds>
set -eu
here=$(pwd -P); root="$here/.."; b="$root/.build/native-macos-arm64"
copy="$root/hp-band"; tree="$b/hp-band"
mkdir -p "$copy"
rsync -a --delete --exclude=/.git "$here/" "$copy/"
cp "$here/.hp/band_proto.cpp" "$copy/benchmarks/band_proto.cpp"
cp "$here/.hp/damage_probe.cpp" "$copy/benchmarks/damage_probe.cpp"
cat >> "$copy/CMakeLists.txt" <<'CM'
if(GUI_FORMS_BUILD_PAINT_PROFILE)
    add_executable(gui_forms_band_proto benchmarks/band_proto.cpp)
    target_link_libraries(gui_forms_band_proto PRIVATE gui_forms_gallery_model gui_forms_skia)
    add_executable(gui_forms_damage_probe benchmarks/damage_probe.cpp)
    target_link_libraries(gui_forms_damage_probe PRIVATE gui_forms_skia)
endif()
CM
if [ ! -f "$tree/CMakeCache.txt" ]; then
    python3 -B - "$copy" "$tree" <<'PY' > "$root/hp-band-configure.log" 2>&1
import subprocess, sys
from pathlib import Path
source = Path(sys.argv[1]); tree = Path(sys.argv[2])
sys.path.insert(0, str(source / 'tools'))
from toolkit_configure import host_options, prepare_host, toolkit_options
import profile_hot_paths
prepare_host(source, True)
subprocess.run(['cmake', '-S', str(source), '-B', str(tree), '-G', 'Ninja',
                *toolkit_options(source, tree.parent / (tree.name + '-sdk'), True, True),
                *host_options('macos'), *profile_hot_paths.WORKLOAD_OPTIONS], check=True)
PY
fi
cmake --build "$tree" --parallel 8 --target gui_forms_band_proto gui_forms_damage_probe > "$root/hp-band-build.log" 2>&1 || { grep -E "error|Error" "$root/hp-band-build.log" | head -30; exit 1; }
fonts="$copy/assets/fonts"
case "$1" in
cases) "$tree/gui_forms_band_proto" "$fonts" cases;;
probe) "$tree/gui_forms_damage_probe" 2>&1 | head -60;;
bisect) shift; for spec in "$@"; do set -- $(echo "$spec" | tr : " "); "$tree/gui_forms_band_proto" "$fonts" bisect "$1" "$2" "$3" 2>&1; done;;
seam) shift; for spec in "$@"; do set -- $(echo "$spec" | tr : " "); "$tree/gui_forms_band_proto" "$fonts" seam "$1" "$2" "$3" 2>&1; done;;
diff) BAND_DIFF=1 "$tree/gui_forms_band_proto" "$fonts" cases 2> "$root/hp-band-diff.txt" > /dev/null; grep -c DIFF "$root/hp-band-diff.txt";;
hold)
    "$tree/gui_forms_band_proto" "$fonts" hold "$2" "$3" "$4" &
    pid=$!
    sleep 12
    /usr/bin/sample "$pid" 8 -file "$root/hp-band-sample-$2-$3.txt" > /dev/null 2>&1 || true
    wait "$pid"
    echo "sample written: $root/hp-band-sample-$2-$3.txt";;
esac
sysctl -n hw.perflevel0.logicalcpu hw.perflevel1.logicalcpu 2>/dev/null || true
uptime
