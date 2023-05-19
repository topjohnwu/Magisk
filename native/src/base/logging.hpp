#pragma once

#include <cerrno>
#include <cstdarg>

void LOGD(const char *fmt, ...) __printflike(1, 2);
void LOGI(const char *fmt, ...) __printflike(1, 2);
void LOGW(const char *fmt, ...) __printflike(1, 2);
void LOGE(const char *fmt, ...) __printflike(1, 2);
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, std::strerror(errno))
