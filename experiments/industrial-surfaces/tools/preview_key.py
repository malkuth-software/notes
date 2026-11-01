"""Compose an engine key render as the dialog does: fill × fill power plus the
lamps × colour × power, unpremultiplied by coverage, toned 1 − e^(−0.75 L),
sRGB, over a mid grey; written 2× as a PPM.
Usage: preview_key.py <frame prefix> <out.ppm> <r> <g> <b> <lamp power> <fill power>"""
import math
import struct
import sys
from array import array


def read(path):
    with open(path, 'rb') as f:
        w, h = struct.unpack('<ii', f.read(8))
        data = array('f')
        data.frombytes(f.read())
    return w, h, data


prefix, dst = sys.argv[1], sys.argv[2]
colour = [float(v) for v in sys.argv[3:6]]
power, fill_power = float(sys.argv[6]), float(sys.argv[7])
w, h, lamp = read(prefix + '_light.f32')
_, _, cover = read(prefix + '_coverage.f32')
_, _, fill = read(prefix + '_fill.f32')


def enc(v):
    v = max(0.0, min(1.0, v))
    return round(255 * (v * 12.92 if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055))


grey = 0.12
out = bytearray()
for y in range(h * 2):
    for x in range(w * 2):
        p = (y // 2) * w + x // 2
        a = max(0.0, min(1.0, cover[p * 3]))
        px = []
        for c in range(3):
            v = (lamp[p * 3 + c] * colour[c] * power + fill[p * 3 + c] * fill_power) / a if a > 1e-6 else 0.0
            px.append(enc((1 - math.exp(-0.75 * v)) * a + grey * (1 - a)))
        out += bytes(px)
open(dst, 'wb').write(b'P6\n%d %d\n255\n' % (w * 2, h * 2) + bytes(out))
