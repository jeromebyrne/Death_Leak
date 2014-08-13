#include "precompiled.h"
#include "MovingSprite.h"
#include "Environment.h"

MovingSprite::MovingSprite(float x, float y, float z, float width, float height, float breadth, float groundFriction, float airResistance):
	Sprite(x,y,z, width, height, breadth), 
	m_resistance(groundFriction, airResistance, 0), 
	m_velocity(0,0,0),
	m_maxVelocity(0,0,0),
	m_acceleration(0,0,0),
	m_direction(1,0,0), 
	m_applyGravity(true),
	mObjectMovingWith(0),
	mCurrentYResistance(1),
	m_isOnGround(true),
	mIsInWater(false)
{
	if (m_maxVelocity.Y < 0)  // less than 0 actually signifies no maximum
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
	// update our base class 
	Sprite::Update(delta);

	float velocityMod = mIsInWater ? 0.3f : 1.0f;

	// increase velocity
	m_velocity += (m_acceleration * m_direction) * velocityMod;

	if(m_velocity.X > m_maxVelocity.X)
	{
		m_velocity.X = m_maxVelocity.X;
	}
	else if(m_velocity.X < -m_maxVelocity.X)
	{
		m_velocity.X = -m_maxVelocity.X;
	}

	if(m_velocity.Y > m_maxVelocity.Y)
	{
		m_velocity.Y = m_maxVelocity.Y;
	}
	else if(m_velocity.Y < -m_maxVelocity.Y)
	{
		m_velocity.Y = -m_maxVelocity.Y;
	}

	float targetDelta =  Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	if (percentDelta > 1.4f)
	{
		percentDelta = 1.4f;
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

	int groundLevel = Environment::Instance()->GroundLevel();
	
	float bottom = Bottom();
	if (bottom <= groundLevel)
	{
		m_isOnGround = true;
		if(bottom < groundLevel) // if below ground level then set at ground level
		{
			m_position.Y = (groundLevel + m_dimensions.Y/2);
			StopYAccelerating();
			m_velocity.Y = 0;
		}
	}
	else
	{
		m_isOnGround = false;
	}

	// apply gravity?
	int fakeGravity = 1; // must be greater than 1
	if(m_applyGravity && bottom > groundLevel) //if above ground level then apply gravity
	{
		if (!mIsInWater)
		{
			AccelerateY(-1, ((float)fakeGravity/mCurrentYResistance) * percentDelta );
		}
		else
		{
			AccelerateY(-1, ((float)fakeGravity/mCurrentYResistance * 0.3f) * percentDelta );
		}
	}

	// apply friction values
	m_velocity.X = m_velocity.X * m_resistance.X;

	// stop us if we get too slow
	if(m_velocity.X < 0.1 && m_velocity.X > -0.1)
	{
		// then stop
		m_velocity.X = 0;
	}
	if(m_velocity.X > m_maxVelocity.X)
	{
		m_velocity.X = m_maxVelocity.X;
	}
	else if(m_velocity.X < -m_maxVelocity.X)
	{
		m_velocity.X = -m_maxVelocity.X;
	}

	if(m_velocity.Y > m_maxVelocity.Y)
	{
		m_velocity.Y = m_maxVelocity.Y;
	}
	else if(m_velocity.Y < -m_maxVelocity.Y)
	{
		m_velocity.Y = -m_maxVelocity.Y;
	}

	mIsInWater = false;// let the collision manager handle this again
}

void MovingSprite::Initialise()
{
	Sprite::Initialise();

	mCurrentYResistance = m_resistance.Y;
}


void MovingSprite::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	// read friction values
	m_resistance.X = XmlUtilities::ReadAttributeAsFloat(element, "resistance", "x");
	m_resistance.Y = XmlUtilities::ReadAttributeAsFloat(element, "resistance", "y");
	m_resistance.Z = XmlUtilities::ReadAttributeAsFloat(element, "resistance", "z");

	m_maxVelocity.X = XmlUtilities::ReadAttributeAsFloat(element, "maxvelocity", "x");
	m_maxVelocity.Y = XmlUtilities::ReadAttributeAsFloat(element, "maxvelocity", "y");
	m_maxVelocity.Z = XmlUtilities::ReadAttributeAsFloat(element, "maxvelocity", "z");

	if (m_maxVelocity.Y < 0) // less than 0 actually signifies no maximum
	{
		m_maxVelocity.Y = 99999.f;
	}

	m_applyGravity = XmlUtilities::ReadAttributeAsBool(element, "applygravity", "value");
}

void MovingSprite::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	TiXmlElement * resistanceElem = new TiXmlElement("resistance");
	resistanceElem->SetDoubleAttribute("z", m_resistance.Z);
	resistanceElem->SetDoubleAttribute("y", m_resistance.Y);
	resistanceElem->SetDoubleAttribute("x", m_resistance.X);
	element->LinkEndChild(resistanceElem);

	TiXmlElement * maxvelocityElem = new TiXmlElement("maxvelocity");
	maxvelocityElem->SetDoubleAttribute("z", m_maxVelocity.Z);
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
	if(directionX < 0)
	{
		directionX = -1;  // normalise

		if(m_direction.X > -1)
		{
			m_acceleration.X = 0;
		}
		m_direction.X = directionX;
		m_acceleration.X = rate;

	}
	else if(directionX > 0)
	{
		directionX = 1; //normalise

		if(m_direction.X < 1)
		{
			m_acceleration.X = 0;
		}
		m_direction.X = directionX;
		m_acceleration.X = rate;
	}
}

void MovingSprite::AccelerateY(float directionY, float rate)
{
	if(directionY < 0)
	{
		directionY = -1;  // normalise

		if(m_direction.Y > -1)
		{
			m_acceleration.Y = 0;
		}
		m_direction.Y = directionY;
		m_acceleration.Y = rate;
	}
	else if(directionY > 0)
	{
		directionY = 1; //normalise

		if(m_direction.Y < 1)
		{
			m_acceleration.Y = 0;
		}
		m_direction.Y = directionY;
		m_acceleration.Y = rate;
	}
}

void MovingSprite::StopXAccelerating()
{
	m_acceleration.X = 0;
}
void MovingSprite::StopYAccelerating()
{
	m_acceleration.Y = 0;
}