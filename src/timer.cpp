#include <chrono>
#include <cstdint>

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

