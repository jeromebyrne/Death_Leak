#include "precompiled.h"
#include "Smashable.h"
#include "AudioManager.h"
#include "Material.h"
#include "particleEmitterManager.h"
#include "CurrencyOrb.h"
#include "Debris.h"
#include "SaveManager.h"
#include "Game.h"

Smashable::Smashable(float x, float y, float z, float width, float height, float breadth) :
	SolidMovingSprite(x,y,z,width, height, breadth),
	mState(kNotSmashed)
{
	mCanBeDamaged = false;
	mIsSmashable = true;
}

Smashable::~Smashable(void)
{
}

void Smashable::Update(float delta)
{
	// update the base classes
	SolidMovingSprite::Update(delta);
}

void Smashable::Initialise()
{
	// update the base classes
	SolidMovingSprite::Initialise();

	if (!Game::GetInstance()->GetIsLevelEditMode() )
	{
		std::vector<unsigned int> breakablesBroken;
		SaveManager::GetInstance()->GetBreakablesBroken(GameObjectManager::Instance()->GetCurrentLevelFile(), breakablesBroken);

		if (std::find(breakablesBroken.begin(), breakablesBroken.end(), ID()) != breakablesBroken.end())
		{
			mState = kSmashed;
		}
	}
}

void Smashable::XmlRead(TiXmlElement * element)
{
	// update base classes
	SolidMovingSprite::XmlRead(element);
}

void Smashable::XmlWrite(TiXmlElement * element)
{
	SolidMovingSprite::XmlWrite(element);
}

bool Smashable::OnCollision(SolidMovingSprite * object)
{
	if (mState == kSmashed)
	{
		return false;
	}

	auto player = GameObjectManager::Instance()->GetPlayer();
	if (player && player->GetIsDownwardDashing())
	{
		// we are downward dashing, this is the only thing that can break a smashable
		// just make sure we are on top of the object
		if (player->CollisionBottom() <= CollisionTop() &&
			player->CollisionBottom() > CollisionCentreY())
		{
			mState = kSmashed;
			GameObjectManager::Instance()->SetBreakableBroken(ID());
			CurrencyOrb::SpawnOrbs(m_position, (rand() % 8) + 1);
			if (m_material)
			{
				AudioManager::Instance()->PlaySoundEffect(m_material->GetRandomDestroyedSound());
			}
			SpawnSmashedParticles();
		}
	}

	return SolidMovingSprite::OnCollision(object);
}

void Smashable::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	SolidMovingSprite::OnDamage(damageDealer, damageAmount, pointOfContact, shouldExplode);
}

void Smashable::UpdateAnimations()
{
	std::string currentSequence = "Still";

	switch (mState)
	{
		case kSmashed:
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

void Smashable::SpawnSmashedParticles()
{
	if (!m_material)
	{
		return;
	}

	Vector2 nativeDimensions = GetTextureDimensions();

	float scaleX = m_dimensions.X / nativeDimensions.X;
	float scaleY = m_dimensions.Y / nativeDimensions.Y;

	ParticleEmitterManager::Instance()->CreateDirectedSpray(5,
																Vector3(m_position.X, m_position.Y, m_position.Z),
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

	float debrisPosZ = 51.0f;
	if (m_position.Z < 50.0f)
	{
		debrisPosZ = m_position.Z - 0.01f;
	}

	const auto & debrisTextures = m_material->GetDebrisTextures();

	Vector3 size;

	// A massive hack...
	if (m_material->GetMaterialName() == "crate")
	{
		size = Vector3(160 * scaleX, 443 * scaleY, 0);
	}
	else
	{
		// pot
		size = Vector3(310 * scaleX, 284 * scaleY, 0);
	}

	for (const auto & d : debrisTextures)
	{
		Debris * debris = new Debris(nullptr, Vector3(m_position.X,
									m_position.Y + 50,
									debrisPosZ),
									size,
									Vector3(30, 30, 0),
									d.c_str(),
									false,
									1.0f);

		debris->SetMaterial(m_material);

		GameObjectManager::Instance()->AddGameObject(debris);
	}
} 