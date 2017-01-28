#include "precompiled.h"
#include "WeatherManager.h"
#include "ParallaxLayer.h"
#include "AudioManager.h"
#include "AudioObject.h"
#include "Game.h"

static const float kTimeUntilFirstWeather = 10.0f;
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

static const float kNormalFoliageSwayMultiplier = 2.5f;
static const float kRainFoliageSwayMultiplier = 4.0f;
static const float kSnowFoliageSwayMultiplier = 6.0f;

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
	mFoliageSwayMultiplier(1.0f),
	mRainLayer3(nullptr),
	mSnowLayer3(nullptr),
	mAllowWeather(true)
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
				mBottomRainLayer = nullptr;
				mTopRainLayer = nullptr;
				mRainLayer3 = nullptr;
				mGroundRainLayer = nullptr;
				mRainSFX = nullptr;
				mLightningLayer = nullptr;
				CreateRainAssets();

				break;
			}
		case kSnowing:
			{
				mBottomSnowLayer = nullptr;
				mTopSnowLayer = nullptr;
				mSnowSFX = nullptr;
				mSnowLayer3 = nullptr;
				CreateSnowAssets();

				break;
			}
		};
	}
}

void WeatherManager::CreateRainAssets()
{
	float gameScale = Game::GetGameScale().X;

	if (!mBottomRainLayer)
	{
		mBottomRainLayer = new ParallaxLayer(Camera2D::GetInstance());

		mBottomRainLayer->m_textureFilename = "Media\\rainlayer.png";
		mBottomRainLayer->m_drawAtNativeDimensions = true;
		mBottomRainLayer->m_updateable = true;
		mBottomRainLayer->m_position = Vector3(0, 450 * gameScale, 15);
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
		mTopRainLayer->m_position = Vector3(0, 2502 * gameScale, 15);
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

	if (!mRainLayer3)
	{
		mRainLayer3 = new ParallaxLayer(Camera2D::GetInstance());

		mRainLayer3->m_textureFilename = "Media\\rainlayer.png";
		mRainLayer3->m_drawAtNativeDimensions = true;
		mRainLayer3->m_updateable = true;
		mRainLayer3->m_position = Vector3(0, -1598 * gameScale, 15);
		mRainLayer3->m_dimensions = Vector3(2048, 2048, 0);
		mRainLayer3->mRepeatTextureX = false;
		mRainLayer3->mRepeatTextureY = false;
		mRainLayer3->m_repeatWidth = 2048;
		mRainLayer3->m_cameraParallaxMultiplierX = 0.000488;
		mRainLayer3->m_cameraParallaxMultiplierY = 0;
		mRainLayer3->m_followCamXPos = true;
		mRainLayer3->m_followCamYPos = false;
		mRainLayer3->m_autoScrollY = true;
		mRainLayer3->m_autoScrollX = false;
		mRainLayer3->m_autoScrollXSpeed = 0;
		mRainLayer3->m_autoScrollYSpeed = 6.5f;
		mRainLayer3->EffectName = "effectlighttexture";
		mRainLayer3->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mRainLayer3);
	}

	// add the audio object
	if (!mRainSFX)
	{
		mRainSFX = new AudioObject(0, 1500, 0, 50000, 3000);
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
}

void WeatherManager::CreateSnowAssets()
{
	float gameScale = Game::GetGameScale().X;

	if (!mBottomSnowLayer)
	{
		mBottomSnowLayer = new ParallaxLayer(Camera2D::GetInstance());

		mBottomSnowLayer->m_textureFilename = "Media\\snowlayer.png";
		mBottomSnowLayer->m_drawAtNativeDimensions = true;
		mBottomSnowLayer->m_updateable = true;
		mBottomSnowLayer->m_position = Vector3(0, 450 * gameScale, 15);
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
		mBottomSnowLayer->m_autoScrollYSpeed = 2.5f;
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
		mTopSnowLayer->m_position = Vector3(0, 2502 * gameScale, 15);
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
		mTopSnowLayer->m_autoScrollYSpeed = 2.5f;
		mTopSnowLayer->EffectName = "effectlighttexture";
		mTopSnowLayer->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mTopSnowLayer);
	}

	if (!mSnowLayer3)
	{
		mSnowLayer3 = new ParallaxLayer(Camera2D::GetInstance());

		mSnowLayer3->m_textureFilename = "Media\\snowlayer.png";
		mSnowLayer3->m_drawAtNativeDimensions = true;
		mSnowLayer3->m_updateable = true;
		mSnowLayer3->m_position = Vector3(0, -1598 * gameScale, 15);
		mSnowLayer3->m_dimensions = Vector3(2048, 2048, 0);
		mSnowLayer3->mRepeatTextureX = false;
		mSnowLayer3->mRepeatTextureY = false;
		mSnowLayer3->m_repeatWidth = 2048;
		mSnowLayer3->m_cameraParallaxMultiplierX = 0.000488;
		mSnowLayer3->m_cameraParallaxMultiplierY = 0;
		mSnowLayer3->m_followCamXPos = true;
		mSnowLayer3->m_followCamYPos = false;
		mSnowLayer3->m_autoScrollY = true;
		mSnowLayer3->m_autoScrollX = false;
		mSnowLayer3->m_autoScrollXSpeed = 0;
		mSnowLayer3->m_autoScrollYSpeed = 2.5f;
		mSnowLayer3->EffectName = "effectlighttexture";
		mSnowLayer3->m_alpha = 1.0f;

		GameObjectManager::Instance()->AddGameObject(mSnowLayer3);
	}

	// add the audio object
	if (!mSnowSFX)
	{
		mSnowSFX = new AudioObject(0, 1500, 0, 50000, 3000);
		mSnowSFX->SetAdjustVolumeToCamera(false);
		mSnowSFX->SetAudioFilename("weather\\wind.mp3");
		mSnowSFX->SetRepeat(true);
		mSnowSFX->SetVolumeFadeDimensions(Vector2(50000, 3000));

		GameObjectManager::Instance()->AddGameObject(mSnowSFX);
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

	if (!mAllowWeather)
	{
		// pause all weather while weather is not allowed
		return;
	}

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

	FadeWeatherIfApplicable(delta);

	mElapsedTime += delta;
}

void WeatherManager::StartRaining()
{
	mFoliageSwayMultiplier = kRainFoliageSwayMultiplier;

	mHasRained = true;

	CreateRainAssets();
	
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
	if (mRainLayer3 && Camera2D::GetInstance()->Y() < mRainLayer3->Y())
	{
		// if we're below rain layer 3 the we have moved underground
		// don't show lightning effect if underground
		return;
	}
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
	if (mRainLayer3)
	{
		GameObjectManager::Instance()->RemoveGameObject(mRainLayer3);
		mRainLayer3 = nullptr;
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
			mRainLayer3->SetAlpha(alphaVal);
			// mGroundRainLayer->SetAlpha(alphaVal);
			if (Camera2D::GetInstance()->Y() > mRainLayer3->Y())
			{
				mRainSFX->SetVolume(alphaVal);
			}
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
			if (Camera2D::GetInstance()->Y() > mRainLayer3->Y())
			{
				mRainSFX->SetVolume(alphaVal);
				mRainLayer3->SetAlpha(alphaVal);
			}
		}
		else 
		{
			mBottomRainLayer->SetAlpha(1.0f);
			mTopRainLayer->SetAlpha(1.0f);
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
		else if (mTimeUntilNextLightning < mElapsedTime &&
				mRainLayer3 && Camera2D::GetInstance()->Y() > mRainLayer3->Y())
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
			float alphaVal = 1.0f - ((mElapsedTime - stopSnowingTime) / kSnowOutroTime);
			mBottomSnowLayer->SetAlpha(alphaVal);
			mTopSnowLayer->SetAlpha(alphaVal);
			mSnowSFX->SetVolume(alphaVal);
			mSnowLayer3->SetAlpha(alphaVal);
			if (Camera2D::GetInstance()->Y() > mSnowLayer3->Y())
			{
				mSnowSFX->SetVolume(alphaVal);
			}
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
			if (Camera2D::GetInstance()->Y() > mSnowLayer3->Y())
			{
				mSnowSFX->SetVolume(alphaVal);
				mSnowLayer3->SetAlpha(alphaVal);
			}
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
		//StartRaining();
		
		StartSnowing();

		mHasHadWeather = true;
	}
	else if (mHasHadWeather)
	{
		if (mNextWeatherDecisionTime < mElapsedTime)
		{
			StartSnowing();
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

	CreateSnowAssets();

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
	if (mSnowLayer3)
	{
		GameObjectManager::Instance()->RemoveGameObject(mSnowLayer3);
		mSnowLayer3 = nullptr;
	}
	if (mSnowSFX)
	{
		GameObjectManager::Instance()->RemoveGameObject(mSnowSFX);
		mSnowSFX = nullptr;
	}

	RemoveState(kSnowing);
}

void WeatherManager::FadeWeatherIfApplicable(float delta)
{
	float camPosY = Camera2D::GetInstance()->Position().Y;

	if (HasCurrentWeatherState(kRaining))
	{
		// rain layer 3 is the lowest layer and it needs to be faded out before we see the
		// bottom of it because it will just be cut off
		if (mRainLayer3)
		{
			if (camPosY < mRainLayer3->Y())
			{
				float alpha = mRainLayer3->Alpha();
				if (alpha > 0.0f)
				{
					alpha -= delta;
					mRainLayer3->SetAlpha(alpha);
					mRainSFX->SetVolume(alpha);
				}
			}
			else
			{
				float alpha = mRainLayer3->Alpha();
				if (alpha < 1.0f)
				{
					alpha += delta * 1.5f;
					mRainLayer3->SetAlpha(alpha);
					mRainSFX->SetVolume(alpha);
				}
			}
		}
	}
	else if (HasCurrentWeatherState(kSnowing))
	{
		if (mSnowLayer3)
		{
			if (camPosY < mSnowLayer3->Y())
			{
				float alpha = mSnowLayer3->Alpha();
				if (alpha > 0.0f)
				{
					alpha -= delta;
					mSnowLayer3->SetAlpha(alpha);
					mSnowSFX->SetVolume(alpha);
				}
			}
			else
			{
				float alpha = mSnowLayer3->Alpha();
				if (alpha < 1.0f)
				{
					alpha += delta * 1.5f;
					mSnowLayer3->SetAlpha(alpha);
					mSnowSFX->SetVolume(alpha);
				}
			}
		}
	}
}

void WeatherManager::StopAllWeather()
{
	StopRaining();
	StopSnowing();

	mStateKillList.clear();
	mCurrentStates.clear();
}
