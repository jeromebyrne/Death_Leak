#include "precompiled.h"
#include "StringManager.h"

StringManager * StringManager::mInstance = nullptr;

StringManager::StringManager(void) :
	mCurrentLocale("en-gb")
{
}

StringManager::~StringManager(void)
{
}

void StringManager::Create()
{
	if (mInstance != nullptr)
	{
		// shouldn't be creating more than once
		GAME_ASSERT(false);
		return;
	}

	mInstance = new StringManager();
}

void StringManager::LoadStringsFile(const char * file)
{
	mStringsMap.clear();

	XmlDocument root_doc;
	root_doc.Load(file);

	TiXmlElement * stringXml = root_doc.GetRoot()->FirstChildElement();

	while (stringXml)
	{
		string key = stringXml->Value();

		TiXmlElement * langEntriesXml = stringXml->FirstChildElement();

		while (langEntriesXml)
		{
			mStringsMap[langEntriesXml->Value()][key] = langEntriesXml->GetText();

			langEntriesXml = langEntriesXml->NextSiblingElement();
		}

		stringXml = stringXml->NextSiblingElement();
	}

	mLocaleStrings = mStringsMap[mCurrentLocale];
}

void StringManager::SetLocale(string & locale)
{
	mCurrentLocale = locale;

	mLocaleStrings = mStringsMap[mCurrentLocale];
	// TODO: maybe do some locale validation
}

string StringManager::GetLocalisedString(const char * key)
{
	return mLocaleStrings[key];
}
