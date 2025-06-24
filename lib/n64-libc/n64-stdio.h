#ifndef N64_STDLIB_N64_STDIO_H_
#define N64_STDLIB_N64_STDIO_H_

#include "n64-stddef.h"
#include "n64-stdbool.h"

#ifndef EOF
#define EOF -1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* IMPORTANT NOTICE
 * The printf family of functions provided have some important deviations from the C standard library:
 * 1. The floating point formats (%f, %e, %g, and %a) expect a (32-bit) float instead of a double. To format a double, use the 'l'
 *    length modifier (ie. %lf, %le, %lg, or %la) to indicate that the argument is double precision. You can still pass in a
 *    double argument, however it will be cast to a float if the 'l', 'll', or 'L' length specifier is not present.
 * 2. Formatting 128-bit long doubles is not supported. The 'L' length specifier is treated the same as 'll'
 * 3. The 'l' length modifier is ignored for characters and strings (wide chars are not supported)
 */


/* Extension: n64_bprintf / n64_vbprintf
 * The n64_bprintf (buffered printf) is an extension intended to be used as a helper function for implementing your own printf
 * function to do things like printing to something other than IS Viewer or drawing formatted text on screen.
 * 
 * bprintf functions like snprintf, but when the provided buffer is filled up, it invokes the provided callback function, then,
 * depending on the return value, either moves back to the start of the buffer and continues writing, or ceases output. Once all
 * characters have been processed, the callback function is invoked one final time.
 * 
 * When the provided buffer is filled, the callback function is invoked, passing `state` to the first argument, and the buffer
 * size to the second argument. If you return false, the function will no longer write any data and will not invoke the callback
 * again, but it will still continue processing the text so that it can return the character count. If you return true, the
 * function will continue, moving back to the start buffer and continuing writing output. Every time the buffer is filled, the
 * callback is invoked again in the same way. Finally, once all characters have been processed and output is complete, the
 * callback will be invoked one final time, with the number of characters written since the last callback (which will be 0 if the
 * output size is a multiple of the buffer size) passed into the second argument of the callback function. Note that this final
 * callback is not invoked if a previous callback returned false.
 */
__attribute__((format(printf,5,0), nonnull(3, 5), access(write_only, 1)))
int n64_vbprintf( char *buffer, unsigned int bufsz, n64_bool(*callback)(void*, unsigned int), void *state, const char *format, __builtin_va_list args );

__attribute__((format(printf,3,0), nonnull(3), access(write_only, 1)))
int n64_vsnprintf( char *buffer, unsigned int bufsz, const char *format, __builtin_va_list args );

__attribute__((format(printf,1,0), nonnull(1)))
int n64_vprintf( const char *format, __builtin_va_list args );


__attribute__((format(printf,5,6), nonnull(3, 5), access(write_only, 1)))
int n64_bprintf( char *buffer, unsigned int bufsz, n64_bool(*callback)(void*, unsigned int), void *state, const char *format, ... );

__attribute__((format(printf,3,4), nonnull(3), access(write_only, 1)))
int n64_snprintf( char *buffer, unsigned int bufsz, const char *format, ... );

__attribute__((format(printf,1,2), nonnull(1)))
int n64_printf( const char *format, ... );

#ifdef __cplusplus
}
#endif

#endif
