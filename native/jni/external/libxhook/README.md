<p align="center"><img src="https://github.com/iqiyi/xHook/blob/master/docs/xhooklogo.png?raw=true" alt="xhook" width="50%"></p>

# xHook

![](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/release-1.2.0-red.svg?style=flat)
![](https://img.shields.io/badge/Android-4.0%20--%2010-blue.svg?style=flat)
![](https://img.shields.io/badge/arch-armeabi%20%7C%20armeabi--v7a%20%7C%20arm64--v8a%20%7C%20x86%20%7C%20x86__64-blue.svg?style=flat)

[README 中文版](README.zh-CN.md)

[Android PLT hook 概述 中文版](docs/overview/android_plt_hook_overview.zh-CN.md)

xHook is a PLT (Procedure Linkage Table) hook library for Android native ELF (executable and shared libraries).

xHook has been keeping optimized for stability and compatibility.


## Features

* Support Android 4.0 - 10 (API level 14 - 29).
* Support armeabi, armeabi-v7a, arm64-v8a, x86 and x86_64.
* Support **ELF HASH** and **GNU HASH** indexed symbols.
* Support **SLEB128** encoded relocation info.
* Support setting hook info via regular expressions.
* Do not require root permission or any system permissions.
* Do not depends on any third-party shared libraries.


## Build

* Download [Android NDK r16b](https://developer.android.com/ndk/downloads/revision_history.html), set environment PATH. (support for armeabi has been removed since r17)

* Build and install the native libraries.

```
./build_libs.sh
./install_libs.sh
```


## Demo

```
cd ./xhookwrapper/
./gradlew assembleDebug
adb install ./app/build/outputs/apk/debug/app-debug.apk
```


## API

External API header file: `libxhook/jni/xhook.h`

### 1. Register hook info

```c
int xhook_register(const char  *pathname_regex_str,  
                   const char  *symbol,  
                   void        *new_func,  
                   void       **old_func);
```

In current process's memory space, in every loaded ELF which pathname matches regular expression `pathname_regex_str`, every PLT entries to `symbol` will be **replaced with** `new_func`. The original one will be saved in `old_func`.

The `new_func` **must** have the same function declaration as the original one.

Return zero if successful, non-zero otherwise.

The regular expression for `pathname_regex_str` only support **POSIX BRE (Basic Regular Expression)**.

### 2. Ignore some hook info

```c
int xhook_ignore(const char *pathname_regex_str,  
                 const char *symbol);
```

Ignore some hook info according to `pathname_regex_str` and `symbol`, from registered hooks by `xhook_register`. If `symbol` is `NULL`, xhook will ignore all symbols from ELF which pathname matches `pathname_regex_str`.

Return zero if successful, non-zero otherwise.

The regular expression for `pathname_regex_str` only support **POSIX BRE**.

### 3. Do hook

```c
int xhook_refresh(int async);
```

Do the real hook operations according to the registered hook info.

Pass `1` to `async` for asynchronous hook. Pass `0` to `async` for synchronous hook.

Return zero if successful, non-zero otherwise.

xhook will keep a global cache for saving the last ELF loading info from `/proc/self/maps`. This cache will also be updated in `xhook_refresh`. With this cache, `xhook_refresh` can determine which ELF is newly loaded. We only need to do hook in these newly loaded ELF.

### 4. Clear cache

```c
void xhook_clear();
```

Clear all cache owned by xhook, reset all global flags to default value.

If you confirm that all PLT entries you want have been hooked, you could call this function to save some memory.

### 5. Enable/Disable debug info

```c
void xhook_enable_debug(int flag);
```

Pass `1` to `flag` for enable debug info. Pass `0` to `flag` for disable. (**disabled** by default)

Debug info will be sent to logcat with tag `xhook`.

### 6. Enable/Disable SFP (segmentation fault protection)

```c
void xhook_enable_sigsegv_protection(int flag);
```

Pass `1` to `flag` for enable SFP. Pass `0` to `flag` for disable. (**enabled** by default) 

xhook is NOT a compliant business layer library. We have to calculate the value of some pointers directly. Reading or writing the memory pointed to by these pointers will cause a segmentation fault in some unusual situations and environment. The APP crash rate increased which caused by xhook is about one ten-millionth (0.0000001) according to our test. (The increased crash rate is also related to the ELFs and symbols you need to hook). Finally, we have to use some trick to prevent this harmless crashing. We called it SFP (segmentation fault protection) which consists of: `sigaction()`, `SIGSEGV`, `siglongjmp()` and `sigsetjmp()`.

**You should always enable SFP for release-APP, this will prevent your app from crashing. On the other hand, you should always disable SFP for debug-APP, so you can't miss any common coding mistakes that should be fixed.**


## Examples

```c
//detect memory leaks
xhook_register(".*\\.so$", "malloc",  my_malloc,  NULL);
xhook_register(".*\\.so$", "calloc",  my_calloc,  NULL);
xhook_register(".*\\.so$", "realloc", my_realloc, NULL);
xhook_register(".*\\.so$", "free",    my_free,    NULL);

//inspect sockets lifecycle
xhook_register(".*\\.so$", "getaddrinfo", my_getaddrinfo, NULL);
xhook_register(".*\\.so$", "socket",      my_socket,      NULL);
xhook_register(".*\\.so$", "setsockopt"   my_setsockopt,  NULL);
xhook_register(".*\\.so$", "bind",        my_bind,        NULL);
xhook_register(".*\\.so$", "listen",      my_listen,      NULL);
xhook_register(".*\\.so$", "connect",     my_connect,     NULL);
xhook_register(".*\\.so$", "shutdown",    my_shutdown,    NULL);
xhook_register(".*\\.so$", "close",       my_close,       NULL);

//filter off and save some android log to local file
xhook_register(".*\\.so$", "__android_log_write",  my_log_write,  NULL);
xhook_register(".*\\.so$", "__android_log_print",  my_log_print,  NULL);
xhook_register(".*\\.so$", "__android_log_vprint", my_log_vprint, NULL);
xhook_register(".*\\.so$", "__android_log_assert", my_log_assert, NULL);

//tracking (ignore linker and linker64)
xhook_register("^/system/.*$", "mmap",   my_mmap,   NULL);
xhook_register("^/vendor/.*$", "munmap", my_munmap, NULL);
xhook_ignore  (".*/linker$",   "mmap");
xhook_ignore  (".*/linker$",   "munmap");
xhook_ignore  (".*/linker64$", "mmap");
xhook_ignore  (".*/linker64$", "munmap");

//defense to some injection attacks
xhook_register(".*com\\.hacker.*\\.so$", "malloc",  my_malloc_always_return_NULL, NULL);
xhook_register(".*/libhacker\\.so$",     "connect", my_connect_with_recorder,     NULL);

//fix some system bug
xhook_register(".*some_vendor.*/libvictim\\.so$", "bad_func", my_nice_func, NULL);

//ignore all hooks in libwebviewchromium.so
xhook_ignore(".*/libwebviewchromium.so$", NULL);

//hook now!
xhook_refresh(1);
```


## Support

1. Check the [xhook-sample](libbiz/jni).
2. Communicate on [GitHub issues](https://github.com/iqiyi/xHook/issues).
3. Mail: <a href="mailto:caikelun@gmail.com">caikelun@gmail.com</a>
4. QQ group: 603635869. QR code:

<p align="left"><img src="docs/qq_group.jpg" alt="qq group" width="300px"></p>


## Contributing

See [xHook Contributing Guide](CONTRIBUTING.md).


## License

xHook is MIT licensed, as found in the [LICENSE](LICENSE) file.

xHook documentation is Creative Commons licensed, as found in the [LICENSE-docs](LICENSE-docs) file.
