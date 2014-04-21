#include "precompiled.h"
#include "Player.h"
#include "AnimationPart.h"
#include "AudioManager.h"
#include "orb.h"
#include "particleemittermanager.h"

Player::Player(float x, float y, float z, float width, float height, float breadth):
Character(x,y,z,width,height,breadth)
{
	mHealth = 100;
}

Player::~Player(void)
{
}

void Player::Update(float delta)
{
	// update base classes
	Character::Update(delta);
}
void Player::Initialise()
{
	// update the base classes
	Character::Initialise();
}
void Player::XmlRead(TiXmlElement * element)
{
	// update the base classes
	Character::XmlRead(element);
}

void Player::XmlWrite(TiXmlElement * element)
{
	Character::XmlWrite(element);
}

void Player::OnCollision(SolidMovingSprite * object)
{
	// update the base classes
	Character::OnCollision(object);

	Orb * orb = dynamic_cast<Orb*>(object);
	if (orb)
	{
		IncreaseHealth(orb->GetValue());
	}
}

void Player::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageAmount, pointOfContact, shouldExplode);

	if (mHealth <= 0)
	{
		m_alpha = 0.2; 
	}
}
