#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "DataValue.h"

class SaveManager
{	
public:
	static SaveManager * GetInstance();

	void ReadSaveFile();

	void WriteSaveFile();

	void AddKeyValuePair(const std::string & key, const DataValue & value);

private:

	static SaveManager * mInstance;

	SaveManager();
	~SaveManager(void) { }

	void WriteValue(const DataValue & value, TiXmlElement * xmlElement);

	std::map<std::string, DataValue> mSaveMap;
};

#endif
