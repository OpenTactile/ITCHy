#include "statusled.h"

#include <WProgram.h>

const std::array<unsigned char, 3> pins = { 22, 21, 23 };

StatusLED& StatusLED::instance()
{
    static StatusLED inst;
    return inst;
}

StatusLED::StatusLED()
{
    cl = {0, 0, 0};
    analogWriteResolution(8);

    for(const unsigned char& pin : pins)
    {
        analogWriteFrequency(pin, 187500);
        analogWrite(pin, 0);
    }
}

void StatusLED::on()
{
    active = true;
    update();
}

void StatusLED::off()
{
    active = false;
    update();
}

void StatusLED::flip()
{
    active = !active;
    update();
}

void StatusLED::setColor(const color& colorRGB)
{
    cl = colorRGB;
    update();
}

void StatusLED::blink(const color &colorRGB, float time)
{
    unsigned int dl = time * 500.0f;
    unsigned int dlHalf = time * 250.0f;

    auto colorBefore = cl;
    bool oldActive = active;

    off();
    delay(dlHalf);
    cl = colorRGB;
    on();
    delay(dl);
    off();
    delay(dlHalf);

    active = oldActive;
    cl = colorBefore;
    update();
}

void StatusLED::update()
{
    for(int n = 0; n < 3; n++)
    {
        if(active)
        {
            analogWrite(pins[n], cl[n]);
        }
        else
        {
            analogWrite(pins[n], 0);
        }
    }
}

