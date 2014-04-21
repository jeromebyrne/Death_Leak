#ifndef PLATFORM_H
#define PLATFORM_H

#include "solidmovingsprite.h"

class Platform : public SolidMovingSprite
{
public:

	Platform(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0,float groundFriction = 1, float airResistance = 1);
	virtual ~Platform(void);

private:

};

#endif
