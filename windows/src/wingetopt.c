#include "wingetopt.h"

int	opterr = 1;
int	optind = 1;
int	optopt;
char *optarg;

int getopt(int argc, char *const argv[], char const* optstring)
{
	static int nextchar = 1;

	int c;
	char* cp;

	if (optind >= argc || 
			(argv[optind][0] != '-' && argv[optind][1] == '\0')) {
		return -1;
	}

	c = argv[optind][nextchar];

	if (!(cp = strchr(optstring, c))) {
		// invalid option
		optopt = c;
		return '?';
	}

	cp++;

	if (cp[0] == ':') {
		// option takes an argument
		if (++optind >= argc) {
			// argument required
			nextchar = 1;
			return '?';
		}

		optarg = argv[optind];
	} else {
		optarg = NULL;
	}

	optind++;
	nextchar = 1;

	return c;
}
