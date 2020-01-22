#include "multiplx.h"
#include "stdgopher.h"
#include "stdlog.h"
#include "stdthread.h"

static void* thread_main(void* arg)
{
    exchange_message((info_t*) arg);
    
	return 0;
}

void make_thread(info_t* info)
{
    thrd_t thr;

	if (thrd_create(&thr, thread_main, (void*) info) == THRD_SUCCESS) {
		thrd_detach(thr);
	}
}

void make_process(info_t* info)
{
    switch (fork()) {
		case 0: 
			log_debug("child %d starting", getpid()); 
			exchange_message(info); 
			
			_exit(EXIT_SUCCESS);
		case -1: 
			log_errno_msg("server_wrap::make_process - fork()"); 
			exit(EXIT_FAILURE);
		default: break;
	}
}
