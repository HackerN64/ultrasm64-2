#include "PR/os_internal.h"
#include "macros.h"

FORCE_BSS OSMesg osPiMesgBuff[1];
ALIGNED8 OSMesgQueue __osPiAccessQueue;
u32 __osPiAccessQueueEnabled = FALSE;

void __osPiCreateAccessQueue(void) {
    __osPiAccessQueueEnabled = TRUE;
    osCreateMesgQueue(&__osPiAccessQueue, &osPiMesgBuff[0], 1);
    osSendMesg(&__osPiAccessQueue, NULL, OS_MESG_NOBLOCK);
}

void __osPiGetAccess(void) {
    OSMesg dummyMesg;
    if (!__osPiAccessQueueEnabled) {
        __osPiCreateAccessQueue();
    }
    osRecvMesg(&__osPiAccessQueue, &dummyMesg, OS_MESG_BLOCK);
}

void __osPiRelAccess(void) {
    osSendMesg(&__osPiAccessQueue, NULL, OS_MESG_NOBLOCK);
}
