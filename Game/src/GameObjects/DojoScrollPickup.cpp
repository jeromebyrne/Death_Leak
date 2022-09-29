#include "precompiled.h"
#include "DojoScrollPickup.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "Game.h"
#include "UIManager.h"
#include "StringManager.h"
#include "TextObject.h"

static const D3DXCOLOR kCostTextColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
static const D3DXCOLOR kDescTextColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

static const float kCostYOffset = -111.0f;
static const float kCostYOffsetCliffhut = -100.0f;

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

	if (FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(mUnlocksFeature) ||
		(mUnlocksFeature == FeatureUnlockManager::kBombUnlock && SaveManager::GetInstance()->GetNumTimesGameCompleted() < 1)) // bombs only unlock in new game +
	{
		if (sCurrentInteractable == ID())
		{
			// bug fix 
			sCurrentInteractable = -1;
		}

		GameObjectManager::Instance()->RemoveGameObject(this, true);
		m_updateable = false; // this stops us hitting this code again
		return;
	}
}

void DojoScrollPickup::Initialise()
{
	Sprite::Initialise();

	mOrbCost = FeatureUnlockManager::GetInstance()->GetFeatureCost(mUnlocksFeature);
	mOrbCostString = std::to_string(mOrbCost);

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(0.0f, 50.0f);
	mInteractableProperties.DisableInteractivityOnInteract = false;
	mInteractableProperties.InteractCountdown = 0.5f;

	string featureAsString = FeatureUnlockManager::GetInstance()->GetFeatureAsString(mUnlocksFeature);
	string descKey = "dojo_scroll_desc_";
	descKey += featureAsString;
	mLocalizedDescription = StringManager::GetInstance()->GetLocalisedString(descKey.c_str());

	SetDepthLayer(GameObject::kGround);
}

void DojoScrollPickup::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	string featureAsString = XmlUtilities::ReadAttributeAsString(element, "feature_unlock", "value");
	mUnlocksFeature = FeatureUnlockManager::GetInstance()->GetFeatureTypeFromString(featureAsString);

	// Just hardcoding this now as it's easier to set values quicker
	// mOrbCost = XmlUtilities::ReadAttributeAsInt(element, "orb_cost", "value");
}

void DojoScrollPickup::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	string featureAsString = FeatureUnlockManager::GetInstance()->GetFeatureAsString(mUnlocksFeature);

	TiXmlElement * unlockElement = new TiXmlElement("feature_unlock");
	unlockElement->SetAttribute("value", featureAsString.c_str());
	element->LinkEndChild(unlockElement);

	TiXmlElement * orbCostElement = new TiXmlElement("orb_cost");
	orbCostElement->SetAttribute("value", mOrbCost);
	element->LinkEndChild(orbCostElement);
}

void DojoScrollPickup::OnInteracted()
{
	Game::GetInstance()->DisplayUpgradeModal("NEW TECHNIQUE", mLocalizedDescription, mUnlocksFeature, mOrbCost);
}

bool DojoScrollPickup::CanInteract()
{
	if (mUnlocksFeature == FeatureUnlockManager::kNone)
	{
		return false;
	}

	return true;
}