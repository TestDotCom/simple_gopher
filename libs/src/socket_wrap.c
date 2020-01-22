#include "socket_wrap.h"
#include "stdsocket.h"
#include "stdlog.h"
#include "stdconf.h"
#include "log_client.h"
#include "stdsignal.h"
#include "multiplx.h"

static int sockfd = -1;
static int sfd = -1;

static jmp_buf env;

static void cleanup()
{
	teardown(sockfd);
	sockfd = -1;

	teardown(sfd);
	sfd = -1;
}

static void close_server()
{
	cleanup();

	#ifdef WIN32
	if (WSACleanup() == SOCKET_ERROR) {
		log_error("socket_wrap::close_server - WSACleanup(); %d", WSAGetLastError());
	}
	#endif

	logclient_destroy();
    simplelog_destroy();
}

static int getpeer(info_t* info)
{
	struct sockaddr_storage remote_addr;
	socklen_t addrlen = sizeof remote_addr;

	int new_sockfd = accept(sockfd, (struct sockaddr*) &remote_addr, &addrlen);

	if (new_sockfd == -1) {
		#ifdef WIN32
		log_error("socket_wrap::start_server - accept(). Error-code: %d", WSAGetLastError());
		#else
		log_errno_msg("socket_wrap::start_server - accept()");

		if (errno == EWOULDBLOCK) {		// see select(2) - BUGS
			return 0;
		}
		#endif

		return -1;
	}

	info->sockfd = new_sockfd;

	int err = getnameinfo(
		(struct sockaddr*) &remote_addr,
		addrlen,
		info->client_host,
		NI_MAXHOST,
		info->client_port,
		NI_MAXSERV,
		NI_NUMERICHOST | NI_NUMERICSERV
	);

	if (err) {
		log_error("socket_wrap::start_server - getnameinfo(). Error-code: %s", gai_strerror(err));
		return -1;
	}

	return 1;
}

void start_server(unsigned int mode, char* srv_port)
{
	info_t* info;

	int err;
    fd_set read_fds;
	int maxfd;

    #ifdef WIN32
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		log_error("socket_wrap::start_server - WSAStartup(). Error-code: %d", WSAGetLastError());
		ExitProcess(1);
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        log_error("socket_wrap::start_server - Winsock.dll < 2. Error-code: %d", WSAGetLastError());
		ExitProcess(1);
    }
	#endif

	if (setjmp(env)) {
		read_conf(&mode, srv_port);
	}

	log_debug("port: %s, mode: %d", srv_port, mode);

	if (-1 == (sfd = install_sig_handler())) {
		close_server();
		exit(EXIT_FAILURE);
	}

	if (-1 == (sockfd = make_socket(NULL, srv_port, TCPSRV, NULL, NULL))) {
		close_server();
		exit(EXIT_FAILURE);
	}

    if (listen(sockfd, SOMAXCONN) == -1) {
		#ifdef WIN32
		log_error("socket_wrap::start_server - listen(). Error-code: %d", WSAGetLastError());
		#else
		log_errno_msg("socket_wrap::start_server - listen()");
		#endif

		close_server();
		exit(EXIT_FAILURE);
	}

	maxfd = (sockfd > sfd) ? sockfd : sfd;
    FD_ZERO(&read_fds);

	log_info("waiting for connections");

    for (;;) {
		FD_SET(sockfd, &read_fds);
		FD_SET(sfd, &read_fds);

		err = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
		if (err == -1) {
			#ifdef WIN32
			log_error("socket_wrap::start_server - select(). Error-code: %d", WSAGetLastError());
			#else
			log_errno_msg("socket_wrap::start_server - select()");
			#endif

			close_server();
			exit(EXIT_FAILURE);
		}
		
		if (FD_ISSET(sockfd, &read_fds)) {
			if (!(info = malloc(sizeof *info))) {
				log_errno_msg("socket_wrap::start_server - malloc(info)");
				close_server();

				exit(EXIT_FAILURE);
			}

			if ((err = getpeer(info)) <= 0) {
				if (!err) {
					continue;
				}

				close_server();
				exit(EXIT_FAILURE);
			}

			// ready to handle new connection
			if (mode == SRV_THRD) {
				make_thread(info);
			} else {
				make_process(info);
				free(info);
			}
		}

		if (FD_ISSET(sfd, &read_fds)) {
			switch (read_signal()) {
				case SIGHUP: log_debug("SIGHUP"); cleanup(); longjmp(env, 1); break;
				case SIGTERM: log_debug("SIGTERM"); close_server(); exit(EXIT_SUCCESS); break;
				case SIGCHLD: wait_child(); break;		// linux only
				default: log_warn("signal not handled");
			}
		}
	}
}
