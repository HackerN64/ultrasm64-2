#include "libpl2-launcher.h"

#include "libpl2-internal.h"

typedef struct {
	unsigned int size;
	unsigned char format;
	char name[];
} sdcard_internal_t;

lpl2_bool lpl2_get_launcher_version( lpl2_version *version, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_ABI( 3, err, FALSE )
	LPL2_VERIFY_NOTNULL( version, err, FALSE )
	volatile const __lpl2_response *response = __lpl2_query_basic( 0x0100 );
	volatile const lpl2_version *payload = (volatile const lpl2_version*)response->payload;
	if( response->status == 0 ) {
		version->major = payload->major;
		version->minor = payload->minor;
		version->patch = payload->patch;
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	} else {
		version->major = version->minor = version->patch = 0;
		if( err ) {
			switch( response->protocolStatus ) {
				case 3: case 4: *err = LPL2_ERR_BROKEN_PIPE; break;
				default: *err = LPL2_UNEXPECTED_ERROR( response->status ); break;
			}
		}
		return FALSE;
	}
}

lpl2_bool lpl2_get_sd_card_info( const char *uid, lpl2_sd_card_info *info, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_ABI( 4, err, FALSE )
	LPL2_VERIFY_NOTNULL( uid, err, FALSE )
	LPL2_VERIFY_NOTNULL( info, err, FALSE )

	volatile const __lpl2_response *response = __lpl2_query_string( 0x0101, uid );
	volatile const sdcard_internal_t *payload = (volatile const sdcard_internal_t*)response->payload;
	if( response->status == 0 ) {
		info->size = payload->size;
		info->format = (lpl2_sd_format)payload->format;
		info->name[__lpl2_strcpy( info->name, payload->name, response->payloadSize - 5 )] = '\0';
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	} else if( err ) {
		switch( response->status ) {
			case 1: *err = LPL2_ERR_SD_CARD_CORRUPTED; break;
			case 4: *err = LPL2_ERR_SD_CARD_NOT_FOUND; break;
			case 0x200: case 0x600: *err = LPL2_ERR_INVALID_ARGUMENTS; break;
			case 0x300: case 0x400: *err = LPL2_ERR_BROKEN_PIPE; break;
			default: *err = LPL2_UNEXPECTED_ERROR( response->status ); break;
		}
	}

	return FALSE;
}

lpl2_bool lpl2_create_sd_card( const char *uid, const char *name, lpl2_sd_format format, unsigned char sizeMiB, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_ABI( 4, err, FALSE )
	LPL2_VERIFY_NOTNULL( uid, err, FALSE )
	LPL2_VERIFY_NOTNULL( name, err, FALSE )

	volatile char *requestPayload = __lpl2_payload_start();
	requestPayload[0] = (char)sizeMiB;
	requestPayload[1] = (char)format;

	const unsigned short nameLen = __lpl2_strcpy( &requestPayload[2], name, 12 );
	if( nameLen == 0 || nameLen > 11 ) {
		if( err ) *err = LPL2_ERR_INVALID_ARGUMENTS;
		return FALSE;
	}

	requestPayload[nameLen + 2] = '\0';
	const unsigned short uidLen = __lpl2_strcpy( &requestPayload[nameLen + 3], uid, 37 );
	if( uidLen == 0 || uidLen > 36 ) {
		if( err ) *err = LPL2_ERR_INVALID_ARGUMENTS;
		return FALSE;
	}

	requestPayload[nameLen + uidLen + 3] = '\0';
	volatile const __lpl2_response *response = __lpl2_payload_send( 0x0102, nameLen + uidLen + 4 );
	if( response->status == 0 ) {
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	} else if( err ) {
		switch( response->status ) {
			case 1: *err = LPL2_ERR_SD_CARD_ALREADY_LOADED; break;
			case 2: *err = LPL2_ERR_SD_CARD_CORRUPTED; break;
			case 3: *err = LPL2_ERR_SD_CARD_CREATE_ERROR; break;
			case 5: case 6: *err = LPL2_ERR_SD_CARD_ALREADY_EXISTS; break;
			case 0x200: case 0x600: *err = LPL2_ERR_INVALID_ARGUMENTS; break;
			case 0x300: case 0x400: *err = LPL2_ERR_BROKEN_PIPE; break;
			default: *err = LPL2_UNEXPECTED_ERROR( response->status ); break;
		}
	}

	return FALSE;
}

lpl2_bool lpl2_create_auto_sd_card( lpl2_sd_format format, unsigned char sizeMiB, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_ABI( 4, err, FALSE )

	volatile char *requestPayload = __lpl2_payload_start();
	requestPayload[0] = (char)sizeMiB;
	requestPayload[1] = (char)format;

	volatile const __lpl2_response *response = __lpl2_payload_send( 0x0103, 2 );
	if( response->status == 0 ) {
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	} else if( err ) {
		switch( response->status ) {
			case 1: *err = LPL2_ERR_SD_CARD_ALREADY_LOADED; break;
			case 2: *err = LPL2_ERR_SD_CARD_CORRUPTED; break;
			case 3: *err = LPL2_ERR_SD_CARD_CREATE_ERROR; break;
			case 0x600: *err = LPL2_ERR_INVALID_ARGUMENTS; break;
			case 0x300: case 0x400: *err = LPL2_ERR_BROKEN_PIPE; break;
			default: *err = LPL2_UNEXPECTED_ERROR( response->status ); break;
		}
	}

	return FALSE;
}

lpl2_bool lpl2_load_sd_card( const char *uid, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_ABI( 4, err, FALSE )
	LPL2_VERIFY_NOTNULL( uid, err, FALSE )

	volatile const __lpl2_response *response = __lpl2_query_string( 0x0104, uid );
	if( response->status == 0 ) {
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	} else if( err ) {
		switch( response->status ) {
			case 1: *err = LPL2_ERR_SD_CARD_ALREADY_LOADED; break;
			case 2: *err = LPL2_ERR_SD_CARD_CORRUPTED; break;
			case 4: *err = LPL2_ERR_SD_CARD_NOT_FOUND; break;
			case 0x200: case 0x600: *err = LPL2_ERR_INVALID_ARGUMENTS; break;
			case 0x300: case 0x400: *err = LPL2_ERR_BROKEN_PIPE; break;
			default: *err = LPL2_UNEXPECTED_ERROR( response->status ); break;
		}
	}

	return FALSE;
}
