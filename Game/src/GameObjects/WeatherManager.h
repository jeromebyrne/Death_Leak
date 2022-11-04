#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

class ParallaxLayer;
class AudioObject;

class WeatherManager
{
public:

	enum WeatherState
	{
		kRaining,
		// kSnowing, REMOVING SNOW CODE to simplify
		kNone
	};

	void Update(float delta);

	static WeatherManager * GetInstance();

	void Destroy();

	// called after we switch levels
	void RefreshAssets();

	float GetFoliageSwayMultiplier() const { return mFoliageSwayMultiplier; }

	void StartRainingIfNotAlready();

	void AllowWeather(bool allow);

private:

	WeatherManager(void);
	virtual ~WeatherManager(void);

	void StartRaining();
	void StopRaining();

	void DoLightningEffect();

	bool HasCurrentWeatherState(WeatherState state);

	static WeatherManager * mInstance;

	ParallaxLayer * mGroundRainLayer;

	ParallaxLayer * mLightningLayer;

	ParticleSpray * mRainParticleSpray = nullptr;

	void UpdateRaining(float delta);
	void UpdateNoWeather(float delta);

	void RemoveState(WeatherState state);

	void CreateRainAssets(bool preWarm = false);

	void FadeWeatherIfApplicable(float delta);

	list<WeatherState> mCurrentStates;

	double mElapsedTime;

	double mRainStartTime;

	double mLightningStartTime;

	double mCurrentRainSessionTime;

	bool mHasHadWeather;

	list<WeatherState> mStateKillList;

	AudioObject * mRainSFX;

	float mLastWeatherTime;

	float mNextWeatherDecisionTime;

	bool mPLayingLightningEffect;
	float mTimeUntilNextLightning;

	float mFoliageSwayMultiplier;

	bool mAllowWeather;
};

#endif

