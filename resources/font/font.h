#include <unordered_map>

#ifndef PORTALCALENDAR_FONT_H
#define PORTALCALENDAR_FONT_H

typedef struct {
    const uint16_t width;
    const uint16_t height;
    const int16_t top;
    const int16_t left;
    const uint8_t *data;
} FontGlyph;

typedef struct {
    const std::unordered_map<uint16_t, FontGlyph> glyphs;
    const uint8_t fgColor;
    const uint8_t bgColor;
    const uint16_t ascent;
    const uint16_t descent;
    const uint16_t spaceWidth;

    FontGlyph getGlyph(uint16_t cp) const
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
} Font;

#endif // PORTALCALENDAR_FONT_H
