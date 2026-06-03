#!/usr/bin/env bash
# Run on Windows after build, or unpack CI artifact as-is.
set -euo pipefail
VST3="${1:-build/MetronomeVST_artefacts/Release/VST3/Metronome.vst3}"
OUT="${2:-dist/Metronome-Windows-x64.zip}"
if [[ ! -d "$VST3" ]]; then
  echo "Not found: $VST3"
  exit 1
fi
mkdir -p "$(dirname "$OUT")"
rm -f "$OUT"
(cd "$(dirname "$VST3")" && zip -r "$(cd - >/dev/null && pwd)/$OUT" "$(basename "$VST3")")
echo "Created: $OUT"
