#ifndef GPU_PRIMS_H
#define GPU_PRIMS_H

#include "system.h"

// Colors
#define COLOR_RED   255, 0, 0
#define COLOR_BLUE  0, 0, 255

// Textures
static inline void LoadTexture(unsigned long *tim, TIM_IMAGE *tparam, int tx, int ty) {
    OpenTIM(tim);
    ReadTIM(tparam);
    tparam->prect->x = tx;
    tparam->prect->y = ty;
    LoadImage(tparam->prect, tparam->paddr);
    DrawSync(0);
}

// Primitives
static inline void Draw_Rect(int x, int y, int w, int h, int r, int g, int b, int z_index) {
    TILE *tile = (TILE *)nextpri;
    setTile(tile);
    setXY0(tile, x, y);
    setWH(tile, w, h);
    setRGB0(tile, r, g, b);
    addPrim(ot[db][z_index], tile);
    nextpri += sizeof(TILE);
}

static inline void Draw_Rect_SemiTrans(int x, int y, int w, int h, int r, int g, int b, int z_index) {
    TILE *tile = (TILE *)nextpri;
    setTile(tile);
    setXY0(tile, x, y);
    setWH(tile, w, h);
    setRGB0(tile, r, g, b);
    setSemiTrans(tile, 1);
    addPrim(ot[db][z_index], tile);
    nextpri += sizeof(TILE);
}

static inline void Draw_Line(int x0, int y0, int x1, int y1, int r, int g, int b, int z_index) {
    LINE_F2 *line = (LINE_F2 *)nextpri;
    setLineF2(line);
    setXY2(line, x0, y0, x1, y1);
    setRGB0(line, r, g, b);
    addPrim(ot[db][z_index], line);
    nextpri += sizeof(LINE_F2);
}

static inline void Draw_Sprite(int x, int y, int u, int v, int w, int h, int tpage_id, int z_index) {
    POLY_FT4 *poly = (POLY_FT4 *)nextpri;
    setPolyFT4(poly);
    setXY4(poly, x, y, x + w, y, x, y + h, x + w, y + h);
    setUV4(poly, u, v, u + w, v, u, v + h, u + w, v + h);
    poly->tpage = tpage_id;
    poly->clut = 0;
    setRGB0(poly, 128, 128, 128); // Neutral lighting
    setSemiTrans(poly, 0);
    addPrim(ot[db][z_index], poly);
    nextpri += sizeof(POLY_FT4);
}

#endif
