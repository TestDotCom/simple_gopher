#include "log_client.h"
#include "stdlog.h"

static HANDLE hPipe = INVALID_HANDLE_VALUE;
static HANDLE hEvent = INVALID_HANDLE_VALUE;

static VOID cleanup()
{
	if (hPipe != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(hPipe)) {
			log_error("log_client::cleanup - CloseHandle(hPipe). Error-code: %ld", GetLastError());
		}

		hPipe = INVALID_HANDLE_VALUE;
	}

	if (hEvent != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(hEvent)) {
			log_error("log_client::cleanup - CloseHandle(hEvent). Error-code: %ld", GetLastError());
		}

		hEvent = INVALID_HANDLE_VALUE;
	}
}

VOID logclient_destroy()
{
	if ((hEvent = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, TRUE, "log_event"))) {
		// logger enters critical section, then cleanup resources
    	if (!SetEvent(hEvent)) {
			log_error("log_client::logclient_destroy - SetEvent(). Error-code: %ld", GetLastError());
		}
	} else {
		log_error("log_client::logclient_destroy - OpenEvent(). Error-code: %ld", GetLastError());
	}

	cleanup();
}

INT logclient_init()
{
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

	SecureZeroMemory(&piProcInfo, sizeof (PROCESS_INFORMATION));
	SecureZeroMemory(&siStartInfo, sizeof (STARTUPINFO));

	BOOL res = CreateProcess(
		NULL,
		"client_logger.exe",
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&siStartInfo,
		&piProcInfo);

	if (!res) {
		log_error("log_client::logclient_init - CreateProcess(). Error-code: %ld", GetLastError());
		cleanup();

		return -1;
	}

	if (!CloseHandle(piProcInfo.hProcess)) {
		log_error("log_client::logclient_init() - CloseHandle(piProcInfo.hProcess). Error-code: %ld", GetLastError());
	}

	if (!CloseHandle(piProcInfo.hThread)) {
		log_error("log_client::logclient_init() - CloseHandle(piProcInfo.hThread). Error-code: %ld", GetLastError());
	}

	return 0;
}

INT write_log(info_t const* info)
{
	INT len;
	CHAR* logstr = fmt_clientlog(info, &len);

	if (!logstr) {
		return -1;
	}

	hPipe = CreateFile(
		"\\\\.\\pipe\\log_pipe",
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE) {
		log_error("log_client::write_log - CreateFile(). Error-code: %ld", GetLastError());
		return -1;
	}

	if (!(hEvent = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, TRUE, "log_event"))) {
		log_error("log_client::write_log - OpenEvent(). Error-code: %ld", GetLastError());
		cleanup();

		return -1;
	}

    if (!WriteFile(hPipe, logstr, len, NULL, NULL)) {
		log_error("log_client::write_log - WriteFile(). Error-code: %ld", GetLastError());
		free(logstr);
		cleanup();

		return -1;
	}

	free(logstr);

	if (!(SetEvent(hEvent))) {
		log_error("log_client::write_log - SetEvent(). Error-code: %ld", GetLastError());
		cleanup();

		return -1;
	}

	cleanup();

	return 0;
}
