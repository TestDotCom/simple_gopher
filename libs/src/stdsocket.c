#include "stdsocket.h"
#include "stdlog.h"

int send_message(int sockfd, void const* msg, size_t size)
{
	size_t total = 0;
	size_t bytesleft = size;
	ssize_t n;

	while (total < size) {
		n = send(sockfd, msg + total, bytesleft, 0);
		if (n == -1) { 
			#ifdef WIN32
			log_error("stdsock::send_message - send(). Error-code: %d", WSAGetLastError());
			#else
			if (errno != EPIPE) {
				log_errno_msg("stdsock::send_message - send()");
				return -1;
			}
			#endif

			return 0;		// peer closed
		}
			
		total += n;
		bytesleft -= n;
	}

	return 0;
}

char* receive_message(int sockfd)
{
	size_t total = 0;
	size_t bytesleft = FILENAME_MAX - 1;
	ssize_t n;

	char* buf;
	char* endmsg;

	if (!(buf = malloc(FILENAME_MAX))) {
		log_errno_msg("stdsock::receive_message - malloc()");
		_exit(EXIT_FAILURE);
	}

	while ((n = recv(sockfd, buf + total, bytesleft - total, 0)) > 0) {
		total += n;

		if ((endmsg = strchr(buf, '\r'))) {
			if (*(endmsg + 1) == '\n') {
				// full message received
				*endmsg = *(endmsg + 1) = '\0';
				break;
			}
		}
	}

	if (n == 0) {
        log_debug("peer gracefully closed its connection");
	} else if (n == -1) {
        #ifdef WIN32
		log_error("stdsock::receive_message - recv(). Error-code: %d", WSAGetLastError());
        #else
        log_errno_msg("stdsock::receive_message - recv()");
        #endif

		return NULL;
	}

	return buf;
}

void teardown(int sockfd)
{
    if (sockfd != -1) {
		#ifdef WIN32
		if (closesocket(sockfd) == SOCKET_ERROR) {
			log_error("stdsock::teardown - closesocket(). Error-code: %d", WSAGetLastError());
		}
		#else
		if (close(sockfd) == -1) {
			log_errno_msg("stdsock::teardown - close()");
		}
		#endif
	}
}

/*
 * allow to bind to exactly the same [source address, port] 
 * and set non-blocking I/O mode (linux - workaround select() bug).
*/
static void setup_socket(int sockfd)
{
	const int optval = 1;

	#ifdef WIN32
	if (setsockopt(sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &optval, sizeof optval) == SOCKET_ERROR) {
		log_error("socket_wrap::start_server - setsockopt(); %d", WSAGetLastError());
	}
	#else
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval) == -1) {
		log_errno_msg("socket_wrap::start_server - setsockopt()");
	}

	int flags;
	if (-1 != (flags = fcntl(sockfd, F_GETFL, 0))) {
		if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
			log_errno_msg("socket_wrap::setup_socket - fcntl(F_SETFL)");
		}
	} else {
		log_errno_msg("socket_wrap::setup_socket - fcntl(F_GETFL)");
	}
	#endif
}

int make_socket(char const* nodename, char const* srv_port, int type, 
		struct sockaddr* ai_addr, socklen_t* ai_addrlen)
{
	int err;

	int s;
    struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;		// IPV4 or IPV6

	switch (type) {
		case TCPSRV:
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;

			break;
		case UDPSRV:
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_flags = AI_PASSIVE;

			break;
		case UDPCLI: hints.ai_socktype = SOCK_DGRAM; break;
		default: break;
	}

	err = getaddrinfo(nodename, srv_port, &hints, &servinfo);

	if (err) {
		log_error("stdsock::make_socket - getaddrinfo(). Error-code: %s", gai_strerror(err));
		return -1;
	}

	// loop through all the results and bind to the first available
    for (p = servinfo; p != NULL; p = p->ai_next) {
		if (-1 == (s = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {
			#ifdef WIN32
			log_error("stdsock::make_socket - socket(). Error-code: %d", WSAGetLastError());
			#else
			log_errno_msg("stdsock::make_socket - socket()");
			#endif

			continue;
		}

		if (type == UDPSRV || type == TCPSRV) {
			setup_socket(s);

			if (bind(s, p->ai_addr, p->ai_addrlen) == -1) {
				#ifdef WIN32
				log_error("stdsock::make_socket - bind(). Error-code: %d", WSAGetLastError());
				#else
				log_errno_msg("stdsock::make_socket - bind()");
				#endif

				teardown(s);
				continue;
			}
		}

		// successful connection
		break;
	}

	if (p == NULL) {
		log_error("could not bind");
		freeaddrinfo(servinfo);

		return -1;
	}

	if (ai_addr) {
		memcpy(ai_addr, p->ai_addr, sizeof (struct sockaddr));
	}

	if (ai_addrlen) {
		*ai_addrlen = p->ai_addrlen;
	}

	freeaddrinfo(servinfo);

	return s;
}
