#ifndef WS_BUTTONPIN_HPP
#define WS_BUTTONPIN_HPP

#include <Arduino.h>
#include <cstdint>

#include "ButtonPinStatus.hpp"

class ButtonPin final
{
public:
    using PinType = pin_size_t;

    ButtonPin(PinType pin) : _pin{ pin } {}

    void begin()
    {
        pinMode(_pin, INPUT);
    }

    ButtonPinStatus read()
    {
        return ButtonPinStatus{ digitalRead(_pin) };
    }

private:
    PinType _pin{};
};

#endif
