#ifndef WS_CLOCKS_HPP
#define WS_CLOCKS_HPP

#include <Arduino.h>

template<typename TValue = unsigned long>
struct MilliClock final
{
    using ClockType = TValue;
    ClockType operator()() { return now(); }
    static ClockType now() { return millis(); };
};

template<typename TValue = unsigned long>
struct MicroClock final
{
    using ClockType = TValue;
    ClockType operator()() { return now(); }
    static ClockType now() { return micros(); };
};

#endif
