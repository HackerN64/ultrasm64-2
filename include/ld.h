#pragma once

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
