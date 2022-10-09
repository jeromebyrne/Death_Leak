#include "precompiled.h"
#include "StringManager.h"
#include <isteamutils.h>

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

	m_IsOnSteamDeck = SteamUtils() != nullptr ? SteamUtils()->IsSteamRunningOnSteamDeck() : false;
}

void StringManager::SetLocale(string & locale)
{
	mCurrentLocale = locale;

	mLocaleStrings = mStringsMap[mCurrentLocale];
	// TODO: maybe do some locale validation
}

string StringManager::GetLocalisedString(const char * key)
{
	if (m_IsOnSteamDeck)
	{
		string altkey = key;
		altkey += "_steamdeck";

		if (mLocaleStrings.find(altkey) != mLocaleStrings.end())
		{
			return mLocaleStrings[altkey];
		}
	}

	return mLocaleStrings[key];
}
