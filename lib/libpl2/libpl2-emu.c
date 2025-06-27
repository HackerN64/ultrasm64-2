#include "libpl2-emu.h"

#include "libpl2-internal.h"

typedef struct {
	unsigned short pluginId;
	unsigned short capabilities;
	char name[];
} plugin_internal_t;

lpl2_bool lpl2_get_core_version( lpl2_version *version, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_NOTNULL( version, err, FALSE )
	volatile const __lpl2_response *response = __lpl2_query_basic( 0x0001 );
	volatile const lpl2_version *payload = (volatile const lpl2_version*)response->payload;
	if( response->protocolStatus == 0 ) {
		version->major = payload->major;
		version->minor = payload->minor;
		version->patch = payload->patch;
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	} else {
		version->major = version->minor = version->patch = 0;
		if( err ) *err = LPL2_UNEXPECTED_ERROR( response->status );
		return FALSE;
	}
}

lpl2_cheat_flags lpl2_get_cheat_flags( lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, LPL2_NO_CHEAT_FLAGS )
	LPL2_VERIFY_ABI( 2, err, LPL2_NO_CHEAT_FLAGS )
	volatile const __lpl2_response *response = __lpl2_query_basic( 0x0005 );
	if( response->protocolStatus != 0 ) {
		if( err ) *err = LPL2_UNEXPECTED_ERROR( response->status );
		return LPL2_NO_CHEAT_FLAGS;
	}

	if( err ) *err = LPL2_ERR_OKAY;
	return (lpl2_cheat_flags)response->commandStatus;
}

lpl2_cheat_flags lpl2_clear_cheat_flags( lpl2_cheat_flags flags, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, LPL2_NO_CHEAT_FLAGS )
	LPL2_VERIFY_ABI( 2, err, LPL2_NO_CHEAT_FLAGS )
	volatile const __lpl2_response *response = __lpl2_query_byte( 0x0006, (unsigned char)flags );
	if( response->protocolStatus != 0 ) {
		if( err ) *err = LPL2_UNEXPECTED_ERROR( response->status );
		return LPL2_NO_CHEAT_FLAGS;
	}

	if( err ) *err = LPL2_ERR_OKAY;
	return (lpl2_cheat_flags)response->commandStatus;
}

lpl2_bool lpl2_get_graphics_plugin( lpl2_plugin_info *plugin, lpl2_err *err ) {
	LPL2_VERIFY_INITIALIZED( err, FALSE )
	LPL2_VERIFY_NOTNULL( plugin, err, FALSE )
	volatile const __lpl2_response *response = __lpl2_query_basic( 0x0004 );
	volatile const plugin_internal_t *payload = (volatile const plugin_internal_t*)response->payload;
	if( response->status == 0 ) {
		plugin->pluginId = (lpl2_gfx_plugin)payload->pluginId;
		plugin->capabilities = (lpl2_gfx_capabilities)payload->capabilities;
		plugin->name[__lpl2_strcpy( plugin->name, payload->name, 9 )] = '\0';
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	} else {
		if( err ) *err = LPL2_UNEXPECTED_ERROR( response->status );
		return FALSE;
	}
}
