#ifndef N64_STDLIB_N64_STDBOOL_H_
#define N64_STDLIB_N64_STDBOOL_H_

#ifdef __cplusplus
	typedef bool n64_bool;
#else
	#if __STDC_VERSION__ >= 202311L
		typedef bool n64_bool;
	#else

		#if __STDC_VERSION__ >= 199901L
			typedef _Bool n64_bool;
		#elif __GNUC__ >= 15
			typedef unsigned char __attribute__((hardbool(0, 1))) n64_bool;
		#else
			typedef unsigned char n64_bool;
		#endif

		#ifndef true
			#define true 1
		#endif

		#ifndef false
			#define false 0
		#endif

	#endif
#endif

#ifndef __bool_true_false_are_defined
	#define __bool_true_false_are_defined 1
#endif

#endif
