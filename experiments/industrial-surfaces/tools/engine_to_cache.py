"""Combine an engine key render into the lamp cache format of cpp/lamps.cpp.

Input: the lit pass (both lamps at unit power) and the coverage pass, each
int32 width, height then float32 RGB. Output: int32 width, height, lamps (1),
then base (zero: no room light), the lamp's plane, and coverage, float32.
The engine's pixels already average the key with the black beyond it, so the
lit plane is premultiplied by coverage as the cache expects.
"""
import struct
import sys
from array import array


def read(path):
    with open(path, 'rb') as f:
        w, h = struct.unpack('<ii', f.read(8))
        data = array('f')
        data.frombytes(f.read())
    return w, h, data


lit_path, cover_path, out_path = sys.argv[1:4]
w, h, lit = read(lit_path)
cw, ch, cover = read(cover_path)
assert (w, h) == (cw, ch)
n = w * h
alpha = array('f', (min(1.0, max(0.0, cover[i * 3])) for i in range(n)))
with open(out_path, 'wb') as f:
    f.write(struct.pack('<iii', w, h, 1))
    f.write(array('f', [0.0]).tobytes() * (n * 3))
    f.write(lit.tobytes())
    f.write(alpha.tobytes())
