#ifndef COMMON_H
#define COMMON_H

#include <netdb.h>

struct connection{
	sockaddr_storage src;
	sockaddr_storage dst;
};

#include <stdlib.h>

//Crash if OOM.
void* kalloc(size_t nmemb, size_t size);

#include <stdio.h>

extern FILE* error_log;

//##__VA_ARGS is gcc/clang only syntax.
#define errprint(fmt, ...) \
            do { fprintf(error_log, fmt, ##__VA_ARGS__); fflush(error_log); } while (0)

#endif
