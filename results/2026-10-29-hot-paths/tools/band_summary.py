"""SCRATCH (B1): summarize band_proto cases output.

Per group (damage cases by scale/percent, pages by scale): sums of medians of
direct paint, record, serial replay, and for each threads x bands config the
banded wall time, the projected frame (record + banded wall) and its speed-up
over direct, the busy-time inflation over serial replay, and pixel identity.
"""
import json
import sys

rows = [json.loads(line) for line in open(sys.argv[1]) if line.startswith('{')]
def report(name, items):
    direct = sum(r['direct_ns'] for r in items)
    record = sum(r['record_ns'] for r in items)
    replay = sum(r['replay1_ns'] for r in items)
    identical = all(r['replay_identical'] for r in items)
    print('%s: cases %d, direct %.2f ms, record %.2f ms, replay1 %.2f ms (replay share %.0f%%), replay identical %s'
          % (name, len(items), direct / 1e6, record / 1e6, replay / 1e6, 100.0 * replay / max(direct, 1), identical))
    configs = len(items[0]['banded'])
    for index in range(configs):
        config = items[0]['banded'][index]
        wall = sum(r['banded'][index]['wall_ns'] for r in items)
        busy = sum(r['banded'][index]['busy_per_frame_ns'] for r in items)
        same = all(r['banded'][index]['identical'] for r in items)
        print('  T%-2d B%-2d wall %.2f ms, record+wall %.2f ms, speed-up %.2fx, replay speed-up %.2fx, busy/replay1 %.2f, identical %s'
              % (config['threads'], config['bands'], wall / 1e6, (record + wall) / 1e6, direct / max(record + wall, 1),
                 replay / max(wall, 1), busy / max(replay, 1), same))

# The first six rows are the damage phase (scales 1, 2, 3 x 10 %, 100 %).
for index in range(6):
    report('damage scale %g %g%%' % (rows[index]['scale'], rows[index]['damage']), [rows[index]])
pages = rows[6:]
for scale in (1, 2):
    report('pages scale %g' % scale, [r for r in pages if r['scale'] == scale])
