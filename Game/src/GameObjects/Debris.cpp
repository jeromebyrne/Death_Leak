#include "precompiled.h"
#include "Debris.h"
#include "GameObjectManager.h"
#include "particleemittermanager.h"
#include "material.h"
#include "materialmanager.h"
#include "audiomanager.h"

unsigned long Debris::mLastTimePlayedSFX = 0;

static const float kMinTimeBetweenSFX = 0.5f;

Debris::Debris(SolidMovingSprite * target,
		 Vector3 position, 
		 Vector3 dimensions, 
		 Vector3 collisionDimensions, 
		 const char * textureFile,
		 bool nativeDimensions,
		 float speedMultiplier) :
	SolidMovingSprite(position.X, position.Y, position.Z, dimensions.X, dimensions.Y, dimensions.Z),
	mCreationTime(0.0f)  
{
	mIsDebris = true;

	m_textureFilename = textureFile;

	m_applyGravity = true;

	SetMaxVelocityXYZ(25 * speedMultiplier, 25 * speedMultiplier, 0);
	
	m_resistance.X = 0.99;
	m_resistance.Y = 2.5;

	srand(ID() * 500);
	
	float directionX = ((rand()%99) + 1) * 0.01;
	float directionY = ((rand()%99) + 1) * 0.01;

	float rand_speed = ((rand()%25) + 1) * speedMultiplier;

	bool plus_sign = (bool)(rand()%2);

	if (!plus_sign)
	{
		directionX = -directionX;
	}

	SetVelocityXYZ(directionX * rand_speed, directionY * rand_speed, 0);

	m_drawAtNativeDimensions = nativeDimensions;
	
	// set default material
	m_material = MaterialManager::Instance()->GetMaterial("default_orb");

	mCreationTime = Timing::Instance()->GetTotalTimeSeconds();
}

Debris::~Debris(void)
{
}

void Debris::Update(float delta)
{
	SolidMovingSprite::Update(delta);

	// TODO: review this code
	float targetDelta =  Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	SetRotationAngle(m_rotationAngle - (m_velocity.X * 0.02) * percentDelta);

	switch (mCurrentDebrisState)
	{
		case kActive:
		{
			break;
		}
		case kFadingOut:
		{
			mTimeUntilFadeOutFully -= delta;

			
			if (mTimeUntilFadeOutFully <= 0.0f)
			{
				mTimeUntilFadeOutFully = 0.0f;
				GameObjectManager::Instance()->RemoveGameObject(this, true);
			}
			else if (mTimeUntilFadeOutFully <= 0.2f)
			{
				float percentScale = mTimeUntilFadeOutFully / 0.2f;

				SetMatrixScaleX(percentScale);
				SetMatrixScaleY(percentScale);
			}

			break;
		}
	}
}

bool Debris::OnCollision(SolidMovingSprite* object)
{
	if (object->IsCurrencyOrb())
	{
		return false;
	}

	if (object->IsWaterBlock())
	{
		return false;
	}

	/*
	if (object->IsCharacter())
	{
		return false;
	}*/

	if (object->IsSolidLineStrip())
	{
		// SolidLineStrip * lineStrip = static_cast<SolidLineStrip*>(object);
		// HandleSolidLineStripCollision(lineStrip);
		// return false;

		if (mCurrentDebrisState == kActive)
		{
			mCurrentDebrisState = kFadingOut;
		}
	}

	return SolidMovingSprite::OnCollision(object);
}

void Debris::Initialise()
{
	SolidMovingSprite::Initialise();

	mOriginalDimensions = m_dimensions;
}