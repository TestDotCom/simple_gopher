#ifndef SRVSIZE_H_
#define SRVSIZE_H_

#include <stddef.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

#define FILESIZE_LEN 8      // format range 001.0B - 999.9EB
#define ERRMSG_MAX 1024
#define MSG_MAX (ERRMSG_MAX + 127)

#define MAX_LOG (NI_MAXHOST + NI_MAXSERV + FILENAME_MAX + FILESIZE_LEN + 15)

#endif // SRVSIZE_H_
