#include "stdthread.h"
#include "srvsize.h"
#include "stdlog.h"

struct evn_t 
{
    int count;
    pthread_mutex_t mtx;
    pthread_cond_t cnd;
};

int thrd_create(thrd_t* thr, thrd_start_t func, void* args)
{
    int err;
	char errbuf[ERRMSG_MAX];

    thrd_t t;

    if ((err = pthread_create(&t, NULL, func, args))) {
		log_error("stdthread::thrd_create - pthread_create(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        return THRD_ERROR;
    }

    if (thr) {
        *thr = t;
    }

    return THRD_SUCCESS;
}

int thrd_join(thrd_t thr, int* res)
{
    int err;
	char errbuf[ERRMSG_MAX];

    void* retval;

    if ((err = pthread_join(thr, &retval))) {
        log_error("stdthread::thrd_join - pthread_join(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        return THRD_ERROR;
    }

    if (res) {
        res = retval;
    }

    return THRD_SUCCESS;
}

int thrd_detach(thrd_t thr)
{
    int err;
	char errbuf[ERRMSG_MAX];

    if ((err = pthread_detach(thr))) {
        log_error("stdthread::thrd_detach - pthread_detach(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        return THRD_ERROR;
    }

    return THRD_SUCCESS;
}

static void destroy_mtx(pthread_mutex_t* mtx)
{
    int err;
    char errbuf[ERRMSG_MAX];

    if ((err = pthread_mutex_destroy(mtx))) {
        log_error("event::destroy_mtx - pthread_mutex_destroy(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
    }
}

static void destroy_mtxattr(pthread_mutexattr_t* mattr)
{
    int err;
    char errbuf[ERRMSG_MAX];

    if ((err = pthread_mutexattr_destroy(mattr))) {
        log_error("event::destroy_mtx - pthread_mutexattr_destroy(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
    }
}

static int ipc_mutex(pthread_mutex_t* mtx)
{
    int err;
    char errbuf[ERRMSG_MAX];

    pthread_mutexattr_t mattr;

    if ((err = pthread_mutexattr_init(&mattr))) {
        log_error("event::ipc_mutex - pthread_mutexattr_init(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

        return EVN_ERROR;
    }

    if ((err = pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED))) {
        log_error("event::ipc_mutex - pthread_mutexattr_setpshared(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        destroy_mtxattr(&mattr);

        return EVN_ERROR;
    }

    if ((err = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK))) {
        log_error("event::ipc_mutex - pthread_mutexattr_settype(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        destroy_mtxattr(&mattr);

        return EVN_ERROR;
    }

    if ((err = pthread_mutex_init(mtx, &mattr))) {
        log_error("event::ipc_mutex - pthread_mutex_init(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        destroy_mtxattr(&mattr);

        return EVN_ERROR;
    }

    destroy_mtxattr(&mattr);

    return EVN_SUCCESS;
}

static void destroy_cond(pthread_cond_t* cond)
{
    int err;
    char errbuf[ERRMSG_MAX];

    if ((err = pthread_cond_destroy(cond))) {
        log_error("event::destroy_cond - pthread_cond_destroy(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
    }
}

static void destroy_condattr(pthread_condattr_t* cattr)
{
    int err;
    char errbuf[ERRMSG_MAX];

    if ((err = pthread_condattr_destroy(cattr))) {
        log_error("event::ipc_cond - pthread_condattr_destroy(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
    }
}

static int ipc_cond(pthread_cond_t* cnd)
{
    int err;
    char errbuf[ERRMSG_MAX];

    pthread_condattr_t cattr;

    if ((err = pthread_condattr_init(&cattr))) {
        log_error("event::ipc_cond - pthread_condattr_init(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

        return EVN_ERROR;
    }

    if ((err = pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED))) {
        log_error("event::ipc_cond - pthread_condattr_setpshared(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        destroy_condattr(&cattr);

        return EVN_ERROR;
    }

    if ((err = pthread_cond_init(cnd, &cattr))) {
        log_error("event::ipc_cond - pthread_cond_init(). %s", strerror_r(err, errbuf, ERRMSG_MAX));
        destroy_condattr(&cattr);

        return EVN_ERROR;
    }
    
    destroy_condattr(&cattr);

    return EVN_SUCCESS;
}

evn_t* evn_init(int value)
{
    evn_t* evn = 
        mmap((void*) -1, sizeof (evn_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (evn == MAP_FAILED) {
        log_errno_msg("event::init_evn - mmap()");
        return NULL;
    }

    if (ipc_mutex(&(evn->mtx)) == EVN_ERROR) {
        return NULL;
    }

    if (ipc_cond(&(evn->cnd)) == EVN_ERROR) {
        destroy_mtx(&(evn->mtx));
        return NULL;
    }

    evn->count = value;

    return evn;
}

int evn_signal(evn_t* evn)
{
    int err;
    char errbuf[ERRMSG_MAX];
    
    assert(evn);

    if ((err = pthread_mutex_lock(&(evn->mtx)))) {
        log_error("event::evn_signal - pthread_mutex_lock(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

        return EVN_ERROR;
    }

    evn->count++;

    if ((err = pthread_cond_signal(&(evn->cnd)))) {
        log_error("event::evn_signal - pthread_cond_signal(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

        return EVN_ERROR;
    }

    if ((err = pthread_mutex_unlock(&(evn->mtx)))) {
        log_error("event::evn_signal - pthread_mutex_unlock(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

        return EVN_ERROR;
    }

    return EVN_SUCCESS;
}

int evn_wait(evn_t* evn)
{
    int err;
    char errbuf[ERRMSG_MAX];
    
    assert(evn);

    if ((err = pthread_mutex_lock(&(evn->mtx)))) {
        log_error("event::evn_wait - pthread_mutex_lock(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

        return EVN_ERROR;
    }

    while (!evn->count) {
        if ((err = pthread_cond_wait(&(evn->cnd), &(evn->mtx)))) {
            log_error("event::evn_wait - pthread_cond_wait(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

            return EVN_ERROR;
        }
    }

    evn->count--;

    if ((err = pthread_mutex_unlock(&(evn->mtx)))) {
        log_error("event::evn_wait - pthread_mutex_unlock(). %s", strerror_r(err, errbuf, ERRMSG_MAX));

        return EVN_ERROR;
    }

    return EVN_SUCCESS;
}

void evn_destroy(evn_t* evn)
{
    assert(evn);

    destroy_mtx(&(evn->mtx));
    destroy_cond(&(evn->cnd));

    if (munmap(evn, sizeof (evn_t)) == -1) {
        log_errno_msg("event::evn_destroy - munmpap()");
    }
}
