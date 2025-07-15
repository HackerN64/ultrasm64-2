OUTPUT_ARCH (mips)

/* include/segments.h defines SEG_POOL_START, SEG_POOL_END, SEG_BUFFERS,
 * SEG_GODDARD, SEG_ENGINE, SEG_FRAMEBUFFERS */

#define _LD
#include "segments.h"

#include "config.h"
#include "ld.h"

SECTIONS
{
    __romPos = 0;

    BEGIN_SEG(boot, 0x04000000)
    {
        BUILD_DIR/asm/rom_header.o(.text);
        BUILD_DIR/asm/boot.o(.text);
#ifndef LIBDRAGON_IPL3
        BUILD_DIR/asm/ipl3_font.o(.text);
#endif
    }
    END_SEG(boot)

    . = 0x80000400;
    BEGIN_NOLOAD(zbuffer) {
        BUILD_DIR/src/buffers/zbuffer.o(.bss*);
        . = ALIGN(0x10);
    }
    END_NOLOAD(zbuffer)
 
    . = _zbufferSegmentNoloadEnd;

    BEGIN_NOLOAD(buffers)
    {
        BUILD_DIR/src/buffers/buffers.o(.bss*);
        . = ALIGN(0x10);
        BUILD_DIR/src/buffers/gfx_output_buffer.o(.bss*);
        . = ALIGN(0x10);
    }
    END_NOLOAD(buffers)

   . = _buffersSegmentNoloadEnd;

    BEGIN_SEG(main, .)
    {
        BUILD_DIR/asm/entry.o(.text);
        BUILD_DIR/asm/decompress.o(.text*);
        BUILD_DIR/asm/reboot.o(.text);
        BUILD_DIR/src/init/dma_async.o(.text);
        BUILD_DIR/src/init*.o(.text);
        AUDIO_DIR*.o(.text);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.text);
        BUILD_DIR/src/audio/external.o(.text);
        BUILD_DIR/n64-libc.a:*.o(.text*);
        BUILD_DIR/libpl2.a:*.o(.text);
        BUILD_DIR/lib/rsp.o(.text);

        BUILD_DIR/asm/entry.o(.data*);
        BUILD_DIR/asm/decompress.o(.data*);
        BUILD_DIR/asm/reboot.o(.data*);
        BUILD_DIR/src/init/dma_async.o(.data);
        BUILD_DIR/src/init*.o(.data*);
        AUDIO_DIR*.o(.data*);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.data*);
        BUILD_DIR/src/audio/external.o(.data*);
        BUILD_DIR/n64-libc.a:*.o(.data*);
        BUILD_DIR/libpl2.a:*.o(.data*);
        BUILD_DIR/lib/rsp.o(.data*);

        BUILD_DIR/asm/entry.o(.rodata*);
        BUILD_DIR/asm/decompress.o(.rodata*);
        BUILD_DIR/asm/reboot.o(.rodata*);
        BUILD_DIR/src/init/dma_async.o(.rodata);
        BUILD_DIR/src/init*.o(.rodata*);
        AUDIO_DIR*.o(.rodata*);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.rodata*);
        BUILD_DIR/src/audio/external.o(.rodata*);
        BUILD_DIR/n64-libc.a:*.o(.rodata*);
        BUILD_DIR/libpl2.a:*.o(.rodata*);
        BUILD_DIR/lib/rsp.o(.rodata*);
#ifdef LIBDRAGON_IPL3
        BUILD_DIR/src/game*.o(.text);
        BUILD_DIR/src/game*.o(.data*);
        BUILD_DIR/src/game*.o(.rodata*);
#endif
        . = ALIGN(0x10);
    }
    END_SEG(main)

    BEGIN_NOLOAD(main)
    {
        BUILD_DIR/src/init*.o(.bss*);
        AUDIO_DIR*.o(.bss*);
        BUILD_DIR/src/audio/external.o(.bss*);
        BUILD_DIR/n64-libc.a:*.o(.bss*);
        BUILD_DIR/libpl2.a:*.o(.bss*);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.bss*);
        BUILD_DIR/src/game*.o(.bss*);
        . = ALIGN(0x10);
    }
    END_NOLOAD(main)

    CREATE_LO_HI_PAIR(_mainSegmentNoloadSize, SIZEOF (.main.noload))
    . = _mainSegmentNoloadEnd;

#ifndef LIBDRAGON_IPL3
    . = ALIGN(0x10);
#else
    . = ALIGN(0x40);
#endif

#ifndef LIBDRAGON_IPL3
    BEGIN_SEG(engine, .)
    {
        BUILD_DIR/src/game*.o(.text);
        BUILD_DIR/src/game*.o(.data*);
        BUILD_DIR/src/game*.o(.rodata*);
        . = ALIGN(0x40);
    }
    END_SEG(engine)

    . = _engineSegmentEnd;
#endif

    BEGIN_NOLOAD(framebuffers)
    {
        BUILD_DIR/src/buffers/framebuffers.o(.bss*);
        . = ALIGN(0x10);
    }
    END_NOLOAD(framebuffers)

    __mainPoolStart = .;
    __mainPoolSize = RAM_END - .;

   ASSERT((. <= (SEG_POOL_START + POOL_SIZE)), "Error: extended past pool end.")

    BEGIN_SEG(entry, 0x10000000)
    {
        BUILD_DIR/levels/entry.o(.data);
    }
    END_SEG(entry)

    /* load MIO0 and level data */
    MIO0_SEG(segment2, 0x02000000)
#ifdef VERSION_EU
    MIO0_EU_SEG(translation_en, 0x19000000)
    MIO0_EU_SEG(translation_fr, 0x19000000)
    MIO0_EU_SEG(translation_de, 0x19000000)
#endif

    /* mario actor group */
    STANDARD_OBJECTS(group0,  0x04000000, 0x17000000)

    /* load the other actor groups */
    STANDARD_OBJECTS(group1,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group2,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group3,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group4,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group5,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group6,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group7,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group8,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group9,  0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group10, 0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group11, 0x05000000, 0x0C000000)
    STANDARD_OBJECTS(group12, 0x06000000, 0x0D000000)
    STANDARD_OBJECTS(group13, 0x06000000, 0x0D000000)
    STANDARD_OBJECTS(group14, 0x06000000, 0x0D000000)
    STANDARD_OBJECTS(group15, 0x06000000, 0x0D000000)
    STANDARD_OBJECTS(group16, 0x06000000, 0x0D000000)
    STANDARD_OBJECTS(group17, 0x06000000, 0x0D000000)

    /* load the common actor groups */
    STANDARD_OBJECTS(common0, 0x08000000, 0x0F000000)
    STANDARD_OBJECTS(common1, 0x03000000, 0x16000000)

    /* use segmented addressing for behaviors */
    BEGIN_SEG(behavior, 0x13000000)
    {
        BUILD_DIR/data/behavior_data.o(.data);
        . = ALIGN(0x10);
    }
    END_SEG(behavior)

    BEGIN_SEG(gd_dynlists, 0x04000000)
    {
        EXCLUDE_FILE(BUILD_DIR/libgoddard.a:dynlist_proc.o)
        BUILD_DIR/libgoddard.a:dynlist_*.o(.data*);
        BUILD_DIR/libgoddard.a:dynlists_*.o(.data*);
        BUILD_DIR/libgoddard.a:anim_*.o(.data*);
        EXCLUDE_FILE(BUILD_DIR/libgoddard.a:dynlist_proc.o)
        BUILD_DIR/libgoddard.a:dynlist_*.o(.rodata*);
        BUILD_DIR/libgoddard.a:dynlists_*.o(.rodata*);
        BUILD_DIR/libgoddard.a:anim_*.o(.rodata*);
        . = ALIGN(0x10);
    }
    END_SEG(gd_dynlists)

    BEGIN_SEG(goddard, SEG_GODDARD)
    {
        BUILD_DIR/src/menu*.o(.text);
        BUILD_DIR/libgoddard.a:*.o(.text);
        BUILD_DIR/libgoddard.a:dynlist_proc.o(.text);
        BUILD_DIR/src/menu*.o(.data*);
        BUILD_DIR/src/menu*.o(.rodata*);
        BUILD_DIR/libgoddard.a:*.o(.data*);
        BUILD_DIR/libgoddard.a:dynlist_proc.o(.data*);
        BUILD_DIR/libgoddard.a:*.o(.rodata*);
        BUILD_DIR/libgoddard.a:dynlist_proc.o(.rodata*);
        . = ALIGN(0x10);
    }

    END_SEG(goddard)
    BEGIN_NOLOAD(goddard)
    {
        BUILD_DIR/src/menu*.o(.bss*);
        BUILD_DIR/libgoddard.a:*.o(.bss*);
        . = ALIGN(0x10);
    }
    END_NOLOAD(goddard)

    BEGIN_SEG(intro, 0x14000000)
    {
        BUILD_DIR/levels/intro/script.o(.data);
        BUILD_DIR/levels/intro/geo.o(.data);
    }
    END_SEG(intro)
    BEGIN_SEG(intro_segment_7, 0x07000000)
    {
        BUILD_DIR/levels/intro/leveldata.mio0.o(.data);
        . = ALIGN(0x10);
    }
    END_SEG(intro_segment_7)

    MIO0_SEG(debug_level_select, 0x07000000)
    MIO0_SEG(title_screen_bg, 0x0A000000)

    gMainLevels = __romPos;

    BEGIN_SEG(menu, 0x14000000)
    {
        BUILD_DIR/levels/menu/script.o(.data);
        BUILD_DIR/levels/menu/geo.o(.data);
    }
    END_SEG(menu)
    BEGIN_SEG(menu_segment_7, 0x07000000)
    {
        BUILD_DIR/levels/menu/leveldata.mio0.o(.data);
        . = ALIGN(0x10);
    }
    END_SEG(menu_segment_7)

    BEGIN_SEG(scripts, 0x15000000)
    {
        BUILD_DIR/levels/scripts.o(.data);
    }
    END_SEG(scripts)

#define SKYBOX_LD
#include "skyboxes.h"
#undef SKYBOX_LD

    // Texture bins
    MIO0_SEG(fire, 0x09000000)
    MIO0_SEG(spooky, 0x09000000)
    MIO0_SEG(generic, 0x09000000)
    MIO0_SEG(water, 0x09000000)
    MIO0_SEG(sky, 0x09000000)
    MIO0_SEG(snow, 0x09000000)
    MIO0_SEG(cave, 0x09000000)
    MIO0_SEG(machine, 0x09000000)
    MIO0_SEG(mountain, 0x09000000)
    MIO0_SEG(grass, 0x09000000)
    MIO0_SEG(outside, 0x09000000)
    MIO0_SEG(inside, 0x09000000)
    MIO0_SEG(effect, 0x0B000000)

#define STUB_LEVEL(_0, _1, _2, _3, _4, _5, _6, _7, _8)
#define DEFINE_LEVEL(_0, _1, _2, folder, _4, _5, _6, _7, _8, _9, _10) STANDARD_LEVEL(folder)

    #include "levels/level_defines.h"

#undef STUB_LEVEL
#undef DEFINE_LEVEL

    BEGIN_SEG(assets, __romPos)
    {
        BUILD_DIR/assets/mario_anim_data.o(.data);
        BUILD_DIR/assets/mario_anim_data.o(.rodata);
        BUILD_DIR/assets/demo_data.o(.data);
        BUILD_DIR/sound/sound_data.o(.data);
    }
    END_SEG(assets)

   /* DWARF debug sections.
      Symbols in the DWARF debugging sections are relative to the beginning
      of the section so we begin them at 0.  */
   .debug_rnglists 0 : { *(.debug_rnglists) }
   .debug_line_str 0 : { *(.debug_line_str) }
   /* DWARF 1 */
   .debug          0 : { *(.debug) }
   .line           0 : { *(.line) }
   /* GNU DWARF 1 extensions */
   .debug_srcinfo  0 : { *(.debug_srcinfo) }
   .debug_sfnames  0 : { *(.debug_sfnames) }
   /* DWARF 1.1 and DWARF 2 */
   .debug_aranges  0 : { *(.debug_aranges) }
   .debug_pubnames 0 : { *(.debug_pubnames) }
   /* DWARF 2 */
   .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
   .debug_abbrev   0 : { *(.debug_abbrev) }
   .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }
   .debug_frame    0 : { *(.debug_frame) }
   .debug_str      0 : { *(.debug_str) }
   .debug_loc      0 : { *(.debug_loc) }
   .debug_macinfo  0 : { *(.debug_macinfo) }
   /* SGI/MIPS DWARF 2 extensions */
   .debug_weaknames 0 : { *(.debug_weaknames) }
   .debug_funcnames 0 : { *(.debug_funcnames) }
   .debug_typenames 0 : { *(.debug_typenames) }
   .debug_varnames  0 : { *(.debug_varnames) }
   /* DWARF 3 */
   .debug_pubtypes 0 : { *(.debug_pubtypes) }
   .debug_ranges   0 : { *(.debug_ranges) }
   /* DWARF Extension.  */
   .debug_macro    0 : { *(.debug_macro) }
   .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }

    /* Discard everything not specifically mentioned above. */
    /DISCARD/ :
    {
        *(*);
    }

    ASSERT((_goddardSegmentNoloadEnd <= SEG_POOL_START + POOL_SIZE), "Error: menu segment extended past pool end")
#ifndef LIBDRAGON_IPL3
    ASSERT((_mainSegmentNoloadEnd <= _engineSegmentStart), "Error: main segment extended into engine.")
    ASSERT((_engineSegmentEnd <= _framebuffersSegmentNoloadStart), "Error: engine segment extended into framebuffers.")
#else
    ASSERT((_mainSegmentNoloadEnd <= _framebuffersSegmentNoloadStart), "Error: main segment extended into framebuffers.")
#endif
    ASSERT((_framebuffersSegmentNoloadEnd <= RAM_END), "Error: framebuffers segment extended past RDRAM end")
}
