#include "precompiled.h"
#include "Timing.h"
Timing * Timing::m_instance = 0;

Timing::Timing(void) :
	mTargetDelta(0),
	mLastUpdateDelta(0),
	mTotalTimeSeconds(0),
	mTimeModifier(1.0f),
	mTimeModScheduled(false),
	mScheduledTimeModTimeLeft(0.0f),
	mTimeModBeforeSchedule(0.0f)
{
}

Timing::~Timing(void)
{
}

void Timing::SetTimeModifierForNumSeconds(float timeMod, float seconds)
{
	if (mTimeModScheduled)
	{
		// can't have a schedule if already doing one
		return;
	}

	mTimeModScheduled = true;
	mTimeModBeforeSchedule = mTimeModifier;
	mScheduledTimeModTimeLeft = seconds;

	mTimeModifier = timeMod;
}

void Timing::Update(float delta)
{
	if (mTimeModScheduled)
	{
		mScheduledTimeModTimeLeft -= delta;

		if (mScheduledTimeModTimeLeft <= 0.0f)
		{
			mScheduledTimeModTimeLeft = 0.0f;
			mTimeModScheduled = false;
			mTimeModifier = mTimeModBeforeSchedule;
		}
	}
}
