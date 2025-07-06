#include <ultra64.h>
#include "init/memory.h"
#include "game_init.h"
#include "rgfx_hud.h"
#include "n64-libc.h"
#include "sm64.h"

#include "segment2.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

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

static RgfxHud sRgfxHudBuffer[RGFX_HUD_LAYERS][RGFX_HUD_BUFFER_SIZE];
static RgfxHud *sRgfxHudHead[RGFX_HUD_LAYERS] = { &sRgfxHudBuffer[0][0], &sRgfxHudBuffer[1][0], &sRgfxHudBuffer[2][0] };

static Texture *sRgfxClownfontCharMap[] = {

};

static Texture *sRgfxCursiveCharMap[] = {

};

static Texture *sRgfxCursiveHdCharMap[] = {

};

static Texture *sRgfxFasttextCharMap[] = {

};

static Texture *sRgfxInterCharMap[] = {
    NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL,
    /* \n */ NULL,
    NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    /* ! */
    /* " */
    /* # */
    /* $ */
    /* % */
    /* & */
    /* ' */
    /* ( */
    /* ) */
    /* * */
    /* + */
    /* , */
    /* - */
    /* . */
    /* / */
    /* 0 */
    /* 1 */
    /* 2 */
    /* 3 */
    /* 4 */
    /* 5 */
    /* 6 */
    /* 7 */
    /* 8 */
    /* 9 */
    /* : */
    /* ; */
    /* < */
    /* = */
    /* > */
    /* ? */
    /* @ */
    /* A */
    /* B */
    /* C */
    /* D */
    /* E */
    /* F */
    /* G */
    /* H */
    /* I */
    /* J */
    /* K */
    /* L */
    /* M */
    /* O */
    /* P */
    /* Q */
    /* R */
    /* S */
    /* T */
    /* U */
    /* V */
    /* W */
    /* X */
    /* Y */
    /* Z */
    /* [ */
    /* \ */ NULL,
    /* ] */
    /* ^ */
    /* _ */
    /* ` */
    /* a */
    /* b */
    /* c */
    /* d */
    /* e */
    /* f */
    /* g */
    /* h */
    /* i */
    /* j */
    /* k */
    /* l */
    /* m */
    /* o */
    /* p */
    /* q */
    /* r */
    /* s */
    /* t */
    /* u */
    /* v */
    /* w */
    /* x */
    /* y */
    /* z */
    /* { */
    /* | */
    /* } */
    /* ~ */
    /* DEL */ NULL
};

static RgfxFontProperties sRgfxFontProperties[RGFX_HUD_FONTS] = {
    { &sRgfxClownfontCharMap[0], { 16, 16 }, FALSE },
    { &sRgfxCursiveCharMap[0], { 8, 12 }, TRUE },
    { &sRgfxCursiveHdCharMap[0], { 8, 12 }, TRUE },
    { &sRgfxFasttextCharMap[0], { 8, 8 }, FALSE },
    { &sRgfxInterCharMap[0], { 32, 32 }, FALSE },
};

static RgfxHud *alloc_hud(u8 layer) {
    n64_assert((u32)sRgfxHudHead[layer] <= (u32)&sRgfxHudBuffer[layer] + RGFX_HUD_BUFFER_SIZE);
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

static inline f32 get_true_scale(RgfxHud *c, f32 s) {
    f32 ret = 1.0f;
    if (c->parent != NULL) {
        ret = get_true_scale(c->parent, s * c->scale);
    } else {
        ret = c->scale * s;
    }
    return ret;
}

static void apply_parent_transform(RgfxHud *c) {
    Mtx *matrix;
    if (c->parent != NULL) {
        apply_parent_transform(c->parent);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        guTranslate(matrix, c->x, -c->y, 0);
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        guRotate(matrix, (f32)c->roll / 182.0f, 0.0f, 0.0f, 1.0f);
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        guScale(matrix, get_true_scale(c, 1.0f), get_true_scale(c, 1.0f), get_true_scale(c, 1.0f));
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    } else {
        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        guTranslate(matrix, c->x + (c->d.box.sX / 2), SCREEN_HEIGHT - (c->y + (c->d.box.sY / 2)), 0);
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        guRotate(matrix, (f32)c->roll / 182.0f, 0.0f, 0.0f, 1.0f);
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        guScale(matrix, get_true_scale(c, 1.0f), get_true_scale(c, 1.0f), get_true_scale(c, 1.0f));
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    }
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

    gDPPipeSync(MASTERDL);
    gDPSetCombineLERP(MASTERDL, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT, 0, 0, 0, ENVIRONMENT);
    gDPSetEnvColor(MASTERDL, c->d.box.color[0], c->d.box.color[1], c->d.box.color[2], c->d.box.color[3]);
    if (is_2d_element(c) && !is_parent_3d(c)) { // we are using fillrect
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

        gDPFillRectangle(MASTERDL, x, y, x2, y2);
    } else { // we are using ortho triangles
        gDPSetCycleType(MASTERDL, G_CYC_1CYCLE);
        if (c->d.box.color[3] == 255) {
            gDPSetRenderMode(MASTERDL, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        } else {
            gDPSetRenderMode(MASTERDL, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        }

        Mtx *matrix = (Mtx *) alloc_display_list(sizeof(Mtx));

        guMtxIdent(matrix);
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

        apply_parent_transform(c);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        guOrtho(matrix, 0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -1000.0f, 1000.0f, 1.0f);
        gSPPerspNormalize(MASTERDL, 0xFFFF);
        gSPMatrix(MASTERDL, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);

        gSPClearGeometryMode(MASTERDL, G_LIGHTING);
        v = alloc_display_list(sizeof(Vtx) * 4);

        v[0].v.ob[0] = -(x - x2) / 2; // 0 top left
        v[0].v.ob[1] = -(y - y2) / 2;
        v[0].v.ob[2] = z;

        v[1].v.ob[0] = (x - x2) / 2; // 1 top right
        v[1].v.ob[1] = -(y - y2) / 2;
        v[1].v.ob[2] = z;

        v[2].v.ob[0] = -(x - x2) / 2; // 2 bottom left
        v[2].v.ob[1] = (y - y2) / 2;
        v[2].v.ob[2] = z;

        v[3].v.ob[0] = (x - x2) / 2; // 3 bottom right
        v[3].v.ob[1] = (y - y2) / 2;
        v[3].v.ob[2] = z;

        gSPVertex(MASTERDL, v, 4, 0);
        gSP2Triangles(MASTERDL, 1, 2, 0, 0, /* */ 2, 1, 3, 0);
        gSPPopMatrix(MASTERDL, G_MTX_PROJECTION);
        gSPPopMatrix(MASTERDL, G_MTX_MODELVIEW);
    }
    gDPPipeSync(MASTERDL);
    gDPSetEnvColor(MASTERDL, 255, 255, 255, 255);
    gSPDisplayList(MASTERDL, dl_hud_img_end);
}

s16 get_text_width(RgfxHud *c) {
    switch (c->d.txt.font) {

    }
}

// scale 1.0f is equal to a height of 16px
// if integer scaling is detected and we are NOT using 3d otherwise, fallback to texture rect

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
