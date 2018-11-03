#include "precompiled.h"
#include "PositionalAudio.h"
#include "AudioManager.h"

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

PositionalAudio::PositionalAudio()
{
}

PositionalAudio::~PositionalAudio()
{
	DeleteSoundInstance();
}

void PositionalAudio::Initialise(const string & audioFilename,
									bool repeat,
									const Vector2 & dimensions,
									const Vector2 & fadeDimensions)
{
	mAudioFilename = audioFilename;
	mDimensions = dimensions;
	mRepeat = repeat;
	mFadeDimensions = fadeDimensions;

	mIsInitialised = true;
}

void PositionalAudio::Update(float delta, const Vector2 & position)
{
	if (mSoundInstance == nullptr)
	{
		return;
	}

	if (mPlaying == false)
	{
		return;
	}

	if (mIsDisabled)
	{
		return;
	}

	if (!mRepeat && mSoundInstance->isFinished())
	{
		DeleteSoundInstance();
		return;
	}

	Camera2D * camera = Camera2D::GetInstance();
	const Vector3 & camPos = camera->Position();

	bool hasVolume = true;

	float left = position.X - (mDimensions.X * 0.5f);
	float right = position.X + (mDimensions.X * 0.5f);
	float bottom = position.Y - (mDimensions.Y * 0.5f);
	float top = position.Y + (mDimensions.Y * 0.5f);

	float distX = 0.0f;
	if (camPos.X < position.X)
	{
		distX = left - camPos.X;
	}
	else
	{
		distX = camPos.X - right;
	}

	if (camera->IsCameraOriginInsideRect(position, mDimensions))
	{
		mSoundInstance->setVolume(1.0f);
	}
	else if (camera->IsCameraOriginInsideRect(position, Vector2(mFadeDimensions.X, mDimensions.Y))) // TODO: optimize
	{
		float fadeLenghtX = (mFadeDimensions.X - mDimensions.X) * 0.5f;

		float percent = distX / fadeLenghtX;

		mSoundInstance->setVolume(1.0f - percent);
	}
	else if (camera->IsCameraOriginInsideRect(position, Vector2(mDimensions.X, mFadeDimensions.Y))) // TODO: optimize
	{
		float distY = 0.0f;

		if (camPos.Y < position.Y)
		{
			distY = bottom - camPos.Y;
		}
		else
		{
			distY = camPos.Y - top;
		}

		float fadeLenghtY = (mFadeDimensions.Y - mDimensions.Y) * 0.5f; // TODO: optimize

		float percent = distY / fadeLenghtY;

		mSoundInstance->setVolume(1.0f - percent);
	}
	else if (camera->IsCameraOriginInsideRect(position, mFadeDimensions))
	{
		float fadeLenghtX = (mFadeDimensions.X - mDimensions.X) * 0.5f;
		float fadeLenghtY = (mFadeDimensions.Y - mDimensions.Y) * 0.5f;

		float offsetX = 0;
		float offsetY = 0;

		float xPercent = 0.0f;
		float yPercent = 0.0f;

		if (camPos.X > right)
		{
			offsetX = camPos.X - right;
		}
		else if (camPos.X < left)
		{
			offsetX = std::abs(camPos.X - left);
		}

		if (fadeLenghtX > 0.0f)
		{
			xPercent = 1.0f - (offsetX / fadeLenghtX);
		}

		if (camPos.Y > top)
		{
			offsetY = camPos.Y - top;
		}
		else if (camPos.Y < bottom)
		{
			offsetY = std::abs(camPos.Y - bottom);
		}

		if (fadeLenghtY > 0.0f)
		{
			yPercent = 1.0f - (offsetY / fadeLenghtY);
		}

		if (xPercent < yPercent)
		{
			mSoundInstance->setVolume(xPercent);
		}
		else
		{
			mSoundInstance->setVolume(yPercent);
		}
	}
	else
	{
		hasVolume = false;
		mSoundInstance->setVolume(0.0f);
	}

	if (hasVolume)
	{
		float panVal = camPos.X - position.X;
		panVal /= mFadeDimensions.X;
		mSoundInstance->setPan(panVal);

		mSoundInstance->setPlaybackSpeed(Timing::Instance()->GetTimeModifier());
	}
}

void PositionalAudio::Play()
{
	if (mPlaying == true)
	{
		return;
	}

	if (mIsDisabled)
	{
		return;
	}

	if (mSoundInstance == nullptr)
	{
		mSoundInstance = AudioManager::Instance()->PlaySoundEffect(mAudioFilename, mRepeat, true, true);
		// set the volume to 0 at start in case outside of the update zone
		if (mSoundInstance)
		{
			mSoundInstance->setVolume(0.0f);
			mPlaying = true;
		}
	}
}

void PositionalAudio::Stop()
{
	if (mSoundInstance != nullptr)
	{
		mSoundInstance->stop();
	}
	mPlaying = false;
}

void PositionalAudio::DeleteSoundInstance()
{
	if (mSoundInstance)
	{
		mSoundInstance->stop();
		mSoundInstance->drop();
		mSoundInstance = nullptr;
	}
}

bool PositionalAudio::IsInitialised() const
{
	return mIsInitialised;
}

void PositionalAudio::Disable()
{
	mIsDisabled = true;
	Stop();
}