# SCRATCH: per case, before/after pixel differences and the band checks.
import sys
from pathlib import Path
out = Path(sys.argv[1])
def probe(side):
    rows = {}
    for line in (out / side / 'probe.txt').read_text().split('\n'):
        if line:
            parts = line.split()
            rows[parts[0]] = tuple(parts[4:])
    return rows
def load(path):
    data = path.read_bytes()
    head, _, body = data.partition(b'\n')
    w, h, rb = (int(v) for v in head.split())
    return w, h, rb, body
pb, pa = probe('before'), probe('after')
totals = {'moved': 0, 'same': 0}
for raw in sorted(r for r in (out / 'after').glob('*.raw') if '.columns' not in r.name):
    name = raw.stem
    w, h, rb, after = load(raw)
    _, _, _, before = load(out / 'before' / raw.name)
    count = 0; peak = 0; rows = set(); cols = set()
    for y in range(h):
        a = after[y * rb:y * rb + w * 4]; b = before[y * rb:y * rb + w * 4]
        if a == b: continue
        for x in range(w):
            pa4 = a[x * 4:x * 4 + 4]; pb4 = b[x * 4:x * 4 + 4]
            if pa4 != pb4:
                count += 1; rows.add(y); cols.add(x)
                peak = max(peak, max(abs(p - q) for p, q in zip(pa4, pb4)))
    totals['moved' if count else 'same'] += 1
    rlist = sorted(rows)
    print(f"{name:40s} {w}x{h} diff_px={count:6d} max_delta={peak:3d} rows={len(rlist):4d} "
          f"first_rows={rlist[:6]} | before {' '.join(pb[name])} | after {' '.join(pa[name])}")
print(totals)
# Column seams: a frame repainted in uneven column bands against the whole frame.
for side in ('before', 'after'):
    for raw in sorted((out / side).glob('*.columns.raw')):
        name = raw.name[:-len('.columns.raw')]
        w, h, rb, cols_px = load(raw)
        _, _, _, whole = load(out / side / (name + '.raw'))
        count = 0; peak = 0; xs = set()
        for y in range(h):
            a = cols_px[y * rb:y * rb + w * 4]; b = whole[y * rb:y * rb + w * 4]
            if a == b: continue
            for x in range(w):
                if a[x * 4:x * 4 + 4] != b[x * 4:x * 4 + 4]:
                    count += 1; xs.add(x)
                    peak = max(peak, max(abs(p - q) for p, q in zip(a[x * 4:x * 4 + 4], b[x * 4:x * 4 + 4])))
        print(f"COLUMNS {side} {name:40s} diff_px={count} max_delta={peak} columns={sorted(xs)[:12]}")
