#ifndef N64_STDLIB_N64_STDCKDINT_H_
#define N64_STDLIB_N64_STDCKDINT_H_

#define n64_ckd_add( result, a, b ) __builtin_add_overflow( a, b, result )
#define n64_ckd_sub( result, a, b ) __builtin_sub_overflow( a, b, result )
#define n64_ckd_mul( result, a, b ) __builtin_mul_overflow( a, b, result )

#endif
