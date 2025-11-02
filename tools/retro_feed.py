```python
#!/usr/bin/env python3
# retro_feed.py
# Minimal analyzer that inspects a snapshot zip produced by bug_snapshot.sh
# and returns a simple report of key indicators.

import sys
import json
import zipfile
import os
from pathlib import Path

def analyze_snapshot(zip_path):
    if not Path(zip_path).exists():
        print("Snapshot not found:", zip_path)
        return 2
    with zipfile.ZipFile(zip_path, 'r') as z:
        namelist = z.namelist()
        print("Snapshot contains", len(namelist), "entries")
        # try to read manifest
        for candidate in ["manifest.json", "RAFAELIA_MANIFEST.json"]:
            if candidate in namelist:
                with z.open(candidate) as f:
                    m = json.load(f)
                    print("Manifest:", json.dumps(m, indent=2))
        # extract logcat head
        if "emulator-logcat-final.txt" in namelist:
            with z.open("emulator-logcat-final.txt") as f:
                lines = f.read().decode(errors='ignore').splitlines()
                print("---- logcat head (first 30 lines) ----")
                for L in lines[:30]:
                    print(L)
        # show tombstone list
        tombs = [n for n in namelist if 'tombstones' in n]
        if tombs:
            print("Tombstones in snapshot:")
            for t in tombs:
                print(" -", t)
    return 0

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: retro_feed.py <snapshot.zip>")
        sys.exit(1)
    sys.exit(analyze_snapshot(sys.argv[1]))
