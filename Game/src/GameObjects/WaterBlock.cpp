#include "precompiled.h"
#include "WaterBlock.h"
#include "Material.h"
#include "ParticleEmittermanager.h"
#include "AudioManager.h"

WaterBlock::WaterBlock(void) :
	mIsDeepWater(false)
{
	mIsWaterBlock = true;
}

WaterBlock::~WaterBlock(void)
{
}

void WaterBlock::OnCollision(SolidMovingSprite * object)
{
	bool wasInwater = object->WasInWaterLastFrame();

	object->SetIsInWater(true, GetIsDeepWater());
	
	if (!wasInwater && object->VelocityY() < -2.0f)
	{
		Material * material = GetMaterial();
		if (material)
		{
			if (object->IsProjectile() && !GetIsDeepWater())
			{
				string soundFile = material->GetRandomDamageSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundFile);

				string particleTexFile = material->GetRandomParticleTexture();
				ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
																		Vector3(object->X(), object->CollisionBottom(), object->Z() - 0.1f),
																		Vector3(0, 1, 0),
																		0.4,
																		Vector3(3200, 1200, 0),
																		particleTexFile,
																		1.5,
																		5.5,
																		0.6f,
																		1.0f,
																		30,
																		75,
																		1.0,
																		false,
																		1.0f,
																		1.0f,
																		10.0f,
																		true,
																		2.5,
																		2.0f,
																		0.0f,
																		0.15f,
																		0.2f);
			}
			else if (!GetIsDeepWater())
			{
				AudioManager::Instance()->PlaySoundEffect("water\\water_splash_medium_2.wav");

				ParticleEmitterManager::Instance()->CreateDirectedSpray(30,
																		Vector3(object->X(), object->CollisionBottom() - 20, object->Z() - 0.1f),
																		Vector3(0, 1, 0),
																		1.0f,
																		Vector3(3200, 1200, 0),
																		"Media\\wetsplashparticle.png",
																		2.5,
																		3.5,
																		0.4f,
																		0.75f,
																		40,
																		60,
																		1.0,
																		false,
																		1.0f,
																		1.0f,
																		10.0f,
																		true,
																		3.5,
																		10.0f,
																		0.0f,
																		0.15f,
																		0.4f);

				string particleTexFile = material->GetRandomParticleTexture();
				ParticleEmitterManager::Instance()->CreateDirectedSpray(30,
																		Vector3(object->X(), object->CollisionBottom() - 50, object->Z() - 0.1f),
																		Vector3(0, 1, 0),
																		0.4,
																		Vector3(3200, 1200, 0),
																		particleTexFile,
																		1.5,
																		5.5,
																		0.6f,
																		1.0f,
																		30,
																		75,
																		1.0,
																		false,
																		1.0f,
																		1.0f,
																		10.0f,
																		true,
																		3.5,
																		10.0f,
																		0.0f,
																		0.15f,
																		0.3f);
			}
		}
	}
}

void WaterBlock::XmlRead(TiXmlElement * element)
{
	SolidMovingSprite::XmlRead(element);

	mIsDeepWater = XmlUtilities::ReadAttributeAsBool(element, "is_deep_water", "value");
}

void WaterBlock::XmlWrite(TiXmlElement * element)
{
	SolidMovingSprite::XmlWrite(element);

	TiXmlElement * deepWater = new TiXmlElement("is_deep_water");
	deepWater->SetAttribute("value", mIsDeepWater ? "true" : "false");
	element->LinkEndChild(deepWater);
}
