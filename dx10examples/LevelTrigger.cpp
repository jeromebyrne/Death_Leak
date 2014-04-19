#include "precompiled.h"
#include "LevelTrigger.h"

LevelTrigger::LevelTrigger(float x, float y, float z, float width, float height):
	GameObject(x, y, z, width, height)
{
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
			GameObjectManager::Instance()->SwitchToLevel(mLevelToLoad.c_str(), true);
		}
	}
}

void LevelTrigger::XmlRead(TiXmlElement * element)
{
	GameObject::XmlRead(element);

	// level_to_load
	mLevelToLoad = XmlUtilities::ReadAttributeAsString(element, "level_to_load", "file");
}

void LevelTrigger::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * levelFile = new TiXmlElement("level_to_load");
	levelFile->SetAttribute("value", mLevelToLoad.c_str());
	element->LinkEndChild(levelFile);
}
