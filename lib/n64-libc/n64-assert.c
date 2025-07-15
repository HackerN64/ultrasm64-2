#include "n64-assert.h"

#include "n64-stdlib.h"
#include "n64-stdio.h"

extern char *assertMsg;
char assertBuf[256];

void __n64_assert_fail( const char *assertion, const char *file, unsigned int line, const char *fcn ) {
    n64_snprintf( assertBuf, 256, "%s:%u:%s: Assertion `%s' failed.", file, line, fcn, assertion );
    assertMsg = &assertBuf[0];
    n64_printf( "%s:%u: %s: Assertion `%s' failed.\n", file, line, fcn, assertion );
	  n64_abort();
}
