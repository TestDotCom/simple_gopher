#ifndef STDCONF_H_
#define STDCONF_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>

#define SRV_THRD 0
#define SRV_PROC 1

#define MAX_PORT 6     // valid range 0 - 65.535
#define MAX_LINE 17    // small file line limit

void copy_port(char* value, char* port);
void copy_mode(char* value, unsigned int* mode);
int read_conf(unsigned int* mode, char* port);

#endif // STDCONF_H_
