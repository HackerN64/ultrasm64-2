#ifndef LIBPL2_STDDEF_
#define LIBPL2_STDDEF_

#if __GNUC__ >= 14
typedef int __attribute__((hardbool)) lpl2_bool;
#else
typedef int lpl2_bool;
#endif

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#ifdef __cplusplus
		#if __cplusplus >= 199711L
			#define NULL nullptr
		#else
			#define NULL 0
		#endif
	#else
		#if __STDC_VERSION__ >= 202311L
			#define NULL nullptr
		#else
			#define NULL ((void*)0)
		#endif
	#endif
#endif

#ifdef __cplusplus
	#define lpl2_static_assert( expr ) static_assert( expr )
#else
	#if __STDC_VERSION__ >= 202311L
		#define lpl2_static_assert( expr ) static_assert( expr )
	#elif __STDC_VERSION__ >= 201112L
		#define lpl2_static_assert( expr ) _Static_assert( expr )
	#else
		#define lpl2_static_assert( expr )
	#endif
#endif

#endif
