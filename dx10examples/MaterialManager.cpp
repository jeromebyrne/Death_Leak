#include "precompiled.h"
#include "MaterialManager.h"

MaterialManager * MaterialManager::m_instance = 0;

MaterialManager * MaterialManager::Instance()
{
	if(m_instance == 0)
	{
		m_instance = new MaterialManager();
	}
	return m_instance;
}

MaterialManager::MaterialManager(void)
{
}

MaterialManager::~MaterialManager(void)
{
}

void MaterialManager::Initialise(char *materialXMLFile)
{
	// read through the xml file and load all materials
	XmlDocument doc;
	doc.Load(materialXMLFile);

	TiXmlHandle * hdoc = doc.Handle();
	TiXmlElement * root = hdoc->FirstChildElement().Element();

	TiXmlElement * child = root->FirstChildElement();

	// loop through our materials
	while(child)
	{
		// get the name of the material 
		char* matName = XmlUtilities::ReadAttributeAsString(child, "", "name");
		
		// create the new material
		Material* mat = new Material(matName);
		mat->ReadXml(child); // read the properties

		m_materialMap[matName] = mat; // add to the material map

		// move to the next material
		child = child->NextSiblingElement();
	}
}

void MaterialManager::Release()
{
}

Material* MaterialManager::GetMaterial(string name)
{
	return m_materialMap[name];
}
