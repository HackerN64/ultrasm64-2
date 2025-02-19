/*
 * Super Mario 64 ROM header
 * Only the first 0x18 bytes matter to the console.
 */

.byte  0x80, 0x37, 0x12, 0x40   /* PI BSD Domain 1 register */
.word  0x0000000F               /* Clockrate setting*/
.word  entry_point              /* Entrypoint */

/* Revision */
.half 0x0000
.byte 20 /* Major version 2.0 */
.ascii "L" /* Minor Version */

#ifdef VERSION_CN
    .fill 0x30
#else

.word  0x4EAA3D0E               /* Checksum 1 */
.word  0x74757C24               /* Checksum 2 */
.word  0x00000000               /* Unknown */
.word  0x00000000               /* Unknown */
#ifdef VERSION_SH
.ascii "SUPERMARIO64        "   /* Internal ROM name */
#else
.ascii "SUPER MARIO 64      "   /* Internal ROM name */
#endif
.word  0x00000000               /* Unknown */
.word  0x0000004E               /* Cartridge */
.ascii "SM"                     /* Cartridge ID */

/* Region */
#ifdef VERSION_EU
    .ascii "P"                  /* PAL (Europe) */
#elif defined(VERSION_US)
    .ascii "E"                  /* NTSC-U (North America) */
#else
    .ascii "J"                  /* NTSC-J (Japan) */
#endif


#ifdef VERSION_SH
    .byte  0x03                 /* Version (Shindou) */
#else
    .byte  0x00                 /* Version */
#endif

#endif
