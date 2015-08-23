#include "precompiled.h"
#include "SaveManager.h"

SaveManager * SaveManager::mInstance = nullptr;
static const char * saveFilename = "save.xml";

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
}

void SaveManager::WriteSaveFile()
{
	XmlDocument root_doc;
	TiXmlElement * root = new TiXmlElement("save_data");

	for (const auto & kvp : mSaveMap)
	{
		TiXmlElement * currentElement = new TiXmlElement(kvp.first.c_str());
		WriteObject(kvp.second, currentElement);
		root->LinkEndChild(currentElement);
	}

	// audio properties
	// TiXmlElement * audioProps = new TiXmlElement("audio");
	// audioProps->SetAttribute("music_enabled", mAudioSettings.MusicOn);
	// audioProps->SetAttribute("sfx_enabled", mAudioSettings.SfxOn);
	// root->LinkEndChild(audioProps)

	root_doc.Save(saveFilename, root);
}

void SaveManager::WriteObject(const DataValue & value, TiXmlElement * xmlElement)
{
	switch (value.getType())
	{
		case DataValue::Type::BOOLEAN:
			{
				XmlUtilities::
				break;
			}
		case DataValue::Type::INTEGER:
			{
				break;
			}
		case DataValue::Type::STRING:
			{
				break;
			}
		case DataValue::Type::DOUBLE:
		case DataValue::Type::FLOAT:
			{
				break;
			}
		case DataValue::Type::VECTOR:
			{
				break;
			}
		case DataValue::Type::MAP:
			{
				break;
			}
		case DataValue::Type::NONE:
		default:
			{
				GAME_ASSERT(false);
				break;
			}
	}
}