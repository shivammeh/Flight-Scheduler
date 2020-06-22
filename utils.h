#pragma once

#include <stdint.h>

typedef uint64_t u64;

extern double time_sec(void);

extern u64 random_u64(void);

extern void srandom_u64();
