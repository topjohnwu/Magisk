# Magisk Modules
## Magisk Module Format
A Magisk module is a folder placed in the root folder in `magisk.img`, which has a structure as described below:

```
$MOUNTPOINT
├── .
├── .
├── $MODID                  <--- The ID of the module, should match with module.prop
│   ├── auto_mount          <--- If this file exists, auto mount is enabled
│   ├── disable             <--- If this file exists, the module is disabled
│   ├── module.prop         <--- This files stores the identity and properties of the module
│   ├── post-fs-data.sh     <--- This script will be executed in post-fs-data
│   ├── remove              <--- If this file exists, the module will be removed next reboot
│   ├── service.sh          <--- This script will be executed in late_start service
│   ├── system.prop         <--- This file will be loaded as system props
│   ├── system              <--- If auto mount is enabled, Magisk will "Magic Mount" this folder
│   │   ├── .
│   │   ├── .
│   │   └── .
│   ├── vendor              <--- Auto generated. A symlink to $MODID/system/vendor
│   ├── .                   <--- Any other files/folders are allowed
│   └── .		
├── another_module
│   ├── .
│   └── .
├── .
├── .
```
You are not required to use my Magisk Module Template to create a module. As long as you place files with the structure above, it will be recognized as a module.

## Magisk Module Template
The **Magisk Module Template** is hosted **[here](https://github.com/topjohnwu/magisk-module-template)**.

It is a template to create a flashable zip to install Magisk Modules. It is designed to be simple to use so that anyone can create their own modules easily. The template itself contains minimal scripting for installation; most of the functions are located externally in [util_functions.sh](https://github.com/topjohnwu/Magisk/blob/master/scripts/util_functions.sh), which will be installed along with Magisk, and can be upgraded through a Magisk upgrade without the need of a template update.

Here are some files you would want to know:

- `config.sh`: A simple script used as a configuration file. It is the place to configure which features your module needs/disables. A detailed instructions on how to use the template is also written in this file.
- `module.prop`: This file contains your module's indentity and properties, including name and versions etc.. This file will be used to identify your module on an actual device and in the [Magisk Modules Repo](https://github.com/Magisk-Modules-Repo)
- `common/*`: Boot stage scripts and `system.prop`
- `META-INF/com/google/android/update-binary`: The actual installation script. Modify this file for advanced custom behavior

And here are some notes to be aware of:

- The template depends on external Magisk scripts, please specify the correct `minMagisk` value in `module.prop` with the template version your module is based on, or the minimum Magisk version your module is tested on.
- **Windows users please check here!!** The line endings of all text files should be in the **Unix format**. Please use advanced text editors like Sublime, Atom, Notepad++ etc. to edit **ALL** text files, **NEVER** use Windows Notepad.
- In `module.prop`, `version` can be an arbitrary string, so any fancy version name (e.g. `ultra-beta-v1.1.1.1`) is allowed. However, `versionCode` **MUST** be an integer. The value is used for version comparison.
- Make sure your module ID **does not contain any spaces**.