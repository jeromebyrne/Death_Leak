#ifndef STRINGMANAGER_H
#define STRINGMANAGER_H

#include <map>
#include <string>

class StringManager
{
public:

	StringManager(void);
	virtual ~StringManager(void);

	static void Create();

	static StringManager * GetInstance() { return mInstance; }

	void LoadStringsFile(const char * file);

	void SetLocale(const std::string & locale);

	std::string GetLocalisedString(const char * key);

	void SetIsOnSteamDeck(bool value) { m_IsOnSteamDeck = value; }

private:

	std::map<std::string, std::map<std::string, std::string>> mStringsMap;

	std::map<std::string, std::string> mLocaleStrings;

	static StringManager * mInstance;

	std::string mCurrentLocale;

	bool m_IsOnSteamDeck = false;
};

#endif
