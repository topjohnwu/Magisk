#!/usr/bin/env bash
# scripts/date_utils.sh
# Portable date helpers for Linux (GNU date) and macOS/BSD (date).
# Uses gdate if available, falls back to date and Python3 for parsing/relative ops.
# Usage: source scripts/date_utils.sh

set -euo pipefail 

# Determine which date command to use:
if command -v gdate >/dev/null 2>&1; then
  DATE_CMD=gdate
elif command -v date >/dev/null 2>&1; then 
  DATE_CMD=date
else
  echo "No date command found" >&2
  exit 1
fi

# portable_date_utc FORMAT...
portable_date_utc(){
  "$DATE_CMD" -u "$@"
}

# to_epoch DATE_STRING INPUT_FORMAT
to_epoch(){
  local datestr="$1"
  local infmt="${2:-}"
  if "$DATE_CMD" --version >/dev/null 2>&1 && "$DATE_CMD" -d "$datestr" "+%s" >/dev/null 2>&1; then
    "$DATE_CMD" -d "$datestr" "+%s"
    return
  fi

  if [ -n "$infmt" ]; then
    if "$DATE_CMD" -j -f "$infmt" "$datestr" "+%s" >/dev/null 2>&1; then
      "$DATE_CMD" -j -f "$infmt" "$datestr" "+%s"
      return
    fi
  fi

  python3 - <<PY
import sys, datetime
s = """$datestr"""
fmt = """$infmt"""
try:
    if fmt:
        dt = datetime.datetime.strptime(s, fmt)
    else:
        from datetime import datetime as _dt
        dt = _dt.fromisoformat(s)
except Exception:
    try:
        from dateutil import parser as p
        dt = p.parse(s)
    except Exception as e:
        print("ERROR: unable to parse date string", s, file=sys.stderr)
        sys.exit(2)
if dt.tzinfo:
    ts = int(dt.astimezone(datetime.timezone.utc).timestamp())
else:
    ts = int(dt.timestamp())
print(ts)
PY
}

# from_epoch EPOCH OUTPUT_FORMAT
from_epoch(){
  local epoch="$1"
  local outfmt="${2:-+%Y-%m-%d %H:%M:%S %Z}"
  if "$DATE_CMD" --version >/dev/null 2>&1 && "$DATE_CMD" -d "@$epoch" "$outfmt" >/dev/null 2>&1; then
    "$DATE_CMD" -d "@$epoch" "$outfmt"
    return
  fi
  if "$DATE_CMD" -r "$epoch" "$outfmt" >/dev/null 2>&1; then
    "$DATE_CMD" -r "$epoch" "$outfmt"
    return
  fi
  python3 - <<PY
import sys, datetime
e = int("$epoch")
dt = datetime.datetime.utcfromtimestamp(e)
print(dt.strftime("""${outfmt}"""))
PY
}

# add_seconds_to_date INPUT_DATE_STRING SECONDS INPUT_FORMAT OUTPUT_FORMAT
add_seconds_to_date(){
  local datestr="$1"
  local seconds="$2"
  local infmt="$3"
  local outfmt="$4"
  python3 - <<PY
import sys, datetime
s = """$datestr"""
secs = int("""$seconds""")
fmt = """$infmt"""
out = """$outfmt"""
try:
    dt = datetime.datetime.strptime(s, fmt)
except Exception:
    try:
        from dateutil import parser as p
        dt = p.parse(s)
    except Exception as e:
        print("ERROR: unable to parse", s, file=sys.stderr)
        sys.exit(2)
dt2 = dt + datetime.timedelta(seconds=secs)
print(dt2.strftime(out))
PY
}

# add_relative INPUT_DATE_STRING RELATIVE_EXPR INPUT_FORMAT OUTPUT_FORMAT
add_relative(){
  local datestr="$1"
  local expr="$2"
  local infmt="$3"
  local outfmt="$4"
  python3 - <<PY
import sys,datetime,re
s = """$datestr"""
expr = """$expr"""
fmt = """$infmt"""
out = """$outfmt"""
try:
    if fmt:
        dt = datetime.datetime.strptime(s, fmt)
    else:
        dt = datetime.datetime.fromisoformat(s)
except Exception:
    try:
        from dateutil import parser as p
        dt = p.parse(s)
    except Exception as e:
        print("ERROR: cannot parse input date", s, file=sys.stderr)
        sys.exit(2)
m = re.match(r'([+-])\s*(\d+)\s*(seconds|minutes|hours|days|weeks)?', expr.strip(), re.I)
if not m:
    print("ERROR: unsupported relative expr: "+expr, file=sys.stderr); sys.exit(2)
sign, val, unit = m.group(1), int(m.group(2)), (m.group(3) or "seconds")
kwargs = {}
unit = unit.lower()
if unit.startswith("second"): kwargs['seconds'] = val
elif unit.startswith("minute"): kwargs['minutes'] = val
elif unit.startswith("hour"): kwargs['hours'] = val
elif unit.startswith("day"): kwargs['days'] = val
elif unit.startswith("week"): kwargs['weeks'] = val
if sign == "-":
    for k in kwargs: kwargs[k] = -kwargs[k]
dt2 = dt + datetime.timedelta(**kwargs)
print(dt2.strftime(out))
PY
}

export DATE_CMD
