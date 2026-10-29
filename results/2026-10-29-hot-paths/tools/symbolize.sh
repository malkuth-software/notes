#!/bin/sh
# SCRATCH: make dSYMs for the profile binaries and symbolicate each trace, then summarize.
set -eu
o=$(pwd -P)/../.build/native-macos-arm64/hp-out
b=$(pwd -P)/../.build/native-macos-arm64/gui-forms-profile
for t in gui_forms_paint_profile_workload gui_forms_text_engine_benchmark gui_forms_document_view_benchmark; do
    [ -d "$b/$t.dSYM" ] || dsymutil "$b/$t" -o "$b/$t.dSYM" 2>/dev/null
done
for w in paint:gui_forms_paint_profile_workload text_engine:gui_forms_text_engine_benchmark document_view:gui_forms_document_view_benchmark; do
    n=${w%%:*}; t=${w#*:}
    [ -d "$o/$n-sym.trace" ] && continue
    xcrun xctrace symbolicate --input "$o/$n.trace" --output "$o/$n-sym.trace" --dsym "$b/$t.dSYM" > /dev/null 2>&1 || true
done
python3 -B .hp/xt_summary.py --binary "$b/gui_forms_paint_profile_workload" "$o/paint-sym.trace" --top 30 --paths 'rect_memset32' --depth 4 > "$o/paint.txt" 2>&1
python3 -B .hp/xt_summary.py --binary "$b/gui_forms_text_engine_benchmark" "$o/text_engine-sym.trace" --top 20 > "$o/text_engine.txt" 2>&1
python3 -B .hp/xt_summary.py --binary "$b/gui_forms_document_view_benchmark" "$o/document_view-sym.trace" --process gui_forms_document_view_benchmark --top 12 > "$o/document_view.txt" 2>&1
echo done
