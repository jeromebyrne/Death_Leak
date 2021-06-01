#include "precompiled.h"
#include "KeyPickup.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"
#include "SaveManager.h"
#include "Game.h"
#include "StringManager.h"

static const string kKeyPickupSFX = "key_pickup.wav";

void KeyPickup::DoPickup()
{
	AudioManager::Instance()->PlaySoundEffect(kKeyPickupSFX);

	Player * p = GameObjectManager::Instance()->GetPlayer();

	if (p == nullptr)
	{
		return;
	}

	if (!mKeyId.empty())
	{
		SaveManager::GetInstance()->SetHasDoorkey(mKeyId, true);
	}

	DoPickupEffects(p);

	GameObjectManager::Instance()->RemoveGameObject(this);

	auto strMan = StringManager::GetInstance();
	
	Game::GetInstance()->DisplayTextModal(strMan->GetLocalisedString(mKeyNameLocId.c_str()), 
											strMan->GetLocalisedString(mKeyDescLocId.c_str()));
}

void KeyPickup::DoPickupEffects(Player * player)
{
	// TODO:

	Game::GetInstance()->Vibrate(0.1f, 0.0f, 0.1f);
}

void KeyPickup::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	mKeyId = XmlUtilities::ReadAttributeAsString(element, "key_props", "id");
	mKeyNameLocId = XmlUtilities::ReadAttributeAsString(element, "key_props", "name_loc_id");
	mKeyDescLocId = XmlUtilities::ReadAttributeAsString(element, "key_props", "desc_loc_id");
}

void KeyPickup::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	TiXmlElement * keyElem = new TiXmlElement("key_props");
	keyElem->SetAttribute("id", mKeyId.c_str());
	keyElem->SetAttribute("name_loc_id", mKeyNameLocId.c_str());
	keyElem->SetAttribute("desc_loc_id", mKeyDescLocId.c_str());
	element->LinkEndChild(keyElem);
}

void KeyPickup::Update(float delta)
{
#if _DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		Pickup::Update(delta);
		return;
	}
#endif

	if (!mHasInitCheckedCollected)
	{
		if (SaveManager::GetInstance()->HasDoorKey(mKeyId))
		{
			GameObjectManager::Instance()->RemoveGameObject(this, true);
			return;
		}

		mHasInitCheckedCollected = true;
	}

	Pickup::Update(delta);
}