#include "precompiled.h"
#include "Timing.h"
Timing * Timing::m_instance = 0;

Timing::Timing(void) :
mTargetDelta(0),
mLastUpdateDelta(0),
mTotalTimeSeconds(0),
mTimeModifier(1.0f)
{
}

Timing::~Timing(void)
{
}

/*void Timing::Update(bool lockFrameRate, int framesPerSecond)
{
	if(lockFrameRate)
	{
		TICKS_PER_SECOND = framesPerSecond;

		long elapsed;
		elapsed=m_newTime-m_oldTime;

		long frameTime = 1000/TICKS_PER_SECOND;
		while(elapsed< frameTime)
		{
			Sleep(1);
			m_newTime=timeGetTime();
			elapsed=m_newTime-m_oldTime;
		}

		m_lastFrameTime = elapsed;
		m_oldTime=m_newTime;
		m_totalGameTime = m_newTime; // increment total time
	}
	else
	{
		m_newTime=timeGetTime();
		m_totalGameTime = m_newTime; // increment total time
		m_lastFrameTime = m_newTime - m_oldTime;
		m_oldTime = m_newTime;
	}
}*/

