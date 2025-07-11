#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

#if BUILD_VERSION >= VERSION_J
static s32 __osPfsCheckRamArea(OSPfs* pfs);
#endif

s32 osPfsInitPak(OSMesgQueue* queue, OSPfs* pfs, int channel) {
#if BUILD_VERSION < VERSION_J
    int k;
#endif
    s32 ret = 0;
    u16 sum;
    u16 isum;
    u8 temp[BLOCKSIZE];
    __OSPackId* id;
    __OSPackId newid;

    __osSiGetAccess();

    ret = __osPfsGetStatus(queue, channel);

    __osSiRelAccess();

    if (ret != 0) {
        return ret;
    }

    pfs->queue = queue;
    pfs->channel = channel;
    pfs->status = 0;

#if BUILD_VERSION >= VERSION_J
    ERRCK(__osPfsCheckRamArea(pfs));
#endif
    ERRCK(SELECT_BANK(pfs, 0));
    ERRCK(__osContRamRead(pfs->queue, pfs->channel, PFS_ID_0AREA, temp));

    __osIdCheckSum((u16*)temp, &sum, &isum);
    id = (__OSPackId*)temp;

    if ((id->checksum != sum) || (id->inverted_checksum != isum)) {
        ret = __osCheckPackId(pfs, id);

        if (ret != 0) {
#if BUILD_VERSION >= VERSION_J
            pfs->status |= PFS_ID_BROKEN;
#endif
            return ret;
        }

#if BUILD_VERSION < VERSION_J
        // Duplicated check
        else if (ret != 0) {
            return ret;
        }
#endif
    }

    if (!(id->deviceid & 1)) {
        ret = __osRepairPackId(pfs, id, &newid);

        if (ret != 0) {
#if BUILD_VERSION >= VERSION_J
            if (ret == PFS_ERR_ID_FATAL) {
                pfs->status |= PFS_ID_BROKEN;
            }
#endif
            return ret;
        }

        id = &newid;

        if (!(id->deviceid & 1)) {
            return PFS_ERR_DEVICE;
        }
    }

#if BUILD_VERSION >= VERSION_J
    bcopy(id, pfs->id, BLOCKSIZE);
#else
    for (k = 0; k < ARRLEN(pfs->id); k++) {
        pfs->id[k] = ((u8*)id)[k];
    }
#endif

    pfs->version = id->version;
    pfs->banks = id->banks;
    pfs->inode_start_page = 1 + DEF_DIR_PAGES + (2 * pfs->banks);
    pfs->dir_size = DEF_DIR_PAGES * PFS_ONE_PAGE;
    pfs->inode_table = 1 * PFS_ONE_PAGE;
    pfs->minode_table = (1 + pfs->banks) * PFS_ONE_PAGE;
    pfs->dir_table = pfs->minode_table + (pfs->banks * PFS_ONE_PAGE);

    ERRCK(__osContRamRead(pfs->queue, pfs->channel, PFS_LABEL_AREA, pfs->label));

    ret = osPfsChecker(pfs);
    pfs->status |= PFS_INITIALIZED;

    return ret;
}

#if BUILD_VERSION >= VERSION_J
static s32 __osPfsCheckRamArea(OSPfs* pfs) {
    s32 i;
    s32 ret = 0;
    u8 temp1[BLOCKSIZE];
    u8 temp2[BLOCKSIZE];
    u8 save[BLOCKSIZE];

    ERRCK(SELECT_BANK(pfs, PFS_ID_BANK_256K));
    ERRCK(__osContRamRead(pfs->queue, pfs->channel, 0, save));

    for (i = 0; i < BLOCKSIZE; i++) {
        temp1[i] = i;
    }

    ERRCK(__osContRamWrite(pfs->queue, pfs->channel, 0, temp1, FALSE));
    ERRCK(__osContRamRead(pfs->queue, pfs->channel, 0, temp2));

    if (bcmp(temp1, temp2, BLOCKSIZE) != 0) {
        return PFS_ERR_DEVICE;
    }

    ret = __osContRamWrite(pfs->queue, pfs->channel, 0, save, FALSE);
    return ret;
}
#endif
