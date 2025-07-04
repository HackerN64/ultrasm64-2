#include <ultra64.h>
#include "init/memory.h"
#include "rgfx_hud.h"
#include "n64-string.h"
#ifdef ULTRASM64_2
#define RGFX_PRINTF n64_printf
#else
#define RGFX_PRINTF osSyncPrintf
#endif

static RgfxHud sRgfxHudBuffer[RGFX_HUD_LAYERS][RGFX_HUD_BUFFER_SIZE];
static RgfxHud *sRgfxHudHead[RGFX_HUD_LAYERS] = { &sRgfxHudBuffer[0][0], &sRgfxHudBuffer[1][0], &sRgfxHudBuffer[2][0] };

#pragma GCC diagnostic ignored "-Wunused-parameter"

// platform independent assert

static void debug_crash(char *s) {
    RGFX_PRINTF(s);
    while (TRUE) {}
}

static RgfxHud *alloc_hud(u8 layer) {
    if ((u32)sRgfxHudHead[layer] >= (u32)&sRgfxHudBuffer[layer] + RGFX_HUD_BUFFER_SIZE) {
        debug_crash("RGFX: Buffer too large.\n");
    }
    return sRgfxHudHead[layer]++;
}

static inline u8 is_2d_element(RgfxHud *c) {
    return c->z == 0 && c->pitch == 0 && c->yaw == 0 && c->roll == 0 && c->scale == 1.0f;
}

/*
 * if z == 0 && rotation == 0:
 * texture rect
 * else:
 * ortho
 *
 * if alpha == 255:
 * opaque rendermode
 * else:
 * transparent rendermode (no coverage)
 */

static inline void set_default_color(u8 *color) {
    for (u8 i = 0; i < 4; i++) {
        color[i] = 255;
    }
}

static inline void set_default_3d_settings(RgfxHud *c) {
    c->pitch = 0;
    c->yaw = 0;
    c->roll = 0;
    c->scale = 1.0f;
}

RgfxHud *rgfx_hud_create_box(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, s16 sX, s16 sY) {
    RgfxHud *c = alloc_hud(layer);
    c->type = RGFX_BOX;
    c->parent = parent;
    c->x = x;
    c->y = y;
    c->z = z;
    set_default_3d_settings(c);
    set_default_color(&c->d.box.color[0]);
    c->d.box.sX = sX;
    c->d.box.sY = sY;
    return c;
}

RgfxHud *rgfx_hud_create_txt(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, u8 font, char *s) {
    RgfxHud *c = alloc_hud(layer);
    c->type = RGFX_TEXT;
    c->parent = parent;
    c->x = x;
    c->y = y;
    c->z = z;
    set_default_3d_settings(c);
    set_default_color(&c->d.txt.color[0]);
    c->d.txt.c = alloc_display_list(n64_strlen(s) + 1);
    n64_strcpy(c->d.txt.c, s);
    return c;
}

RgfxHud *rgfx_hud_create_sprite(RgfxHud *parent,  u8 layer, s16 x, s16 y, s16 z, s16 sX, s16 sY, u8 fmt, Texture *sprite) {
    RgfxHud *c = alloc_hud(layer);
    c->type = RGFX_SPRITE;
    c->parent = parent;
    c->x = x;
    c->y = y;
    c->z = z;
    set_default_3d_settings(c);
    return c;
}

RgfxHud *rgfx_hud_create_scissor(RgfxHud *parent, u8 layer, s16 x, s16 y, s16 sX, s16 sY) {
    RgfxHud *c = alloc_hud(layer);
    c->type = RGFX_SCISSOR;
    c->parent = parent;
    c->x = x;
    c->y = y;
    c->d.box.sX = sX;
    c->d.box.sY = sY;
    return c;
}

RgfxHud *rgfx_hud_create_gfx(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, Gfx *gfx) {
    RgfxHud *c = alloc_hud(layer);
    c->type = RGFX_GFX;
    c->parent = parent;
    c->x = x;
    c->y = y;
    c->z = z;
    set_default_3d_settings(c);
    c->d.gfx.d = gfx;
    return c;
}

s16 get_true_x(RgfxHud *c) {
    return 0;
}

s16 get_true_y(RgfxHud *c) {
    return 0;
}

void rgfx_hud_draw() {
    for (u8 i = 0; i < RGFX_HUD_LAYERS; i++) {
        sRgfxHudHead[i]->type = RGFX_END;
        RgfxHud *c = &sRgfxHudBuffer[i][0];
        while (c->type != RGFX_END) {
            switch (c->type) {
                case RGFX_BOX:
                    if (is_2d_element(c)) {
                    } else {
                    }
                break;
                case RGFX_TEXT:
                    if (is_2d_element(c)) {
                    } else {
                    }
                break;
                case RGFX_SPRITE: break;
                    if (is_2d_element(c)) {
                    } else {
                    }
                break;
                case RGFX_SCISSOR: break;
                case RGFX_GFX: break;
                case RGFX_END: break; // avoids gcc warning
            }
            c++;
        }
    }
}
