#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#include "stdgopher.h"
#include "info.h"
#include "stdlog.h"

static HANDLE hStdin = INVALID_HANDLE_VALUE;
static info_t* info = NULL;

static void cleanup()
{
    if (WSACleanup() == SOCKET_ERROR) {
		log_error("subserver::cleanup - WSACleanup(). Error-code: %d", WSAGetLastError());
	}
    
    free(info);
    info = NULL;

    if (hStdin != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(hStdin)) {
            log_error("subserver::WinMain - CloseHandle(hStdin). Error-code: %ld", GetLastError());
        }

        hStdin = INVALID_HANDLE_VALUE;
    }

    simplelog_destroy();
}

INT main()
{
    INT res;

    WSAPROTOCOL_INFO protocolInfo;
    WSADATA wsaData;

    if (simplelog_init(1, "subsrv.log") == -1) {
        fprintf(stderr, "logging messages only to console\n");
    }

    log_debug("child %ld starting", GetCurrentProcessId());

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        log_error("subserver::WinMain - GetStdHandle(). Error-code: %ld", GetLastError());
        cleanup();

        ExitProcess(1);
    }

    if (!(info = malloc(sizeof *info))) {
        log_error("subserver::WinMain - malloc(). Error-code: %ld", GetLastError());
        cleanup();

        ExitProcess(1);
    }

	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
        log_error("subserver::WinMain - WSAstartup(). Error-code: %ld", WSAGetLastError());
        cleanup();

        ExitProcess(1);
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        log_error("subserver::WinMain - Winsock.dll < 2. Error-code: %ld", WSAGetLastError());
        cleanup();

        ExitProcess(1);
    }

    if (!ReadFile(hStdin, &protocolInfo, sizeof (WSAPROTOCOL_INFO), NULL, NULL)) {
        log_error("subserver::WinMain - ReadFile(protocolInfo). Error-code: %ld", GetLastError());
        cleanup();

        ExitProcess(1);
    }

    if (!ReadFile(hStdin, info, sizeof *info, NULL, NULL)) {
        log_error("subserver::WinMain - ReadFile(info). Error-code: %ld", GetLastError());
        cleanup();

        ExitProcess(1);
    }

    info->sockfd = WSASocket(AF_UNSPEC, SOCK_STREAM, 0, &protocolInfo, 0, 0);
    if (info->sockfd == -1) {
        log_error("subserver::WinMain - WSAsocket(). Error-code: %d", WSAGetLastError());
        cleanup();
        
        ExitProcess(1);
    }

    exchange_message(info);
    cleanup();

    return 0;
}
