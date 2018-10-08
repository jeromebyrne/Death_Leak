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

	void SetCurrencyOrbsCollected(const std::string & levelFile, std::vector<unsigned int> orbGameIds);

	void GetCurrencyOrbsCollected(const std::string & levelFile, std::vector<unsigned int> & orbGameIdsOut);

	void GetBreakablesBroken(const std::string & levelFile, std::vector<unsigned int> & breakableIdsOut);

	void SetBreakablesBroken(const std::string & levelFile, std::vector<unsigned int> breakableIds);

	bool IsGameFeatureUnlocked(const int featureType);

	void SetGameFeatureUnlocked(const int featureType);

	bool HasDoorKey(const std::string & keyId);

	void SetHasDoorkey(const std::string & keyId, bool value);

	void SetLanguage(const std::string & langLocaleKey);

	int GetHealthDevilRewardCount();

	void SetHealthDevilRewardCount(int value);

	bool HasHealthDevilGivenReward(const string & healthDevilId);

	void SetHealthDevilGivenReward(const string & healthDevilId, bool value);

	void SetPlayerMaxHealth(const int value);

	int GetPlayerMaxHealth();

	std::string GetLanguageSet();

	void SetLevelLastSavedAt(const string & levelId);

	string GetLevelLastSavedAt();

private:

	static SaveManager * mInstance;

	SaveManager();
	~SaveManager(void) { }

	int GetIntValue(const std::string & key, int defaultValue = 0) const;

	bool GetBoolValue(const std::string & key, bool defaultValue = false) const;

	string GetStringValue(const std::string & key, string defaultValue = "") const;

	void AddKeyValuePair(const std::string & key, const DataValue & value);

	void WriteValue(const DataValue & value, TiXmlElement * xmlElement);

	const DataValue ReadValue(TiXmlElement * xmlElement);

	std::map<std::string, DataValue> mSaveMap;
};

#endif
