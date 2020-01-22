#ifndef STDLOG_H_
#define STDLOG_H_

#include "info.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define S_INFO "info"
#define S_WARN "warn"
#define S_DEBUG "debug"
#define S_ERROR "error"

#define log_info(...) log_msg(S_INFO, __VA_ARGS__);
#define log_warn(...) log_msg(S_WARN, __VA_ARGS__);
#define log_debug(...) log_msg(S_DEBUG, __VA_ARGS__);
#define log_error(...) log_msg(S_ERROR, __VA_ARGS__);

int simplelog_init(int logtofile, char const* fname);
void simplelog_destroy(void);

void log_msg(char const* level, char const* fmt, ...);

inline void log_errno_msg(char const* fmt)
{
    log_msg(S_ERROR, "%s. %s", fmt, strerror(errno));
}

char* fmt_clientlog(info_t const* info, int* len);

#endif // STDLOG_H_
