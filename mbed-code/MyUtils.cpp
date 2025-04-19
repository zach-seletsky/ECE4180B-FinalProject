#include "MyUtils.h"

bool debounceBtn(DigitalIn * btn) {
    bool pressed = false;
    if (btn->read()) {
        ThisThread::sleep_for(100ms);
        if (btn->read()) {
            pressed = true;
        }
    }
    return pressed;
}