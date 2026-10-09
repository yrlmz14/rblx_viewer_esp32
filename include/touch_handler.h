#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <stdint.h>

struct TouchPoint {
    uint16_t x;
    uint16_t y;
    bool touched;
};

class TouchHandler {
public:
    TouchHandler();
    bool init();
    bool getTouch(TouchPoint& pt);

    void updateCameraState(float& camRotX, float& camRotY, float& camZoom);

private:
    TouchPoint lastPoint;
    bool isDragging;
};

#endif // TOUCH_HANDLER_H
