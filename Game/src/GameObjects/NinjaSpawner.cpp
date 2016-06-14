#include "precompiled.h"
#include "NinjaSpawner.h"
#include "NPC.h"
#include "MaterialManager.h";
#include "ParticleEmittermanager.h"
#include "AudioManager.h"

NinjaSpawner::NinjaSpawner(void)
{
}

NinjaSpawner::~NinjaSpawner(void)
{
}

void NinjaSpawner::SpawnNPC(const float posX,
							const float posY,
							bool playSoundEffect, 
							std::string animationFile, 
							Vector3 & dimensions, 
							Vector3 & collisionDimensions, 
							Vector2 & collisionBoxOffset)
{
	float randJumpSpeed = rand() % 4000;
	randJumpSpeed *= 0.001;
	randJumpSpeed += 20;
	float randMaxXVelocity = rand() % 3000;
	randMaxXVelocity *= 0.001;
	randMaxXVelocity += 10;

	int randZ = rand() % 20;

	NPC * npc = new NPC(posX, posY, 49 + (randZ * 0.1f));
	npc->m_animationFile = animationFile;
	npc->m_drawAtNativeDimensions = false;
	npc->m_dimensions = Vector3(dimensions.X, dimensions.Y, 0);
	npc->m_isAnimated = true;
	npc->SetMaxVelocityXYZ(randMaxXVelocity, 99999, 0);
	npc->SetCollisionDimensions(Vector3(collisionDimensions.X, collisionDimensions.Y, 0));
	npc->SetCollisionBoxOffset(Vector2(collisionBoxOffset.X, collisionBoxOffset.Y));   
	npc->SetPlayer(GameObjectManager::Instance()->GetPlayer());
	npc->SetResistanceXYZ(1.0, 1.4f, 0);
	npc->setAccelXRate(1.0);
	npc->SetMaterial(MaterialManager::Instance()->GetMaterial("demon1"));
	npc->SetMaxJumpSpeed(randJumpSpeed);
	npc->SetIsPlayerEnemy(false);

	npc->m_dimensions = Vector3(133.777786, 387.761719, 0);
	npc->SetCollisionDimensions(npc->m_dimensions);
	npc->SetCollisionBoxOffset(Vector2(0, 0));
	// height = "387.761719" width = "133.777786"

	GameObjectManager::Instance()->AddGameObject(npc);

	// TODO: set behaviour

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

void NinjaSpawner::SpawnMultiple(const unsigned int numNPC, Vector2 boundsPos, Vector2 boundsDimensions)
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
		int randAnim = rand() % 5;

		Vector3 dimensions;
		Vector3 collisionDimensions;
		Vector2 collisionOffset;

		switch (randAnim)
		{
			case 0:
			case 1:
			case 2:
			{
				dimensions = Vector3(220, 220, 0);
				collisionDimensions = Vector3(100, 200, 0);
				collisionOffset = Vector2(0, -20);
				animFile = "XmlFiles\\ninjaAnimation3.xml";
				break;
			}
			case 3:
			case 4:
			{
				dimensions = Vector3(220, 220, 0);
				collisionDimensions = Vector3(100,200,0);
				collisionOffset = Vector2(0, -20);
				animFile = "XmlFiles\\ninjaAnimation2.xml";
				break;
			}
			default:
			{
				dimensions = Vector3(50, 50, 0);
				collisionDimensions = Vector3(200, 200, 0);
				collisionOffset = Vector2(0, 100);
				animFile = "XmlFiles\\ninjaAnimation2.xml";
				break;
			}
		}

		SpawnNPC(boundsPos.X + 200, boundsPos.Y + 100, false, "XmlFiles\\joe_anim.xml", dimensions, collisionDimensions, collisionOffset);
	}

	// AudioManager::Instance()->PlaySoundEffect("gong.wav", false, false, false);
	AudioManager::Instance()->PlaySoundEffect("explosion\\smoke_explosion.wav");
}
