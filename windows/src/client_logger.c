#include "srvsize.h"
#include "stdlog.h"

#include <windows.h>
#include <stdio.h>

static HANDLE hLog = INVALID_HANDLE_VALUE;
static HANDLE hPipe = INVALID_HANDLE_VALUE;
static HANDLE hEvent = INVALID_HANDLE_VALUE;

static VOID cleanup()
{
    if (hPipe != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(hPipe)) {
            log_error("logger::cleanup - CloseHandle(hPipe). Error-code: %ld", GetLastError());
        }

        hPipe = INVALID_HANDLE_VALUE;
    }
    
    if (hEvent != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(hEvent)) {
            log_error("logger::cleanup - CloseHandle(hEvent). Error-code: %ld", GetLastError());
        }

        hEvent = INVALID_HANDLE_VALUE;
    }

    if (hLog != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(hLog)) {
            log_error("logger::cleanup - CloseHandle(hLog). Error-code: %ld", GetLastError());
        }

        hLog = INVALID_HANDLE_VALUE;
    }

    simplelog_destroy();
}

static HANDLE make_namedpipe()
{
    if (hPipe != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(hPipe)) {
            log_error("logger::make_namedpipe - CloseHandle(). Error-code: %ld", GetLastError());
        }
    }

    hPipe = CreateNamedPipe(
        "\\\\.\\pipe\\log_pipe",
        PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
        1,
        0,
        MAX_LOG,
        0,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        log_error("logger::make_namedpipe - CreateNamedPipe(). Error-code: %ld", GetLastError());
    }

    return hPipe;
}

INT main()
{
    CHAR buf[MAX_LOG];
    DWORD byteRead;

    if (simplelog_init(1, "logger.log") == -1) {
        fprintf(stderr, "logging messages only to console\n");
    }

    // ignore CTRL-C
    if (!SetConsoleCtrlHandler(NULL, TRUE)) {
        log_error("logger::WinMain - SetConsoleCtrlHandler(). Error-code: %ld", GetLastError());
        return -1;
    }

    hLog = CreateFile("client.log", 
        FILE_APPEND_DATA, 
        0, 
        NULL, 
        OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL
    );

    if (hLog == INVALID_HANDLE_VALUE) {
        log_error("logger::WinMain - CreateFile(). Error-code: %ld", GetLastError());
        cleanup();

        ExitProcess(1);
    }

    if (INVALID_HANDLE_VALUE == (hPipe = make_namedpipe())) {
        cleanup();

        ExitProcess(1);
    }

    if (!(hEvent = CreateEvent(NULL, FALSE, FALSE, "log_event"))) {
        log_error("logger::WinMain - CreateEvent(). Error-code: %ld", GetLastError());
        cleanup();

        ExitProcess(1);
    }

    for (;;) {
        if (WaitForSingleObject(hEvent, INFINITE) != WAIT_OBJECT_0) {
            log_error("logger::WinMain - WaitForSingleObject(). Error-code: %ld", GetLastError());
            cleanup();

            ExitProcess(1);
        }

        if (!ReadFile(hPipe, buf, MAX_LOG - 1, &byteRead, NULL)) {
            cleanup();
            ExitProcess(1);
        }

        // open new pipe instance
        if (INVALID_HANDLE_VALUE == (hPipe = make_namedpipe())) {
            cleanup();

            ExitProcess(1);
        }

        if (!WriteFile(hLog, buf, byteRead - 1, NULL, NULL)) {
            log_error("logger::WinMain - WriteFile(). Error-code: %ld", GetLastError());
            cleanup();
            
            ExitProcess(1);
        }

        // force write of buffered data
        if (!FlushFileBuffers(hLog)) {
            log_error("logger::WinMain - FlushFileBuffers(). Error-code: %ld", GetLastError());
        }
    }

    return 0;
}
