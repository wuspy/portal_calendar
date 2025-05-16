#include <Arduino.h>
#include <SPI.h>
#include <utility>
#include <vector>
#include "qrcodegen.h"
#include "resources/image.h"
#include "resources/font/font.h"

#ifndef PORTALCALENDAR_FRAMEBUFFER_H
#define PORTALCALENDAR_FRAMEBUFFER_H

class FrameBuffer
{
private:
    static const uint8_t _ALIGN_LEFT     = 0b000001;
    static const uint8_t _ALIGN_TOP      = 0b000010;
    static const uint8_t _ALIGN_RIGHT    = 0b000100;
    static const uint8_t _ALIGN_BOTTOM   = 0b001000;
    static const uint8_t _ALIGN_VCENTER  = 0b010000;
    static const uint8_t _ALIGN_HCENTER  = 0b100000;

public:
    static const uint8_t NO_ALPHA = 0b100;

    enum Rotation: uint8_t {
        ROTATION_0,
        ROTATION_90,
        ROTATION_180,
        ROTATION_270,
    };

    enum Align: uint8_t {
        TOP_LEFT        = _ALIGN_LEFT | _ALIGN_TOP,
        TOP_CENTER      = _ALIGN_TOP | _ALIGN_HCENTER,
        TOP_RIGHT       = _ALIGN_TOP | _ALIGN_RIGHT,
        RIGHT_CENTER    = _ALIGN_RIGHT | _ALIGN_VCENTER,
        BOTTOM_RIGHT    = _ALIGN_BOTTOM | _ALIGN_RIGHT,
        BOTTOM_CENTER   = _ALIGN_BOTTOM | _ALIGN_HCENTER,
        BOTTOM_LEFT     = _ALIGN_BOTTOM | _ALIGN_LEFT,
        LEFT_CENTER     = _ALIGN_LEFT | _ALIGN_VCENTER,
        CENTER          = _ALIGN_HCENTER | _ALIGN_VCENTER,
    };

    enum Color: uint8_t {
        WHITE = 0b00,
        LGREY = 0b01,
        DGREY = 0b10,
        BLACK = 0b11,
    };

    uint8_t *data;
    inline size_t getLength() const { return _length; }

    FrameBuffer(uint32_t nativeWidth, uint32_t nativeHeight);
    ~FrameBuffer();
    void clear(Color color = WHITE);
    void test();
    void refresh();
    inline uint32_t getWidth() const { return _width; };
    inline uint32_t getHeight() const { return _height; };
    inline Rotation getRotation() const { return _rotation; };
    void setRotation(Rotation rotation);
    inline uint8_t getAlpha() const { return _rotation; };
    void setAlpha(uint8_t alpha);
    uint8_t getPx(int32_t x, int32_t y) const;
    void setPx(int32_t x, int32_t y, Color color);
    void drawImage(const Image &image, int32_t x, int32_t y, Align align = TOP_LEFT);
    uint32_t measureText(String str, const Font &font, int32_t tracking = 0);
    std::vector<String> wordWrap(String str, const Font &font, uint32_t maxLineLength, int32_t tracking = 0);
    void drawText(
        String str,
        const Font &font,
        int32_t x,
        int32_t y,
        Align align = TOP_LEFT,
        int32_t tracking = 0
    );
    void drawMultilineText(
        String str,
        const Font &font,
        int32_t x,
        int32_t y,
        uint32_t maxLineLength,
        Align align = TOP_LEFT,
        int32_t tracking = 0,
        int32_t leading = 0
    );
    void drawQrCode(qrcodegen::QrCode qrcode, int32_t x, int32_t y, int32_t scale = 1, Align align = TOP_LEFT);
    void drawVLine(int32_t x, int32_t y, int32_t length, uint32_t thickness, Color color, Align align = TOP_CENTER);
    void drawHLine(int32_t x, int32_t y, int32_t length, uint32_t thickness, Color color, Align align = LEFT_CENTER);
    void fillRect(int32_t x, int32_t y, int32_t width, int32_t height, Color color, Align align = TOP_LEFT);
    void strokeRect(
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height,
        uint32_t strokeWidth,
        Color color,
        bool strokeOutside = false,
        Align align = TOP_LEFT
    );

private:
    uint32_t _nativeWidth;
    uint32_t _nativeHeight;
    size_t _length;
    uint32_t _width;
    uint32_t _height;
    uint8_t _alpha;
    Rotation _rotation;

    static void adjustAlignment(int32_t *x, int32_t *y, int32_t width, int32_t height, Align align);
    size_t getPixelIndex(int32_t x, int32_t y) const;
};

#endif // PORTALCALENDAR_FRAMEBUFFER_H
