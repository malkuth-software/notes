b=../.build/stx-skia-full
for f in region-clips canvas-device android-clip layers render mask-filter; do
    total=$(wc -c < "$b/candidate/$f.raw")
    diff=$(cmp -l "$b/baseline/$f.raw" "$b/candidate/$f.raw" 2>/dev/null | wc -l)
    first=$(cmp "$b/baseline/$f.raw" "$b/candidate/$f.raw" 2>/dev/null | head -1)
    echo "$f: $total bytes, $diff differ; $first"
done
