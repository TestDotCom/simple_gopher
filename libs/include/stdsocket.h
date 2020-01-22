#ifndef STDSOCKET_H_
#define STDSOCKET_H_

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define TCPSRV 0
#define UDPSRV 1
#define UDPCLI 2

void teardown(int sockfd);

int make_socket(char const* nodename, char const* srv_port, int type, 
        struct sockaddr* ai_addr, socklen_t* ai_addrlen);

int send_message(int sockfd, void const* msg, size_t size);
char* receive_message(int sockfd);

#endif // STDSOCKET_H_
