#include "precompiled.h"
#include "WeatherManager.h"
#include "ParallaxLayer.h"
#include "AudioManager.h"
#include "AudioObject.h"
#include "Game.h"
#include "ParticleEmitterManager.h"

static const float kTimeUntilFirstWeather = 80.0f;
static const float kRainSessionMinTime = 70.0f;
static const float kRainSessionMaxTime = 120.0f;
static const float kRainIntroTime = 5.0f;
static const float kRainOutroTime = 3.0f;

static const float kLightningTime = 0.4f;
static const float kLightningMinDelay = 10.0f;
static const float kLightningMaxDelay = 30.0f;

static const float kSnowSessionMinTime = 70.0f;
static const float kSnowSessionMaxTime = 90.0f;
static const float kSnowIntroTime = 10.0f;
static const float kSnowOutroTime = 6.0f;

static const float kMinWeatherIntervalTime = 90.0f;
static const float kMaxWeatherIntervalTime = 200.0f;

static const float kNormalFoliageSwayMultiplier = 2.5f;
static const float kRainFoliageSwayMultiplier = 4.0f;
static const float kSnowFoliageSwayMultiplier = 6.0f;

static const Vector2 kRainParticleCamOffset = Vector2(0.0f, 700.0f);

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
	mElapsedTime(0),
	mRainStartTime(0),
	mHasHadWeather(false),
	mCurrentRainSessionTime(0),
	mGroundRainLayer(nullptr),
	mRainSFX(nullptr),
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
	mAllowWeather(true)
{
}

WeatherManager::~WeatherManager(void)
{
}

void WeatherManager::RefreshAssets()
{
	// preload large textures
	TextureManager::Instance()->LoadTexture("Media\\rainlayer.png");
	TextureManager::Instance()->LoadTexture("Media\\snowlayer.png");
	/*
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_1.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_2.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_3.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_4.png");
	TextureManager::Instance()->LoadTexture("Media\\ground\\ground_rain_5.png");
	*/

	float gameScale = Game::GetGameScale().X;

	for (auto state : mCurrentStates)
	{
		switch (state)
		{
		case kRaining:
			{
				mGroundRainLayer = nullptr;
				mRainSFX = nullptr;
				mLightningLayer = nullptr;
				mRainParticleSpray = nullptr;
				CreateRainAssets(true);

				break;
			}
		case kSnowing:
			{
				mSnowSFX = nullptr;
				mSnowParticleSpray = nullptr;
				CreateSnowAssets(true);

				break;
			}
		};
	}
}

void WeatherManager::CreateRainAssets(bool preWarm)
{
	float gameScale = Game::GetGameScale().X;

	// add the audio object
	if (!mRainSFX)
	{
		mRainSFX = new AudioObject(0.0f, 1500.0f, GameObject::kNearForeground, 50000.0f, 3000.0f);
		mRainSFX->SetAdjustVolumeToCamera(false);
		mRainSFX->SetAudioFilename("weather\\heavy_rain.wav");
		mRainSFX->SetRepeat(true);
		mRainSFX->SetVolumeFadeDimensions(Vector2(50000.0f, 3000.0f));

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
		mLightningLayer->m_position = Vector2(0, -220.0f * gameScale);
		mLightningLayer->SetDepthLayer(GameObject::kWeatherForeground);
		mLightningLayer->m_dimensions = Vector2(2048.0f, 2048.0f);
		mLightningLayer->mRepeatTextureX = false;
		mLightningLayer->mRepeatTextureY = false;
		mLightningLayer->m_repeatWidth = 2048.0f;
		mLightningLayer->m_cameraParallaxMultiplierX = 0.0f;
		mLightningLayer->m_cameraParallaxMultiplierY = 0.0f;
		mLightningLayer->m_followCamXPos = true;
		mLightningLayer->m_followCamYPos = true;
		mLightningLayer->m_autoScrollY = false;
		mLightningLayer->m_autoScrollX = false;
		mLightningLayer->m_autoScrollXSpeed = 0.0f;
		mLightningLayer->m_autoScrollYSpeed = 0.0f;
		mLightningLayer->EffectName = "effectlighttexture";
		mLightningLayer->m_alpha = 0.0f;

		GameObjectManager::Instance()->AddGameObject(mLightningLayer);
	}

	if (!mRainParticleSpray)
	{
		mRainParticleSpray = ParticleEmitterManager::Instance()->CreateDirectedSpray(300,
																					Vector2(99999.0f, 9999.0f),
																					GameObject::kWeatherForeground,
																					Vector2(-0.2f, -0.8f),
																					0.0f,
																					Vector2(3200.0f, 2000.0f),
																					"Media\\rain_drop.png",
																					26.0f,
																					32.0f,
																					0.7f,
																					0.85f,
																					28.0f,
																					40.0f,
																					-1.0f,
																					true,
																					0.9f,
																					1.0f,
																					-1.0f,
																					true,
																					1.2f,
																					140.0f,
																					40.0f,
																					0.15f,
																					0.8f);

		if (preWarm)
		{
			mRainParticleSpray->Warm( true);
		}

		mRainParticleSpray->SetAlwaysUpdate(true);

		mRainParticleSpray->AttachToCamera(kRainParticleCamOffset);
	}
}

void WeatherManager::CreateSnowAssets(bool preWarm)
{
	float gameScale = Game::GetGameScale().X;

	// add the audio object
	if (!mSnowSFX)
	{
		mSnowSFX = new AudioObject(0.0f, 1500.0f, GameObject::kNearForeground, 50000.0f, 3000.0f);
		mSnowSFX->SetAdjustVolumeToCamera(false);
		mSnowSFX->SetAudioFilename("weather\\wind.mp3");
		mSnowSFX->SetRepeat(true);
		mSnowSFX->SetVolumeFadeDimensions(Vector2(50000.0f, 3000.0f));

		GameObjectManager::Instance()->AddGameObject(mSnowSFX);
	}

	if (!mSnowParticleSpray)
	{
		mSnowParticleSpray = ParticleEmitterManager::Instance()->CreateDirectedSpray(300,
			Vector2(99999.0f, 9999.0f),
			GameObject::kWeatherForeground,
			Vector2(-0.4f, -0.6f),
			0.9f,
			Vector2(3200.0f, 2000.0f),
			"Media\\snowflake.png",
			1.5f,
			5.5f,
			3.7f,
			5.3f,
			6.0f,
			32.0f,
			-1.0f,
			true,
			0.9f,
			1.0f,
			-0.2f,
			false,
			1.0f,
			200.0f,
			60.0f,
			0.15f,
			0.9f);

		if (preWarm)
		{
			mSnowParticleSpray->Warm(true);
		}

		mSnowParticleSpray->SetAlwaysUpdate(true);

		mSnowParticleSpray->AttachToCamera(kRainParticleCamOffset);
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
	if (mRainParticleSpray)
	{
		mRainParticleSpray->SetIsLooping(false); // will delete itself
		mRainParticleSpray = nullptr;
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
			float alphaVal = 1.0f - ((mElapsedTime - stopRainingTime) / kRainOutroTime);
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
			mRainSFX->SetVolume(alphaVal);
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
			float alphaVal = 1.0f - ((mElapsedTime - stopSnowingTime) / kSnowOutroTime);
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
			mSnowSFX->SetVolume(alphaVal);
		}
		else
		{
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

	if (mSnowSFX)
	{
		GameObjectManager::Instance()->RemoveGameObject(mSnowSFX);
		mSnowSFX = nullptr;
	}

	if (mSnowParticleSpray)
	{
		mSnowParticleSpray->SetIsLooping(false);
		mSnowParticleSpray = nullptr;
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
		/*
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
		*/
	}
	else if (HasCurrentWeatherState(kSnowing))
	{
		/*
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
		*/
	}
}

void WeatherManager::StopAllWeather()
{
	StopRaining();
	StopSnowing();

	mStateKillList.clear();
	mCurrentStates.clear();
}
