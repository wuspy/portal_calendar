#ifndef FONT_H
#define FONT_H

typedef struct FontGlyph
{
    const uint16_t width;
    const uint16_t height;
    const int16_t top;
    const int16_t left;
    const uint8_t *data;
};

typedef struct Font
{
    const char rangeStart;
    const char rangeEnd;
    const uint8_t fgColor;
    const uint8_t bgColor;
    const uint16_t ascent;
    const uint16_t descent;
    const uint16_t spaceWidth;
    const FontGlyph *glyphs;
};

#endif // FONT_H