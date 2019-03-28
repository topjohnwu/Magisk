# Developer Guides

## Magisk Modules
A Magisk module is a folder placed in `/data/adb/modules` with a structure below:

```
/data/adb/modules
├── .
├── .
|
├── $MODID                  <--- The folder is named with the ID of the module
│   │
│   │      *** Module Identity ***
│   │
│   ├── module.prop         <--- This files stores the metadata of the module
│   │
│   │      *** Status files ***
│   │
│   ├── skip_mount          <--- If exists, Magisk will NOT mount your files
│   ├── disable             <--- If exists, the module will be disabled
│   ├── remove              <--- If exists, the module will be removed next reboot
│   │
│   │      *** Scripts ***
│   │
│   ├── post-fs-data.sh     <--- This script will be executed in post-fs-data
│   ├── service.sh          <--- This script will be executed in late_start service
|   ├── uninstall.sh        <--- This script will be executed when Magisk removes your module
│   │
│   │      *** Resetprop Files ***
│   │
│   ├── system.prop         <--- This file will be loaded as system properties by resetprop
│   │
│   │      *** Module contents ***
│   │
│   ├── system              <--- This folder will be Magic Mounted if skip_mount does not exists
│   │   ├── .
│   │   ├── .
│   │   └── .
│   ├── vendor              <--- Auto generated. A symlink to $MODID/system/vendor
│   │
│   │      *** Others ***
│   │
│   ├── .                   <--- Any additional files / folders are allowed
│   └── .
|
├── another_module
│   ├── .
│   └── .
├── .
├── .
```
As long as a folder follows the structure above, it will be recognized as a module.

Here is the **strict** format of `module.prop`:

```
id=<string>
name=<string>
version=<string>
versionCode=<int>
author=<string>
description=<string>
```
- `id` has to match this regular expression: `^[a-zA-Z][a-zA-Z0-9._-]+$`<br>
ex: ✓ `a_module`, ✓ `a.module`, ✓ `module-101`, ✗ `a module`, ✗ `1_module`, ✗ `-a-module`<br>
This is the **unique identifier** of your module. You should not change it once published.
- `versionCode` has to be an **integer**. This is used to compare versions
- Others that isn't mentioned above can be any **single line** string.

## Magisk Module Installer
The official Magisk Module installer is hosted **[here](https://github.com/topjohnwu/magisk-module-installer)**.

That repo is a starting point for creating a Magisk module installer zip. Here are some details:

- You will found out that `META-INF/com/google/android/update-binary` is a dummy file. If you are creating a module locally for personal usage or testing, download the latest installer [`module_installer.sh`](https://github.com/topjohnwu/Magisk/blob/master/scripts/module_installer.sh) and replace `update-binary` with the script
- The next thing to do is to modify `module.prop` to provide information about your module
- Finally, modify `install.sh` to fit your need. The script is heavily documented so you should know what to do.
- (Optional) If you want to run custom uninstallation logic when your module is removed, add a new file `uninstall.sh` to the root of the installer
- **Windows users aware!!** The line endings of all text files should be in the **Unix format**. Please use advanced text editors like Sublime, Atom, Notepad++ etc. to edit **ALL** text files, **NEVER** use Windows Notepad.

## Submit Modules
You can submit a module to **Magisk-Module-Repo** so users can download your module directly in Magisk Manager. Before submitting, follow the instructions in the previous section to create a valid installer for your module. You can then create a request for submission via this link: [submission](https://github.com/Magisk-Modules-Repo/submission).

- When your module is downloaded with Magisk Manager, `META-INF/com/google/android/update-binary` will be **forcefully** replaced with the latest [`module_installer.sh`](https://github.com/topjohnwu/Magisk/blob/master/scripts/module_installer.sh) to make sure all installation uses the latest scripts.
- Since `update-binary` will be replaced, this means that all modules in the repo are expected to follow how the installation framework works: the installation framework will load your `install.sh` script and run the corresponding callbacks.
- This also means that you should NOT add custom logic in `update-binary` as they will simply be ignored.
- **Existing module devs please read!!** For devs migrating from the old template based modules to the new installer format, one thing you might overlook is the change in configuration flags: it no longer uses `AUTO_MOUNT`, but instead uses `SKIP_MOUNT`. In a nutshell, `AUTO_MOUNT=true` behaves exactly the same as `SKIP_MOUNT=false`, and 99% of the time you should NOT touch this flag.

## Notes on Partitions
On modern Android, `/system/vendor` is moved out from the system partition into its own separate `vendor` partition. For module developers, Magisk will handle these different configurations transparently so you do not need to worry anything about it. If you want to modify files in `vendor`, place the modified files under `/system/vendor` and you're good to go!

Starting in Android Q and some devices on older Android versions, a separate partition `platform` is available. Support for `platform` will come soon in upcoming Magisk versions, please stay tuned!

## Boot Scripts
In Magisk, you can run boot scripts in 2 different modes: **post-fs-data** and **late_start service** mode.

- post-fs-data mode
    - This stage is BLOCKING. Boot process is paused before execution is done, or 10 seconds has passed.
    - Scripts run before any modules are mounted. This allows a module developer to dynamically adjust their modules before it gets mounted.
    - This stage happens before Zygote is started, which pretty much means everything in Android
    - **Run scripts in this mode only if necessary!**
- late_start service mode
    - This stage is NON-BLOCKING. Your script runs in parallel along with the booting process.
    - **This is the recommended stage to run most scripts!**

In Magisk, there are also 2 kinds of scripts: **general scripts** and **module scripts**.

- General Scripts
    - Placed in `/data/adb/post-fs-data.d` or `/data/adb/service.d`
    - Only executed if the script is executable (execution permissions, `chmod +x script.sh`)
    - Scripts in `post-fs-data.d` runs in post-fs-data mode, and scripts in `service.d` runs in late_start service mode.
    - Will still be executed when **Core-Only** mode is enabled.
    - Modules should **NOT** add general scripts since it violates encapsulation
- Module Scripts
    - Placed in the folder of the module
    - Only executed if the module is enabled
    - `post-fs-data.sh` runs in post-fs-data mode, and `service.sh` runs in late_start service mode.
    - Will NOT be executed when **Core-Only** mode is enabled (all modules are disabled)
    - Modules require boot scripts should **ONLY** use module scripts instead of general scripts

Magisk's internal busybox's path `$BBPATH` is always prepended in `PATH`. This means all commands you call in scripts are always using busybox unless the applet is not included. This makes sure that your script always run in a predictable environment and always have the full suite of commands regardless of which Android version it is running on.

## Remove Files
How to remove a file systemless-ly? To actually make the file *disappear* is complicated (possible, not worth the effort). Replacing it with a dummy file should be good enough! Create an empty file with the same name and place it in the same path within a module, it shall replace your target file with a dummy file.

## Remove Folders
Same as mentioned above, actually making the folder *disappear* is not worth the effort. Replacing it with an empty folder should be good enough! A handy trick for module developers is to add the folder you want to remove into the `REPLACE` list within `install.sh`. If your module doesn't provide a corresponding folder, it will create an empty folder, and automatically add `.replace` into the empty folder so the dummy folder will properly replace the one in `/system`.
