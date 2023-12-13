#include <unordered_map>
#include "../image.h"

#ifndef PORTALCALENDAR_FONT_H
#define PORTALCALENDAR_FONT_H

struct FontGlyph : Image {
    FontGlyph(uint16_t width, uint16_t height, uint16_t top, uint16_t left, uint8_t rleBits, const uint8_t *data):
        Image(width, height, rleBits, data),
        top(top),
        left(left)
    { }
    const int16_t top;
    const int16_t left;
};

struct Font {
    const std::unordered_map<uint16_t, const FontGlyph> glyphs;
    const uint8_t fgColor;
    const uint8_t bgColor;
    const uint16_t ascent;
    const uint16_t descent;
    const uint16_t spaceWidth;

    const FontGlyph getGlyph(uint16_t cp) const
    {
        auto iterator = glyphs.find(cp);
        if (iterator != glyphs.cend()) {
            return iterator->second;
        }
        iterator = glyphs.find(0xFFFD);
        if (iterator != glyphs.cend()) {
            return iterator->second;
        }
        // No replacement glyph included in the font, but we still have to return something.
        // This code should never be hit.
        return glyphs.cbegin()->second;
    };
};

#endif // PORTALCALENDAR_FONT_H
