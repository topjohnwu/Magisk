## Boot Stages
If you are working on complicated projects, you shall need more control to the whole process. Magisk can run scripts in different boot stages, so you can fine tune exactly what you want to do. It's recommended to read this documentation along with the procedure graph.

- post-fs mode
    - **This stage is BLOCKING. Boot process will NOT continue until everything is done, or 20 seconds has passed**
    - Happens after most partitions are mounted, except `/data`
    - Magisk will bind mount files under `/cache/magisk_mount/system` to corresponding paths
    - It is only **Simple Mount**, which means it will replace existing files, but cannot add/remove files.
- post-fs-data mode
    - **This stage is BLOCKING. Boot process will NOT continue until everything is done, or 60 seconds has passed**
    - Happens after `/data` is ready (including the case when `/data` is encrypted)
    - Happens before Zygote and system servers are started (which means pretty much everything)
    - Mirrors will be mounted. These mirrors play a critical role in **Magic Mount**
    - `/data/magisk.img` will be merged, trimmed, and mounted to `/magisk`
    - Magisk will run scripts under `/magisk/.core/post-fs-data.d`
    - Magisk will run scripts: `/magisk/$MODID/post-fs-data.sh` (placed in each module directory)
    - Magisk will finally **Magisk Mount** module files
- late_start service mode
    - **This stage is NON-BLOCKING, it will run in parallel with other processes**
    - Happens when class late_start is started
    - Put time consuming but non time critical tasks here. Boot process will be stuck if it took too long to finish your tasks in previous modes
    - SELinux is guaranteed to be fully patched in this stage; **it is recommended to run most scripts in this stage**, unless your scripts require some time critical operations
    - Magisk will run scripts under `/magisk/.core/service.d`
    - Magisk will run scripts: `/magisk/$MODID/service.sh` (placed in each module directory)

## Magic Mount Details
### Terminology
- **Item**: A folder, file, or symbolic link
- **Leaf**: The very end of a directory structure tree, can be either a file or symbolic link
- **`$MODPATH`**: A variable to represent the path of a module folder
- **Source item**: An item under `$MODPATH/system`, for example, `$MODPATH/system/build.prop` is a source item
- **Existing item**: An item under `/system`, for example, `/system/bin/app_process` is an existing item
- **Target item**: A corresponding item of a source item. For example, the target item of `$MODPATH/system/build.prop` is `/system/build.prop`

Note: A target item does not imply it is an existing item. A target item might not exist in the actual `/system`

### Policies
- For a source leaf: if its target item is also an existing item, the existing item will be replaced with the source leaf
- For a source leaf: if its target item is not an existing item, the source leaf will be added to the path of its target item
- For any existing item that's not a target item, it will stay intact

Above is the rule of thumb. Basically it means that Magic Mount merges the files from `$MODPATH/system` into the real `/system`. A simpler way to understand is to think as it dirty copies the contents from `$MODPATH/system` into `/system`.

However, an addition rule will override the above policies:

- For a source folder containing the file `.replace`, the source folder will be treated as if it is a source leaf. That is, the items within the target folder will be completely discarded, and the target folder will be replaced with the source folder.

Directories containing a file named `.replace` will NOT be merged into the system. It will directly replace the target directory. A simpler way to understand is to think as if it wipes the target folder, and then copies the whole folder to the target path.

### Notes
- Sometimes, completely replacing a folder is inevitable. For example you want to replace `/system/priv-app/SystemUI` in your stock rom. In stock roms, system apps usually comes with pre-optimized files. If your replacement `SystemUI.apk` is deodexed (which is most likely the case), you would want to replace the whole `/system/priv-app/SystemUI` to make sure the folder only contains the modified `SystemUI.apk` and **NOT** merge with the pre-optimized files.
- If you are using the [Magisk Module Template](https://github.com/topjohnwu/magisk-module-template), you can create a list of folders you want to replace in the file `config.sh`. The installation scripts will handle the creation of `.replace` files into the listed folders for you.
- Adding non-existing target items is a relatively expensive operation. Magisk would need to do **MANY** under-the-hood tasks to achieve it. Replacing a whole folder is recommended if viable, it reduces the complexity of the construction of the mounting tree and could speed up the booting time

## Simple Mount Details
Some files require to be mounted much earlier in the boot process, currently known are bootanimation and some libs (most users won't change them). You can simply place your modified files into the corresponding path under `/cache/magisk_mount`. At boot time, Magisk will **clone all the attributes from the target file**, which includes selinux context, permission mode, owner, group. It'll then bind mount the file to the target. This means you don't need to worry about the metadatas for files placed under `/cache/magisk_mount`: copy the file to the correct place, reboot then you're done!

This mode does not feature the same complex Magic Mount implementation, it will mount a source leaf to an existing target item under `/system`.

For example, you want to replace `/system/media/bootanimation.zip`, copy your new boot animation zip to `/cache/magisk_mount/system/media/bootanimation.zip,` Magisk will mount your files in the next reboot.
