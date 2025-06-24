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
