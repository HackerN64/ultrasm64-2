#include "libpl2-init.h"
#include "libpl2-emu.h"
#include "libpl2-launcher.h"
#include "libpl2-rhdc.h"

/*! @mainpage
 * 
 * @section main_about About libpl2
 * libpl2 is a helper library to enable roms to integrate with ParallelN64 and Parallel Launcher, allowing for features such as
 * savestate detection, graphics plugin detection, and in-game romhacking.com integration.
 * 
 * This page documents the newer libpl2 library, which was redesigned to be easier to use. The older v1 libpl documentation can
 * be found <a href="https://parallel-launcher.gitlab.io/libpl/">here</a>
 * 
 * @section main_modules Pages
 * -# @ref page_init
 * -# @ref page_error_codes
 * -# @ref page_emu
 * -# @ref page_launcher
 * -# @ref page_rhdc
 * -# @ref page_version
 * -# @ref page_texture
 * 
 * @attention You must call @ref lpl2_init before calling any other libpl2 function, otherwise they will return errors.
 * 
 * @note libpl2 is console safe in the sense that calling libpl2 functions will not cause crashes or instability; however, since
 * libpl2 is an extension designed to interface with Parallel Launcher, all libpl2 functions will return false and set any passed
 * in error value to LPL2_ERR_LIBPL_NOT_SUPPORTED when used on console or an unsupported emulator.
 * 
 * @attention libpl2 functions are not thread safe. Normally, this will not matter since you will rarely want to call libpl
 * function from anything other than the main thread or boot code; however, if you want to use libpl2 on multiple concurrent
 * threads, you must disable interrupts before calling the libpl2 functions.
 * 
 */
