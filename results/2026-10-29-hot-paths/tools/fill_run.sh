#!/bin/sh
# SCRATCH (hot-paths track): build the fill-trace instrumented paint workload
# from a patched copy of this checkout (../hp-src) and run it once.
set -eu
here=$(pwd -P)
copy="$here/../hp-src"
mkdir -p "$copy"
rsync -a --delete --exclude=/.git "$here/" "$copy/"
cd "$copy"
patch -p1 -s < .hp/fill_trace.patch
python3 -B .hp/m4_profile.py build --tree gui-forms-hp-fill > "$here/../hp-fill-build.log" 2>&1
out="$here/../.build/native-macos-arm64/hp-fill"
mkdir -p "$out"
HP_FILL_OUT="$out/fill.tsv" "$here/../.build/native-macos-arm64/gui-forms-hp-fill/gui_forms_paint_profile_workload" "$out/workload" > /dev/null
python3 -B .hp/fill_summary.py "$out/fill.tsv"
