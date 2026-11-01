#!/bin/sh
# Render the dialog keys with the bfft engine (key scene) and convert them to
# the lamp cache format the dialog compositor reads.
# Usage: sh tools/engine_keys.sh <output directory>
set -e
here=$(cd "$(dirname "$0")/.." && pwd)
engine="$here/bfft/experiments/photonic_transport_field/native/regime_scene_native"
out="$1"; mkdir -p "$out"
for style in cancel ok; do
  for state in up down; do
    press=0; [ "$state" = down ] && press=0.04
    "$engine" --scene key --key-style $style --key-press $press --max-primitives 4000 --max-build-seconds 900 --width 280 --height 174 \
      --out "$out/${style}_${state}_lit.ppm" --out-linear "$out/${style}_${state}_lit.f32"
    "$engine" --scene key --key-style $style --key-press $press --key-pass coverage --max-primitives 4000 --max-build-seconds 900 --width 280 --height 174 \
      --out "$out/${style}_${state}_cover.ppm" --out-linear "$out/${style}_${state}_cover.f32"
    python3 "$here/tools/engine_to_cache.py" "$out/${style}_${state}_lit.f32" "$out/${style}_${state}_cover.f32" "$out/dialog_${style}_${state}.f32"
  done
done
