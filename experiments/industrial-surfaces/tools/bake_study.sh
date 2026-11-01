#!/bin/sh
# Bake study: time a lean_keys variant (median of three runs) and compare its
# bakes with the reference bakes (REF, default build/study/ref_snap: the
# renderer with hits snapped to the surface).
# Usage: sh tools/bake_study.sh <name> [lean_keys switches...]
set -e
name="$1"; shift
out="build/study/$name"; mkdir -p "$out"
times=""
for run in 1 2 3; do
  t=$(build/lean_keys "$out" 240 dialog "$@" | tee "$out/log.txt" | awk '/^total render/ {print $3}')
  times="$times $t"
done
median=$(echo $times | tr ' ' '\n' | sort -n | sed -n 2p)
refined=$(build/lean_keys "$out" 240 dialog "$@" | tee "$out/log.txt" | awk '/refined/ {s+=$8; n++} END {printf "%.1f", s/n}')
R=${REF:-build/study/ref_snap}
c=$(build/bake_compare 1 .12 .04 $R/dialog_cancel_up.f32 $R/dialog_cancel_down.f32 $out/dialog_cancel_up.f32 $out/dialog_cancel_down.f32)
o=$(build/bake_compare .16 1 .25 $R/dialog_ok_up.f32 $R/dialog_ok_down.f32 $out/dialog_ok_up.f32 $out/dialog_ok_down.f32)
printf "%-22s render %ss (runs:%s)  refined %s%%\n   CANCEL %s\n   OK     %s\n" "$name" "$median" "$times" "$refined" "$c" "$o"
