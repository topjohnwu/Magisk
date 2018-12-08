# Developer Guides
## Scripts
In Magisk, you can run boot scripts in 2 different modes: **post-fs-data** and **late_start service** mode.

- post-fs-data mode
    - This stage is BLOCKING. Boot process is paused before execution is done, or 10 seconds has passed.
    - Upside: Executed before Zygote is started (which pretty much means everything)
    - Downside: Execution has time limit; blocks boot process
    - Environment variable `PATH` is set to `PATH=$BBPATH:/sbin:$SYSTEMMIR/bin:$SYSTEMMIR/xbin:$VENDORMIR/bin`
    - **Run scripts in this mode only if necessary!**
- late_start service mode
    - This stage is NON-BLOCKING. Your script runs in parallel along with the booting process.
    - Upside: No time limit, will not block boot process
    - Downside: Scripts are not executed as early as post-fs-data mode
    - Environment variable `PATH` is set to `PATH=$BBPATH/sbin:$PATH`
    - **This is the recommended stage to run most scripts!**

In Magisk, there are also 2 kinds of scripts: **general scripts** and **module scripts**.

- General Scripts
    - Placed in `$COREDIR/post-fs-data.d` or `$COREDIR/service.d`
    - Only executed if the script is executable (execution permissions, `chmod +x script.sh`)
    - Scripts in `post-fs-data.d` runs in post-fs-data mode, and scripts in `service.d` runs in late_start service mode.
    - Will still be executed when **Core-Only** mode is enabled.
- Module Scripts
    - Placed in the folder of the module
    - Only executed if the module is enabled
    - `post-fs-data.sh` runs in post-fs-data mode, and `service.sh` runs in late_start service mode.
    - Will NOT be executed when **Core-Only** mode is enabled (all modules are disabled)

### Notes
- Be aware that the 10 seconds time limit in post-fs-data mode is shared by **ALL** post-fs-data operations, including all scripts and **Magic Mount**! If a post-fs-data script is stuck, it will affect magic mount and cause it to not work properly!
- Magisk's internal busybox's path `$BBPATH` is always prepended in `PATH`. This means all commands you call in scripts are always using the Magisk busybox unless the applet is not included, which in that case will fallback to use system included binaries (the most common one should be `chcon` since internal busybox does not support SELinux). This makes sure that your script always run in a predictable environment and always have the full suite of commands to use regardless of which Android version it is running on.


## Magisk Modules
A Magisk module is a folder placed in `magisk.img` with a structure defined below:

```
$MOUNTPOINT
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
│   ├── auto_mount          <--- If this file exists, auto mount is enabled
│   ├── disable             <--- If this file exists, the module is disabled
│   ├── remove              <--- If this file exists, the module will be removed next reboot
│   │
│   │      *** Boot Scripts ***
│   │
│   ├── post-fs-data.sh     <--- This script will be executed in post-fs-data
│   ├── service.sh          <--- This script will be executed in late_start service
│   │
│   │      *** Resetprop Files ***
│   │
│   ├── system.prop         <--- This file will be loaded as system properties by resetprop
│   │
│   │      *** Module contents ***
│   │
│   ├── system              <--- If auto mount is enabled, this folder will be Magic Mounted
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
As long as a folder follows the structure above, it will be recognized as a module. The only thing *required* is `module.prop`, other files are based on what your module aims to accomplish.

Here is the **strict** format of `module.prop`:

```
id=<string>
name=<string>
version=<string>
versionCode=<int>
author=<string>
description=<string>
minMagisk=<int>
```
- `id` has to match this regular expression: `^[a-zA-Z][a-zA-Z0-9\._-]+$`.  
ex: ✓ `a_module`, ✓ `a.module`, ✓ `module-101`, ✗ `a module`, ✗ `1_module`, ✗ `-a-module`  
This is the **sole identifier** of your module. You should not change it once published.
- `versionCode` has to be an **integer**. This is used to compare versions
- `minMagisk`: an **integer** set as the target Magisk versionCode (this is also the module's minimum requirement).  
If you are creating a new module, you should set this value to `17000`.
- Others that isn't mentioned above can be any **single line** string.

## Magisk Module Template
The **Magisk Module Template** is hosted **[here](https://github.com/topjohnwu/magisk-module-template)**.

It is a template to create a flashable zip to install Magisk Modules. It is designed to be simple to use so that anyone can create their own modules easily. The template itself contains minimal scripting for installation; most of the functions are located externally in [util_functions.sh](https://github.com/topjohnwu/Magisk/blob/master/scripts/util_functions.sh), which will be installed along with Magisk, and can be upgraded through a Magisk upgrade without the need of a template update.

Here are some files you would want to know:

- `config.sh`: A simple script used as a configuration file. It is the place to configure which features your module requires. A detailed instructions on how to use the template is directly written in the top of this file.
- `module.prop`: This file is your module's metadata.
- `common/*`: Boot stage scripts and `system.prop`
- `META-INF/com/google/android/update-binary`: The actual installation script. Modify this file for advanced custom behavior

And here are some notes to be aware of:

- The template depends on external Magisk scripts, please correctly specify the `minMagisk` value in `module.prop` so the template can fail fast when an unsupported Magisk is installed.
- **!! Windows users aware !!** The line endings of all text files should be in the **Unix format**. Please use advanced text editors like Sublime, Atom, Notepad++ etc. to edit **ALL** text files, **NEVER** use Windows Notepad.

## Submit Modules to Magisk-Modules-Repo
Directly go to the [submission link](https://github.com/Magisk-Modules-Repo/submission).

## Remove Files
How to remove a file systemless-ly? To actually make the file *disappear* is complicated (possible, not worth the effort). Replacing it with a dummy file should be good enough! Create an empty file with the same name and place it in the same path within a module, it shall replace your target file with a dummy file.

## Remove Folders
Same as mentioned above, actually making the folder *disappear* is not worth the effort. Replacing it with an empty folder should be good enough! A handy trick for module developers using [Magisk Module Template](https://github.com/topjohnwu/magisk-module-template) is to add the folder you want to remove into the `REPLACE` list within `config.sh`. If your module doesn't provide a corresponding folder, it will create an empty folder, and automatically add `.replace` into the empty folder so the dummy folder will properly replace the one in `/system`.
