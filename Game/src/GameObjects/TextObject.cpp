#include "precompiled.h"
#include "TextObject.h"
#include "StringManager.h"

TextObject::TextObject(float x, float y, float z, float width, float height) :
	DrawableObject(x, y, z, width, height)
{
}

TextObject::~TextObject(void)
{
}

void TextObject::Update(float delta)
{
	DrawableObject::Update(delta);
	/*
	GameObject::Update(delta);

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player)
	{
		if (Utilities::IsObjectInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y))
		{
			if (player->IsOnSolidSurface())
			{
				player->SetVelocityY(1.0f);
			}
			
			float yDiff = Top() - player->Bottom();
			float percent = yDiff / m_dimensions.Y;

			// player->AccelerateX(mForceDirection.X, mForce * delta);
			player->AccelerateY(mForceDirection.Y, (mForce * percent) * delta);
		}
	}
	*/
}

void TextObject::XmlRead(TiXmlElement * element)
{
	DrawableObject::XmlRead(element);

	/*
	mForce = XmlUtilities::ReadAttributeAsFloat(element, "force", "value");
	mForceDirection.X = XmlUtilities::ReadAttributeAsFloat(element, "force_direction", "x");
	mForceDirection.Y = XmlUtilities::ReadAttributeAsFloat(element, "force_direction", "y");
	*/
}

void TextObject::XmlWrite(TiXmlElement * element)
{
	DrawableObject::XmlWrite(element);
	/*
	GameObject::XmlWrite(element);

	TiXmlElement * force = new TiXmlElement("force");
	force->SetDoubleAttribute("value", mForce);
	element->LinkEndChild(force);

	TiXmlElement * forceDirection = new TiXmlElement("force_direction");
	forceDirection->SetDoubleAttribute("x", mForceDirection.X);
	forceDirection->SetDoubleAttribute("y", mForceDirection.Y);
	element->LinkEndChild(forceDirection);
	*/
}

void TextObject::Draw(ID3D10Device * device, Camera2D * camera)
{
	DrawableObject::Draw(device, camera);

	Vector2 worldPos = Vector2(m_position.X, m_position.Y);
	Vector2 screenPos = Utilities::WorldToScreen(worldPos);

	std::string str = StringManager::GetInstance()->GetLocalisedString("test_string");
	Graphics::GetInstance()->DrawDebugText(str.c_str(), screenPos.X, screenPos.Y);
}
