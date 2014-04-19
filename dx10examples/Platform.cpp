#include "precompiled.h"
#include "Platform.h"

Platform::Platform(float x , float y , float z , float width , float height , float breadth ,float groundFriction , float airResistance ):
SolidMovingSprite(x, y, z, width, height, breadth, groundFriction, airResistance)
{
}

Platform::~Platform(void)
{
}
