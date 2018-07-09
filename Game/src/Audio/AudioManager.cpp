#include "precompiled.h"
#include "AudioManager.h"

AudioManager* AudioManager::m_instance = 0;

AudioManager::AudioManager(void):
	mMusicEnabled(false),
	mSfxEnabled(false)
{
}

AudioManager::~AudioManager(void)
{
}

void AudioManager::Initialise()
{
	m_irrKlangEngine = createIrrKlangDevice(ESOD_AUTO_DETECT);

	m_audioPath = "Media\\Audio\\";
	m_irrKlangEngine->setSoundVolume(1.0);
}

void AudioManager::Release()
{
	if (m_irrKlangEngine)
	{
		m_irrKlangEngine->drop();
	}
}

ISound * AudioManager::PlaySoundEffect(string fileName, bool loop, bool track, bool applyTimeMod)
{
	if (mSfxEnabled)
	{
		string file = m_audioPath + fileName;

		bool manuallyTrackSloMo = false;
		float timeMod = Timing::Instance()->GetTimeModifier();
		if (!track)
		{
			if (applyTimeMod)
			{
				manuallyTrackSloMo = true;
			}
			track = true;
		}
		ISound * sound = m_irrKlangEngine->play2D(file.c_str(), loop, false, track);
		if (!sound)
		{
			LOG_ERROR("Sound file not found: %s", fileName.c_str());
			GAME_ASSERT(sound);
		}
		else
		{
			if (applyTimeMod)
			{
				sound->setPlaybackSpeed(timeMod);
			}

			if (manuallyTrackSloMo)
			{
				mSloMoUntrackedSounds.push_back(sound);
			}			
		}
		return sound;
	}

	return nullptr;
}

void AudioManager::PlayMusic(string fileName, bool loop)
{
	if (mMusicEnabled)
	{
		string file = m_audioPath + fileName;
		m_irrKlangEngine->play2D(file.c_str(), loop);
	}
}

void AudioManager::StopAllSounds()
{
	if (m_irrKlangEngine)
	{
		m_irrKlangEngine->stopAllSounds();
	}
}

void AudioManager::Update()
{
	if (!mSfxEnabled)
	{
		return;
	}

	float timeMod = Timing::Instance()->GetTimeModifier();

	list<ISound *> killList;
	for (auto sound : mSloMoUntrackedSounds)
	{
		sound->setPlaybackSpeed(timeMod);

		if (sound->isFinished())
		{
			sound->stop();
			killList.push_back(sound);
		}
	}

	for (auto sound : killList)
	{
		LOG_INFO("TODO: for some reason this was crashing");
		mSloMoUntrackedSounds.remove(sound);
		sound->drop();
		sound = nullptr;
	}
}

