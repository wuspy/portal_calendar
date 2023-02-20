/**
 * Display class supporting 4-level greyscale for GDEW075T7 e-paper display, aka the 800x480 7.5" B/W from Waveshare.
 * 
 * https://www.good-display.com/product/244.html
 * 
 * Datasheet (for a different display controller) that has the LUT commands documented:
 * https://www.smart-prototyping.com/image/data/9_Modules/EinkDisplay/GDEW0154T8/IL0373.pdf
 */

#include <Arduino.h>
#include <stdlib.h>
#include "DisplayGDEW075T7.h"

// Display commands
const uint8_t CMD_PSR           = 0x00;
const uint8_t CMD_PWR           = 0x01;
const uint8_t CMD_POWEROFF      = 0x02;
const uint8_t CMD_POWERON       = 0x04;
const uint8_t CMD_BTST          = 0x06;
const uint8_t CMD_DEEPSLEEP     = 0x07;
const uint8_t CHK_DEEPSLEEP     = 0xA5;
const uint8_t CMD_DTM1          = 0x10;
const uint8_t CMD_REFRESH       = 0x12;
const uint8_t CMD_DSP           = 0x11;
const uint8_t CMD_DTM2          = 0x13;
const uint8_t CMD_DUAL_SPI      = 0x15;
const uint8_t CMD_SET_LUTVCOM   = 0x20;
const uint8_t CMD_SET_LUTWW     = 0x21;
const uint8_t CMD_SET_LUTBW     = 0x22;
const uint8_t CMD_SET_LUTWB     = 0x23;
const uint8_t CMD_SET_LUTBB     = 0x24;
const uint8_t CMD_SET_LUTBD     = 0x25;
const uint8_t CMD_PLL           = 0x30;
const uint8_t CMD_VCOM_CDI      = 0x50;
const uint8_t CMD_TCON          = 0x60;
const uint8_t CMD_TRES          = 0x61;
const uint8_t CMD_GSST          = 0x65;
const uint8_t CMD_FLG           = 0x71;
const uint8_t CMD_VDCS          = 0x82;

// Supported LUT voltage levels
#define LEVEL_GND           0b00
#define LEVEL_VDH           0b01
#define LEVEL_VDL           0b10
#define LEVEL_VDHR          0b11
#define LEVEL_VCOM_VCMDC    0b00
#define LEVEL_VCOM_FLOAT    0b11

/**
 * Macro to generate an LUT row while preserving some readability.
 * 
 * l* params are the voltages to apply at each step, and t* are the number of frames to keep that voltage applied.
 * r is the number of times the enitre sequence is to be repeated.
 */
#define LUT_ROW(l0, t0, l1, t1, l2, t2, l3, t3, r) (l0 << 6) | (l1 << 4) | (l2 << 2) | l3, t0, t1, t2, t3, r
#define LUT_ROW_NOOP LUT_ROW(0, 0, 0, 0, 0, 0, 0, 0, 0)

/**
 * These LUTs are the original ones provided for this display by Waveshare.
 * 
 * Currently not used, but kept here for reference.
 * 
 * https://github.com/waveshare/e-Paper/blob/master/Arduino/epd7in5_V2/epd7in5_V2.cpp
 */

const uint8_t LUT_VCOM_1BIT[] = {
    LUT_ROW(LEVEL_VCOM_VCMDC, 15, LEVEL_VCOM_VCMDC, 15, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VCOM_VCMDC, 15, LEVEL_VCOM_VCMDC, 1, LEVEL_VCOM_VCMDC, 15, LEVEL_VCOM_VCMDC, 1, 2),
    LUT_ROW(LEVEL_VCOM_VCMDC, 15, LEVEL_VCOM_VCMDC, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_WW_1BIT[] = {
    LUT_ROW(LEVEL_GND, 15, LEVEL_VDH, 15, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, 15, LEVEL_GND, 1, LEVEL_VDH, 15, LEVEL_GND, 1, 2),
    LUT_ROW(LEVEL_GND, 15, LEVEL_VDL, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_BW_1BIT[] = {
    LUT_ROW(LEVEL_GND, 15, LEVEL_VDH, 15, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, 15, LEVEL_GND, 1, LEVEL_VDH, 15, LEVEL_GND, 1, 2),
    LUT_ROW(LEVEL_GND, 15, LEVEL_VDL, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_WB_1BIT[] = {
    LUT_ROW(LEVEL_VDL, 15, LEVEL_GND, 15, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, 15, LEVEL_GND, 1, LEVEL_VDH, 15, LEVEL_GND, 1, 2),
    LUT_ROW(LEVEL_VDH, 15, LEVEL_GND, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_BB_1BIT[] = {
    LUT_ROW(LEVEL_VDL, 15, LEVEL_GND, 15, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, 15, LEVEL_GND, 1, LEVEL_VDH, 15, LEVEL_GND, 1, 2),
    LUT_ROW(LEVEL_VDH, 15, LEVEL_GND, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

/**
 * These LUTs are from the GxEPD2 project, they enable support for 4 color greyscale.
 * The author of that project says these waveforms are provided by GoodDisplay, however
 * they aren't meant for this particular display and may have varying results between displays,
 * although they look perfect on the one I have.
 * 
 * The only modification I've made is repeating the 2nd waveform twice to reduce ghosting.
 * I don't forsee this having any negative effects since that waveform is balanced between VDH/VDL.
 * Ghosting seems to increase as temperature decreases, which makes sense because the oil in the display
 * will be more viscous. Increasing R2 will help reduce ghosting more at low temperatures at the cost of
 * increated refresh time.
 * 
 * https://github.com/ZinggJM/GxEPD2_4G/blob/master/src/epd/GxEPD2_750_T7.cpp
 */

#define T1 10
#define T2 20
#define R2 2
#define T3_1 20
#define T3_2 10

const uint8_t LUT_VCOM_2BIT[] = {
    LUT_ROW(LEVEL_VCOM_VCMDC, T1, 0, 0, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDH, T2, LEVEL_VDL, T2, 0, 0, 0, 0, R2),
    LUT_ROW(LEVEL_VCOM_VCMDC, T3_1, 0, 0, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VCOM_VCMDC, 19, LEVEL_VCOM_VCMDC, 10, LEVEL_VCOM_VCMDC, 1, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_WHITE_2BIT[] = {
    LUT_ROW(LEVEL_VDH, T1, 0, 0, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, T2, LEVEL_VDH, T2, 0, 0, 0, 0, R2),
    LUT_ROW(LEVEL_GND, T3_1, LEVEL_VDH, T3_2, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, 19, LEVEL_VDL, 1, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_DGREY_2BIT[] = {
    LUT_ROW(LEVEL_VDH, T1, 0, 0, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, T2, LEVEL_VDH, T2, 0, 0, 0, 0, R2),
    LUT_ROW(LEVEL_GND, T3_1, LEVEL_GND, T3_2, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, 12, LEVEL_VDH, 1, LEVEL_VDL, 3, LEVEL_VDH, 4, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_LGREY_2BIT[] = {
	LUT_ROW(LEVEL_VDH, T1, 0, 0, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, T2, LEVEL_VDH, T2, 0, 0, 0, 0, R2),
    LUT_ROW(LEVEL_GND, T3_1, LEVEL_GND, T3_2, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, 11, LEVEL_VDH, 4, LEVEL_VDL, 4, LEVEL_VDH, 1, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_BLACK_2BIT[] = {
    LUT_ROW(LEVEL_VDL, T1, 0, 0, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDL, T2, LEVEL_VDH, T2, 0, 0, 0, 0, R2),
    LUT_ROW(LEVEL_GND, T3_1, LEVEL_VDL, T3_2, 0, 0, 0, 0, 1),
    LUT_ROW(LEVEL_VDH, 19, LEVEL_VDH, 1, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

/**
 * These aren't waveform LUTs, they map 2-bit color values to the old (DTM1) and new (DTM2)
 * data values sent to the display.
 */

const uint8_t LUT_DTM1[] = {
    /* WHITE */ 0,
    /* LGREY */ 0,
    /* DGREY */ 1,
    /* BLACK */ 1,
};

const uint8_t LUT_DTM2[] = {
    /* WHITE */ 0,
    /* LGREY */ 1,
    /* DGREY */ 0,
    /* BLACK */ 1,
};

#define BUSY_TIMEOUT 5000

DisplayGDEW075T7::~DisplayGDEW075T7() {
    _spi->endTransaction();
    _spi->end();
    delete _spi;
    delete[] _frameBuffer;
};

DisplayGDEW075T7::DisplayGDEW075T7(uint8_t spi_bus, uint8_t sck_pin, uint8_t copi_pin, uint8_t cs_pin, uint8_t reset_pin, uint8_t dc_pin, uint8_t busy_pin)
{
    _spiBus = spi_bus;
    _resetPin = reset_pin;
    _dcPin = dc_pin;
    _csPin = cs_pin;
    _busyPin = busy_pin;

    pinMode(_csPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);
    pinMode(_dcPin, OUTPUT);
    pinMode(_busyPin, INPUT);

    _spi = new SPIClass(_spiBus);
    _spi->begin(sck_pin, -1, copi_pin, cs_pin);
    _spi->beginTransaction(SPISettings(7000000, MSBFIRST, SPI_MODE0));
    
    _frameBuffer = new uint8_t[FRAMEBUFFER_LENGTH];

    setRotation(ROTATION_0);
    setAlpha(NO_ALPHA);

    clear();
};

void DisplayGDEW075T7::wakeup()
{
    digitalWrite(_resetPin, HIGH);
    delay(20); 
    digitalWrite(_resetPin, LOW);
    delay(4);
    digitalWrite(_resetPin, HIGH);
    delay(20);

    digitalWrite(_csPin, LOW);

    // Most of this is Waveshare's defaults

    sendCommand(CMD_PWR);   // power setting
    sendData(0x17);         // BD_EN=1, VSR_EN=1, VS_EN=1, VG_EN=1, waveshare
    sendData(0x17);         // VPP_EN=0, VCOM_SLEW=1, VGH=20v, VGL=-20v
    sendData(0x3F);         // VDH=15v
    sendData(0x3F);         // VDL=-15v
    sendData(0x11);         // VDHR=5.8v
  	
    sendCommand(CMD_VDCS);  // VCOM DC Setting (min 0x00 = -0.1v, max 0x4F = -4.05v)
    sendData(0x26);         // -2.0v
  
    sendCommand(CMD_BTST);  // Booster Setting
    sendData(0x27);
    sendData(0x27);
    sendData(0x2F);
    sendData(0x17);
  	
    sendCommand(CMD_PLL);
    sendData(0x06);         // 150hz

    sendCommand(CMD_POWERON);
    delay(100);
    waitUntilIdle();

    sendCommand(CMD_PSR);   // PANNEL SETTING
    sendData(0x3F);         // Custom LUT, KW mode, UD=1, SHL=1, SHD_N=1, RST_N=1

    sendCommand(CMD_TRES);
    sendData(NATIVE_WIDTH / 256);
    sendData(NATIVE_WIDTH % 256);
    sendData(NATIVE_HEIGHT / 256);
    sendData(NATIVE_HEIGHT % 256);

    sendCommand(CMD_DUAL_SPI);
    sendData(0x00);

    sendCommand(CMD_VCOM_CDI);  // VCOM AND DATA INTERVAL SETTING
    sendData(0x00);             // BDZ=0, BDV=00, N2OCP=0, DDX=00
    // sendData(0x00);             // CDI=17
    sendData(0x07);             // CDI=10

    sendCommand(CMD_TCON);      // TCON SETTING
    sendData(0x22);

    sendCommand(CMD_GSST);  // Resolution setting
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);
    sendData(0x00);

    setLut(CMD_SET_LUTVCOM, LUT_VCOM_2BIT);
    setLut(CMD_SET_LUTWW, LUT_WHITE_2BIT);
    setLut(CMD_SET_LUTBW, LUT_DGREY_2BIT);
    setLut(CMD_SET_LUTWB, LUT_LGREY_2BIT);
    setLut(CMD_SET_LUTBB, LUT_BLACK_2BIT);
    setLut(CMD_SET_LUTBD, LUT_WHITE_2BIT);
}

void DisplayGDEW075T7::sendCommand(uint8_t command)
{
    digitalWrite(_dcPin, LOW);
    _spi->transfer(command);
    digitalWrite(_dcPin, HIGH);
}

void DisplayGDEW075T7::sendData(uint8_t data)
{
    _spi->transfer(data);
}

void DisplayGDEW075T7::waitUntilIdle()
{
    unsigned long start = millis();
    do {
        delay(5);
    } while (digitalRead(_busyPin) == LOW && millis() - start < BUSY_TIMEOUT);
    delay(20);
}

void DisplayGDEW075T7::setLut(uint8_t cmd, const uint8_t* lut)
{
    sendCommand(cmd);	
    for (size_t i = 0; i < 42; ++i) {
        sendData(lut[i]);
    }
}

void DisplayGDEW075T7::refresh()
{
    wakeup();

    uint32_t i, j;
    uint16_t chunk; // Holds 8px of frame_buffer
    uint8_t data;   // Holds 8px of display output

    sendCommand(CMD_DTM1);
    for (i = 0; i < FRAMEBUFFER_LENGTH; i += 2) {
        data = 0;
        chunk = (_frameBuffer[i] << 8) | _frameBuffer[i + 1];
        for (uint8_t j = 0; j < 8; ++j) {
            data |= LUT_DTM1[(chunk >> (j * 2)) & 0b11] << j;
        }
        sendData(data);
    }
    sendCommand(CMD_DTM2);
    for (i = 0; i < FRAMEBUFFER_LENGTH; i += 2) {
        data = 0;
        chunk = (_frameBuffer[i] << 8) | _frameBuffer[i + 1];
        for (uint8_t j = 0; j < 8; ++j) {
            data |= LUT_DTM2[(chunk >> (j * 2)) & 0b11] << j;
        }
        sendData(data);
    }

    sendCommand(CMD_REFRESH);
    delay(100);
    waitUntilIdle();
    sleep();
}

void DisplayGDEW075T7::sleep()
{
    sendCommand(CMD_POWEROFF);
    waitUntilIdle();
    sendCommand(CMD_DEEPSLEEP);
    sendData(CHK_DEEPSLEEP);
    digitalWrite(_csPin, HIGH);
}

void DisplayGDEW075T7::clear(uint8_t color)
{
    memset(_frameBuffer, (color << 6) | (color << 4) | (color << 2) | color, FRAMEBUFFER_LENGTH);
}

void DisplayGDEW075T7::test()
{
    memset(
        _frameBuffer,
        (BLACK << 6) | (BLACK << 4) | (BLACK << 2) | BLACK,
        FRAMEBUFFER_LENGTH / 4
    );
    memset(
        &_frameBuffer[FRAMEBUFFER_LENGTH / 4],
        (DGREY << 6) | (DGREY << 4) | (DGREY << 2) | DGREY,
        FRAMEBUFFER_LENGTH / 4
    );
    memset(
        &_frameBuffer[FRAMEBUFFER_LENGTH / 2],
        (LGREY << 6) | (LGREY << 4) | (LGREY << 2) | LGREY,
        FRAMEBUFFER_LENGTH / 4
    );
    memset(
        &_frameBuffer[FRAMEBUFFER_LENGTH * 3 / 4],
        (WHITE << 6) | (WHITE << 4) | (WHITE << 2) | WHITE,
        FRAMEBUFFER_LENGTH / 4
    );
}

size_t DisplayGDEW075T7::getPixelIndex(int32_t x, int32_t y)
{
    if (x < 0 || x > _width || y < 0 || y > _height) {
        return SIZE_MAX;
    }
    switch (_rotation) {
        case ROTATION_90:
            std::swap(x, y);
            x = NATIVE_WIDTH - 1 - x;
            break;
        case ROTATION_180:
            x = NATIVE_WIDTH - 1 - x;
            y = NATIVE_HEIGHT - 1 - y;
            break;
        case ROTATION_270:
            std::swap(x, y);
            y = NATIVE_HEIGHT - 1 - y;
            break;
        default:
            break;
    }
    return NATIVE_WIDTH * y + x;
}

uint8_t DisplayGDEW075T7::getPx(int32_t x, int32_t y)
{
    const uint32_t i = getPixelIndex(x, y);
    if (i != SIZE_MAX) {
        return (_frameBuffer[i / 4] >> ((3 - i % 4) * 2)) & 0b11;
    } else {
        return 0;
    }
}

void DisplayGDEW075T7::setPx(int32_t x, int32_t y, uint8_t color)
{
    const uint32_t i = getPixelIndex(x, y);
    if (i != SIZE_MAX) {
        uint8_t *fb = &_frameBuffer[i / 4];
        const uint8_t shift = (3 - i % 4) * 2;
        *fb &= ~(0b11 << shift);
        *fb |= (color & 0b11) << shift;
    }
}

void DisplayGDEW075T7::drawImage(const Image &image, int32_t x, int32_t y, Align align)
{
    adjustAlignment(&x, &y, image.width, image.height, align);

    uint32_t i = 0;
    uint8_t color;
    for (uint32_t y_src = 0; y_src < image.height; ++y_src) {
        for (uint32_t x_src = 0; x_src < image.width; ++x_src, ++i) {
            color = (image.data[i / 4] >> ((3 - i % 4) * 2)) & 0b11;
            if (color != _alpha) {
                setPx(x + x_src, y + y_src, color);
            }
        }
    }
}

uint32_t DisplayGDEW075T7::measureText(String str, const Font &font, int32_t tracking)
{
    if (str.length() == 0) {
        return 0;
    }

    uint32_t length = 0;
    for (char c : str) {
        if (c == ' ' || c < font.rangeStart || c > font.rangeEnd) {
            length += font.spaceWidth;
        } else {
            const FontGlyph glyph = font.glyphs[c - font.rangeStart];
            length += glyph.width + glyph.left;
        }
    }
    return length + tracking * (str.length() - 1);
}

void DisplayGDEW075T7::drawText(String str, const Font &font, int32_t x, int32_t y, Align align, int32_t tracking)
{
    if (align != TOP_LEFT) {
        uint32_t width;
        // Measurement isn't needed and width isn't used by adjustAligment if horizontal alignment is left
        if (!(align & _ALIGN_LEFT)) {
            width = measureText(str, font);
        }
        adjustAlignment(&x, &y, width, font.ascent + font.descent, align);
    }
    for (char c : str) {
        if (c == ' ' || c < font.rangeStart || c > font.rangeEnd) {
            x += font.spaceWidth + tracking;
        } else  {
            const FontGlyph glyph = font.glyphs[c - font.rangeStart];
            x += glyph.left;
            drawImage({ width: glyph.width, height: glyph.height, data: glyph.data }, x, y + glyph.top);
            x += glyph.width + tracking;
        }
    }
}

void DisplayGDEW075T7::drawMultilineText(
    std::initializer_list<String> lines,
    const Font &font,
    int32_t x,
    int32_t y,
    Align align,
    int32_t tracking,
    int32_t leading
) {
    // This implementation is simple because it assumes justification equals the horizontal alignment,
    // and that's all I needed it to do.
    leading += font.ascent + font.descent;

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

void DisplayGDEW075T7::drawHLine(int32_t x, int32_t y, int32_t length, uint32_t thickness, uint8_t color, Align align)
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

void DisplayGDEW075T7::drawVLine(int32_t x, int32_t y, int32_t length, uint32_t thickness, uint8_t color, Align align)
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

void DisplayGDEW075T7::fillRect(int32_t x, int32_t y, int32_t width, int32_t height, uint8_t color, Align align)
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

void DisplayGDEW075T7::strokeRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t strokeWidth, uint8_t color, bool strokeOutside, Align align)
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

void DisplayGDEW075T7::adjustAlignment(int32_t *x, int32_t *y, int32_t width, int32_t height, Align align)
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

uint32_t DisplayGDEW075T7::getWidth()
{
    return _width;
}

uint32_t DisplayGDEW075T7::getHeight()
{
    return _height;
}

DisplayGDEW075T7::Rotation DisplayGDEW075T7::getRotation()
{
    return _rotation;
}

void DisplayGDEW075T7::setRotation(Rotation rotation)
{
    _rotation = rotation;
    if (rotation == ROTATION_0 || rotation == ROTATION_180) {
        _width = NATIVE_WIDTH;
        _height = NATIVE_HEIGHT;
    } else {
        _width = NATIVE_HEIGHT;
        _height = NATIVE_WIDTH;
    }
}

uint8_t DisplayGDEW075T7::getAlpha()
{
    return _alpha;
}

void DisplayGDEW075T7::setAlpha(uint8_t alpha)
{
    _alpha = alpha;
}
