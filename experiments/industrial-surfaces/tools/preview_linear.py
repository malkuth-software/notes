"""Tone an engine linear render with a lamp colour and power, as the dialog
does (1 − e^(−0.75 L), sRGB), enlarged 2×, to a PPM.
Usage: preview_linear.py <in.f32> <out.ppm> <r> <g> <b> <power>"""
import math
import struct
import sys
from array import array

src, dst = sys.argv[1], sys.argv[2]
colour = [float(v) for v in sys.argv[3:6]]
power = float(sys.argv[6])
with open(src, 'rb') as f:
    w, h = struct.unpack('<ii', f.read(8))
    lin = array('f')
    lin.frombytes(f.read())
values = [lin[i] for i in range(0, len(lin), 3)]
print(f'{w}x{h}: linear max {max(lin):.3f}, mean of lit pixels {sum(v for v in values if v > 0) / max(1, sum(1 for v in values if v > 0)):.3f}')


def enc(v):
    v = max(0.0, min(1.0, v))
    return round(255 * (v * 12.92 if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055))


out = bytearray()
for y in range(h * 2):
    for x in range(w * 2):
        i = ((y // 2) * w + x // 2) * 3
        out += bytes(enc(1 - math.exp(-0.75 * lin[i + c] * colour[c] * power)) for c in range(3))
open(dst, 'wb').write(b'P6\n%d %d\n255\n' % (w * 2, h * 2) + bytes(out))
