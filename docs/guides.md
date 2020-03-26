# Developer Guides

## Magisk Modules
A Magisk module is a folder placed in `/data/adb/modules` with the structure below:

```
/data/adb/modules
├── .
├── .
|
├── $MODID                  <--- The folder is named with the ID of the module
│   │
│   │      *** Module Identity ***
│   │
│   ├── module.prop         <--- This file stores the metadata of the module
│   │
│   │      *** Main Contents ***
│   │
│   ├── system              <--- This folder will be mounted if skip_mount does not exists
│   │   ├── ...
│   │   ├── ...
│   │   └── ...
│   │
│   │      *** Status Flags ***
│   │
│   ├── skip_mount          <--- If exists, Magisk will NOT mount your system folder
│   ├── disable             <--- If exists, the module will be disabled
│   ├── remove              <--- If exists, the module will be removed next reboot
│   │
│   │      *** Optional Files ***
│   │
│   ├── post-fs-data.sh     <--- This script will be executed in post-fs-data
│   ├── service.sh          <--- This script will be executed in late_start service
|   ├── uninstall.sh        <--- This script will be executed when Magisk removes your module
│   ├── system.prop         <--- Properties in this file will be loaded as system properties by resetprop
│   ├── sepolicy.rule       <--- Additional custom sepolicy rules to be patched
│   │
│   │      *** Auto Generated, DO NOT MANUALLY CREATE OR MODIFY ***
│   │
│   ├── vendor              <--- A symlink to $MODID/system/vendor
│   ├── product             <--- A symlink to $MODID/system/product
│   │
│   │      *** Any additional files / folders are allowed ***
│   │
│   ├── ...
│   └── ...
|
├── another_module
│   ├── .
│   └── .
├── .
├── .
```


#### module.prop

This is the **strict** format of `module.prop`

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
- Others that weren't mentioned above can be any **single line** string.
- Make sure to use the `UNIX (LF)` line break type and not the `Windows (CR+LF)` or `Macintosh (CR)` one.

#### Shell scripts (`*.sh`)
Please read the [Boot Scripts](#boot-scripts) section to understand the difference between `post-fs-data.sh` and `service.sh`. For most module developers, `service.sh` should be good enough if you just need to run a boot script.

In all scripts of your module, please use `MODDIR=${0%/*}` to get your module's base directory path; do **NOT** hardcode your module path in scripts.

#### system.prop
This file follows the same format as `build.prop`. Each line comprises of `[key]=[value]`.

#### sepolicy.rule
If your module requires some additional sepolicy patches, please add those rules into this file. The module installer script and Magisk's daemon will make sure this file is copied to somewhere `magiskinit` can read pre-init to ensure these rules are injected properly.

Each line in this file will be treated as a policy statement. For more details how a policy statement is formated, please check [magiskpolicy](tools.md#magiskpolicy)'s documentation.

#### The `system` folder
All files you want Magisk to replace/inject for you should be placed in this folder. Please read through the [Magic Mount](details.md#magic-mount) section to understand how Magisk mount your files.

## Magisk Module Installer

A Magisk Module Installer is a Magisk Module packaged in a zip file that can be flashed in Magisk Manager or custom recoveries such as TWRP. An installer have the same file structure as a Magisk module (please check the previous section for more info). The simplest Magisk Module Installer is just a Magisk Module packed in a zip file, with addition to the following files:

- `update-binary`: Download the latest [module_installer.sh](https://github.com/topjohnwu/Magisk/blob/master/scripts/module_installer.sh) and rename/copy that script as `update-binary`
- `updater-script`: This file should only contain the string `#MAGISK`

By default, `update-binary` will check/setup the environment, load utility functions, extract the module installer zip to where your module will be installed, and finally do some trivial tasks and cleanups, which should cover most simple modules' needs.

```
module.zip
│
├── META-INF
│   └── com
│       └── google
│           └── android
│               ├── update-binary      <--- The module_installer.sh you downloaded
│               └── updater-script     <--- Should only contain the string "#MAGISK"
│
├── customize.sh                       <--- (Optional, more details later)
│                                           This script will be sourced by update-binary
├── ...
├── ...  /* The rest of module's files */
|
```

#### Customization

If you need to customize the module installation process, optionally you can create a new script in the installer called `customize.sh`. This script will be *sourced* (not executed!) by `update-binary` after all files are extracted and default permissions and secontext are applied. This is very useful if your module includes different files based on ABI, or you need to set special permissions/secontext for some of your files (e.g. files in `/system/bin`).

If you need even more customization and prefer to do everything on your own, declare `SKIPUNZIP=1` in `customize.sh` to skip the extraction and applying default permissions/secontext steps. Be aware that by doing so, your `customize.sh` will then be responsible to install everything by itself.

#### `customize.sh` Environment

Magisk's internal busybox's path `$BBPATH` is added in the front of `PATH`. The following variables and shell functions are available for convenience:

##### Variables
- `MAGISK_VER` (string): the version string of current installed Magisk (e.g. `v20.0`)
- `MAGISK_VER_CODE` (int): the version code of current installed Magisk (e.g. `20000`)
- `BOOTMODE` (bool): `true` if the module is being installed in Magisk Manager
- `MODPATH` (path): the path where your module files should be installed
- `TMPDIR` (path): a place where you can temporarily store files
- `ZIPFILE` (path): your module's installation zip
- `ARCH` (string): the CPU architecture of the device. Value is either `arm`, `arm64`, `x86`, or `x64`
- `IS64BIT` (bool): `true` if `$ARCH` is either `arm64` or `x64`
- `API` (int): the API level (Android version) of the device (e.g. `21` for Android 5.0)

##### Functions

```
ui_print <msg>
    print <msg> to console
    Avoid using 'echo' as it will not display in custom recovery's console

abort <msg>
    print error message <msg> to console and terminate installation
    Avoid using 'exit' as it will skip the termination cleanup steps

set_perm <target> <owner> <group> <permission> [context]
    if [context] is not set, the default is "u:object_r:system_file:s0"
    this function is a shorthand for the following commands:
       chown owner.group target
       chmod permission target
       chcon context target

set_perm_recursive <directory> <owner> <group> <dirpermission> <filepermission> [context]
    if [context] is not set, the default is "u:object_r:system_file:s0"
    for all files in <directory>, it will call:
       set_perm file owner group filepermission context
    for all directories in <directory> (including itself), it will call:
       set_perm dir owner group dirpermission context
```

##### Easy Replace
You can declare a list of folders you want to directly replace in the variable name `REPLACE`. The module installer script will pickup this variable and create `.replace` files for you. An example declaration:

```
REPLACE="
/system/app/YouTube
/system/app/Bloatware
"
```
The list above will result in the following files being created: `$MODPATH/system/app/YouTube/.replace` and `$MODPATH/system/app/Bloatware/.replace`

#### Notes

- When your module is downloaded with Magisk Manager, `update-binary` will be **forcefully** replaced with the latest [`module_installer.sh`](https://github.com/topjohnwu/Magisk/blob/master/scripts/module_installer.sh) to ensure all installer uses up-to-date scripts. **DO NOT** try to add any custom logic in `update-binary` as it is pointless.
- Due to historical reasons, **DO NOT** add a file named `install.sh` in your module installer. That specific file was previously used and will be treated differently.
- **DO NOT** call `exit` at the end of `customize.sh`. The module installer would want to do finalizations.

## Submit Modules

You can submit a module to **Magisk-Module-Repo** so users can download your module directly in Magisk Manager.

- Follow the instructions in the previous section to create a valid installer for your module.
- Create `README.md` (filename should be exactly the same) containing all info for your module. If you are not familiar with the Markdown syntax, the [Markdown Cheat Sheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet) will be handy.
- Create a repository with your personal GitHub account, and upload your module installer to the repo
- Create a request for submission via this link: [submission](https://github.com/Magisk-Modules-Repo/submission)

## Module Tricks

#### Remove Files
How to remove a file systemless-ly? To actually make the file *disappear* is complicated (possible, not worth the effort). Replacing it with a dummy file should be good enough! Create an empty file with the same name and place it in the same path within a module, it shall replace your target file with a dummy file.

#### Remove Folders
Same as mentioned above, actually making the folder *disappear* is not worth the effort. Replacing it with an empty folder should be good enough! A handy trick for module developers is to add the folder you want to remove into the `REPLACE` list within `customize.sh`. If your module doesn't provide a corresponding folder, it will create an empty folder, and automatically add `.replace` into the empty folder so the dummy folder will properly replace the one in `/system`.


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

Magisk's internal busybox's path `$BBPATH` is added in the front of `PATH`. This means all commands you call in scripts are always using busybox unless the applet is not included. This makes sure that your script always run in a predictable environment and always have the full suite of commands regardless of which Android version it is running on.

## Root Directory Overlay System

Since `/` is read-only in system-as-root devices, Magisk provides an overlay system, allowing developers to patch files / add new rc scripts. Additional files shall be placed in the `overlay.d` folder in the ramdisk, and they will have the following restrictions:

- All `*.rc` files in `overlay.d` will be read and concatenated *AFTER* `init.rc`
- Replacing existing files are allowed.<br>
e.g. you can replace `/res/random.png` by adding the file `overlay.d/res/random.png`
- Non-existing files will be ignored (with exceptions detailed in the next point).<br>
e.g. `overlay.d/new_file` will be ignored if `/new_file` does not exist
- Additional files in `overlay.d/sbin` is allowed as they will be copied into Magisk's sbin overlay.<br>
e.g. `overlay.d/sbin/libfoo.ko` will be copied to `/sbin/libfoo.ko`.
