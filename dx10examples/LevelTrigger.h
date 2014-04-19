#ifndef LEVELTRIGGER_H
#define LEVELTRIGGER_H

#include "gameobject.h"

class LevelTrigger : public GameObject
{
public:
	LevelTrigger(float x = 0, float y = 0, float z = 0, float width = 1, float height = 1);
	virtual ~LevelTrigger();

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	std::string mLevelToLoad;

};

#endif

