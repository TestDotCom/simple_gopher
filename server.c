#include "socket_wrap.h"
#include "stdconf.h"
#include "log_client.h"
#include "stdlog.h"

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include "wingetopt.h"
#include <windows.h>
#else
#include "daemonize.h"
#include <unistd.h>
#endif

static void parse_cmd_args(int argc, char** argv, unsigned int* mode, char* port)
{
    int opt;

    while (-1 != (opt = getopt(argc, argv, "p:m:h"))) {
        switch (opt) {
            case 'p': copy_port(optarg, port); break;
            case 'm': copy_mode(optarg, mode); break;
            case 'h': 
                printf("Usage: %s [OPTIONS]\nOptions:\n\t"
                    "-p\tlistening port, default 7070\n\t"
                    "-m\texecution mode, 0 for multi-thread, 1 for multi-process\n\t"
                    "-h\tprint help\n", 
                    argv[0]
                );

                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s [-p port] [-m 0|1]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv)
{
    unsigned int mode;
    char port[MAX_PORT];

    #ifdef WIN32
    if (!SetCurrentDirectory("home/")) {
        log_error("server::main - SetCurrentDirectory(). Error-code: %ld", GetLastError());
        ExitProcess(1);
    }
    #else
    daemonize();
    /*if (chdir("home/") == -1) {
        perror("server - chdir()");
        exit(EXIT_FAILURE);
    }*/
    #endif

    if (simplelog_init(1, "server.log") == -1) {
        fprintf(stderr, "logging messages only to console\n");
    }

    if (logclient_init() == -1) {
        simplelog_destroy();
        exit(EXIT_FAILURE);
    }

    if (read_conf(&mode, port) == -1) {
        log_warn("invalid conf.ini");
    }

    parse_cmd_args(argc, argv, &mode, port);

    start_server(mode, port);
}
