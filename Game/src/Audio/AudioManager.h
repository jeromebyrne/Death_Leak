#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

using namespace irrklang;

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

class AudioManager
{
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
	irrklang::ISound * PlaySoundEffect(string fileName, bool loop = false, bool track = false, bool applyTimeMod = true);
	void PlayMusic(string fileName, bool loop = false);
	irrklang::ISound* GetSound(string filename); // get a sound object

	void StopAllSounds();

	void Update();
	
	void SetMusicEnabled(bool value) { mMusicEnabled = value; }

	void SetSfxEnabled(bool value) { mSfxEnabled = value; }

private:
	static AudioManager * m_instance;

	string m_audioPath;

	// irrKlang sound engine
	ISoundEngine * m_irrKlangEngine;

	bool mMusicEnabled;
	bool mSfxEnabled;

	// when slow mo is active we want to modify the playback speed
	// however you need to manually track the sound in that case
	list<irrklang::ISound*> mSloMoUntrackedSounds;
};

#endif