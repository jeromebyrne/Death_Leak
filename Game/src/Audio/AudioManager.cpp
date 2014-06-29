#include "precompiled.h"
#include "AudioManager.h"

AudioManager* AudioManager::m_instance = 0;

AudioManager::AudioManager(void):
m_musicMuted(false),
m_effectsMuted(false)
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
	m_irrKlangEngine->drop();
}

ISound * AudioManager::PlaySoundEffect(string fileName, bool loop, bool track)
{
	if (!m_effectsMuted)
	{
		string file = m_audioPath + fileName;

		bool manuallyTrackSloMo = false;
		float timeMod = Timing::Instance()->GetTimeModifier();
		if (!track)
		{
			manuallyTrackSloMo = true;
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
			sound->setPlaybackSpeed(timeMod);

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
	if (!m_musicMuted)
	{
		string file = m_audioPath + fileName;
		m_irrKlangEngine->play2D(file.c_str(), loop);
	}
}

void AudioManager::StopAllSounds()
{
	m_irrKlangEngine->stopAllSounds();
}

void AudioManager::Update()
{
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

