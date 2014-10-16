#include "precompiled.h"
#include "Explosion.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "NPCManager.h"
#include "Orb.h"
#include "Projectile.h"

static const unsigned int kExplosionAliveTime = 2000;

Explosion::Explosion(float damage,float radius, float x, float y, float z, float width, float height, float breadth, char* effectName) :
	DrawableObject(x, y, z, width, height, breadth, effectName),
	mDamage(damage),
	mRadius(radius),
	mFramesActive(0)
{
}

Explosion::~Explosion(void)
{
}

void Explosion::ApplyDamage()
{
	// loop through all of the NPCs in the game and damage them
	list<GameObject*> objects;
	GameObjectManager::Instance()->GetSolidSpritesOnScreen(objects);

	Player * player = GameObjectManager::Instance()->GetPlayer();
	for (auto obj : objects)
	{
		if (obj == player)
		{
			// don't want to damage ourselves
			continue;
		}

		Vector3 diff =  m_position - obj->Position();
		float distSquared = diff.LengthSquared();

		if (distSquared < (mRadius * mRadius))
		{
			obj->OnDamage(mDamage, Vector3());
		}
	}
}

void Explosion::ApplyForceToApplicable()
{
	list<GameObject *> objects;
	GameObjectManager::Instance()->GetTypesOnScreen<DrawableObject>(objects);

	for (auto obj : objects)
	{
		if (!obj)
		{
			GAME_ASSERT(obj);
			continue;
		}

		if (obj->IsOrb() ||
			obj->IsProjectile())
		{
			MovingSprite * moveable = static_cast<MovingSprite *>(obj); // has to be a movingsprite if it's one of the above

			Vector3 direction =  obj->Position() - m_position;

			float distSquared = direction.LengthSquared();

			if (distSquared < (mRadius * mRadius))
			{
				direction.Normalise();

				moveable->SetVelocityXYZ(moveable->GetVelocity().X + (direction.X * 10), moveable->GetVelocity().Y + (direction.Y * 15), 0);
			}
		}
	}
}

void Explosion::Update(float delta)
{
	if (mFramesActive == 1)
	{
		PlaySFX();
		Display();
		Inflict();
	}
	else if (mFramesActive == 2)
	{
		GameObjectManager::Instance()->RemoveGameObject(this, true);
	}

	++mFramesActive;
}

void Explosion::PlaySFX()
{
	bool loop = false;
	unsigned long loopTime = -1;

	int randBang = rand() % 4;
	switch (randBang)
	{
		case 0:
			{
				AudioManager::Instance()->PlaySoundEffect("explosion\\bang.wav");
				break;
			}
		case 1:
			{
				AudioManager::Instance()->PlaySoundEffect("explosion\\bang2.wav");
				break;
			}
		case 2:
			{
				AudioManager::Instance()->PlaySoundEffect("explosion\\bang3.wav");
				break;
			}
		case 3:
			{
				AudioManager::Instance()->PlaySoundEffect("explosion\\bang4.wav");
				break;
			}
		default:
			{
				AudioManager::Instance()->PlaySoundEffect("explosion\\bang.wav");
				break;
			}
	}
}

void Explosion::Display()
{
	ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
															m_position,
															Vector3(0, 0, 0),
															0.4,
															Vector3(3200, 1200, 0),
															"Media\\explosion_lines.png",
															0.01,
															0.01,
															0.3f,
															0.3f,
															128,
															128,
															0,
															false,
															0.7,
															1.0,
															10000,
															true,
															15,
															0.0f,
															0.0f,
															0.0f,
															0.5f);

	ParticleEmitterManager::Instance()->CreateDirectedSpray(50,
															Vector3(m_position.X, m_position.Y + 200, m_position.Z),
															Vector3(0, 1, 0),
															0.5f,
															Vector3(3200, 1200, 0),
															"Media\\smoke4.png",
															2.0f,
															12.5f,
															3.5f,
															4.5f,
															40.0f,
															80.0f,
															1.1f,
															false,
															0.3f,
															0.6f,
															10.0f,
															true,
															10.5f,
															1.0f,
															8.0f,
															0.25f,
															0.1f);

	ParticleEmitterManager::Instance()->CreateDirectedSpray(20,
															Vector3(m_position.X, m_position.Y - 100, m_position.Z),
															Vector3(0, 1, 0),
															0.2f,
															Vector3(3200, 1200, 0),
															"Media\\flame3.png",
															10.0f,
															20.5f,
															0.3f,
															0.8f,
															20.0f,
															40.0f,
															0.5f,
															false,
															1.0f,
															1.0f,
															10.0f,
															true,
															10.5f,
															1.0f,
															2.0f,
															0.0f,
															0.2f);

	ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
															m_position,
															Vector3(0, 0, 0),
															0.4,
															Vector3(3200, 1200, 0),
															"Media\\blast_circle.png",
															0.01,
															0.01,
															0.4f,
															0.55f,
															32,
															32,
															0,
															false,
															0.7,
															1.0,
															10000,
															true,
															80,
															0.0f,
															0.0f,
															0.2f,
															0.5f);
}

void Explosion::Inflict()
{
	ApplyDamage();

	ApplyForceToApplicable();
}
