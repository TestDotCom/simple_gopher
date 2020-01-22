#ifndef IO_WRAP_H_
#define IO_WRAP_H_

#include "info.h"

#ifdef _WIN32
#include <shlwapi.h>
#include <strsafe.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#endif

#define ISDIR 1
#define ISFILE 0

int find_file(info_t* info);

#endif // IO_WRAP_H_
