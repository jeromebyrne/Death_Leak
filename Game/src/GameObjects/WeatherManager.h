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
		kSnowing,
		kNone
	};

	void Update(float delta);

	static WeatherManager * GetInstance();

	void Destroy();

	// called after we switch levels
	void RefreshAssets();

	float GetFoliageSwayMultiplier() const { return mFoliageSwayMultiplier; }

	void SetAllowWeather(bool value) { mAllowWeather = value; }

private:

	WeatherManager(void);
	virtual ~WeatherManager(void);

	void StartRaining();
	void StopRaining();

	void StartSnowing();
	void StopSnowing();

	void DoLightningEffect();

	bool HasCurrentWeatherState(WeatherState state);

	static WeatherManager * mInstance;

	ParallaxLayer * mTopRainLayer;
	ParallaxLayer * mBottomRainLayer;
	ParallaxLayer * mRainLayer3;
	ParallaxLayer * mGroundRainLayer;

	ParallaxLayer * mTopSnowLayer;
	ParallaxLayer * mBottomSnowLayer;
	ParallaxLayer * mSnowLayer3;
	ParallaxLayer * mLightningLayer;

	void UpdateRaining(float delta);
	void UpdateSnowing(float delta);
	void UpdateNoWeather(float delta);

	void RemoveState(WeatherState state);

	void CreateRainAssets();

	void CreateSnowAssets();

	void FadeWeatherIfApplicable(float delta);

	list<WeatherState> mCurrentStates;

	double mElapsedTime;

	double mRainStartTime;

	double mSnowStartTime;

	double mLightningStartTime;

	double mCurrentRainSessionTime;

	double mCurrentSnowSessionTime;

	bool mHasHadWeather;

	list<WeatherState> mStateKillList;

	AudioObject * mRainSFX;

	AudioObject * mSnowSFX;

	float mLastWeatherTime;

	bool mHasRained;
	bool mHasSnowed;

	float mNextWeatherDecisionTime;

	bool mPLayingLightningEffect;
	float mTimeUntilNextLightning;

	float mFoliageSwayMultiplier;

	bool mAllowWeather;
};

#endif

