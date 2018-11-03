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
							Vector2 & dimensions, 
							Vector2 & collisionDimensions, 
							Vector2 & collisionBoxOffset)
{
	float randJumpSpeed = rand() % 4000;
	randJumpSpeed *= 0.001f;
	randJumpSpeed += 16.0f;
	float randMaxXVelocity = rand() % 3000;
	randMaxXVelocity *= 0.001f;
	randMaxXVelocity += 14.0f;

	NPC * npc = new NPC(posX, posY, GameObject::kNpc,  49.0f);
	npc->m_animationFile = animationFile;
	npc->m_drawAtNativeDimensions = false;
	npc->m_dimensions = Vector2(dimensions.X, dimensions.Y);
	npc->m_isAnimated = true;
	npc->SetMaxVelocityXY(randMaxXVelocity, 99999.0f);
	npc->SetCollisionDimensions(collisionDimensions);
	npc->SetCollisionBoxOffset(collisionBoxOffset);
	npc->SetPlayer(GameObjectManager::Instance()->GetPlayer());
	npc->SetResistanceXY(1.0f, 1.4f);
	npc->setAccelXRate(1.0f);
	npc->SetMaterial(MaterialManager::Instance()->GetMaterial("demon1"));
	npc->SetMaxJumpSpeed(randJumpSpeed);
	npc->SetIsPlayerEnemy(true);
	npc->SetFadeAlphaWhenPlayerOccluded(true, 0.5f);

	GameObjectManager::Instance()->AddGameObject(npc);
	npc->FlipVertical();

	// show some effects when we spawn - smoke
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

		Vector2 dimensions;
		Vector2 collisionDimensions;
		Vector2 collisionOffset;

		switch (randAnim)
		{
			// height = "231.714279" width = "183.142853"
			case 0:
			case 1:
			case 2:
			{
				dimensions = Vector2(183.142853f, 231.714279f);
				collisionDimensions = Vector2(100.0f, 200.0f);
				collisionOffset = Vector2(0.0f, 0.0f);
				animFile = "XmlFiles\\animation\\ninjaAnimation.xml";
				break;
			}
			case 3:
			case 4:
			{
				dimensions = Vector2(183.142853f, 231.714279f);
				collisionDimensions = Vector2(100.0f,200.0f);
				collisionOffset = Vector2(0.0f, 0.0f);
				animFile = "XmlFiles\\animation\\ninjaAnimation.xml";
				break;
			}
			default:
			{
				dimensions = Vector2(183.142853f, 231.714279f);
				collisionDimensions = Vector2(200.0f, 200.0f);
				collisionOffset = Vector2(0.0f, 0.0f);
				animFile = "XmlFiles\\animation\\ninjaAnimation.xml";
				break;
			}
		}

		SpawnNPC(boundsPos.X + randX, boundsPos.Y + randY, false, animFile, dimensions, collisionDimensions, collisionOffset);
	}

	// AudioManager::Instance()->PlaySoundEffect("gong.wav", false, false, false);
	AudioManager::Instance()->PlaySoundEffect("explosion\\smoke_explosion.wav");
}
