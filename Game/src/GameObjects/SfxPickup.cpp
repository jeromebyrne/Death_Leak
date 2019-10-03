#include "precompiled.h"
#include "SfxPickup.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "Game.h"

void SfxPickup::DoPickup()
{
	PlayRandomSfx();

	mInteractableProperties.InteractCountdown = 2.0f;
	mInteractableProperties.DisableInteractivityOnInteract = false;
}

void SfxPickup::Initialise()
{
	Pickup::Initialise();

	// hardcoding koto sfx
	mSfxList.reserve(10);
	mSfxList.push_back("koto\\koto.wav");
	mSfxList.push_back("koto\\koto_2.wav");
	mSfxList.push_back("koto\\koto_3.wav");
	mSfxList.push_back("koto\\koto_4.wav");
	mSfxList.push_back("koto\\koto_5.wav");
}

void SfxPickup::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	/*
	mKeyId = XmlUtilities::ReadAttributeAsString(element, "key_props", "id");
	mKeyNameLocId = XmlUtilities::ReadAttributeAsString(element, "key_props", "name_loc_id");
	mKeyDescLocId = XmlUtilities::ReadAttributeAsString(element, "key_props", "desc_loc_id");
	*/
}

void SfxPickup::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	/*
	TiXmlElement * keyElem = new TiXmlElement("key_props");
	keyElem->SetAttribute("id", mKeyId.c_str());
	keyElem->SetAttribute("name_loc_id", mKeyNameLocId.c_str());
	keyElem->SetAttribute("desc_loc_id", mKeyDescLocId.c_str());
	element->LinkEndChild(keyElem);
	*/
}

void SfxPickup::Update(float delta)
{
	Pickup::Update(delta);
}

bool SfxPickup::CanInteract()
{
	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player == nullptr)
	{
		return false;
	}

	return player->IsOnSolidSurface();
}

void SfxPickup::PlayRandomSfx()
{
	int randNum = rand() % mSfxList.size();

	AudioManager::Instance()->PlaySoundEffect(mSfxList[randNum]);
}