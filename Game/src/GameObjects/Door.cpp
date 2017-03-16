#include "precompiled.h"
#include "Door.h"
#include "InputManager.h"
#include "Game.h"

static const float kWarmUpTime = 2.0f;

Door::Door() :
	Sprite()
{
}

Door::~Door()
{
}

void Door::Initialise()
{
	Sprite::Initialise();
}

void Door::Update(float delta)
{
	Sprite::Update(delta);

	if (mDoorWarmUpTime < kWarmUpTime)
	{
		mDoorWarmUpTime += delta;
		return;
	}

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (!player)
	{
		return;
	}

	if (Utilities::IsObjectInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y))
	{
		auto inputManager = Game::GetInstance()->GetInputManager();
		if (inputManager.IsPressingEnterDoor() &&
			player->IsOnSolidSurface() && 
			std::abs(player->GetVelocity().X) < 0.5f)
		{
			if (!mRequiredKey.empty())
			{
				// TODO: check has key

				// TODO: play locked SFX if don't have a key
			}

			// TODO: play open SFX
			EnterDoor();
		}
	}
}
void Door::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	mToLevelFile = XmlUtilities::ReadAttributeAsString(element, "to_level_file", "value");

	mToLevelPosition.X = XmlUtilities::ReadAttributeAsFloat(element, "player_start_pos", "x");
	mToLevelPosition.Y = XmlUtilities::ReadAttributeAsFloat(element, "player_start_pos", "y");

	mRequiredKey = XmlUtilities::ReadAttributeAsString(element, "required_key", "value");

	mDoorOpenSFX = XmlUtilities::ReadAttributeAsString(element, "door_sfx", "open");
	mDoorLockedSFX = XmlUtilities::ReadAttributeAsString(element, "door_sfx", "locked");
}

void Door::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	TiXmlElement * levelFile = new TiXmlElement("to_level_file");
	levelFile->SetAttribute("value", mToLevelFile.c_str());
	element->LinkEndChild(levelFile);

	TiXmlElement * posElem = new TiXmlElement("player_start_pos");
	posElem->SetDoubleAttribute("x", mToLevelPosition.X);
	posElem->SetDoubleAttribute("y", mToLevelPosition.Y);
	element->LinkEndChild(posElem);

	TiXmlElement * requiredKey = new TiXmlElement("required_key");
	requiredKey->SetAttribute("value", mRequiredKey.c_str());
	element->LinkEndChild(requiredKey);

	TiXmlElement * doorSFX = new TiXmlElement("door_sfx");
	doorSFX->SetAttribute("open", mDoorOpenSFX.c_str());
	doorSFX->SetAttribute("locked", mDoorLockedSFX.c_str());
	element->LinkEndChild(doorSFX);
}

void Door::EnterDoor()
{
	GameObjectManager::Instance()->SetPlayerStartPos(mToLevelPosition);
	GameObjectManager::Instance()->SwitchToLevel(mToLevelFile.c_str(), true);
}
