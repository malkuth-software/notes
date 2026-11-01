#!/bin/sh
# SCRATCH (AAA clip invariance, case by case): build the paint workload with
# the probe from origin/main's stx_skia (before) and this tree (after), dump
# every case's pixels and repaint each frame whole and in uneven bands.
set -eu
here=$(pwd -P); root="$here/.."; b="$root/.build/native-macos-arm64"; out="$b/hp-out/aaa-probe"
mkdir -p "$out"
for side in before after; do
    copy="$root/hp-probe-$side"
    mkdir -p "$copy"
    rsync -a --delete --exclude=/.git "$here/" "$copy/"
    if [ "$side" = before ]; then
        for file in $(git -C "$here" diff --name-only origin/main -- stx_skia/skia/src); do
            git -C "$here" show "origin/main:$file" > "$copy/$file"
        done
    fi
    python3 -B "$here/.hp/patch_workload.py" "$copy/benchmarks/paint_profile_workload.cpp"
    sed -i '' -e 's/damage_warmup_frames = 24/damage_warmup_frames = 1/' -e 's/damage_measured_frames = 240/damage_measured_frames = 2/' \
        -e 's/page_warmup_frames = 4/page_warmup_frames = 1/' -e 's/page_measured_frames = 24/page_measured_frames = 2/' "$copy/benchmarks/paint_profile_workload.cpp"
    tree="$b/hp-probe-$side"
    if [ ! -f "$tree/CMakeCache.txt" ]; then
        python3 -B - "$copy" "$tree" <<'PY' > "$root/hp-probe-$side-configure.log" 2>&1
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
    cmake --build "$tree" --parallel 10 --target gui_forms_paint_profile_workload > "$root/hp-probe-$side-build.log" 2>&1 || { tail -30 "$root/hp-probe-$side-build.log"; exit 1; }
    [ "${1:-}" = tall ] && continue
    rm -rf "$out/$side"; mkdir -p "$out/$side/run"
    GUI_FORMS_PAINT_DUMP_DIR="$out/$side" "$tree/gui_forms_paint_profile_workload" "$out/$side/run" > /dev/null
done
[ "${1:-}" = tall ] || python3 -B "$here/.hp/aaa_probe_compare.py" "$out" > "$out/compare.txt"
cp "$out/compare.txt" ../hp-aaa-compare.txt; tail -3 "$out/compare.txt"
for round in 1 2; do for side in before after; do echo "$side"; GUI_FORMS_PAINT_TALL=1 "$b/hp-probe-$side/gui_forms_paint_profile_workload" "$out/tall-run"; done; done
uptime
