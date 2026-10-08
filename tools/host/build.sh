#!/bin/sh
# Host-side test and screen preview for uSonicTimer (no hardware needed).
#   U8G2_CSRC=/path/to/u8g2/csrc tools/host/build.sh [output dir]
# U8G2_CSRC defaults to the U8g2 copy PlatformIO downloaded for the release build (macOS cache
# path from platformio.ini). Needs a C/C++ compiler; the PNGs need python3 with Pillow.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
U8G2_CSRC=${U8G2_CSRC:-$HOME/Library/Caches/PlatformIO/uSonicTimer/libdeps/release/U8g2/src/clib}
OUT=${1:-$HERE/out}
mkdir -p "$OUT/obj"

for c in "$U8G2_CSRC"/*.c; do
    o="$OUT/obj/$(basename "$c" .c).o"
    [ "$o" -nt "$c" ] || cc -O1 -w -I"$U8G2_CSRC" -c "$c" -o "$o"
done
CXXFLAGS="-std=c++11 -Wall -Wextra -O1 -I$HERE/shim -I$U8G2_CSRC -I$ROOT/include"
c++ $CXXFLAGS -o "$OUT/test_logic" "$HERE/test_logic.cpp" "$ROOT/src/screens.cpp" "$HERE/shim/U8g2lib_host.cpp" "$OUT"/obj/*.o
c++ $CXXFLAGS -o "$OUT/preview" "$HERE/preview.cpp" "$ROOT/src/screens.cpp" "$HERE/shim/U8g2lib_host.cpp" "$OUT"/obj/*.o
"$OUT/test_logic"
"$OUT/preview" "$OUT" > /dev/null
python3 "$HERE/render.py" "$OUT" || echo "(no Pillow: the .pbm frames are in $OUT)"
