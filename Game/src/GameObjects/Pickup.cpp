#include "precompiled.h"
#include "Pickup.h"

Pickup::~Pickup()
{
}

bool Pickup::OnCollision(SolidMovingSprite * object)
{
	if (mIsPickedUp == true)
	{
		return false;
	}

	if (!object->IsPlayer())
	{
		return false;
	}

	Player * p = static_cast<Player*>(object);
	
	if (p->IsFullyCrouched())
	{
		DoPickup();
		mIsPickedUp = true;
	}

	return false;
}

void Pickup::Initialise()
{
	mIsPickup = true;

	SolidMovingSprite::Initialise();
}

void Pickup::Update(float delta)
{
	SolidMovingSprite::Update(delta);
}
