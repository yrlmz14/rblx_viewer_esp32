#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino.h>
#include <stdint.h>
#include "pin_config.h"

class DisplayDriver {
public:
    DisplayDriver();
    bool init();
    void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void writePixels(const uint16_t* colors, uint32_t len);
    void setBrightness(uint8_t brightness);
    void clear(uint16_t color = 0x0000);

    uint16_t getWidth() const { return SCREEN_WIDTH; }
    uint16_t getHeight() const { return SCREEN_HEIGHT; }

private:
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeDataBuffer(const uint8_t* buffer, uint32_t len);
};

#endif // DISPLAY_DRIVER_H
