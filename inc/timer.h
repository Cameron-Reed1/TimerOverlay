#include <cstdint>
#include <chrono>


struct TimeDuration
{
    int64_t hours;
    int64_t minutes;
    int64_t seconds;
    int64_t milliseconds;

    int64_t minutes_absolute;
    int64_t seconds_absolute;
    int64_t milliseconds_absolute;

    bool negative;
};


class Timer
{
public:
    Timer();
    TimeDuration GetTimeLeft();
    void AddMinutes(int64_t minutes);
    void Clear();

private:
    std::chrono::system_clock::time_point end_time;
};

