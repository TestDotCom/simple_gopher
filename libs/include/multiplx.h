#ifndef MULTIPLX_H_
#define MULTIPLX_H_

#include "info.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <stdlib.h>
#endif

void make_thread(info_t* info);
void make_process(info_t* info);

#endif // MULTIPLX_H_
