#ifndef LIBPL2_ERROR_
#define LIBPL2_ERROR_

#ifdef __cplusplus
extern "C" {
#endif

/*! @defgroup page_error_codes Error Codes
 *
 * @{
 */

/*! libpl2 error codes
 * Almost all libpl2 functions take in an optional err pointer. When a non-NULL pointer is passed in, its value will be set to one
 * of the documented error codes.
 */
typedef enum {
	LPL2_ERR_OKAY = 0x0000, /*!< No error */
	LPL2_ERR_WAIT = 0x0001, /*!< Waiting on async result (not an error) */

	LPL2_ERR_INVALID_ARGUMENTS = 0x0101, /*!< One or more arguments passed to the function are invalid */
	LPL2_ERR_MISALIGNED_POINTER_ARG = 0x0102, /*!< One or more pointer arguments passed to the function has incorrect alignment */
	LPL2_ERR_BROKEN_PIPE = 0x0103, /*!< The connection to Parallel Launcher has been lost */

	LPL2_ERR_LIBPL_NOT_INITIALIZED = 0x0301, /*!< @ref lpl2_init has not been called yet */
	LPL2_ERR_LIBPL_NOT_SUPPORTED = 0x0302, /*!< The emulator does not support libpl */
	LPL2_ERR_LIBPL_OLD_ABI = 0x0303, /*!< The function you are trying to use is from a newer ABI than the one you provied in your @ref lpl2_init call */

	LPL2_ERR_SD_CARD_ALREADY_LOADED = 0x0401, /*!< An SD card has already been loaded */
	LPL2_ERR_SD_CARD_ALREADY_EXISTS = 0x0402, /*!< An SD card with the given uid already exists */
	LPL2_ERR_SD_CARD_CORRUPTED = 0x0403, /*!< The emulator failed to load the SD card image */
	LPL2_ERR_SD_CARD_NOT_FOUND = 0x0404, /*!< An SD card with the given uid does not exist */
	LPL2_ERR_SD_CARD_CREATE_ERROR = 0x0405, /*!< An unknown error occurred creating the SD card image */

	LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED = 0x0501, /*!< The user has not enabled RHDC integration */
	LPL2_ERR_RHDC_NETWORK_ERROR = 0x0502, /*!< A network error occurred while calling RHDC */
	LPL2_ERR_RHDC_RATE_LIMIT = 0x0503, /*!< The emulator refused to run this command because you are making too many requests to RHDC too quickly */
	LPL2_ERR_RHDC_AVATAR_NOT_FOUND = 0x0504, /*!< The user either does not exist or does not have an avatar */
	LPL2_ERR_RHDC_AVATAR_INVALID = 0x0505, /*!< An image processing error occurred */
} lpl2_err;

/*! Returns a string containing the name of the enum value for the given error code. Useful for printing debug messages.
 *
 * @param error An error code set by libpl2 function
 * @return A pointer to a string in static memory containing the enum name (or LPL2_ERR_UNKNOWN for an unknown error code)
 */
const char *lpl2_get_error_enum_string( lpl2_err error ) __attribute__((pure, warn_unused_result, returns_nonnull));

/*! Returns a string containing a description of what the given error code means. Useful for printing debug messages.
 *
 * @param error An error code set by libpl2 function
 * @return A pointer to a string in static memory describing what the error code means
 */
const char *lpl2_get_error_description( lpl2_err error ) __attribute__((pure, warn_unused_result, returns_nonnull));

/*! @} */

#ifdef __cplusplus
}
#endif

#endif
