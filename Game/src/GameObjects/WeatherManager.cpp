#include "precompiled.h"
#include "WeatherManager.h"
#include "ParallaxLayer.h"
#include "AudioManager.h"
#include "AudioObject.h"
#include "Game.h"

static const float kTimeUntilFirstWeather = 15.0f;
static const float kRainSessionMinTime = 70.0f;
static const float kRainSessionMaxTime = 120.0f;
static const float kRainIntroTime = 10.0f;
static const float kRainOutroTime = 8.0f;

static const float kLightningTime = 0.4f;
static const float kLightningMinDelay = 10.0f;
static const float kLightningMaxDelay = 30.0f;

static const float kSnowSessionMinTime = 50.0f;
static const float kSnowSessionMaxTime = 90.0f;
static const float kSnowIntroTime = 8.0f;
static const float kSnowOutroTime = 8.0f;

static const float kMinWeatherIntervalTime = 90.0f;
static const float kMaxWeatherIntervalTime = 200.0f;

static const float kNormalFoliageSwayMultiplier = 1.0f;
static const float kRainFoliageSwayMultiplier = 1.5f;
static const float kSnowFoliageSwayMultiplier = 3.0f;

WeatherManager * WeatherManager::mInstance = nullptr;

WeatherManager * WeatherManager::GetInstance()
{
	if (!mInstance)
	{
		mInstance = new WeatherManager();
	}

	return mInstance;
}

WeatherManager::WeatherManager(void):
	mTopRainLayer(nullptr),
	mBottomRainLayer(nullptr),
	mElapsedTime(0),
	mRainStartTime(0),
	mHasHadWeather(false),
	mCurrentRainSessionTime(0),
	mGroundRainLayer(nullptr),
	mRainSFX(nullptr),
	mTopSnowLayer(nullptr),
	mBottomSnowLayer(nullptr),
	mSnowSFX(nullptr),
	mSnowStartTime(0),
	mCurrentSnowSessionTime(0),
	mLastWeatherTime(0),
	mHasSnowed(false),
	mNextWeatherDecisionTime(0),
	mHasRained(false),
	mLightningLayer(nullptr),
	mLightningStartTime(0),
	mPLayingLightningEffect(false),
	mTimeUntilNextLightning(0),
	mFoliageSwayMultiplier(1.0f)
{
	// preload large textures
	/*TextureManager::Instance()->LoadTexture("Media\\rainlayer.png");
	TextureManager::Instance()->LoadTexture("Media\\snowlayer.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_1.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_2.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_3.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_4.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_5.png");*/
}

WeatherManager::~WeatherManager(void)
{
}

void WeatherManager::RefreshAssets()
{
	float gameScale = Game::GetGameScale().X;

	for (auto state : mCurrentStates)
	{
		switch (state)
		{
		case kRaining:
			{
				mBottomRainLayer = new ParallaxLayer(Camera2D::GetInstance());
				mBottomRainLayer->m_textureFilename = "Media\\rainlayer.png";
				mBottomRainLayer->m_drawAtNativeDimensions = true;
				mBottomRainLayer->m_updateable = true;
				mBottomRainLayer->m_position = Vector3(0, 598 * gameScale, 15);
				mBottomRainLayer->m_dimensions = Vector3(2048, 2048, 0);
				mBottomRainLayer->mRepeatTextureX = false;
				mBottomRainLayer->mRepeatTextureY = false;
				mBottomRainLayer->m_repeatWidth = 2048;
				mBottomRainLayer->m_cameraParallaxMultiplierX = 0.000488;
				mBottomRainLayer->m_cameraParallaxMultiplierY = 0;
				mBottomRainLayer->m_followCamXPos = true;
				mBottomRainLayer->m_followCamYPos = false;
				mBottomRainLayer->m_autoScrollY = true;
				mBottomRainLayer->m_autoScrollX = false;
				mBottomRainLayer->m_autoScrollXSpeed = 0;
				mBottomRainLayer->m_autoScrollYSpeed = 6.5f;
				mBottomRainLayer->EffectName = "effectlighttexture";
				mBottomRainLayer->m_alpha = 1.0f;
				GameObjectManager::Instance()->AddGameObject(mBottomRainLayer);

				mTopRainLayer = new ParallaxLayer(Camera2D::GetInstance());
				mTopRainLayer->m_textureFilename = "Media\\rainlayer.png";
				mTopRainLayer->m_drawAtNativeDimensions = true;
				mTopRainLayer->m_updateable = true;
				mTopRainLayer->m_position = Vector3(0, 2650 * gameScale, 15);
				mTopRainLayer->m_dimensions = Vector3(2048, 2048, 0);
				mTopRainLayer->mRepeatTextureX = false;
				mTopRainLayer->mRepeatTextureY = false;
				mTopRainLayer->m_repeatWidth = 1920;
				mTopRainLayer->m_cameraParallaxMultiplierX = 0.000488;
				mTopRainLayer->m_cameraParallaxMultiplierY = 0;
				mTopRainLayer->m_followCamXPos = true;
				mTopRainLayer->m_followCamYPos = false;
				mTopRainLayer->m_autoScrollY = true;
				mTopRainLayer->m_autoScrollX = false;
				mTopRainLayer->m_autoScrollXSpeed = 0;
				mTopRainLayer->m_autoScrollYSpeed = 6.5f;
				mTopRainLayer->EffectName = "effectlighttexture";
				mTopRainLayer->m_alpha = 1.0f;
				GameObjectManager::Instance()->AddGameObject(mTopRainLayer);

				mGroundRainLayer = new ParallaxLayer(Camera2D::GetInstance());
				mGroundRainLayer->m_textureFilename = "";
				mGroundRainLayer->m_isAnimated = true;
				mGroundRainLayer->m_animationFile = "XmlFiles\\ground_rain.xml";
				mGroundRainLayer->m_drawAtNativeDimensions = true;
				mGroundRainLayer->m_updateable = true;
				mGroundRainLayer->m_position = Vector3(0, -220 * gameScale, 99);
				mGroundRainLayer->m_dimensions = Vector3(2048, 2048, 0);
				mGroundRainLayer->mRepeatTextureX = false;
				mGroundRainLayer->mRepeatTextureY = false;
				mGroundRainLayer->m_repeatWidth = 2048;
				mGroundRainLayer->m_cameraParallaxMultiplierX = 0.000488;
				mGroundRainLayer->m_cameraParallaxMultiplierY = 0;
				mGroundRainLayer->m_followCamXPos = true;
				mGroundRainLayer->m_followCamYPos = false;
				mGroundRainLayer->m_autoScrollY = false;
				mGroundRainLayer->m_autoScrollX = false;
				mGroundRainLayer->m_autoScrollXSpeed = 0;
				mGroundRainLayer->m_autoScrollYSpeed = 0;
				mGroundRainLayer->EffectName = "effectlighttexture";
				mGroundRainLayer->m_alpha = 1.0f;
				GameObjectManager::Instance()->AddGameObject(mGroundRainLayer);

				mRainSFX = new AudioObject(0, 1500,0,50000, 3000);
				mRainSFX->SetAdjustVolumeToCamera(false);
				mRainSFX->SetAudioFilename("weather\\heavy_rain.wav");
				mRainSFX->SetRepeat(true);
				mRainSFX->SetVolumeFadeDimensions(Vector2(50000, 3000));
		
				GameObjectManager::Instance()->AddGameObject(mRainSFX);

				mLightningLayer = new ParallaxLayer(Camera2D::GetInstance());
				mLightningLayer->m_textureFilename = "Media\\lightning_layer.png";
				mLightningLayer->m_isAnimated = false;
				mLightningLayer->m_animationFile = "";
				mLightningLayer->m_drawAtNativeDimensions = true;
				mLightningLayer->m_updateable = true;
				mLightningLayer->m_position = Vector3(0, -220 * gameScale, 1);
				mLightningLayer->m_dimensions = Vector3(2048, 2048, 0);
				mLightningLayer->mRepeatTextureX = false;
				mLightningLayer->mRepeatTextureY = false;
				mLightningLayer->m_repeatWidth = 2048;
				mLightningLayer->m_cameraParallaxMultiplierX = 0.0f;
				mLightningLayer->m_cameraParallaxMultiplierY = 0.0f;
				mLightningLayer->m_followCamXPos = true;
				mLightningLayer->m_followCamYPos = true;
				mLightningLayer->m_autoScrollY = false;
				mLightningLayer->m_autoScrollX = false;
				mLightningLayer->m_autoScrollXSpeed = 0;
				mLightningLayer->m_autoScrollYSpeed = 0;
				mLightningLayer->EffectName = "effectlighttexture";
				mLightningLayer->m_alpha = 0.0f;
				GameObjectManager::Instance()->AddGameObject(mLightningLayer);

				break;
			}
		case kSnowing:
			{
				mBottomSnowLayer = new ParallaxLayer(Camera2D::GetInstance());
				mBottomSnowLayer->m_textureFilename = "Media\\snowlayer.png";
				mBottomSnowLayer->m_drawAtNativeDimensions = true;
				mBottomSnowLayer->m_updateable = true;
				mBottomSnowLayer->m_position = Vector3(0, 598 * gameScale, 15);
				mBottomSnowLayer->m_dimensions = Vector3(2048, 2048, 0);
				mBottomSnowLayer->mRepeatTextureX = false;
				mBottomSnowLayer->mRepeatTextureY = false;
				mBottomSnowLayer->m_repeatWidth = 2048;
				mBottomSnowLayer->m_cameraParallaxMultiplierX = 0.000588;
				mBottomSnowLayer->m_cameraParallaxMultiplierY = 0;
				mBottomSnowLayer->m_followCamXPos = true;
				mBottomSnowLayer->m_followCamYPos = false;
				mBottomSnowLayer->m_autoScrollY = true;
				mBottomSnowLayer->m_autoScrollX = false;
				mBottomSnowLayer->m_autoScrollXSpeed = 0;
				mBottomSnowLayer->m_autoScrollYSpeed = 6.0f;
				mBottomSnowLayer->EffectName = "effectnoise";
				mBottomSnowLayer->m_alpha = 1.0f;
				mBottomSnowLayer->mNoiseShaderIntensity = 0.001f;
				GameObjectManager::Instance()->AddGameObject(mBottomSnowLayer);

				mTopSnowLayer = new ParallaxLayer(Camera2D::GetInstance());
				mTopSnowLayer->m_textureFilename = "Media\\snowlayer.png";
				mTopSnowLayer->m_drawAtNativeDimensions = true;
				mTopSnowLayer->m_updateable = true;
				mTopSnowLayer->m_position = Vector3(0, 2650 * gameScale, 15);
				mTopSnowLayer->m_dimensions = Vector3(2048, 2048, 0);
				mTopSnowLayer->mRepeatTextureX = false;
				mTopSnowLayer->mRepeatTextureY = false;
				mTopSnowLayer->m_repeatWidth = 2048;
				mTopSnowLayer->m_cameraParallaxMultiplierX = 0.000588;
				mTopSnowLayer->m_cameraParallaxMultiplierY = 0;
				mTopSnowLayer->m_followCamXPos = true;
				mTopSnowLayer->m_followCamYPos = false;
				mTopSnowLayer->m_autoScrollY = true;
				mTopSnowLayer->m_autoScrollX = false;
				mTopSnowLayer->m_autoScrollXSpeed = 0;
				mTopSnowLayer->m_autoScrollYSpeed = 6.0f;
				mTopSnowLayer->EffectName = "effectnoise";
				mTopSnowLayer->m_alpha = 1.0f;
				mTopSnowLayer->mNoiseShaderIntensity = 0.001f;
				GameObjectManager::Instance()->AddGameObject(mTopSnowLayer);

				mSnowSFX = new AudioObject(0, 1500,0,50000, 3000);
				mSnowSFX->SetAdjustVolumeToCamera(false);
				mSnowSFX->SetAudioFilename("weather\\wind.mp3");
				mSnowSFX->SetRepeat(true);
				mSnowSFX->SetVolumeFadeDimensions(Vector2(50000, 3000));
				GameObjectManager::Instance()->AddGameObject(mSnowSFX);
				break;
			}
		};
	}
}

void WeatherManager::Destroy()
{
	if (mInstance)
	{
		delete mInstance;
		mInstance = nullptr;
	}
}

void WeatherManager::Update(float delta)
{
	for (auto state : mStateKillList)
	{
		mCurrentStates.remove(state);
	}
	mStateKillList.clear();

	for (auto state : mCurrentStates)
	{
		switch(state)
		{
			case kRaining:
				{
					UpdateRaining(delta);
					break;
				}
			case kSnowing:
				{
					UpdateSnowing(delta);
					break;
				}
			case kNone:
			deafult:
				{
					UpdateNoWeather(delta);
					break;
				}
		};
	}

	if (mCurrentStates.size() == 0)
	{
		UpdateNoWeather(delta);
	}

	mElapsedTime += delta;
}

void WeatherManager::StartRaining()
{
	mFoliageSwayMultiplier = kRainFoliageSwayMultiplier;

	mHasRained = true;

	float gameScale = Game::GetGameScale().X;

	if (!mBottomRainLayer)
	{
		mBottomRainLayer = new ParallaxLayer(Camera2D::GetInstance());

		mBottomRainLayer->m_textureFilename = "Media\\rainlayer.png";
		mBottomRainLayer->m_drawAtNativeDimensions = true;
		mBottomRainLayer->m_updateable = true;
		mBottomRainLayer->m_position = Vector3(0, 598 * gameScale, 15);
		mBottomRainLayer->m_dimensions = Vector3(2048, 2048, 0);
		mBottomRainLayer->mRepeatTextureX = false;
		mBottomRainLayer->mRepeatTextureY = false;
		mBottomRainLayer->m_repeatWidth = 2048;
		mBottomRainLayer->m_cameraParallaxMultiplierX = 0.000488;
		mBottomRainLayer->m_cameraParallaxMultiplierY = 0;
		mBottomRainLayer->m_followCamXPos = true;
		mBottomRainLayer->m_followCamYPos = false;
		mBottomRainLayer->m_autoScrollY = true;
		mBottomRainLayer->m_autoScrollX = false;
		mBottomRainLayer->m_autoScrollXSpeed = 0;
		mBottomRainLayer->m_autoScrollYSpeed = 6.5f;
		mBottomRainLayer->EffectName = "effectlighttexture";
		mBottomRainLayer->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mBottomRainLayer);
	}

	if (!mTopRainLayer)
	{
		mTopRainLayer = new ParallaxLayer(Camera2D::GetInstance());

		mTopRainLayer->m_textureFilename = "Media\\rainlayer.png";
		mTopRainLayer->m_drawAtNativeDimensions = true;
		mTopRainLayer->m_updateable = true;
		mTopRainLayer->m_position = Vector3(0, 2650 * gameScale, 15);
		mTopRainLayer->m_dimensions = Vector3(2048, 2048, 0);
		mTopRainLayer->mRepeatTextureX = false;
		mTopRainLayer->mRepeatTextureY = false;
		mTopRainLayer->m_repeatWidth = 1920;
		mTopRainLayer->m_cameraParallaxMultiplierX = 0.000488;
		mTopRainLayer->m_cameraParallaxMultiplierY = 0;
		mTopRainLayer->m_followCamXPos = true;
		mTopRainLayer->m_followCamYPos = false;
		mTopRainLayer->m_autoScrollY = true;
		mTopRainLayer->m_autoScrollX = false;
		mTopRainLayer->m_autoScrollXSpeed = 0;
		mTopRainLayer->m_autoScrollYSpeed = 6.5f;
		mTopRainLayer->EffectName = "effectlighttexture";
		mTopRainLayer->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mTopRainLayer);
	}

	if (!mGroundRainLayer)
	{
		mGroundRainLayer = new ParallaxLayer(Camera2D::GetInstance());

		mGroundRainLayer->m_textureFilename = "";
		mGroundRainLayer->m_isAnimated = true;
		mGroundRainLayer->m_animationFile = "XmlFiles\\ground_rain.xml";
		mGroundRainLayer->m_drawAtNativeDimensions = true;
		mGroundRainLayer->m_updateable = true;
		mGroundRainLayer->m_position = Vector3(0, -220 * gameScale, 99);
		mGroundRainLayer->m_dimensions = Vector3(2048, 2048, 0);
		mGroundRainLayer->mRepeatTextureX = false;
		mGroundRainLayer->mRepeatTextureY = false;
		mGroundRainLayer->m_repeatWidth = 2048;
		mGroundRainLayer->m_cameraParallaxMultiplierX = 0.000488;
		mGroundRainLayer->m_cameraParallaxMultiplierY = 0;
		mGroundRainLayer->m_followCamXPos = true;
		mGroundRainLayer->m_followCamYPos = false;
		mGroundRainLayer->m_autoScrollY = false;
		mGroundRainLayer->m_autoScrollX = false;
		mGroundRainLayer->m_autoScrollXSpeed = 0;
		mGroundRainLayer->m_autoScrollYSpeed = 0;
		mGroundRainLayer->EffectName = "effectlighttexture";
		mGroundRainLayer->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mGroundRainLayer);
	}

	// add the audio object
	if (!mRainSFX)
	{
		mRainSFX = new AudioObject(0, 1500,0,50000, 3000);
		mRainSFX->SetAdjustVolumeToCamera(false);
		mRainSFX->SetAudioFilename("weather\\heavy_rain.wav");
		mRainSFX->SetRepeat(true);
		mRainSFX->SetVolumeFadeDimensions(Vector2(50000, 3000));
		
		GameObjectManager::Instance()->AddGameObject(mRainSFX);
	}

	if (!mLightningLayer)
	{
		mLightningLayer = new ParallaxLayer(Camera2D::GetInstance());

		mLightningLayer->m_textureFilename = "Media\\lightning_layer.png";
		mLightningLayer->m_isAnimated = false;
		mLightningLayer->m_animationFile = "";
		mLightningLayer->m_drawAtNativeDimensions = true;
		mLightningLayer->m_updateable = true;
		mLightningLayer->m_position = Vector3(0, -220 * gameScale, 1);
		mLightningLayer->m_dimensions = Vector3(2048, 2048, 0);
		mLightningLayer->mRepeatTextureX = false;
		mLightningLayer->mRepeatTextureY = false;
		mLightningLayer->m_repeatWidth = 2048;
		mLightningLayer->m_cameraParallaxMultiplierX = 0.0f;
		mLightningLayer->m_cameraParallaxMultiplierY = 0.0f;
		mLightningLayer->m_followCamXPos = true;
		mLightningLayer->m_followCamYPos = true;
		mLightningLayer->m_autoScrollY = false;
		mLightningLayer->m_autoScrollX = false;
		mLightningLayer->m_autoScrollXSpeed = 0;
		mLightningLayer->m_autoScrollYSpeed = 0;
		mLightningLayer->EffectName = "effectlighttexture";
		mLightningLayer->m_alpha = 0.0f;

		GameObjectManager::Instance()->AddGameObject(mLightningLayer);
	}

	if (!HasCurrentWeatherState(kRaining))
	{
		mCurrentStates.push_back(kRaining);
		mRainStartTime = mElapsedTime;

		unsigned range = kRainSessionMaxTime * 100.0f - kRainSessionMinTime * 100.0f;
		float randTime = rand() % range;
		randTime *= 0.01;

		mCurrentRainSessionTime = kRainSessionMinTime + randTime;

		// play thunder sound to start the rainfall
		AudioManager::Instance()->PlaySoundEffect("weather\\thunder.wav");
	}
}

void WeatherManager::DoLightningEffect()
{
	mLightningStartTime = mElapsedTime;
	mPLayingLightningEffect = true;

	AudioManager::Instance()->PlaySoundEffect("weather\\thunder2.wav");

	unsigned int range = kLightningMaxDelay * 100.0f - kLightningMinDelay * 100.0f;
	float randTime = rand() % range;
	randTime *= 0.01f;

	mTimeUntilNextLightning = mElapsedTime + (kLightningMinDelay + randTime);
}

void WeatherManager::StopRaining()
{
	mFoliageSwayMultiplier = kNormalFoliageSwayMultiplier;

	if (mTopRainLayer)
	{
		GameObjectManager::Instance()->RemoveGameObject(mTopRainLayer);
		mTopRainLayer = nullptr;
	}
	if (mBottomRainLayer)
	{
		GameObjectManager::Instance()->RemoveGameObject(mBottomRainLayer);
		mBottomRainLayer = nullptr;
	}
	if (mGroundRainLayer)
	{
		GameObjectManager::Instance()->RemoveGameObject(mGroundRainLayer);
		mGroundRainLayer = nullptr;
	}
	if (mRainSFX)
	{
		GameObjectManager::Instance()->RemoveGameObject(mRainSFX);
		mRainSFX = nullptr;
	}
	if (mLightningLayer)
	{
		GameObjectManager::Instance()->RemoveGameObject(mLightningLayer);
		mLightningLayer = nullptr;
	}


	RemoveState(kRaining);
}

void WeatherManager::RemoveState(WeatherState state)
{
	if (HasCurrentWeatherState(state) && mCurrentStates.size() == 1)
	{
		mLastWeatherTime = mElapsedTime;

		unsigned range = kMaxWeatherIntervalTime * 100.0f - kMinWeatherIntervalTime * 100.0f;
		float randTime = rand() % range;
		randTime *= 0.01;

		mNextWeatherDecisionTime = mElapsedTime + (kMinWeatherIntervalTime + randTime);
	}

	mStateKillList.push_back(state);
}

void WeatherManager::UpdateRaining(float delta)
{
	float stopRainingTime = mRainStartTime + mCurrentRainSessionTime;
	if (stopRainingTime < mElapsedTime)
	{
		float timeUntilTotalStop = mRainStartTime + mCurrentRainSessionTime + kRainOutroTime;
		if (timeUntilTotalStop < mElapsedTime)
		{
			StopRaining();
		}
		else
		{
			float alphaVal = 1.0f - ((mElapsedTime - stopRainingTime) / kRainOutroTime) ;
			mBottomRainLayer->SetAlpha(alphaVal);
			mTopRainLayer->SetAlpha(alphaVal);
			mGroundRainLayer->SetAlpha(alphaVal);
			mRainSFX->SetVolume(alphaVal);
		}
	}
	else
	{
		float timeSinceRain = mElapsedTime - mRainStartTime;
		if (timeSinceRain < kRainIntroTime)
		{
			// fade in intro
			float alphaVal = timeSinceRain / kRainIntroTime;
			mBottomRainLayer->SetAlpha(alphaVal);
			mTopRainLayer->SetAlpha(alphaVal);
			mGroundRainLayer->SetAlpha(alphaVal);
			mRainSFX->SetVolume(alphaVal);
		}
		else 
		{
			mBottomRainLayer->SetAlpha(1.0f);
			mTopRainLayer->SetAlpha(1.0f);
			mGroundRainLayer->SetAlpha(1.0f);
			mRainSFX->SetVolume(1.0f);
		}

		if (mPLayingLightningEffect)
		{
			if (mElapsedTime < mLightningStartTime + kLightningTime)
			{
				float timeSinceLightning = mElapsedTime - mLightningStartTime;
				float alpha = 1.0f - (timeSinceLightning / kLightningTime);
				mLightningLayer->SetAlpha(alpha);
			}
			else
			{
				mLightningLayer->m_alpha = 0.0f;
				mPLayingLightningEffect = false;
			}
		}
		else if (mTimeUntilNextLightning < mElapsedTime)
		{
			DoLightningEffect();
		}
	}
}

void WeatherManager::UpdateSnowing(float delta)
{
	float stopSnowingTime = mSnowStartTime + mCurrentSnowSessionTime;
	if (stopSnowingTime < mElapsedTime)
	{
		float timeUntilTotalStop = mSnowStartTime + mCurrentSnowSessionTime + kSnowOutroTime;
		if (timeUntilTotalStop < mElapsedTime)
		{
			StopSnowing();
		}
		else
		{
			float alphaVal = 1.0f - ((mElapsedTime - stopSnowingTime) / kSnowOutroTime) ;
			mBottomSnowLayer->SetAlpha(alphaVal);
			mTopSnowLayer->SetAlpha(alphaVal);
			mSnowSFX->SetVolume(alphaVal);
		}
	}
	else
	{
		float timeSinceSnow = mElapsedTime - mSnowStartTime;
		if (timeSinceSnow < kSnowIntroTime)
		{
			// fade in intro
			float alphaVal = timeSinceSnow / kSnowIntroTime;
			mBottomSnowLayer->SetAlpha(alphaVal);
			mTopSnowLayer->SetAlpha(alphaVal);
			mSnowSFX->SetVolume(alphaVal);
		}
		else 
		{
			mBottomSnowLayer->SetAlpha(1.0f);
			mTopSnowLayer->SetAlpha(1.0f);
			mSnowSFX->SetVolume(1.0f);
		}
	}
}
void WeatherManager::UpdateNoWeather(float delta)
{
	// always start raining after 20 seconds the first time you play
	if (!mHasHadWeather && mElapsedTime > kTimeUntilFirstWeather)
	{
		StartRaining();
		
		// StartSnowing();

		mHasHadWeather = true;
	}
	else if (mHasHadWeather)
	{
		if (mNextWeatherDecisionTime < mElapsedTime)
		{
			// time to make a weather decision
			if (!mHasRained)
			{
				StartRaining();
			}
			else if (!mHasSnowed)
			{
				StartSnowing();
			}
			else
			{
				// pick some random weather
				int randNum = rand() % 2;

				switch (randNum)
				{
					case 0:
						{
							StartRaining();
							break;
						}
					case 1:
						{
							StartSnowing();
							break;
						}
					default:
						{
							StartRaining();
							break;
						}
				}
			}
		}
	}
}

bool WeatherManager::HasCurrentWeatherState(WeatherState state)
{
	for (auto currState : mCurrentStates)
	{
		if (state == currState)
		{
			return true;
		}
	}

	return false;
}

void WeatherManager::StartSnowing()
{
	mFoliageSwayMultiplier = kSnowFoliageSwayMultiplier;

	mHasSnowed = true;

	float gameScale = Game::GetGameScale().X;

	if (!mBottomSnowLayer)
	{
		mBottomSnowLayer = new ParallaxLayer(Camera2D::GetInstance());

		mBottomSnowLayer->m_textureFilename = "Media\\snowlayer.png";
		mBottomSnowLayer->m_drawAtNativeDimensions = true;
		mBottomSnowLayer->m_updateable = true;
		mBottomSnowLayer->m_position = Vector3(0, 598 * gameScale, 15);
		mBottomSnowLayer->m_dimensions = Vector3(2048, 2048, 0);
		mBottomSnowLayer->mRepeatTextureX = false;
		mBottomSnowLayer->mRepeatTextureY = false;
		mBottomSnowLayer->m_repeatWidth = 2048;
		mBottomSnowLayer->m_cameraParallaxMultiplierX = 0.000588;
		mBottomSnowLayer->m_cameraParallaxMultiplierY = 0;
		mBottomSnowLayer->m_followCamXPos = true;
		mBottomSnowLayer->m_followCamYPos = false;
		mBottomSnowLayer->m_autoScrollY = true;
		mBottomSnowLayer->m_autoScrollX = false;
		mBottomSnowLayer->m_autoScrollXSpeed = 0;
		mBottomSnowLayer->m_autoScrollYSpeed = 6.0f;
		mBottomSnowLayer->EffectName = "effectlighttexture";
		mBottomSnowLayer->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mBottomSnowLayer);
	}

	if (!mTopSnowLayer)
	{
		mTopSnowLayer = new ParallaxLayer(Camera2D::GetInstance());

		mTopSnowLayer->m_textureFilename = "Media\\snowlayer.png";
		mTopSnowLayer->m_drawAtNativeDimensions = true;
		mTopSnowLayer->m_updateable = true;
		mTopSnowLayer->m_position = Vector3(0, 2650 * gameScale, 15);
		mTopSnowLayer->m_dimensions = Vector3(2048, 2048, 0);
		mTopSnowLayer->mRepeatTextureX = false;
		mTopSnowLayer->mRepeatTextureY = false;
		mTopSnowLayer->m_repeatWidth = 2048;
		mTopSnowLayer->m_cameraParallaxMultiplierX = 0.000588;
		mTopSnowLayer->m_cameraParallaxMultiplierY = 0;
		mTopSnowLayer->m_followCamXPos = true;
		mTopSnowLayer->m_followCamYPos = false;
		mTopSnowLayer->m_autoScrollY = true;
		mTopSnowLayer->m_autoScrollX = false;
		mTopSnowLayer->m_autoScrollXSpeed = 0;
		mTopSnowLayer->m_autoScrollYSpeed = 6.0f;
		mTopSnowLayer->EffectName = "effectlighttexture";
		mTopSnowLayer->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mTopSnowLayer);
	}

	// add the audio object
	if (!mSnowSFX)
	{
		mSnowSFX = new AudioObject(0, 1500,0,50000, 3000);
		mSnowSFX->SetAdjustVolumeToCamera(false);
		mSnowSFX->SetAudioFilename("weather\\wind.mp3");
		mSnowSFX->SetRepeat(true);
		mSnowSFX->SetVolumeFadeDimensions(Vector2(50000, 3000));
		
		GameObjectManager::Instance()->AddGameObject(mSnowSFX);
	}

	if (!HasCurrentWeatherState(kSnowing))
	{
		mCurrentStates.push_back(kSnowing);
		mSnowStartTime = mElapsedTime;

		unsigned range = kSnowSessionMaxTime * 100.0f - kSnowSessionMinTime * 100.0f;
		float randTime = rand() % range;
		randTime *= 0.01;

		mCurrentSnowSessionTime = kSnowSessionMinTime + randTime;
	}
}

void WeatherManager::StopSnowing()
{
	mFoliageSwayMultiplier = kNormalFoliageSwayMultiplier;

	if (mTopSnowLayer)
	{
		GameObjectManager::Instance()->RemoveGameObject(mTopSnowLayer);
		mTopSnowLayer = nullptr;
	}
	if (mBottomSnowLayer)
	{
		GameObjectManager::Instance()->RemoveGameObject(mBottomSnowLayer);
		mBottomSnowLayer = nullptr;
	}
	if (mSnowSFX)
	{
		GameObjectManager::Instance()->RemoveGameObject(mSnowSFX);
		mSnowSFX = nullptr;
	}

	RemoveState(kSnowing);
}
