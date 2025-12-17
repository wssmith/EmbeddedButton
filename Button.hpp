#ifndef WS_BUTTON_HPP
#define WS_BUTTON_HPP

#include <cstdint>

#include "ButtonPin.hpp"
#include "ButtonPinStatus.hpp"
#include "Clocks.hpp"
#include "Timer.hpp"

enum class ButtonEvent : uint8_t
{
    None = 0,
    Press,
    Release,
    Hold,
    LongPress
};

struct EmptyTag final {};

template<typename TTag = EmptyTag>
class Button final
{
public:
    using ButtonEventHandler = void(*)(Button&);
    using Clock = MilliClock<>;
    using ClockType = Clock::ClockType;
    using TagType = TTag;
    using PinType = ButtonPin::PinType;

    explicit Button(PinType pin, ClockType longPressMs = DefaultLongPressDuration, const TTag& tag = TTag{})
        : Button{ pin, DefaultActiveLevel, longPressMs, tag }
    {
    }

    Button(PinType pin, const TTag& tag)
        : Button{ pin, DefaultActiveLevel, DefaultLongPressDuration, tag }
    {
    }

    Button(PinType pin, ButtonPinStatus activeLevel, const TTag& tag)
        : Button{ pin, activeLevel, DefaultLongPressDuration, tag }
    {
    }

    Button(PinType pin, ButtonPinStatus activeLevel, ClockType longPressMs = DefaultLongPressDuration, const TTag& tag = TTag{})
        : _tag{ tag },
          _debounceTimer{ nullptr, DebounceDelayDuration, 0, Duration::Never },
          _longPressMs{ longPressMs },
          _pin{ pin }
    {
        switch (activeLevel)
        {
            case ButtonPinStatus::Low:
            {
                _upState = ButtonPinStatus::High;
                _downState = ButtonPinStatus::Low;
                _prevState = ButtonPinStatus::High;
                break;
            }
            case ButtonPinStatus::High:
            default:
            {
                _upState = ButtonPinStatus::Low;
                _downState = ButtonPinStatus::High;
                _prevState = ButtonPinStatus::Low;
                break;
            }
        }
    }

    ~Button() = default;

    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;

    Button(Button&&) = default;
    Button& operator=(Button&&) = default;

    void begin()
    {
        _pin.begin();
    }

    void setHandler(ButtonEvent btnEvent, ButtonEventHandler handler)
    {
        switch (btnEvent)
        {
            case ButtonEvent::Press:
                _press = handler;
                break;
            case ButtonEvent::Release:
                _release = handler;
                break;
            case ButtonEvent::Hold:
                _hold = handler;
                break;
            case ButtonEvent::LongPress:
                _longPress = handler;
                break;
            case ButtonEvent::None:
            default:
                break;
        }
    }

    void removeHandler(ButtonEvent btnEvent)
    {
        setHandler(btnEvent, nullptr);
    }

    void removeAllHandlers()
    {
        removeHandler(ButtonEvent::Press);
        removeHandler(ButtonEvent::Release);
        removeHandler(ButtonEvent::Hold);
        removeHandler(ButtonEvent::LongPress);
    }

    ButtonEvent update()
    {
        const ButtonEvent btnEvent = detectEvent();
        switch (btnEvent)
        {
            case ButtonEvent::Press:
                doPress();
                break;
            case ButtonEvent::Release:
                doRelease();
                break;
            case ButtonEvent::Hold:
                doHold();
                break;
            case ButtonEvent::LongPress:
                doLongPress();
                doHold(); // a long press happens during a hold
                break;
            case ButtonEvent::None:
            default:
                break;
        }

        return btnEvent;
    }

    ClockType lastPress() const { return _lastPress; }
    ClockType lastRelease() const { return _lastRelease; }

    ClockType longPressDuration() const { return _longPressMs; }
    void setLongPressDuration(ClockType longPressMs) { _longPressMs = longPressMs; }

    const TTag& tag() const { return _tag; }
    void setTag(const TTag& tag) { _tag = tag; }

private:
    ButtonEvent detectEvent();

    void doPress()
    {
        _lastPress = Clock::now();
        _longPressTriggered = false;

        if (_press != nullptr)
            _press(*this);
    }

    void doRelease()
    {
        _lastRelease = Clock::now();
        _longPressTriggered = false;

        if (_release != nullptr)
            _release(*this);
    }

    void doHold()
    {
        if (_hold != nullptr)
            _hold(*this);
    }

    void doLongPress()
    {
        _longPressTriggered = true;

        if (_longPress != nullptr)
            _longPress(*this);
    }

    TTag _tag{};

    Timer _debounceTimer;

    ButtonEventHandler _press = nullptr;
    ButtonEventHandler _release = nullptr;
    ButtonEventHandler _hold = nullptr;
    ButtonEventHandler _longPress = nullptr;

    ClockType _lastPress = 0;
    ClockType _lastRelease = 0;
    ClockType _longPressMs = DefaultLongPressDuration;

    ButtonPin _pin;

    bool _longPressTriggered = false;

    ButtonPinStatus _upState = ButtonPinStatus::Low;
    ButtonPinStatus _downState = ButtonPinStatus::High;
    ButtonPinStatus _prevState = ButtonPinStatus::Low;

    inline static constexpr ClockType DebounceDelayDuration = 5;
    inline static constexpr ClockType DefaultLongPressDuration = 2000;
    inline static constexpr ButtonPinStatus DefaultActiveLevel = ButtonPinStatus::High;
};

template<typename TTag>
inline ButtonEvent Button<TTag>::detectEvent()
{
    ButtonEvent btnEvent = ButtonEvent::None;

    if (!_debounceTimer.running())
    {
        const ButtonPinStatus currentState = _pin.read();
        if (_prevState != currentState)
        {
            // begin debounce
            _debounceTimer.restart(DebounceDelayDuration);
        }
        else if (currentState == _downState)
        {
            // button is being held
            // a long press event can be triggered once during a hold
            if ((_longPressMs != 0) && !_longPressTriggered && ((Clock::now() - _lastPress) > _longPressMs))
                btnEvent = ButtonEvent::LongPress;
            else
                btnEvent = ButtonEvent::Hold;
        }
    }
    else if (_debounceTimer.update())
    {
        // end debounce
        const ButtonPinStatus currentState = _pin.read();

        if ((_prevState == _upState) && (currentState == _downState))
            btnEvent = ButtonEvent::Press;
        else if ((_prevState == _downState) && (currentState == _upState))
            btnEvent = ButtonEvent::Release;
        else
            btnEvent = ButtonEvent::None;

        _prevState = currentState;
    }

    return btnEvent;
}

#endif
