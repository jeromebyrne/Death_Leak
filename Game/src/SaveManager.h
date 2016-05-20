#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "DataValue.h"

class SaveManager
{	
public:
	static SaveManager * GetInstance();

	void ReadSaveFile();

	void WriteSaveFile();

	int GetPlayerLevel() const;

	void SetPlayerLevel(int value);

	int GetNumCurrencyOrbsCollected() const;

	void SetNumCurrencyOrbsCollected(int value);

	void SetOrbsCollected(const std::string & levelFile, std::vector<unsigned int> orbGameIds);

	void GetOrbsCollected(const std::string & levelFile, std::vector<unsigned int> & orbGameIdsOut);

private:

	static SaveManager * mInstance;

	SaveManager();
	~SaveManager(void) { }

	int GetIntValue(const std::string & key, int defaultValue = 0) const;

	void AddKeyValuePair(const std::string & key, const DataValue & value);

	void WriteValue(const DataValue & value, TiXmlElement * xmlElement);

	const DataValue ReadValue(TiXmlElement * xmlElement);

	std::map<std::string, DataValue> mSaveMap;
};

#endif
