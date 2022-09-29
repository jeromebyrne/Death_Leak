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
		 Vector2 position, 
		 DepthLayer depthLayer,
		 Vector2 dimensions, 
		 Vector2 collisionDimensions, 
		 const char * textureFile,
		 bool nativeDimensions,
		 float speedMultiplier) :
	SolidMovingSprite(position.X, position.Y, depthLayer, dimensions.X, dimensions.Y),
	mCreationTime(0.0f)  
{
	mIsDebris = true;

	m_textureFilename = textureFile;

	m_applyGravity = true;

	SetMaxVelocityXY(25.0f * speedMultiplier, 25.0f * speedMultiplier);
	
	m_resistance.X = 0.95f;
	m_resistance.Y = 1.0f;

	// srand(ID() * 500);
	
	float directionX = 1.0f; //((rand()%99) + 1) * 0.01f;
	float directionY = 0.5f; //((rand()%99) + 1) * 0.01f;

	float rand_speed = ((rand()%20) + 5) * speedMultiplier;

	bool plus_sign = (bool)(rand()%2);

	if (!plus_sign)
	{
		directionX = -directionX;
	}

	SetVelocityXY(directionX * rand_speed, directionY * rand_speed);

	m_drawAtNativeDimensions = nativeDimensions;
	
	// set default material
	m_material = MaterialManager::Instance()->GetMaterial("default_orb");

	mCreationTime = Timing::Instance()->GetTotalTimeSeconds();

	m_collisionBoxDimensions = collisionDimensions;

	// disabled bouncing as it's not working correctly
	mBouncable = false;
}

Debris::~Debris(void)
{
}

void Debris::Update(float delta)
{
	mHasUpdatedOnce = true;

	SolidMovingSprite::Update(delta);

	// TODO: review this code
	float targetDelta =  Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	SetRotationAngle(m_rotationAngle - (m_velocity.X * (mIsOnSolidLine ? 0.003 : 0.02)) * percentDelta);

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
				mAlwaysUpdate = false;
				m_updateable = false;
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
	if (!mHasUpdatedOnce)
	{
		return false;
	}

	if (!mCollidesWithOtherDebris)
	{
		if (object->IsDebris())
		{
			return false;
		}
	}

	if (object->IsCurrencyOrb())
	{
		return false;
	}

	if (object->IsWaterBlock())
	{
		return false;
	}

	if (object->IsProjectile())
	{
		return false;
	}

	if (object->IsFoliage())
	{
		return false;
	}

	if (object->IsCharacter())
	{
		Character * c = static_cast<Character*>(object);

		if (std::abs(c->VelocityX()) > 2.0f)
		{
			m_velocity.X *= 2.0f;
		}
	}

	if (object->IsSolidLineStrip())
	{
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
}