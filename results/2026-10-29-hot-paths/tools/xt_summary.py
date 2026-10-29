#!/usr/bin/env python3
"""Scratch (hot-paths track, not committed): summarize an xctrace Time
Profiler trace: self and inclusive shares per function, area shares, and the
call paths that lead to chosen leaf functions.

  xt_summary.py TRACE --top N [--paths REGEX --depth D --path-top M]
"""
from __future__ import annotations
import argparse
import collections
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

SOURCE: Path = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SOURCE / 'tools'))
import profile_hot_paths  # noqa: E402

RUNTIME_BINARIES = re.compile(r'^(?:libsystem_|libc\+\+|libdyld|dyld|libobjc|libmalloc)')


def area(symbol: str, binary: str) -> str:
    name: str = profile_hot_paths.area_of(symbol, binary)
    if name == 'other' and RUNTIME_BINARIES.search(binary):
        return 'runtime'
    return name


def samples(trace: Path, process_filter: str | None = None):
    """Yield (weight, [frame names leaf first], [binaries]) per sample."""
    export = subprocess.Popen(['xcrun', 'xctrace', 'export', '--input', str(trace), '--xpath',
                               '/trace-toc/run[@number="1"]/data/table[@schema="time-profile"]'],
                              stdout=subprocess.PIPE)
    frames: dict[str, tuple[str, str]] = {}
    binaries: dict[str, str] = {}
    backtraces: dict[str, list[str]] = {}
    weights: dict[str, int] = {}
    processes: dict[str, str] = {}
    for event, element in ET.iterparse(export.stdout, events=('end',)):
        tag: str = element.tag
        if tag == 'binary':
            if element.get('id') is not None:
                binaries[element.get('id')] = element.get('name', '?')
        elif tag == 'frame':
            if element.get('id') is not None:
                binary_element = element.find('binary')
                binary: str = '?'
                if binary_element is not None:
                    reference = binary_element.get('ref') or binary_element.get('id')
                    binary = binaries.get(reference, binary_element.get('name', '?'))
                frames[element.get('id')] = (element.get('name') or element.get('addr', '?'), binary, int(element.get('addr', '0x0'), 16))
        elif tag == 'process':
            if element.get('id') is not None:
                processes[element.get('id')] = element.get('fmt', '')
        elif tag == 'weight':
            if element.get('id') is not None:
                weights[element.get('id')] = int(element.text or '1')
        elif tag == 'backtrace':
            if element.get('id') is not None:
                ids: list[str] = []
                for frame in element.findall('frame'):
                    ids.append(frame.get('ref') or frame.get('id'))
                backtraces[element.get('id')] = ids
        elif tag == 'tagged-backtrace':
            if element.get('id') is not None:
                inner = element.find('backtrace')
                if inner is not None:
                    backtraces['t' + element.get('id')] = backtraces.get(inner.get('ref') or inner.get('id'), [])
        elif tag == 'row':
            process_element = element.find('process')
            if process_filter is not None and process_element is not None:
                owner = processes.get(process_element.get('ref') or process_element.get('id'), '')
                if process_filter not in owner:
                    element.clear()
                    continue
            backtrace_element = element.find('tagged-backtrace')
            weight_element = element.find('weight')
            weight: int = 1
            if weight_element is not None:
                reference = weight_element.get('ref') or weight_element.get('id')
                weight = weights.get(reference, 1)
            if backtrace_element is not None:
                reference = backtrace_element.get('ref') or backtrace_element.get('id')
                ids = backtraces.get('t' + reference, [])
                yield weight, [frames[i][0] for i in ids], [frames[i][1] for i in ids], [frames[i][2] for i in ids]
            element.clear()
    export.wait()


class Resolver:
    """Map addresses in the profiled executable to their containing function through nm (the
    trace's own symbolication names inlined callees and misses some frames)."""

    def __init__(self, binary: str):
        import bisect
        self.bisect = bisect
        listing = subprocess.run(['nm', '-n', '-C', binary], capture_output=True, text=True).stdout
        self.starts: list[int] = []
        self.names: list[str] = []
        exact: dict[str, int] = {}
        for line in listing.splitlines():
            parts = line.split(' ', 2)
            if len(parts) == 3 and parts[0] and len(parts[1]) == 1 and parts[1] in 'tT':
                self.starts.append(int(parts[0], 16))
                self.names.append(parts[2])
                exact[parts[2]] = int(parts[0], 16)
        self.exact = exact
        self.end = self.starts[-1] + 0x100000 if self.starts else 0
        self.slide = None
        self.votes: collections.Counter = collections.Counter()

    def vote(self, name: str, address: int) -> None:
        start = self.exact.get(name)
        if start is not None and address > start:
            self.votes[(address - start) & ~0x3fff] += 1

    def name(self, address: int, fallback: str) -> str:
        if self.slide is None:
            self.slide = self.votes.most_common(1)[0][0] if self.votes else 0
        local = address - 1 - self.slide
        if not self.starts or local < self.starts[0] or local > self.end:
            return fallback
        index = self.bisect.bisect_right(self.starts, local) - 1
        return self.names[index]


def short(name: str, width: int = 110) -> str:
    return name if len(name) <= width else name[:width - 3] + '...'


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('trace')
    parser.add_argument('--binary', required=True)
    parser.add_argument('--process', default=None)
    parser.add_argument('--top', type=int, default=30)
    parser.add_argument('--paths', default=None)
    parser.add_argument('--depth', type=int, default=4)
    parser.add_argument('--path-top', type=int, default=25)
    parser.add_argument('--skip', default=r'^(?:Sk|sk_|neon::|portable::|SkOpts|\(anonymous namespace\)::Sk|'
                        r'std::|void std::|gui_forms::render::SkiaPainter::fill_rect_device|SkScan|'
                        r'SkRasterPipelineBlitter|SkCanvas|SkBitmapDevice|SkDraw|SkBlitter|SkRectClipBlitter)')
    arguments = parser.parse_args()
    total: int = 0
    self_weight: collections.Counter = collections.Counter()
    inclusive_weight: collections.Counter = collections.Counter()
    binary_of: dict[str, str] = {}
    path_weight: collections.Counter = collections.Counter()
    path_total: int = 0
    target = re.compile(arguments.paths) if arguments.paths else None
    skip = re.compile(arguments.skip)
    resolver = Resolver(arguments.binary)
    recorded = []
    for weight, names, binaries, addresses in samples(Path(arguments.trace), arguments.process):
        for name, address in zip(names, addresses):
            resolver.vote(name, address)
        recorded.append((weight, names, binaries, addresses))
    for weight, names, binaries, addresses in recorded:
        if not names:
            continue
        names = [resolver.name(a, n) for n, a in zip(names, addresses)]
        total += weight
        self_weight[names[0]] += weight
        binary_of.setdefault(names[0], binaries[0])
        for name, binary in zip(names, binaries):
            binary_of.setdefault(name, binary)
        for name in set(names):
            inclusive_weight[name] += weight
        if target is not None and target.search(names[0]):
            path_total += weight
            kept: list[str] = []
            for name in names[1:]:
                if skip.search(name) or not name.startswith(('gui_forms::', '(anonymous namespace)')):
                    continue
                kept.append(re.sub(r'\(.*$', '', name))
                if len(kept) >= arguments.depth:
                    break
            path_weight[' <- '.join(kept)] += weight
    total_count = total
    print(f'samples: {sum(1 for r in recorded if r[1])}; slide {resolver.slide:#x}')
    areas: collections.Counter = collections.Counter()
    for name, weight in self_weight.items():
        areas[area(name, binary_of.get(name, '?'))] += weight
    print('areas (self): ' + ', '.join(f'{a} {100.0 * w / total:.2f}%' for a, w in areas.most_common()))
    print('| # | Self | Incl | Area | Function |')
    for index, (name, weight) in enumerate(self_weight.most_common(arguments.top), 1):
        print(f'| {index} | {100.0 * weight / total:.2f}% | {100.0 * inclusive_weight[name] / total:.2f}% | '
              f'{area(name, binary_of.get(name, "?"))} | {short(name)} |')
    if target is not None:
        print(f'\nleaf {arguments.paths}: {100.0 * path_total / max(total, 1):.2f}% of samples; '
              f'GUI.Forms call paths (nearest first), share of those samples:')
        for path, weight in path_weight.most_common(arguments.path_top):
            print(f'  {100.0 * weight / max(path_total, 1):6.2f}%  {path}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
