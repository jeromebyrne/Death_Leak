#ifndef SETTINGS_H
#define SETTINGS_H

class Settings
{	
public:
	static Settings * GetInstance();

	void ReadSettingsFile();

	void ApplySettings();

	void SetSfxEnabled(bool value, bool writeSettings);

	void SetMusicEnabled(bool value, bool writeSettings);

private:
	static Settings * mInstance;

	void WriteSettings();

	Settings();
	~Settings(void) {}

	struct AudioSettings
	{
		bool SfxOn;
		bool MusicOn;

		AudioSettings()
		{
			SfxOn = true;
			MusicOn = true;
		}
	};

	AudioSettings mAudioSettings;
};

#endif
