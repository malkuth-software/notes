#!/bin/sh
set -u
sh .hp/glyph_check.sh
echo "== tests plain"
python3 tools/ci_build.py --jobs 8 --tests "gui_forms_glyph_run_raster_tests" > ../hp-f1-plain.log 2>&1; echo "exit $?"; grep -E "tests passed|tests failed|Failed|\*\*\*" ../hp-f1-plain.log | tail -5
echo "== tests sanitize"
python3 tools/ci_build.py --jobs 8 --sanitize --tests "gui_forms_glyph_run_raster_tests" > ../hp-f1-san.log 2>&1; echo "exit $?"; grep -E "tests passed|tests failed|Failed|ERROR|runtime error|\*\*\*" ../hp-f1-san.log | tail -5
