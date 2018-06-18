#ifndef LEVELTRIGGER_H
#define LEVELTRIGGER_H

#include "gameobject.h"

class LevelTrigger : public GameObject
{
public:
	LevelTrigger(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 1.0f, float height = 1.0f);
	virtual ~LevelTrigger();

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	std::string mLevelToLoad;

	Vector2 mPlayerStartPos;
	float mPlayerDirectionX;
};

#endif

