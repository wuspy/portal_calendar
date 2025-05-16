#include <SPI.h>
#include "FrameBuffer.h"

#ifndef PORTALCALENDAR_DISPLAY_GDEW075T7_H
#define PORTALCALENDAR_DISPLAY_GDEW075T7_H

class DisplayGDEW075T7
{
public:
    static const uint32_t NATIVE_WIDTH = 800;
    static const uint32_t NATIVE_HEIGHT = 480;

    DisplayGDEW075T7(
        uint8_t spi_bus,
        uint8_t sck_pin,
        uint8_t copi_pin,
        uint8_t cs_pin,
        uint8_t reset_pin,
        uint8_t dc_pin,
        uint8_t busy_pin,
        uint8_t pwr_pin
    );
    ~DisplayGDEW075T7();
    void refresh(const FrameBuffer *frameBuffer);
    void fastClear(bool black = false);

private:
    uint8_t _resetPin;
    uint8_t _dcPin;
    uint8_t _csPin;
    uint8_t _busyPin;
    uint8_t _pwrPin;
    SPIClass *_spi;

    void wakeup();
    void sleep();
    void setLut(uint8_t cmd, const uint8_t* lut);
    void sendCommand(uint8_t command);
    void sendData(uint8_t data);
    void waitUntilIdle();
};

#endif // PORTALCALENDAR_DISPLAY_GDEW075T7_H
