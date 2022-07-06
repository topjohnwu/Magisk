#pragma once

#include <cerrno>
#include <cstdarg>

struct log_callback {
    void (*d)(const char* fmt, va_list ap);
    void (*i)(const char* fmt, va_list ap);
    void (*w)(const char* fmt, va_list ap);
    void (*e)(const char* fmt, va_list ap);
};

extern log_callback log_cb;

void LOGD(const char *fmt, ...) __printflike(1, 2);
void LOGI(const char *fmt, ...) __printflike(1, 2);
void LOGW(const char *fmt, ...) __printflike(1, 2);
void LOGE(const char *fmt, ...) __printflike(1, 2);
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, std::strerror(errno))

void nop_log(const char *, va_list);

void forward_logging_to_rs();
void cmdline_logging();
void exit_on_error(bool b);
