#include "n64-stdlib.h"

#include "n64-stdckdint.h"
#include "n64-stdbool.h"
#include "n64-stddef.h"
#include "n64-ctype.h"
#include "n64-util.h"

typedef void(*memswp_func_t)( void*, void*, unsigned int );

#define __DECLARE_MEMSWP_FUNC(type) \
	static void memswp_##type( unsigned type *a, unsigned type *b, __attribute__((unused)) unsigned int n ) { \
		register unsigned type temp; \
		temp = *a; \
		*a = *b; \
		*b = temp; \
	} \
	\
	static void memswp_n##type( unsigned type *a, unsigned type *b, unsigned int n ) { \
		register unsigned type temp; \
		for( unsigned int i = 0; i < n; i++ ) { \
			temp = *a; \
			*(a++) = *b; \
			*(b++) = temp; \
		} \
	}

__DECLARE_MEMSWP_FUNC(char)
__DECLARE_MEMSWP_FUNC(short)
__DECLARE_MEMSWP_FUNC(int)

static void n64_qsort_impl(
	void *first,
	void *lo,
	void *hi,
	int(*comp)(const void*, const void*),
	void(*swap)(void*, void*, unsigned int),
	unsigned int sz,
	unsigned int csz
) {
	if( lo >= hi || lo < first ) return;

	void *i = lo;
	void *p = hi;
	for( void *j = lo; j < hi; j += sz ) {
		if( comp( j, p ) <= 0 ) {
			swap( i, j, csz );
			i += sz;
		}
	}

	if( i != hi ) swap( i, hi, csz );
	n64_qsort_impl( first, lo, i - sz, comp, swap, sz, csz );
	n64_qsort_impl( first, i + sz, hi, comp, swap, sz, csz );
}

void n64_qsort( void *ptr, unsigned int count, unsigned int size, int(*comp)(const void*, const void*) ) {
	unsigned int csize = size;

	memswp_func_t memswp;
	if( !size || !count ) {
		return;
	} else if( size == 1 ) {
		memswp = (memswp_func_t)memswp_char;
	} else if( size == 2 ) {
		memswp = ((unsigned int)ptr & 0x1) ? (memswp_func_t)memswp_nchar : (memswp_func_t)memswp_short;
	} else if( size == 4 ) {
		if( !((unsigned int)ptr & 0x3) ) {
			memswp = (memswp_func_t)memswp_int;
		} else if( !((unsigned int)ptr & 0x1) ) {
			memswp = (memswp_func_t)memswp_nshort;
			csize <<= 1;
		} else {
			memswp = (memswp_func_t)memswp_nchar;
		}
	} else if( !(size & 0x3) && !((unsigned int)ptr & 0x3) ) {
		memswp = (memswp_func_t)memswp_nint;
		csize <<= 2;
	} else if( !(size & 0x1) && !((unsigned int)ptr & 0x1) ) {
		memswp = (memswp_func_t)memswp_nshort;
		csize <<= 1;
	} else {
		memswp = (memswp_func_t)memswp_nchar;
	}

	n64_qsort_impl( ptr, ptr, ptr + (size * (count - 1)), comp, memswp, size, csize );
}

void *n64_bsearch( const void *key, const void *ptr, unsigned int count, unsigned int size, int(*comp)(const void*, const void*) ) {
	if( !size ) return NULL;
	while( count ) {
		register const int i = (count - 1) >> 1;
		register const void *p = ptr + size * i;
		register const int c = comp( p, key );

		if( c < 0 ) {
			ptr = p + size;
			count -= i + 1;
		} else if( c > 0 ) {
			count = i;
		} else {
			return (void*)p;
		}
	}

	return NULL;
}

int n64_atoi( const char *str ) {
	int x = 0;
	while( n64_isspace( *str ) ) str++;
	const char sign = *str;
	if( sign == '+' || sign == '-' ) str++;
	while( n64_isdigit( *str ) ) {
		x *= 10;
		x += (int)(*str++ - '0');
	}
	return (sign == '-') ? -x : x;
}

long long n64_atoll( const char *str ) {
	long long x = 0ll;
	while( n64_isspace( *str ) ) str++;
	const char sign = *str;
	if( sign == '+' || sign == '-' ) str++;
	while( n64_isdigit( *str ) ) {
		x *= 10ll;
		x += (long long)(*str++ - '0');
	}
	return (sign == '-') ? -x : x;
}

#define N64_STRTO_COMMON \
	n64_bool overflow = false;\
	n64_bool negative = false;\
\
	while( n64_isspace( *str ) ) str++;\
	if( *str == '+' ) {\
		str++;\
	} else if( *str == '-' ) {\
		negative = true;\
		str++;\
	}\
\
	if( *str == '0' ) {\
		str++;\
		if( *str == 'x' || *str == 'X' ) {\
			if( base == 0 ) {\
				base = 16;\
				str++;\
			} else if( base == 16 ) {\
				str++;\
			} else if( base < 34 ) {\
				if( str_end ) *str_end = (char*)str;\
				return 0;\
			}\
		} else if( base == 0 ) {\
			base = 8;\
		}\
	}\
\
	if( base == 0 ) {\
		base = 10;\
	}\
\
	while( true ) {\
		const char c = *str;\
\
		if( n64_isdigit( c ) ) {\
			digit = (int)(c - '0');\
		} else if( n64_islower( c ) ) {\
			digit = 10 + (int)(c - 'a');\
		} else if( n64_isupper( c ) ) {\
			digit = 10 + (int)(c - 'A');\
		} else break;\
\
		if( digit >= base ) break;\
\
		if( overflow ) {\
			str++;\
			continue;\
		}\
\
		if( n64_ckd_mul( &x, x, base ) ) {\
			overflow = true;\
			str++;\
			continue;\
		}\
\
		overflow = n64_ckd_add( &x, x, digit );\
		str++;\
	}

int n64_strtoi( const char *str, char **str_end, int base ) {
	if( base < 0 || base > 36 ) {
		if( str_end ) *str_end = (char*)str;
		return 0;
	}

	int x = 0;
	int digit = 0;

	N64_STRTO_COMMON

	if( str_end ) *str_end = (char*)str;
	if( overflow ) return negative ? -0x80000000 : 0x7fffffff;
	return negative ? -x : x;
}

__attribute__((always_inline))
static inline unsigned int n64_strtoui_impl( const char *str, char **str_end, unsigned int base ) {
	unsigned int x = 0;
	unsigned int digit = 0;

	N64_STRTO_COMMON

	if( str_end ) *str_end = (char*)str;
	if( overflow ) return 0x7fffffffu;
	return negative ? -x : x;
}

unsigned int n64_strtoui( const char *str, char **str_end, int base ) {
	if( base < 0 || base > 36 ) {
		if( str_end ) *str_end = (char*)str;
		return 0u;
	}

	return n64_strtoui_impl( str, str_end, (unsigned int)base );
}

long long n64_strtoll( const char *str, char **str_end, int base ) {
	if( base < 0 || base > 36 ) {
		if( str_end ) *str_end = (char*)str;
		return 0;
	}

	long long x = 0ll;
	int digit = 0;

	N64_STRTO_COMMON

	if( str_end ) *str_end = (char*)str;
	if( overflow ) return negative ? -0x8000000000000000ll : 0x7fffffffffffffffll;
	return negative ? -x : x;
}

__attribute__((always_inline))
static inline unsigned long long n64_strtoull_impl( const char *str, char **str_end, unsigned int base ) {
	unsigned long long x = 0;
	unsigned int digit = 0;

	N64_STRTO_COMMON

	if( str_end ) *str_end = (char*)str;
	if( overflow ) return 0x7fffffffffffffffu;
	return negative ? -x : x;
}

unsigned long long n64_strtoull( const char *str, char **str_end, int base ) {
	if( base < 0 || base > 36 ) {
		if( str_end ) *str_end = (char*)str;
		return 0u;
	}

	return n64_strtoull_impl( str, str_end, (unsigned int)base );
}

static unsigned int g_randi = 24u;
static unsigned int g_randv[32] = {
	0xdb48f936u, 0x14898454u, 0x37ffd106u, 0xb58bff9cu, 0x59e17104u, 0xcf918a49u, 0x09378c83u, 0x52c7a471u, 
	0x8d293ea9u, 0x1f4fc301u, 0xc3db71beu, 0x39b44e1cu, 0xf8a44ef9u, 0x4c8b80b1u, 0x19edc328u, 0x87bf4bddu, 
	0xc9b240e5u, 0xe9ee4b1bu, 0x4382aee7u, 0x535b6b41u, 0xf3bec5dau, 0x991539b1u, 0x16a5bce3u, 0x6774a4cdu, 
	0x73b5def3u, 0x3e01511eu, 0x4e508aaau, 0x61048c05u, 0xf5500617u, 0x846b7115u, 0x6a19892cu, 0x896a97afu
};

void n64_srand( unsigned int seed ) {
	if( !seed ) seed = 1u;

	g_randv[0] = seed;

	int r = (int)seed;
	for( int i = 1; i < 31; i++ ) {
		r = (int)(16807ll * (long long)(r % 127773) - 2836ll * (long long)(r / 127773));
		if( r < 0 ) r += 0x7FFFFFFF;
		g_randv[i] = (unsigned int)r;
	}

	g_randv[31] = seed;
	g_randv[0] = g_randv[1];
	g_randv[1] = g_randv[2];
	g_randi = 2u;

	for( int i = 0; i < 310; i++ ) {
		n64_rand();
	}
}

unsigned int n64_randu() {
	register const unsigned int next = (g_randi + 1u) & 0x1Fu;
	register const unsigned int r = g_randv[(g_randi + 29u) & 0x1Fu] + g_randv[next];
	g_randv[g_randi] = r;
	g_randi = next;
	return next;
}

float n64_randf() {
	const unsigned int bits = 0x3F800000u | (n64_randu() >> 9);
	return n64_bit_cast_itof( bits ) - 1.f;
}

double n64_randd() {
	const unsigned int bits[2] __attribute__((aligned(8))) = {
		0x3FF00000u | (n64_randu() >> 12),
		n64_randu()
	};
	return *((const double*)bits) - 1.0;
}
