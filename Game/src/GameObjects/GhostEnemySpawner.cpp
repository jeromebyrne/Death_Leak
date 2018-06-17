#include "precompiled.h"
#include "GhostEnemySpawner.h"
#include "GhostEnemy.h"
#include "NPC.h"
#include "MaterialManager.h";
#include "ParticleEmittermanager.h"
#include "AudioManager.h"

GhostEnemySpawner::GhostEnemySpawner(void)
{
}

GhostEnemySpawner::~GhostEnemySpawner(void)
{
}

void GhostEnemySpawner::SpawnNPC(const float posX,
							const float posY,
							bool playSoundEffect, 
							std::string animationFile, 
							Vector3 & dimensions, 
							Vector3 & collisionDimensions, 
							Vector2 & collisionBoxOffset)
{
	float randJumpSpeed = rand() % 4000;
	randJumpSpeed *= 0.001f;
	randJumpSpeed += 16.0f;
	float randMaxXVelocity = rand() % 3000;
	randMaxXVelocity *= 0.001f;
	randMaxXVelocity += 14.0f;

	GhostEnemy * npc = new GhostEnemy();
	npc->SetXY(posX, posY, 49.0f); // TODO: set depth layer
	npc->m_animationFile = animationFile;
	npc->m_drawAtNativeDimensions = false;
	npc->m_dimensions = Vector2(dimensions.X, dimensions.Y);
	npc->m_isAnimated = true;
	npc->SetMaxVelocityXY(randMaxXVelocity, 99999);
	npc->SetCollisionDimensions(Vector3(collisionDimensions.X, collisionDimensions.Y, 0));
	npc->SetCollisionBoxOffset(Vector2(collisionBoxOffset.X, collisionBoxOffset.Y));   
	npc->SetPlayer(GameObjectManager::Instance()->GetPlayer());
	npc->SetResistanceXY(1.0f, 1.4f);
	npc->setAccelXRate(1.0);
	npc->SetMaterial(MaterialManager::Instance()->GetMaterial("ghost"));
	npc->SetMaxJumpSpeed(randJumpSpeed);
	npc->SetIsPlayerEnemy(true);
	npc->SetFadeAlphaWhenPlayerOccluded(false, 0.5f);
	npc->SetAlpha(0.4f);

	GameObjectManager::Instance()->AddGameObject(npc);
	npc->FlipVertical();

	// show some effects when we spawn - smoke
	ParticleEmitterManager::Instance()->CreateRadialSpray(50,
														Vector3(npc->X(), npc->Bottom(), npc->Z() - 1.0f),
														Vector3(3200, 1200, 0),
														(rand() % 3) > 1 ? "Media\\smoke3.png" : "Media\\smoke4.png",
														4.5f,
														6.0f,
														0.5f,
														1.0f,
														100,
														200,
														0.5,
														false,
														0.5,
														1.0,
														800,
														true, 
														2.2,
														0.0f,
														0.5f,
														10,
														50);

	if (playSoundEffect)
	{
		AudioManager::Instance()->PlaySoundEffect("explosion\\explosion.wav");
	}
}

void GhostEnemySpawner::SpawnMultiple(const unsigned int numNPC, Vector2 boundsPos, Vector2 boundsDimensions)
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

		Vector3 dimensions;
		Vector3 collisionDimensions;
		Vector2 collisionOffset;

		dimensions = Vector3(160, 259, 0);
		collisionDimensions = Vector3(100, 200, 0);
		collisionOffset = Vector2(0, 0);
		animFile = "XmlFiles\\animation\\ghost_enemy_anim.xml";

		SpawnNPC(boundsPos.X + randX, boundsPos.Y + randY, false, animFile, dimensions, collisionDimensions, collisionOffset);
	}

	// AudioManager::Instance()->PlaySoundEffect("gong.wav", false, false, false);
	AudioManager::Instance()->PlaySoundEffect("explosion\\smoke_explosion.wav");
}
