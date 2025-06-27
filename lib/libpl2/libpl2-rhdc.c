#include "libpl2-rhdc.h"

#include "libpl2-internal.h"

unsigned short lpl2_get_my_rhdc_username( char *username, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, 0 )
	LPL2_VERIFY_ABI( 3, err, 0 )
	LPL2_VERIFY_NOTNULL( username, err, 0 )

	volatile const __lpl2_response *response = __lpl2_query_basic( 0x0200 );
	if( response->status == 0 && response->payloadSize <= 30 ) {
		if( err ) *err = LPL2_ERR_OKAY;
		const unsigned short len = __lpl2_strcpy( username, response->payload, 30 );
		username[len] = '\0';
		return len;
	} else if( err ) {
		switch( response->status ) {
			case 1: *err = LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED; break;
			case 0x300: case 0x400: *err = LPL2_ERR_BROKEN_PIPE; break;
			default: *err = LPL2_UNEXPECTED_ERROR( response->status ); break;
		}
	}

	return 0;
}

lpl2_bool lpl2_get_rhdc_avatar_async( const char *username, lpl2_avatar_options options, void *avatar, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_ABI( 7, err, FALSE )
	LPL2_VERIFY_NOTNULL( username, err, FALSE )
	LPL2_VERIFY_NOTNULL( avatar, err, FALSE )

	if( ((unsigned int)avatar & 7) != 0 ) {
		if( err ) *err = LPL2_ERR_MISALIGNED_POINTER_ARG;
		return FALSE;
	}

	avatar = __builtin_assume_aligned( avatar, 8 );

	volatile char *requestPayload = __lpl2_payload_start();
	*((volatile unsigned int*)requestPayload) = (unsigned int)options;
	const unsigned short usernameLen = __lpl2_strcpy( &requestPayload[4], username, 31 );
	if( usernameLen == 0 || usernameLen > 30 ) {
		if( err ) *err = LPL2_ERR_INVALID_ARGUMENTS;
		return FALSE;
	}

	volatile const __lpl2_response *response = __lpl2_payload_send( 0x0203, usernameLen + 4 );
	volatile const int *tempAvatar = (volatile const int*)__builtin_assume_aligned( (const void*)response->payload, 4 );
	if( response->status == 0 ) {
		if( err ) *err = LPL2_ERR_OKAY;
		for( unsigned short i = 0; i < (response->payloadSize >> 2); i++ ) {
			((unsigned int*)avatar)[i] = tempAvatar[i];
		}
		return TRUE;
	} else if( err ) {
		switch( response->status ) {
			case 0x500: *err = LPL2_ERR_WAIT; break;
			case 1: *err = LPL2_ERR_RHDC_AVATAR_NOT_FOUND; break;
			case 2: *err = LPL2_ERR_RHDC_NETWORK_ERROR; break;
			case 3: *err = LPL2_ERR_RHDC_AVATAR_INVALID; break;
			case 4: *err = LPL2_ERR_RHDC_RATE_LIMIT; break;
			case 0x200: case 0x201: case 0x600: *err = LPL2_ERR_INVALID_ARGUMENTS; break;
			case 0x300: case 0x400: *err = LPL2_ERR_BROKEN_PIPE; break;
			default: *err = LPL2_UNEXPECTED_ERROR( response->status ); break;
		}
	}

	return FALSE;
}
