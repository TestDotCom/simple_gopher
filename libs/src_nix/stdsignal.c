#include "stdsignal.h"
#include "stdlog.h"

static int sfd;

int read_signal()
{
	struct signalfd_siginfo fdsi;

	if (read(sfd, &fdsi, sizeof (struct signalfd_siginfo)) == -1) {
		log_errno_msg("signal_handler::read_signal - read()");
		return -1;
	}

	return fdsi.ssi_signo;
}

int install_sig_handler()
{
    sigset_t mask;

    sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGPIPE);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
		log_errno_msg("signal_handler::install_sig_handler - sigprocmask()");
        return -1;
	}

	if (-1 == (sfd = signalfd(-1, &mask, 0))) {
		log_errno_msg("signal_handler::install_sig_handler - signalfd()");
	}

    return sfd;
}

void wait_child()
{
	int pid;
	while ((pid = waitpid((pid_t) -1, NULL, WNOHANG)) > 0) {
		log_debug("SIGCHLD");
	}

	if (pid == -1) {
		log_errno_msg("signal_handler::wait_child - waitpid");
	}
}
