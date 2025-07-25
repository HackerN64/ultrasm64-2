#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PRinternal/controller.h"

#define CHECK_IPAGE(p)                                                                                                 \
    (((p).ipage >= pfs->inode_start_page) && ((p).inode_t.bank < pfs->banks) && ((p).inode_t.page >= 0x01)             \
     && ((p).inode_t.page < 0x80))

static s32 __osPfsGetNextPage(OSPfs* pfs, u8* bank, __OSInode* inode, __OSInodeUnit* page) {
    s32 ret;

    if (page->inode_t.bank != *bank) {
        *bank = page->inode_t.bank;
        ERRCK(__osPfsRWInode(pfs, inode, PFS_READ, *bank));
    }

    *page = inode->inode_page[page->inode_t.page];

    if (!CHECK_IPAGE(*page)) {
        if (page->ipage == PFS_EOF) {
            return PFS_ERR_INVALID;
        }

        return PFS_ERR_INCONSISTENT;
    }
    return 0;
}
s32 osPfsReadWriteFile(OSPfs* pfs, s32 file_no, u8 flag, int offset, int size_in_bytes, u8* data_buffer) {
    s32 ret;
    __OSDir dir;
    __OSInode inode;
    __OSInodeUnit cur_page;
    int cur_block;
    int siz_block;
    u8* buffer;
    u8 bank;
    u16 blockno;

    if ((file_no >= (s32)pfs->dir_size) || (file_no < 0)) {
        return PFS_ERR_INVALID;
    }

    if ((size_in_bytes <= 0) || ((size_in_bytes % BLOCKSIZE) != 0)) {
        return PFS_ERR_INVALID;
    }

    if ((offset < 0) || ((offset % BLOCKSIZE) != 0)) {
        return PFS_ERR_INVALID;
    }

    PFS_CHECK_STATUS();
    PFS_CHECK_ID();
    SET_ACTIVEBANK_TO_ZERO();
    ERRCK(__osContRamRead(pfs->queue, pfs->channel, pfs->dir_table + file_no, (u8*)&dir));

    if (dir.company_code == 0 || dir.game_code == 0) {
        return PFS_ERR_INVALID;
    }

    if (!CHECK_IPAGE(dir.start_page)) {
        if ((dir.start_page.ipage == PFS_EOF)) {
            return PFS_ERR_INVALID;
        }

        return PFS_ERR_INCONSISTENT;
    }

    if (flag == PFS_READ && (dir.status & DIR_STATUS_OCCUPIED) == 0) {
        return PFS_ERR_BAD_DATA;
    }

    bank = -1;
    cur_block = offset / BLOCKSIZE;
    cur_page = dir.start_page;

    while (cur_block >= PFS_ONE_PAGE) {
        ERRCK(__osPfsGetNextPage(pfs, &bank, &inode, &cur_page));
        cur_block -= PFS_ONE_PAGE;
    }

    siz_block = size_in_bytes / BLOCKSIZE;
    buffer = data_buffer;

    while (siz_block > 0) {
        if (cur_block == PFS_ONE_PAGE) {
            ERRCK(__osPfsGetNextPage(pfs, &bank, &inode, &cur_page));
            cur_block = 0;
        }

        if (pfs->activebank != cur_page.inode_t.bank) {
            ERRCK(SELECT_BANK(pfs, cur_page.inode_t.bank));
        }

        blockno = cur_page.inode_t.page * PFS_ONE_PAGE + cur_block;

        if (flag == OS_READ) {
            ret = __osContRamRead(pfs->queue, pfs->channel, blockno, buffer);
        } else {
            ret = __osContRamWrite(pfs->queue, pfs->channel, blockno, buffer, FALSE);
        }

        if (ret != 0) {
            return ret;
        }
        buffer += BLOCKSIZE;
        cur_block++;
        siz_block--;
    }

    if (flag == PFS_WRITE && (dir.status & DIR_STATUS_OCCUPIED) == 0) {
        dir.status |= DIR_STATUS_OCCUPIED;
#if BUILD_VERSION >= VERSION_J
        SET_ACTIVEBANK_TO_ZERO();
#else
        ERRCK(SELECT_BANK(pfs, 0));
#endif
        ERRCK(__osContRamWrite(pfs->queue, pfs->channel, pfs->dir_table + file_no, (u8*)&dir, FALSE));
    }

#if BUILD_VERSION >= VERSION_J
    ret = __osPfsGetStatus(pfs->queue, pfs->channel);
    return ret;
#else
    return 0;
#endif
}
