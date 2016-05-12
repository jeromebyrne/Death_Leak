#ifndef STRINGMANAGER_H
#define STRINGMANAGER_H

#include <map>

using namespace std;

class StringManager
{
public:

	StringManager(void);
	virtual ~StringManager(void);

	static void Create();

	static StringManager * GetInstance() { return mInstance; }

	void LoadStringsFile(const char * file);

	void SetLocale(string & locale);

	string GetLocalisedString(const char * key);

private:

	map<string, map<string, string>> mStringsMap;

	map<string, string> mLocaleStrings;

	static StringManager * mInstance;

	std::string mCurrentLocale;
};

#endif

