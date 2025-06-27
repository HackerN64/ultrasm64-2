#include "libpl2-init.h"

#ifdef LIBDRAGON
	#include <libdragon.h>
#else
	#include <ultra64.h>
#endif

#include "libpl2-internal.h"
#include "libpl2-launcher.h"
#include "libpl2-emu.h"

struct lpl_abi_requirements {
	lpl2_version coreVersion;
	lpl2_version launcherVersion;
};

static const struct lpl_abi_requirements sAbiReqs[] = {
	{ { 2, 13, 0 }, { 0, 0, 0 } },  // LIBPL_ABI_VERSION_2
	{ { 2, 13, 0 }, { 6, 20, 0 } }, // LIBPL_ABI_VERSION_3
	{ { 2, 14, 3 }, { 6, 22, 0 } }, // LIBPL_ABI_VERSION_4
	{ { 2, 15, 0 }, { 6, 22, 0 } }, // LIBPL_ABI_VERSION_5
	{ { 2, 16, 0 }, { 6, 22, 0 } }, // LIBPL_ABI_VERSION_6
	{ { 2, 16, 0 }, { 7, 10, 1 } }, // LIBPL_ABI_VERSION_7
};

lpl2_bool lpl2_init( libpl_abi_version minimumAbiVersion, lpl2_err *err ) {
	if( __libpl2_status == __LPL2_INITIALIZED && minimumAbiVersion <= __libpl2_abi ) {
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	}

	__libpl2_status = __LPL2_NOT_SUPPORTED;
	if( minimumAbiVersion < LIBPL_ABI_VERSION_1 || minimumAbiVersion > LIBPL_ABI_VERSION_CURRENT ) {
		if( err ) *err = LPL2_ERR_INVALID_ARGUMENTS;
		return FALSE;
	}

	__libpl2_abi = (int)minimumAbiVersion;
	if( minimumAbiVersion == LIBPL_ABI_VERSION_1 ) {
		__libpl2_status = __LPL2_INITIALIZED;
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	}
	
#ifdef LIBDRAGON
	io_write( 0x1ffb0000u, 0u );
	const unsigned int response = io_read( 0x1ffb0000u );
#else 
	unsigned int response = 0;
	osPiWriteIo( 0x1ffb0000u, 0u );
	osPiReadIo( 0x1ffb0000u, &response );
#endif
	if( response != 0x00500000u ) {
		if( err ) *err = LPL2_ERR_LIBPL_NOT_SUPPORTED;
		return FALSE;
	}

	lpl2_version version;
	__libpl2_status = __LPL2_INITIALIZED;

	if( !lpl2_get_core_version( &version, NULL ) || lpl2_compare_versions( &version, &sAbiReqs[(int)minimumAbiVersion - 2].coreVersion ) < 0 ) {
		__libpl2_status = __LPL2_NOT_SUPPORTED;
		if( err ) *err = LPL2_ERR_LIBPL_OLD_ABI;
		return FALSE;
	}

	if( minimumAbiVersion == LIBPL_ABI_VERSION_2 ) {
		if( err ) *err = LPL2_ERR_OKAY;
		return TRUE;
	}

	lpl2_err plErr;
	if( !lpl2_get_launcher_version( &version, &plErr ) || lpl2_compare_versions( &version, &sAbiReqs[(int)minimumAbiVersion - 2].launcherVersion ) < 0 ) {
		__libpl2_status = __LPL2_NOT_SUPPORTED;
		if( err ) *err = (plErr == LPL2_ERR_BROKEN_PIPE) ? LPL2_ERR_BROKEN_PIPE : LPL2_ERR_LIBPL_OLD_ABI;
		return FALSE;
	}

	if( err ) *err = LPL2_ERR_OKAY;
	return TRUE;
}
