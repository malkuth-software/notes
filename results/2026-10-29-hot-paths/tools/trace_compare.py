# SCRATCH (cover hint): per Gallery page, whether the after trace is the
# before trace with commands removed only, and what was removed.
import collections
import sys
from pathlib import Path

before_dir, after_dir = Path(sys.argv[1]), Path(sys.argv[2])
pages = sorted(p.name for p in before_dir.glob('*.trace'))
total_removed = collections.Counter()
for page in pages:
    before = (before_dir / page).read_text().splitlines()
    after = (after_dir / page).read_text().splitlines()
    removed = []
    i = 0
    subsequence = True
    for line in after:
        while i < len(before) and before[i] != line:
            removed.append(before[i])
            i += 1
        if i == len(before):
            subsequence = False
            break
        i += 1
    removed.extend(before[i:])
    kinds = collections.Counter(line.split(' ')[0] for line in removed)
    total_removed.update(kinds)
    status = 'removals only' if subsequence else 'NOT A SUBSEQUENCE'
    print(f'{page}: before {len(before)} after {len(after)} {status}; removed {dict(kinds)}')
print('pages', len(pages), 'removed in all', dict(total_removed))
