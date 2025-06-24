#pragma once

typedef enum {
    RGFX_BOX,
    RGFX_TEXT,
    RGFX_SPRITE,
    RGFX_SCISSOR,
    RGFX_GFX
} RgfxType;

typedef enum {
    RGFX_FONT_CLOWNFONT,
    RGFX_FONT_CURSIVE,
    RGFX_FONT_FASTTEXT,
    RGFX_FONT_HELVETICA
} RgfxFont;

typedef struct {
    s16 sX, sY;
    u8 color[4];
} RgfxBox;

typedef struct {
    char *c;
    u8 color[4];
    RgfxFont font;
    u16 fontProperties;
} RgfxText;

typedef struct {
    void *t;
} RgfxSprite;

typedef struct {
    Gfx *d;
} RgfxGfx;

typedef union {
    RgfxBox     box;
    RgfxText    txt;
    RgfxSprite  spt;
    RgfxGfx     gfx;
} RgfxHudData;

typedef struct {
    RgfxType type;        // type
    s16 x, y, z;          // position
    s16 pitch, yaw, roll; // pitch yaw roll
    f32 scale;            // size
    void *parent;         // parent
    RgfxHudData d;
} RgfxHud;
