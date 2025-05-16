/**
 * Display class supporting 4-level greyscale for GDEW075T7 e-paper display, aka the 800x480 7.5" B/W from Waveshare.
 *
 * https://www.good-display.com/product/244.html
 *
 * Datasheet (for a different display controller) that has the LUT commands documented:
 * https://www.smart-prototyping.com/image/data/9_Modules/EinkDisplay/GDEW0154T8/IL0373.pdf
 */

#include "DisplayGDEW075T7.h"
#include "config.h"

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
 * LUTs for blanking the screen quickly
 */

const uint8_t LUT_VCOM_FAST_CLEAR[] = {
    LUT_ROW(LEVEL_VCOM_VCMDC, 15, LEVEL_VCOM_VCMDC, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_WHITE_FAST_CLEAR[] = {
    LUT_ROW(LEVEL_GND, 15, LEVEL_VDL, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
};

const uint8_t LUT_BLACK_FAST_CLEAR[] = {
    LUT_ROW(LEVEL_GND, 15, LEVEL_VDH, 15, 0, 0, 0, 0, 1),
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
    LUT_ROW_NOOP,
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
    #ifndef HEADLESS
    _spi->endTransaction();
    _spi->end();
    delete _spi;
    #endif
};

DisplayGDEW075T7::DisplayGDEW075T7(
    uint8_t spi_bus,
    uint8_t sck_pin,
    uint8_t copi_pin,
    uint8_t cs_pin,
    uint8_t reset_pin,
    uint8_t dc_pin,
    uint8_t busy_pin,
    uint8_t pwr_pin
) {
    _resetPin = reset_pin;
    _dcPin = dc_pin;
    _csPin = cs_pin;
    _busyPin = busy_pin;
    _pwrPin = pwr_pin;

    pinMode(_csPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);
    pinMode(_dcPin, OUTPUT);
    pinMode(_busyPin, INPUT);
    pinMode(_pwrPin, OUTPUT);

    #ifndef HEADLESS
    _spi = new SPIClass(spi_bus);
    _spi->begin(sck_pin, -1, copi_pin, cs_pin);
    _spi->beginTransaction(SPISettings(7000000, MSBFIRST, SPI_MODE0));
    #endif
};

void DisplayGDEW075T7::wakeup()
{
    digitalWrite(_pwrPin, HIGH);
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
    sendData(0x22);         // -1.8v

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
}

void DisplayGDEW075T7::sendCommand(uint8_t command)
{
    #ifndef HEADLESS
    digitalWrite(_dcPin, LOW);
    _spi->transfer(command);
    digitalWrite(_dcPin, HIGH);
    #endif
}

void DisplayGDEW075T7::sendData(uint8_t data)
{
    #ifndef HEADLESS
    _spi->transfer(data);
    #endif
}

void DisplayGDEW075T7::waitUntilIdle()
{
    #ifndef HEADLESS
    unsigned long start = millis();
    do {
        delay(5);
    } while (digitalRead(_busyPin) == LOW && millis() - start < BUSY_TIMEOUT);
    #endif
    delay(20);
}

void DisplayGDEW075T7::setLut(uint8_t cmd, const uint8_t* lut)
{
    sendCommand(cmd);
    for (size_t i = 0; i < 42; ++i) {
        sendData(lut[i]);
    }
}

void DisplayGDEW075T7::refresh(const FrameBuffer *frameBuffer)
{
    wakeup();

    setLut(CMD_SET_LUTVCOM, LUT_VCOM_2BIT);
    setLut(CMD_SET_LUTWW, LUT_WHITE_2BIT);
    setLut(CMD_SET_LUTBW, LUT_DGREY_2BIT);
    setLut(CMD_SET_LUTWB, LUT_LGREY_2BIT);
    setLut(CMD_SET_LUTBB, LUT_BLACK_2BIT);
    setLut(CMD_SET_LUTBD, LUT_WHITE_2BIT);

    size_t i;
    uint8_t j;
    uint16_t chunk; // Holds 8px of frame_buffer
    uint8_t output;   // Holds 8px of display output
    size_t len = frameBuffer->getLength();
    const uint8_t *data = frameBuffer->data;

    sendCommand(CMD_DTM1);
    for (i = 0; i < len; i += 2) {
        output = 0;
        chunk = (data[i] << 8) | data[i + 1];
        for (j = 0; j < 8; ++j) {
            output |= LUT_DTM1[(chunk >> (j * 2)) & 0b11] << j;
        }
        sendData(output);
    }
    sendCommand(CMD_DTM2);
    for (i = 0; i < len; i += 2) {
        output = 0;
        chunk = (data[i] << 8) | data[i + 1];
        for (j = 0; j < 8; ++j) {
            output |= LUT_DTM2[(chunk >> (j * 2)) & 0b11] << j;
        }
        sendData(output);
    }

    sendCommand(CMD_REFRESH);
    delay(100);
    waitUntilIdle();
    sleep();
}

void DisplayGDEW075T7::fastClear(bool black)
{
    wakeup();

    setLut(CMD_SET_LUTVCOM, LUT_VCOM_FAST_CLEAR);
    setLut(CMD_SET_LUTWW, black ? LUT_BLACK_FAST_CLEAR : LUT_WHITE_FAST_CLEAR);
    setLut(CMD_SET_LUTBW, black ? LUT_BLACK_FAST_CLEAR : LUT_WHITE_FAST_CLEAR);
    setLut(CMD_SET_LUTWB, black ? LUT_BLACK_FAST_CLEAR : LUT_WHITE_FAST_CLEAR);
    setLut(CMD_SET_LUTBB, black ? LUT_BLACK_FAST_CLEAR : LUT_WHITE_FAST_CLEAR);
    setLut(CMD_SET_LUTBD, black ? LUT_BLACK_FAST_CLEAR : LUT_WHITE_FAST_CLEAR);

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
    digitalWrite(_pwrPin, LOW);
}
