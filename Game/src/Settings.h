#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings
{	
public:
	static Settings * GetInstance();

	void ReadSettingsFile();
	void ReadSettingsFile(const std::string& filename);

	void ApplySettings();

	void SetSfxEnabled(bool value, bool writeSettings);

	void SetMusicEnabled(bool value, bool writeSettings);

	bool IsMusicEnabled() const;

	bool IsSFXEnabled() const;

private:
	static Settings * mInstance;

	void WriteSettings();
	void WriteSettings(const std::string& filename);

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
