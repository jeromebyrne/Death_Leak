#include "precompiled.h"
#include "Breakable.h"
#include "AudioManager.h"
#include "Material.h"
#include "particleEmitterManager.h"
#include "CurrencyOrb.h"
#include "Orb.h"
#include "SaveManager.h"
#include "Game.h"

Breakable::Breakable(float x, float y, float z, float width, float height, float breadth) :
	SolidMovingSprite(x,y,z,width, height, breadth),
	mHealth(10.0f),
	mMaxHealth(10.0f),
	mState(kNoDamage),
	mDeleteWhenBroken(false)
{
	mIsBreakable = true;
}

Breakable::~Breakable(void)
{
}

void Breakable::Update(float delta)
{
	// update the base classes
	SolidMovingSprite::Update(delta);
}

void Breakable::Initialise()
{
	// update the base classes
	SolidMovingSprite::Initialise();

	if (!Game::GetInstance()->GetIsLevelEditMode() )
	{
		std::vector<unsigned int> breakablesBroken;
		SaveManager::GetInstance()->GetBreakablesBroken(GameObjectManager::Instance()->GetCurrentLevelFile(), breakablesBroken);

		if (std::find(breakablesBroken.begin(), breakablesBroken.end(), ID()) != breakablesBroken.end())
		{
			mHealth = 0.0f;
			mState = kBroken;
			UpdateState();
		}
	}
}

void Breakable::XmlRead(TiXmlElement * element)
{
	// update base classes
	SolidMovingSprite::XmlRead(element);

	mMaxHealth = XmlUtilities::ReadAttributeAsFloat(element, "health", "value");
	mHealth = mMaxHealth;

	mDeleteWhenBroken = XmlUtilities::ReadAttributeAsBool(element, "health", "delete_on_break");
}

void Breakable::XmlWrite(TiXmlElement * element)
{
	SolidMovingSprite::XmlWrite(element);

	TiXmlElement * healthElem = new TiXmlElement("health");
	healthElem->SetDoubleAttribute("value", mMaxHealth);
	healthElem->SetAttribute("delete_on_break", mDeleteWhenBroken ? "true" : "false");
	element->LinkEndChild(healthElem);
}

bool Breakable::OnCollision(SolidMovingSprite * object)
{
	if (mState == kBroken)
	{
		return false;
	}

	if (!object->IsProjectile())
	{
		return false;
	}

	return SolidMovingSprite::OnCollision(object);
}

void Breakable::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	if (mState == kBroken)
	{
		return;
	}

	UpdateState();

	if (mCanBeDamaged)
	{
		mHealth -= damageAmount;
	}

	SolidMovingSprite::OnDamage(damageDealer, damageAmount, pointOfContact, shouldExplode);
}

void Breakable::UpdateState()
{
	BreakableState previousState = mState;

	if (mMaxHealth == 0.0f)
	{
		return;
	}

	float percentHealthy = mHealth / mMaxHealth;
	if (percentHealthy == 1.0f)
	{
		mState = kNoDamage;
	}
	else if (percentHealthy > 0.8f)
	{
		mState = kSlightDamage;
	}
	else if (percentHealthy > 0.4f)
	{
		mState = kDamaged;
	}
	else if (percentHealthy > .0f)
	{
		mState = kHeavyDamaged;
	}
	else if (mState != kBroken)
	{
		GameObjectManager::Instance()->SetBreakableBroken(ID());

		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																Vector3(m_position.X - (m_dimensions.X * 0.5f), m_position.Y - (m_dimensions.Y * 0.5f), m_position.Z - 0.02f),
																Vector3(0, 0, 0),
																0.4,
																Vector3(3200, 1200, 0),
																"Media\\blast_circle.png",
																0.01,
																0.01,
																0.7f,
																0.80f,
																32,
																32,
																0,
																false,
																0.7,
																1.0,
																10000,
																true,
																35,
																0.0f,
																0.0f,
																0.1f,
																0.1f);

		mState = kBroken;
		CurrencyOrb::SpawnOrbs(m_position, (rand() % 8) + 1);
		AudioManager::Instance()->PlaySoundEffect(m_material->GetRandomDestroyedSound());

		if (mDeleteWhenBroken)
		{
			GameObjectManager::Instance()->RemoveGameObject(this, true);
		}
	}

	if (previousState != mState &&
		mState == kBroken)
	{
		SpawnDamageTransitionParticles();
	}
}

void Breakable::UpdateAnimations()
{
	std::string currentSequence = "Still";

	switch (mState)
	{
		case kSlightDamage:
		{
			currentSequence = "Damage1";
			break;
		}
		case kDamaged:
		{
			currentSequence = "Damage2";
			break;
		}
		case kHeavyDamaged:
		{
			currentSequence = "Damage3";
			break;
		}
		case kBroken:
		{
			currentSequence = "Damage4";
			break;
		}
		default:
		{
			break;
		}
	}

	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	if (bodyPart)
	{
		std::string sequence_name = bodyPart->CurrentSequence()->Name();
		if (sequence_name != currentSequence)
		{
			bodyPart->SetSequence(currentSequence);
		}

		bodyPart->AnimateLooped();

		m_texture = bodyPart->CurrentFrame(); // set the current texture
	}
}

void Breakable::SpawnDamageTransitionParticles()
{
	if (!m_material)
	{
		return;
	}

	Vector2 nativeDimensions = GetTextureDimensions();

	float scaleX = m_dimensions.X / nativeDimensions.X;
	float scaleY = m_dimensions.Y / nativeDimensions.Y;

	// TODO: only do this for crates, read from material file
	for (int i = 0; i < 4; ++i)
	{
		Orb * debris = new Orb(nullptr, Vector3(m_position.X,
			m_position.Y + 50,
			m_position.Z),
			Vector3(160 * scaleX, 443 * scaleY, 0),
			Vector3(15, 15, 0),
			"Media\\crate\\debris.png",
			false,
			1.0f);

		GameObjectManager::Instance()->AddGameObject(debris);
	}

	ParticleEmitterManager::Instance()->CreateDirectedSpray(5,
															Vector3(m_position.X, m_position.Y, m_position.Z - 0.02f),
															Vector3(0, 1, 0),
															0.7f,
															Vector3(3200, 1200, 0),
															m_material->GetRandomParticleTexture(),
															1.0f,
															7.0f,
															0.7f,
															1.50f,
															180,
															180,
															0,
															false,
															0.7,
															1.0,
															10000,
															true,
															2,
															m_dimensions.X * 0.03f,
															m_dimensions.Y * 0.1f,
															0.1f,
															0.1f);
} 