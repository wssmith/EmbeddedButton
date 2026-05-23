#ifndef WS_BUTTONPIN_HPP
#define WS_BUTTONPIN_HPP

#include <Arduino.h>
#include <stdint.h>

using pin_t = decltype(A0);

enum class ButtonPinStatus : uint8_t
{
    Low = 0,
    High = 1
};

class ButtonPin final
{
public:
    using PinType = pin_t;

    ButtonPin(PinType pin) : _pin{ pin } {}

    void begin(uint8_t mode = INPUT)
    {
        pinMode(_pin, mode);
    }

    ButtonPinStatus read()
    {
        return static_cast<ButtonPinStatus>(digitalRead(_pin));
    }

private:
    PinType _pin{};
};

#endif
