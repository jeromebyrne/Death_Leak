#include "precompiled.h"
#include "WaterBlock.h"
#include "Material.h"
#include "ParticleEmittermanager.h"
#include "AudioManager.h"

WaterBlock::WaterBlock(void)
{
	mIsWaterBlock = true;
}

WaterBlock::~WaterBlock(void)
{
}

bool WaterBlock::OnCollision(SolidMovingSprite * object)
{
	bool wasInwater = object->WasInWaterLastFrame();

	object->SetIsInWater(true);
	
	if (!wasInwater && object->VelocityY() < -2.0f)
	{
		Material * material = GetMaterial();
		if (material)
		{
			if (object->IsProjectile())
			{
				string soundFile = material->GetRandomDamageSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundFile);

				string particleTexFile = material->GetRandomParticleTexture();
				ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
																		Vector2(object->X(), object->CollisionBottom()),
																		object->GetDepthLayer(),
																		Vector2(0.0f, 1.0f),
																		0.4f,
																		Vector2(3200.0f, 1200.0f),
																		particleTexFile,
																		1.5f,
																		5.5f,
																		0.6f,
																		1.0f,
																		30.0f,
																		75.0f,
																		1.0,
																		false,
																		1.0f,
																		1.0f,
																		10.0f,
																		true,
																		2.5f,
																		2.0f,
																		0.0f,
																		0.15f,
																		0.2f);
			}
			/*
			else if (!GetIsDeepWater())
			{
				AudioManager::Instance()->PlaySoundEffect("water\\water_splash_medium_2.wav");

				ParticleEmitterManager::Instance()->CreateDirectedSpray(30,
																		Vector2(object->X(), object->CollisionBottom() - 20.0f),
																		object->GetDepthLayer(),
																		Vector2(0.0f, 1.0f),
																		1.0f,
																		Vector2(3200.0f, 1200.0f),
																		"Media\\wetsplashparticle.png",
																		2.5f,
																		3.5f,
																		0.4f,
																		0.75f,
																		40.0f,
																		60.0f,
																		1.0f,
																		false,
																		1.0f,
																		1.0f,
																		10.0f,
																		true,
																		3.5f,
																		10.0f,
																		0.0f,
																		0.15f,
																		0.4f);

				string particleTexFile = material->GetRandomParticleTexture();
				ParticleEmitterManager::Instance()->CreateDirectedSpray(30,
																		Vector2(object->X(), object->CollisionBottom() - 50.0f),
																		object->GetDepthLayer(),
																		Vector2(0.0f, 1.0f),
																		0.4f,
																		Vector2(3200.0f, 1200.0f),
																		particleTexFile,
																		1.5f,
																		5.5f,
																		0.6f,
																		1.0f,
																		30.0f,
																		75.0f,
																		1.0f,
																		false,
																		1.0f,
																		1.0f,
																		10.0f,
																		true,
																		3.5f,
																		10.0f,
																		0.0f,
																		0.15f,
																		0.3f);
			}*/
		}
	}

	return true;
}
