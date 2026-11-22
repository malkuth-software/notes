#!/bin/sh
# Builds the experiment's tools. Run on the M4 (see m4.sh).
#   sh build.sh            optimised tools in build/
#   sh build.sh sanitize   ASan + UBSan tools in build/sanitize/
set -e
CXX=${CXX:-clang++}
OUT=build
FLAGS="-std=c++20 -O2 -g -Wall -Wextra -Isrc $EXTRA"
if [ "$1" = sanitize ]; then
    OUT=build/sanitize
    FLAGS="-std=c++20 -O1 -g -Wall -Wextra -Isrc -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer $EXTRA"
fi
mkdir -p "$OUT"
CODEC="src/fp_codec_enc.cpp src/fp_codec_dec.cpp"
$CXX $FLAGS src/fontpack.cpp src/fp_pack.cpp src/fp_load.cpp $CODEC -o "$OUT/fontpack"
$CXX $FLAGS src/lab_codec.cpp $CODEC -o "$OUT/lab_codec"
$CXX $FLAGS src/lab_fuzzy.cpp -o "$OUT/lab_fuzzy"
# Optional FreeType smoke test, when FreeType is installed.
if pkg-config --exists freetype2 2>/dev/null; then
    $CXX $FLAGS $(pkg-config --cflags freetype2) src/ft_check.cpp src/fp_load.cpp src/fp_codec_dec.cpp \
        $(pkg-config --libs freetype2) -o "$OUT/ft_check"
fi
