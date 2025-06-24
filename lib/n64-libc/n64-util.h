#ifndef N64_STDLIB_N64_UTIL_H_
#define N64_STDLIB_N64_UTIL_H_

#include "n64-stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enables or disables interrupts and returns whether interrupts were previously enabled or not
 * 
 * Example usage:
 * const n64_bool intr = n64_set_interrupts( false );
 * // Do critical section work here
 * n64_set_interrupts( intr );
 */
n64_bool n64_set_interrupts( n64_bool enable );

/* Waits for PI I/O operations (like a DMA) to complete. You should disable interrupts before calling this.
 *
 * Example usage:
 * const n64_bool intr = n64_set_interrupts( false );
 * n64_await_pi_io();
 * // Now you can safely do a PI write here
 * n64_set_interrupts( intr );
 */
void n64_await_pi_io();

__attribute__((always_inline))
static inline void n64_memory_barrier() {
	asm volatile( "":::"memory" );
}

__attribute__((const, always_inline))
static inline unsigned int n64_bit_cast_ftoi( float x ) {
	union { float f; unsigned int i; } b = { x };
	return b.i;
}

__attribute__((const, always_inline))
static inline float n64_bit_cast_itof( unsigned int x ) {
	union { unsigned int i; float f; } b = { x };
	return b.f;
}

__attribute__((const, always_inline))
static inline unsigned long long n64_bit_cast_dtol( double x ) {
	union { double f; unsigned long long i; } b = { x };
	return b.i;
}

__attribute__((const, always_inline))
static inline double n64_bit_cast_ltod( unsigned long long x ) {
	union { unsigned long long i; double f; } b = { x };
	return b.f;
}

#ifdef __cplusplus
}
#endif

#endif
