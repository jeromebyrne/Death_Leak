#ifndef LEVELENTRY_H
#define LEVELENTRY_H

#include "gameobject.h"

class LevelEntry : public GameObject
{
public:
	LevelEntry(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 1.0f, float height = 1.0f);
	virtual ~LevelEntry();

	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	void DebugDraw(ID3D10Device *  device) override;

private:

	std::string mLevelCameFrom;
	std::string mDoorIdentifier;
	float mPlayerDirectionX;
};

#endif

