#include "button.h"
#include <WProgram.h>

Button::Button(unsigned char pinID)
{
    pin = pinID;
    pinMode(pin, INPUT_PULLUP);

}

bool Button::query()
{
    return !digitalRead(pin);
}
