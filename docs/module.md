## Magisk Module Format
The Magisk module is a folder under `magisk`, which has a structure as described below:

```
magisk
├── .
├── .
├── a_module                <--- The ID of the module, should match with module.prop
│   ├── auto_mount          <--- If this file exists, auto mount is enabled
│   ├── disable             <--- If this file exists, the module is disabled
│   ├── module.prop         <--- This files stores the indentity and properties of the module
│   ├── post-fs-data.sh     <--- This script will be executed in post-fs-data
│   ├── remove              <--- If this file exists, the module will be removed next reboot
│   ├── service.sh          <--- This script will be executed in late_start service
│   ├── system.prop         <--- This file will be loaded as system props
│   ├── system              <--- If auto mount is enabled, Magisk will "Magic Mount" this folder
│   │   ├── app
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

## Magisk Module Template
The **Magisk Module Template** is hosted **[here](https://github.com/topjohnwu/magisk-module-template)**.

It is a template to create a flashable zip to install Magisk Modules. It is created to be simple to use so that anyone can create their own modules easily. The template itself contains minimal scripting for installation; most of the functions are located externally in [util_functions.sh](https://github.com/topjohnwu/Magisk/blob/master/scripts/util_functions.sh), which will be installed along with Magisk, and can be upgraded through a Magisk upgrade instead of a template update.

The template can cover most modules' needs: you can add files into `system`, which will be cloned into `/system` by Magisk after installing the module. In addition to module files, `system.prop`, `post-fs-data.sh`, `service.sh` are also installable through this template.

Here are some files you would want to know:

- `config.sh`: A simple script used as a configuration file for the actual installation file to correctly install you module. It is the place you can select which features your module needs/disables.
- `module.prop`: This file contains your module's indentity and properties, including name and versions etc.. This file will be used to identify your module on an actual device and in the [Magisk Modules Repo](https://github.com/Magisk-Modules-Repo)
- `common/*`: These files will be installed to the module with the correspond role
- `META-INF/com/google/android/update-binary`: The actual installation script. Modify this file for advanced custom behavior

## Create a Magisk Module With The Template
1. Clone / download [this repo](https://github.com/topjohnwu/magisk-module-template)
1. Open `config.sh` and carefully read the fully documented file. Follow the instructions within the script
1. You should at least modified `config.sh` and `module.prop`
1. Directly zip your files, and the result zip file is a flashable zip for both Magisk Manager and custom recoveries
1. Please check **Notes** for precautions

## Notes
- The Module Template depends on external scripts installed by Magisk, so it each version of template will has its own minimal Magisk version requirement.
- **Windows users please aware!!** The line endings on all text files should be the **Unix format**. Please use advanced text editors like Sublime, Atom, Notepad++ etc., **NEVER** use Windows Notepad.
- In `module.prop`, `version` can be an arbitrary string, so any fancy version name (e.g. ultra-beta-v1.1.1.1) is allowed. However, `versionCode` **MUST** be an integer. The value is used for version comparison.
- Make sure your module ID **doesn't contain any spaces**.
