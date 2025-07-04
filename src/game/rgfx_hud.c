#include <ultra64.h>
#include "init/memory.h"
#include "game_init.h"
#include "rgfx_hud.h"
#include "n64-string.h"
#include "n64-stdio.h"
#include "sm64.h"

#include "segment2.h"

/*
 * RGFXHUD 5
 * Features 3D transforms for menu elements.
 */

#define ULTRASM64_2

#ifdef ULTRASM64_2
#define RGFX_PRINTF n64_printf
#else
#define RGFX_PRINTF osSyncPrintf
#endif

#define ABS(d)   ((d) > 0) ? (d) : -(d)

#define CURR_BOX_COLOR       c->d.box.color[0], c->d.box.color[1], c->d.box.color[2]
#define CURR_BOX_COLOR_ALPHA c->d.box.color[0], c->d.box.color[1], c->d.box.color[2], c->d.box.color[3]

static RgfxHud sRgfxHudBuffer[RGFX_HUD_LAYERS][RGFX_HUD_BUFFER_SIZE];
static RgfxHud *sRgfxHudHead[RGFX_HUD_LAYERS] = { &sRgfxHudBuffer[0][0], &sRgfxHudBuffer[1][0], &sRgfxHudBuffer[2][0] };

#pragma GCC diagnostic ignored "-Wunused-parameter"

// platform independent assert

static inline void debug_crash(char *s) {
    RGFX_PRINTF(s);
    CRASH;
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

static inline u8 is_parent_3d(RgfxHud *c) {
    u8 ret;
    if (c->parent != NULL) {
        ret = is_parent_3d(c->parent);
    } else {
        ret = !is_2d_element(c);
    }
    return ret;
}

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

RgfxHud *rgfx_hud_create_txt(RgfxHud *parent,     u8 layer, s16 x, s16 y, s16 z, RgfxFont font, char *s) {
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

static inline s16 get_true_x(RgfxHud *c, s16 x) {
    s16 ret = 0;
    if (c->parent != NULL) {
        ret = get_true_x(c->parent, x + c->x);
    } else {
        ret = c->x + x;
    }
    return ret;
}

static inline s16 get_true_y(RgfxHud *c, s16 y) {
    s16 ret = 0;
    if (c->parent != NULL) {
        ret = get_true_y(c->parent, y + c->y);
    } else {
        ret = c->y + y;
    }
    return ret;
}

static inline s16 get_true_z(RgfxHud *c, s16 z) {
    s16 ret = 0;
    if (c->parent != NULL) {
        ret = get_true_z(c->parent, z + c->z);
    } else {
        ret = c->z + z;
    }
    return ret;
}

static void rgfx_draw_box(RgfxHud *c) {
    s16 x = get_true_x(c, 0);
    s16 y = get_true_y(c, 0);
    s16 z = get_true_z(c, 0);
    s16 x2 = x + c->d.box.sX;
    s16 y2 = y + c->d.box.sY;
    s16 t;
    Vtx *v;

    // swap x/y with x2/y2 so that x/y is always < x2/y2

    if (x2 < x) {
        t = x2;
        x2 = x;
        x = t;
    }

    if (y2 < y) {
        t = x2;
        x2 = x;
        x = t;
    }

    // don't go out of bounds

    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (x2 > SCREEN_WIDTH) {
        x2 = SCREEN_WIDTH;
    }

    if (y2 > SCREEN_HEIGHT) {
        y2 = SCREEN_HEIGHT;
    }
    gDPPipeSync(MASTERDL);
    gDPSetCombineLERP(MASTERDL, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
    if (is_2d_element(c) && !is_parent_3d(c)) { // we are using fillrect
        if (c->d.box.color[3] == 255 && ABS(x - x2) % 4) { // fill cycle
            gDPSetCycleType(MASTERDL, G_CYC_FILL);
            gDPSetRenderMode(MASTERDL, G_RM_NOOP, G_RM_NOOP);
            gDPSetFillColor(MASTERDL, (GPACK_RGBA5551(c->d.box.color[0], c->d.box.color[1], c->d.box.color[2], 1) << 16) | GPACK_RGBA5551(c->d.box.color[0], c->d.box.color[1], c->d.box.color[2], 1));
            x2 -= 1;
            y2 -= 1;
        } else { // 1 cycle
            gDPSetCycleType(MASTERDL, G_CYC_1CYCLE);
            if (c->d.box.color[3] == 255) {
                gDPSetRenderMode(MASTERDL, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            } else {
                gDPSetRenderMode(MASTERDL, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            }
        }
        gDPSetEnvColor(MASTERDL, c->d.box.color[0], c->d.box.color[1], c->d.box.color[2], c->d.box.color[3]);
        gDPFillRectangle(MASTERDL, x, y, x2, y2);
    } else { // we are using ortho triangles
        v = alloc_display_list(sizeof(Vtx) * 4);
    }
    gDPPipeSync(MASTERDL);
    gDPSetEnvColor(MASTERDL, 255, 255, 255, 255);
    gSPDisplayList(MASTERDL, dl_hud_img_end);
}

static void rgfx_draw_text(RgfxHud *c) {
    if (is_2d_element(c)) { // we are using texture rectangles
    } else { // we are using ortho triangles
    }
}

static void rgfx_draw_sprite(RgfxHud *c) {
    if (is_2d_element(c)) { // we are using texture rectangles
    } else { // we are using ortho triangles
    }
}

static void rgfx_draw_scissor(RgfxHud *c) {

}

static void rgfx_draw_gfx(RgfxHud *c) {

}

void rgfx_hud_draw() {
    for (u8 i = 0; i < RGFX_HUD_LAYERS; i++) {
        sRgfxHudHead[i]->type = RGFX_END;
        RgfxHud *c = &sRgfxHudBuffer[i][0];
        while (c->type != RGFX_END) {
            switch (c->type) {
                case RGFX_BOX:      rgfx_draw_box(c);     break;
                case RGFX_TEXT:     rgfx_draw_text(c);    break;
                case RGFX_SPRITE:   rgfx_draw_sprite(c);  break;
                case RGFX_SCISSOR:  rgfx_draw_scissor(c); break;
                case RGFX_GFX:      rgfx_draw_gfx(c);     break;
                case RGFX_END:                            break; // avoids gcc warning
            }
            c++;
        }
        sRgfxHudHead[i] = &sRgfxHudBuffer[i][0];
    }
}
