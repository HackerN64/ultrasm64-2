#ifndef LIBPL2_LAUNCHER_
#define LIBPL2_LAUNCHER_

#include "libpl2-error.h"
#include "libpl2-version.h"
#include "libpl2-stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @defgroup page_launcher Parallel Launcher Commands
 * @{
 */

/*! SD card formats */
typedef enum __attribute__((packed)) {
	LPL2_SD_UNFORMATTED = 0, /*!< Unformatted */
	LPL2_SD_FAT12 = 12, /*!< FAT-12 */
	LPL2_SD_FAT16 = 16, /*!< FAT-16 */
	LPL2_SD_FAT32 = 32, /*!< FAT-32 */
} lpl2_sd_format;

/*! SD card info */
typedef struct {
	unsigned int size; /*!< The size of the SD card in bytes */
	lpl2_sd_format format; /*!< The filesystem format of the SD card */
	char name[12]; /*< The name of the SD card */
} lpl2_sd_card_info;

/*! Gets the emulator core version
 * 
 * @param[out] version A pointer to an @ref lpl2_version struct where the Parallel Launcher version will be stored
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE on success, FALSE on an error.
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS version is NULL
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * 
 * @see ::lpl2_get_core_version
 */
lpl2_bool lpl2_get_launcher_version( lpl2_version *version, lpl2_err *err ) __attribute__((access(write_only, 1), access(write_only, 2)));

/*! Fetches info about an SD card created with @ref lpl2_create_sd_card. This can be used to check if a shared SD card exists and
 * has the expected size and format before loading it.
 * 
 * @param[in] uid A unique string identifying the shared SD card (max 36 characters)
 * @param[out] info A pointer to a struct where the SD card information will be stored
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE on success, FALSE on an error.
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS info or uid is NULL or uid is empty or too long
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * @exception LPL2_ERR_SD_CARD_NOT_FOUND no SD card with the given uid was found
 * @exception LPL2_ERR_SD_CARD_CORRUPTED an SD card with the uid exists, but the file could not be accessed
 * 
 * @since LPL_ABI_VERSION_4
 */
lpl2_bool lpl2_get_sd_card_info( const char *uid, lpl2_sd_card_info *info, lpl2_err *err ) __attribute__((access(write_only, 2), access(write_only, 3)));

/*! Creates a new SD card and loads it. This function can only be used to create an SD card when the user has not already selected
 * one in Parallel Launcher. If the user has already loaded an SD card, this function will do nothing and return FALSE.
 * 
 * Unlike @ref lpl2_create_auto_sd_card, this function requires you to provide a unique identifier for the SD card. Any other
 * ROM can request this card by passing the same uid to @ref lpl2_load_sd_card. This allows collaborating romhacks to share a 
 * common SD card.
 * 
 * @param[in] uid A unique string that the shared SD card can be identified with (max 36 characters)
 * @param[in] name The filename of the SD card and the volume name for FAT formatted cards (max 11 characters)
 * @param[in] format The format of the SD card
 * @param[in] sizeMiB The desired size of the SD card in MiB (2^20 bytes). \n
 * For FAT12, sizeMiB must be between 1 and 32 \n
 * For FAT16 and FAT32, sizeMiB must be between 32 and 255
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE if an SD card was created and loaded; FALSE otherwise
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS info or uid is NULL, empty, or too long, format is invalid, or sizeMiB is out of range
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * @exception LPL2_ERR_SD_CARD_ALREADY_LOADED an SD card has already been loaded
 * @exception LPL2_ERR_SD_CARD_ALREADY_EXISTS an SD card with this uid or name already exists
 * @exception LPL2_ERR_SD_CARD_CREATE_ERROR an unknown error occurred when attempting to create the SD card image
 * @exception LPL2_ERR_SD_CARD_CORRUPTED the SD card was created, but then somehow could not be accessed
 * 
 * @since LPL_ABI_VERSION_4
 */
lpl2_bool lpl2_create_sd_card( const char *uid, const char *name, lpl2_sd_format format, unsigned char sizeMiB, lpl2_err *err ) __attribute__((access(write_only, 5)));

/*! Creates a new SD card and loads it. This function can only be used to create an SD card when the user has not already selected
 * one in Parallel Launcher. If the user has already loaded an SD card, this function will do nothing and return FALSE.
 * 
 * @param[in] format The format of the SD card
 * @param[in] sizeMiB The desired size of the SD card in MiB (2^20 bytes) \n
 * For FAT12, sizeMiB must be between 1 and 32 \n
 * For FAT16 and FAT32, sizeMiB must be between 32 and 255
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE if an SD card was created and loaded; FALSE otherwise
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS format is invalid or sizeMiB is out of range
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * @exception LPL2_ERR_SD_CARD_ALREADY_LOADED an SD card has already been loaded
 * @exception LPL2_ERR_SD_CARD_CREATE_ERROR an unknown error occurred when attempting to create the SD card image
 * @exception LPL2_ERR_SD_CARD_CORRUPTED the SD card was created, but then somehow could not be accessed
 * 
 * @since LPL_ABI_VERSION_4
 */
lpl2_bool lpl2_create_auto_sd_card( lpl2_sd_format format, unsigned char sizeMiB, lpl2_err *err ) __attribute__((access(write_only, 3)));

/*! Loads an SD card that was created with @ref lpl2_create_sd_card. This function can only be used to load an SD card when the
 * the user has not already selected one in Parallel Launcher. If the user has already loaded an SD card, this function will do
 * nothing and return FALSE.
 * 
 * @param[in] uid A unique string identifying the shared SD card (max 36 characters)
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE if an SD card was loaded; FALSE otherwise
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS uid is NULL, empty, or too long
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * @exception LPL2_ERR_SD_CARD_NOT_FOUND no SD card with the given uid was found
 * @exception LPL2_ERR_SD_CARD_ALREADY_LOADED an SD card has already been loaded
 * @exception LPL2_ERR_SD_CARD_CORRUPTED the SD card exists, but could not be read
 * 
 * @note It would be wise to call @ref lpl2_get_sd_card_info first, to verify that the SD card has the expected size and format
 * 
 * @since LPL_ABI_VERSION_4
 */
lpl2_bool lpl2_load_sd_card( const char *uid, lpl2_err *err ) __attribute__((access(write_only, 2)));

/*! @} */

#ifdef __cplusplus
}
#endif

#endif
