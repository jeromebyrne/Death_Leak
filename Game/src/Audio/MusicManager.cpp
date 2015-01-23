#include "precompiled.h"
#include "MusicManager.h"
#include "AudioManager.h"

MusicManager::MusicManager(void) :
	mMusicLength(0.0f),
	mInitialDelay(0.0f),
	mTimeBetween(0.0f),
	mHasPlayedMusicOnce(false),
	mTimeUntilPlayAgain(0.0f)
{

}
MusicManager::~MusicManager(void)
{

}

void MusicManager::Initialise(const char * musicFile, float musicLength, float initialDelay, float timeBetween)
{
	mMusicFile = musicFile;
	mMusicLength = musicLength;
	mInitialDelay = initialDelay;
	mTimeBetween = timeBetween;
}

void MusicManager::Update(float delta)
{
	if (!mHasPlayedMusicOnce)
	{
		mInitialDelay -= delta;

		if (mInitialDelay <= 0.0f)
		{
			mHasPlayedMusicOnce = true;
			AudioManager::Instance()->PlayMusic(mMusicFile, false);
			mTimeUntilPlayAgain = mMusicLength + mTimeBetween;
		}

		return;
	}

	mTimeUntilPlayAgain -= delta;
	if (mTimeUntilPlayAgain <= 0.0f)
	{
		AudioManager::Instance()->PlayMusic(mMusicFile, false);
		mTimeUntilPlayAgain = mMusicLength + mTimeBetween;
	}
}