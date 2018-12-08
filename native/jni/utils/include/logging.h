/* logging.h - Error handling and logging
 */

#pragma once

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define str(a) #a
#define xstr(a) str(a)

typedef enum {
	L_DEBUG,
	L_INFO,
	L_WARN,
	L_ERR
} log_type;

struct log_callback {
	int (*d)(const char* fmt, va_list ap);
	int (*i)(const char* fmt, va_list ap);
	int (*w)(const char* fmt, va_list ap);
	int (*e)(const char* fmt, va_list ap);
	void (*ex)(int code);
};

extern struct log_callback log_cb;

#define LOGD(...) log_handler(L_DEBUG, __VA_ARGS__)
#define LOGI(...) log_handler(L_INFO, __VA_ARGS__)
#define LOGW(...) log_handler(L_WARN, __VA_ARGS__)
#define LOGE(...) log_handler(L_ERR, __VA_ARGS__)
#define PLOGE(fmt, args...) LOGE(fmt " failed with %d: %s\n", ##args, errno, strerror(errno))

int nop_log(const char *fmt, va_list ap);
void nop_ex(int i);

void no_logging();
void android_logging();
void cmdline_logging();

int log_handler(log_type t, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
