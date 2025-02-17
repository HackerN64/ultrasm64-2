#include "PR/os_internal.h"
#include "PR/rcp.h"

s32 __osSiDeviceBusy() {
    register u32 stat = IO_READ(SI_STATUS_REG);

    if (stat & (SI_STATUS_DMA_BUSY | SI_STATUS_RD_BUSY)) {
        return TRUE;
    }

    return FALSE;
}
