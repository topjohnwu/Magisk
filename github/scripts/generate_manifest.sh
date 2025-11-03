#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <apk-path> <out-json>"
  exit 2
fi

APK="$1"
OUT="$2"

if [ ! -f "$APK" ]; then
  echo "ERROR: APK not found at $APK"
  exit 3
fi

mkdir -p "$(dirname "$OUT")"

# Compute sha256 and filesize
if command -v sha256sum >/dev/null 2>&1; then
  SHA256=$(sha256sum "$APK" | awk '{print $1}')
else
  SHA256=$(shasum -a 256 "$APK" | awk '{print $1}')
fi

if command -v stat >/dev/null 2>&1; then
  FILESIZE=$(stat -c%s "$APK" 2>/dev/null || stat -f%z "$APK" 2>/dev/null || echo 0)
else
  FILESIZE=0
fi

PACKAGE="unknown"
VERSION_NAME="unknown"
if command -v aapt >/dev/null 2>&1; then
  # Parse package and version name from aapt
  PKG_INFO=$(aapt dump badging "$APK" | awk -F"'" '/package: name=/{print $2","$6}') || true
  PACKAGE=$(echo "$PKG_INFO" | cut -d',' -f1 || true)
  VERSION_NAME=$(echo "$PKG_INFO" | cut -d',' -f2 || true)
fi

cat > "$OUT" <<EOF
{
  "apk_path": "${APK}",
  "package": "${PACKAGE}",
  "version_name": "${VERSION_NAME}",
  "sha256": "${SHA256}",
  "filesize": ${FILESIZE},
  "generated_by": "generate_manifest.sh",
  "timestamp": "$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
}
EOF

echo "Manifest generated at $OUT"
