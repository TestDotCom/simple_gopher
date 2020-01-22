#ifndef INFO_H_
#define INFO_H_

#include "srvsize.h"

typedef struct info_t 
{
    int sockfd;
    char client_host[NI_MAXHOST];
    char client_port[NI_MAXSERV];
    char* filename;
    void* sendbuf;
    size_t sendlen;
} info_t;

#endif // INFO_H_
