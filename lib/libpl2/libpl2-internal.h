#ifndef LIBPL2_INTERNAL_
#define LIBPL2_INTERNAL_

#include "libpl2-error.h"
#include "libpl2-stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

extern enum __libpl2_status_t {
	__LPL2_NOT_INITIALIZED = 2,
	__LPL2_NOT_SUPPORTED = 1,
	__LPL2_INITIALIZED = 0
} __libpl2_status;

extern unsigned int __libpl2_abi;

#define LPL2_VERIFY_INITIALIZED( errPtr, returnValue ) \
	if( __libpl2_status != __LPL2_INITIALIZED ) { \
		if( errPtr ) *errPtr = (__libpl2_status == __LPL2_NOT_INITIALIZED) ? LPL2_ERR_LIBPL_NOT_INITIALIZED : LPL2_ERR_LIBPL_NOT_SUPPORTED; \
		return returnValue; \
	}

#define LPL2_VERIFY_ABI( minAbiVersion, errPtr, returnValue ) \
	if( __libpl2_abi < minAbiVersion ) { \
		if( errPtr ) *errPtr = LPL2_ERR_LIBPL_OLD_ABI; \
		return returnValue; \
	}

#define LPL2_VERIFY_NOTNULL( arg, errPtr, returnValue ) \
	if( arg == NULL ) { \
		if( errPtr ) *errPtr = LPL2_ERR_INVALID_ARGUMENTS; \
		return returnValue; \
	}

#define LPL2_UNEXPECTED_ERROR( status ) ((lpl2_err)((unsigned short)0x8000 | (unsigned short)status))

typedef struct {
	union {
		struct {
			unsigned char protocolStatus;
			unsigned char commandStatus;
		};
		unsigned short status;
	};
	unsigned short payloadSize;
	char payload[];
} __attribute__((aligned(8))) __lpl2_response;

// Copies a string up to but NOT INCLUDING the null terminator or until maxChars are copied
unsigned short __lpl2_strcpy( volatile char *dest, volatile const char *src, unsigned short maxChars ) __attribute__((nonnull(1,2)));

volatile const __lpl2_response *__lpl2_query_basic( unsigned short commandId ) __attribute__((returns_nonnull));
volatile const __lpl2_response *__lpl2_query_string( unsigned short commandId, const char *arg ) __attribute__((returns_nonnull, nonnull(2)));
volatile const __lpl2_response *__lpl2_query_byte( unsigned short commandId, unsigned char arg ) __attribute__((returns_nonnull));
volatile const __lpl2_response *__lpl2_query_uint( unsigned short commandId, unsigned int arg ) __attribute__((returns_nonnull));

inline __attribute__((always_inline, assume_aligned(4), const)) volatile char *__lpl2_payload_start() { return (volatile char*)0xbffb0004u; }
volatile const __lpl2_response *__lpl2_payload_send( unsigned short commandId, unsigned short payloadSize ) __attribute__((returns_nonnull));

#ifdef __cplusplus
}
#endif

#endif
