# SCRATCH (cover hint): compare paint workload records before and after.
# Usage: cover_compare.py <out-dir> <rounds>
import json
import statistics
import sys
from pathlib import Path

out = Path(sys.argv[1])
rounds = int(sys.argv[2])


def load(side: str, round_number: int) -> dict:
    records = {}
    for line in (out / f'{side}-{round_number}' / 'paint-profile.jsonl').read_text().splitlines():
        record = json.loads(line)
        key = (record['phase'], record['page'], record['scale'], record['damage_percent'])
        records[key] = record
    return records


before = [load('before', r) for r in range(1, rounds + 1)]
after = [load('after', r) for r in range(1, rounds + 1)]
mismatches = 0
for runs in (before, after):
    for records in runs[1:]:
        for key, record in records.items():
            if record['pixel_hash'] != runs[0][key]['pixel_hash']:
                print('UNSTABLE', key)
for key, record in before[0].items():
    if after[0][key]['pixel_hash'] != record['pixel_hash']:
        mismatches += 1
        print('PIXELS DIFFER', key)
print(f'cases {len(before[0])}, pixel hash mismatches {mismatches}')
for phase in ('damage', 'pages'):
    keys = [k for k in before[0] if k[0] == phase]
    sums = {}
    for name, runs in (('before', before), ('after', after)):
        # Per case, the median over rounds of the median frame time.
        sums[name] = sum(statistics.median(r[k]['frame_ns']['median'] for r in runs) for k in keys)
    print(f'{phase}: {len(keys)} cases, sum of median frame ns before {sums["before"] / 1e6:.2f} ms '
          f'after {sums["after"] / 1e6:.2f} ms ratio {sums["after"] / sums["before"]:.3f}')
    for scale in sorted({k[2] for k in keys}):
        subset = [k for k in keys if k[2] == scale]
        b = sum(statistics.median(r[k]['frame_ns']['median'] for r in before) for k in subset)
        a = sum(statistics.median(r[k]['frame_ns']['median'] for r in after) for k in subset)
        print(f'  scale {scale}: before {b / 1e6:.2f} ms after {a / 1e6:.2f} ms ratio {a / b:.3f}')
counters = ('controls_painted', 'paint_nodes_visited', 'display_commands_replayed')
for name in counters:
    b = sum(r['counters'][name] for r in before[0].values())
    a = sum(r['counters'][name] for r in after[0].values())
    print(f'{name}: before {b} after {a}')
