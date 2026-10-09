#include "touch_handler.h"
#include <Arduino.h>
#include <Wire.h>
#include "pin_config.h"

#define TOUCH_I2C_ADDR 0x5D // CO5300 / CST820 default I2C address

TouchHandler::TouchHandler() : isDragging(false) {
    lastPoint = {0, 0, false};
}

bool TouchHandler::init() {
    Wire.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL);
    pinMode(TOUCH_INT, INPUT_PULLUP);
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(50);
    return true;
}

bool TouchHandler::getTouch(TouchPoint& pt) {
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    Wire.write(0x02); // Point count / status register
    if (Wire.endTransmission() != 0) {
        pt.touched = false;
        return false;
    }

    Wire.requestFrom((uint8_t)TOUCH_I2C_ADDR, (uint8_t)5);
    if (Wire.available() >= 5) {
        uint8_t status = Wire.read();
        uint8_t x_hi = Wire.read();
        uint8_t x_lo = Wire.read();
        uint8_t y_hi = Wire.read();
        uint8_t y_lo = Wire.read();

        uint8_t points = status & 0x0F;
        if (points > 0) {
            pt.x = ((x_hi & 0x0F) << 8) | x_lo;
            pt.y = ((y_hi & 0x0F) << 8) | y_lo;
            pt.touched = true;
            return true;
        }
    }

    pt.touched = false;
    return false;
}

void TouchHandler::updateCameraState(float& camRotX, float& camRotY, float& camZoom) {
    TouchPoint pt;
    if (getTouch(pt)) {
        if (isDragging) {
            int dx = (int)pt.x - (int)lastPoint.x;
            int dy = (int)pt.y - (int)lastPoint.y;

            camRotY += dx * 0.01f;
            camRotX += dy * 0.01f;
        }
        lastPoint = pt;
        isDragging = true;
    } else {
        isDragging = false;
    }
}
