# SCRATCH: differing pixels of one isa case, per 4x3 grid cell (the paths case's layout).
import struct, sys
def cases(path):
    data = open(path, 'rb').read(); i = 0; out = {}
    while i < len(data):
        name = data[i:i+32].rstrip(b'\0').decode(); w, h = struct.unpack('<II', data[i+32:i+40]); i += 40
        out[name] = (w, h, data[i:i + w*h*4]); i += w*h*4
    return out
a, b = cases(sys.argv[1]), cases(sys.argv[2]); name = sys.argv[3]
w, h, pa = a[name]; pb = b[name][2]
cells = {}
for p in range(w*h):
    if pa[p*4:p*4+4] != pb[p*4:p*4+4]:
        x, y = p % w, p // w
        c = (int(x // (w/4)), int(y // (h/3)))
        e = cells.setdefault(c, [0, w, h, 0, 0, 0]); e[0] += 1
        e[1] = min(e[1], x); e[2] = min(e[2], y); e[3] = max(e[3], x); e[4] = max(e[4], y)
        e[5] = max(e[5], max(abs(s-t) for s, t in zip(pa[p*4:p*4+4], pb[p*4:p*4+4])))
for c in sorted(cells): print('cell', c, 'px %d x %d-%d y %d-%d max %d' % tuple(cells[c]))
