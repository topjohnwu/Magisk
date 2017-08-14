# Boot Stages
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
    - Mirrors will be mounted. These mirrors play a critical part in **Magic Mount**
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

