#include "precompiled.h"
#include "SaveManager.h"

SaveManager * SaveManager::mInstance = nullptr;
static const char * saveFilename = "XmlFiles\\save.xml";

SaveManager::SaveManager()
{
}

SaveManager * SaveManager::GetInstance()
{
	if(!mInstance)
	{
		mInstance = new SaveManager();
	}

	return mInstance;
}

void SaveManager::ReadSaveFile()
{
	XmlDocument root_doc;
	if (!root_doc.Load(saveFilename))
	{
		WriteSaveFile();
		return;
	}

	TiXmlElement * rootElement = root_doc.GetRoot();

	// mAudioSettings.MusicOn = XmlUtilities::ReadAttributeAsBool(rootElement, "audio", "music_enabled");
	// mAudioSettings.SfxOn = XmlUtilities::ReadAttributeAsBool(rootElement, "audio", "sfx_enabled");
}

void SaveManager::WriteSaveFile()
{
	XmlDocument root_doc;
	TiXmlElement * root = new TiXmlElement("save_data");

	// audio properties
	// TiXmlElement * audioProps = new TiXmlElement("audio");
	// audioProps->SetAttribute("music_enabled", mAudioSettings.MusicOn);
	// audioProps->SetAttribute("sfx_enabled", mAudioSettings.SfxOn);
	// root->LinkEndChild(audioProps);

	root_doc.Save(saveFilename, root);
}