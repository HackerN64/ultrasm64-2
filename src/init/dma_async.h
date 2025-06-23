#pragma once

#include "types.h"

typedef struct {
    u8* srcStart;
    u8* dest;
    u32 size;
} DMAAsyncCtx;

// Starts to DMA the first block
void dma_async_ctx_init(DMAAsyncCtx* ctx, u8 *dest, u8 *srcStart, u8 *srcEnd);

// Starts to DMA the next block and waits for the previous block
void* dma_async_ctx_read(DMAAsyncCtx* ctx);
