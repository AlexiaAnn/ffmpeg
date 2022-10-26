#pragma once
#include "IncludeFFmpeg.h"
#include "PlayerMisco.h"
class Clock
{
public:
	double pts;
	double pts_drift;
	double last_updated;
	double speed;
	int serial;
	bool paused;
	int *queue_serial;

private:
	void SetClockAt();

public:
	Clock() : pts(0.0), pts_drift(0.0), last_updated(0.0), speed(1.0), serial(0), paused(false), queue_serial(nullptr) {}
	int ClockInit(int *queue_serial);
	double GetClock() const;
	void SetClock(double pts, int serial);
	void SetClockAt(double pts, double serial, double time);
	void SetClockSpeed(double speed);
	void sync_clock_to_slave(Clock* slave);
};