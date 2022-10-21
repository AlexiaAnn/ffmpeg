#include "Clock.h"
double Clock::GetClock() const
{
    if (*queue_serial != serial)
        return NAN;
    if (paused)
        return pts;
    else
    {
        double time = av_gettime_relative() / 1000000.0;
        return pts_drift + time - (time - last_updated) * (1.0 - speed);
    }
}
void Clock::SetClockAt()
{
    double time = av_gettime_relative() / 1000000.0;
    this->last_updated = time;
    this->pts_drift = this->pts - time;
}
int Clock::ClockInit(int *queue_serial)
{
    this->queue_serial = queue_serial;
    this->pts = NAN;
    this->serial = -1;
    SetClockAt();
    return 0;
}
void Clock::SetClock(double pts, int serial)
{
    this->pts = pts;
    this->serial = serial;
    SetClockAt();
}

void Clock::SetClockAt(double pts, double serial, double time)
{
    this->pts = pts;
    this->last_updated = time;
    this->pts_drift = this->pts - time;
    this->serial = serial;
}

void Clock::SetClockSpeed(double speed)
{
    SetClock(GetClock(),this->serial);
    this->speed = speed;
}

void Clock::sync_clock_to_slave(Clock* slave)
{
    double clock = this->GetClock();
    double slave_clock = slave->GetClock();
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        this->SetClock(slave_clock, slave->serial);
}
