## Boot Stages
If you are working on complicated projects, you shall need more control to the whole process. Magisk can run scripts in different boot stages, so you can fine tune exactly what you want to do. It's recommended to read this documentation along with the procedure graph.

- post-fs mode
    - **This stage is BLOCKING. Boot process will NOT continue until everything is done, or 10 seconds has passed**
    - Happens after most partitions are mounted. `/data` might not be available since `vold` is not started yet
    - Magisk will bind mount files under `/cache/magisk_mount/system` and `/cache/magisk_mount/vendor`
    - It is only **Simple Mount**, which means it will replace existing files, but cannot add/remove files.
    - This part is mostly deprecated (reasons in details)
- post-fs-data mode
    - **This stage is BLOCKING. Boot process will NOT continue until everything is done, or 10 seconds has passed**
    - Happens after `/data` is ready (including the case when `/data` is encrypted)
    - Happens before Zygote and system servers are started (which means pretty much everything)
    - `/data/adb/magisk.img` will be merged, trimmed, and mounted to `MOUNTPOINT=/sbin/.core/img`
    - Magisk will run scripts under `$MOUNTPOINT/.core/post-fs-data.d`
    - Magisk will run scripts: `$MOUNTPOINT/$MODID/post-fs-data.sh` (placed in each module directory)
    - Magisk will finally **Magisk Mount** module files
- late_start service mode
    - **This stage is NON-BLOCKING, it will run in parallel with other processes**
    - Happens when class late_start is triggered
    - The daemon will wait for the full `sepolicy` patch before running this stage, so SELinux is guaranteed to be fully patched
    - Put time consuming scripts here. Boot process will get stuck if it took too long to finish your tasks in `post-fs-data`
    - **It is recommended to run all scripts in this stage**, unless your scripts requires doing stuffs before Zygote is started
    - Magisk will run scripts under `$MOUNTPOINT/.core/service.d`
    - Magisk will run scripts: `$MOUNTPOINT/$MODID/service.sh` (placed in each module directory)

## Magic Mount Details
### Terminology
- **Item**: A folder, file, or symbolic link
- **Leaf**: An item that is on the very end of a directory structure tree. It can be either a file or symbolic link
- **`$MODPATH`**: A variable to represent the path of a module folder
- **Source item**: An item under `$MODPATH/system`, for example, `$MODPATH/system/bin/app_process32` is a source item
- **Existing item**: An item in the actual filesystem, for example, `/system/bin/app_process32` is an existing item
- **Target item**: The corresponding item of a source item. For example, the target item of `$MODPATH/system/bin/app_process32` is `/system/bin/app_process32`

Note: A target item **does not** imply it is an existing item. A target item might not exist in the actual filesystem

### Policies
- For a source leaf: if its target item is also an existing item, the existing item will be replaced with the source leaf
- For a source leaf: if its target item is not an existing item, the source leaf will be added to the path of its target item
- For any existing item that's not a target item, it will stay intact

Above is the rule of thumb. Basically it means that Magic Mount merges the two folders, `$MODPATH/system` into `/system`. A simpler way to understand is to think as the items is dirty copied from `$MODPATH/system` into `/system`.

However, an addition rule will override the above policies:

- For a source folder containing the file `.replace`, the source folder will be treated as if it is a leaf. That is, the items within the target folder will be completely discarded, and the target folder will be replaced with the source folder.

Directories containing a file named `.replace` will **NOT** be merged, instead it directly replaces the target directory. A simpler way to understand is to think as if it wipes the target folder, and then copies the whole folder to the target path.

### Notes
- If you want to replace files in `/vendor`, please place it under `$MODPATH/system/vendor`. Magisk will handle both cases, whether the vendor partition is separated or not under-the-hood, developers don't need to bother.
- Sometimes, completely replacing a folder is inevitable. For example you want to replace `/system/priv-app/SystemUI` in your stock rom. In stock roms, system apps usually comes with pre-optimized files. If your replacement `SystemUI.apk` is deodexed (which is most likely the case), you would want to replace the whole `/system/priv-app/SystemUI` to make sure the folder only contains the modified `SystemUI.apk` without the pre-optimized files.
- If you are using the [Magisk Module Template](https://github.com/topjohnwu/magisk-module-template), you can create a list of folders you want to replace in the file `config.sh`. The installation scripts will handle the creation of `.replace` files into the listed folders for you.

## Simple Mount Details
(Note: this part is mostly deprecated, since starting with devices using A/B partitions, there is no longer a dedicated partition for cache because OTAs are applied live at boot. Instead, `/cache` now points to `/data/cache`, which means `post-fs` mode does not have access to `/cache` anymore)

Some files require to be mounted much earlier in the boot process, currently known are bootanimation and some libs (most users won't change them). You can simply place your modified files into the corresponding path under `/cache/magisk_mount`. For example, you want to replace `/system/media/bootanimation.zip`, copy your new boot animation zip to `/cache/magisk_mount/system/media/bootanimation.zip`, and Magisk will mount your files in the next reboot. Magisk will **clone all the attributes from the target file**, which includes selinux context, permission mode, owner, group. This means you don't need to worry about the metadata for files placed under `/cache/magisk_mount`: just copy the file to the correct place, reboot then you're done!
