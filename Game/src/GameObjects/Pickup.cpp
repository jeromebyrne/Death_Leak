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
	mInteractableProperties.InteractTime = 0.05f;
	mInteractableProperties.InteractCountdown = mInteractableProperties.InteractTime;
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

bool Pickup::CanInteract()
{
	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player == nullptr)
	{
		return false;
	}

	return player->IsCrouching();
}
