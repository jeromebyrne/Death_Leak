#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "DataValue.h"

class SaveManager
{	
public:
	static SaveManager * GetInstance();

	void ReadSaveFile();

	void WriteSaveFile();

	int GetNumCurrencyOrbsCollected() const;

	void SetNumCurrencyOrbsCollected(int value);

	int GetNumTimesGameCompleted() const;

	void SetNumTimesGameCompleted(int value);

	void SetCurrencyOrbsCollected(const std::string & levelFile, std::vector<unsigned int> orbGameIds);

	void GetCurrencyOrbsCollected(const std::string & levelFile, std::vector<unsigned int> & orbGameIdsOut);

	void GetBreakablesBroken(const std::string & levelFile, std::vector<unsigned int> & breakableIdsOut);

	void SetBreakablesBroken(const std::string & levelFile, std::vector<unsigned int> breakableIds);

	bool IsGameFeatureUnlocked(const int featureType);

	void SetGameFeatureUnlocked(const int featureType);

	void SetPaperPickupCollected(const string & loc_id);

	bool IsPaperPickupCollected(const string & loc_id);

	void SetSmashableBroken(const string& levelId);

	bool IsSmashableBroken(const string& levelId);

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

	void ResetSession();

private:

	static SaveManager * mInstance;

	SaveManager();
	~SaveManager(void) { }

	int GetIntValue(std::map<std::string, DataValue> dataMap, const std::string & key, int defaultValue = 0) const;

	int GetDoubleValue(std::map<std::string, DataValue> dataMap, const std::string& key, double defaultValue = 0.0) const;

	bool GetBoolValue(std::map<std::string, DataValue> dataMap, const std::string & key, bool defaultValue = false) const;

	string GetStringValue(std::map<std::string, DataValue> dataMap, const std::string & key, string defaultValue = "") const;

	const DataValue ReadValue(TiXmlElement * xmlElement);

	std::map<std::string, DataValue> mSaveMapPermanentData;
	std::map<std::string, DataValue> mSaveMapTemporaryData;

};

#endif
