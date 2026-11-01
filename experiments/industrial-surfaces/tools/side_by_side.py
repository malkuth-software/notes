"""Side by side: lamp-cache bakes composited as demo.html does (over 397), one
row per bake, columns off and lit; scaled 2x. Usage:
side_by_side.py <out.png> <r> <g> <b> <label=dir/stem> ...  (stem without _up.f32)"""
import math, struct, sys, zlib
from array import array
def load(p):
    b = open(p, 'rb').read(); w, h, l = struct.unpack('<iii', b[:12]); f = array('f'); f.frombytes(b[12:]); n = w * h * 3
    return w, h, f[:n], [f[n * (i + 1):n * (i + 2)] for i in range(l)], f[n * (l + 1):n * (l + 1) + w * h]
def lin(c): c /= 255; return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
def enc(v): v = max(0, min(1, v)); return round(255 * (v * 12.92 if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055))
body = [lin(0x4d), lin(0x67), lin(0x6d)]
out, col = sys.argv[1], [float(v) for v in sys.argv[2:5]]
rows = []
for spec in sys.argv[5:]:
    w, h, base, glows, alpha = load(spec.split('=')[1] + '_up.f32')
    tiles = []
    for power in (0, 1):
        t = []
        for y in range(h):
            r = []
            for x in range(w):
                k = y * w + x; a = alpha[k]; inv = 1 / a if a > 1e-6 else 0
                px = []
                for c in range(3):
                    v = base[k * 3 + c] + sum(power * col[c] * g[k * 3 + c] for g in glows)
                    px.append(enc((1 - math.exp(-1.3 * v * inv)) * a + body[c] * (1 - a)))
                r.append(px)
            t.append(r)
        tiles.append(t)
    rows.append((w, h, tiles))
w, h = rows[0][0], rows[0][1]; S = 2; gap = 8
W = (2 * w + gap) * S; H = (len(rows) * (h + gap)) * S
img = bytearray()
for ri, (_, _, tiles) in enumerate(rows):
    for y in range(h + gap):
        for _ in range(S):
            img.append(0)
            for ti in range(2):
                for x in range(w + (gap if ti == 0 else 0)):
                    px = tiles[ti][y][x] if (y < h and x < w) else [30, 30, 30]
                    img += bytes(px) * S
def chunk(t, d): return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d))
open(out, 'wb').write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', W, H, 8, 2, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(bytes(img))) + chunk(b'IEND', b''))
