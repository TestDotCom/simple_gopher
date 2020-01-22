#include "stdconf.h"
#include "stdlog.h"

static char const* confpath = "conf.ini";

void copy_port(char* value, char* port) {
    while (isspace((unsigned char) *value)) {
        ++value;    // trim leading space
    }

    unsigned int i = 0;
    while (isdigit((unsigned char) value[i])
            && i < MAX_PORT - 1) {
        port[i] = value[i];
        ++i;
    }

    port[i] = '\0';
}

void copy_mode(char* value, unsigned int* mode) {
    *mode = strtoimax(value, NULL, 10);

    if (*mode > 1) {
        log_warn("mode not valid: choose 0 or 1\n\tstarting with default mode 0");
        *mode = SRV_THRD;
    }
}

int read_conf(unsigned int* mode, char* port)
{
    FILE* file;

    char line[MAX_LINE];
    char* value;

    if (!(file = fopen(confpath, "r"))) {
        log_errno_msg("server::read_conf - fopen()");
        return -1;
    }

    // search for a valid config line [param=value]
    while (fgets(line, MAX_LINE, file)) {
        if ((value = strchr(line, '='))) {
            ++value;    // trim '='

            if (strstr(line, "mode")) {
                copy_mode(value, mode);
            } else if (strstr(line, "port")) {
                copy_port(value, port);
            }
        }
    }

    if (fclose(file) == EOF) {
        log_errno_msg("server::read_conf - fclose()");
    }

    return 0;
}
