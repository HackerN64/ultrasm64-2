#include "libpl2-internal.h"

enum __libpl2_status_t __libpl2_status = __LPL2_INITIALIZED;
unsigned int __libpl2_abi = 0;

typedef struct {
    unsigned short commandId;
    unsigned short payloadSize;
    char payload[];
} __lpl2_request;

static volatile __lpl2_request *g_request = (volatile __lpl2_request*)0xbffb0000u;

unsigned short __lpl2_strcpy( volatile char *dest, volatile const char *src, unsigned short maxChars ) {
	unsigned short len = 0;
	while( *src != '\0' && len < maxChars ) {
		dest[len++] = *(src++);
	}
	return len;
}

volatile const __lpl2_response *__lpl2_query_basic( unsigned short commandId ) {
	g_request->payloadSize = 0;
	g_request->commandId = commandId;
	return (volatile __lpl2_response*)g_request;
}

volatile const __lpl2_response *__lpl2_query_string( unsigned short commandId, const char *arg ) {
	g_request->payloadSize = __lpl2_strcpy( (char*)g_request->payload, arg, 0xFFFC );
	g_request->commandId = commandId;
	return (volatile __lpl2_response*)g_request;
}

volatile const __lpl2_response *__lpl2_query_byte( unsigned short commandId, unsigned char arg ) {
	g_request->payloadSize = 1;
	g_request->payload[0] = (char)arg;
	g_request->commandId = commandId;
	return (volatile __lpl2_response*)g_request;
}

volatile const __lpl2_response *__lpl2_query_uint( unsigned short commandId, unsigned int arg ) {
	g_request->payloadSize = 4;
	*((volatile unsigned int*)g_request->payload) = arg;
	g_request->commandId = commandId;
	return (volatile __lpl2_response*)g_request;
}

volatile const __lpl2_response *__lpl2_payload_send( unsigned short commandId, unsigned short payloadSize ) {
	g_request->payloadSize = payloadSize;
	g_request->commandId = commandId;
	return (volatile __lpl2_response*)g_request;
}
