"""Pack engine press frames as a delta animation.

Each frame has the lamps' linear light (RGB at unit power), the coverage, and
the faint fill light (RGB), the light planes premultiplied by coverage since
the engine averages the key with the black beyond it. The
file holds frame 0 whole and, for every later frame, only the runs of pixels
that differ from the previous frame.

Layout (little-endian): int32 width, height, frames, channels (7: lamp R G B,
coverage, fill R G B); frame 0 as width*height*4 float32; then per frame: int32 run
count, and per run int32 first pixel, int32 length, length*4 float32.
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


directory, frames, out_path = sys.argv[1], int(sys.argv[2]), sys.argv[3]
CH = 7
planes = []
for i in range(frames):
    w, h, light = read(f'{directory}/f{i}_light.f32')
    _, _, cover = read(f'{directory}/f{i}_coverage.f32')
    _, _, fill = read(f'{directory}/f{i}_fill.f32')
    n = w * h
    plane = array('f', bytes(n * 4 * CH))
    for p in range(n):
        plane[p * CH] = light[p * 3]
        plane[p * CH + 1] = light[p * 3 + 1]
        plane[p * CH + 2] = light[p * 3 + 2]
        plane[p * CH + 3] = min(1.0, max(0.0, cover[p * 3]))
        plane[p * CH + 4] = fill[p * 3]
        plane[p * CH + 5] = fill[p * 3 + 1]
        plane[p * CH + 6] = fill[p * 3 + 2]
    planes.append(plane)

# a pixel has changed when any channel moved by more than this (linear units)
threshold = 1e-4
body = bytearray()
body += struct.pack('<iiii', w, h, frames, CH)
body += planes[0].tobytes()
changed_total = 0
for k in range(1, frames):
    previous, current = planes[k - 1], planes[k]
    runs = []
    p = 0
    while p < n:
        if any(abs(current[p * CH + c] - previous[p * CH + c]) > threshold for c in range(CH)):
            start = p
            while p < n and any(abs(current[p * CH + c] - previous[p * CH + c]) > threshold for c in range(CH)):
                p += 1
            runs.append((start, p - start))
        else:
            p += 1
    body += struct.pack('<i', len(runs))
    for start, length in runs:
        body += struct.pack('<ii', start, length)
        body += current[start * CH:(start + length) * CH].tobytes()
        changed_total += length
with open(out_path, 'wb') as f:
    f.write(body)
full = frames * n * 4 * CH
print(f'{frames} frames, {w}x{h}: delta file {len(body)/1024:.0f} KiB vs {full/1024:.0f} KiB as full frames '
      f'({changed_total/(n*(frames-1))*100:.0f}% of pixels change per frame on average)')
