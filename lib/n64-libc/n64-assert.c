#include "n64-assert.h"

#include "n64-stdlib.h"
#include "n64-stdio.h"

void __n64_assert_fail( const char *assertion, const char *file, unsigned int line, const char *fcn ) {
	n64_printf( "%s:%u: %s: Assertion `%s' failed.\n", file, line, fcn, assertion );
	n64_abort();
}
