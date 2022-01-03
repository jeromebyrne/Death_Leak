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

	void SetPaperPickupCollected(const string & loc_id);

	bool IsPaperPickupCollected(const string & loc_id);

	bool HasDoorKey(const std::string & keyId);

	void SetHasDoorkey(const std::string & keyId, bool value);

	void SetLanguage(const std::string & langLocaleKey);

	int GetHealthDevilRewardCount();

	void SetHealthDevilRewardCount(int value);

	bool HasHealthDevilGivenReward(const string & healthDevilId);

	void SetHealthDevilGivenReward(const string & healthDevilId, bool value);

	void SetPlayerMaxHealth(const int value);

	void SetDoorWasUnlocked(const string & doorId, bool value);

	bool DoorWasUnlocked(const string & doorId);

	int GetPlayerMaxHealth();

	std::string GetLanguageSet();

	void SetLevelLastSavedAt(const string & levelId);

	string GetLevelLastSavedAt();

	static void WriteValue(const DataValue & value, TiXmlElement * xmlElement);

	bool HasPulledSwordFromStomach();

	void SetHasPulledSwordFromStomach(bool value);

	bool HasRepairTools();

	void SetHasRepairTools(bool value);

	double GetLastTimeNPCSpawnerTriggered(const string & levelName, int objectID);

	void SetLastTimeNPCSpawnerTriggered(const string& levelName, int objectID, double time);

	void WipeSaveFile();

private:

	static SaveManager * mInstance;

	SaveManager();
	~SaveManager(void) { }

	int GetIntValue(const std::string & key, int defaultValue = 0) const;

	int GetDoubleValue(const std::string& key, double defaultValue = 0.0) const;

	bool GetBoolValue(const std::string & key, bool defaultValue = false) const;

	string GetStringValue(const std::string & key, string defaultValue = "") const;

	void AddKeyValuePair(const std::string & key, const DataValue & value);

	const DataValue ReadValue(TiXmlElement * xmlElement);

	std::map<std::string, DataValue> mSaveMap;

};

#endif
