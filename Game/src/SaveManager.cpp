#include "precompiled.h"
#include "SaveManager.h"
#include <algorithm>
#include "Game.h"

SaveManager * SaveManager::mInstance = nullptr;
static const char * fname = "bargon.xml";

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
	if (!root_doc.Load(fname, true))
	{
		WriteSaveFile();
		return;
	}

	TiXmlElement * rootElement = root_doc.GetRoot();

	// only permanent items exist in the save file
	mSaveMapPermanentData.clear();

	TiXmlElement * currentChild = rootElement->FirstChildElement();

	while (currentChild)
	{
		mSaveMapPermanentData[currentChild->Value()] = ReadValue(currentChild);
		currentChild = currentChild->NextSiblingElement();
	}
}

void SaveManager::WriteSaveFile()
{
	XmlDocument root_doc;
	TiXmlElement * root = new TiXmlElement("save_data");

	for (const auto & kvp : mSaveMapPermanentData)
	{
		TiXmlElement * currentElement = new TiXmlElement(kvp.first.c_str());
		WriteValue(kvp.second, currentElement);
		root->LinkEndChild(currentElement);
	}

	root_doc.Save(fname, root, true);
}

void SaveManager::WipeSaveFile()
{
	// Just make sure save the language that was set
	string language = GetLanguageSet();

	mSaveMapTemporaryData.clear();
	mSaveMapPermanentData.clear();

	if (language.empty() == false)
	{
		SetLanguage(language);
	}

	WriteSaveFile();
}

void SaveManager::ResetSession()
{
	// wipe all temp data and save permanent data
	mSaveMapTemporaryData.clear();

	WriteSaveFile();
}

void SaveManager::WriteValue(const DataValue & value, TiXmlElement * xmlElement)
{
	switch (value.getType())
	{
		case DataValue::Type::BOOLEAN:
			{
				xmlElement->SetAttribute("type", "bool");
				const char * asStr = value.asBool() ? "true" : "false";
				xmlElement->SetAttribute("value", asStr);
				break;
			}
		case DataValue::Type::INTEGER:
			{
				xmlElement->SetAttribute("type", "int");
				xmlElement->SetAttribute("value", value.asInt());
				break;
			}
		case DataValue::Type::STRING:
			{
				xmlElement->SetAttribute("type", "string");
				xmlElement->SetAttribute("value", value.asString().c_str());
				break;
			}
		case DataValue::Type::DOUBLE:
			{
				xmlElement->SetAttribute("type", "double");
				xmlElement->SetDoubleAttribute("value", value.asDouble());
				break;
			}
		case DataValue::Type::FLOAT:
			{
				xmlElement->SetAttribute("type", "float");
				xmlElement->SetDoubleAttribute("value", value.asFloat());
				break;
			}
		case DataValue::Type::VECTOR:
			{
				xmlElement->SetAttribute("type", "vector");
				
				unsigned count = 1;
				for (const auto & v : value.asVector())
				{
					string elementName = std::string("e_") + Utilities::ConvertDoubleToString(count).c_str();
					TiXmlElement * elem = new TiXmlElement(elementName.c_str());
					WriteValue(v, elem);
					xmlElement->LinkEndChild(elem);
					++count;
				}
				break;
			}
		case DataValue::Type::MAP:
			{
				xmlElement->SetAttribute("type", "map");

				for (const auto & kvp : value.asMap())
				{
					TiXmlElement * elem = new TiXmlElement(kvp.first.c_str());
					WriteValue(kvp.second, elem);
					xmlElement->LinkEndChild(elem);
				}
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

const DataValue SaveManager::ReadValue(TiXmlElement * xmlElement)
{
	std::string type = XmlUtilities::ReadAttributeAsString(xmlElement, "", "type");

	DataValue dataValue;

	if (type == "bool")
	{
		dataValue = XmlUtilities::ReadAttributeAsBool(xmlElement, "", "value");
	}
	else if (type == "int")
	{
		dataValue = (int)XmlUtilities::ReadAttributeAsFloat(xmlElement, "", "value");
	}
	else if (type == "string")
	{
		dataValue = XmlUtilities::ReadAttributeAsString(xmlElement, "", "value");
	}
	else if (type == "double")
	{
		dataValue = (double)XmlUtilities::ReadAttributeAsFloat(xmlElement, "", "value");
	}
	else if (type == "float")
	{
		dataValue = (float)XmlUtilities::ReadAttributeAsFloat(xmlElement, "", "value");
	}
	else if (type == "vector")
	{
		TiXmlElement * currentChild = xmlElement->FirstChildElement();
		unsigned int childCount = 0;
		while (currentChild)
		{
			++childCount;
			currentChild = currentChild->NextSiblingElement();
		}

		std::vector<DataValue> vec;
		vec.reserve(childCount);

		currentChild = xmlElement->FirstChildElement();
		while (currentChild)
		{
			DataValue v = ReadValue(currentChild);
			vec.push_back(v);
			currentChild = currentChild->NextSiblingElement();
		}

		dataValue = vec;
	}
	else if (type == "map")
	{
		std::map<std::string, DataValue> dataMap;
		TiXmlElement * currentChild = xmlElement->FirstChildElement();
		while (currentChild)
		{
			std::string key = currentChild->Value();

			DataValue v = ReadValue(currentChild);

			dataMap[key] = v;

			currentChild = currentChild->NextSiblingElement();
		}

		dataValue = dataMap;
	}
	else
	{
		GAME_ASSERT(false);
	}

	return dataValue;
}

int SaveManager::GetIntValue(std::map<std::string, DataValue> dataMap, const std::string & key, int defaultValue) const
{
	const auto & iter = dataMap.find(key);
	
	if (iter == dataMap.end())
	{
		return defaultValue;
	}

	if (iter->second.getType() != DataValue::Type::INTEGER)
	{
		GAME_ASSERT(false);
		return defaultValue;
	}

	return iter->second.asInt();
}

int SaveManager::GetDoubleValue(std::map<std::string, DataValue> dataMap, const std::string& key, double defaultValue) const
{
	const auto& iter = dataMap.find(key);

	if (iter == dataMap.end())
	{
		return defaultValue;
	}

	if (iter->second.getType() != DataValue::Type::DOUBLE)
	{
		GAME_ASSERT(false);
		return defaultValue;
	}

	return iter->second.asDouble();
}

bool SaveManager::GetBoolValue(std::map<std::string, DataValue> dataMap, const std::string & key, bool defaultValue) const
{
	const auto & iter = dataMap.find(key);

	if (iter == dataMap.end())
	{
		return defaultValue;
	}

	if (iter->second.getType() != DataValue::Type::BOOLEAN)
	{
		GAME_ASSERT(false);
		return defaultValue;
	}

	return iter->second.asBool();
}

string SaveManager::GetStringValue(std::map<std::string, DataValue> dataMap, const std::string & key, string defaultValue) const
{
	const auto & iter = dataMap.find(key);

	if (iter == dataMap.end())
	{
		return defaultValue;
	}

	if (iter->second.getType() != DataValue::Type::STRING)
	{
		GAME_ASSERT(false);
		return defaultValue;
	}

	return iter->second.asString();
}

int SaveManager::GetNumCurrencyOrbsCollected() const
{
	return GetIntValue(mSaveMapTemporaryData, "currency_orbs_collected");
}

void SaveManager::SetNumCurrencyOrbsCollected(int value)
{
	mSaveMapTemporaryData["currency_orbs_collected"] = value;
}

void SaveManager::SetCurrencyOrbsCollected(const std::string & levelFile, std::vector<unsigned int> orbGameIds)
{
	std::string key = levelFile + "_currency_orbs_collected";

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	DataValue orbVector = mSaveMapTemporaryData[key];

	if (orbVector.getType() != DataValue::Type::VECTOR)
	{
		// we haven't created this vector yet
		orbVector = DataValue(std::vector<DataValue>());
	}

	auto vec = orbVector.asVector();

	for (const unsigned int i : orbGameIds)
	{
		vec.push_back(DataValue((int)i));
	}

	mSaveMapTemporaryData[key] = vec;
}

void SaveManager::GetCurrencyOrbsCollected(const std::string & levelFile, std::vector<unsigned int> & orbGameIdsOut)
{
	std::string key = levelFile + "_currency_orbs_collected";

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	DataValue orbVector = mSaveMapTemporaryData[key];

	if (orbVector.getType() != DataValue::Type::VECTOR)
	{
		mSaveMapTemporaryData[key] = DataValue(std::vector<DataValue>());
		return;
	}

	auto vec = orbVector.asVector();

	for (const auto & item : vec)
	{
		if (item.getType() != DataValue::Type::INTEGER)
		{
			continue;
		}

		orbGameIdsOut.push_back(item.asInt());
	}
}

void SaveManager::GetBreakablesBroken(const std::string & levelFile, std::vector<unsigned int> & breakableIdsOut)
{
	std::string key = levelFile + "_breakables_broken";

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	DataValue breakablesVector = mSaveMapPermanentData[key];

	if (breakablesVector.getType() != DataValue::Type::VECTOR)
	{
		mSaveMapPermanentData[key] = DataValue(std::vector<DataValue>());
		return;
	}

	auto vec = breakablesVector.asVector();

	for (const auto & item : vec)
	{
		if (item.getType() != DataValue::Type::INTEGER)
		{
			continue;
		}

		breakableIdsOut.push_back(item.asInt());
	}
}

void SaveManager::SetBreakablesBroken(const std::string & levelFile, std::vector<unsigned int> breakableIds)
{
	std::string key = levelFile + "_breakables_broken";

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	DataValue breakablesVector = mSaveMapPermanentData[key];

	if (breakablesVector.getType() != DataValue::Type::VECTOR)
	{
		// we haven't created this vector yet
		breakablesVector = DataValue(std::vector<DataValue>());
	}

	auto vec = breakablesVector.asVector();

	for (const unsigned int i : breakableIds)
	{
		vec.push_back(DataValue((int)i));
	}

	mSaveMapPermanentData[key] = vec;
}

bool SaveManager::IsGameFeatureUnlocked(const int featureType)
{
	std::string key = "gameplay_features_unlocked";

	DataValue featuresVector = mSaveMapPermanentData[key];

	if (featuresVector.getType() != DataValue::Type::VECTOR)
	{
		featuresVector = DataValue(std::vector<DataValue>());
		mSaveMapPermanentData[key] = featuresVector.asVector();
		return false;
	}

	auto vec = featuresVector.asVector();

	for (const auto & item : vec)
	{
		if (item.getType() != DataValue::Type::INTEGER)
		{
			continue;
		}

		if (item.asInt() == featureType)
		{
			return true;
		}
	}

	return false;
}

void SaveManager::SetGameFeatureUnlocked(const int featureType)
{
	std::string key = "gameplay_features_unlocked";

	DataValue featuresVector = mSaveMapPermanentData[key];

	if (featuresVector.getType() != DataValue::Type::VECTOR)
	{
		// we haven't created this vector yet
		featuresVector = DataValue(std::vector<DataValue>());
	}

	auto vec = featuresVector.asVector();

	for (const auto & val : vec)
	{
		if (val.asInt() == featureType)
		{
			// we already unlocked this so just return
			return;
		}
	}

	vec.push_back(DataValue(featureType));

	mSaveMapPermanentData[key] = vec;
}

void SaveManager::SetPaperPickupCollected(const string & loc_id)
{
	std::string key = "paper_pickups_collected";

	DataValue paperPickupVector = mSaveMapPermanentData[key];

	if (paperPickupVector.getType() != DataValue::Type::VECTOR)
	{
		// we haven't created this vector yet
		paperPickupVector = DataValue(std::vector<DataValue>());
	}

	auto vec = paperPickupVector.asVector();

	for (const auto & val : vec)
	{
		if (val.asString() == loc_id)
		{
			// we already unlocked this so just return
			return;
		}
	}

	vec.push_back(DataValue(loc_id));

	mSaveMapPermanentData[key] = vec;
}

bool SaveManager::IsPaperPickupCollected(const string & loc_id)
{
	std::string key = "paper_pickups_collected";

	DataValue paperPickupVector = mSaveMapPermanentData[key];

	if (paperPickupVector.getType() != DataValue::Type::VECTOR)
	{
		// we haven't created this vector yet
		paperPickupVector = DataValue(std::vector<DataValue>());
		mSaveMapPermanentData[key] = paperPickupVector.asVector();
		return false;
	}

	auto vec = paperPickupVector.asVector();

	for (const auto & item : vec)
	{
		if (item.getType() != DataValue::Type::STRING)
		{
			continue;
		}

		if (item.asString() == loc_id)
		{
			return true;
		}
	}

	return false;
}

bool SaveManager::HasDoorKey(const std::string & keyId)
{
	return GetBoolValue(mSaveMapPermanentData, keyId, false);
}

void SaveManager::SetHasDoorkey(const std::string & keyId, bool value)
{
	mSaveMapPermanentData[keyId] = value;
}

void SaveManager::SetDoorWasUnlocked(const string & doorId, bool value)
{
	mSaveMapPermanentData[doorId + "_unlocked"] = value;
}

bool SaveManager::DoorWasUnlocked(const string & doorId)
{
	return GetBoolValue(mSaveMapPermanentData, doorId + "_unlocked", false);
}

void SaveManager::SetLanguage(const std::string & langLocaleKey)
{
	mSaveMapPermanentData["language_locale_key"] = langLocaleKey;
}

std::string SaveManager::GetLanguageSet()
{
	DataValue asDataValue = mSaveMapPermanentData["language_locale_key"];

	if (asDataValue.getType() != DataValue::Type::STRING)
	{
		return std::string();
	}

	return asDataValue.asString();
}

int SaveManager::GetHealthDevilRewardCount()
{
	return GetIntValue(mSaveMapPermanentData, "health_devil_reward_count");
}

void SaveManager::SetHealthDevilRewardCount(int value)
{
	mSaveMapPermanentData["health_devil_reward_count"] = value;
}

bool SaveManager::HasHealthDevilGivenReward(const string & healthDevilId)
{
	std::string key = "health_devil_reward_" + healthDevilId;

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	return GetBoolValue(mSaveMapPermanentData, key);
}

void SaveManager::SetHealthDevilGivenReward(const string & healthDevilId, bool value)
{
	std::string key = "health_devil_reward_" + healthDevilId;

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	mSaveMapPermanentData[key] = value;
}

void SaveManager::SetPlayerMaxHealth(const int value)
{
	mSaveMapPermanentData["player_max_health"] = value;
}

int SaveManager::GetPlayerMaxHealth()
{
	return GetIntValue(mSaveMapPermanentData, "player_max_health", 0);
}

void SaveManager::SetLevelLastSavedAt(const string & levelId)
{
	// mSaveMap["last_level_saved"] = levelId;
}

string SaveManager::GetLevelLastSavedAt()
{
	return "";
	//return GetStringValue("last_level_saved");
}

bool SaveManager::HasPulledSwordFromStomach()
{
#ifdef DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return true;
	}
#endif

	return GetBoolValue(mSaveMapTemporaryData, "has_ungutted");
}

void SaveManager::SetHasPulledSwordFromStomach(bool value)
{
	mSaveMapTemporaryData["has_ungutted"] = value;
}

bool SaveManager::HasRepairTools()
{
	return GetBoolValue(mSaveMapPermanentData, "has_repair_tools");
}

void SaveManager::SetHasRepairTools(bool value)
{
	mSaveMapPermanentData["has_repair_tools"] = value;
}

double SaveManager::GetLastTimeNPCSpawnerTriggered(const string& levelName, int objectID)
{
	std::string key = levelName + "_npc_" + Utilities::ConvertDoubleToString(objectID);

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	return GetDoubleValue(mSaveMapTemporaryData, key);
}

void SaveManager::SetLastTimeNPCSpawnerTriggered(const string& levelName, int objectID, double time)
{
	std::string key = levelName + "_npc_" + Utilities::ConvertDoubleToString(objectID);

	std::replace(key.begin(), key.end(), '\\', '-'); // replace back slashes as they will mess up the xml file

	mSaveMapTemporaryData[key] = time;
}

