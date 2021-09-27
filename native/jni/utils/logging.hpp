#pragma once

#include <cerrno>
#include <cstdarg>

struct log_callback {
    int (*d)(const char* fmt, va_list ap);
    int (*i)(const char* fmt, va_list ap);
    int (*w)(const char* fmt, va_list ap);
    int (*e)(const char* fmt, va_list ap);
    void (*ex)(int code);
};

extern log_callback log_cb;

void LOGD(const char *fmt, ...) __printflike(1, 2);
void LOGI(const char *fmt, ...) __printflike(1, 2);
void LOGW(const char *fmt, ...) __printflike(1, 2);
void LOGE(const char *fmt, ...) __printflike(1, 2);
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, std::strerror(errno))

int nop_log(const char *, va_list);
void nop_ex(int);

void no_logging();
void cmdline_logging();
