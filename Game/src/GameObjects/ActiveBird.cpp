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

	// create some feathers
	for (int i = 0; i < 4; ++i)
	{
		Debris * feather = new Debris(nullptr, m_position, Vector3(30, 30, 0), Vector3(15, 15, 0), "Media\\characters\\bird\\feather.png", false, 0.1f);
		GameObjectManager::Instance()->AddGameObject(feather);
	}

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