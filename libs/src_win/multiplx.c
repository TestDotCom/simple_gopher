#include "multiplx.h"
#include "stdsocket.h"
#include "stdgopher.h"
#include "stdlog.h"
#include "stdthread.h"

static DWORD thread_main(void* arg)
{
	exchange_message((info_t*) arg);
    
	return 0;
}

VOID make_thread(info_t* info)
{
    thrd_t thr = INVALID_HANDLE_VALUE;

	if (thrd_create(&thr, thread_main, (void*) info) == THRD_SUCCESS) {
		thrd_detach(thr);
	}
}

static VOID close_pipe(HANDLE hPipe[2])
{
	if (!CloseHandle(hPipe[0])) {
		log_error("multiplx::close_pipe - CloseHandle[0]. Error-code: %ld", GetLastError());
	}

	if (!CloseHandle(hPipe[1])) {
		log_error("multiplx::close_pipe - CloseHandle[1]. Error-code: %ld", GetLastError());
	}
}

VOID make_process(info_t* info)
{
    HANDLE hStdIn[2];

	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

	WSAPROTOCOL_INFO protocolInfo;

	SecureZeroMemory(&sa, sizeof (SECURITY_ATTRIBUTES));
	SecureZeroMemory(&piProcInfo, sizeof (PROCESS_INFORMATION));
	SecureZeroMemory(&siStartInfo, sizeof (STARTUPINFO));

    SecureZeroMemory(&protocolInfo, sizeof (WSAPROTOCOL_INFO));
	
	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE; 
    sa.lpSecurityDescriptor = NULL;

	// child inheritable pipe
	if (!CreatePipe(&hStdIn[0], &hStdIn[1], &sa, 0)) {
		log_error("multiplx::make_process - CreatePipe(). Error-code: %ld", GetLastError());
		ExitProcess(1);
	}

	// but do not inherit write-side
	if (!SetHandleInformation(hStdIn[1], HANDLE_FLAG_INHERIT, 0)) {
		log_error("multiplx::make_process - CreatePipe(). Error-code: %ld", GetLastError());
		close_pipe(hStdIn);

		ExitProcess(1);
	}

    siStartInfo.cb = sizeof (STARTUPINFO);
    siStartInfo.hStdInput = hStdIn[0];
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	BOOL res = CreateProcess(
		NULL,
		"subserver.exe",
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&siStartInfo,
		&piProcInfo
	);

	if (!res) {
		log_error("multiplx::make_process - CreateProcess(). Error-code: %ld", GetLastError());
		close_pipe(hStdIn);

		ExitProcess(1);
	}

	if (!CloseHandle(piProcInfo.hProcess)) {
		log_error("multiplx::make_process - CloseHandle(piProcInfo.hProcess). Error-code: %ld", GetLastError());
	}

	if (!CloseHandle(piProcInfo.hThread)) {
		log_error("multiplx::make_process - CloseHandle(piProcInfo.hThread). Error-code: %ld", GetLastError());
	}

	// share this socket between process
    if (WSADuplicateSocket(info->sockfd, piProcInfo.dwProcessId, &protocolInfo) == SOCKET_ERROR) {
		log_error("multiplx::make_process - WSADuplicateSocket(). Error-code: %d", WSAGetLastError());
		close_pipe(hStdIn);

		ExitProcess(1);
	}

	teardown(info->sockfd);

	if (!WriteFile(hStdIn[1], &protocolInfo, sizeof (WSAPROTOCOL_INFO), NULL, NULL)) {
		log_error("multiplx::make_process - WriteFile(protocolInfo). Error-code: %ld", GetLastError());
		close_pipe(hStdIn);

		ExitProcess(1);
	}

	if (!WriteFile(hStdIn[1], info, sizeof *info, NULL, NULL)) {
		log_error("multiplx::make_process - WriteFile(info). Error-code: %ld", GetLastError());
		close_pipe(hStdIn);

		ExitProcess(1);
	}

	close_pipe(hStdIn);
}
