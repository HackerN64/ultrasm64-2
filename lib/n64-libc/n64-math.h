#ifndef N64_STDLIB_N64_MATH_H_
#define N64_STDLIB_N64_MATH_H_

/* Only a subset of math.h is currently implemented */

#define N64_NAN __builtin_nanf( "0" )
#define N64_HUGE_VALF __builtin_inff()
#define N64_HUGE_VAL __builtin_inf()
#define N64_INFINITY N64_HUGE_VALF

#ifndef NAN
	#define NAN N64_NAN
#endif

#ifndef HUGE_VALF
	#define HUGE_VALF N64_HUGE_VALF
#endif

#ifndef HUGE_VAL
	#define HUGE_VAL N64_HUGE_VAL
#endif

#ifndef INFINITY
	#define INFINITY N64_INFINITY
#endif

__attribute__((const, warn_unused_result))
float n64_expf( float arg );

__attribute__((const, warn_unused_result))
float n64_exp2f( float arg );

__attribute__((const, warn_unused_result))
float n64_logf( float arg );

__attribute__((const, warn_unused_result))
float n64_log10f( float arg );

__attribute__((const, warn_unused_result))
float n64_log2f( float arg );

__attribute__((const, warn_unused_result))
float n64_powf( float base, float exponent );

__attribute__((const, warn_unused_result))
float n64_cbrtf( float arg );

__attribute__((const, warn_unused_result))
float n64_hypotf( float x, float y );

__attribute__((const, warn_unused_result))
double n64_hypot( double x, double y );

__attribute__((const, warn_unused_result))
float n64_sinf( float arg );

__attribute__((const, warn_unused_result))
float n64_cosf( float arg );

__attribute__((const, warn_unused_result, always_inline))
static inline float n64_sqrtf( float arg ) {
	float result;
	asm volatile( "sqrt.s %0, %1" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline double n64_sqrt( double arg ) {
	double result;
	asm volatile( "sqrt.d %0, %1" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline float n64_fabsf( float arg ) {
	float result;
	asm volatile( "abs.s %0, %1" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline double n64_fabs( double arg ) {
	double result;
	asm volatile( "abs.d %0, %1" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline double n64_floor( double arg ) {
	if( arg > 4503599627370496.0 || arg < -4503599627370496.0 ) return arg;
	double result;
	asm volatile( "floor.l.d %0, %1 \n\t cvt.d.l %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline double n64_ceil( double arg ) {
	if( arg > 4503599627370496.0 || arg < -4503599627370496.0 ) return arg;
	double result;
	asm volatile( "ceil.l.d %0, %1 \n\t cvt.d.l %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline double n64_trunc( double arg ) {
	if( arg > 4503599627370496.0 || arg < -4503599627370496.0 ) return arg;
	double result;
	asm volatile( "trunc.l.d %0, %1 \n\t cvt.d.l %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline double n64_round( double arg ) {
	if( arg > 4503599627370496.0 || arg < -4503599627370496.0 ) return arg;
	double result;
	asm volatile( "round.l.d %0, %1 \n\t cvt.d.l %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline double n64_rint( double arg ) {
	if( arg > 4503599627370496.0 || arg < -4503599627370496.0 ) return arg;
	double result;
	asm volatile( "cvt.l.d %0, %1 \n\t cvt.d.l %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline float n64_floorf( float arg ) {
	if( arg > 8388608.f || arg < -8388608.f ) return arg;
	float result;
	asm volatile( "floor.w.s %0, %1 \n\t cvt.s.w %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline float n64_ceilf( float arg ) {
	if( arg > 8388608.f || arg < -8388608.f ) return arg;
	float result;
	asm volatile( "ceil.w.s %0, %1 \n\t cvt.s.w %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline float n64_truncf( float arg ) {
	if( arg > 8388608.f || arg < -8388608.f ) return arg;
	float result;
	asm volatile( "trunc.w.s %0, %1 \n\t cvt.s.w %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline float n64_roundf( float arg ) {
	if( arg > 8388608.f || arg < -8388608.f ) return arg;
	float result;
	asm volatile( "round.w.s %0, %1 \n\t cvt.s.w %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline float n64_rintf( float arg ) {
	if( arg > 8388608.f || arg < -8388608.f ) return arg;
	float result;
	asm volatile( "cvt.w.s %0, %1 \n\t cvt.s.w %0, %0" : "=f"( result ) : "f"( arg ) );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline long long n64_llround( double arg ) {
#ifdef _ABIO32
	double bits;
	asm volatile( "round.l.d %0, %1" : "=f"( bits ) : "f"( arg ) );
	union { double f; long long i; } b = { bits };
	return b.i;
#else
	long long result;
	asm volatile( "round.l.d $f10, %1 \n\t dmfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
#endif
}

__attribute__((const, warn_unused_result, always_inline))
static inline long long n64_llrint( double arg ) {
#ifdef _ABIO32
	double bits;
	asm volatile( "cvt.l.d %0, %1" : "=f"( bits ) : "f"( arg ) );
	union { double f; long long i; } b = { bits };
	return b.i;
#else
	long long result;
	asm volatile( "cvt.l.d $f10, %1 \n\t dmfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
#endif
}

// Extensions: fill in gaps in the C standard for rounding functions

__attribute__((const, warn_unused_result, always_inline))
static inline long long n64_llfloor( double arg ) {
#ifdef _ABIO32
	double bits;
	asm volatile( "floor.l.d %0, %1" : "=f"( bits ) : "f"( arg ) );
	union { double f; long long i; } b = { bits };
	return b.i;
#else
	long long result;
	asm volatile( "floor.l.d $f10, %1 \n\t dmfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
#endif
}

__attribute__((const, warn_unused_result, always_inline))
static inline long long n64_llceil( double arg ) {
#ifdef _ABIO32
	double bits;
	asm volatile( "ceil.l.d %0, %1" : "=f"( bits ) : "f"( arg ) );
	union { double f; long long i; } b = { bits };
	return b.i;
#else
	long long result;
	asm volatile( "ceil.l.d $f10, %1 \n\t dmfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
#endif
}

__attribute__((const, warn_unused_result, always_inline))
static inline long long n64_lltrunc( double arg ) {
#ifdef _ABIO32
	double bits;
	asm volatile( "trunc.l.d %0, %1" : "=f"( bits ) : "f"( arg ) );
	union { double f; long long i; } b = { bits };
	return b.i;
#else
	long long result;
	asm volatile( "trunc.l.d $f10, %1 \n\t dmfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
#endif
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_ifloorf( float arg ) {
	int result;
	asm volatile( "floor.w.s $f10, %1 \n\t mfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_iceilf( float arg ) {
	int result;
	asm volatile( "ceil.w.s $f10, %1 \n\t mfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_itruncf( float arg ) {
	int result;
	asm volatile( "trunc.w.s $f10, %1 \n\t mfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_iroundf( float arg ) {
	int result;
	asm volatile( "round.w.s $f10, %1 \n\t mfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_irintf( float arg ) {
	int result;
	asm volatile( "cvt.w.s $f10, %1 \n\t mfc1 %0, $f10" : "=r"( result ) : "f"( arg ) : "f10" );
	return result;
}

#endif
