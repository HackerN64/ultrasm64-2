#ifndef LIBPL2_EMU_
#define LIBPL2_EMU_

#include "libpl2-error.h"
#include "libpl2-version.h"
#include "libpl2-stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @defgroup page_emu Emulator Commands
 * 
 * The functions on this page are handled directly by the emulator core. They
 * will function even if the emulator core is extracted from Parallel Launcher
 * and put into a normal RetroArch installation.
 * 
 * @{
 */

/*! Graphics plugins */
typedef enum {
	LPL2_PLUGIN_PARALLEL =	1, /*!< ParaLLEl */
	LPL2_PLUGIN_GLIDEN64 =	2, /*!< GlideN64 */
	LPL2_PLUGIN_OGRE =		3, /*!< OGRE */
	LPL2_PLUGIN_GLIDE64 =	4, /*!< Glide64 */
	LPL2_PLUGIN_ANGRYLION =	5, /*!< Angrylion */
	LPL2_PLUGIN_RICE =		6, /*!< Rice */
	LPL2_GLN64 =			7, /*!< gln64 */
} lpl2_gfx_plugin;

/*! Graphics plugin capabilities */
typedef enum {
	/*! Indicates that the plugin is rendering at higher resolutions than an actual N64.
	 * @note The ParaLLEl plugin will have this bit set to 0 if its upscaling factor is set to 1
	 */
	LPL2_GFX_UPSCALING =				0x0001,

	/*! Indicates that framebuffer emulation is currently enabled on this plugin */
	LPL2_GFX_FRAMEBUFFER_EMULATION =	0x0002,

	/*! Indicates that the plugin is currently handling depth comparisons accurately (ie. decals will work properly) */
	LPL2_GFX_ACCURATE_DEPTH_COMPARE =	0x0004,

	/*! Indicates that accurate LLE RSP emulation is being used
	 * @since LIBPL_ABI_VERSION_5
	 */
	LPL2_GFX_RSP_EMULATION =			0x0008,

	/*! Indicates that the widescreen viewport hack is being used (GLideN64/OGRE)
	 * @since LIBPL_ABI_VERSION_6
	 */
	LPL2_WIDESCREEN_VIEWPORT =			0x0010,
} lpl2_gfx_capabilities;

/*! Cheat status flags */
typedef enum {
	LPL2_USED_CHEATS =        0x01, /*!< Gameshark codes have been used **/
	LPL2_USED_SAVESTATES =    0x02, /*!< A savestate has been loaded **/
	LPL2_USED_SLOWDOWN =      0x04, /*!< Emulator slowdown has been used **/
	LPL2_USED_FRAME_ADVANCE = 0x08, /*!< The emulation has been advanced in frame by frame mode **/
	LPL2_USED_SPEEDUP =       0x10, /*!< The emulation has been sped up @since LIBPL_ABI_VERSION_4 **/
	
	// convenience values for clearing flags
	LPL2_ALL_CHEAT_FLAGS = 0xff, /*!< A mask that covers all flags */
	LPL2_ALL_TAS_FLAGS = 0xfe, /*!< A mask that covers all flags except for @ref LPL2_USED_CHEATS */
	LPL2_NO_CHEAT_FLAGS = 0x00 /*!< An empty mask with no flags set */
} lpl2_cheat_flags;

/*! A structure containing information about the current graphics plugin and its settings */
typedef struct {
	lpl2_gfx_plugin pluginId;
	lpl2_gfx_capabilities capabilities;
	char name[10];
} lpl2_plugin_info;

/*! Gets the emulator core version
 * 
 * @param[out] version A pointer to an @ref lpl2_version struct where the ParallelN64 emulator core version will be stored
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE on success, FALSE on an error.
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS version is NULL
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * 
 * @see ::lpl2_get_launcher_version
 */
lpl2_bool lpl2_get_core_version( lpl2_version *version, lpl2_err *err ) __attribute__((access(write_only, 1), access(write_only, 2)));

/*! Check if any cheats or TAS tools have been used.
 * 
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return The current state of the cheat flags.
 * 
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * 
 * @since LPL_ABI_VERSION_2
 */
lpl2_cheat_flags lpl2_get_cheat_flags( lpl2_err *err ) __attribute__((access(write_only, 1), warn_unused_result));

/*! Clears the specified cheat flags, meaning that the next call to @ref lpl2_get_graphics_plugin will no longer report this
 * cheat as having been used. If the user is actively using cheats at the time this is called, the flags for the active cheats
 * will not be cleared.
 * 
 * @param[in] flags A bitmask of the cheat flags to clear
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return The new state of the cheat flags.
 * 
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 * @exception LPL2_ERR_LIBPL_OLD_ABI @ref lpl2_init with an older ABI that does not support this function
 * 
 * @since LPL_ABI_VERSION_2
 */
lpl2_cheat_flags lpl2_clear_cheat_flags( lpl2_cheat_flags flags, lpl2_err *err ) __attribute__((access(write_only, 2)));

/*! Gets information about the current graphics plugin
 * @param[out] plugin A pointer to a @ref lpl2_plugin_info struct where the plugin info will be stored
 * @param[out] err On an error, this value (if non-NULL) is set to an error code (see listed "exceptions" below for possible error
 * codes). On success, the value is set to LPL2_ERR_OKAY. If you do not care about the specific error, you may pass in NULL.
 * @return TRUE on success, FALSE on an error.
 * 
 * @exception LPL2_ERR_INVALID_ARGUMENTS plugin is NULL
 * @exception LPL2_ERR_LIBPL_NOT_INITIALIZED @ref lpl2_init has not been called
 * @exception LPL2_ERR_LIBPL_NOT_SUPPORTED the emulator does not support libpl
 */
lpl2_bool lpl2_get_graphics_plugin( lpl2_plugin_info *plugin, lpl2_err *err ) __attribute__((access(write_only, 1), access(write_only, 2)));

/*! @} */

#ifdef __cplusplus
}
#endif

#endif
