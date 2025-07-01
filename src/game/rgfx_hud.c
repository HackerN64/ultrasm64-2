#include <ultra64.h>
#include "rgfx_hud.h"

static RgfxHud sRgfxHudBuffer[RGFX_HUD_BUFFER_SIZE][RGFX_HUD_LAYERS];

#pragma GCC diagnostic ignored "-Wunused-parameter"

RgfxHud *rgfx_hud_create_box(RgfxHud *parent, s16 x, s16 y, s16 z, s16 sX, s16 sY) {
    return NULL;
}

RgfxHud *rgfx_hud_create_txt(RgfxHud *parent, s16 x, s16 y, s16 z, u8 font, char *c) {
    return NULL;
}

RgfxHud *rgfx_hud_create_sprite(RgfxHud *parent, s16 x, s16 y, s16 z, s16 sX, s16 sY, u8 fmt, Texture *sprite) {
    return NULL;
}

RgfxHud *rgfx_hud_create_scissor(RgfxHud *parent, s16 x, s16 y, s16 sX, s16 sY) {
    return NULL;
}

RgfxHud *rgfx_hud_create_gfx(RgfxHud *parent, s16 x, s16 y, s16 z, Gfx *gfx) {
    return NULL;
}

void rgfx_hud_draw() {
    return;
}
