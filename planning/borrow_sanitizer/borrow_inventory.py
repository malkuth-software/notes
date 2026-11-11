#!/usr/bin/env python3
"""Borrow inventory scanner (borrow sanitizer study, 2026-11-11).

Scans first-party headers and sources for functions that return a borrowed
view (std::span, std::string_view, const char*, references or pointers to
members, iterators) and for members that store one. Writes a TSV and prints
counts per kind and area. Usage counts are word matches of `name(` across
include src demo tests examples file_manager_demoboard (approximate).

  python3 -B borrow_inventory.py <gui_forms root> <out dir>
"""
import collections, os, re, subprocess, sys

root, out = sys.argv[1], sys.argv[2]
DIRS = ['include', 'src']
USE_DIRS = ['include', 'src', 'demo', 'tests', 'examples', 'file_manager_demoboard', 'bindings', 'tools']

RET = [
    ('span', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|static\s+|inline\s+|constexpr\s+|friend\s+)*(std::span<[^;{()]*>)\s+(\w+)\s*\(')),
    ('string_view', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|static\s+|inline\s+|constexpr\s+)*(std::(?:u8|u16|u32|w)?string_view)\s+(\w+)\s*\([^)]*\)\s*(?:const)?')),
    ('cref_container', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|static\s+|inline\s+|constexpr\s+)*(const\s+std::(?:vector|string|u8string|u16string|u32string|map|unordered_map|set|deque|array|optional)\b[^;{()]*&)\s*(\w+)\s*\(')),
    ('mref_container', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|static\s+|inline\s+)*(std::(?:vector|string|map|unordered_map|deque)\b[^;{()]*&)\s*(\w+)\s*\(')),
    ('iterator', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|static\s+|inline\s+)*((?:typename\s+)?[\w:<>, ]*(?:const_)?iterator)\s+(\w+)\s*\(')),
    ('backlink_ptr', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|static\s+|inline\s+)*((?:const\s+)?(?:Window|SurfaceRoot|Control|Host|HostWindow|Application|Form|Component|ContainerControl|ControlTable|Document|ThemeEngine|Dispatcher)\s*[*&])\s*(\w+)\s*\([^)]*\)\s*(?:const)?\s*(?:noexcept)?\s*[;{]')),
    ('const_char_ptr', re.compile(r'^\s*(?:GF_\w+\s+|GUI_\w+\s+)*(const\s+(?:char|char8_t|uint8_t|std::uint8_t|float|double|int32_t|uint32_t|void)\s*\*)\s*(?:GF_\w+\s+)?(\w+)\s*\(')),
    ('other_ref', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|inline\s+)*(const\s+[A-Z]\w*(?:::\w+)*\s*&)\s*(\w+)\s*\(\s*\)\s*const')),
    ('other_ptr', re.compile(r'^\s*(?:\[\[nodiscard\]\]\s*)?(?:virtual\s+|inline\s+)*((?:const\s+)?[A-Z]\w*(?:::\w+)*\s*\*)\s*(\w+)\s*\([^)]*\)\s*(?:const)?\s*(?:noexcept)?\s*[;{]')),
]
FIELD = [
    ('field_span', re.compile(r'^\s*(?:const\s+)?(std::span<[^;()]*>)\s+(\w+)\s*(?:\{[^}]*\})?\s*;')),
    ('field_string_view', re.compile(r'^\s*(?:const\s+)?(std::(?:u8|u16)?string_view)\s+(\w+)\s*(?:\{[^}]*\}|=\s*[^;]*)?;')),
    ('field_backlink', re.compile(r'^\s*((?:const\s+)?(?:Window|SurfaceRoot|Control|Host|HostWindow|Application|Form|Component|ContainerControl|ControlTable|Document|Dispatcher)\s*\*)\s*(\w+_?)\s*(?:\{[^}]*\}|=\s*nullptr)?\s*;')),
    ('field_raw_ptr', re.compile(r'^\s*((?:const\s+)?[A-Z]\w*(?:::\w+)*\s*\*)\s*(\w+_)\s*(?:\{[^}]*\}|=\s*nullptr)?\s*;')),
]
SKIP_NAMES = {'operator', 'if', 'for', 'while', 'return', 'switch'}
rows = []
for d in DIRS:
    for dirpath, _, files in os.walk(os.path.join(root, d)):
        for f in files:
            if not f.endswith(('.hpp', '.h', '.cpp', '.mm')):
                continue
            p = os.path.join(dirpath, f)
            rel = os.path.relpath(p, root)
            try:
                lines = open(p, encoding='utf-8', errors='replace').read().split('\n')
            except OSError:
                continue
            for i, line in enumerate(lines, 1):
                if line.lstrip().startswith(('//', '*', 'return', '#')):
                    continue
                for kind, rx in RET + FIELD:
                    m = rx.match(line)
                    if m and m.group(2) not in SKIP_NAMES:
                        rows.append((kind, rel, i, m.group(1).strip(), m.group(2)))
                        break
# usage counts by name (one grep for all names)
names = sorted({r[4] for r in rows if not r[0].startswith('field')})
counts = collections.Counter()
pat = re.compile(r'\b(' + '|'.join(re.escape(n) for n in names) + r')\s*\(') if names else None
if pat:
    for d in USE_DIRS:
        for dirpath, _, files in os.walk(os.path.join(root, d)):
            for f in files:
                if f.endswith(('.hpp', '.h', '.cpp', '.mm', '.c')):
                    try:
                        text = open(os.path.join(dirpath, f), encoding='utf-8', errors='replace').read()
                    except OSError:
                        continue
                    for m in pat.finditer(text):
                        counts[m.group(1)] += 1
os.makedirs(out, exist_ok=True)
with open(os.path.join(out, 'inventory.tsv'), 'w') as fh:
    fh.write('kind\tfile\tline\ttype\tname\tuses\n')
    for r in rows:
        fh.write('\t'.join(map(str, r)) + '\t' + str(counts.get(r[4], 0) if not r[0].startswith('field') else '') + '\n')
by_kind = collections.Counter(r[0] for r in rows)
pub = collections.Counter(r[0] for r in rows if r[1].startswith('include/'))
print('rows', len(rows))
for k, v in by_kind.most_common():
    print(f'  {k}: {v} (public {pub[k]})')
area = collections.Counter('/'.join(r[1].split('/')[:2]) for r in rows if r[0] in ('span', 'string_view', 'cref_container', 'backlink_ptr', 'field_span', 'field_string_view', 'field_backlink'))
print('areas (span/sv/cref/backlink):', ', '.join(f'{a} {n}' for a, n in area.most_common(25)))
