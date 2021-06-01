#include "precompiled.h"
#include "PaperPickup.h"
#include "AudioManager.h"
#include "Game.h"
#include "StringManager.h"
#include "SaveManager.h"

void PaperPickup::DoPickup()
{
	AudioManager::Instance()->PlaySoundEffect("paper_pickup.wav");

	SaveManager::GetInstance()->SetPaperPickupCollected(mLocDescId);

	GameObjectManager::Instance()->RemoveGameObject(this);

	Game::GetInstance()->DisplayTextModal(mLocTitleString, mLocDescString);
}

void PaperPickup::Initialise()
{
	Pickup::Initialise();

	if (!mLocDescId.empty())
	{
		mLocDescString = StringManager::GetInstance()->GetLocalisedString(mLocDescId.c_str());
	}

	if (!mLocTitleId.empty())
	{
		mLocTitleString = StringManager::GetInstance()->GetLocalisedString(mLocTitleId.c_str());
	}
}

void PaperPickup::XmlRead(TiXmlElement * element)
{
	Pickup::XmlRead(element);

	mLocDescId = XmlUtilities::ReadAttributeAsString(element, "loc_id", "desc");
	mLocTitleId = XmlUtilities::ReadAttributeAsString(element, "loc_id", "title");
}

void PaperPickup::XmlWrite(TiXmlElement * element)
{
	Pickup::XmlWrite(element);

	TiXmlElement * locIdElem = new TiXmlElement("loc_id");
	locIdElem->SetAttribute("title", mLocTitleId.c_str());
	locIdElem->SetAttribute("desc", mLocDescId.c_str());
	element->LinkEndChild(locIdElem);
}

void PaperPickup::Update(float delta)
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
		if (!mLocDescId.empty() && SaveManager::GetInstance()->IsPaperPickupCollected(mLocDescId))
		{
			GameObjectManager::Instance()->RemoveGameObject(this, true);
			return;
		}

		mHasInitCheckedCollected = true;
	}

	Pickup::Update(delta);
}

