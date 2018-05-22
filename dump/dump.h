#ifndef DUMP_H
#define DUMP_H
#ifdef __cplusplus
extern "C" {
#define CLOSE_EXTERN }
#else
#define CLOSE_EXTERN
#endif

#include <stdio.h>

#define uint8_t unsigned char

#pragma GCC visibility push(hidden)
void dump(FILE * out, const char *addr, const long len);
#pragma GCC visibility pop

CLOSE_EXTERN
#endif
