#pragma once

#include <cerrno>
#include <cstdarg>
#include <cstring>

enum {
    L_DEBUG,
    L_INFO,
    L_WARN,
    L_ERR
};

struct log_callback {
    int (*d)(const char* fmt, va_list ap);
    int (*i)(const char* fmt, va_list ap);
    int (*w)(const char* fmt, va_list ap);
    int (*e)(const char* fmt, va_list ap);
    void (*ex)(int code);
};

extern log_callback log_cb;

#define LOGD(...) log_handler<L_DEBUG>(__VA_ARGS__)
#define LOGI(...) log_handler<L_INFO>(__VA_ARGS__)
#define LOGW(...) log_handler<L_WARN>(__VA_ARGS__)
#define LOGE(...) log_handler<L_ERR>(__VA_ARGS__)
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, std::strerror(errno))

int nop_log(const char *, va_list);
void nop_ex(int);

void no_logging();
void cmdline_logging();

template<int type>
void log_handler(const char *fmt, ...) __printflike(1, 2);
