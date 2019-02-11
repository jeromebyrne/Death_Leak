#include "precompiled.h"
#include "Foliage.h"

static float kCollisionSwayTime = 0.25f;

Foliage::Foliage(void)
{
	mIsFoliage = true;
}

Foliage::~Foliage(void)
{
}

void Foliage::LoadContent(ID3D10Device * graphicsdevice)
{
	EffectName = "effectfoliagesway";

	mOriginalSwayMultiplier = 0.0f;
	mCollisionSwayMultiplier = 20.0f;

	mFoliageSwayProperties.TopLeftSwayIntensity.X = 10.0f;
	mFoliageSwayProperties.TopRightSwayIntensity.X = 10.0f;
	mFoliageSwayProperties.TimeMultiplier = mOriginalSwayMultiplier;

	SolidMovingSprite::LoadContent(graphicsdevice);
}

bool Foliage::OnCollision(SolidMovingSprite * object)
{
	if (!object->IsCharacter())
	{
		// only characters affect foliage
		return false;
	}

	Character * c = static_cast<Character *>(object);

	float velocityXSqr = c->GetVelocity().LengthSquared();

	bool isCompletelyInside = c->CollisionLeft() > CollisionLeft() && c->CollisionRight() < CollisionRight();

 	if (velocityXSqr > 5.0f && isCompletelyInside)
	{
		mFoliageSwayProperties.TimeMultiplier = mCollisionSwayMultiplier;

		mCollisionSwayWindDownTime = kCollisionSwayTime;
	}
	return true;
}

void Foliage::Update(float delta)
{
	SolidMovingSprite::Update(delta);

	if (mCollisionSwayWindDownTime > 0.0f)
	{
		mCollisionSwayWindDownTime -= delta;

		if (mCollisionSwayWindDownTime < 0.0f)
		{
			mCollisionSwayWindDownTime = 0.0f;

			mFoliageSwayProperties.TimeMultiplier = mOriginalSwayMultiplier;
			return;
		}
	}
}
