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
        BUILD_DIR/asm/ipl3_font.o(.text);
    }
    END_SEG(boot)

    . = 0x80000400;
    BEGIN_NOLOAD(zbuffer) {
        BUILD_DIR/src/buffers/zbuffer.o(.bss*);
    }
    END_NOLOAD(zbuffer)
 
    . = _zbufferSegmentNoloadEnd;

    BEGIN_NOLOAD(buffers)
    {
        BUILD_DIR/src/buffers/buffers.o(.bss*);
#ifdef VERSION_SH
        AUDIO_DIR/port.o(.bss*);
#endif
        AUDIO_DIR/globals_start.o(.bss*);
        AUDIO_DIR/synthesis.o(.bss*);
        AUDIO_DIR/heap.o(.bss*);
        AUDIO_DIR/load.o(.bss*);
        AUDIO_DIR/data.o(.bss*);
        AUDIO_DIR*.o(.bss*);

        BUILD_DIR/src/buffers/gfx_output_buffer.o(.bss*);
    }
    END_NOLOAD(buffers)
   . = _buffersSegmentNoloadEnd;
    BEGIN_SEG(main, .)
    {
        BUILD_DIR/asm/entry.o(.text);
        BUILD_DIR/asm/decompress.o(.text);
        BUILD_DIR/src/game*.o(.text);
        AUDIO_DIR*.o(.text);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.text);
        BUILD_DIR/src/audio/external.o(.text);
        BUILD_DIR/lib/rsp.o(.text);

        BUILD_DIR/asm/entry.o(.data*);
        BUILD_DIR/asm/decompress.o(.data*);
        BUILD_DIR/src/game*.o(.data*);
        AUDIO_DIR*.o(.data*);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.data*);
        BUILD_DIR/src/audio/external.o(.data*);
        BUILD_DIR/lib/rsp.o(.data*);
        
	BUILD_DIR/asm/entry.o(.rodata*);
        BUILD_DIR/asm/decompress.o(.rodata*);
        BUILD_DIR/src/game*.o(.rodata*);
        AUDIO_DIR*.o(.rodata*);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.rodata*);
        BUILD_DIR/src/audio/external.o(.rodata*);
        BUILD_DIR/lib/rsp.o(.rodata*);
    }
    END_SEG(main)

    BEGIN_NOLOAD(main)
    {
        BUILD_DIR/src/game*.o(.bss*);
        BUILD_DIR/src/audio/external.o(.bss*);
        AUDIO_DIR*.o(.bss*);
        ULTRA_BUILD_DIR/libgultra_rom.a:*.o(.bss*);

        . = ALIGN(0x8);
    }
    END_NOLOAD(main)

#ifdef VERSION_CN
    // Hack for CN to make these asm files to match, since we aren't using
    // the original assembler and so can't get lui/ori from la pseudo

    // asm/entry.s
    CREATE_LO_HI_PAIR(_mainSegmentNoloadStart, ADDR (.main.noload))
    CREATE_LO_HI_PAIR(gIdleThreadStack, gIdleThreadStack)
    CREATE_LO_HI_PAIR(main_func, main_func)
    
    // asm/boot.s
    SP_DMEM_CN_UNK0 = 0x0400049C;
    SP_DMEM_CN_UNK1 = 0x0400074C;
#endif

    CREATE_LO_HI_PAIR(_mainSegmentNoloadSize, SIZEOF (.main.noload))
    . = _mainSegmentNoloadEnd;

    BEGIN_SEG(engine, .)
    {
        BUILD_DIR/src/engine*.o(.text);
        BUILD_DIR/src/engine*.o(.data*);
        BUILD_DIR/src/engine*.o(.rodata*);
        . = ALIGN(0x100);
    }
    END_SEG(engine)
    BEGIN_NOLOAD(engine)
    {
        BUILD_DIR/src/engine*.o(.bss*);
    }
    END_NOLOAD(engine)

    . = _engineSegmentNoloadEnd;
    BEGIN_NOLOAD(framebuffers)
    {
        BUILD_DIR/src/buffers/framebuffers.o(.bss*);
        . = ALIGN(0x100);
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
    }
    END_SEG(behavior)

    /* 0x8016F000 21D7D0-255EC0 [386F0] */
    BEGIN_SEG(goddard, SEG_GODDARD)
    {
        BUILD_DIR/src/menu*.o(.text);
        BUILD_DIR/libgoddard.a:*.o(.text);
        /* data, rodata, per file */
        BUILD_DIR/src/menu*.o(.data*);
        BUILD_DIR/src/menu*.o(.rodata*);
        /* goddard subsystem data */
        BUILD_DIR/libgoddard.a:gd_main.o(.data*);
        BUILD_DIR/libgoddard.a:draw_objects.o(.data*);
        BUILD_DIR/libgoddard.a:objects.o(.data*);
        BUILD_DIR/libgoddard.a:particles.o(.data*);
        BUILD_DIR/libgoddard.a:dynlist_proc.o(.data*);
        BUILD_DIR/libgoddard.a:debug_utils.o(.data*);
        BUILD_DIR/libgoddard.a:joints.o(.data*);
        BUILD_DIR/libgoddard.a:shape_helper.o(.data*);
        BUILD_DIR/libgoddard.a:renderer.o(.data*);
        /* goddard subsystem rodata */
        BUILD_DIR/libgoddard.a:gd_main.o(.rodata*);
        BUILD_DIR/libgoddard.a:gd_memory.o(.rodata*);
        BUILD_DIR/libgoddard.a:draw_objects.o(.rodata*);
        BUILD_DIR/libgoddard.a:objects.o(.rodata*);
        BUILD_DIR/libgoddard.a:skin_movement.o(.rodata*);
        BUILD_DIR/libgoddard.a:particles.o(.rodata*);
        BUILD_DIR/libgoddard.a:dynlist_proc.o(.rodata*);
        BUILD_DIR/libgoddard.a:old_menu.o(.rodata*);
        BUILD_DIR/libgoddard.a:debug_utils.o(.rodata*);
        BUILD_DIR/libgoddard.a:joints.o(.rodata*);
        BUILD_DIR/libgoddard.a:skin.o(.rodata*);
        BUILD_DIR/libgoddard.a:gd_math.o(.rodata*);
        BUILD_DIR/libgoddard.a:shape_helper.o(.rodata*);
        BUILD_DIR/libgoddard.a:renderer.o(.rodata*);
    }
    END_SEG(goddard)
    BEGIN_NOLOAD(goddard)
    {
        BUILD_DIR/src/menu*.o(.bss*);
        BUILD_DIR/libgoddard.a:gd_main.o(.bss*);
        BUILD_DIR/libgoddard.a:gd_memory.o(.bss*);
        BUILD_DIR/libgoddard.a:sfx.o(.bss*);
        BUILD_DIR/libgoddard.a:draw_objects.o(.bss*);
        BUILD_DIR/libgoddard.a:objects.o(.bss*);
        BUILD_DIR/libgoddard.a:skin_movement.o(.bss*);
        BUILD_DIR/libgoddard.a:particles.o(.bss*);
        BUILD_DIR/libgoddard.a:dynlist_proc.o(.bss*);
        BUILD_DIR/libgoddard.a:old_menu.o(.bss*);
        BUILD_DIR/libgoddard.a:debug_utils.o(.bss*);
        BUILD_DIR/libgoddard.a:joints.o(.bss*);
        BUILD_DIR/libgoddard.a:skin.o(.bss*);
        BUILD_DIR/libgoddard.a:shape_helper.o(.bss*);
        BUILD_DIR/libgoddard.a:renderer.o(.bss*);
        BUILD_DIR/libgoddard.a:*.o(.bss*);
    }
    END_NOLOAD(goddard)

    /* 0x268020 0x268020-0 [0] */
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

    /* 271B20-2A42A0 [32780] */
    BEGIN_SEG(gd_dynlists, 0x04000000)
    {
        BUILD_DIR/libgoddard.a:dynlist_test_cube.o(.data);
        BUILD_DIR/libgoddard.a:dynlist_unused.o(.data);
        BUILD_DIR/libgoddard.a:dynlist_mario_face.o(.data);
        BUILD_DIR/libgoddard.a:dynlists_mario_eyes.o(.data);
        BUILD_DIR/libgoddard.a:dynlists_mario_eyebrows_mustache.o(.data);
        BUILD_DIR/libgoddard.a:dynlist_mario_master.o(.data);
        BUILD_DIR/libgoddard.a:anim_mario_mustache_right.o(.data);
        BUILD_DIR/libgoddard.a:anim_mario_mustache_left.o(.data);
        BUILD_DIR/libgoddard.a:anim_mario_lips_1.o(.data);
        BUILD_DIR/libgoddard.a:anim_mario_lips_2.o(.data);
        BUILD_DIR/libgoddard.a:anim_mario_eyebrows_1.o(.data);
        BUILD_DIR/libgoddard.a:anim_group_1.o(.data);
        BUILD_DIR/libgoddard.a:anim_group_2.o(.data);
        BUILD_DIR/libgoddard.a:dynlist_test_cube.o(.rodata*);
        BUILD_DIR/libgoddard.a:dynlist_unused.o(.rodata*);
        BUILD_DIR/libgoddard.a:*.o(.data);
        BUILD_DIR/libgoddard.a:*.o(.rodata);
    }
    END_SEG(gd_dynlists)

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

    MIO0_SEG(water_skybox, 0x0A000000)
    MIO0_SEG(ccm_skybox, 0x0A000000)
    MIO0_SEG(clouds_skybox, 0x0A000000)
    MIO0_SEG(bitfs_skybox, 0x0A000000)
    MIO0_SEG(wdw_skybox, 0x0A000000)
    MIO0_SEG(cloud_floor_skybox, 0x0A000000)
    MIO0_SEG(ssl_skybox, 0x0A000000)
    MIO0_SEG(bbh_skybox, 0x0A000000)
    MIO0_SEG(bidw_skybox, 0x0A000000)
    MIO0_SEG(bits_skybox, 0x0A000000)

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

    /* 4E9FA0-? [?] */
    BEGIN_SEG(assets, __romPos)
    {
        BUILD_DIR/assets/mario_anim_data.o(.data);
        BUILD_DIR/assets/mario_anim_data.o(.rodata);
        BUILD_DIR/assets/demo_data.o(.data);
        BUILD_DIR/sound/sound_data.o(.data);
    }
    END_SEG(assets)

    /* Discard everything not specifically mentioned above. */
    /DISCARD/ :
    {
        *(*);
    }

    ASSERT((_goddardSegmentNoloadEnd <= SEG_POOL_START + POOL_SIZE), "Error: menu segment extended past pool end")
    ASSERT((_mainSegmentNoloadEnd <= _engineSegmentStart), "Error: main segment extended into engine.")
    ASSERT((_engineSegmentNoloadEnd <= _framebuffersSegmentNoloadStart), "Error: engine segment extended into framebuffers.")
    ASSERT((_framebuffersSegmentNoloadEnd <= RAM_END), "Error: framebuffers segment extended past RDRAM end")
}
