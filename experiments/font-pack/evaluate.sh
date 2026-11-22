#!/bin/sh
# Measures the experiment on the bundled fonts. Run on the M4 through m4.sh:
#   sh m4.sh 'sh evaluate.sh'
# The single-group baseline (build/all.fpk) is held to the four conditions:
#   1. one pack smaller than 50% of the fonts' bytes
#   2. packed in at most 2 minutes
#   3. loaded (pack bytes to every font's bytes) within 50 ms
#   4. loader scratch at most 200% of the fonts' bytes
# The grouped pack (build/groups.fpk, from fonts.manifest) is reported beside
# it: the whole set, and the eager groups alone, which is what a start pays.
# Loads are timed in fresh processes, one load each, as an application would do it.
# bench_groups.txt holds the full per-group bench (ratios, 1/4/all threads, scratch).
set -e
sh build.sh >/dev/null
build/fontpack pack build/all.fpk fonts/* | tail -2 > build/evaluate-pack.txt
cat build/evaluate-pack.txt
grep -q "verified: the pack loads to the identical bytes" build/evaluate-pack.txt
build/fontpack pack build/groups.fpk --manifest fonts.manifest fonts/* | tail -2 > build/evaluate-pack-groups.txt
cat build/evaluate-pack-groups.txt
grep -q "verified: the pack loads to the identical bytes" build/evaluate-pack-groups.txt
: > build/evaluate-loads.txt
: > build/evaluate-loads-groups.txt
: > build/evaluate-loads-eager.txt
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
    build/fontpack bench build/all.fpk --threads 0 --rounds 1 --only all >> build/evaluate-loads.txt
    build/fontpack bench build/groups.fpk --threads 0 --rounds 1 --only all >> build/evaluate-loads-groups.txt
    build/fontpack bench build/groups.fpk --threads 0 --rounds 1 --only eager >> build/evaluate-loads-eager.txt
done
build/fontpack bench build/groups.fpk --baseline build/all.fpk > build/bench_groups.txt
python3 - <<'PY'
import re
def pack_line(path):
    m = re.search(r'(\d+) -> (\d+) bytes \(([\d.]+)%\) in ([\d.]+) s', open(path).read())
    return int(m.group(1)), int(m.group(2)), float(m.group(3)), float(m.group(4))
def loads(path):
    text = open(path).read()
    times = sorted(float(x) for x in re.findall(r'rounds 1: min ([\d.]+) ms', text))
    scratch = max(float(x) for x in re.findall(r'\(([\d.]+)% of the fonts\)', text))
    return times, scratch
def line(name, value, limit, ok):
    verdict = '' if ok is None else ('PASS' if ok else 'FAIL')
    print('%-44s %-22s limit %-10s %s' % (name, value, limit, verdict))
fonts, packed, percent, seconds = pack_line('build/evaluate-pack.txt')
times, scratch = loads('build/evaluate-loads.txt')
line('pack size / font bytes', '%.2f%% (%d of %d)' % (percent, packed, fonts), '< 50%', percent < 50)
line('pack time', '%.1f s' % seconds, '120 s', seconds <= 120)
line('load, 20 fresh processes: median', '%.1f ms' % times[len(times) // 2], '50 ms', times[len(times) // 2] <= 50)
line('load, 20 fresh processes: slowest', '%.1f ms' % times[-1], '50 ms', times[-1] <= 50)
line('loader scratch / font bytes', '%.1f%%' % scratch, '200%', scratch <= 200)
print('grouped pack (fonts.manifest):')
fonts, packed, percent, seconds = pack_line('build/evaluate-pack-groups.txt')
times, scratch = loads('build/evaluate-loads-groups.txt')
eager, eager_scratch = loads('build/evaluate-loads-eager.txt')
line('  pack size / font bytes', '%.2f%% (%d of %d)' % (percent, packed, fonts), '-', None)
line('  pack time', '%.1f s' % seconds, '-', None)
line('  all groups, 20 fresh processes: median', '%.1f ms' % times[len(times) // 2], '-', None)
line('  eager groups, 20 fresh processes: median', '%.1f ms' % eager[len(eager) // 2], '-', None)
line('  eager groups: slowest', '%.1f ms' % eager[-1], '-', None)
line('  all groups: largest group scratch / all fonts', '%.1f%%' % scratch, '-', None)
line('  eager scratch / eager fonts', '%.1f%%' % eager_scratch, '-', None)
PY
