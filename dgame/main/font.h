
#ifndef __FONT_H__
#define __FONT_H__

#include "main.h"

typedef struct tagGLYPH
{
    uint16_t wWidth, wHeight;
    uint8_t g[8];
} GLYPH, *LPGLYPH;

typedef struct tagFONT
{
    uint16_t      wStartChar; // chars range start
    uint16_t      wEndChar;   // range end
    const GLYPH*  Glyphs;    // array of glyphs
    uint16_t      height;     // height of the font
} FONT;

const FONT& getSystemFont();

#endif // __FONT_H__