#ifndef WINGETOPT_H_
#define WINGETOPT_H_

#include <string.h>

extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;

int getopt(int argc, char *const argv[], char const* optstring);

#endif // WINGETOPT_H_
