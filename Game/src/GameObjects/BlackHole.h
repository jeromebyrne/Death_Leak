#ifndef BLACKHOLE_H
#define BLACKHOLE_H

#include "movingsprite.h"

// Black holes are created by the player and are never loaded from file
class BlackHole : public MovingSprite
{
public:
	BlackHole(void);
	virtual ~BlackHole(void);
};

#endif

