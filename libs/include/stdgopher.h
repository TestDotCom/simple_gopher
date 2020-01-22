#ifndef STDGOPHER_H_
#define STDGOPHER_H_

#include "info.h"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

typedef struct str_t
{
    char* buf;
    int maxlen;
    int len;
} str_t;

int fill_menu(char type, char const* display, info_t const* info, str_t* menu);
void exchange_message(info_t* info);

#endif // STDGOPHER_H_
