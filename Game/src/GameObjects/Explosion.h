#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "drawableobject.h"
class Explosion : public DrawableObject
{
public:
	Explosion(float damage = 100.0f, float radius = 200.0f, float x = 0, float y = 0, DepthLayer depthLayer, float width = 1.0f, float height = 1.0f, char* effectName = "effectlighttexture");
	virtual ~Explosion(void);

private:

	virtual void Update(float delta) override;

	void PlaySFX();
	void Display();
	void Inflict();

	float mDamage;
	float mRadius;

	void ApplyDamage();
	void ApplyForceToApplicable();

	unsigned int mFramesActive;
};

#endif

