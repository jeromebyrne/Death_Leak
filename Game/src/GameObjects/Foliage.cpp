#include "precompiled.h"
#include "Foliage.h"
#include "AudioManager.h"

static float kCollisionSwayTime = 0.5f;

static const string kFoliageSFX_1 = "foliage_1.wav";

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
	mCollisionSwayMultiplier = 12.0f;

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

 	if (velocityXSqr > 5.0f && isCompletelyInside && mCollisionSwayWindDownTime == 0.0f)
	{
		mFoliageSwayProperties.TimeMultiplier = mCollisionSwayMultiplier;

		AudioManager::Instance()->PlaySoundEffect(kFoliageSFX_1);

		mCollisionSwayWindDownTime = kCollisionSwayTime;
	}
	return true;
}

void Foliage::Update(float delta)
{
	SolidMovingSprite::Update(delta);

	// keep overriding this so it doesn't look weird in rain
	mFoliageSwayProperties.GlobalTimeMultiplier = 1.0f;

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
