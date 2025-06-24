#include "n64-string.h"

#include "n64-stddef.h"
#include "n64-stdbool.h"

inline unsigned int __attribute__((const, always_inline)) hasZeroByte( unsigned int x ) {
	return (x - 0x01010101u) & ~x & 0x80808080u;
}

inline unsigned int __attribute__((const, always_inline)) bytepattern( int ch ) {
	register unsigned int w = (unsigned int)(unsigned char)ch;
	w |= w << 8;
	w |= w << 16;
	return w;
}

unsigned int n64_strncpy2( char *dest, const char *src, unsigned int count ) {
	register const char *const end = n64_memchr( src, 0, count );
	if( !end ) {
		n64_memcpy( dest, src, count );
		return count;
	} else {
		register const unsigned int len = (unsigned int)(end - src);
		n64_memcpy( dest, src, len + 1u );
		return len;
	}
}

unsigned int n64_strlen( const char *str ) {
	const char *const start = str;

	while( ((unsigned int)str & 3) ) {
		if( !*str ) return (unsigned int)(str - start);
		str++;
	}

	while( !hasZeroByte( *(unsigned int*)str ) ) str += 4;

	while( *str ) str++;
	return (unsigned int)(str - start);
}

int n64_strcmp( const char *lhs, const char *rhs ) {
	if( ((unsigned int)lhs & 3) == ((unsigned int)rhs & 3) ) {
		while( (unsigned int)lhs & 3 ) {
			register const int c = (int)(unsigned char)*lhs - (int)(unsigned char)*rhs;
			if( c ) return c;
			if( !*lhs ) return 0;
			lhs++;
			rhs++;
		}

		while( true ) {
			register const unsigned int x = *((const unsigned int*)lhs);
			if( hasZeroByte( x ) ) break;

			register const unsigned int y = *((const unsigned int*)rhs);
			if( x != y ) return (x < y) ? -1 : 1;

			lhs += 4;
			rhs += 4;
		}
	}

	while( true ) {
		register const int c = (int)(unsigned char)*lhs - (int)(unsigned char)*rhs;
		if( c ) return c;
		if( !*lhs ) return 0;
		lhs++;
		rhs++;
	}
}

int n64_strncmp( const char *lhs, const char *rhs, unsigned int count ) {
	if( ((unsigned int)lhs & 3) == ((unsigned int)rhs & 3) ) {
		while( count && ((unsigned int)lhs & 3) ) {
			register const int c = (int)(unsigned char)*lhs - (int)(unsigned char)*rhs;
			if( c ) return c;
			if( !*lhs ) return 0;
			lhs++;
			rhs++;
			count--;
		}

		while( count >= 4 ) {
			register const unsigned int x = *((const unsigned int*)lhs);
			if( hasZeroByte( x ) ) break;

			register const unsigned int y = *((const unsigned int*)rhs);
			if( x != y ) return (x < y) ? -1 : 1;

			lhs += 4;
			rhs += 4;
			count -= 4;
		}
	}
	
	while( count ) {
		register const int c = (int)(unsigned char)*lhs - (int)(unsigned char)*rhs;
		if( c ) return c;
		if( !*lhs ) return 0;
		lhs++;
		rhs++;
		count--;
	}

	return 0;
}

char *n64_strchr( const char *str, int ch ) {
	while( (unsigned int)str & 3 ) {
		if( (unsigned char)*str == (unsigned char)ch ) return (char*)str;
		if( !*(str++) ) return NULL;
	}

	register const unsigned int c = bytepattern( ch );
	while( !(hasZeroByte( *(const unsigned int*)str ) | hasZeroByte( (*(const unsigned int*)str) ^ c )) ) str += 4;

	while( (unsigned char)*str != (unsigned char)ch ) {
		if( !*(str++) ) return NULL;
	}

	return (char*)str;
}

char *n64_strrchr( const char *str, int ch ) {
	register const char *const start = str;
	str += n64_strlen( str );

	while( str != start && ((unsigned int)str & 3) != 3 ) {
		if( (unsigned char)*str == (unsigned char)ch ) return (char*)str;
		str--;
	}

	register const unsigned int c = bytepattern( ch );
	while( start <= str - 4 ) {
		if( hasZeroByte( *(const unsigned int*)(str - 3) ^ c ) ) break;
		str -= 4;
	}

	while( start != str ) {
		if( (unsigned char)*str == (unsigned char)ch ) return (char*)str;
		str--;
	}

	return NULL;
}

unsigned int n64_strspn( const char *str, const char *accept ) {
	if( !accept[0] ) return 0u;

	char mask[256];
	n64_memset( mask, 0, 256 );
	mask[0] = 1;

	while( *accept ) mask[(unsigned char)*(accept++)] = 1;

	const char *const start = str;
	while( !mask[(unsigned char)*str] ) str++;
	return (unsigned int)(str - start);
}

unsigned int n64_strcspn( const char *str, const char *reject ) {
	if( !reject[0] ) return n64_strlen( str );
	
	char mask[256];
	n64_memset( mask, 1, 256 );

	while( *reject ) mask[(unsigned char)*(reject++)] = 0;

	const char *const start = str;
	while( !mask[(unsigned char)*str] ) str++;
	return (unsigned int)(str - start);
}

void *n64_memchr( const void *ptr, int ch, unsigned int count ) {

	while( count && ((unsigned int)ptr & 3) ) {
		if( *(unsigned char*)ptr == (unsigned char)ch ) return (void*)ptr;
		ptr++;
		count--;
	}

	register const unsigned int cbp = bytepattern( ch );
	while( count >= 4u ) {
		if( hasZeroByte( *(unsigned int*)ptr ^ cbp ) ) break;
		ptr += 4;
		count -= 4;
	}

	while( count-- ) {
		if( *(unsigned char*)ptr == (unsigned char)ch ) return (void*)ptr;
		ptr++;
	}

	return NULL;
}

int n64_memcmp( const void *lhs, const void *rhs, unsigned int count ) {
	if( lhs == rhs ) return 0;

	if( count > 8u && ((unsigned int)lhs & 0x3) == ((unsigned int)rhs & 0x3) ) {
		if( (count & 3) || ((unsigned int)lhs & 0x3) ) {
			const void *const stop = lhs + count;
			const void *const astop = (const void*)((unsigned int)stop & 0xFFFFFFFCu);

			while( (unsigned int)lhs & 0x3 ) {
				register const unsigned char a = *(unsigned char*)lhs;
				register const unsigned char b = *(unsigned char*)rhs;

				if( a == b ) {
					lhs++;
					rhs++;
					continue;
				}

				return (a > b) ? 1 : -1;
			}

			while( lhs != astop ) {
				register const unsigned int a = *(unsigned int*)lhs;
				register const unsigned int b = *(unsigned int*)rhs;

				if( a == b ) {
					lhs += 4;
					rhs += 4;
					continue;
				}

				return (a > b) ? 1 : -1;
			}

			while( lhs != stop ) {
				register const unsigned char a = *(unsigned char*)lhs;
				register const unsigned char b = *(unsigned char*)rhs;

				if( a == b ) {
					lhs++;
					rhs++;
					continue;
				}

				return (a > b) ? 1 : -1;
			}

			return 0;
		} else {
			const void *const stop = lhs + count;
			while( lhs != stop ) {
				register const unsigned int a = *(unsigned int*)lhs;
				register const unsigned int b = *(unsigned int*)rhs;

				if( a == b ) {
					lhs += 4;
					rhs += 4;
					continue;
				}

				return (a > b) ? 1 : -1;
			}
			return 0;
		}
	} else {
		const void *const stop = lhs + count;
		while( lhs != stop ) {
			register const unsigned char a = *(unsigned char*)lhs;
			register const unsigned char b = *(unsigned char*)rhs;

			if( a == b ) {
				lhs++;
				rhs++;
				continue;
			}

			return (a > b) ? 1 : -1;
		}
		return 0;
	}
}

void *n64_memset( void *dest, int ch, unsigned int count ) {
	if( count > 8u && !((unsigned int)dest & 0x3) ) {
		register const unsigned int w = bytepattern( ch );
		unsigned int *head = (unsigned int*)dest;
		const unsigned int *const end = (unsigned int*)(dest + count);
		while( head != end ) *(head++) = w;
		return dest;
	} else {
		register const unsigned char c = (unsigned char)ch;
		unsigned char* head = (unsigned char*)dest;
		unsigned char *const end = head + count;
		while( head != end ) *(head++) = c;
		return dest;
	}
}

void *n64_memcpy( void *dest, const void *src, unsigned int count ) {
	if( count > 8u && ((unsigned int)dest & 0x3) == ((unsigned int)src & 0x3) ) {
		if( (count & 3) || ((unsigned int)src & 0x3) ) {
			register void *start = dest;
			register const void *end = (void*)((unsigned int)(start + count) & 0xFFFFFFFCu);

			while( (unsigned int)start & 0x3 ) {
				*((char*)start++) = *((char*)src++);
			}

			while( start != end ) {
				*((unsigned int*)start) = *((unsigned int*)src);
				start += 4;
				src += 4;
			}

			end = (void*)((unsigned int)dest + count);
			while( start != end ) {
				*((char*)start++) = *((char*)src++);
			}
		} else {
			register unsigned int *start = (unsigned int*)dest;
			register const unsigned int *const end = (const unsigned int*)((unsigned int)start + count);
			while( start != end ) {
				*(start++) = *(const unsigned int*)src;
				src += 4;
			}
		}

		return dest;
	} else {
		char *head = (char*)dest;
		char *const end = head + count;
		while( head != end ) *(head++) = *(char*)src++;
		return dest;
	}
}

void *n64_memmove( void *dest, const void *src, unsigned int count ) {
	if( src == dest || !count ) return dest;

	if( dest + count <= src || dest >= src + count ) {
		return n64_memcpy( dest, src, count );
	}

	if( dest > src ) {
		// start of dest overlaps end of src
		void *const end = dest;
		dest += count;
		src += count;
		while( dest != end ) *(char*)--dest = *(char*)--src;
		return end;
	} else {
		// end of dest overlaps start of src
		char *head = (char*)dest;
		char *const end = head + count;
		while( head != end ) *(head++) = *(char*)src++;
		return dest;
	}
}
