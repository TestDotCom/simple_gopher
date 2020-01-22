#include "stdlog.h"

static FILE* flog = NULL;

int simplelog_init(int logtofile, char const* fname)
{
    if (logtofile) {
        if (!(flog = fopen(fname, "a"))) {
            perror("simplelog::simplelog_init - fopen()");
            return -1;
        }
    }
    
    return 0;
}

void simplelog_destroy()
{
    if (flog) {
        if (fclose(flog) == EOF) {
            perror("simplelog::simplelog_destroy - fclose()");
        }

        flog = NULL;
    }
}

void log_msg(char const* level, char const* fmt, ...)
{
    va_list args;
    char msg[MSG_MAX];

    time_t rawt;
    char* localt = NULL;

    if (time(&rawt) == ((time_t) -1)) {
        perror("simplelog::log_msg - time()");
    } else {
        if (!(localt = ctime(&rawt))) {
            perror("simplelog::log_msg - asctime_r()");
        }
    }

    va_start(args, fmt);

    if (vsnprintf(msg, MSG_MAX, fmt, args) == -1) {
        perror("simplelog::log_msg - vsnprintf()");
    }

    va_end(args);

    fprintf(stderr, "[%s] %s\t%s\n", level, localt, msg);
    
    if (flog) {
        fprintf(flog, "[%s] %s\t%s\n", level, localt, msg);

        if (fflush(flog) == EOF) {
            perror("simplelog::log_msg - fflush()");
        }
    }
}

void log_errno_msg(char const* fmt);

char* fmt_clientlog(info_t const* info, int* len)
{
    char* logstr = malloc(MAX_LOG);
    if (!logstr) {
        log_errno_msg("stdlog::fmt_clientlog - malloc()");
        return NULL;
    }

    char const*const suffixes[] = { "EB", "PB", "TB", "GB", "MB", "kB", "B" };
    double magnitudes[] = { pow(10, 18), pow(10, 15), pow(10, 12), 
            pow(10, 9), pow(10, 6), pow(10, 3), pow(10, 0) };
    
    size_t n = sizeof(suffixes) / sizeof(suffixes[0]);
    unsigned int i = 0;

    while (i < n && magnitudes[i] > info->sendlen) {
        ++i;
    }

    int count = snprintf(
		logstr, 
		MAX_LOG, 
		"%s:%s, file: %s, size: %.1f%s\n", 
		info->client_host, 
		info->client_port, 
		info->filename,
		info->sendlen / magnitudes[i],
        suffixes[i]
	);

    if (count == -1) {
		log_errno_msg("log_client::fmt_clientlog - snprintf()");
        free(logstr);

		return NULL;
	}

    *len = count + 1;
    return logstr;
}
