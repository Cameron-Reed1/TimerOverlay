#include <chrono>
#include <cstdint>
#include <unistd.h>

#include "timer.h"


Timer::Timer()
{
    Clear();
}

TimeDuration Timer::GetTimeLeft()
{
    auto time_left = end_time - std::chrono::system_clock::now();
    bool negative = false;

    if (time_left.count() < 0) {
        time_left *= -1;
        negative = true;
    }

    return TimeDuration{
        std::chrono::duration_cast<std::chrono::hours>(time_left).count(),
        std::chrono::duration_cast<std::chrono::minutes>(time_left).count() % 60,
        std::chrono::duration_cast<std::chrono::seconds>(time_left).count() % 60,
        std::chrono::duration_cast<std::chrono::milliseconds>(time_left).count() % 1000,

        std::chrono::duration_cast<std::chrono::minutes>(time_left).count(),
        std::chrono::duration_cast<std::chrono::seconds>(time_left).count(),
        std::chrono::duration_cast<std::chrono::milliseconds>(time_left).count(),

        negative,
    };
}

void Timer::AddMinutes(int64_t minutes)
{
    const auto time_left = end_time - std::chrono::system_clock::now();

    if (time_left.count() <= 0) {
        end_time = std::chrono::system_clock::now() + std::chrono::minutes(minutes);
    } else {
        end_time += std::chrono::minutes(minutes);
    }
}

void Timer::Clear()
{
    end_time = std::chrono::system_clock::now() - std::chrono::seconds(60);
}



StopWatch::StopWatch()
    : active(false), paused(false) { }

TimeDuration StopWatch::GetTime()
{
    if (!active) {
        return TimeDuration{
            0, 0, 0, 0,
            0, 0, 0,
            false,
        };
    }

    auto time = paused ? paused_duration : std::chrono::system_clock::now() - start_time;

    return TimeDuration{
        std::chrono::duration_cast<std::chrono::hours>(time).count(),
        std::chrono::duration_cast<std::chrono::minutes>(time).count() % 60,
        std::chrono::duration_cast<std::chrono::seconds>(time).count() % 60,
        std::chrono::duration_cast<std::chrono::milliseconds>(time).count() % 1000,

        std::chrono::duration_cast<std::chrono::minutes>(time).count(),
        std::chrono::duration_cast<std::chrono::seconds>(time).count(),
        std::chrono::duration_cast<std::chrono::milliseconds>(time).count(),

        false,
    };
}

void StopWatch::Start()
{
    if (!active) {
        start_time = std::chrono::system_clock::now();
        active = true;
        paused = false;
    }
}

void StopWatch::Pause()
{
    if (!paused) {
        paused_duration = std::chrono::system_clock::now() - start_time;
        paused = true;
    }
}

void StopWatch::Resume()
{
    if (paused) {
        start_time = std::chrono::system_clock::now() - paused_duration;
        paused = false;
    }
}

void StopWatch::Clear()
{
    active = false;
    paused = false;
}


