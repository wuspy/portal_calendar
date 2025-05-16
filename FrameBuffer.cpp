#include <Arduino.h>
#include <stdlib.h>
#include "FrameBuffer.h"
#include "Utf8Iterator.h"

FrameBuffer::FrameBuffer(uint32_t nativeWidth, uint32_t nativeHeight)
{
    _nativeWidth = nativeWidth;
    _nativeHeight = nativeHeight;
    _length = nativeWidth * nativeHeight / 4;

    setRotation(ROTATION_0);
    setAlpha(NO_ALPHA);

    data = new uint8_t[_length];
    clear();
}

FrameBuffer::~FrameBuffer()
{
    delete[] data;
}

void FrameBuffer::clear(Color color)
{
    memset(data, (color << 6) | (color << 4) | (color << 2) | color, _length);
}

void FrameBuffer::test()
{
    memset(
        data,
        (BLACK << 6) | (BLACK << 4) | (BLACK << 2) | BLACK,
        _length / 4
    );
    memset(
        &data[_length / 4],
        (DGREY << 6) | (DGREY << 4) | (DGREY << 2) | DGREY,
        _length / 4
    );
    memset(
        &data[_length / 2],
        (LGREY << 6) | (LGREY << 4) | (LGREY << 2) | LGREY,
        _length / 4
    );
    memset(
        &data[_length * 3 / 4],
        (WHITE << 6) | (WHITE << 4) | (WHITE << 2) | WHITE,
        _length / 4
    );
}

size_t FrameBuffer::getPixelIndex(int32_t x, int32_t y) const
{
    if (x < 0 || x >= _width || y < 0 || y >= _height) {
        return SIZE_MAX;
    }
    switch (_rotation) {
        case ROTATION_90:
            std::swap(x, y);
            x = _nativeWidth - 1 - x;
            break;
        case ROTATION_180:
            x = _nativeWidth - 1 - x;
            y = _nativeHeight - 1 - y;
            break;
        case ROTATION_270:
            std::swap(x, y);
            y = _nativeHeight - 1 - y;
            break;
        default:
            break;
    }
    return _nativeWidth * y + x;
}

uint8_t FrameBuffer::getPx(int32_t x, int32_t y) const
{
    const size_t i = getPixelIndex(x, y);
    if (i != SIZE_MAX) {
        return (data[i / 4] >> ((3 - i % 4) * 2)) & 0b11;
    } else {
        return 0;
    }
}

void FrameBuffer::setPx(int32_t x, int32_t y, Color color)
{
    const size_t i = getPixelIndex(x, y);
    if (i != SIZE_MAX) {
        uint8_t *fb = &data[i / 4];
        const uint8_t shift = (3 - i % 4) * 2;
        *fb &= ~(0b11 << shift);
        *fb |= (color & 0b11) << shift;
    }
}

void FrameBuffer::drawImage(const Image &image, int32_t x, int32_t y, Align align)
{
    ImageReader reader = ImageReader(image);
    adjustAlignment(&x, &y, image.width, image.height, align);

    Color color;
    uint32_t y_dst;

    for (uint32_t y_src = 0; y_src < image.height; ++y_src) {
        y_dst = y + y_src;
        for (uint32_t x_src = 0; x_src < image.width; ++x_src) {
            color = static_cast<Color>(reader.next());    
            if (color != _alpha) {
                setPx(x + x_src, y_dst, color);
            }
        }
    }
}

uint32_t FrameBuffer::measureText(String str, const Font &font, int32_t tracking)
{
    if (str.length() == 0) {
        return 0;
    }

    uint32_t length = 0;
    Utf8Iterator it = Utf8Iterator(str);
    uint16_t cp;
    while ((cp = it.next())) {
        if (Utf8Iterator::isSpaceCodePoint(cp)) {
            length += font.spaceWidth + tracking;
        } else {
            const FontGlyph glyph = font.getGlyph(cp);
            length += glyph.width + glyph.left + tracking;
        }
    }
    return length - tracking;
}

std::vector<String> FrameBuffer::wordWrap(String str, const Font &font, uint32_t maxLineLength, int32_t tracking)
{
    std::vector<String> lines;

    if (str.length() == 0) {
        return lines;
    }

    unsigned int lineStart = 0, safeLineEnd = 0;
    uint32_t length = 0, safeLength = 0;
    Utf8Iterator it = Utf8Iterator(str);
    uint16_t cp;

    while ((cp = it.next())) {
        if (Utf8Iterator::isNewlineCodePoint(cp)) {
            if (maxLineLength > 0 && length > maxLineLength + tracking) {
                // Wrap at the last word too
                lines.push_back(str.substring(lineStart, safeLineEnd - 1));
                lineStart = safeLineEnd;
            }
            // Wrap here
            lines.push_back(str.substring(lineStart, it.getCurrentPosition() - 1));
            lineStart = safeLineEnd = it.getCurrentPosition();
            length = safeLength = 0;
        } else if (Utf8Iterator::isSpaceCodePoint(cp)) {
            if (maxLineLength > 0 && length > maxLineLength + tracking) {
                if (safeLineEnd == lineStart) {
                    // Line cannot be word wrapped, so wrap at current position
                    lines.push_back(str.substring(lineStart, it.getCurrentPosition() - 1));
                    lineStart = it.getCurrentPosition();
                    length = 0;
                } else {
                    // Wrap at last word
                    lines.push_back(str.substring(lineStart, safeLineEnd - 1));
                    lineStart = safeLineEnd;
                    length -= safeLength;
                }
            } else {
                length += font.spaceWidth + tracking;
            }
            safeLineEnd = it.getCurrentPosition();
            safeLength = length;
        } else {
            const FontGlyph glyph = font.getGlyph(cp);
            length += glyph.width + glyph.left + tracking;
        }
    }
    if (lineStart < str.length()) {
        lines.push_back(str.substring(lineStart));
    }
    return lines;
}

void FrameBuffer::drawText(String str, const Font &font, int32_t x, int32_t y, Align align, int32_t tracking)
{
    if (align != TOP_LEFT) {
        uint32_t width;
        // Measurement isn't needed and width isn't used by adjustAligment if horizontal alignment is left
        if (!(align & _ALIGN_LEFT)) {
            width = measureText(str, font);
        }
        adjustAlignment(&x, &y, width, font.ascent + font.descent, align);
    }
    Utf8Iterator it = Utf8Iterator(str);
    uint16_t cp;
    while ((cp = it.next())) {
        if (Utf8Iterator::isSpaceCodePoint(cp)) {
            x += font.spaceWidth + tracking;
        } else {
            const FontGlyph glyph = font.getGlyph(cp);
            x += glyph.left;
            drawImage(glyph, x, y + glyph.top);
            x += glyph.width + tracking;
        }
    }
}

void FrameBuffer::drawMultilineText(
    String str,
    const Font &font,
    int32_t x,
    int32_t y,
    uint32_t maxLineLength,
    Align align,
    int32_t tracking,
    int32_t leading
) {
    // This implementation is simple because it assumes justification equals the horizontal alignment,
    // and that's all I needed it to do.
    leading += font.ascent + font.descent;
    std::vector<String> lines = wordWrap(str, font, maxLineLength, tracking);

    if (!(align & _ALIGN_TOP)) {
        adjustAlignment(&x, &y, 0, leading * lines.size(), align);
    }

    // Use top alignment for each line since that axis has already been adjusted for the whole block
    align = (Align)(align & ~_ALIGN_BOTTOM & ~_ALIGN_VCENTER | _ALIGN_TOP);
    for (String str : lines) {
        drawText(str, font, x, y, align, tracking);
        y += leading;
    }
}

void FrameBuffer::drawQrCode(qrcodegen::QrCode qrcode, int32_t x, int32_t y, int32_t scale, Align align)
{
    const int32_t size = qrcode.getSize() * scale;
    adjustAlignment(&x, &y, size, size, align);

    Color color;
    int32_t y2;
    for(int y1 = 0; y1 < size; ++y1) {
        y2 = y1 / scale;
        for (int x1 = 0; x1 < size; ++x1) {
            setPx(x + x1, y + y1, qrcode.getModule(x1 / scale, y2) ? BLACK : WHITE);
        }
    }
}

void FrameBuffer::drawHLine(int32_t x, int32_t y, int32_t length, uint32_t thickness, Color color, Align align)
{
    if (length < 0) {
        x += length;
        length = -length;
    }
    adjustAlignment(&x, &y, length, thickness, align);
    const int32_t ymax = y + thickness;
    int32_t i, y1;
    for (i = 0; i < length; ++i) {
        for (y1 = y; y1 < ymax; ++y1) {
            setPx(x + i, y1, color);
        }
    }
}

void FrameBuffer::drawVLine(int32_t x, int32_t y, int32_t length, uint32_t thickness, Color color, Align align)
{
    if (length < 0) {
        y += length;
        length = -length;
    }
    adjustAlignment(&x, &y, thickness, length, align);
    const int32_t xmax = x + thickness;
    int32_t i, x1;
    for (i = 0; i < length; ++i) {
        for (x1 = x; x1 < xmax; ++x1) {
            setPx(x1, y + i, color);
        }
    }
}

void FrameBuffer::fillRect(int32_t x, int32_t y, int32_t width, int32_t height, Color color, Align align)
{
    if (width < 0) {
        x += width;
        width = -width;
    }
    if (height < 0) {
        y += height;
        height = -height;
    }
    adjustAlignment(&x, &y, width, height, align);
    int32_t x2 = x + width, y2 = y + height;
    for (int xi = x; xi < x2; ++xi) {
        for (int yi = y; yi < y2; ++yi) {
            setPx(xi, yi, color);
        }
    }
}

void FrameBuffer::strokeRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t strokeWidth, Color color, bool strokeOutside, Align align)
{
    if (width < 0) {
        x += width;
        width = -width;
    }
    if (height < 0) {
        y += height;
        height = -height;
    }
    adjustAlignment(&x, &y, width, height, align);
    if (strokeOutside) {
        x -= strokeWidth;
        y -= strokeWidth;
        width += strokeWidth * 2;
        height += strokeWidth * 2;
    }
    drawHLine(x, y, width, strokeWidth, color, TOP_LEFT);
    drawVLine(x, y, height, strokeWidth, color, TOP_LEFT);
    drawHLine(x, y + height, width, strokeWidth, color, BOTTOM_LEFT);
    drawVLine(x + width, y, height, strokeWidth, color, TOP_RIGHT);
}

void FrameBuffer::adjustAlignment(int32_t *x, int32_t *y, int32_t width, int32_t height, Align align)
{
    if (align & _ALIGN_HCENTER) {
        *x -= width / 2;
    } else if (align & _ALIGN_RIGHT) {
        *x -= width;
    }

    if (align & _ALIGN_VCENTER) {
        *y -= height / 2;
    } else if (align & _ALIGN_BOTTOM) {
        *y -= height;
    }
}

void FrameBuffer::setRotation(Rotation rotation)
{
    _rotation = rotation;
    if (rotation == ROTATION_0 || rotation == ROTATION_180) {
        _width = _nativeWidth;
        _height = _nativeHeight;
    } else {
        _width = _nativeHeight;
        _height = _nativeWidth;
    }
}

void FrameBuffer::setAlpha(uint8_t alpha)
{
    _alpha = alpha;
}
