#ifndef __COMMON_H
#define __COMMON_H

#define swap(a, b) (a ^= b, b ^= a, a ^= b)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#endif
