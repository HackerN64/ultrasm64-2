#ifndef LIBPL2_RHDC_
#define LIBPL2_RHDC_

#include "libpl2-stddef.h"
#include "libpl2-error.h"
#include "libpl2-texture.h"

/*! @defgroup page_rhdc Romhacking.com Commands
 * @{
 */

/*! Convenience struct for storing a 16x16 RHDC avatar in RGBA16 format. Alias of @ref lpl2_texture_16x16_rgba16 */
typedef lpl2_texture_16x16_rgba16 lpl2_avatar_16x16_rgba16;

/*! Convenience struct for storing a 32x32 RHDC avatar in RGBA16 format. Alias of @ref lpl2_texture_32x32_rgba16 */
typedef lpl2_texture_32x32_rgba16 lpl2_avatar_32x32_rgba16;

/*! Convenience struct for storing a 64x64 RHDC avatar in RGBA16 format. */
typedef struct { lpl2_texture_64x32_rgba16 textures[2]; } lpl2_avatar_64x64_rgba16 __attribute__((aligned(8)));

/*! Convenience struct for storing a 16x16 RHDC avatar in RGBA32 format. Alias of @ref lpl2_texture_16x16_rgba32 */
typedef lpl2_texture_16x16_rgba32 lpl2_avatar_16x16_rgba32;

/*! Convenience struct for storing a 32x32 RHDC avatar in RGBA32 format. Alias of @ref lpl2_texture_32x32_rgba32 */
typedef lpl2_texture_32x32_rgba32 lpl2_avatar_32x32_rgba32;

/*! Convenience struct for storing a 64x64 RHDC avatar in RGBA32 format. */
typedef struct { lpl2_texture_64x16_rgba32 textures[4]; } lpl2_avatar_64x64_rgba32 __attribute__((aligned(8)));

#ifdef __cplusplus
extern "C" {
#endif

/*! Image processing options for the avatar fetching calls */
typedef enum {
	LPL2_AVATAR_16x16 = 0x10000000, /*!< Size Flag. Return a 16x16 image */
	LPL2_AVATAR_32x32 = 0x20000000, /*!< Size Flag. Return a 32x32 image */
	LPL2_AVATAR_64x64 = 0x40000000, /*!< Size Flag. Return a 64x64 image */

	LPL2_AVATAR_RGBA16 = 0x100000, /*!< Format Flag. Return an image in RGBA16 (RGBA5551) format */
	LPL2_AVATAR_RGBA32 = 0x200000, /*!< Format Flag. Return an image in RGBA32 (RGBA8888) format */

	/*! Optional Flag.
	 * If this flag is provided, Floyd-Steinberg dithering will be used when converting the avatar to the RGBA16 format.
	 * This flag has no effect if the LPL2_AVATAR_RGBA32 flag is also provided.
	 */
	LPL2_AVATAR_DITHER = 0x0001,

	/*! Optional Flag.
	 * If this flag is provided, the avatar will be resized to the requested size without using any filtering.
	 * Otherwise, if this flag is not provided, the avatar will be scaled using bilinear filtering.
	 */
	LPL2_AVATAR_NO_FILTERING = 0x0002,

	/*! Optional Flag.
	 * If this flag is provided, any alpha transparency in the image will be removed and replaced with black.
	 */
	LPL2_AVATAR_NO_ALPHA = 0x0004,

	/*! Optional Flag.
	 * If this flag is provided, an avatar that is not square will be zoomed in and the sides cropped to fit in a square box.
	 * Otherwise, if this flag is not provided, the avatar will be given a border of transparent (or black if the
	 * LPL2_AVATAR_NO_ALPHA flag is provided) pixels on the sides.
	 */
	LPL2_AVATAR_ZOOM = 0x0008,
} lpl2_avatar_options;

/*! Gets the username of the current RHDC user
 * @note RHDC usernames are at most 30 characters long as may contain alphanumeric ASCII characters, underscores, and hyphens
 * 
 * @param[out] username A buffer at least 31 bytes large where the username will be stored
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return Returns the length of the username in bytes (not including the null terminator), or 0 on an error
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS username is NULL or not a valid RHDC username
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * @exception LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED the user has not enabled RHDC integration
 * 
 * @since LPL_ABI_VERSION_3
 */
unsigned short lpl2_get_my_rhdc_username( char *username, lpl2_err *err ) __attribute__((access(write_only, 1), access(write_only, 2)));

/*! Asynchronously fetches the avatar for the given username in RHDC as a square image and stores it at the provided memory
 * address. If the operation failed or the user is not found, the provided memory buffer is not modified.
 * 
 * @param[in] username A null-terminated string containing the username or userId
 * @param[in] options Flags for how the avatar should be processed. You must include a Size Flag and a Format Flag.
 * @param[out] avatar A buffer that the avatar will be written to. It is strongly recommended that you pass in a pointer to the
 * appropriate lpl2_avatar_* type, though you may also pass in an 8-byte-aligned pointer to an arbitrary buffer of the appropriate
 * size. The expected types and sizes are listed below:
 * - @ref lpl2_avatar_16x16_rgba16 (512 bytes) for LPL2_AVATAR_16x16 | LPL2_AVATAR_RGBA16
 * - @ref lpl2_avatar_16x16_rgba32 (1024 bytes) for LPL2_AVATAR_16x16 | LPL2_AVATAR_RGBA32
 * - @ref lpl2_avatar_32x32_rgba16 (2048 bytes) for LPL2_AVATAR_32x32 | LPL2_AVATAR_RGBA16
 * - @ref lpl2_avatar_32x32_rgba32 (4096 bytes) for LPL2_AVATAR_32x32 | LPL2_AVATAR_RGBA32
 * - @ref lpl2_avatar_64x64_rgba16 (8192 bytes) for LPL2_AVATAR_64x64 | LPL2_AVATAR_RGBA16 (contains 2 textures)
 * - @ref lpl2_avatar_64x64_rgba32 (16384 bytes) for LPL2_AVATAR_64x64 | LPL2_AVATAR_RGBA32 (contains 4 textures)
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL;
 * however, you likely want to at least check for an error code of LPL2_ERR_WAIT.
 * @return TRUE if the request has completed and the avatar has been copied. FALSE on an error or if the avatar has not been fully
 * downloaded yet.
 * 
 * @exception LPL2_ERR_WAIT the web request to RHDC has not completed yet
 * @exception LPL2_ERR_INVALID_ARGUMENTS username is NULL or not a valid RHDC username, avatar is NULL, or the options are invalid
 * @exception LPL2_ERR_MISALIGNED_POINTER_ARG avatar is not aligned to an 8-byte boundary
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * @exception LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED the user has not enabled RHDC integration
 * @exception LPL2_ERR_RHDC_NETWORK_ERROR could not connect to RHDC (can retry)
 * @exception LPL2_ERR_RHDC_RATE_LIMIT you are being rate limited (can retry later)
 * @exception LPL2_ERR_RHDC_AVATAR_NOT_FOUND the user does not exist or does not have an avatar
 * @exception LPL2_ERR_RHDC_AVATAR_INVALID an unexpected error occurred while processing the avatar image
 * 
 * @note This is an asynchronous request. When called for the first time, or if the result of a prior request with the same
 * arguments is not yet ready, the function will return FALSE and @p err will be set to LPL2_ERR_WAIT. Once the asynchronous
 * request completes successfully, all future requests with the same arguments will return TRUE and copy the avatar. It is safe to
 * make multiple concurrent asynchronous requests-- you do NOT need to wait for it to complete before making another libpl2 call.
 * 
 * @since LPL_ABI_VERSION_7
 */
lpl2_bool lpl2_get_rhdc_avatar_async( const char *username, lpl2_avatar_options options, void *avatar, lpl2_err *err ) __attribute__((access(write_only, 3), access(write_only, 4)));

/*! Fetches the avatar for the given username in RHDC as a square image and stores it at the provided memory address. If the
 * operation failed or the user is not found, the provided memory buffer is not modified. This operation blocks until the avatar
 * is fully downloaded or an error occurs.
 * 
 * @param[in] username A null-terminated string containing the username or userId
 * @param[in] options Flags for how the avatar should be processed. You must include a Size Flag and a Format Flag.
 * @param[out] avatar A buffer that the avatar will be written to. It is strongly recommended that you pass in a pointer to the
 * appropriate lpl2_avatar_* type, though you may also pass in an 8-byte-aligned pointer to an arbitrary buffer of the appropriate
 * size. The expected types and sizes are listed below:
 * - @ref lpl2_avatar_16x16_rgba16 (512 bytes) for LPL2_AVATAR_16x16 | LPL2_AVATAR_RGBA16
 * - @ref lpl2_avatar_16x16_rgba32 (1024 bytes) for LPL2_AVATAR_16x16 | LPL2_AVATAR_RGBA32
 * - @ref lpl2_avatar_32x32_rgba16 (2048 bytes) for LPL2_AVATAR_32x32 | LPL2_AVATAR_RGBA16
 * - @ref lpl2_avatar_32x32_rgba32 (4096 bytes) for LPL2_AVATAR_32x32 | LPL2_AVATAR_RGBA32
 * - @ref lpl2_avatar_64x64_rgba16 (8192 bytes) for LPL2_AVATAR_64x64 | LPL2_AVATAR_RGBA16 (contains 2 textures)
 * - @ref lpl2_avatar_64x64_rgba32 (16384 bytes) for LPL2_AVATAR_64x64 | LPL2_AVATAR_RGBA32 (contains 4 textures)
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE if the user's avatar was successfully downloaded and copied. FALSE on an error or if the user has no avatar.
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS username is NULL or not a valid RHDC username, avatar is NULL, or the options are invalid
 * @exception LPL2_ERR_MISALIGNED_POINTER_ARG avatar is not aligned to an 8-byte boundary
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * @exception LPL2_ERR_BROKEN_PIPE the connection to Parallel Launcher has been lost
 * @exception LPL2_ERR_RHDC_INTEGRATION_NOT_ENABLED the user has not enabled RHDC integration
 * @exception LPL2_ERR_RHDC_NETWORK_ERROR could not connect to RHDC (can retry)
 * @exception LPL2_ERR_RHDC_RATE_LIMIT you are being rate limited (can retry later)
 * @exception LPL2_ERR_RHDC_AVATAR_NOT_FOUND the user does not exist or does not have an avatar
 * @exception LPL2_ERR_RHDC_AVATAR_INVALID an unexpected error occurred while processing the avatar image
 * 
 * @attention This function will loop until the avatar is downloaded from RHDC, which may cause the emulator to stutter or freeze
 * during this time. For a better user expierence, use the @ref lpl2_get_rhdc_avatar_async function instead and handle the
 * LPL_ERR_WAIT error code.
 * 
 * @since LPL_ABI_VERSION_7
 */
static inline lpl2_bool __attribute__((warning("Use of blocking libpl call will likely cause the emulator to stutter. Use the async version of the libpl call for a better user experience."), access(write_only, 3), access(write_only, 4))) lpl2_get_rhdc_avatar_blocking( const char *username, lpl2_avatar_options options, void *avatar, lpl2_err *err ) {
	lpl2_err e = LPL2_ERR_WAIT;
	while( e == LPL2_ERR_WAIT ) lpl2_get_rhdc_avatar_async( username, options, avatar, &e );
	if( err ) *err = e;
	return e == LPL2_ERR_OKAY;
}

/*! @} */

#ifdef __cplusplus
}
#endif

#endif
