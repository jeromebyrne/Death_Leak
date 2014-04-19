#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include "Material.h"

class MaterialManager
{
private:
	static MaterialManager* m_instance;
	map<string, Material*> m_materialMap;
public:
	MaterialManager(void);
	~MaterialManager(void);

	static MaterialManager* Instance();

	void Initialise(char* materialXMLFile);
	void Release();
	Material * GetMaterial(string name);
};

#endif
