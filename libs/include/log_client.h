#ifndef LOG_CLIENT_H_
#define LOG_CLIENT_H_

#include "info.h"

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#endif

int logclient_init(void);
void logclient_destroy(void);

int write_log(info_t const* info);

#endif // LOG_CLIENT_H_
