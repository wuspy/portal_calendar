#include <SPI.h>
#include <utility>
#include <vector>
#include "qrcodegen.h"
#include "resources/image.h"
#include "resources/font/font.h"

#ifndef PORTALCALENDAR_DISPLAY_GDEW075T7_H
#define PORTALCALENDAR_DISPLAY_GDEW075T7_H

class DisplayGDEW075T7
{
private:
    static const uint8_t _ALIGN_LEFT     = 0b000001;
    static const uint8_t _ALIGN_TOP      = 0b000010;
    static const uint8_t _ALIGN_RIGHT    = 0b000100;
    static const uint8_t _ALIGN_BOTTOM   = 0b001000;
    static const uint8_t _ALIGN_VCENTER  = 0b010000;
    static const uint8_t _ALIGN_HCENTER  = 0b100000;

public:
    static const uint32_t NATIVE_WIDTH = 800;
    static const uint32_t NATIVE_HEIGHT = 480;
    static const uint32_t NUM_PIXELS = NATIVE_WIDTH * NATIVE_HEIGHT;
    static const uint32_t FRAMEBUFFER_LENGTH = NUM_PIXELS / 4;

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

    DisplayGDEW075T7(uint8_t spi_bus, uint8_t sck_pin, uint8_t copi_pin, uint8_t cs_pin, uint8_t reset_pin, uint8_t dc_pin, uint8_t busy_pin, uint8_t pwr_pin);
    ~DisplayGDEW075T7();
    void clear(Color color = WHITE);
    void test();
    void refresh();
    uint32_t getWidth();
    uint32_t getHeight();
    Rotation getRotation();
    void setRotation(Rotation rotation);
    uint8_t getAlpha();
    void setAlpha(uint8_t alpha);
    uint8_t getPx(int32_t x, int32_t y);
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
    uint8_t _resetPin;
    uint8_t _dcPin;
    uint8_t _csPin;
    uint8_t _busyPin;
    uint8_t _pwrPin;
    uint32_t _width;
    uint32_t _height;
    uint8_t _alpha;
    Rotation _rotation;
    uint8_t *_frameBuffer;
    SPIClass *_spi;

    void wakeup();
    void sleep();
    void setLut(uint8_t cmd, const uint8_t* lut);
    void sendCommand(uint8_t command);
    void sendData(uint8_t data);
    void waitUntilIdle();
    void adjustAlignment(int32_t *x, int32_t *y, int32_t width, int32_t height, Align align);
    size_t getPixelIndex(int32_t x, int32_t y);
};

#endif // PORTALCALENDAR_DISPLAY_GDEW075T7_H
