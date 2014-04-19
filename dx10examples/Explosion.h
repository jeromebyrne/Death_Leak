#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "drawableobject.h"
class Explosion : public DrawableObject
{
public:
	Explosion(float damage = 100, float radius = 200, float x = 0, float y = 0, float z = 0, float width = 1, float height = 1, float breadth = 1, char* effectName = "effectlighttexture");
	virtual ~Explosion(void);

private:

	float mDamage;
	float mRadius;

	void ApplyDamage();
	void ApplyForceToApplicable();
};

#endif

