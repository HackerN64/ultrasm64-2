#include "n64-stdlib.h"

#include "n64-stddef.h"
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
