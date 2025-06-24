#ifndef N64_STDLIB_N64_STRING_H_
#define N64_STDLIB_N64_STRING_H_

#include "n64-stddef.h"
#include "n64-util.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Extension. Works the same as [n64_]strncpy except that it writes only a
 * single null terminator after copying the string, rather than filling the
 * entire rest of the buffer with zeros. Additionally, it returns the number
 * of characters written not including the null terminator.
 */
__attribute__((access(write_only, 1),  nonnull(1)))
unsigned int n64_strncpy2( char *dest, const char *src, unsigned int count );


__attribute__((pure, warn_unused_result, nonnull(1)))
unsigned int n64_strlen( const char *str );

__attribute__((pure, warn_unused_result, nonnull(1, 2)))
int n64_strcmp( const char *lhs, const char *rhs );

__attribute__((pure, warn_unused_result, nonnull(1, 2)))
int n64_strncmp( const char *lhs, const char *rhs, unsigned int count );

__attribute__((pure, warn_unused_result, nonnull(1), returns_nonnull))
char *n64_strchr( const char *str, int ch );

__attribute__((pure, warn_unused_result, nonnull(1), returns_nonnull))
char *n64_strrchr( const char *str, int ch );

__attribute__((pure, warn_unused_result, nonnull(1, 2)))
unsigned int n64_strspn( const char *str, const char *accept );

__attribute__((pure, warn_unused_result, nonnull(1, 2)))
unsigned int n64_strcspn( const char *str, const char *reject );


__attribute__((pure, warn_unused_result, nonnull(1), returns_nonnull))
void *n64_memchr( const void *ptr, int ch, unsigned int count );

__attribute__((pure, warn_unused_result, nonnull(1, 2)))
int n64_memcmp( const void *lhs, const void *rhs, unsigned int count );

__attribute__((access(write_only, 1), nonnull(1), returns_nonnull))
void *n64_memset( void *dest, int ch, unsigned int count );

__attribute__((access(write_only, 1), nonnull(1, 2), returns_nonnull))
void *n64_memcpy( void *dest, const void *src, unsigned int count );

__attribute__((access(write_only, 1), nonnull(1, 2), returns_nonnull))
void *n64_memmove( void *dest, const void *src, unsigned int count );

__attribute__((access(write_only, 1), nonnull(1)))
static inline void n64_bzero( void *s, unsigned int n ) {
	n64_memset( s, 0, n );
}

__attribute__((nonnull(1), always_inline))
static inline void n64_explicit_bzero( void *s, unsigned int n ) {
	n64_bzero( s, n );
	n64_memory_barrier();
}

__attribute__((access(write_only, 2), nonnull(1, 2), always_inline))
static inline void n64_bcopy( const void *src, void *dest, unsigned int n ) {
	n64_memmove( dest, src, n );
}


__attribute__((pure, warn_unused_result))
static inline unsigned int n64_strlen_s( const char *str, unsigned int strsz ) {
	if( !str ) return 0u;
	const char *stop = (const char*)n64_memchr( str, 0, strsz );
	return stop ? (unsigned int)(stop - str) : strsz;
}

__attribute__((access(write_only, 1),  nonnull(1, 2), returns_nonnull))
static inline char *n64_strcpy( char *dest, const char *src ) {
	return (char*)n64_memcpy( dest, src, n64_strlen( src ) + 1u );
}

__attribute__((access(write_only, 1),  nonnull(1), returns_nonnull))
static inline char *n64_strncpy( char *dest, const char *src, unsigned int count ) {
	const unsigned int len = 1u + n64_strncpy2( dest, src, count );
	if( len < count ) n64_memset( &dest[len], 0, count - len );
	return dest;
}

__attribute__((access(read_write, 1),  nonnull(1, 2), returns_nonnull))
static inline char *n64_strcat( char *dest, const char *src ) {
	n64_strcpy( &dest[n64_strlen( dest )], src );
	return dest;
}

__attribute__((access(read_write, 1),  nonnull(1), returns_nonnull))
static inline char *n64_strncat( char *dest, const char *src, unsigned int count ) {
	unsigned int len = n64_strlen( dest );
	len += n64_strncpy2( &dest[len], src, count );
	dest[len] = '\0';
	return dest;
}

#ifdef __cplusplus
}
#endif

#endif
