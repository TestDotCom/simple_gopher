#ifndef STDTHREAD_H_
#define STDTHREAD_H_

#ifdef _WIN32
#include <windows.h>

typedef HANDLE thrd_t;
typedef LPTHREAD_START_ROUTINE thrd_start_t;
#else
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#define EVN_SUCCESS 0
#define EVN_ERROR 1

typedef pthread_t thrd_t;
typedef void* (*thrd_start_t)(void*);

typedef struct evn_t evn_t;

evn_t* evn_init(int value);
void evn_destroy(evn_t* evn);
int evn_signal(evn_t* evn);
int evn_wait(evn_t* evn);
#endif

#define THRD_SUCCESS 1
#define THRD_ERROR 0

int thrd_create(thrd_t* thr, thrd_start_t func, void* args);
int thrd_join(thrd_t thr, int* res);
int thrd_detach(thrd_t thr);

#endif // STDTHREAD_H_
