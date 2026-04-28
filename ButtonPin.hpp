#ifndef WS_BUTTONPIN_HPP
#define WS_BUTTONPIN_HPP

#include <Arduino.h>
#include <cstdint>

enum class ButtonPinStatus : uint8_t
{
    Low = 0,
    High = 1
};

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
