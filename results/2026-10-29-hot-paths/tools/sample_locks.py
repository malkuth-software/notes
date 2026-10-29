"""SCRATCH (B1): attribute macOS `sample` call-tree counts to lock waits.

For each thread in the sample, its total count and the counts of the deepest
frames whose text matches a lock symbol, with the nearest Skia/gui_forms caller.
"""
import re
import sys

LOCK = re.compile(r'psynch_mutexwait|semaphore_wait|SkSemaphore|SkMutex|__ulock_wait|os_unfair_lock|pthread_mutex_lock|_pthread_mutex_firstfit_lock_slow|psynch_cvwait')
line_re = re.compile(r'^(?P<indent>[\s+!:|]*)(?P<count>\d+)\s+(?P<text>.*)$')
for path in sys.argv[1:]:
    print('==', path)
    threads = []
    stack = []
    in_tree = False
    for raw in open(path, errors='replace'):
        if raw.startswith('Call graph:'):
            in_tree = True
            continue
        if in_tree and raw.strip() == '':
            if stack:
                in_tree = False
            continue
        if not in_tree:
            continue
        match = line_re.match(raw.rstrip('\n'))
        if not match:
            continue
        depth = len(match.group('indent'))
        count = int(match.group('count'))
        text = match.group('text')
        while stack and stack[-1][0] >= depth:
            stack.pop()
        stack.append((depth, text, count))
        if text.startswith('Thread_'):
            threads.append({'name': text[:60], 'total': count, 'locks': {}})
            continue
        if threads and LOCK.search(text) and not any(LOCK.search(t) for _, t, _ in stack[:-1]):
            caller = next((t for _, t, _ in reversed(stack[:-1]) if re.search(r'Sk|gui_forms|band|replay', t) and not LOCK.search(t)), '?')
            caller = re.sub(r'\s+\(in [^)]*\).*', '', caller)[:110]
            key = (re.sub(r'\s+\(in [^)]*\).*', '', text)[:40], caller)
            threads[-1]['locks'][key] = threads[-1]['locks'].get(key, 0) + count
    for thread in threads:
        waits = sum(thread['locks'].values())
        print('%s total %d lock %d (%.1f%%)' % (thread['name'], thread['total'], waits, 100.0 * waits / max(thread['total'], 1)))
        for (symbol, caller), count in sorted(thread['locks'].items(), key=lambda item: -item[1])[:6]:
            print('    %6d %s <- %s' % (count, symbol, caller))
