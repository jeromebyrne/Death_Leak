#include "precompiled.h"
#include "LevelTrigger.h"
#include "Game.h"
#include "DrawUtilities.h"

LevelTrigger::LevelTrigger(float x, float y, DepthLayer depthLayer, float width, float height):
	GameObject(x, y, depthLayer, width, height)
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

#ifdef _DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}
#endif

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player)
	{
		if (Utilities::IsObjectInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y))
		{
			// trigger next level
			GameObjectManager::Instance()->SwitchToLevel(mLevelToLoad.c_str(), "", true);
		}
	}
}

void LevelTrigger::XmlRead(TiXmlElement * element)
{
	GameObject::XmlRead(element);

	// level_to_load
	mLevelToLoad = XmlUtilities::ReadAttributeAsString(element, "level_to_load", "value");
}

void LevelTrigger::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * levelFile = new TiXmlElement("level_to_load");
	levelFile->SetAttribute("value", mLevelToLoad.c_str());
	element->LinkEndChild(levelFile);
}

void LevelTrigger::DebugDraw(ID3D10Device *  device)
{
	GameObject::DebugDraw(device);

	DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3), Vector2(GetLevelEditSelectionDimensions().X, GetLevelEditSelectionDimensions().Y), "Media\\editor\\level_trigger.png");
}
