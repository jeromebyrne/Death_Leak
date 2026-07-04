#include "precompiled.h"
#include "Settings.h"
#include "XmlDocument.h"
#include "XmlUtilities.h"
#if !(defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC)
	#include "AudioManager.h"
#endif

Settings* Settings::mInstance = nullptr;
static const char * settingsFilename = "XmlFiles\\settings.xml";

Settings::Settings()
{

}

Settings* Settings::GetInstance()
{
	if(!mInstance)
	{
		mInstance = new Settings();
	}

	return mInstance;
}

void Settings::ReadSettingsFile()
{
	ReadSettingsFile(settingsFilename);
}

void Settings::ReadSettingsFile(const std::string& filename)
{
	XmlDocument root_doc;
	if (!root_doc.Load(filename))
	{
		return;
	}

	TiXmlElement * rootElement = root_doc.GetRoot();

	mAudioSettings.MusicOn = XmlUtilities::ReadAttributeAsBool(rootElement, "audio", "music_enabled");
	mAudioSettings.SfxOn = XmlUtilities::ReadAttributeAsBool(rootElement, "audio", "sfx_enabled");
}

void Settings::ApplySettings()
{
#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
	// The Mac port applies these through IAudioSystem once a real audio backend is connected.
#else
	AudioManager::Instance()->SetMusicEnabled(mAudioSettings.MusicOn);
	AudioManager::Instance()->SetSfxEnabled(mAudioSettings.SfxOn);
#endif
}

void Settings::WriteSettings()
{
	WriteSettings(settingsFilename);
}

void Settings::WriteSettings(const std::string& filename)
{
	XmlDocument root_doc;
	TiXmlElement * root = new TiXmlElement("settings");

	// audio properties
	TiXmlElement * audioProps = new TiXmlElement("audio");
	audioProps->SetAttribute("music_enabled", mAudioSettings.MusicOn);
	audioProps->SetAttribute("sfx_enabled", mAudioSettings.SfxOn);
	root->LinkEndChild(audioProps);

	root_doc.Save(filename, root);
}

void Settings::SetSfxEnabled(bool value, bool writeSettings)
{
	mAudioSettings.SfxOn = value;

	if (writeSettings)
	{
		WriteSettings();
	}
}

void Settings::SetMusicEnabled(bool value, bool writeSettings)
{
	mAudioSettings.MusicOn = value;

	if (writeSettings)
	{
		WriteSettings();
	}
}

bool Settings::IsMusicEnabled() const
{
	return mAudioSettings.MusicOn;
}

bool Settings::IsSFXEnabled() const
{
	return mAudioSettings.SfxOn;
}
