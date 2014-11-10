#include "precompiled.h"
#include "Settings.h"
#include "AudioManager.h"

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
	XmlDocument root_doc;
	if (!root_doc.Load(settingsFilename))
	{
		return;
	}

	TiXmlElement * rootElement = root_doc.GetRoot();

	mAudioSettings.MusicOn = XmlUtilities::ReadAttributeAsBool(rootElement, "audio", "music_enabled");
	mAudioSettings.SfxOn = XmlUtilities::ReadAttributeAsBool(rootElement, "audio", "sfx_enabled");
}

void Settings::ApplySettings()
{
	AudioManager::Instance()->SetMusicEnabled(mAudioSettings.MusicOn);
	AudioManager::Instance()->SetSfxEnabled(mAudioSettings.SfxOn);
}

void Settings::WriteSettings()
{
	XmlDocument root_doc;
	TiXmlElement * root = new TiXmlElement("settings");

	// audio properties
	TiXmlElement * audioProps = new TiXmlElement("audio");
	audioProps->SetAttribute("music_enabled", mAudioSettings.MusicOn);
	audioProps->SetAttribute("sfx_enabled", mAudioSettings.SfxOn);
	root->LinkEndChild(audioProps);

	root_doc.Save(settingsFilename, root);
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