#include "n64-stdio.h"

#include "n64-util.h"
#include "n64-stdio-bprint.c.inc"

static unsigned int s_isv_buffer[712/4];

static n64_bool s_sprint_callback( void *buffer, unsigned int len ) {
	((char*)buffer)[len] = '\0';
	return false;
}

static n64_bool s_sprint_nobuffer_callback( void*, unsigned int ) {
	return false;
}

static n64_bool s_isv_bprint_callback( void *headPtr, unsigned int chunkSize ) {
	const unsigned int n = (chunkSize + 3u) >> 2;
	volatile unsigned int *head = *(volatile unsigned int**)headPtr;
	
	for( unsigned int i = 0; i < n; i++ ) {
		*(head++) = s_isv_buffer[i];
		n64_await_pi_io();
	}

	*(volatile unsigned int**)headPtr = head;
	return head != NULL && head < (volatile unsigned int*)0xb4000000u;
}

int n64_vsnprintf( char *buffer, unsigned int bufsz, const char *format, __builtin_va_list args ) {
	return bufsz ?
		n64_vbprintf( buffer, bufsz - 1, s_sprint_callback, buffer, format, args ) :
		n64_vbprintf( buffer, 0u, s_sprint_nobuffer_callback, NULL, format, args );
}

int n64_vprintf( const char *format, __builtin_va_list args ) {
	const n64_bool inter = n64_set_interrupts( false );
	n64_await_pi_io();

	volatile unsigned int *head = (volatile unsigned int*)0xb3ff0020u;
	register const int len = n64_vbprintf( (char*)s_isv_buffer, 712u, s_isv_bprint_callback, &head, format, args );
	if( len >= 0 ) {
		n64_memory_barrier();
		*((volatile int*)0xb3ff0014u) = (len > 0xFFE0) ? 0xFFE0 : len;
		n64_memory_barrier();
		n64_await_pi_io();
	}

	n64_set_interrupts( inter );
	return len;
}

int n64_snprintf( char *buffer, unsigned int bufsz, const char *format, ... ) {
	__builtin_va_list args;
	__builtin_va_start( args, format );
	register const int result = n64_vsnprintf( buffer, bufsz, format, args );
	__builtin_va_end( args );
	return result;
}

int n64_printf( const char *format, ... ) {
	__builtin_va_list args;
	__builtin_va_start( args, format );
	register const int result = n64_vprintf( format, args );
	__builtin_va_end( args );
	return result;
}
