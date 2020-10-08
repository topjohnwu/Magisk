## v8.0.2

- Fix an issue with requesting permission on devices older than Android 10
- Make more files download through CDN

## v8.0.1

- Fix `vbmeta.img` patching for Samsung `AP.tar` files. This fixes bootloops on devices like Galaxy S10 after flashing updated AP files.
- Properly truncate existing files before writing to prevent corrupted files
- Prevent a possible UI loop when device ran into very low memory
- Switch to use JSDelivr CDN for several files

## v8.0.0

- 100% full app rewrite! Will highlight functional changes below.
- Add detailed device info in home screen to assist user installation
- Support Magisk v21.0 communication protocol
- Support patching modern Samsung `AP.tar`
