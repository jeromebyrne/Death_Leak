#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include "Material.h"

#include <map>
#include <string>

class MaterialManager
{
public:

	MaterialManager(void);
	~MaterialManager(void);

	static MaterialManager* Instance();

	void Initialise(const char* materialXMLFile);
	void Release();
	Material * GetMaterial(std::string name);

private:

	static MaterialManager* m_instance;
	std::map<std::string, Material*> m_materialMap;
};

#endif
