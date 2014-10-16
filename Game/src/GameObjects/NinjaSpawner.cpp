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

void NinjaSpawner::SpawnNPC(const float posX, const float posY, bool playSoundEffect, std::string animationFile)
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
	npc->m_drawAtNativeDimensions = true;
	npc->m_isAnimated = true;
	npc->SetMaxVelocityXYZ(randMaxXVelocity, 99999, 0);
	npc->SetCollisionDimensions(Vector3(100,200,0));
	npc->SetPlayer(GameObjectManager::Instance()->GetPlayer());
	npc->SetResistanceXYZ(0.88, 2.2, 0);
	npc->setAccelXRate(1.0);
	npc->SetMaterial(MaterialManager::Instance()->GetMaterial("demon1"));
	npc->SetMaxJumpSpeed(randJumpSpeed);

	GameObjectManager::Instance()->AddGameObject(npc);

	// show some effects when we spawn - smoke
	ParticleEmitterManager::Instance()->CreateRadialSpray(20,
														Vector3(npc->X(), npc->Y(), npc->Z() - 0.1f),
														Vector3(3200, 1200, 0),
														(rand() % 3) > 1 ? "Media\\smoke3.png" : "Media\\smoke4.png",
														1.5f,
														4.0f,
														1.0f,
														1.5f,
														150,
														300,
														0.5,
														false,
														0.5,
														1.0,
														800,
														true, 
														2.0,
														0.15f,
														0.8f,
														10,
														10);

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

		switch (randAnim)
		{
			case 0:
				case 1:
					case 2:
			{
				animFile = "XmlFiles\\ninjaAnimation2.xml";
				break;
			}
			case 3:
			{
				animFile = "XmlFiles\\player_female_animation.xml";
				break;
			}
			case 4:
			{
				animFile = "XmlFiles\\player_female_animation2.xml";
				break;
			}
			default:
			{
				animFile = "XmlFiles\\ninjaAnimation2.xml";
				break;
			}
		}

		SpawnNPC(boundsPos.X + randX, boundsPos.Y + randY, false, animFile);
	}

	AudioManager::Instance()->PlaySoundEffect("explosion\\smoke_explosion.wav");
}
