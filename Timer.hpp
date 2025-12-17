#ifndef WS_TIMER_HPP
#define WS_TIMER_HPP

#include <cassert>
#include <cstdint>

#include "Clocks.hpp"

enum Duration : int32_t
{
    Endless = -1,
    Never = 0,
    Once = 1
};

class Timer final
{
public:
    using Clock = MilliClock<>;
    using ClockType = Clock::ClockType;
    using ClockPtr = ClockType(*)();
    using CallbackPtr = void(*)();

    Timer(CallbackPtr callback, ClockType dueTime, ClockType period)
        : Timer{ callback, dueTime, period, DurationDefault, nullptr }
    {
    }

    Timer(CallbackPtr callback, ClockType dueTime, ClockType period, int32_t duration)
        : Timer{ callback, dueTime, period, duration, nullptr }
    {
    }

    Timer(CallbackPtr callback, ClockType dueTime, ClockType period, ClockPtr clock)
        : Timer{ callback, dueTime, period, DurationDefault, clock }
    {
    }

    explicit Timer(ClockType delay)
        : Timer{ nullptr, delay, 0, Duration::Once, nullptr }
    {
    }

    Timer(ClockType delay, ClockPtr clock)
        : Timer{ nullptr, delay, 0, Duration::Once, clock }
    {
    }

    Timer(CallbackPtr callback, ClockType dueTime, ClockType period, int32_t duration, ClockPtr clock)
        : _dueTime{ dueTime },
          _period{ period },
          _callback{ callback },
          _clock{ (clock != nullptr) ? clock : ClockDefault },
          _duration{ (duration >= 0) ? duration : Duration::Endless },
          _remaining{ (duration >= 0) ? duration : Duration::Endless }
    {
    }

    ~Timer() = default;

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    Timer(Timer&& other) noexcept
        : _dueTime{ other._dueTime },
          _period{ other._period },
          _tick{ other._tick },
          _callback{ other._callback },
          _clock{ other._clock },
          _duration{ other._duration },
          _remaining{ other._remaining },
          _running{ other._running }
    {
        other._dueTime = 0;
        other._period = 0;
        other._tick = 0;
        other._callback = nullptr;
        other._clock = nullptr;
        other._duration = 0;
        other._remaining = 0;
        other._running = false;
    }

    Timer& operator=(Timer&& other) noexcept
    {
        if (this != &other)
        {
            _dueTime = other._dueTime;
            _period = other._period;
            _tick = other._tick;
            _callback = other._callback;
            _clock = other._clock;
            _duration = other._duration;
            _remaining = other._remaining;
            _running = other._running;

            other._dueTime = 0;
            other._period = 0;
            other._tick = 0;
            other._callback = nullptr;
            other._clock = nullptr;
            other._duration = 0;
            other._remaining = 0;
            other._running = false;
        }

        return *this;
    }

    void change(ClockType dueTime, ClockType period, int32_t duration)
    {
        assert(!_running); // Timer cannot be changed while running

        _dueTime = dueTime;
        _period = period;
        _duration = (duration >= 0) ? duration : Duration::Endless;
        _remaining = (duration >= 0) ? duration : Duration::Endless;
    }

    void change(ClockType dueTime, ClockType period, int32_t duration, ClockPtr clock)
    {
        change(dueTime, period, duration);
        _clock = clock;
    }

    void change(ClockType delay)
    {
        change(delay, 0, Duration::Once);
    }

    void change(ClockType delay, ClockPtr clock)
    {
        change(delay, 0, Duration::Once, clock);
    }

    void start()
    {
        start(_clock());
    }

    void start(ClockType startTime)
    {
        assert(!_running);   // Timer cannot be started while already running
        assert(!completed()); // Timer cannot be started since it has nothing to do

        _tick = startTime;
        _running = true;
    }

    void stop() noexcept { _running = false; }

    void restart(Timer::ClockType delay)
    {
        stop();
        change(delay);
        start();
    }

    void restart(Timer::ClockType dueTime, Timer::ClockType period, int32_t duration)
    {
        stop();
        change(dueTime, period, duration);
        start();
    }

    bool update() { return update(_clock()); }

    bool update(ClockType updateTime)
    {
        if (_running && ready(updateTime))
        {
            execute();
            return true;
        }
        return false;
    }

    bool running() const noexcept { return _running; }
    int32_t remaining() const { return _remaining; }
    bool endless() const { return _duration == Duration::Endless; }
    bool completed() const { return !endless() && (_remaining == 0); }
    bool paused() const { return !_running && !completed(); }

    ClockType dueTime() const { return _dueTime; }
    ClockType period() const { return _period; }
    int32_t duration() const { return _duration; }

private:
    bool ready(ClockType updateTime) const
    {
        return (updateTime + _period - _tick) >= (_dueTime + _period);
    }

    void execute()
    {
        if (_callback != nullptr)
            _callback();

        _tick += _period;

        if (endless())
            return;

        --_remaining;
        if (_remaining == 0)
            stop();
    }

    ClockType _dueTime = 0;
    ClockType _period = 0;
    ClockType _tick = 0;
    CallbackPtr _callback = nullptr;
    ClockPtr _clock = nullptr;
    int32_t _duration = 0;
    int32_t _remaining = 0;
    bool _running = false;

    inline static constexpr ClockPtr ClockDefault = Clock::now;
    inline static constexpr Duration DurationDefault = Duration::Endless;
};

#endif
