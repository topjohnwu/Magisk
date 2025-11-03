#!/usr/bin/env bash
# bug_snapshot.sh
# Collect emulator/device artifacts and create a ZIP snapshot for CI triage.
# Usage: .github/scripts/bug_snapshot.sh <output-zip> [optional manifest path]
set -euo pipefail
OUT_ZIP=${1:-bug_snapshot.zip}
MANIFEST=${2:-RAFAELIA_MANIFEST.json}
ARTDIR=${GITHUB_WORKSPACE:-$(pwd)}/.rafaelia_artifacts
mkdir -p "$ARTDIR"

echo "Collecting logcat..."
adb wait-for-device
adb logcat -d > "$ARTDIR"/emulator-logcat.txt 2>&1 || true

echo "Collecting dumpsys (activities)..."
adb shell dumpsys activity activities > "$ARTDIR"/dumpsys-activities.txt 2>&1 || true

echo "Collecting getprop..."
adb shell getprop > "$ARTDIR"/emulator-getprop.txt 2>&1 || true

echo "Collecting tombstones (native crashes)..."
if adb shell ls /data/tombstones 2>/dev/null | grep -q .; then
  mkdir -p "$ARTDIR"/tombstones
  adb pull /data/tombstones "$ARTDIR"/tombstones || true
fi

echo "Collecting ANR traces..."
if adb shell ls /data/anr 2>/dev/null | grep -q .; then
  mkdir -p "$ARTDIR"/anr
  adb pull /data/anr "$ARTDIR"/anr || true
fi

echo "Pulling instrumentation output (if any)..."
if [ -f "$GITHUB_WORKSPACE"/artifacts/instrumentation-output.txt ]; then
  cp "$GITHUB_WORKSPACE"/artifacts/instrumentation-output.txt "$ARTDIR"/ || true
fi

if [ -f "$MANIFEST" ]; then
  cp "$MANIFEST" "$ARTDIR"/manifest.json || true
fi

echo "Packing snapshot to $OUT_ZIP"
( cd "$ARTDIR" && zip -r -q "$GITHUB_WORKSPACE/$OUT_ZIP" . )
echo "Snapshot created at $GITHUB_WORKSPACE/$OUT_ZIP"
exit 0
