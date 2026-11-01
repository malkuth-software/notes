# SCRATCH: per-case pixel differences between two isa-workload raw files.
import struct, sys
def cases(path):
    data = open(path, 'rb').read(); i = 0; out = {}
    while i < len(data):
        name = data[i:i+32].rstrip(b'\0').decode(); w, h = struct.unpack('<II', data[i+32:i+40]); i += 40
        out[name] = (w, h, data[i:i + w*h*4]); i += w*h*4
    return out
a, b = cases(sys.argv[1]), cases(sys.argv[2])
for name in a:
    w, h, pa = a[name]; pb = b[name][2]
    if pa == pb: print(name, 'identical'); continue
    n = 0; rows = set(); peak = 0
    for p in range(w*h):
        x, y = p % w, p // w
        if pa[p*4:p*4+4] != pb[p*4:p*4+4]:
            n += 1; rows.add(y); peak = max(peak, max(abs(s-t) for s, t in zip(pa[p*4:p*4+4], pb[p*4:p*4+4])))
    print(name, 'differs px', n, 'max', peak, 'rows', sorted(rows)[:10], '...', sorted(rows)[-5:])
