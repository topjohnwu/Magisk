#if defined(__arm__)
#include <armeabi-v7a_binaries.h>
#elif defined(__aarch64__)
#include <arm64-v8a_binaries.h>
#elif defined(__i386__)
#include <x86_binaries.h>
#elif defined(__x86_64__)
#include <x86_64_binaries.h>
#else
#error Unsupported ABI
#endif
