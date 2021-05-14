#include "precompiled.h"
#include "SkeletonEnemySpawner.h"
#include "SkeletonEnemy.h"
#include "NPC.h"
#include "MaterialManager.h";
#include "ParticleEmittermanager.h"
#include "AudioManager.h"

SkeletonEnemySpawner::SkeletonEnemySpawner(void)
{
}

SkeletonEnemySpawner::~SkeletonEnemySpawner(void)
{
}

void SkeletonEnemySpawner::SpawnNPC(const float posX,
							const float posY,
							bool playSoundEffect, 
							std::string animationFile, 
							Vector2 & dimensions, 
							Vector2 & collisionDimensions, 
							Vector2 & collisionBoxOffset)
{
	float randJumpSpeed = rand() % 4000;
	randJumpSpeed *= 0.001f;
	randJumpSpeed += 16.0f;
	float randMaxXVelocity = rand() % 4000;
	randMaxXVelocity *= 0.001f;
	randMaxXVelocity += 2.0f;

	SkeletonEnemy * npc = new SkeletonEnemy();
	npc->SetXY(posX, posY);
	npc->SetDepthLayer(GameObject::kNpc);
	npc->m_animationFile = animationFile;
	npc->m_drawAtNativeDimensions = false;
	npc->m_dimensions = Vector2(dimensions.X, dimensions.Y);
	npc->m_isAnimated = true;
	/*
	npc->EffectName = "effectpixelwobble";
	npc->mPixelWobbleIntensity = 0.035f;
	npc->mPixelWobbleSpeedMod = 0.75f;
	*/

	npc->EffectName = "effectlighttexture";
	// npc->mNoiseShaderIntensity = 0.02f;
	npc->SetMaxVelocityXY(randMaxXVelocity, 99999.0f);
	npc->SetCollisionDimensions(Vector2(collisionDimensions.X, collisionDimensions.Y));
	npc->SetCollisionBoxOffset(Vector2(collisionBoxOffset.X, collisionBoxOffset.Y));   
	npc->SetPlayer(GameObjectManager::Instance()->GetPlayer());
	npc->SetResistanceXY(0.99f, 1.4f);
	npc->setAccelXRate(10.0f);
	npc->SetMaterial(MaterialManager::Instance()->GetMaterial("skeleton"));
	npc->SetMaxJumpSpeed(randJumpSpeed);
	npc->SetIsPlayerEnemy(true);
	npc->SetFadeAlphaWhenPlayerOccluded(false, 0.5f);
	npc->SetAlpha(1.0f);

	GameObjectManager::Instance()->AddGameObject(npc);
	npc->FlipVertical();

	// show some effects when we spawn - smoke
	/*
	ParticleEmitterManager::Instance()->CreateRadialSpray(50,
														Vector2(npc->X(), npc->Bottom()),
														GameObject::kNpc,
														Vector2(3200.0f, 1200.0f),
														(rand() % 3) > 1 ? "Media\\smoke3.png" : "Media\\smoke4.png",
														4.5f,
														6.0f,
														0.5f,
														1.0f,
														100.0f,
														200.0f,
														0.5f,
														false,
														0.5f,
														1.0f,
														800.0f,
														true, 
														2.2f,
														0.0f,
														0.5f,
														10.0f,
														50.0f);
														*/

	if (playSoundEffect)
	{
		AudioManager::Instance()->PlaySoundEffect("explosion\\explosion.wav");
	}
}

void SkeletonEnemySpawner::SpawnMultiple(const unsigned int numNPC, Vector2 boundsPos, Vector2 boundsDimensions)
{
	for (int i = 0; i < numNPC; ++i)
	{
		// pick a random position between the middle of the bounds and the upper bounds
		int randX = (rand() % (int)(boundsDimensions.X * 0.5));
		bool negSign = rand() %2;
		if (negSign == 1)
		{
			randX = -randX;
		}
		int randY = (rand() % (int)(boundsDimensions.Y * 0.25)) + (boundsDimensions.Y * 0.25);

		// randomly pick an animation
		std::string animFile;

		Vector2 dimensions;
		Vector2 collisionDimensions;
		Vector2 collisionOffset;

		dimensions = Vector2(256.0f, 256.0f);
		collisionDimensions = Vector2(100.0f, 200.0f);
		collisionOffset = Vector2(0.0f, 0.0f);
		animFile = "XmlFiles\\animation\\skeleton_animation.xml";

		SpawnNPC(boundsPos.X + randX, boundsPos.Y + randY, false, animFile, dimensions, collisionDimensions, collisionOffset);
	}

	// AudioManager::Instance()->PlaySoundEffect("gong.wav", false, false, false);
	AudioManager::Instance()->PlaySoundEffect("explosion\\smoke_explosion.wav");
}

bool SkeletonEnemySpawner::CanSpawnNPC()
{
	return true;
}
