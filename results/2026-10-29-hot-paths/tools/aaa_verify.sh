#!/bin/sh
# SCRATCH (AAA clip rows): touched tests (plain or --sanitize), then optionally the paint workload check.
# Usage: sh .hp/cover_verify.sh plain|sanitize|measure
set -u
tests="'gui_forms_(skia_damage_edge|skia_edge_snapping|backplane_replay_identity|canvas_raster|engine_gate|layered_material_raster|opaque_live_surface_raster|part_composite_identity|prepared_text_skia|retained_draw_cache_raster|text_identity_raster|gallery_pages|gallery_display_trace|png_pixels|resource_pixels)_tests|gui_forms_skia_smoke|gui_drawing_skia_tests'"
case "$1" in
plain) eval python3 tools/ci_build.py --jobs 8 --gallery --tests "$tests"; echo "exit $?";;
sanitize) eval python3 tools/ci_build.py --jobs 8 --gallery --sanitize --tests "$tests"; echo "exit $?";;
measure) sh .hp/cover_check.sh 3; echo "exit $?";;
esac
case "$1" in
all)
    eval python3 tools/ci_build.py --jobs 8 --gallery --tests "$tests" > ../hp-aaa-plain.log 2>&1; echo "plain exit $?"; grep -E "tests passed|\*\*\*" ../hp-aaa-plain.log
    eval python3 tools/ci_build.py --jobs 8 --gallery --sanitize --tests "$tests" > ../hp-aaa-sanitize.log 2>&1; echo "sanitize exit $?"; grep -E "tests passed|\*\*\*" ../hp-aaa-sanitize.log
    python3 -B ../template_harness/harness/check_house_style.py include src tests tools demo > ../hp-aaa-style.log 2>&1; echo "house style exit $?"; tail -5 ../hp-aaa-style.log
    python3 -B ../template_harness/harness/check_lifetime_rules.py include src tests tools demo > ../hp-aaa-lifetime.log 2>&1; echo "lifetime exit $?"; tail -5 ../hp-aaa-lifetime.log
    python3 -B ../template_harness/harness/check_state_machines.py --root . --build-dir ../.build/native-macos-arm64/gui-forms --jobs 8 --gate --changed-from origin/main > ../hp-aaa-gate.log 2>&1; echo "gate exit $?"; tail -5 ../hp-aaa-gate.log
    python3 -B tools/check_stx_skia_style.py > ../hp-aaa-stx-style.log 2>&1; echo "stx_skia style exit $?"; tail -5 ../hp-aaa-stx-style.log
    python3 tools/generate_library_docs.py --check > ../hp-aaa-docs.log 2>&1; echo "docs check exit $?"; tail -5 ../hp-aaa-docs.log;;
esac
