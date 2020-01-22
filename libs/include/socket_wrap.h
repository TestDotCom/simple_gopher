#ifndef SOCKET_WRAP_H_
#define SOCKET_WRAP_H_

#include <setjmp.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#endif

void start_server(unsigned int mode, char* port);

#endif // SOCKET_WRAP_H_
