#ifndef LIBPL2_INIT_
#define LIBPL2_INIT_

#include "libpl2-stddef.h"
#include "libpl2-error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @defgroup page_init Initialization
 *
 * @{
 */

/*! libpl ABI versions */
typedef enum {
	LIBPL_ABI_VERSION_1 = 1, /*!< initial libpl prototype (2023-07-17) */
	LIBPL_ABI_VERSION_2 = 2, /*!< revised prototype (2023-08-1) */
	LIBPL_ABI_VERSION_3 = 3, /*!< first advertised public release (2023-08-1) */
	LIBPL_ABI_VERSION_4 = 4, /*!< SD card update (2023-10-03) */
	LIBPL_ABI_VERSION_5 = 5, /*!< LLE RSP check (2024-02-16) */
	LIBPL_ABI_VERSION_6 = 6, /*!< Widescreen viewport check (2024-03-05) */
	LIBPL_ABI_VERSION_7 = 7, /*!< Variable size RHDC avatar fetching (2025-01-04) */
	LIBPL_ABI_VERSION_CURRENT = LIBPL_ABI_VERSION_7 /*!< the latest version */
} libpl_abi_version;

/*! Initializes libpl and checks if it is supported by the emulator.
 * This function should be called before any other libpl function.
 * 
 * @param[in] minimumAbiVersion The required libpl ABI version
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE if the emulator supports the requested version of libpl or later
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS minimumAbiVersion is invalid
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI the emulator does not support the requested ABI version. You may try calling lpl2_init again with a lower ABI version.
 * @exception LPL2_ERR_BROKEN_PIPE the emulator core appears to support libpl, however, it failed to establish a connection with Parallel Launcher
 */
lpl2_bool lpl2_init( libpl_abi_version minimumAbiVersion, lpl2_err *err ) __attribute__((access(write_only, 2)));

/*! @} */

#ifdef __cplusplus
}
#endif

#endif
