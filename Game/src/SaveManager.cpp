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
					TiXmlElement * elem = new TiXmlElement(Utilities::ConvertDoubleToString(count).c_str());
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

void SaveManager::AddKeyValuePair(const std::string & key, const DataValue & value)
{
	mSaveMap[key] = value;
}