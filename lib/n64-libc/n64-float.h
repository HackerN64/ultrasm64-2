#ifndef FLT_RADIX
	#define FLT_RADIX 2
#endif

#ifndef FLT_DECIMAL_DIG
	#define FLT_DECIMAL_DIG 9
#endif

#ifndef DBL_DECIMAL_DIG
	#define DBL_DECIMAL_DIG 17
#endif

#ifndef FLT_MIN
	#define FLT_MIN 1.1754944e-38f
#endif

#ifndef DBL_MIN
	#define DBL_MIN 2.2250738585072014e-308
#endif

#ifndef FLT_TRUE_MIN
	#define FLT_TRUE_MIN 1e-45f
#endif

#ifndef DBL_TRUE_MIN
	#define DBL_TRUE_MIN 5e-324
#endif

#ifndef FLT_MAX
	#define FLT_MAX 3.4028235e+38f
#endif

#ifndef DBL_MAX
	#define DBL_MAX 1.7976931348623157e+308
#endif

#ifndef FLT_EPSILON
	#define FLT_EPSILON 1.1920929e-07f
#endif

#ifndef DBL_EPSILON
	#define DBL_EPSILON 2.220446049250313e-16
#endif

#ifndef FLT_DIG
	#define FLT_DIG 6
#endif

#ifndef DBL_DIG
	#define DBL_DIG 15
#endif

#ifndef FLT_MANT_DIG
	#define FLT_MANT_DIG 24
#endif

#ifndef DBL_MANT_DIG
	#define DBL_MANT_DIG 53
#endif

#ifndef FLT_MIN_EXP
	#define FLT_MIN_EXP -125
#endif

#ifndef DBL_MIN_EXP
	#define DBL_MIN_EXP -1021
#endif

#ifndef FLT_MIN_10_EXP
	#define FLT_MIN_10_EXP -37
#endif

#ifndef DBL_MIN_10_EXP
	#define DBL_MIN_10_EXP -307
#endif

#ifndef FLT_MAX_EXP
	#define FLT_MAX_EXP 128
#endif

#ifndef DBL_MAX_EXP
	#define DBL_MAX_EXP 1024
#endif

#ifndef FLT_MAX_10_EXP
	#define FLT_MAX_10_EXP 38
#endif

#ifndef DBL_MAX_10_EXP
	#define DBL_MAX_10_EXP 308
#endif

#ifndef FLT_EVAL_METHOD
	#define FLT_EVAL_METHOD 0
#endif

#ifndef FLT_HAS_SUBNORM
	#define FLT_HAS_SUBNORM 1
#endif

#ifndef DBL_HAS_SUBNORM
	#define DBL_HAS_SUBNORM 1
#endif

#ifndef DECIMAL_DIG
	#if defined(_ABIO64) || defined(_ABIO32)
		#define DECIMAL_DIG DBL_DECIMAL_DIG
	#else
		#define DECIMAL_DIG 36
	#endif
#endif
