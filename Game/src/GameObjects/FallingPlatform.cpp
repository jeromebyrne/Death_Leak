#include "precompiled.h"
#include "FallingPlatform.h"

const int kReturnTime = 2.0f;
const int kNumTriggerAlphaPulses = 2;

FallingPlatform::FallingPlatform(float x , float y , float z , float width , float height , float breadth ,float groundFriction , float airResistance ):
Platform(x,y,z,width,height,breadth,groundFriction,airResistance),
mCurrentState(kStatic),
mInitialPosition(x,y,z),
mTriggerStartTime(0.0f),
mMaxTriggerTime(1.5f),
mTimeUntilReturn(5.0f),
mFallStartTime(0.0f),
mReturnStartTime(0.0f),
mInitialReturnPosition(0,0,0)
{
}

FallingPlatform::~FallingPlatform(void)
{
}

void FallingPlatform::OnCollision(SolidMovingSprite * object)
{
	Platform::OnCollision(object);
	
	if (dynamic_cast<Player*>(object)) // TODO: optimise with flag
	{
		if(object->Bottom() > Y () && object->X() > Left() && object->X() < Right() && object->VelocityY() <= 0.0)
		{
			if (mCurrentState == kStatic)
			{
				mCurrentState = kTriggered;
				mTriggerStartTime = Timing::Instance()->GetTotalTimeSeconds();
			}
		}
	}
}

void FallingPlatform::Scale(float x, float y, bool ScalePosition)
{
	Platform::Scale(x,y,ScalePosition);

	// scale initial position
	mInitialPosition.X = mInitialPosition.X * x;
	mInitialPosition.Y = mInitialPosition.Y * y;
}

void FallingPlatform::XmlRead(TiXmlElement * element)
{
	Platform::XmlRead(element);

	mInitialPosition.X = XmlUtilities::ReadAttributeAsFloat(element, "position", "x");
	mInitialPosition.Y = XmlUtilities::ReadAttributeAsFloat(element, "position", "y");
	mInitialPosition.Z = XmlUtilities::ReadAttributeAsFloat(element, "position", "z");

	mMaxTriggerTime = XmlUtilities::ReadAttributeAsFloat(element, "maxtriggertime", "value");
	mTimeUntilReturn = XmlUtilities::ReadAttributeAsFloat(element, "timeuntilreturn", "value");
}

void FallingPlatform::XmlWrite(TiXmlElement * element)
{
	Platform::XmlWrite(element);
	
	TiXmlElement * maxtriggertimeElem = new TiXmlElement("maxtriggertime");
	maxtriggertimeElem->SetDoubleAttribute("value", mMaxTriggerTime);
	element->LinkEndChild(maxtriggertimeElem);

	TiXmlElement * timeuntilreturnElem = new TiXmlElement("timeuntilreturn");
	timeuntilreturnElem->SetDoubleAttribute("value", mTimeUntilReturn);
	element->LinkEndChild(timeuntilreturnElem);
}

void FallingPlatform::Update(float delta)
{
	switch (mCurrentState)
	{
		case kStatic:
			{
				break;
			}
		case kTriggered:
			{
				DoTriggerLogic();
				break;
			}
		case kFalling:
			{
				DoFallingLogic();
				break;
			}
		case kReturning:
			{
				DoReturnLogic();
				break;
			}
		default:
			{
				break;
			}
	};

	Platform::Update(delta);
}

void FallingPlatform::DoTriggerLogic()
{
	float current_time = Timing::Instance()->GetTotalTimeSeconds();
	float fall_time = mTriggerStartTime + mMaxTriggerTime;

	if (fall_time < current_time)
	{
		m_alpha = 1.0f;
		mCurrentState = kFalling;
		mFallStartTime = Timing::Instance()->GetTotalTimeSeconds();
		m_applyGravity = true;
		return;
	}
	
	// alpha pulse 4 times before drop
	float time_left = fall_time - current_time;
	float percent_left = ((float)time_left / (float)mMaxTriggerTime);
	if (percent_left > 0.75f)
	{
		float val = ((percent_left * 4) - 3) * 0.5f;
		m_alpha = 0.5f + val;
	}
	else if (percent_left > 0.5f)
	{
		float val = (3 - (percent_left * 4)) * 0.5f;
		m_alpha = 0.5f + val;
	}
	else if (percent_left > 0.25f)
	{
		float val = ((percent_left * 4) - 1) * 0.5f;
		m_alpha = 0.5f + val;
	}
	else
	{
		float val = (1 - (percent_left * 4)) * 0.5f;
		m_alpha = 0.5f + val;;
	}
}

void FallingPlatform::DoReturnLogic()
{
	// lerp back to the initial position
	float current_time = Timing::Instance()->GetTotalTimeSeconds();
	float return_at_time = mReturnStartTime + kReturnTime;
	float time_left = return_at_time - current_time;

	if (time_left < 0.01f)
	{
		mCurrentState = kStatic;
		m_position = mInitialPosition;
		return;
	}

	float percent_distance = 1 - ((float)time_left / (float)kReturnTime);

	// get the distance to the initial position
	Vector3 distance = mInitialPosition - mInitialReturnPosition;
	
	Vector3 increment = mInitialReturnPosition + (distance * percent_distance);

	m_position = increment;
}

void FallingPlatform::DoFallingLogic()
{
	if ( mFallStartTime + mTimeUntilReturn < Timing::Instance()->GetTotalTimeSeconds())
	{
		// time to return 
		mReturnStartTime = Timing::Instance()->GetTotalTimeSeconds();
		mInitialReturnPosition = m_position;
		mCurrentState = kReturning;

		m_applyGravity = false;
		m_velocity = Vector3(0,0,0);
		StopYAccelerating();
		StopXAccelerating();
	}
}

void FallingPlatform::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	Platform::OnDamage(damageAmount, pointOfContact, shouldExplode);

	if (shouldExplode)
	{
		if (mCurrentState == kStatic)
		{
			mCurrentState = kTriggered;
			mTriggerStartTime = Timing::Instance()->GetTotalTimeSeconds();
		}
	}
}
