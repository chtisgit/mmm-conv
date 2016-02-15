#ifndef MMMCONV_H
#define MMMCONV_H

/* DEBUG MACROS */

#ifdef NDEBUG
#ifdef DEBUG
#undef DEBUG
#endif
#else
#define DEBUG
#endif

#ifdef DEBUG
#define debug(...) fprintf(stderr, "debug:: " __VA_ARGS__)
#else
#define debug(...) 
#endif

/* NEEDED INCLUDES */

#include <stdlib.h>


/* OTHER MACROS AND DEFINES */

#define error(...)	do{ \
			fprintf(stderr, "Error: " __VA_ARGS__); exit(1); \
			}while(0)

#define OUTPUT_FILENAME	"questions.js"

/* RESOURCES */

extern const char *progname;

void not_enough_mem(size_t sz, int line);
void help(void);
#endif
