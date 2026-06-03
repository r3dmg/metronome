#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

if ! command -v x86_64-w64-mingw32-g++ &>/dev/null; then
  echo "Install MinGW: brew install mingw-w64"
  exit 1
fi

rm -rf build-win
cmake -B build-win \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake \
  -G "MinGW Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DMETRONOME_WINDOWS_CROSS=ON

cmake --build build-win -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

OUT="build-win/MetronomeVST_artefacts/Release/VST3/Metronome.vst3"
DIST="dist"
mkdir -p "$DIST"
rm -f "$DIST/Metronome-Windows-x64.zip"
(cd "$(dirname "$OUT")" && zip -r "$(pwd)/../../$DIST/Metronome-Windows-x64.zip" "$(basename "$OUT")")

echo ""
echo "Ready: $DIST/Metronome-Windows-x64.zip"
echo "Copy to Windows -> C:\\Program Files\\Common Files\\VST3\\"
