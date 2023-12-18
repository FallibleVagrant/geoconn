#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#include <stdio.h>

//##__VA_ARGS is gcc/clang only syntax.
#define dbgprint(fmt, ...) \
            do { if (DEBUG) {fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr);} } while (0)

#endif

