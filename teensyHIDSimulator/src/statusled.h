#ifndef STATUSLED_H
#define STATUSLED_H

#include "types.h"

class StatusLED
{
public:
    static StatusLED& instance();

    void on();
    void off();
    void flip();

    void setColor(const color& colorRGB);
    void blink(const color& colorRGB, float time);

private:
    StatusLED();
    void update();

    color cl;
    bool active = false;
};

#endif // STATUSLED_H
