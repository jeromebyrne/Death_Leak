#ifndef DEATHLEAK_ENGINE_PLATFORM_ITIMESOURCE_H
#define DEATHLEAK_ENGINE_PLATFORM_ITIMESOURCE_H

class ITimeSource
{
public:
    virtual ~ITimeSource() = default;

    virtual double Seconds() const = 0;
    virtual void SleepUntilNextFrame(double targetDeltaSeconds) = 0;
};

#endif
