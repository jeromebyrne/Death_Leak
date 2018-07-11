#include "precompiled.h"
#include "MovingSprite.h"
#include "Environment.h"
#include "SolidLineStrip.h"
#include "ParticleEmitterManager.h"
#include "Game.h"

const float kGravityWhenFallingMultiplier = 3.0f;
const float kGravityWhenFallingVelocityThreshold = -15.0f;

MovingSprite::MovingSprite(float x, float y, DepthLayer depthLayer, float width, float height, float groundFriction, float airResistance):
	Sprite(x,y, depthLayer, width, height), 
	m_resistance(groundFriction, airResistance), 
	m_velocity(0.0f,0.0f),
	m_maxVelocity(0.0f,0.0f),
	m_acceleration(0.0f,0.0f),
	m_direction(1.0f,0.0f), 
	m_applyGravity(true),
	mObjectMovingWith(nullptr),
	mCurrentYResistance(1.0f),
	mCurrentXResistance(1.0f),
	mIsInWater(false),
	mWasInWaterLastFrame(false),
	mTimeUntilCanSpawnWaterBubbles(0.0f),
	mHittingSolidLineEdge(false),
	mMaxVelocityXLimitEnabled(true)
{
	if (m_maxVelocity.Y < 0.0f)  // less than 0 actually signifies no maximum
	{
		m_maxVelocity.Y = (numeric_limits<int>::max)();
	}
}

MovingSprite::~MovingSprite(void)
{
}

void MovingSprite::Scale(float xScale, float yScale, bool scalePosition)
{
	Sprite::Scale(xScale, yScale, scalePosition);

	m_maxVelocity.X = m_maxVelocity.X * xScale;
	m_maxVelocity.Y = m_maxVelocity.Y * yScale;

	m_acceleration.X = m_acceleration.X * xScale;
	m_acceleration.Y = m_acceleration.Y * yScale;

	//m_resistance.X = m_resistance.X * xScale;
	m_resistance.Y = m_resistance.Y / yScale;
}

void MovingSprite::Update(float delta)
{	
	// TODO: optmize so stuff like SolidLineStrip isn't calling this function

	// update our base class 
	Sprite::Update(delta);

	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}

	if (GetIsInWater() &&
			(std::abs(m_velocity.X) > 0.1f || (std::abs(m_velocity.Y) > 1.0f &&
			!(IsCharacter() && static_cast<Character *>(this)->IsOnSolidSurface() ))))
	{
		DoWaterAccelerationBubbles();
	}

	float velocityMod = mIsInWater ? 0.2f : 1.0f;
	Vector2 nextVelocity = m_velocity + (m_acceleration * m_direction) * velocityMod;

	if (nextVelocity.X > m_maxVelocity.X * velocityMod && mMaxVelocityXLimitEnabled)
	{
		nextVelocity.X = m_maxVelocity.X * velocityMod;
	}
	else if (nextVelocity.X < -m_maxVelocity.X * velocityMod && mMaxVelocityXLimitEnabled)
	{
		nextVelocity.X = -m_maxVelocity.X * velocityMod;
	}

	if (nextVelocity.Y > m_maxVelocity.Y * velocityMod)
	{
		nextVelocity.Y = m_maxVelocity.Y * velocityMod;
	}
	else if (nextVelocity.Y < -m_maxVelocity.Y * velocityMod)
	{
		nextVelocity.Y = -m_maxVelocity.Y * velocityMod;
	}

	mHittingSolidLineEdge = false;

	if (IsCharacter())
	{
		Character * character = static_cast<Character *>(this);
		if (character->IsOnSolidLine())
		{
			SolidLineStrip * solidLineStrip = character->GetCurrentSolidLineStrip();
			GAME_ASSERT(solidLineStrip);

			if (solidLineStrip)
			{
				Vector2 rightMostPoint = solidLineStrip->GetRightMostPoint();
				Vector2 leftMostPoint = solidLineStrip->GetLeftMostPoint();

				float nextPositionX = m_position.X + nextVelocity.X;

				if ((solidLineStrip->GetHasHardRightEdge() && 
					nextVelocity.X > 0.0f &&
					nextPositionX > rightMostPoint.X - solidLineStrip->GetHardRightEdgeOffsetX()) ||
					(solidLineStrip->GetHasHardLeftEdge() && 
					nextVelocity.X < 0.0f &&
					nextPositionX < leftMostPoint.X + solidLineStrip->GetHardLeftEdgeOffsetX()))
				{
					nextVelocity.X = 0.0f;
					StopXAccelerating();
					mHittingSolidLineEdge = true;
				}
			}
		}
	}

	// increase velocity
	m_velocity = nextVelocity;

	bool isOnSolidSurface = IsOnSolidSurface();

	float targetDelta = Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	if (m_applyGravity && !isOnSolidSurface)
	{
		if (!mIsInWater)
		{
			if (m_velocity.Y > 0.0f || m_velocity.Y < kGravityWhenFallingVelocityThreshold)
			{
				AccelerateY(-1.0f, (mGravityApplyAmount / mCurrentYResistance) * percentDelta);
			}
			else
			{
				AccelerateY(-1.0f, ((mGravityApplyAmount * kGravityWhenFallingMultiplier) / mCurrentYResistance) * percentDelta);
			}
			
		}
		else
		{
			if (m_velocity.Y > -2.0f) // TODO: make this a constant
			{
				float waterGravityMultiplier = 0.15f; // TODO: make this a constant
				AccelerateY(-1.0f, (mGravityApplyAmount * waterGravityMultiplier / mCurrentYResistance) * percentDelta);
			}
		}
	}

	if (mObjectMovingWith)
	{
		m_position += m_velocity * percentDelta;
		m_position.X += mObjectMovingWith->VelocityX() * percentDelta;
	}
	else
	{
		m_position += m_velocity * percentDelta; // update our position by velocity
	}

	// apply friction values
	m_velocity.X = m_velocity.X * mCurrentXResistance;

	// stop us if we get too slow
	if(m_velocity.X < 0.1 && m_velocity.X > -0.1)
	{
		// then stop
		m_velocity.X = 0;
	}

	if (mIsInWater == false)
	{
		mWasInWaterLastFrame = false;
	}

	mIsInWater = false;// let the collision manager handle this again

	if (mTimeUntilCanSpawnWaterBubbles > 0.0f)
	{
		mTimeUntilCanSpawnWaterBubbles -= delta;
	}
}

void MovingSprite::Initialise()
{
	Sprite::Initialise();

	mCurrentYResistance = m_resistance.Y;
	mCurrentXResistance = m_resistance.X;
}

void MovingSprite::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	// read friction values
	m_resistance.X = XmlUtilities::ReadAttributeAsFloat(element, "resistance", "x");
	m_resistance.Y = XmlUtilities::ReadAttributeAsFloat(element, "resistance", "y");

	m_maxVelocity.X = XmlUtilities::ReadAttributeAsFloat(element, "maxvelocity", "x");
	m_maxVelocity.Y = XmlUtilities::ReadAttributeAsFloat(element, "maxvelocity", "y");

	if (m_maxVelocity.Y < 0.0f) // less than 0 actually signifies no maximum
	{
		// TODO: This is a very bad idea, no idea why I originally wanted this
		m_maxVelocity.Y = 99999.f;
	}

	m_applyGravity = XmlUtilities::ReadAttributeAsBool(element, "applygravity", "value");
}

void MovingSprite::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	TiXmlElement * resistanceElem = new TiXmlElement("resistance");
	resistanceElem->SetDoubleAttribute("y", m_resistance.Y);
	resistanceElem->SetDoubleAttribute("x", m_resistance.X);
	element->LinkEndChild(resistanceElem);

	TiXmlElement * maxvelocityElem = new TiXmlElement("maxvelocity");
	maxvelocityElem->SetDoubleAttribute("y", m_maxVelocity.Y);
	maxvelocityElem->SetDoubleAttribute("x", m_maxVelocity.X);
	element->LinkEndChild(maxvelocityElem);

	const char * applGravValAsStr = m_applyGravity ? "true" : "false";
	TiXmlElement * applygravityElem = new TiXmlElement("applygravity");
	applygravityElem->SetAttribute("value", applGravValAsStr);
	element->LinkEndChild(applygravityElem);
}

void MovingSprite::AccelerateX(float directionX, float rate)
{
	if(directionX < 0.0f)
	{
		directionX = -1.0f;  // normalise

		if(m_direction.X > -1.0f)
		{
			m_acceleration.X = 0.0f;
		}
		m_direction.X = directionX;
		m_acceleration.X = rate;
	}
	else if(directionX > 0.0f)
	{
		directionX = 1.0f; //normalise

		if(m_direction.X < 1.0f)
		{
			m_acceleration.X = 0.0f;
		}
		m_direction.X = directionX;
		m_acceleration.X = rate;
	}
}

void MovingSprite::AccelerateY(float directionY, float rate)
{
	if(directionY < 0.0f)
	{
		directionY = -1.0f;  // normalise

		if(m_direction.Y > -1.0f)
		{
			m_acceleration.Y = 0.0f;
		}
		m_direction.Y = directionY;
		m_acceleration.Y = rate;
	}
	else if(directionY > 0.0f)
	{
		directionY = 1.0f; //normalise

		if(m_direction.Y < 1.0f)
		{
			m_acceleration.Y = 0.0f;
		}
		m_direction.Y = directionY;
		m_acceleration.Y = rate;
	}
}

void MovingSprite::StopXAccelerating()
{
	m_acceleration.X = 0.0f;
}
void MovingSprite::StopYAccelerating()
{
	m_acceleration.Y = 0.0f;
}

void MovingSprite::SetIsInWater(bool value)
{
	mIsInWater = value;

	if (mIsInWater)
	{
		// TODO: what is the point of this?
		mWasInWaterLastFrame = true;
	}
}

void MovingSprite::DoWaterAccelerationBubbles()
{
	if (mTimeUntilCanSpawnWaterBubbles <= 0.0f)
	{
		ParticleEmitterManager::Instance()->CreateDirectedSpray(30,
																Vector2(m_position.X - (m_direction.X < 0.0f ? 30.0f : -30.0f), m_position.Y),
																GetDepthLayer(),
																Vector2(-m_direction.X, -m_direction.Y),
																0.15f,
																Vector2(3200.0f, 2000.0f),
																"Media\\Ambient\\bubble.png",
																0.03f,
																0.15f,
																1.5f,
																3.0f,
																2.0f,
																4.0f,
																-0.2f,
																false,
																1.0f,
																1.0f,
																-1.0f,
																true,
																2.0f,
																0.01f * Dimensions().X,
																0.03f * Dimensions().Y,
																0.0f,
																0.9f);

		mTimeUntilCanSpawnWaterBubbles = 0.2f;
	}
}