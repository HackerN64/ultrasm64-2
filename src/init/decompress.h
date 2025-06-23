#ifndef DECOMPRESS_H
#define DECOMPRESS_H

void lz4t_unpack_fast(const uint8_t* restrict inbuf, uint8_t* restrict dst, DMAAsyncCtx *ctx);
#define DMA_ASYNC_HEADER_SIZE 16

#endif // DECOMPRESS_H
