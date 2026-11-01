#!/bin/sh
# Render a key's press as N in-between frames with the bfft engine, every
# frame an independent process, all in parallel; then pack them as a delta
# animation (tools/pack_press.py).
# Usage: sh tools/engine_press.sh <cancel|ok> <frames> <output directory> <lens mesh> [sidewall] [width height]
# Three passes per frame: the lamps (unit power), the coverage, and the faint
# fill light (lamps off).
set -e
here=$(cd "$(dirname "$0")/.." && pwd)
engine="$here/bfft/experiments/photonic_transport_field/native/regime_scene_native"
style="$1"; frames="$2"; out="$3"; mesh="$4"; sidewall="${5:-0.12}"; width="${6:-280}"; height="${7:-174}"; mkdir -p "$out"
jobs=$(sysctl -n hw.ncpu)
i=0
: > "$out/jobs.txt"
while [ $i -lt "$frames" ]; do
  press=$(python3 -c "print(0.04*$i/($frames-1))")
  common="$engine --scene key --key-style $style --key-press $press --key-mesh $mesh --key-sidewall $sidewall --max-primitives 20000 --max-build-seconds 900 --width $width --height $height"
  echo "$common --out $out/f${i}_light.ppm --out-linear $out/f${i}_light.f32" >> "$out/jobs.txt"
  echo "$common --key-pass coverage --out $out/f${i}_coverage.ppm --out-linear $out/f${i}_coverage.f32" >> "$out/jobs.txt"
  echo "$common --key-lamp -2 --key-fill on --out $out/f${i}_fill.ppm --out-linear $out/f${i}_fill.f32" >> "$out/jobs.txt"
  i=$((i+1))
done
start=$(python3 -c "import time; print(time.time())")
tr '\n' '\0' < "$out/jobs.txt" | xargs -0 -P "$jobs" -n 1 sh -c 'eval "$0" > /dev/null'
python3 -c "import time; print('rendered %d frames x 3 passes in %.2f s on %d processes' % ($frames, time.time()-$start, $jobs))"
python3 "$here/tools/pack_press.py" "$out" "$frames" "$out/${style}_press.anim"
