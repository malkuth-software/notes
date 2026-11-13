#!/bin/sh
# Measures the experiment's four conditions on the bundled fonts. Run on the M4
# through m4.sh:  sh m4.sh 'sh evaluate.sh'
#   1. one shared pack smaller than 50% of the fonts' bytes
#   2. packed in at most 2 minutes
#   3. loaded (pack bytes to every font's bytes) within 50 ms
#   4. loader scratch at most 200% of the fonts' bytes
# The load is timed in fresh processes, one load each, as an application would do it.
set -e
sh build.sh >/dev/null
build/fontpack pack build/all.fpk fonts/* | tail -2 > build/evaluate-pack.txt
cat build/evaluate-pack.txt
grep -q "verified: the pack loads to the identical bytes" build/evaluate-pack.txt
: > build/evaluate-loads.txt
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
    build/fontpack bench build/all.fpk 0 1 >> build/evaluate-loads.txt
done
python3 - <<'PY'
import re
pack = open('build/evaluate-pack.txt').read()
m = re.search(r'(\d+) -> (\d+) bytes \(([\d.]+)%\) in ([\d.]+) s', pack)
fonts, packed, percent, seconds = int(m.group(1)), int(m.group(2)), float(m.group(3)), float(m.group(4))
loads = open('build/evaluate-loads.txt').read()
times = sorted(float(x) for x in re.findall(r'rounds 1: min ([\d.]+) ms', loads))
scratch = max(float(x) for x in re.findall(r'\(([\d.]+)% of the fonts\)', loads))
def line(name, value, limit, ok):
    print('%-44s %-22s limit %-10s %s' % (name, value, limit, 'PASS' if ok else 'FAIL'))
line('pack size / font bytes', '%.2f%% (%d of %d)' % (percent, packed, fonts), '< 50%', percent < 50)
line('pack time', '%.1f s' % seconds, '120 s', seconds <= 120)
line('load, 20 fresh processes: median', '%.1f ms' % times[len(times) // 2], '50 ms', times[len(times) // 2] <= 50)
line('load, 20 fresh processes: slowest', '%.1f ms' % times[-1], '50 ms', times[-1] <= 50)
line('loader scratch / font bytes', '%.1f%%' % scratch, '200%', scratch <= 200)
PY
