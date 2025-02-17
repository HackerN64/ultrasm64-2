#include "PR/os_internal.h"
#include "osint.h"

OSThread *__osGetCurrFaultedThread() {
    return __osActiveQueue; // should be __osFaultedThread
}
