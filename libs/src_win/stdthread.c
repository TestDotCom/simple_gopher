#include "stdlog.h"
#include "stdthread.h"

int thrd_create(thrd_t* thr, thrd_start_t func, void* args)
{
    thrd_t t;

	if (!(t = CreateThread(NULL, 0, func, args, 0, NULL))) {
        log_error("stdthread::make_thread - CreateThread(). Error-code: %ld", GetLastError());
        return THRD_ERROR;
    }

    if (thr != NULL) {
        *thr = t;
    }

    return THRD_SUCCESS;
}

int thrd_join(thrd_t thr, int* res)
{
    if (WaitForSingleObject(thr, INFINITE) != WAIT_OBJECT_0) {
        if (res) {
            *res = THRD_ERROR;
        }

        log_error("stdthread::make_thread - WaitForSingleObject(). Error-code: %ld", GetLastError());
        return THRD_ERROR;
    }

    if (res) {
        *res = THRD_SUCCESS;
    }

    return THRD_SUCCESS;
}

int thrd_detach(thrd_t thr)
{
    if (!CloseHandle(thr)) {
        log_error("stdthread::thrd_detach - CloseHandle(). Error-code: %ld", GetLastError());
        return THRD_ERROR;
    }

    return THRD_SUCCESS;
}
