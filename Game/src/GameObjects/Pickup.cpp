#include "precompiled.h"
#include "Pickup.h"

Pickup::~Pickup()
{
}

bool Pickup::OnCollision(SolidMovingSprite * object)
{
	return false;
}

void Pickup::Initialise()
{
	mIsPickup = true;

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(0.0f, 40.0f);
	mInteractableProperties.CurrentScale = 0.75f;

	SolidMovingSprite::Initialise();
}

void Pickup::Update(float delta)
{
	SolidMovingSprite::Update(delta);
}

void Pickup::OnInteracted()
{
	DoPickup();
	mIsPickedUp = true;
}
