#include "log_client.h"
#include "stdthread.h"
#include "stdlog.h"

static int pipefd[2] = { -1 };
static evn_t* event = NULL;
static FILE* log_file = NULL;

static void cleanup()
{
	if (pipefd[0] != -1) {
		if (close(pipefd[0]) == -1) {
			log_errno_msg("log_client::cleanup - close(pipefd[0])");
		}

		pipefd[0] = -1;
	}

	if (event) {
		evn_destroy(event);
		event = NULL;
	}

	if (log_file) {
		if (fclose(log_file) == EOF) {
			log_errno_msg("log_client::cleanup - fclose()");
		}

		log_file = NULL;
	}
}

void logclient_destroy()
{
	if (pipefd[1] != -1) {
		if (close(pipefd[1]) == -1) {
			log_errno_msg("log_client::logclient_destroy - close pipefd[1]");
		}

		pipefd[1] = -1;
	}

	// logger enters critical section, then cleanup resources
	if (evn_signal(event) == EVN_ERROR) {
		log_debug("log_client::logclient_destroy - logger process not signaled");
	}
}

static void logger_main()
{
	char buf[MAX_LOG];
	ssize_t nbytes;

    if (close(pipefd[1]) == -1) {
		log_errno_msg("[logger process] close(pipefd[1])");
	}

    if (!(log_file = fopen("client.log", "a"))) {
		log_errno_msg("[logger process] fopen()");
		cleanup();

		_exit(EXIT_FAILURE);
    }

    for (;;) {
		if (evn_wait(event) == EVN_ERROR) {
			log_errno_msg("[logger process] evn_wait()");
			cleanup();

			_exit(EXIT_FAILURE);
		}

		if ((nbytes = read(pipefd[0], buf, MAX_LOG - 1)) <= 0) {
			cleanup();

			if (nbytes == 0) {	// write-end closed
				log_debug("[logger process] parent write-end pipe closed, exiting");
				_exit(EXIT_SUCCESS);
			}

			log_errno_msg("[logger process] read()");
			_exit(EXIT_FAILURE);
		}

		if (fprintf(log_file, "%s", buf) == -1) {
	        log_errno_msg("[logger process] fprintf()");
			cleanup();

			_exit(EXIT_FAILURE);
        }

		// force write of buffered data
		if (fflush(log_file) == EOF) {
			log_errno_msg("[logger process] fflush()");
		}
    }
}

int logclient_init()
{
    if (pipe(pipefd) == -1) {
        log_errno_msg("log_client::logclient_init - pipe()");
		return -1;
    }

    if (!(event = evn_init(0))) {
		cleanup();
		return -1;
	}

	switch (fork()) {
		case 0: logger_main(); break;
		case -1: 
			log_errno_msg("client_info::init_logger - fork()");
			cleanup();

			return -1;
		default:
			if (close(pipefd[0]) == -1) {
				log_errno_msg("log_client::logclient_init - close(pipefd[0])");
			}
	}

	return 0;
}

int write_log(info_t const* info)
{
	int len;
	char* logstr = fmt_clientlog(info, &len);

	if (!logstr) {
		return -1;
	}

    if (write(pipefd[1], logstr, len) <= 0) {
		log_errno_msg("log_client::write_log - write()");
		free(logstr);

		if (errno == EPIPE) {	// read-end closed
			log_debug("logger read-end pipe closed")
			return 0;
		}

		return -1;
	}

	free(logstr);

	if (evn_signal(event) == EVN_ERROR) {
		log_error("log_client::write_log - event signal not sent");
		return -1;
	}

	return 0;
}
