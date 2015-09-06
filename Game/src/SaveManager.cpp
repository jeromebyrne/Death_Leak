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

	mSaveMap.clear();

	TiXmlElement * currentChild = rootElement->FirstChildElement();

	while (currentChild)
	{
		mSaveMap[currentChild->Value()] = ReadValue(currentChild);
		currentChild = currentChild->NextSiblingElement();
	}
}

void SaveManager::WriteSaveFile()
{
	XmlDocument root_doc;
	TiXmlElement * root = new TiXmlElement("save_data");

	for (const auto & kvp : mSaveMap)
	{
		TiXmlElement * currentElement = new TiXmlElement(kvp.first.c_str());
		WriteValue(kvp.second, currentElement);
		root->LinkEndChild(currentElement);
	}

	root_doc.Save(saveFilename, root);
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

void SaveManager::AddKeyValuePair(const std::string & key, const DataValue & value)
{
	mSaveMap[key] = value;
}

int SaveManager::GetIntValue(const std::string & key, int defaultValue) const
{
	const auto & iter = mSaveMap.find(key);
	
	if (iter == mSaveMap.end())
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

int SaveManager::GetPlayerLevel() const
{
	return GetIntValue("player_level", 1);
}

void SaveManager::SetPlayerLevel(int value)
{
	mSaveMap["player_level"] = value;
}

int SaveManager::GetNumCurrencyOrbsCollected() const
{
	return GetIntValue("currency_orbs_collected");
}

void SaveManager::SetNumCurrencyOrbsCollected(int value)
{
	mSaveMap["currency_orbs_collected"] = value;
}