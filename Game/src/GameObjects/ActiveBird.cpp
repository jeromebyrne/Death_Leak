#include "precompiled.h"
#include "ActiveBird.h"
#include "Debris.h"
#include "ParticleEmitterManager.h"

ActiveBird::ActiveBird(void) :
	AmbientBird()
{
}

ActiveBird::~ActiveBird(void)
{
}

void ActiveBird::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageDealer, damageAmount, pointOfContact, false);

	Debris * deadBird = new Debris(nullptr, m_position, m_dimensions, Vector3(15, 15, 0), "Media\\characters\\bird\\dead.png", false, 0.5f);
	GameObjectManager::Instance()->AddGameObject(deadBird);

	ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(5, m_position, true, 2.0f);
	if (spray)
	{
		spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(deadBird->ID()), Vector3(0, 0, 0));
	}

	ParticleEmitterManager::Instance()->CreateRadialSpray(10,
														m_position,
														Vector3(3000, 3000, 0),
														"Media\\characters\\bird\\feather.png",
														5.0f,
														9.0f,
														0.5,
														0.7f,
														20,
														40,
														2.0f,
														false,
														1.0f,
														1.0f,
														0.0f,
														false,
														1.0f,
														0.5f,
														0.9f,
														5.0f,
														5.0f);
}

void ActiveBird::Initialise()
{
	AmbientBird::Initialise();

	mRunAnimFramerateMultiplier = 8.0f;

	mFlapWingsMinDelay = 0.35f;
	mFlapWingsMaxDelay = 0.7f;
	mFlapTimeMinDelay = 1.0f;
	mFlapTimeMaxDelay = 2.4f;
}