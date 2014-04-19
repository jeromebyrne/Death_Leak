#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include "EffectAbstract.h"

class EffectManager
{
	static EffectManager * m_instance;
	map<std::string, EffectAbstract*> m_effectMap;
	EffectManager(void);
	~EffectManager(void);
public:
	void Initialise(Graphics * graphicsSystem);

	inline static EffectManager * Instance()
	{
		if(m_instance == 0)
		{
			m_instance = new EffectManager();
		}
		return m_instance;
	}

	inline EffectAbstract * GetEffect(char* effectname)
	{
		return m_effectMap[effectname];
	}

	void Release(); // deallocate
};

#endif
