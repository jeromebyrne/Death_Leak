#ifndef FORCEBOX_H
#define FORCEBOX_H

#include "gameobject.h"

class ForceBox : public GameObject
{
public:
	ForceBox(float x = 0.0f, float y = 0, DepthLayer depthLayer = kPlayer, float width = 1.0f, float height = 1.0f);
	virtual ~ForceBox();

	virtual void Initialise() override;
	virtual void Update(float delta) override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	Vector2 mForceDirection;
	float mForce;
};

#endif

