#include "precompiled.h"
#include "DojoScrollPickup.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "Game.h"

DojoScrollPickup::DojoScrollPickup(float x,
									float y, 
									DepthLayer 
									depthLayer, 
									float width, 
									float height):
									Sprite(x,y, depthLayer, width, height)
{
}

DojoScrollPickup::~DojoScrollPickup(void)
{
}

void DojoScrollPickup::Update(float delta)
{	
	Sprite::Update(delta);

#ifdef DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}
#endif

	if (FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(mUnlocksFeature))
	{
		GameObjectManager::Instance()->RemoveGameObject(this, true);
		return;
	}
}

void DojoScrollPickup::Initialise()
{
	Sprite::Initialise();

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(0, 0);
}

void DojoScrollPickup::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	string featureAsString = XmlUtilities::ReadAttributeAsString(element, "feature_unlock", "value");

	mUnlocksFeature = FeatureUnlockManager::GetInstance()->GetFeatureTypeFromString(featureAsString);
}

void DojoScrollPickup::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	string featureAsString = FeatureUnlockManager::GetInstance()->GetFeatureAsString(mUnlocksFeature);

	TiXmlElement * unlockElement = new TiXmlElement("feature_unlock");
	unlockElement->SetAttribute("value", featureAsString.c_str());
	element->LinkEndChild(unlockElement);
}

void DojoScrollPickup::OnInteracted()
{
	// TODO: remove the currency
	FeatureUnlockManager::GetInstance()->SetFeatureUnlocked(mUnlocksFeature);
}

bool DojoScrollPickup::CanInteract()
{
	if (mUnlocksFeature == FeatureUnlockManager::kNone)
	{
		return false;
	}

	// TODO: check if we have the currency to purchase

	return true;
}
