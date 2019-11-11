#include "precompiled.h"
#include "Explosion.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "NPCManager.h"
#include "Debris.h"
#include "Projectile.h"
#include "Game.h"

static const unsigned int kExplosionAliveTime = 2000;

Explosion::Explosion(float damage,float radius, float x, float y, GameObject::DepthLayer depthLayer, float width, float height, char* effectName) :
	DrawableObject(x, y, depthLayer, width, height, effectName),
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

		Vector2 diff =  m_position - obj->Position();
		float distSquared = diff.LengthSquared();

		if (distSquared < (mRadius * mRadius))
		{
			obj->OnDamage(this, mDamage, Vector2());
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

		if (obj->IsDebris() ||
			obj->IsProjectile())
		{
			MovingSprite * moveable = static_cast<MovingSprite *>(obj); // has to be a movingsprite if it's one of the above

			Vector2 direction =  obj->Position() - m_position;

			float distSquared = direction.LengthSquared();

			if (distSquared < (mRadius * mRadius))
			{
				direction.Normalise();

				moveable->SetVelocityXY(moveable->GetVelocity().X + (direction.X * 10.0f), moveable->GetVelocity().Y + (direction.Y * 15.0f));
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

		Game::GetInstance()->Vibrate(0.8f, 1.0f, 0.2f);
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

	int randBang = rand() % 3;
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
															GameObject::kImpactCircles,
															Vector2(0.0f, 0.0f),
															0.4f,
															Vector2(3200.0f, 1200.0f),
															"Media\\blast_circle.png",
															0.01f,
															0.01f,
															0.25f,
															0.4f,
															32.0f,
															32.0f,
															0.0f,
															false,
															0.7f,
															1.0f,
															10000.0f,
															true,
															75.0f,
															0.0f,
															0.0f,
															0.2f,
															0.5f);

	ParticleEmitterManager::Instance()->CreateRadialSpray(15,
															m_position,
															GetDepthLayer(),
															Vector2(2000.0f, 2000.0f),
															"Media\\smoke4.png",
															10.0f,
															30.5f,
															0.4f,
															0.6f,
															80.0f,
															160.0f,
															1.1f,
															false,
															0.3f,
															0.6f,
															10.0f,
															true,
															12.5f,
															0.1f,
															0.5f,
															5.0f,
															5.0f);

	ParticleEmitterManager::Instance()->CreateRadialSpray(20,
															m_position,
															GetDepthLayer(),
															Vector2(2000.0f, 2000.0f),
															"Media\\flame3.png",
															5.0f,
															10.5f,
															0.15f,
															0.3f,
															300.0f,
															400.0f,
															1.1f,
															false,
															1.0f,
															1.0f,
															10.0f,
															true,
															0.4f,
															0.1f,
															0.5f,
															5.0f,
															5.0f);

}

void Explosion::Inflict()
{
	ApplyDamage();

	ApplyForceToApplicable();
}
