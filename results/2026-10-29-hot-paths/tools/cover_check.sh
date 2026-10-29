#!/bin/sh
# SCRATCH (cover hint): build the paint workload from origin/main (before) and
# this tree (after), compare every case's pixel hash, and time both, alternating.
# Usage: sh .hp/cover_check.sh [rounds]
set -eu
here=$(pwd -P); root="$here/.."; b="$root/.build/native-macos-arm64"; out="$b/hp-out/cover"
rounds="${1:-3}"
mkdir -p "$out"
for side in before after; do
    copy="$root/hp-cover-$side"
    mkdir -p "$copy"
    rsync -a --delete --exclude=/.git "$here/" "$copy/"
    if [ "$side" = before ]; then
        for file in $(git -C "$here" diff --name-only origin/main -- . ':!.hp'); do
            if git -C "$here" cat-file -e "origin/main:$file" 2>/dev/null; then
                git -C "$here" show "origin/main:$file" > "$copy/$file"
            else
                rm -f "$copy/$file"
            fi
        done
    fi
    tree="$b/hp-cover-$side"
    if [ ! -f "$tree/CMakeCache.txt" ]; then
        python3 -B - "$copy" "$tree" <<'PY' > "$root/hp-cover-$side-configure.log" 2>&1
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
    cmake --build "$tree" --parallel 8 --target gui_forms_paint_profile_workload gui_forms_gallery_pages_tests > "$root/hp-cover-$side-build.log" 2>&1 || { tail -30 "$root/hp-cover-$side-build.log"; exit 1; }
done
for side in before after; do
    rm -rf "$out/trace-$side"; mkdir -p "$out/trace-$side"
    (cd "$b/hp-cover-$side" && GUI_FORMS_GALLERY_TRACE_DIR="$out/trace-$side" ./gui_forms_gallery_pages_tests --display-trace > /dev/null 2>&1 || true)
done
python3 -B "$here/.hp/trace_compare.py" "$out/trace-before" "$out/trace-after"
round=1
while [ "$round" -le "$rounds" ]; do
    for side in before after; do
        rm -rf "$out/$side-$round"; mkdir -p "$out/$side-$round"
        start=$(python3 -c 'import time; print(time.time())')
        "$b/hp-cover-$side/gui_forms_paint_profile_workload" "$out/$side-$round" > /dev/null
        python3 -c "import time; print('$side round $round wall %.2f s' % (time.time() - $start))"
    done
    round=$((round + 1))
done
python3 -B "$here/.hp/cover_compare.py" "$out" "$rounds"
uptime
