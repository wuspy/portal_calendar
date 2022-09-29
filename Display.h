#include "Arduino.h"
#include "DisplayGDEW075T7.h"
#include "resource/image.h"

#ifndef PORTALCALENDAR_DISPLAY_H
#define PORTALCALENDAR_DISPLAY_H

class Display
{
public:
    Display();
    ~Display();
    void error(String message);
    void update(const tm *now);

private:
    void init();
    void drawIcon(const Image& icon, int32_t x, int32_t y);
    DisplayGDEW075T7 *_display;
};

#endif // PORTALCALENDAR_DISPLAY_H
