#include "stdgopher.h"
#include "stdsocket.h"
#include "io_wrap.h"
#include "log_client.h"
#include "stdlog.h"
#include "stdthread.h"

int fill_menu(char type, char const* display, info_t const* info,  str_t* menu)
{
    const size_t size = strlen(display) + strlen(info->filename) + 
        strlen(info->client_host) + strlen(info->client_port) + 7;

    char row[size];
	const int count = snprintf(
        row,
        size,
        "%c%s\t%s\t%s\t%s\r\n",
        type,
        display,
        info->filename,
        info->client_host,
        info->client_port
    );
    
    if (count == -1) {
        log_errno_msg("stdgopher::fill_menu - snprintf()");
		free(menu->buf);

        return -1;
    }

    if (menu->maxlen - menu->len < count) {
        // make extra space
        menu->maxlen = (menu->maxlen + count) * 4;
        
        menu->buf = realloc(menu->buf, menu->maxlen);
        if (!menu->buf) {
            log_errno_msg("stdgopher::fill_menu - realloc()");
            return -1;
        }
    }

    memcpy(menu->buf + menu->len, row, count);
    menu->len += count;

    return 0;
}

static void* send_file(void* args)
{
	info_t* info = (info_t*) args;
	
	if (send_message(info->sockfd, info->sendbuf, info->sendlen) != -1) {
		if (write_log(info) == -1) {
			log_error("file not logged");
		}
	} else {
		log_error("file not sent");
	}

	#ifdef WIN32
	if (!UnmapViewOfFile(info->sendbuf)) {
		log_error("stdgopher::send_file - UnmapViewOfFile(). Error-code: %ld", GetLastError());
	}
	#else
	if (munmap(info->sendbuf, info->sendlen) == -1) {
		log_errno_msg("stdgopher::send_file - munmap()");
	}
	#endif

	return 0;
}

void exchange_message(info_t* info)
{
	char const* err_str = "3\tnot found\t\t\r\n";
	const size_t err_len = strlen(err_str);

	int type;

	char* recv_buf;
	if (!(recv_buf = receive_message(info->sockfd))) {
		teardown(info->sockfd);
		free(info);

		return;
	}

	if (recv_buf[0] == '\0' || !strcmp(recv_buf, "/")) {
		info->filename = ".";	// select this root
	} else {
		info->filename = recv_buf + 1;	// trim leading "/"
	}

	if (ISDIR == (type = find_file(info))) {
		send_message(info->sockfd, info->sendbuf, info->sendlen);
		free(info->sendbuf);
	} else if (type == ISFILE) {
		thrd_t thr;
		if (thrd_create(&thr, send_file, (void*) info) == THRD_SUCCESS) {
			thrd_join(thr, NULL);
		}	
	} else {
		send_message(info->sockfd, err_str, err_len);
	}

	free(recv_buf);

	// gracefully shutdown this connection
	#ifdef WIN32
	if (shutdown(info->sockfd, SD_SEND) == SOCKET_ERROR) {
		log_error("stdgopher::exchange_message - shutdown(). Error-code: %d", WSAGetLastError());
	}
	#else
	if (shutdown(info->sockfd, SHUT_WR) == -1) {
		log_errno_msg("stdgopher::exchange_message - shutdown()");
	}
	#endif

	teardown(info->sockfd);
	
	free(info);
	info = NULL;
}
