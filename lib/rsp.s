#include "macros.inc"
.set UCODE_SIZE, 0x800

.section .text

.balign 16
glabel rspF3DBootStart
    .incbin "rsp/rspboot.bin"
.balign 16
glabel rspF3DBootEnd

.balign 16
glabel rspF3DStart
#ifdef F3DZEX_GBI_2
    .incbin "lib/PR/f3dex2/F3DZEX_NoN.bin"
#elif defined(F3DEX_GBI)
    .incbin "lib/PR/f3dex/F3DEX_NoN.bin"
#elif defined(F3DEX_GBI_2)
    .incbin "lib/PR/f3dex2/F3DEX2_NoN.bin"
#else
    .incbin "rsp/fast3d.bin"
#endif
.balign 16
glabel rspF3DEnd

.balign 16
glabel rspAspMainStart
    .incbin "rsp/audio.bin"
.balign 16
glabel rspAspMainEnd

/* DATA SECTION START */

.section .rodata

.balign 16
glabel rspF3DDataStart
#ifdef F3DZEX_GBI_2
    .incbin "lib/PR/f3dex2/F3DZEX_NoN_data.bin"
#elif defined(F3DEX_GBI)
    .incbin "lib/PR/f3dex/F3DEX_NoN_data.bin"
#elif defined(F3DEX_GBI_2)
    .incbin "lib/PR/f3dex2/F3DEX2_NoN_data.bin"
#else
    .incbin "rsp/fast3d_data.bin"
#endif
.balign 16
glabel rspF3DDataEnd

.balign 16
glabel rspAspMainDataStart
    .incbin "rsp/audio_data.bin"
.balign 16
glabel rspAspMainDataEnd
