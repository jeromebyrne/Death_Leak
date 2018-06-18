#include "precompiled.h"
#include "Platform.h"

Platform::Platform(float x , float y , DepthLayer depthLayer , float width , float height ,float groundFriction , float airResistance ):
SolidMovingSprite(x, y, depthLayer, width, height, groundFriction, airResistance)
{
	mIsPlatform = true;
}

Platform::~Platform(void)
{
}
