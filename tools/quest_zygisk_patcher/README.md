# Quest fixed-Zygisk boot patcher

This project embeds the fixed arm64 Magisk binary into the boot image belonging
to the slot reported by the **currently booted Android system**. It never asks
fastboot for a slot and never changes the active slot.

The fixed Magisk source is published in the
[`meta-quest-zygisk-fix`](https://github.com/xAstroBoy/Magisk/tree/meta-quest-zygisk-fix)
branch. Commit `7727ce4` contains the Quest Zygisk changes: keep the
native-bridge property available for Meta's lazily spawned partition zygotes,
install the JNI hooks at the reliable trigger, make hooking idempotent, and
contain JNI local references. The repository bundles the tested arm64 binary
as `fixed_magisk_arm64`.

## Requirements

- Windows with Python 3.9+ and `adb` in `PATH`
- A fully booted, A/B Android headset visible over ADB
- Existing root access (`su`) and `/data/adb/magisk/magiskboot`
- An unlocked bootloader

## Run

First make and pull a verified backup without flashing:

```powershell
python .\quest_zygisk_patch.py
```

Patch, flash the system-reported current slot, verify the written partition,
then reboot normally:

```powershell
python .\quest_zygisk_patch.py --flash --reboot
```

Use `--serial SERIAL` if multiple ADB devices are connected. Use
`--magisk-binary PATH` to choose another fixed arm64 Magisk build.

## Safety behavior

- Requires `sys.boot_completed=1`; it will not operate from fastboot.
- Reads `ro.boot.slot_suffix` and falls back to `ro.boot.slot` via `getprop`
  after Android reports that boot has completed.
- Aborts if available runtime sources disagree about `a` versus `b`.
- Resolves only `boot_a` or `boot_b`, based on that runtime result.
- Pulls and hashes the stock boot image before any optional flash.
- Verifies the embedded binary after repacking and hashes the flashed partition.
- Does not call `fastboot`, set an active slot, or flash the opposite slot.

Backups are stored under `backups\` and should be copied somewhere safe.

## License and source

The bundled Magisk binary is built from the linked GPLv3 source branch. Magisk
and this redistribution are provided under GPLv3; see `LICENSE`.
