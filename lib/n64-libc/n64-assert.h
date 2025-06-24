#ifndef N64_STDLIB_N64_ASSERT_H_
#define N64_STDLIB_N64_ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#define n64_assert( expr ) (void)(0)
#else

__attribute__((noreturn))
void __n64_assert_fail( const char *assertion, const char *file, unsigned int line, const char *fcn );

#define n64_assert( expr ) ((expr) ? (void)(0) : __n64_assert_fail( #expr, __FILE__, __LINE__, __func__ ))
#endif

#ifdef __cplusplus
	#define n64_static_assert( expr ) static_assert( expr )
#else
	#if __STDC_VERSION__ >= 202311L
		#define n64_static_assert( expr ) static_assert( expr )
	#else
		#define n64_static_assert( expr ) _Static_assert( expr )
	#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
