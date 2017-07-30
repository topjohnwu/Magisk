# Magisk Manager
This is one of the submodules used in Magisk. The project is licensed under GPL v3 (or newer).
More info are written in the [Magisk Main Repo](https://github.com/topjohnwu/Magisk)

## Building Notes
You need to install CMake and NDK to build the zipadjust library.
There are several files required to let Magisk Manager work properly, and should be compiled/copied by using the build script in the [Magisk Main Repo](https://github.com/topjohnwu/Magisk). These files are: `jniLibs/<arch>/libbusybox.so`, `assets/magisk_uninstaller.sh`, `assets/magisk_uninstaller.sh`, `assets/public.certificate.x509.pem`, and `assets/private.key.pk8`.
