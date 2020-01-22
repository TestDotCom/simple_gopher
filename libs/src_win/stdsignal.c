#include "stdsignal.h"
#include "stdsocket.h"
#include "stdlog.h"

static SOCKET sigSock;

INT write_signal(int signum)
{
    int s;
    struct sockaddr remote_addr;
	socklen_t addrlen;

    char buf[SIGBUF];
    errno_t err;

    if (-1 == (s = make_socket("localhost", SIGPORT, UDPCLI, &remote_addr, &addrlen))) {
        log_debug("signal not sent");
        return -1;
    }

    if ((err = _itoa_s(signum, buf, SIGBUF, 10)) > 0) {
        log_error("signal_handler::write_signal - sendto(). Error-code: %d", err);
        return -1;
    }

    if (sendto(s, buf, SIGBUF - 1, 0, &remote_addr, addrlen) == -1) {
        log_error("signal_handler::write_signal - sendto(). Error-code: %d", WSAGetLastError());
        return -1;
    }

    closesocket(s);

    return 0;
}

INT read_signal()
{
    char buf[SIGBUF];
    
    if (recvfrom(sigSock, buf, SIGBUF - 1 , 0, NULL, NULL) == SOCKET_ERROR) {
        log_error("signal_handler::write_signal - recvfrom(). Error-code: %d", WSAGetLastError());
        return -1;
    }

    return atoi(buf);
}

static BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType) {
        case CTRL_C_EVENT: write_signal(SIGHUP); return TRUE;
        case CTRL_CLOSE_EVENT: write_signal(SIGTERM); return TRUE;
        default: return FALSE;
    }
}

INT install_sig_handler()
{
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        log_error("signal_handler::install_sig_handler - SetConsoleCtrlHandler(). Error-code: %ld", GetLastError());
        return -1;
    }

    sigSock = make_socket("localhost", SIGPORT, UDPSRV, NULL, NULL);

    return sigSock;
}

/*
 * stub
*/
VOID wait_child() {
    return;
}
