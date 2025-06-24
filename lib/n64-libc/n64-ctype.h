#ifndef N64_STDLIB_N64_CTYPE_H_
#define N64_STDLIB_N64_CTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned short __n64_internal_charflag_table[256];

__attribute__((const, always_inline))
static inline int __n64_internal_has_charflag( int ch, unsigned short flag ) {
	return __n64_internal_charflag_table[(unsigned int)ch & 0xFFu] & (int)flag;
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isalnum( int ch ) {
	return __n64_internal_has_charflag( ch, 0x003 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isalpha( int ch ) {
	return __n64_internal_has_charflag( ch, 0x002 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_islower( int ch ) {
	return __n64_internal_has_charflag( ch, 0x008 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isupper( int ch ) {
	return __n64_internal_has_charflag( ch, 0x010 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isdigit( int ch ) {
	return __n64_internal_has_charflag( ch, 0x001 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isxdigit( int ch ) {
	return __n64_internal_has_charflag( ch, 0x004 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_iscntrl( int ch ) {
	return __n64_internal_has_charflag( ch, 0x020 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isgraph( int ch ) {
	return __n64_internal_has_charflag( ch, 0x203 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isspace( int ch ) {
	return __n64_internal_has_charflag( ch, 0x040 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isblank( int ch ) {
	return __n64_internal_has_charflag( ch, 0x080 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_isprint( int ch ) {
	return __n64_internal_has_charflag( ch, 0x100 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_ispunct( int ch ) {
	return __n64_internal_has_charflag( ch, 0x200 );
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_tolower( int ch ) {
	return (ch >= (int)'A' && ch <= (int)'Z') ? (ch + 32) : ch;
}

__attribute__((const, warn_unused_result, always_inline))
static inline int n64_toupper( int ch ) {
	return (ch >= (int)'a' && ch <= (int)'z') ? (ch - 32) : ch;
}

#ifdef __cplusplus
}
#endif

#endif
