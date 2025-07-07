#ifndef N64_STDLIB_N64_STDLIB_H_
#define N64_STDLIB_N64_STDLIB_H_

#include "n64-stddef.h"

#ifndef RAND_MAX
#define RAND_MAX 0x7fffffff
#endif

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((always_inline, artificial, noreturn))
static inline void n64_abort() {
	__builtin_trap();
}

__attribute__((access(none, 1), const, warn_unused_result, always_inline))
static inline unsigned int n64_memalignment( const void *p ) {
	return (unsigned int)p & (-(unsigned int)p);
}

__attribute__((nonnull(1, 4), access(read_write, 1)))
void n64_qsort( void *ptr, unsigned int count, unsigned int size, int(*comp)(const void*, const void*) );

__attribute__((nonnull(2, 5), alloc_align(4), alloc_size(3, 4), warn_unused_result))
void *n64_bsearch( const void *key, const void *ptr, unsigned int count, unsigned int size, int(*comp)(const void*, const void*) );

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_abs( int n ) {
	return (n < 0) ? -n : n;
}

__attribute__((const, warn_unused_result, always_inline))
static inline long long n64_llabs( long long n ) {
	return (n < 0ll) ? -n : n;
}

__attribute__((pure, nonnull(1), warn_unused_result))
int n64_atoi( const char *str );

__attribute__((pure, nonnull(1), warn_unused_result))
long long n64_atoll( const char *str );

// Extension. Same as stroll, but returns an 32-bit int
__attribute__((pure, nonnull(1), access(read_write, 2)))
int n64_strtoi( const char *str, char **str_end, int base );

// Extension. Same as stroull, but returns an 32-bit unsigned int
__attribute__((pure, nonnull(1), access(read_write, 2)))
unsigned int n64_strtoui( const char *str, char **str_end, int base );

__attribute__((pure, nonnull(1), access(read_write, 2)))
long long n64_strtoll( const char *str, char **str_end, int base );

__attribute__((pure, nonnull(1), access(read_write, 2)))
unsigned long long n64_strtoull( const char *str, char **str_end, int base );

__attribute__((flatten))
void n64_srand( unsigned int seed );

/* Extension. Works just like the standard C `rand` function, but returns an
 * unsigned value with a full 32 bits of randomness instead of just 31 bits.
 */
unsigned int n64_randu();

/* Extension. Works just like the standard C `rand' function, but returns an
 * unsigned short value with 16 bits of randomness.
 */
__attribute__((always_inline))
static inline unsigned short n64_randhu() {
	return (unsigned short)(n64_randu() >> 16);
}

__attribute__((always_inline))
static inline int n64_rand() {
	return (int)(n64_randu() >> 1);
}

/* Extension. Generate a random single precision floating point value
 * greater than or equal to 0 and strictly less than 1
 */
__attribute__((warn_unused_result))
float n64_randf();

/* Extension. Generate a random double precision floating point value
 * greater than or equal to 0 and strictly less than 1
 */
__attribute__((warn_unused_result))
double n64_randd();

#ifdef __cplusplus
}
#endif

#endif
