#ifndef DAEMONIZE_H_
#define DAEMONIZE_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

void daemonize(void);

#endif // DAEMONIZE_H_
