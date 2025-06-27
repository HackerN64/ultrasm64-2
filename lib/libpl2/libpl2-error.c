#include "libpl2-error.h"

const char *lpl2_get_error_enum_string( lpl2_err error ) {
	switch( error ) {
		case LPL2_ERR_OKAY: return "LPL2_ERR_OKAY";
		case LPL2_ERR_WAIT: return "LPL2_ERR_WAIT";

		case LPL2_ERR_INVALID_ARGUMENTS: return "LPL2_ERR_INVALID_ARGUMENTS";
		case LPL2_ERR_MISALIGNED_POINTER_ARG: return "LPL2_ERR_MISALIGNED_POINTER_ARG";
		case LPL2_ERR_BROKEN_PIPE: return "LPL2_ERR_BROKEN_PIPE";

		case LPL2_ERR_LIBPL_NOT_INITIALIZED: return "LPL2_ERR_LIBPL_NOT_INITIALIZED";
		case LPL2_ERR_LIBPL_NOT_SUPPORTED: return "LPL2_ERR_LIBPL_NOT_SUPPORTED";
		case LPL2_ERR_LIBPL_OLD_ABI: return "LPL2_ERR_LIBPL_OLD_ABI";

		case LPL2_ERR_SD_CARD_ALREADY_LOADED: return "LPL2_ERR_SD_CARD_ALREADY_LOADED";
		case LPL2_ERR_SD_CARD_ALREADY_EXISTS: return "LPL2_ERR_SD_CARD_ALREADY_EXISTS";
		case LPL2_ERR_SD_CARD_CORRUPTED: return "LPL2_ERR_SD_CARD_CORRUPTED";
		case LPL2_ERR_SD_CARD_CREATE_ERROR: return "LPL2_ERR_SD_CARD_CREATE_ERROR";
		case LPL2_ERR_SD_CARD_NOT_FOUND: return "LPL2_ERR_SD_CARD_NOT_FOUND";

		case LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED: return "LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED";
		case LPL2_ERR_RHDC_NETWORK_ERROR: return "LPL2_ERR_RHDC_NETWORK_ERROR";
		case LPL2_ERR_RHDC_RATE_LIMIT: return "LPL2_ERR_RHDC_RATE_LIMIT";
		case LPL2_ERR_RHDC_AVATAR_NOT_FOUND: return "LPL2_ERR_RHDC_AVATAR_NOT_FOUND";
		case LPL2_ERR_RHDC_AVATAR_INVALID: return "LPL2_ERR_RHDC_AVATAR_INVALID";

		default: return "LPL2_ERR_UNKNOWN";
	}
}

const char *lpl2_get_error_description( lpl2_err error ) {
	switch( error ) {
		case LPL2_ERR_OKAY: return "No error";
		case LPL2_ERR_WAIT: return "Waiting on async result (not an error)";

		case LPL2_ERR_INVALID_ARGUMENTS: return "One or more arguments passed to the function are invalid";
		case LPL2_ERR_MISALIGNED_POINTER_ARG: return "One or more pointer arguments passed to the function has incorrect alignment";
		case LPL2_ERR_BROKEN_PIPE: return "The connection to Parallel Launcher has been lost";

		case LPL2_ERR_LIBPL_NOT_INITIALIZED: return "lpl2_init has not been called yet";
		case LPL2_ERR_LIBPL_NOT_SUPPORTED: return "The emulator does not support libpl";
		case LPL2_ERR_LIBPL_OLD_ABI: return "The function you are trying to use is from a newer ABI than the one you provied in your lpl2_init call";

		case LPL2_ERR_SD_CARD_ALREADY_LOADED: return "An SD card has already been loaded";
		case LPL2_ERR_SD_CARD_ALREADY_EXISTS: return "An SD card with the given uid already exists";
		case LPL2_ERR_SD_CARD_CORRUPTED: return "The emulator failed to load the SD card image";
		case LPL2_ERR_SD_CARD_CREATE_ERROR: return "An unknown error occurred creating the SD card image";
		case LPL2_ERR_SD_CARD_NOT_FOUND: return "An SD card with the given uid does not exist";

		case LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED: return "The user has not enabled RHDC integration";
		case LPL2_ERR_RHDC_NETWORK_ERROR: return "A network error occurred while calling RHDC";
		case LPL2_ERR_RHDC_RATE_LIMIT: return "The emulator refused to run this command because you are making too many requests to RHDC too quickly";
		case LPL2_ERR_RHDC_AVATAR_NOT_FOUND: return "The user either does not exist or does not have an avatar";
		case LPL2_ERR_RHDC_AVATAR_INVALID: return "An image processing error occurred";

		default: return "UNKNOWN: Error code does not match a known libpl2 error";
	}
}
