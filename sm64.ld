OUTPUT_ARCH (mips)

/* include/segments.h defines SEG_POOL_START, SEG_POOL_END, SEG_BUFFERS,
 * SEG_GODDARD, SEG_ENGINE, SEG_FRAMEBUFFERS */
#include "segments.h"
#include "config.h"

#ifndef USE_EXT_RAM
#define ADDR_OR_ALIGN(addr) (addr)
#else
#define ADDR_OR_ALIGN(addr) ALIGN(0x1000)
#endif

#if defined(VERSION_US) || defined(VERSION_JP)
#define AUDIO_DIR BUILD_DIR/src/audio/us_jp
#elif defined(VERSION_EU)
#define AUDIO_DIR BUILD_DIR/src/audio/eu
#elif defined(VERSION_SH) || defined(VERSION_CN)
#define AUDIO_DIR BUILD_DIR/src/audio/sh
#endif

#define BEGIN_SEG(name, addr) \
    _##name##SegmentStart = ADDR(.name); \
    _##name##SegmentRomStart = __romPos; \
    .name (addr) : AT(__romPos)

#define END_SEG(name) \
    _##name##SegmentEnd = ADDR(.name) + SIZEOF(.name); \
    _##name##SegmentRomEnd = __romPos + SIZEOF(.name); \
    __romPos += SIZEOF(.name);

#define BEGIN_NOLOAD(name) \
    _##name##SegmentNoloadStart = ADDR(.name.noload); \
    .name.noload (NOLOAD) :

#define END_NOLOAD(name) \
    _##name##SegmentNoloadEnd = ADDR(.name.noload) + SIZEOF(.name.noload);

#define MIO0_SEG(name, segAddr) \
    BEGIN_SEG(name##_mio0, segAddr) \
    { \
        BUILD_DIR/bin/name.mio0.o(.data); \
        . = ALIGN(0x10); \
    } \
    END_SEG(name##_mio0)

#define MIO0_EU_SEG(name, segAddr) \
    BEGIN_SEG(name##_mio0, segAddr) \
    { \
        BUILD_DIR/bin/eu/name.mio0.o(.data); \
        . = ALIGN(0x10); \
    } \
    END_SEG(name##_mio0)

#define STANDARD_LEVEL(name) \
    BEGIN_SEG(name##_segment_7, 0x07000000) \
    { \
        BUILD_DIR/levels/name/leveldata.mio0.o(.data); \
        . = ALIGN(0x10); \
    } \
    END_SEG(name##_segment_7) \
    BEGIN_SEG(name, 0x0E000000) \
    { \
        BUILD_DIR/levels/name/script.o(.data); \
        BUILD_DIR/levels/name/geo.o(.data); \
    } \
    END_SEG(name)

#define STANDARD_OBJECTS(name, segAddr, geoAddr) \
    BEGIN_SEG(name##_mio0, segAddr) \
    { \
        BUILD_DIR/actors/name.mio0.o(.data); \
        . = ALIGN(0x10); \
    } \
    END_SEG(name##_mio0) \
    BEGIN_SEG(name##_geo, geoAddr) \
    { \
        BUILD_DIR/actors/name##_geo.o(.data); \
    } \
    END_SEG(name##_geo)

#define CREATE_LO_HI_PAIR(name, value) \
    name##Hi = (value) >> 16; \
    name##Lo = (value) & 0xffff;

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

#ifdef VERSION_CN
    gZBufferEnd = .;
#endif

    . = SEG_BUFFERS;
    BEGIN_NOLOAD(buffers)
    {
        BUILD_DIR/src/buffers/buffers.o(.bss*);
#if defined(VERSION_SH) || defined(VERSION_CN)
        AUDIO_DIR/port.o(.bss*);
#else
        AUDIO_DIR/globals_start.o(.bss*);
        AUDIO_DIR/synthesis.o(.bss*);
        AUDIO_DIR/heap.o(.bss*);
        AUDIO_DIR/load.o(.bss*);
        AUDIO_DIR/data.o(.bss*);
#endif

#ifdef VERSION_EU
        . = ALIGN(0x200);
#else
        . = ALIGN(0x1000);
#endif
#if defined(VERSION_SH) || defined(VERSION_CN)
        . += 0xB000;
#endif
        BUILD_DIR/src/buffers/gfx_output_buffer.o(.bss*);
    }
    END_NOLOAD(buffers)

    /* lib/src/__osDevMgrMain.c and lib/src/osCreateViManager.c contain infinite
     * loops compiled without -g, which cause the return statements and the .o
     * files themselves to be aligned to 32-byte boundaries. But the linker
     * Nintendo used did not respect .o file alignment, and aligned the files
     * only to 16 bytes, in some cases misaligning them. We force the same to
     * happen using the SUBALIGN directive. This is harmless; the alignment is
     * just an optimization. */
    BEGIN_SEG(main, )
#ifndef VERSION_CN
        SUBALIGN(16)
#endif
    {
        BUILD_DIR/asm/entry.o(.text);
        BUILD_DIR/src/game/crash_screen.o(.text);
        BUILD_DIR/src/game/main.o(.text);
        BUILD_DIR/src/game/game_init.o(.text);
#ifdef VERSION_CN
        . += 0x10;
#endif
        BUILD_DIR/src/game/sound_init.o(.text);
#if ENABLE_RUMBLE
        BUILD_DIR/src/game/rumble_init.o(.text);
#endif
        BUILD_DIR/src/game/level_update.o(.text);
        BUILD_DIR/src/game/interaction.o(.text);
        BUILD_DIR/src/game/mario.o(.text);
        BUILD_DIR/src/game/mario_step.o(.text);
        BUILD_DIR/src/game/mario_actions_cutscene.o(.text);
        BUILD_DIR/src/game/mario_actions_automatic.o(.text);
        BUILD_DIR/src/game/mario_actions_stationary.o(.text);
        BUILD_DIR/src/game/mario_actions_moving.o(.text);
        BUILD_DIR/src/game/mario_actions_airborne.o(.text);
        BUILD_DIR/src/game/mario_actions_submerged.o(.text);
        BUILD_DIR/src/game/mario_actions_object.o(.text);
        BUILD_DIR/src/game/level_geo.o(.text);
        BUILD_DIR/src/game/mario_misc.o(.text);
        BUILD_DIR/src/game/memory.o(.text);
        BUILD_DIR/src/game/save_file.o(.text);
        BUILD_DIR/src/game/area.o(.text);
        BUILD_DIR/src/game/rendering_graph_node.o(.text);
        BUILD_DIR/src/game/profiler.o(.text);
        BUILD_DIR/asm/decompress.o(.text);
        BUILD_DIR/src/game/camera.o(.text);
        BUILD_DIR/src/game/debug_course.o(.text);
        BUILD_DIR/src/game/object_list_processor.o(.text);
#ifndef VERSION_EU
        BUILD_DIR/src/game/object_helpers.o(.text);
#endif
        BUILD_DIR/src/game/behavior_actions.o(.text);
        BUILD_DIR/src/game/platform_displacement.o(.text);
#ifndef VERSION_EU
        BUILD_DIR/src/game/object_collision.o(.text);
        BUILD_DIR/src/game/spawn_object.o(.text);
#endif
        BUILD_DIR/src/game/spawn_sound.o(.text);
        BUILD_DIR/src/game/debug.o(.text);
        BUILD_DIR/src/game/screen_transition.o(.text);
        BUILD_DIR/src/game/shadow.o(.text);
        BUILD_DIR/src/game/skybox.o(.text);
        BUILD_DIR/src/game/moving_texture.o(.text);
        BUILD_DIR/src/game/geo_misc.o(.text);
        BUILD_DIR/src/game/paintings.o(.text);
        BUILD_DIR/src/game/print.o(.text);
        BUILD_DIR/src/game/ingame_menu.o(.text);
        BUILD_DIR/src/game/envfx_snow.o(.text);
        BUILD_DIR/src/game/envfx_bubbles.o(.text);
        BUILD_DIR/src/game/macro_special_objects.o(.text);
        BUILD_DIR/src/game/hud.o(.text);
        BUILD_DIR/src/game/obj_behaviors.o(.text);
        BUILD_DIR/src/game/obj_behaviors_2.o(.text);
        AUDIO_DIR/synthesis.o(.text);
        AUDIO_DIR/heap.o(.text);
        AUDIO_DIR/load.o(.text);
#if defined(VERSION_SH) || defined(VERSION_CN)
        AUDIO_DIR/port.o(.text);
#endif
        AUDIO_DIR/playback.o(.text);
        AUDIO_DIR/effects.o(.text);
        AUDIO_DIR/seqplayer.o(.text);
#ifndef LIBULTRA_EXCLUSIVE
#if defined(VERSION_SH) || defined(VERSION_CN)
        BUILD_DIR/libultra.a:driverominit.o(.text);
#endif
#endif
        BUILD_DIR/src/audio/external.o(.text);
#ifdef VERSION_EU
        AUDIO_DIR/port.o(.text);
#endif

#define SECTION text
#ifdef VERSION_JP
// JP has an additional 0x40 of weird padding space in text
#define JP_PADDING_TEXT
#endif
        #include "lib/ultra/ld.inc"
#undef SECTION
#ifdef VERSION_JP
#undef JP_PADDING_TEXT
#endif

#ifndef __sgi
        BUILD_DIR/libgcc.a:_divdi3.o(.text);
        BUILD_DIR/libgcc.a:_moddi3.o(.text);
        BUILD_DIR/libgcc.a:_udivdi3.o(.text);
        BUILD_DIR/libgcc.a:_umoddi3.o(.text);
        BUILD_DIR/libgcc.a:_cmpdi2.o(.text);
        BUILD_DIR/libgcc.a:_floatdidf.o(.text);
        BUILD_DIR/libgcc.a:_floatdisf.o(.text);
        BUILD_DIR/libgcc.a:_fixunsdfdi.o(.text);
        BUILD_DIR/libgcc.a:_fixdfdi.o(.text);
        BUILD_DIR/libgcc.a:_fixunssfdi.o(.text);
        BUILD_DIR/libgcc.a:_fixsfdi.o(.text);
#endif
        BUILD_DIR/lib/rsp.o(.text);

        /* data */
        BUILD_DIR/src/game/crash_screen.o(.data*);
        BUILD_DIR/src/game/main.o(.data*);
        BUILD_DIR/src/game/game_init.o(.data*);
        BUILD_DIR/src/game/sound_init.o(.data*);
#if ENABLE_RUMBLE
        BUILD_DIR/src/game/rumble_init.o(.data*);
#endif
        BUILD_DIR/src/game/level_update.o(.data*);
        BUILD_DIR/src/game/interaction.o(.data*);
        BUILD_DIR/src/game/mario.o(.data*);
        BUILD_DIR/src/game/mario_step.o(.data*);
        BUILD_DIR/src/game/mario_actions_cutscene.o(.data*);
        BUILD_DIR/src/game/mario_actions_moving.o(.data*);
        BUILD_DIR/src/game/mario_actions_submerged.o(.data*);
        BUILD_DIR/src/game/mario_actions_object.o(.data*);
        BUILD_DIR/src/game/mario_misc.o(.data*);
        BUILD_DIR/src/game/memory.o(.data*);
        BUILD_DIR/src/game/save_file.o(.data*);
        BUILD_DIR/src/game/area.o(.data*);
        BUILD_DIR/src/game/rendering_graph_node.o(.data*);
        BUILD_DIR/src/game/profiler.o(.data*);
        BUILD_DIR/src/game/camera.o(.data*);
        BUILD_DIR/src/game/object_list_processor.o(.data*);
#ifndef VERSION_EU
        BUILD_DIR/src/game/object_helpers.o(.data*);
#endif
        BUILD_DIR/src/game/behavior_actions.o(.data*);
        BUILD_DIR/src/game/platform_displacement.o(.data*);
        BUILD_DIR/src/game/spawn_sound.o(.data*);
        BUILD_DIR/src/game/debug.o(.data*);
        BUILD_DIR/src/game/screen_transition.o(.data*);
        BUILD_DIR/src/game/shadow.o(.data*);
        BUILD_DIR/src/game/skybox.o(.data*);
        BUILD_DIR/src/game/moving_texture.o(.data*);
        BUILD_DIR/src/game/geo_misc.o(.data*);
        BUILD_DIR/src/game/paintings.o(.data*);
        BUILD_DIR/src/game/print.o(.data*);
        BUILD_DIR/src/game/ingame_menu.o(.data*);
        BUILD_DIR/src/game/envfx_snow.o(.data*);
        BUILD_DIR/src/game/envfx_bubbles.o(.data*);
        BUILD_DIR/src/game/macro_special_objects.o(.data*);
        BUILD_DIR/src/game/hud.o(.data*);
        BUILD_DIR/src/game/obj_behaviors.o(.data*);
        BUILD_DIR/src/game/obj_behaviors_2.o(.data*);
#ifndef VERSION_EU
        /* wildcard doesn't match on EU due to files being moved to engine/ */
        BUILD_DIR/src/game*.o(.data*);
#endif
        AUDIO_DIR/synthesis.o(.data*);
        AUDIO_DIR/heap.o(.data*);
#if !defined(VERSION_SH) && !defined(VERSION_CN)
        AUDIO_DIR/load.o(.data*);
#endif
        AUDIO_DIR/playback.o(.data*);
        AUDIO_DIR/effects.o(.data*);
        AUDIO_DIR/seqplayer.o(.data*);
#if defined(VERSION_SH) || defined(VERSION_CN)
        AUDIO_DIR/data.o(.data*);
        AUDIO_DIR/debug_prints.o(.data*);
        AUDIO_DIR/port.o(.data*);
#endif
        BUILD_DIR/src/audio/external.o(.data*);
#if defined(VERSION_SH) || defined(VERSION_CN)
        AUDIO_DIR/audio_session_presets.o(.data*);
#else
#ifdef VERSION_EU
        AUDIO_DIR/port.o(.data*);
#endif
        AUDIO_DIR/data.o(.data*);
#endif

#define SECTION data*
        #include "lib/ultra/ld.inc"
#undef SECTION

        /* rodata */
        BUILD_DIR/src/game/crash_screen.o(.rodata*);
        BUILD_DIR/src/game/main.o(.rodata*);
        BUILD_DIR/src/game/game_init.o(.rodata*);
        BUILD_DIR/src/game/sound_init.o(.rodata*);
        BUILD_DIR/src/game/rumble_init.o(.rodata*);
        BUILD_DIR/src/game/level_update.o(.rodata*);
        BUILD_DIR/src/game/interaction.o(.rodata*);
        BUILD_DIR/src/game/mario.o(.rodata*);
        BUILD_DIR/src/game/mario_step.o(.rodata*);
        BUILD_DIR/src/game/mario_actions_cutscene.o(.rodata*);
        BUILD_DIR/src/game/mario_actions_automatic.o(.rodata*);
        BUILD_DIR/src/game/mario_actions_stationary.o(.rodata*);
        BUILD_DIR/src/game/mario_actions_moving.o(.rodata*);
        BUILD_DIR/src/game/mario_actions_airborne.o(.rodata*);
        BUILD_DIR/src/game/mario_actions_submerged.o(.rodata*);
        BUILD_DIR/src/game/mario_actions_object.o(.rodata*);
        BUILD_DIR/src/game/mario_misc.o(.rodata*);
        BUILD_DIR/src/game/memory.o(.rodata*);
        BUILD_DIR/src/game/area.o(.rodata*);
        BUILD_DIR/src/game/rendering_graph_node.o(.rodata*);
        BUILD_DIR/src/game/camera.o(.rodata*);
        BUILD_DIR/src/game/object_list_processor.o(.rodata*);
#ifndef VERSION_EU
        BUILD_DIR/src/game/object_helpers.o(.rodata*);
#endif
        BUILD_DIR/src/game/behavior_actions.o(.rodata*);
        BUILD_DIR/src/game/platform_displacement.o(.rodata*);
#ifndef VERSION_EU
        BUILD_DIR/src/game/object_collision.o(.rodata*);
        BUILD_DIR/src/game/spawn_object.o(.rodata*);
#endif
        BUILD_DIR/src/game/spawn_sound.o(.rodata*);
        BUILD_DIR/src/game/debug.o(.rodata*);
        BUILD_DIR/src/game/screen_transition.o(.rodata*);
        BUILD_DIR/src/game/shadow.o(.rodata*);
        BUILD_DIR/src/game/skybox.o(.rodata*);
        BUILD_DIR/src/game/moving_texture.o(.rodata*);
        BUILD_DIR/src/game/geo_misc.o(.rodata*);
        BUILD_DIR/src/game/paintings.o(.rodata*);
        BUILD_DIR/src/game/print.o(.rodata*);
        BUILD_DIR/src/game/ingame_menu.o(.rodata*);
        BUILD_DIR/src/game/envfx_snow.o(.rodata*);
        BUILD_DIR/src/game/envfx_bubbles.o(.rodata*);
        BUILD_DIR/src/game/macro_special_objects.o(.rodata*);
        BUILD_DIR/src/game/hud.o(.rodata*);
        BUILD_DIR/src/game/obj_behaviors.o(.rodata*);
        BUILD_DIR/src/game/obj_behaviors_2.o(.rodata*);
#if defined(VERSION_JP) || defined(VERSION_US)
        BUILD_DIR/src/game*.o(.rodata*);
#endif
        AUDIO_DIR/synthesis.o(.rodata*);
        AUDIO_DIR/heap.o(.rodata*);
        AUDIO_DIR/load.o(.rodata*);
#if defined(VERSION_SH) || defined(VERSION_CN)
        AUDIO_DIR/port.o(.rodata*);
#endif
        AUDIO_DIR/playback.o(.rodata*);
        AUDIO_DIR/effects.o(.rodata*);
        AUDIO_DIR/seqplayer.o(.rodata*);
        BUILD_DIR/src/audio/external.o(.rodata*);
#ifdef VERSION_EU
        AUDIO_DIR/port.o(.rodata*);
#endif
        AUDIO_DIR*.o(.rodata*);

#define SECTION rodata*
        #include "lib/ultra/ld.inc"
#undef SECTION

#ifndef __sgi
        BUILD_DIR/libgcc.a:_divdi3.o(.rodata*);
        BUILD_DIR/libgcc.a:_moddi3.o(.rodata*);
        BUILD_DIR/libgcc.a:_udivdi3.o(.rodata*);
        BUILD_DIR/libgcc.a:_umoddi3.o(.rodata*);
        BUILD_DIR/libgcc.a:_cmpdi2.o(.rodata*);
        BUILD_DIR/libgcc.a:_floatdidf.o(.rodata*);
        BUILD_DIR/libgcc.a:_floatdisf.o(.rodata*);
        BUILD_DIR/libgcc.a:_fixunsdfdi.o(.rodata*);
        BUILD_DIR/libgcc.a:_fixdfdi.o(.rodata*);
        BUILD_DIR/libgcc.a:_fixunssfdi.o(.rodata*);
        BUILD_DIR/libgcc.a:_fixsfdi.o(.rodata*);
#endif
        /* audio blobs, should really be moved into a separate file */
#if defined(VERSION_SH) || defined(VERSION_CN)
        AUDIO_DIR/load.o(.data*);
#endif
        BUILD_DIR/lib/rsp.o(.rodata*);
    }
    END_SEG(main)

    BEGIN_NOLOAD(main)
    {
        BUILD_DIR/src/game/cn_common_syms_1.o(.bss*);
        BUILD_DIR/src/game/crash_screen.o(.bss*);
        BUILD_DIR/src/game/main.o(.bss*);
        BUILD_DIR/src/game/game_init.o(.bss*);
        BUILD_DIR/src/game/sound_init.o(.bss*);
#if ENABLE_RUMBLE
        BUILD_DIR/src/game/rumble_init.o(.bss*);
#endif
        BUILD_DIR/src/game/level_update.o(.bss*);
        BUILD_DIR/src/game/interaction.o(.bss*);
        BUILD_DIR/src/game/mario.o(.bss*);
        BUILD_DIR/src/game/mario_actions_cutscene.o(.bss*);
        BUILD_DIR/src/game/mario_actions_moving.o(.bss*);
        BUILD_DIR/src/game/mario_actions_submerged.o(.bss*);
        BUILD_DIR/src/game/mario_misc.o(.bss*);
        BUILD_DIR/src/game/memory.o(.bss*);
        BUILD_DIR/src/game/save_file.o(.bss*);
        BUILD_DIR/src/game/area.o(.bss*);
        BUILD_DIR/src/game/rendering_graph_node.o(.bss*);
        BUILD_DIR/src/game/profiler.o(.bss*);
        BUILD_DIR/src/game/camera.o(.bss*);
        BUILD_DIR/src/game/object_list_processor.o(.bss*);
#ifndef VERSION_EU
        BUILD_DIR/src/game/object_helpers.o(.bss*);
#endif
        BUILD_DIR/src/game/behavior_actions.o(.bss*);
        BUILD_DIR/src/game/debug.o(.bss*);
        BUILD_DIR/src/game/shadow.o(.bss*);
        BUILD_DIR/src/game/skybox.o(.bss*);
        BUILD_DIR/src/game/moving_texture.o(.bss*);
        BUILD_DIR/src/game/geo_misc.o(.bss*);
        BUILD_DIR/src/game/paintings.o(.bss*);
        BUILD_DIR/src/game/print.o(.bss*);
        BUILD_DIR/src/game/ingame_menu.o(.bss*);
        BUILD_DIR/src/game/envfx_snow.o(.bss*);
        BUILD_DIR/src/game/envfx_bubbles.o(.bss*);
        BUILD_DIR/src/game/macro_special_objects.o(.bss*);
        BUILD_DIR/src/game/hud.o(.bss*);
        BUILD_DIR/src/game/obj_behaviors.o(.bss*);
        BUILD_DIR/src/game/obj_behaviors_2.o(.bss*);
#if !defined(VERSION_EU) && !defined(VERSION_CN)
        BUILD_DIR/src/game*.o(.bss*);
#endif
        BUILD_DIR/src/audio/external.o(.bss*);
#ifdef VERSION_EU
        AUDIO_DIR/port.o(.bss*);
#endif
#if defined(VERSION_SH) || defined(VERSION_CN)
        AUDIO_DIR/globals_start.o(.bss*);
        AUDIO_DIR/synthesis.o(.bss*);
        AUDIO_DIR/heap.o(.bss*);
        AUDIO_DIR/load.o(.bss*);
        AUDIO_DIR/data.o(.bss*);
#endif

#define SECTION bss*
#ifdef VERSION_SH
// osPfsIsPlug bss section is located slightly different in Shindou.
#define SH_PFS_BSS
#endif
        #include "lib/ultra/ld.inc"
#undef SECTION
#ifdef VERSION_SH
#undef SH_PFS_BSS
#endif

        BUILD_DIR/src/game/cn_common_syms_2.o(.bss*);

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

    BEGIN_SEG(engine, ADDR_OR_ALIGN(SEG_ENGINE))
    {
        BUILD_DIR/src/engine/math_util.o(.text);
        BUILD_DIR/src/engine/graph_node.o(.text);
        BUILD_DIR/src/engine/graph_node_manager.o(.text);
        BUILD_DIR/src/engine/geo_layout.o(.text);
        BUILD_DIR/src/engine/level_script.o(.text);
        BUILD_DIR/src/engine/surface_collision.o(.text);
        BUILD_DIR/src/engine/surface_load.o(.text);
        BUILD_DIR/src/engine/behavior_script.o(.text);
#ifdef VERSION_EU
        BUILD_DIR/src/game/object_collision.o(.text);
        BUILD_DIR/src/game/spawn_object.o(.text);
        BUILD_DIR/src/game/object_helpers.o(.text);
#endif
        BUILD_DIR/src/engine*.o(.text);
        /* data */
        BUILD_DIR/src/engine/graph_node.o(.data*);
        BUILD_DIR/src/engine/graph_node_manager.o(.data*);
        BUILD_DIR/src/engine/math_util.o(.data*);
        BUILD_DIR/src/engine/geo_layout.o(.data*);
        BUILD_DIR/src/engine/level_script.o(.data*);
        BUILD_DIR/src/engine/behavior_script.o(.data*);
#ifdef VERSION_EU
        BUILD_DIR/src/game/object_collision.o(.data*);
        BUILD_DIR/src/game/spawn_object.o(.data*);
        BUILD_DIR/src/game/object_helpers.o(.data*);
#endif
        BUILD_DIR/src/engine*.o(.data*);
        /* rodata */
        BUILD_DIR/src/engine/math_util.o(.rodata*);
        BUILD_DIR/src/engine/level_script.o(.rodata*);
        BUILD_DIR/src/engine/surface_collision.o(.rodata*);
        BUILD_DIR/src/engine/surface_load.o(.rodata*);
#ifdef VERSION_EU
        BUILD_DIR/src/game/object_collision.o(.rodata*);
        BUILD_DIR/src/game/spawn_object.o(.rodata*);
        BUILD_DIR/src/game/object_helpers.o(.rodata*);
#endif
#ifdef VERSION_CN
        BUILD_DIR/src/engine/stub.o(.rodata*);
#endif
        BUILD_DIR/src/engine*.o(.rodata*);
    }
    END_SEG(engine)
    BEGIN_NOLOAD(engine)
    {
        BUILD_DIR/src/engine/math_util.o(.bss*);
        BUILD_DIR/src/engine/geo_layout.o(.bss*);
        BUILD_DIR/src/engine/level_script.o(.bss*);
        BUILD_DIR/src/engine/surface_collision.o(.bss*);
        BUILD_DIR/src/engine/surface_load.o(.bss*);
        BUILD_DIR/src/engine/behavior_script.o(.bss*);
#ifdef VERSION_EU
        BUILD_DIR/src/game/object_collision.o(.bss*);
        BUILD_DIR/src/game/spawn_object.o(.bss*);
        BUILD_DIR/src/game/object_helpers.o(.bss*);
#endif
        BUILD_DIR/src/engine*.o(.bss*);
    }
    END_NOLOAD(engine)

    . = ADDR_OR_ALIGN(SEG_FRAMEBUFFERS);
    BEGIN_NOLOAD(framebuffers)
    {
        BUILD_DIR/src/buffers/framebuffers.o(.bss*);
    }
    END_NOLOAD(framebuffers)

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
        BUILD_DIR/src/menu/title_screen.o(.text);
        BUILD_DIR/src/menu/intro_geo.o(.text);
        BUILD_DIR/src/menu/file_select.o(.text);
        BUILD_DIR/src/menu/star_select.o(.text);
        BUILD_DIR/src/menu*.o(.text);
        BUILD_DIR/libgoddard.a:gd_main.o(.text);
        BUILD_DIR/libgoddard.a:gd_memory.o(.text);
        BUILD_DIR/libgoddard.a:sfx.o(.text);
        BUILD_DIR/libgoddard.a:draw_objects.o(.text);
        BUILD_DIR/libgoddard.a:objects.o(.text);
        BUILD_DIR/libgoddard.a:skin_movement.o(.text);
        BUILD_DIR/libgoddard.a:particles.o(.text);
        BUILD_DIR/libgoddard.a:dynlist_proc.o(.text);
        BUILD_DIR/libgoddard.a:old_menu.o(.text);
        BUILD_DIR/libgoddard.a:debug_utils.o(.text);
        BUILD_DIR/libgoddard.a:joints.o(.text);
        BUILD_DIR/libgoddard.a:skin.o(.text);
        BUILD_DIR/libgoddard.a:gd_math.o(.text);
        BUILD_DIR/libgoddard.a:shape_helper.o(.text);
        BUILD_DIR/libgoddard.a:renderer.o(.text);
        BUILD_DIR/libgoddard.a:*.o(.text);
        /* data, rodata, per file */
        BUILD_DIR/src/menu/title_screen.o(.data*);
        BUILD_DIR/src/menu/title_screen.o(.rodata*);
        BUILD_DIR/src/menu/intro_geo.o(.data*);
        BUILD_DIR/src/menu/file_select.o(.data*);
        BUILD_DIR/src/menu/file_select.o(.rodata*);
        BUILD_DIR/src/menu/star_select.o(.data*);
        BUILD_DIR/src/menu/star_select.o(.rodata*);
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
        BUILD_DIR/src/menu/intro_geo.o(.bss*);
        BUILD_DIR/src/menu/file_select.o(.bss*);
        BUILD_DIR/src/menu/star_select.o(.bss*);
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

    ASSERT((_goddardSegmentNoloadEnd <= SEG_POOL_END), "Error: menu segment extended past pool end")
    ASSERT((_mainSegmentNoloadEnd <= _engineSegmentStart), "Error: main segment extended into engine.")
    ASSERT((_engineSegmentNoloadEnd <= _framebuffersSegmentNoloadStart), "Error: engine segment extended into framebuffers.")
    ASSERT((_framebuffersSegmentNoloadEnd <= SEG_RAM_END), "Error: framebuffers segment extended past RDRAM end")
}
