#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

class Timer
{
public:
	Timer()
	{
		started = false;
	}
    void Start()
	{
		started = true;
        gettimeofday(&startTime, NULL);
    }
    void Stop()
    {
        started = false;
    }

    double GetDuration()
	{
        long seconds, useconds;
        double duration;

        if (started)
            gettimeofday(&endTime, NULL);

        seconds  = endTime.tv_sec  - startTime.tv_sec;
        useconds = endTime.tv_usec - startTime.tv_usec;

        duration = seconds + useconds/1000000.0;

        return duration;
    }

	bool IsStarted()
	{
		return started;
	}

private:
    timeval startTime;
	bool started;
	timeval endTime;
};
