#!/usr/bin/env python3
"""Patch and flash the boot slot reported by a booted Android headset.

The slot is intentionally determined only through ADB from the running Android
system. Fastboot slot metadata is never queried or used.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import shlex
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path


REMOTE_ROOT = "/data/local/tmp/quest-zygisk-patcher"


class PatchError(RuntimeError):
    pass


def quote(value: str) -> str:
    return shlex.quote(value)


@dataclass
class Adb:
    executable: str
    serial: str | None = None

    def _base(self) -> list[str]:
        cmd = [self.executable]
        if self.serial:
            cmd += ["-s", self.serial]
        return cmd

    def run(self, *args: str, capture: bool = True) -> str:
        proc = subprocess.run(
            self._base() + list(args),
            check=False,
            text=True,
            stdout=subprocess.PIPE if capture else None,
            stderr=subprocess.PIPE if capture else None,
        )
        if proc.returncode:
            detail = (proc.stderr or proc.stdout or "").strip()
            raise PatchError(f"adb {' '.join(args)} failed: {detail}")
        return (proc.stdout or "").strip()

    def shell(self, command: str) -> str:
        return self.run("shell", command)

    def root_shell(self, command: str) -> str:
        # Pass one quoted command to su so redirects and pipelines run as root.
        return self.shell(f"su -c {quote(command)}")


def normalized_slot(raw: str) -> str | None:
    value = raw.strip().lower().removeprefix("_")
    if value in {"a", "0"}:
        return "a"
    if value in {"b", "1"}:
        return "b"
    return None


def detect_runtime_slot(adb: Adb) -> tuple[str, dict[str, str]]:
    probes = {
        "ro.boot.slot_suffix": adb.shell("getprop ro.boot.slot_suffix"),
        "ro.boot.slot": adb.shell("getprop ro.boot.slot"),
    }
    parsed = {name: normalized_slot(value) for name, value in probes.items() if value.strip()}
    usable = {name: slot for name, slot in parsed.items() if slot is not None}
    if not usable:
        shown = ", ".join(f"{k}={v!r}" for k, v in probes.items())
        raise PatchError(f"running Android did not report an A/B slot ({shown})")

    distinct = set(usable.values())
    if len(distinct) != 1:
        shown = ", ".join(f"{k}={probes[k]!r}" for k in usable)
        raise PatchError(f"runtime slot sources disagree; refusing to flash ({shown})")
    return distinct.pop(), probes


def find_boot_partition(adb: Adb, slot: str) -> str:
    candidates = [
        f"/dev/block/by-name/boot_{slot}",
        f"/dev/block/bootdevice/by-name/boot_{slot}",
    ]
    script = "for p in " + " ".join(map(quote, candidates)) + "; do [ -b \"$p\" ] && { echo \"$p\"; exit 0; }; done; exit 1"
    try:
        result = adb.root_shell(script).splitlines()
    except PatchError as exc:
        raise PatchError(f"could not find boot_{slot} on the booted headset") from exc
    if not result:
        raise PatchError(f"could not find boot_{slot} on the booted headset")
    return result[-1].strip()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def remote_sha256(adb: Adb, path: str) -> str:
    output = adb.root_shell(f"sha256sum {quote(path)}")
    digest = output.split()[0].lower() if output else ""
    if not re.fullmatch(r"[0-9a-f]{64}", digest):
        raise PatchError(f"could not hash {path} on device: {output!r}")
    return digest


def preflight(adb: Adb) -> None:
    adb.run("wait-for-device")
    state = adb.run("get-state")
    if state != "device":
        raise PatchError(f"ADB device is not ready (state={state!r})")
    if adb.shell("getprop sys.boot_completed") != "1":
        raise PatchError("headset is not fully booted (sys.boot_completed is not 1)")
    if adb.root_shell("id -u").splitlines()[-1].strip() != "0":
        raise PatchError("su did not grant root")
    adb.root_shell("test -x /data/adb/magisk/magiskboot")


def patch_on_device(adb: Adb, partition: str, custom_magisk: Path) -> tuple[str, str, int]:
    adb.root_shell(
        f"rm -rf {quote(REMOTE_ROOT)} && mkdir -p {quote(REMOTE_ROOT)} && "
        f"chmod 0777 {quote(REMOTE_ROOT)}"
    )
    remote_binary = f"{REMOTE_ROOT}/magisk.custom"
    adb.run("push", str(custom_magisk), remote_binary)

    script = f"""
set -eu
cd {quote(REMOTE_ROOT)}
cp /data/adb/magisk/magiskboot ./magiskboot
chmod 0755 ./magiskboot
dd if={quote(partition)} of=original.img bs=4M
partition_size=$(blockdev --getsize64 {quote(partition)})
original_size=$(wc -c < original.img)
[ \"$original_size\" = \"$partition_size\" ]
rm -rf unpack verify
mkdir unpack verify
cd unpack
../magiskboot unpack ../original.img
test -f ramdisk.cpio
../magiskboot compress=xz ../magisk.custom magisk.xz
../magiskboot cpio ramdisk.cpio \"add 0644 overlay.d/sbin/magisk.xz magisk.xz\"
../magiskboot repack ../original.img ../patched.img
cd ../verify
../magiskboot unpack ../patched.img >/dev/null
../magiskboot cpio ramdisk.cpio \"extract overlay.d/sbin/magisk.xz extracted.xz\"
../magiskboot decompress extracted.xz extracted.magisk
cmp -s extracted.magisk ../magisk.custom
patched_size=$(wc -c < ../patched.img)
[ \"$patched_size\" = \"$partition_size\" ]
printf 'ORIGINAL_SHA256=%s\\n' \"$(sha256sum ../original.img | awk '{{print $1}}')\"
printf 'PATCHED_SHA256=%s\\n' \"$(sha256sum ../patched.img | awk '{{print $1}}')\"
printf 'PATCHED_SIZE=%s\\n' \"$patched_size\"
""".strip()
    output = adb.root_shell(script)
    values = dict(re.findall(r"^(ORIGINAL_SHA256|PATCHED_SHA256|PATCHED_SIZE)=(\S+)$", output, re.MULTILINE))
    if set(values) != {"ORIGINAL_SHA256", "PATCHED_SHA256", "PATCHED_SIZE"}:
        raise PatchError(f"patch completed without expected verification output:\n{output}")
    return values["ORIGINAL_SHA256"], values["PATCHED_SHA256"], int(values["PATCHED_SIZE"])


def pull_backup(adb: Adb, backup_dir: Path, slot: str, expected_hash: str) -> Path:
    backup_dir.mkdir(parents=True, exist_ok=True)
    stamp = time.strftime("%Y%m%d-%H%M%S")
    destination = backup_dir / f"boot_{slot}_stock_{stamp}.img"
    adb.run("pull", f"{REMOTE_ROOT}/original.img", str(destination))
    actual = sha256_file(destination)
    if actual != expected_hash:
        destination.unlink(missing_ok=True)
        raise PatchError("pulled backup hash does not match the live boot image")
    return destination


def flash_and_verify(adb: Adb, partition: str, patched_hash: str) -> None:
    output = adb.root_shell(
        f"dd if={quote(REMOTE_ROOT + '/patched.img')} of={quote(partition)} bs=4M conv=fsync && "
        f"sync && sha256sum {quote(partition)}"
    )
    flashed_hash = output.splitlines()[-1].split()[0].lower()
    if flashed_hash != patched_hash:
        raise PatchError(f"post-flash verification failed: expected {patched_hash}, got {flashed_hash}")


def parse_args() -> argparse.Namespace:
    here = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--adb", default="adb", help="adb executable (default: adb from PATH)")
    parser.add_argument("--serial", help="ADB device serial when more than one device is connected")
    parser.add_argument(
        "--magisk-binary",
        type=Path,
        default=here / "fixed_magisk_arm64",
        help="fixed arm64 Magisk binary to embed",
    )
    parser.add_argument("--backup-dir", type=Path, default=here / "backups")
    parser.add_argument("--flash", action="store_true", help="write the verified image to the runtime-reported slot")
    parser.add_argument("--reboot", action="store_true", help="reboot Android normally after a successful flash")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.reboot and not args.flash:
        raise PatchError("--reboot requires --flash")
    binary = args.magisk_binary.resolve()
    if not binary.is_file():
        raise PatchError(f"fixed Magisk binary not found: {binary}")

    adb = Adb(args.adb, args.serial)
    preflight(adb)
    slot, probes = detect_runtime_slot(adb)
    partition = find_boot_partition(adb, slot)
    print("Runtime slot sources:")
    for name, value in probes.items():
        print(f"  {name}: {value or '<empty>'}")
    print(f"Selected from booted Android: slot {slot} ({partition})")

    original_hash, patched_hash, patched_size = patch_on_device(adb, partition, binary)
    backup = pull_backup(adb, args.backup_dir.resolve(), slot, original_hash)
    print(f"Verified stock backup: {backup} ({original_hash})")
    print(f"Verified patched boot: {patched_size} bytes ({patched_hash})")

    if not args.flash:
        print("Patch-only run complete. Re-run with --flash (and optionally --reboot) to write it.")
        return 0

    flash_and_verify(adb, partition, patched_hash)
    print(f"Flash verified on runtime-selected boot_{slot}.")
    if args.reboot:
        print("Rebooting normally through Android...")
        adb.run("reboot", capture=False)
    else:
        print("Not rebooting; use 'adb reboot' when ready.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (PatchError, OSError, ValueError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
