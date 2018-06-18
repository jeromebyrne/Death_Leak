#ifndef PLATFORM_H
#define PLATFORM_H

#include "solidmovingsprite.h"

class Platform : public SolidMovingSprite
{
public:

	Platform(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = GameObject::kPlayer, float width = 10.0f, float height = 10.0f,float groundFriction = 1.0f, float airResistance = 1.0f);
	virtual ~Platform(void);

private:

};

#endif
