#include "precompiled.h"
#include "PaperPickup.h"
#include "AudioManager.h"
#include "Game.h"
#include "StringManager.h"

void PaperPickup::DoPickup()
{
	// TODO: chnage this sound effect
	AudioManager::Instance()->PlaySoundEffect("character/drink_health_upgrade.wav");

	GameObjectManager::Instance()->RemoveGameObject(this);

	Game::GetInstance()->DisplayTextModal(mLocalizedString);
}

void PaperPickup::Initialise()
{
	Pickup::Initialise();

	if (!mLocalizationId.empty())
	{
		mLocalizedString = StringManager::GetInstance()->GetLocalisedString(mLocalizationId.c_str());
	}
}

void PaperPickup::XmlRead(TiXmlElement * element)
{
	Pickup::XmlRead(element);

	mLocalizationId = XmlUtilities::ReadAttributeAsString(element, "loc_id", "value");
}

void PaperPickup::XmlWrite(TiXmlElement * element)
{
	Pickup::XmlWrite(element);

	TiXmlElement * locIdElem = new TiXmlElement("loc_id");
	locIdElem->SetAttribute("value", mLocalizationId.c_str());
	element->LinkEndChild(locIdElem);
}

