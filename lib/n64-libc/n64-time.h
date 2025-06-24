#ifndef N64_STDLIB_N64_TIME_H_
#define N64_STDLIB_N64_TIME_H_

#include "n64-stddef.h"
#include "n64-stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef long long n64_time_t;
typedef unsigned int n64_clock_t;

// Don't expect this to be accurate on emulator!
#define N64_CLOCKS_PER_SEC 46875000u

#ifndef CLOCKS_PER_SEC
	#define CLOCKS_PER_SEC N64_CLOCKS_PER_SEC
#endif

typedef struct {
	int tm_sec; // seconds after the minute [0-59]
	int tm_min; // minutes after the hour [0-59]
	int tm_hour; // hours since midnight [0-23]
	int tm_mday; // day of the month [1-31]
	int tm_mon; // months since January [0-11]
	int tm_year; // years since 1900 (remember the epoch is 1900, not AD/CE!)
	int tm_wday; // days since Sunday [0-6]
	int tm_yday; // days since January 1 [0-365]
	int tm_isdst; // Field included for the sake of matching the C STL. Has no effect.
} n64_tm;

/**********************************************************************************
* New functions made for this library that are not part of the C standard library *
**********************************************************************************/

/* !NOTICE!
 * Some flashcarts have buggy RTC implementations that do not correctly set status
 * registers. To handle these cases, after initializing the RTC, we need to wait a
 * short time (~20ms) before requesting the time to make sure the RTC has had time
 * to initialize, since we can't rely on the status registers to know when it's
 * ready.
 * 
 * Some helper functions are provided to help handle this case as laglessly as
 * possible. You are NOT required to call the init functions, and can simply use
 * the n64_rtc_read and n64_time functions without any init call; however, this
 * will result in a short stutter when playing on console as the library will wait
 * 20ms on the very first call to either n64_rtc_read or n64_time to ensure we
 * don't read garbage data from buggy flashcarts.
 * 
 * To avoid this stutter on console, you can call n64_rtc_init() at the start of
 * your program to initialize the RTC at this point. Then, the next call to
 * n64_rtc_read or n64_time will only wait if it has not yet been 20ms since
 * n64_rtc_init was called.
 * 
 * To determine whether 20ms has passed, a comparison is done to the COUNT
 * register, but since this overflows every ~90 seconds, if the very first call to
 * n64_rtc_read or n64_time occurs precisely at this time, it will incorrectly
 * believe that 20ms have not passed yet. To avoid this scenario, you can call
 * n64_rtc_ready( NULL ) periodically (such as at the start of each frame) to check
 * if the 20ms timer has finished, which will mark it as complete if it has, so you
 * will no longer need to worry about the clock counter overflowing.
 */

/* Initializes the RTC clock. Returns true if the RTC is supported, and false
 * otherwise. See the comment above for more information about what this call
 * is used for.
 */
n64_bool n64_rtc_init();

/* Returns true if it has been at least 20ms since n64_rtc_init() was called
 * or if it is being played on emulator.
 */
__attribute__((access(write_only, 1)))
n64_bool n64_rtc_ready( n64_bool *good );

/* New function that directly puts the current time in an n64_tm struct.
 *
 * Because the RTC already provides the time in a tm-like format, this is
 * much faster than using n64_time followed by n64_gmtime_r.
 *
 * Note that the n64_tm struct still matches the C standard library's tm struct--
 * of particular note is that tm_year is the number of years since 1900
 *
 * If the flashcart or emulator does not support RTC, returns false
 */
__attribute__((nonnull(1), access(write_only, 1)))
n64_bool n64_rtc_read( n64_tm *time );

/* Adds `seconds` to `time` and normalizes it. Returns back the `time` pointer.
 * If this would cause the time to overflow, it is capped at its min/max value.
 */
__attribute__((nonnull(1), access(read_write, 1), returns_nonnull))
n64_tm *n64_tm_add( n64_tm *time, n64_time_t seconds );

/* Like n64_mktime, but doesn't normalize the input time */
__attribute__((pure, nonnull(1), warn_unused_result))
n64_time_t n64_to_unix_time( const n64_tm *time );

/**********************************************************************************
* C standard library functions                                                    *
**********************************************************************************/

__attribute__((always_inline, warn_unused_result))
static inline n64_clock_t n64_clock() {
	n64_clock_t count;
	asm volatile( "mfc0 %0, $9": "=r"( count ) );
	return count;
}

__attribute__((access(write_only, 1)))
n64_time_t n64_time( n64_time_t *arg );

// Assumes UTC timezone
__attribute__((nonnull(1), access(read_write, 1)))
n64_time_t n64_mktime( n64_tm *time );

__attribute__((nonnull(1, 2), access(write_only, 2)))
n64_tm *n64_gmtime_r( const n64_time_t *timer, n64_tm *buf );

// Assumes UTC timezone and C locale
__attribute__((format(strftime, 3, 0), nonnull(1, 3), access(write_only, 1)))
unsigned int n64_strftime( char *str, unsigned int count, const char *format, const n64_tm *tp );

#ifdef __cplusplus
}
#endif

#endif
