#ifndef TIMING_H
#define TIMING_H

///Timing Class
/** Used to control the frame rate.
*/
class Timing
{
	private:

		static Timing * m_instance; ///< static instance of the class
		double mTimeStep;

		Timing(void);
		~Timing(void);

		float mLastUpdateDelta;
		float mTargetDelta;
		float mTotalTimeSeconds;
		float mTimeModifier;

	public:
		 
		inline static Timing * Instance()
		{
			if(m_instance == 0)
			{
				m_instance = new Timing();
			}
			return m_instance;
		}

		void SetTargetDelta(float target) { mTargetDelta = target; }
		float GetTargetDelta() const { return mTargetDelta; } 

		void SetLastUpdateDelta(float value) { mLastUpdateDelta = value; }
		float GetLastUpdateDelta() const { return mLastUpdateDelta; }

		float GetTotalTimeSeconds() const { return mTotalTimeSeconds; }
		void IncrementTotalTimeSeconds(float delta) { mTotalTimeSeconds += delta; }

		float GetTimeModifier() const { return mTimeModifier; }
		void SetTimeModifier(float value) { mTimeModifier = value; }
};

#endif
