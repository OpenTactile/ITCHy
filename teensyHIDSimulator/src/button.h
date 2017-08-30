#ifndef BUTTON_H
#define BUTTON_H

class Button
{
public:
    Button(unsigned char pinID);
    bool query();

private:
    unsigned char pin;
};

#endif // BUTTON_H
