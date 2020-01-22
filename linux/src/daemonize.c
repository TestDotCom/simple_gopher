#include "daemonize.h"
#include "stdlog.h"

/**
 * Detach from this current terminal session and continue
 * running in the background.
 */
void daemonize()
{
    // parent exits and its child is is guaranteed not to be a process group leader
    switch (fork()) {
        case 0: break;
        case -1: perror("daemonize - #1 fork()"); exit(EXIT_FAILURE);
        default: _exit(EXIT_SUCCESS);
    }
    
    // become a process group and session group leader,
    // without a controlling terminal
    if (setsid() == -1) {
        perror("daemonize - setsid()");
		exit(EXIT_FAILURE);
	}

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        perror("daemonize - sigprocmask()");
    }

    // parent (session group leader) exits and its child, which is not a
    // session group leader, can never regain a controlling terminal
    switch (fork()) {
        case 0: break;
        case -1: perror("daemonize - #2 fork()"); exit(EXIT_FAILURE);
        default: _exit(EXIT_SUCCESS);
    }

    if (chdir("home/") == -1) {
        perror("daemonize - chdir()");
        exit(EXIT_FAILURE);
    }

    // complete controll over the permissions of 
    // anything this process writes
    umask(0);   

    // close every inherithd file descriptor,
    // releasing these resources
    for (int fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
		close(fd);
	}

    // ignore standard streams

    if (!(stdin = fopen("/dev/null", "r"))) {
        perror("daemonize - fopen(stdin)");
        exit(EXIT_FAILURE);
    }

    if (!(stdout = fopen("/dev/null", "w"))) {
        perror("daemonize - fopen(stdout)");
        exit(EXIT_FAILURE);
    }

    if (!(stderr = fopen("/dev/null", "w"))) {
        perror("daemonize - fopen(stderr)");
        exit(EXIT_FAILURE);
    }
}
