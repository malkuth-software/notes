#!/usr/bin/env python3
"""SCRATCH: summarize fill.tsv from the fill-trace build (area shares by recording call path)."""
import collections
import re
import sys

path = sys.argv[1]
depth = int(sys.argv[2]) if len(sys.argv) > 2 else 4
head = {}
rows = []
for line in open(path):
    if line.startswith('ROW\t'):
        _, kind, opaque, translucent, count, chain = line.rstrip('\n').split('\t')
        rows.append((kind, float(opaque), float(translucent), float(count), chain))
    else:
        key, value = line.split()
        head[key] = float(value)
damage = head['damage_device_px']
opaque_total = head['opaque_fill_px']
print(f"damage device px {damage:.3e}; opaque fill px {opaque_total:.3e} ({opaque_total / damage:.2f}x damage); "
      f"translucent {head['translucent_fill_px']:.3e} ({head['translucent_fill_px'] / damage:.2f}x)")
by_kind = collections.Counter()
for kind, opaque, translucent, count, chain in rows:
    by_kind[kind] += opaque
print('opaque by kind: ' + ', '.join(f'{k} {100 * v / opaque_total:.1f}%' for k, v in by_kind.most_common()))
for level in (2, depth):
    paths = collections.Counter()
    counts = collections.Counter()
    for kind, opaque, translucent, count, chain in rows:
        frames = [re.sub(r'^gui_forms::', '', f) for f in chain.split(' <- ')]
        key = kind + ': ' + ' <- '.join(frames[:level])
        paths[key] += opaque
        counts[key] += count
    print(f'\nopaque fill area by recording path (first {level} frames):')
    for key, value in paths.most_common(18):
        print(f'  {100 * value / opaque_total:6.2f}%  {value / damage:5.2f}x  n={counts[key]:.0f}  {key}')
