#ifndef TIMING_H
#define TIMING_H

class Timing
{
public:
	
	static void Create();

	inline static Timing * Instance()
	{
		return m_instance;
	}

	void SetTargetDelta(double target) { mTargetDelta = target; }
	double GetTargetDelta() const { return mTargetDelta; } 

	void SetLastUpdateDelta(float value) { mLastUpdateDelta = value; }
	double GetLastUpdateDelta() const { return mLastUpdateDelta; }

	float GetTotalTimeSeconds() const { return mTotalTimeSeconds; }
	void IncrementTotalTimeSeconds(float delta) { mTotalTimeSeconds += delta; }

	float GetTimeModifier() const { return mTimeModifier; }
	void SetTimeModifier(float value) { mTimeModifier = value; }

	void SetTimeModifierForNumSeconds(float timeMod, float seconds);

	void Update(float delta);

private:

	static Timing * m_instance; ///< static instance of the class
	double mTimeStep;

	Timing(void);
	~Timing(void);

	double mLastUpdateDelta;
	double mTargetDelta;
	float mTotalTimeSeconds;
	float mTimeModifier;

	bool mTimeModScheduled;
	float mScheduledTimeModTimeLeft;
	float mTimeModBeforeSchedule;
};

#endif
