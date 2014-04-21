#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

using namespace irrklang;

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

class AudioManager
{
private:
	static AudioManager * m_instance;

	string m_audioPath;

	// irrKlang sound engine
	ISoundEngine * m_irrKlangEngine;

	bool m_musicMuted;
	bool m_effectsMuted;

	// when slow mo is active we want to modify the playback speed
	// however you need to manually track the sound in that case
	list<ISound*> mSloMoUntrackedSounds;

public:
	AudioManager(void);
	~AudioManager(void);

	inline static AudioManager * Instance()
	{
		if(m_instance == 0)
		{
			m_instance = new AudioManager();
		}
		return m_instance;
	}

	void Initialise();
	void Release();
	ISound * PlaySoundEffect(string fileName, bool loop = false, bool track = false);
	void PlayMusic(string fileName, bool loop = false);
	ISound* GetSound(string filename); // get a sound object

	void StopAllSounds();

	void Update();
	
	void MuteSoundEffects() { m_effectsMuted = true; }
	void UnMuteSoundEffects(){ m_effectsMuted = false; }
	void MuteMusic() { m_musicMuted = true; StopAllSounds();}
	void UnMuteMusic() { m_musicMuted = false; }
};

#endif