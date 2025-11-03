#!/usr/bin/env bash
# rafaelia_rollback.sh
# Rollback helper: verify manifest and flash backup via fastboot.
# Usage: .github/scripts/rafaelia_rollback.sh <manifest.json> [--force]
set -euo pipefail
 
MANIFEST=${1:-}
FORCE=${2:-}

if [ -z "$MANIFEST" ] || [ ! -f "$MANIFEST" ]; then
  echo "Usage: $0 <manifest.json> [--force]" >&2
  exit 2
fi

BACKUP_PATH=$(jq -r '.backupPath // .artifact_path // empty' "$MANIFEST")
HMAC=$(jq -r '.hmac_b64 // empty' "$MANIFEST")
SESSION=$(jq -r '.sessionId // .build_id // empty' "$MANIFEST")

if [ -z "$BACKUP_PATH" ] || [ ! -f "$BACKUP_PATH" ]; then
  echo "Backup file not found in manifest: $BACKUP_PATH" >&2
  exit 3
fi

echo "Preparing rollback for session: ${SESSION}"
echo "Backup path: ${BACKUP_PATH}"

if [ "$FORCE" != "--force" ]; then
  read -p "Confirm flashing backup to device via fastboot? Type YES to proceed: " CONF
  if [ "$CONF" != "YES" ]; then
    echo "Aborting rollback."
    exit 1
  fi
fi

# Optional: verify HMAC locally if verification key available (not implemented here)
# Flash using fastboot (ensure device in fastboot mode)
if ! command -v fastboot >/dev/null 2>&1; then
  echo "fastboot not found. Install Android platform-tools." >&2
  exit 4
fi

echo "Flashing backup via fastboot..."
fastboot flash boot "$BACKUP_PATH"
fastboot reboot
echo "Device rebooting. Monitor device for success."

exit 0
