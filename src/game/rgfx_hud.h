#pragma once

#include "types.h"

#define RGFX_HUD_BUFFER_SIZE 128
#define RGFX_HUD_LAYERS      3

typedef enum {
    RGFX_BOX,
    RGFX_TEXT,
    RGFX_SPRITE,
    RGFX_SCISSOR,
    RGFX_GFX,
    RGFX_END
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

RgfxHud *rgfx_hud_create_box(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, s16 sX, s16 sY);
RgfxHud *rgfx_hud_create_txt(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, u8 font, char *s);
RgfxHud *rgfx_hud_create_sprite(RgfxHud *parent,  u8 layer, s16 x, s16 y, s16 z, s16 sX, s16 sY, u8 fmt, Texture *sprite);
RgfxHud *rgfx_hud_create_scissor(RgfxHud *parent, u8 layer, s16 x, s16 y, s16 sX, s16 sY);
RgfxHud *rgfx_hud_create_gfx(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, Gfx *gfx);
void rgfx_hud_draw();
