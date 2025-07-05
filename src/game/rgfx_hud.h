#pragma once

#include "types.h"

#define RGFX_HUD_BUFFER_SIZE 128
#define RGFX_HUD_LAYERS      3

#define RGFX_HUD_FONTS 5

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
    RGFX_FONT_CURSIVE_HD,
    RGFX_FONT_FASTTEXT,
    RGFX_FONT_HELVETICA
} RgfxFont;

typedef struct {
    Texture **charMap;
    u8 size[2];
    u8 flipped; // vanilla cursive font
} RgfxFontProperties;

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
RgfxHud *rgfx_hud_create_txt(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, RgfxFont font, char *s);
RgfxHud *rgfx_hud_create_sprite(RgfxHud *parent,  u8 layer, s16 x, s16 y, s16 z, s16 sX, s16 sY, u8 fmt, Texture *sprite);
RgfxHud *rgfx_hud_create_scissor(RgfxHud *parent, u8 layer, s16 x, s16 y, s16 sX, s16 sY);
RgfxHud *rgfx_hud_create_gfx(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, Gfx *gfx);

// inline helper functions

static inline void rgfx_hud_set_color(RgfxHud *c, u8 r, u8 g, u8 b, u8 a) {
    if (c->type == RGFX_BOX) {
        c->d.box.color[0] = r; c->d.box.color[1] = g; c->d.box.color[2] = b; c->d.box.color[3] = a;
    } else if (c->type == RGFX_TEXT) {
        c->d.txt.color[0] = r; c->d.txt.color[1] = g; c->d.txt.color[2] = b; c->d.txt.color[3] = a;
    }
}

static inline void rgfx_hud_set_rotation(RgfxHud *c, s16 pitch, s16 yaw, s16 roll) {
    c->pitch = pitch; c->yaw = yaw; c->roll = roll;
}

void rgfx_hud_draw();
