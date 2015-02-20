#include "precompiled.h"
#include "LevelTrigger.h"

LevelTrigger::LevelTrigger(float x, float y, float z, float width, float height):
	GameObject(x, y, z, width, height)
{
	mAlwaysUpdate = true;
}

LevelTrigger::~LevelTrigger(void)
{
}

void LevelTrigger::Initialise()
{
	GameObject::Initialise();
}

void LevelTrigger::Update(float delta)
{
	GameObject::Update(delta);

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player)
	{
		if (Utilities::IsObjectInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y))
		{
			// trigger next level
			GameObjectManager::Instance()->SetPlayerStartPos(mPlayerStartPos);
			GameObjectManager::Instance()->SetPlayerStartDirectionX(mPlayerDirectionX);
			GameObjectManager::Instance()->SwitchToLevel(mLevelToLoad.c_str(), true);
		}
	}
}

void LevelTrigger::XmlRead(TiXmlElement * element)
{
	GameObject::XmlRead(element);

	// level_to_load
	mLevelToLoad = XmlUtilities::ReadAttributeAsString(element, "level_to_load", "value");

	mPlayerStartPos.X = XmlUtilities::ReadAttributeAsFloat(element, "player_start_pos", "x");
	mPlayerStartPos.Y = XmlUtilities::ReadAttributeAsFloat(element, "player_start_pos", "y");
	mPlayerDirectionX = XmlUtilities::ReadAttributeAsFloat(element, "player_direction", "x");
}

void LevelTrigger::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * levelFile = new TiXmlElement("level_to_load");
	levelFile->SetAttribute("value", mLevelToLoad.c_str());
	element->LinkEndChild(levelFile);

	TiXmlElement * posElem = new TiXmlElement("player_start_pos");
	posElem->SetDoubleAttribute("x", mPlayerStartPos.X);
	posElem->SetDoubleAttribute("y", mPlayerStartPos.Y);
	element->LinkEndChild(posElem);

	TiXmlElement * dirElem = new TiXmlElement("player_direction");
	dirElem->SetDoubleAttribute("x", mPlayerDirectionX);
	element->LinkEndChild(dirElem);
}
