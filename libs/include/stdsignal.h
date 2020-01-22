#ifndef STDSIGNAL_H_
#define STDSIGNAL_H_

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define SIGHUP 1
#define SIGTERM 15
#define SIGCHLD 17

#define SIGPORT "8888"
#define SIGBUF 4

int write_signal(int signum);
#else
#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

int install_sig_handler(void);
int read_signal(void);

void wait_child(void);      // linux only

#endif // STDSIGNAL_H_
