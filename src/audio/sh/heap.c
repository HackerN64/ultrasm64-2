#include <ultra64.h>

#include "heap.h"
#include "data.h"
#include "load.h"
#include "synthesis.h"
#include "seqplayer.h"
#include "effects.h"

#define ALIGN16(val) (((val) + 0xF) & ~0xF)

struct PoolSplit {
    u32 wantSeq;
    u32 wantBank;
    u32 wantUnused;
    u32 wantCustom;
}; // size = 0x10

struct PoolSplit2 {
    u32 wantPersistent;
    u32 wantTemporary;
}; // size = 0x8

struct SoundAllocPool gAudioSessionPool;
struct SoundAllocPool gAudioInitPool;
struct SoundAllocPool gNotesAndBuffersPool;
u8 sAudioHeapPad[0x20]; // probably two unused pools
struct SoundAllocPool gSeqAndBankPool;
struct SoundAllocPool gPersistentCommonPool;
struct SoundAllocPool gTemporaryCommonPool;

struct SoundMultiPool gSeqLoadedPool;
struct SoundMultiPool gBankLoadedPool;
struct SoundMultiPool gUnusedLoadedPool;

struct Unk1Pool gUnkPool1;
struct UnkPool gUnkPool2;
struct UnkPool gUnkPool3;

struct PoolSplit sSessionPoolSplit;
struct PoolSplit2 sSeqAndBankPoolSplit;
struct PoolSplit sPersistentCommonPoolSplit;
struct PoolSplit sTemporaryCommonPoolSplit;

u8 gUnkLoadStatus[0x40];
u8 gBankLoadStatus[0x40];
u8 gSeqLoadStatus[0x100];

volatile u8 gAudioResetStatus;
u8 gAudioResetPresetIdToLoad;
s32 gAudioResetFadeOutFramesLeft;

u8 gAudioUnusedBuffer[0x1000];

extern s32 gMaxAudioCmds;

void *get_bank_or_seq_inner(s32 poolIdx, s32 arg1, s32 bankId);
struct UnkEntry *func_sh_802f1ec4(u32 size);
void func_sh_802f2158(struct UnkEntry *entry);
struct UnkEntry *unk_pool2_alloc(u32 size);
void func_sh_802F2320(struct UnkEntry *entry, struct AudioBankSample *sample);
void func_sh_802f23ec(void);

void unk_pools_init(u32 size1, u32 size2);

void reset_bank_and_seq_load_status(void) {
    s32 i;

    for (i = 0; i < 64; i++) {
        if (gBankLoadStatus[i] != SOUND_LOAD_STATUS_5) {
            gBankLoadStatus[i] = SOUND_LOAD_STATUS_NOT_LOADED;
        }
    }

    for (i = 0; i < 64; i++) {
        if (gUnkLoadStatus[i] != SOUND_LOAD_STATUS_5) {
            gUnkLoadStatus[i] = SOUND_LOAD_STATUS_NOT_LOADED;
        }
    }

    for (i = 0; i < 256; i++) {
        if (gSeqLoadStatus[i] != SOUND_LOAD_STATUS_5) {
            gSeqLoadStatus[i] = SOUND_LOAD_STATUS_NOT_LOADED;
        }
    }
}

void discard_bank(s32 bankId) {
    s32 i;

    for (i = 0; i < gMaxSimultaneousNotes; i++) {
        struct Note *note = &gNotes[i];

        if (note->bankId == bankId)
        {
            // (These prints are unclear. Arguments are picked semi-randomly.)
            eu_stubbed_printf_1("Warning:Kill Note  %x \n", i);
            if (note->unkSH34 == NOTE_PRIORITY_DISABLED && note->priority)
            {
                eu_stubbed_printf_3("Kill Voice %d (ID %d) %d\n", note->waveId,
                        bankId, note->priority);
                eu_stubbed_printf_0("Warning: Running Sequence's data disappear!\n");
                note->parentLayer->enabled = FALSE;
                note->parentLayer->finished = TRUE;
            }
            note_disable(note);
            audio_list_remove(&note->listItem);
            audio_list_push_back(&gNoteFreeLists.disabled, &note->listItem);
        }
    }
}

void discard_sequence(s32 seqId) {
    s32 i;

    for (i = 0; i < SEQUENCE_PLAYERS; i++) {
        if (gSequencePlayers[i].enabled && gSequencePlayers[i].seqId == seqId) {
            sequence_player_disable(&gSequencePlayers[i]);
        }
    }
}

void *soundAlloc(struct SoundAllocPool *pool, u32 size) {
    u8 *start;
    u8 *pos;
    u32 alignedSize = ALIGN16(size);

    start = pool->cur;
    if (start + alignedSize <= pool->start + pool->size) {
        pool->cur += alignedSize;
        for (pos = start; pos < pool->cur; pos++) {
            *pos = 0;
        }
    } else {
        eu_stubbed_printf_1("Heap OverFlow : Not Allocate %d!\n", size);
        return NULL;
    }
    pool->numAllocatedEntries++;
    return start;
}

void *sound_alloc_uninitialized(struct SoundAllocPool *pool, u32 size) {
    u8 *start;
    u32 alignedSize = ALIGN16(size);

    start = pool->cur;
    if (start + alignedSize <= pool->start + pool->size) {
        pool->cur += alignedSize;
    } else {
        return NULL;
    }

    pool->numAllocatedEntries++;
    return start;
}

void sound_alloc_pool_init(struct SoundAllocPool *pool, void *memAddr, u32 size) {
    pool->cur = pool->start = (u8 *) ALIGN16((uintptr_t) memAddr);
    pool->size = size - ((uintptr_t) memAddr & 0xf);
    pool->numAllocatedEntries = 0;
}

void persistent_pool_clear(struct PersistentPool *persistent) {
    persistent->pool.numAllocatedEntries = 0;
    persistent->pool.cur = persistent->pool.start;
    persistent->numEntries = 0;
}

void temporary_pool_clear(struct TemporaryPool *temporary) {
    temporary->pool.numAllocatedEntries = 0;
    temporary->pool.cur = temporary->pool.start;
    temporary->nextSide = 0;
    temporary->entries[0].ptr = temporary->pool.start;
    temporary->entries[1].ptr = temporary->pool.start + temporary->pool.size;
    temporary->entries[0].id = -1;
    temporary->entries[1].id = -1;
}

void unused_803160F8(struct SoundAllocPool *pool) {
    pool->numAllocatedEntries = 0;
    pool->cur = pool->start;
}

extern s32 D_SH_80315EE8;
void sound_init_main_pools(s32 sizeForAudioInitPool) {
    sound_alloc_pool_init(&gAudioInitPool, gAudioHeap, sizeForAudioInitPool);
    sound_alloc_pool_init(&gAudioSessionPool, gAudioHeap + sizeForAudioInitPool, gAudioHeapSize - sizeForAudioInitPool);
}

void session_pools_init(struct PoolSplit *a) {
    gAudioSessionPool.cur = gAudioSessionPool.start;
    sound_alloc_pool_init(&gNotesAndBuffersPool, sound_alloc_uninitialized(&gAudioSessionPool, a->wantSeq), a->wantSeq);
    sound_alloc_pool_init(&gSeqAndBankPool, sound_alloc_uninitialized(&gAudioSessionPool, a->wantCustom), a->wantCustom);
}

void seq_and_bank_pool_init(struct PoolSplit2 *a) {
    gSeqAndBankPool.cur = gSeqAndBankPool.start;
    sound_alloc_pool_init(&gPersistentCommonPool, sound_alloc_uninitialized(&gSeqAndBankPool, a->wantPersistent), a->wantPersistent);
    sound_alloc_pool_init(&gTemporaryCommonPool, sound_alloc_uninitialized(&gSeqAndBankPool, a->wantTemporary), a->wantTemporary);
}

void persistent_pools_init(struct PoolSplit *a) {
    gPersistentCommonPool.cur = gPersistentCommonPool.start;
    sound_alloc_pool_init(&gSeqLoadedPool.persistent.pool, sound_alloc_uninitialized(&gPersistentCommonPool, a->wantSeq), a->wantSeq);
    sound_alloc_pool_init(&gBankLoadedPool.persistent.pool, sound_alloc_uninitialized(&gPersistentCommonPool, a->wantBank), a->wantBank);
    sound_alloc_pool_init(&gUnusedLoadedPool.persistent.pool, sound_alloc_uninitialized(&gPersistentCommonPool, a->wantUnused), a->wantUnused);
    persistent_pool_clear(&gSeqLoadedPool.persistent);
    persistent_pool_clear(&gBankLoadedPool.persistent);
    persistent_pool_clear(&gUnusedLoadedPool.persistent);
}

void temporary_pools_init(struct PoolSplit *a) {
    gTemporaryCommonPool.cur = gTemporaryCommonPool.start;
    sound_alloc_pool_init(&gSeqLoadedPool.temporary.pool, sound_alloc_uninitialized(&gTemporaryCommonPool, a->wantSeq), a->wantSeq);
    sound_alloc_pool_init(&gBankLoadedPool.temporary.pool, sound_alloc_uninitialized(&gTemporaryCommonPool, a->wantBank), a->wantBank);
    sound_alloc_pool_init(&gUnusedLoadedPool.temporary.pool, sound_alloc_uninitialized(&gTemporaryCommonPool, a->wantUnused), a->wantUnused);
    temporary_pool_clear(&gSeqLoadedPool.temporary);
    temporary_pool_clear(&gBankLoadedPool.temporary);
    temporary_pool_clear(&gUnusedLoadedPool.temporary);
}

void *alloc_bank_or_seq(s32 poolIdx, s32 size, s32 arg3, s32 id) {
    // arg3 = 0, 1 or 2?

    struct SoundMultiPool *arg0;
    struct TemporaryPool *tp;
    struct SoundAllocPool *pool;
    void *ret;
    u16 firstVal;
    u16 secondVal;
    u32 nullID = -1;
    UNUSED s32 i;
    u8 *table;

    switch (poolIdx) {
        case 0:
            arg0 = &gSeqLoadedPool;
            table = gSeqLoadStatus;
            break;

        case 1:
            arg0 = &gBankLoadedPool;
            table = gBankLoadStatus;
            break;

        case 2:
            arg0 = &gUnusedLoadedPool;
            table = gUnkLoadStatus;
            break;
    }

    if (arg3 == 0) {
        tp = &arg0->temporary;

        if (tp->entries[0].id == (s8)nullID) {
            firstVal = SOUND_LOAD_STATUS_NOT_LOADED;
        } else {
            firstVal = table[tp->entries[0].id];
        }
        if (tp->entries[1].id == (s8)nullID) {
            secondVal = SOUND_LOAD_STATUS_NOT_LOADED;
        } else {
            secondVal = table[tp->entries[1].id];
        }

        if (poolIdx == 1) {
            if (firstVal == SOUND_LOAD_STATUS_4) {
                for (i = 0; i < gMaxSimultaneousNotes; i++) {
                    if (gNotes[i].bankId == tp->entries[0].id && gNotes[i].noteSubEu.enabled) {
                        break;
                    }
                }
                if (i == gMaxSimultaneousNotes) {
                    if (gBankLoadStatus[tp->entries[0].id] != SOUND_LOAD_STATUS_5) {
                        gBankLoadStatus[tp->entries[0].id] = SOUND_LOAD_STATUS_DISCARDABLE;
                    }
                    firstVal = SOUND_LOAD_STATUS_DISCARDABLE;
                }
            }
            if (secondVal == SOUND_LOAD_STATUS_4) {
                for (i = 0; i < gMaxSimultaneousNotes; i++) {
                    if (gNotes[i].bankId == tp->entries[1].id && gNotes[i].noteSubEu.enabled) {
                        break;
                    }
                }
                if (i == gMaxSimultaneousNotes) {
                    if (gBankLoadStatus[tp->entries[1].id] != SOUND_LOAD_STATUS_5) {
                        gBankLoadStatus[tp->entries[1].id] = SOUND_LOAD_STATUS_DISCARDABLE;
                    }
                    secondVal = SOUND_LOAD_STATUS_DISCARDABLE;
                }
            }
        }

        if (firstVal == SOUND_LOAD_STATUS_NOT_LOADED) {
            tp->nextSide = 0;
        } else if (secondVal == SOUND_LOAD_STATUS_NOT_LOADED) {
            tp->nextSide = 1;
        } else {
            eu_stubbed_printf_0("WARNING: NO FREE AUTOSEQ AREA.\n");
            if ((firstVal == SOUND_LOAD_STATUS_DISCARDABLE) && (secondVal == SOUND_LOAD_STATUS_DISCARDABLE)) {
                // Use the opposite side from last time.
            } else if (firstVal == SOUND_LOAD_STATUS_DISCARDABLE) {
                tp->nextSide = 0;
            } else if (secondVal == SOUND_LOAD_STATUS_DISCARDABLE) {
                tp->nextSide = 1;
            } else {
                if (poolIdx == 0) {
                    if (firstVal == SOUND_LOAD_STATUS_COMPLETE) {
                        for (i = 0; i < SEQUENCE_PLAYERS; i++) {
                            if (gSequencePlayers[i].enabled && gSequencePlayers[i].seqId == tp->entries[0].id) {
                                break;
                            }
                        }
                        if (i == SEQUENCE_PLAYERS) {
                            tp->nextSide = 0;
                            goto out;
                        }
                    }
                    if (secondVal == SOUND_LOAD_STATUS_COMPLETE) {
                        for (i = 0; i < SEQUENCE_PLAYERS; i++) {
                            if (gSequencePlayers[i].enabled && gSequencePlayers[i].seqId == tp->entries[1].id) {
                                break;
                            }
                        }
                        if (i == SEQUENCE_PLAYERS) {
                            tp->nextSide = 1;
                            goto out;
                        }
                    }
                } else if (poolIdx == 1) {
                    if (firstVal == SOUND_LOAD_STATUS_COMPLETE) {
                        for (i = 0; i < gMaxSimultaneousNotes; i++) {
                            if (gNotes[i].bankId == tp->entries[0].id && gNotes[i].noteSubEu.enabled) {
                                break;
                            }
                        }
                        if (i == gMaxSimultaneousNotes) {
                            tp->nextSide = 0;
                            goto out;
                        }
                    }
                    if (secondVal == SOUND_LOAD_STATUS_COMPLETE) {
                        for (i = 0; i < gMaxSimultaneousNotes; i++) {
                            if (gNotes[i].bankId == tp->entries[1].id && gNotes[i].noteSubEu.enabled) {
                                break;
                            }
                        }
                        if (i == gMaxSimultaneousNotes) {
                            tp->nextSide = 1;
                            goto out;
                        }
                    }
                }
                if (tp->nextSide == 0) {
                    if (firstVal == SOUND_LOAD_STATUS_IN_PROGRESS) {
                        if (secondVal != SOUND_LOAD_STATUS_IN_PROGRESS) {
                            tp->nextSide = 1;
                            goto out;
                        }
                    } else {
                        goto out;
                    }
                } else {
                    if (secondVal == SOUND_LOAD_STATUS_IN_PROGRESS) {
                        if (firstVal != SOUND_LOAD_STATUS_IN_PROGRESS) {
                            tp->nextSide = 0;
                            goto out;
                        }
                    } else {
                        goto out;
                    }
                }
                return NULL;
                out:;
            }
        }

        pool = &arg0->temporary.pool;
        if (tp->entries[tp->nextSide].id != (s8)nullID) {
            table[tp->entries[tp->nextSide].id] = SOUND_LOAD_STATUS_NOT_LOADED;
            if (poolIdx == 1) {
                discard_bank(tp->entries[tp->nextSide].id);
            }
        }

        switch (tp->nextSide) {
            case 0:
                tp->entries[0].ptr = pool->start;
                tp->entries[0].id = id;
                tp->entries[0].size = size;

                pool->cur = pool->start + size;

                if (tp->entries[1].id != (s32)nullID)
                if (tp->entries[1].ptr < pool->cur) {
                    eu_stubbed_printf_0("WARNING: Before Area Overlaid After.");

                    // Throw out the entry on the other side if it doesn't fit.
                    // (possible @bug: what if it's currently being loaded?)
                    table[tp->entries[1].id] = SOUND_LOAD_STATUS_NOT_LOADED;

                    switch (poolIdx) {
                        case 0:
                            discard_sequence(tp->entries[1].id);
                            break;
                        case 1:
                            discard_bank(tp->entries[1].id);
                            break;
                    }

                    tp->entries[1].id = (s32)nullID;
                    tp->entries[1].ptr = pool->start + pool->size;
                }

                ret = tp->entries[0].ptr;
                break;

            case 1:
                tp->entries[1].ptr = (u8 *) ((uintptr_t) (pool->start + pool->size - size) & ~0x0f);
                tp->entries[1].id = id;
                tp->entries[1].size = size;

                if (tp->entries[0].id != (s32)nullID)
                if (tp->entries[1].ptr < pool->cur) {
                    eu_stubbed_printf_0("WARNING: After Area Overlaid Before.");

                    table[tp->entries[0].id] = SOUND_LOAD_STATUS_NOT_LOADED;

                    switch (poolIdx) {
                        case 0:
                            discard_sequence(tp->entries[0].id);
                            break;
                        case 1:
                            discard_bank(tp->entries[0].id);
                            break;
                    }

                    tp->entries[0].id = (s32)nullID;
                    pool->cur = pool->start;
                }

                ret = tp->entries[1].ptr;
                break;

            default:
                eu_stubbed_printf_1("MEMORY:SzHeapAlloc ERROR: sza->side %d\n", tp->nextSide);
                return NULL;
        }

        // Switch sides for next time in case both entries are
        // SOUND_LOAD_STATUS_DISCARDABLE.
        tp->nextSide ^= 1;

        return ret;
    }

    ret = sound_alloc_uninitialized(&arg0->persistent.pool, size);
    arg0->persistent.entries[arg0->persistent.numEntries].ptr = ret;

    if (ret == NULL)
    {
        switch (arg3) {
            case 2:
                return alloc_bank_or_seq(poolIdx, size, 0, id);
            case 1:
            case 0:
                eu_stubbed_printf_1("MEMORY:StayHeap OVERFLOW (REQ:%d)", arg1 * size);
                return NULL;
        }
    }

    // TODO: why is this guaranteed to write <= 32 entries...?
    // Because the buffer is small enough that more don't fit?
    arg0->persistent.entries[arg0->persistent.numEntries].id = id;
    arg0->persistent.entries[arg0->persistent.numEntries].size = size;
    return arg0->persistent.entries[arg0->persistent.numEntries++].ptr;
}

void *get_bank_or_seq(s32 poolIdx, s32 arg1, s32 id) {
    void *ret;

    ret = unk_pool1_lookup(poolIdx, id);
    if (ret != NULL) {
        return ret;
    }
    if (arg1 == 3) {
        return NULL;
    }
    return get_bank_or_seq_inner(poolIdx, arg1, id);
}
void *get_bank_or_seq_inner(s32 poolIdx, s32 arg1, s32 bankId) {
    u32 i;
    struct SoundMultiPool* loadedPool;
    struct TemporaryPool* temporary;
    struct PersistentPool* persistent;

    switch (poolIdx) {
        case 0:
            loadedPool = &gSeqLoadedPool;
            break;
        case 1:
            loadedPool = &gBankLoadedPool;
            break;
        case 2:
            loadedPool = &gUnusedLoadedPool;
            break;
    }

    temporary = &loadedPool->temporary;
    if (arg1 == 0) {
        if (temporary->entries[0].id == bankId) {
            temporary->nextSide = 1;
            return temporary->entries[0].ptr;
        } else if (temporary->entries[1].id == bankId) {
            temporary->nextSide = 0;
            return temporary->entries[1].ptr;
        } else {
            return NULL;
        }
    }

    persistent = &loadedPool->persistent;
    for (i = 0; i < persistent->numEntries; i++) {
        if (persistent->entries[i].id == bankId) {
            return persistent->entries[i].ptr;
        }
    }

    if (arg1 == 2) {
        return get_bank_or_seq(poolIdx, 0, bankId);
    }
    return NULL;
}

void func_eu_802e27e4_unused(f32 arg0, f32 arg1, u16 *arg2) {
    s32 i;
    f32 tmp[16];

    tmp[0] = (f32) (arg1 * 262159.0f);
    tmp[8] = (f32) (arg0 * 262159.0f);
    tmp[1] = (f32) ((arg1 * arg0) * 262159.0f);
    tmp[9] = (f32) (((arg0 * arg0) + arg1) * 262159.0f);

    for (i = 2; i < 8; i++) {
        //! @bug they probably meant to store the value to tmp[i] and tmp[8 + i]
        arg2[i] = arg1 * tmp[i - 2] + arg0 * tmp[i - 1];
        arg2[8 + i] = arg1 * tmp[6 + i] + arg0 * tmp[7 + i];
    }

    for (i = 0; i < 16; i++) {
        arg2[i] = tmp[i];
    }
}

void fill_zero_filter(s16 filter[]) {
    s32 i;
    for (i = 0; i < 8; i++) {
        filter[i] = 0;
    }
}

extern s16 unk_sh_data_3[15 * 8];
extern s16 unk_sh_data_4[15 * 8];
void func_sh_802F0DE8(s16 filter[8], s32 arg1) {
    s32 i;
    s16 *ptr = &unk_sh_data_3[8 * (arg1 - 1)];
    for (i = 0; i < 8; i++) {
        filter[i] = ptr[i];
    }
}

void func_sh_802F0E40(s16 filter[8], s32 arg1) { // Unused
    s32 i;
    s16 *ptr = &unk_sh_data_4[8 * (arg1 - 1)];
    for (i = 0; i < 8; i++) {
        filter[i] = ptr[i];
    }
}

void fill_filter(s16 filter[8], s32 arg1, s32 arg2) {
    s32 i;
    s16 *ptr;
    if (arg1 != 0) {
        func_sh_802F0DE8(filter, arg1);
    } else {
        fill_zero_filter(filter);
    }
    if (arg2 != 0) {
        ptr = &unk_sh_data_4[8 * (arg2 - 1)];
        for (i = 0; i < 8; i++) {
            filter[i] += ptr[i];
        }
    }
}

void decrease_reverb_gain(void) {
    s32 i, j;
    s32 v0 = gAudioBufferParameters.presetUnk4 == 2 ? 2 : 1;
    for (i = 0; i < gNumSynthesisReverbs; i++) {
        for (j = 0; j < v0; j++) {
            gSynthesisReverbs[i].reverbGain -= gSynthesisReverbs[i].reverbGain / 3;
        }
    }
}

void clear_curr_ai_buffer(void) {
    s32 currIndex = gCurrAiBufferIndex;
    s32 i;
    gAiBufferLengths[currIndex] = gAudioBufferParameters.minAiBufferLength;
    for (i = 0; i < (s32) (AIBUFFER_LEN / sizeof(s16)); i++) {
        gAiBuffers[currIndex][i] = 0;
    }
}


s32 audio_shut_down_and_reset_step(void) {
    s32 i;
    s32 j;
    s32 num = gAudioBufferParameters.presetUnk4 == 2 ? 2 : 1;

    switch (gAudioResetStatus) {
        case 5:
            for (i = 0; i < SEQUENCE_PLAYERS; i++) {
                sequence_player_disable(&gSequencePlayers[i]);
            }
            gAudioResetFadeOutFramesLeft = 4 / num;
            gAudioResetStatus--;
            break;
        case 4:
            if (gAudioResetFadeOutFramesLeft != 0) {
                gAudioResetFadeOutFramesLeft--;
                decrease_reverb_gain();
            } else {
                for (i = 0; i < gMaxSimultaneousNotes; i++) {
                    if (gNotes[i].noteSubEu.enabled && gNotes[i].adsr.state != ADSR_STATE_DISABLED) {
                        gNotes[i].adsr.fadeOutVel = gAudioBufferParameters.updatesPerFrameInv;
                        gNotes[i].adsr.action |= ADSR_ACTION_RELEASE;
                    }
                }
                gAudioResetFadeOutFramesLeft = 16 / num;
                gAudioResetStatus--;
            }
            break;
        case 3:
            if (gAudioResetFadeOutFramesLeft != 0) {
                gAudioResetFadeOutFramesLeft--;
                if (1) {
                }
                decrease_reverb_gain();
            } else {
                for (i = 0; i < NUMAIBUFFERS; i++) {
                    for (j = 0; j < (s32) (AIBUFFER_LEN / sizeof(s16)); j++) {
                        gAiBuffers[i][j] = 0;
                    }
                }
                gAudioResetFadeOutFramesLeft = 4 / num;
                gAudioResetStatus--;
            }
            break;
        case 2:
            clear_curr_ai_buffer();
            if (gAudioResetFadeOutFramesLeft != 0) {
                gAudioResetFadeOutFramesLeft--;
            } else {
                gAudioResetStatus--;
                func_sh_802f23ec();
            }
            break;
        case 1:
            audio_reset_session();
            gAudioResetStatus = 0;
            for (i = 0; i < NUMAIBUFFERS; i++) {
                gAiBufferLengths[i] = gAudioBufferParameters.maxAiBufferLength;
                for (j = 0; j < (s32) (AIBUFFER_LEN / sizeof(s16)); j++) {
                    gAiBuffers[i][j] = 0;
                }
            }
    }
    if (gAudioResetFadeOutFramesLeft) {
    }
    if (gAudioResetStatus < 3) {
        return 0;
    }
    return 1;
}

void audio_reset_session(void) {
    struct AudioSessionSettingsEU *preset = &gAudioSessionPresets[gAudioResetPresetIdToLoad];
    struct ReverbSettingsEU *reverbSettings;
    s16 *mem;
    s32 i;
    s32 j;
    s32 persistentMem;
    s32 temporaryMem;
    s32 totalMem;
    s32 wantMisc;
    struct SynthesisReverb *reverb;
    eu_stubbed_printf_1("Heap Reconstruct Start %x\n", gAudioResetPresetIdToLoad);

    gSampleDmaNumListItems = 0;
    gAudioBufferParameters.frequency = preset->frequency;
    gAudioBufferParameters.aiFrequency = osAiSetFrequency(gAudioBufferParameters.frequency);
    gAudioBufferParameters.samplesPerFrameTarget = ALIGN16(gAudioBufferParameters.frequency / gRefreshRate);
    gAudioBufferParameters.minAiBufferLength = gAudioBufferParameters.samplesPerFrameTarget - 0x10;
    gAudioBufferParameters.maxAiBufferLength = gAudioBufferParameters.samplesPerFrameTarget + 0x10;
    gAudioBufferParameters.updatesPerFrame = (gAudioBufferParameters.samplesPerFrameTarget + 0x10) / 192 + 1;
    gAudioBufferParameters.samplesPerUpdate = (gAudioBufferParameters.samplesPerFrameTarget / gAudioBufferParameters.updatesPerFrame) & -8;
    gAudioBufferParameters.samplesPerUpdateMax = gAudioBufferParameters.samplesPerUpdate + 8;
    gAudioBufferParameters.samplesPerUpdateMin = gAudioBufferParameters.samplesPerUpdate - 8;
    gAudioBufferParameters.resampleRate = 32000.0f / FLOAT_CAST(gAudioBufferParameters.frequency);
    gAudioBufferParameters.unkUpdatesPerFrameScaled = (1.0f / 256.0f) / gAudioBufferParameters.updatesPerFrame;
    gAudioBufferParameters.updatesPerFrameInv = 1.0f / gAudioBufferParameters.updatesPerFrame;

    gMaxSimultaneousNotes = preset->maxSimultaneousNotes;
    gVolume = preset->volume;
    gTempoInternalToExternal = (u32) (gAudioBufferParameters.updatesPerFrame * 2880000.0f / gTatumsPerBeat / D_EU_802298D0);

    gAudioBufferParameters.presetUnk4 = preset->unk1;
    gAudioBufferParameters.samplesPerFrameTarget *= gAudioBufferParameters.presetUnk4;
    gAudioBufferParameters.maxAiBufferLength *= gAudioBufferParameters.presetUnk4;
    gAudioBufferParameters.minAiBufferLength *= gAudioBufferParameters.presetUnk4;
    gAudioBufferParameters.updatesPerFrame *= gAudioBufferParameters.presetUnk4;

    if (gAudioBufferParameters.presetUnk4 >= 2) {
        gAudioBufferParameters.maxAiBufferLength -= 0x10;
    }
    gMaxAudioCmds = gMaxSimultaneousNotes * 0x14 * gAudioBufferParameters.updatesPerFrame + preset->numReverbs * 0x20 + 0x1E0;

    persistentMem = DOUBLE_SIZE_ON_64_BIT(preset->persistentSeqMem + preset->persistentBankMem + preset->unk18 + preset->unkMem28 + 0x10);
    temporaryMem = DOUBLE_SIZE_ON_64_BIT(preset->temporarySeqMem + preset->temporaryBankMem + preset->unk24 + preset->unkMem2C + 0x10);
    totalMem = persistentMem + temporaryMem;
    wantMisc = gAudioSessionPool.size - totalMem - 0x100;
    sSessionPoolSplit.wantSeq = wantMisc;
    sSessionPoolSplit.wantCustom = totalMem;
    session_pools_init(&sSessionPoolSplit);
    sSeqAndBankPoolSplit.wantPersistent = persistentMem;
    sSeqAndBankPoolSplit.wantTemporary = temporaryMem;
    seq_and_bank_pool_init(&sSeqAndBankPoolSplit);
    sPersistentCommonPoolSplit.wantSeq = DOUBLE_SIZE_ON_64_BIT(preset->persistentSeqMem);
    sPersistentCommonPoolSplit.wantBank = DOUBLE_SIZE_ON_64_BIT(preset->persistentBankMem);
    sPersistentCommonPoolSplit.wantUnused = preset->unk18;
    persistent_pools_init(&sPersistentCommonPoolSplit);
    sTemporaryCommonPoolSplit.wantSeq = DOUBLE_SIZE_ON_64_BIT(preset->temporarySeqMem);
    sTemporaryCommonPoolSplit.wantBank = DOUBLE_SIZE_ON_64_BIT(preset->temporaryBankMem);
    sTemporaryCommonPoolSplit.wantUnused = preset->unk24;
    temporary_pools_init(&sTemporaryCommonPoolSplit);
    unk_pools_init(preset->unkMem28, preset->unkMem2C);
    reset_bank_and_seq_load_status();

    gNotes = soundAlloc(&gNotesAndBuffersPool, gMaxSimultaneousNotes * sizeof(struct Note));
    note_init_all();
    init_note_free_list();

    gNoteSubsEu = soundAlloc(&gNotesAndBuffersPool, (gAudioBufferParameters.updatesPerFrame * gMaxSimultaneousNotes) * sizeof(struct NoteSubEu));

    for (j = 0; j != 2; j++) {
        gAudioCmdBuffers[j] = soundAlloc(&gNotesAndBuffersPool, gMaxAudioCmds * sizeof(u64));
    }

    for (j = 0; j < 4; j++) {
        gSynthesisReverbs[j].useReverb = 0;
    }
    gNumSynthesisReverbs = preset->numReverbs;
    for (j = 0; j < gNumSynthesisReverbs; j++) {
        reverb = &gSynthesisReverbs[j];
        reverbSettings = &preset->reverbSettings[j];
        reverb->downsampleRate = reverbSettings->downsampleRate;
        reverb->windowSize = reverbSettings->windowSize * 64;
        reverb->windowSize /= reverb->downsampleRate;
        reverb->reverbGain = reverbSettings->gain;
        reverb->panRight = reverbSettings->unk4;
        reverb->panLeft = reverbSettings->unk6;
        reverb->unk5 = reverbSettings->unk8;
        reverb->unk08 = reverbSettings->unkA;
        reverb->useReverb = 8;
        reverb->ringBuffer.left = soundAlloc(&gNotesAndBuffersPool, reverb->windowSize * 2);
        reverb->ringBuffer.right = soundAlloc(&gNotesAndBuffersPool, reverb->windowSize * 2);
        reverb->nextRingBufferPos = 0;
        reverb->unkC = 0;
        reverb->curFrame = 0;
        reverb->bufSizePerChannel = reverb->windowSize;
        reverb->framesLeftToIgnore = 2;
        reverb->resampleFlags = A_INIT;
        if (reverb->downsampleRate != 1) {
            reverb->resampleRate = 0x8000 / reverb->downsampleRate;
            reverb->resampleStateLeft = soundAlloc(&gNotesAndBuffersPool, 16 * sizeof(s16));
            reverb->resampleStateRight = soundAlloc(&gNotesAndBuffersPool, 16 * sizeof(s16));
            reverb->unk24 = soundAlloc(&gNotesAndBuffersPool, 16 * sizeof(s16));
            reverb->unk28 = soundAlloc(&gNotesAndBuffersPool, 16 * sizeof(s16));
            for (i = 0; i < gAudioBufferParameters.updatesPerFrame; i++) {
                mem = soundAlloc(&gNotesAndBuffersPool, DEFAULT_LEN_2CH);
                reverb->items[0][i].toDownsampleLeft = mem;
                reverb->items[0][i].toDownsampleRight = mem + DEFAULT_LEN_1CH / sizeof(s16);
                mem = soundAlloc(&gNotesAndBuffersPool, DEFAULT_LEN_2CH);
                reverb->items[1][i].toDownsampleLeft = mem;
                reverb->items[1][i].toDownsampleRight = mem + DEFAULT_LEN_1CH / sizeof(s16);
            }
        }
        if (reverbSettings->unkC != 0) {
            reverb->unk108 = sound_alloc_uninitialized(&gNotesAndBuffersPool, 16 * sizeof(s16));
            reverb->unk100 = sound_alloc_uninitialized(&gNotesAndBuffersPool, 8 * sizeof(s16));
            func_sh_802F0DE8(reverb->unk100, reverbSettings->unkC);
        } else {
            reverb->unk100 = NULL;
        }
        if (reverbSettings->unkE != 0) {
            reverb->unk10C = sound_alloc_uninitialized(&gNotesAndBuffersPool, 16 * sizeof(s16));
            reverb->unk104 = sound_alloc_uninitialized(&gNotesAndBuffersPool, 8 * sizeof(s16));
            func_sh_802F0DE8(reverb->unk104, reverbSettings->unkE);
        } else {
            reverb->unk104 = NULL;
        }
    }

    init_sample_dma_buffers(gMaxSimultaneousNotes);

    D_SH_8034F68C = 0;
    D_SH_803479B4 = 4096;

    osWritebackDCacheAll();
}

void *unk_pool1_lookup(s32 poolIdx, s32 id) {
    s32 i;

    for (i = 0; i < gUnkPool1.pool.numAllocatedEntries; i++) {
        if (gUnkPool1.entries[i].poolIndex == poolIdx && gUnkPool1.entries[i].id == id) {
            return gUnkPool1.entries[i].ptr;
        }
    }
    return NULL;
}

void *unk_pool1_alloc(s32 poolIndex, s32 arg1, u32 size) {
    void *ret;
    s32 pos;

    pos = gUnkPool1.pool.numAllocatedEntries;
    ret = sound_alloc_uninitialized(&gUnkPool1.pool, size);
    gUnkPool1.entries[pos].ptr = ret;
    if (ret == NULL) {
        return NULL;
    }
    gUnkPool1.entries[pos].poolIndex = poolIndex;
    gUnkPool1.entries[pos].id = arg1;
    gUnkPool1.entries[pos].size = size;

#ifdef AVOID_UB
    //! @bug UB: missing return. "ret" is in v0 at this point, but doing an
    //  explicit return uses an additional register.
    return ret;
#endif
}

u8 *func_sh_802f1d40(u32 size, s32 bank, u8 *arg2, s8 medium) {
    struct UnkEntry *ret;

    ret = func_sh_802f1ec4(size);
    if (ret != NULL) {
        ret->bankId = bank;
        ret->dstAddr = arg2;
        ret->medium = medium;
        return ret->srcAddr;
    }
    return NULL;
}
u8 *func_sh_802f1d90(u32 size, s32 bank, u8 *arg2, s8 medium) {
    struct UnkEntry *ret;

    ret = unk_pool2_alloc(size);
    if (ret != NULL) {
        ret->bankId = bank;
        ret->dstAddr = arg2;
        ret->medium = medium;
        return ret->srcAddr;
    }
    return NULL;
}
u8 *func_sh_802f1de0(u32 size, s32 bank, u8 *arg2, s8 medium) { // duplicated function?
    struct UnkEntry *ret;

    ret = unk_pool2_alloc(size);
    if (ret != NULL) {
        ret->bankId = bank;
        ret->dstAddr = arg2;
        ret->medium = medium;
        return ret->srcAddr;
    }
    return NULL;
}
void unk_pools_init(u32 size1, u32 size2) {
    void *mem;

    mem = sound_alloc_uninitialized(&gPersistentCommonPool, size1);
    if (mem == NULL) {
        gUnkPool2.pool.size = 0;
    } else {
        sound_alloc_pool_init(&gUnkPool2.pool, mem, size1);
    }
    mem = sound_alloc_uninitialized(&gTemporaryCommonPool, size2);

    if (mem == NULL) {
        gUnkPool3.pool.size = 0;
    } else {
        sound_alloc_pool_init(&gUnkPool3.pool, mem, size2);
    }

    gUnkPool2.numEntries = 0;
    gUnkPool3.numEntries = 0;
}

struct UnkEntry *func_sh_802f1ec4(u32 size) {
    u8 *temp_s2;
    u8 *phi_s3;
    u8 *memLocation;
    u8 *cur;

    s32 i;
    s32 chosenIndex;

    struct UnkStructSH8034EC88 *unkStruct;
    struct UnkPool *pool = &gUnkPool3;

    u8 *itemStart;
    u8 *itemEnd;

    phi_s3 = pool->pool.cur;
    memLocation = sound_alloc_uninitialized(&pool->pool, size);
    if (memLocation == NULL) {
        cur = pool->pool.cur;
        pool->pool.cur = pool->pool.start;
        memLocation = sound_alloc_uninitialized(&pool->pool, size);
        if (memLocation == NULL) {
            pool->pool.cur = cur;
            return NULL;
        }
        phi_s3 = pool->pool.start;
    }
    temp_s2 = pool->pool.cur;

    chosenIndex = -1;
    for (i = 0; i < D_SH_8034F68C; i++) {
        unkStruct = &D_SH_8034EC88[i];
        if (unkStruct->isFree == FALSE) {
            itemStart = unkStruct->ramAddr;
            itemEnd = unkStruct->ramAddr + unkStruct->sample->size - 1;
            if (itemEnd < phi_s3 && itemStart < phi_s3) {
                continue;
            }
            if (itemEnd >= temp_s2 && itemStart >= temp_s2) {
                continue;
            }

            unkStruct->isFree = TRUE;
        }
    }

    for (i = 0; i < pool->numEntries; i++) {
        if (pool->entries[i].used == FALSE) {
            continue;
        }
        itemStart = pool->entries[i].srcAddr;
        itemEnd = itemStart + pool->entries[i].size - 1;

        if (itemEnd < phi_s3 && itemStart < phi_s3) {
            continue;
        }

        if (itemEnd >= temp_s2 && itemStart >= temp_s2) {
            continue;
        }

        func_sh_802f2158(&pool->entries[i]);
        if (chosenIndex == -1) {
            chosenIndex = i;
        }
    }

    if (chosenIndex == -1) {
        chosenIndex = pool->numEntries++;
    }
    pool->entries[chosenIndex].used = TRUE;
    pool->entries[chosenIndex].srcAddr = memLocation;
    pool->entries[chosenIndex].size = size;

    return &pool->entries[chosenIndex];
}

void func_sh_802f2158(struct UnkEntry *entry) {
    s32 idx;
    s32 seqCount;
    s32 bankId1;
    s32 bankId2;
    s32 instId;
    s32 drumId;
    struct Drum *drum;
    struct Instrument *inst;

    seqCount = gAlCtlHeader->seqCount;
    for (idx = 0; idx < seqCount; idx++) {
        bankId1 = gCtlEntries[idx].bankId1;
        bankId2 = gCtlEntries[idx].bankId2;
        if ((bankId1 != 0xff && entry->bankId == bankId1) || (bankId2 != 0xff && entry->bankId == bankId2) || entry->bankId == 0) {
            if (get_bank_or_seq(1, 2, idx) != NULL) {
                if (IS_BANK_LOAD_COMPLETE(idx) != FALSE) {
                    for (instId = 0; instId < gCtlEntries[idx].numInstruments; instId++) {
                        inst = get_instrument_inner(idx, instId);
                        if (inst != NULL) {
                            if (inst->normalRangeLo != 0) {
                                func_sh_802F2320(entry, inst->lowNotesSound.sample);
                            }
                            if (inst->normalRangeHi != 127) {
                                func_sh_802F2320(entry, inst->highNotesSound.sample);
                            }
                            func_sh_802F2320(entry, inst->normalNotesSound.sample);
                        }
                    }
                    for (drumId = 0; drumId < gCtlEntries[idx].numDrums; drumId++) {
                        drum = get_drum(idx, drumId);
                        if (drum != NULL) {
                            func_sh_802F2320(entry, drum->sound.sample);
                        }
                    }
                }
            }
        }
    }
}

void func_sh_802F2320(struct UnkEntry *entry, struct AudioBankSample *sample) {
    if (sample != NULL && sample->sampleAddr == entry->srcAddr) {
        sample->sampleAddr = entry->dstAddr;
        sample->medium = entry->medium;
    }
}

struct UnkEntry *unk_pool2_alloc(u32 size) {
    void *data;
    struct UnkEntry *ret;
    s32 *numEntries = &gUnkPool2.numEntries;

    data = sound_alloc_uninitialized(&gUnkPool2.pool, size);
    if (data == NULL) {
        return NULL;
    }
    ret = &gUnkPool2.entries[*numEntries];
    ret->used = TRUE;
    ret->srcAddr = data;
    ret->size = size;
    (*numEntries)++;
    return ret;
}

void func_sh_802f23ec(void) {
    s32 i;
    s32 idx;
    s32 seqCount;
    s32 bankId1;
    s32 bankId2;
    s32 instId;
    s32 drumId;
    struct Drum *drum;
    struct Instrument *inst;
    UNUSED s32 pad;
    struct UnkEntry *entry; //! @bug: not initialized but nevertheless used

    seqCount = gAlCtlHeader->seqCount;
    for (idx = 0; idx < seqCount; idx++) {
        bankId1 = gCtlEntries[idx].bankId1;
        bankId2 = gCtlEntries[idx].bankId2;
        if ((bankId1 != 0xffu && entry->bankId == bankId1) || (bankId2 != 0xff && entry->bankId == bankId2) || entry->bankId == 0) {
            if (get_bank_or_seq(1, 3, idx) != NULL) {
                if (IS_BANK_LOAD_COMPLETE(idx) != FALSE) {
                    for (i = 0; i < gUnkPool2.numEntries; i++) {
                        entry = &gUnkPool2.entries[i];
                        for (instId = 0; instId < gCtlEntries[idx].numInstruments; instId++) {
                            inst = get_instrument_inner(idx, instId);
                            if (inst != NULL) {
                                if (inst->normalRangeLo != 0) {
                                    func_sh_802F2320(entry, inst->lowNotesSound.sample);
                                }
                                if (inst->normalRangeHi != 127) {
                                    func_sh_802F2320(entry, inst->highNotesSound.sample);
                                }
                                func_sh_802F2320(entry, inst->normalNotesSound.sample);
                            }
                        }
                        for (drumId = 0; drumId < gCtlEntries[idx].numDrums; drumId++) {
                            drum = get_drum(idx, drumId);
                            if (drum != NULL) {
                                func_sh_802F2320(entry, drum->sound.sample);
                            }
                        }
                    }
                }
            }
        }
    }
}

