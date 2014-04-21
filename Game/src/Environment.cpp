#include "precompiled.h"
#include "Environment.h"
#include "MaterialManager.h"

Environment* Environment::m_instance = 0;

Environment* Environment::Instance()
{
	if(m_instance == 0)
	{
		m_instance = new Environment();
	}

	return m_instance;
}
Environment::Environment(void): m_groundMaterial(0)
{
}

Environment::~Environment(void)
{
}

void Environment::Initialise(int groundLevel)
{
	m_groundLevel = groundLevel;
	m_groundMaterial = MaterialManager::Instance()->GetMaterial("grassground");

}