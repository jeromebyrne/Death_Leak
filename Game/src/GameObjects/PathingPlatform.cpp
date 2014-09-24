#include "precompiled.h"
#include "PathingPlatform.h"
#include "gameobjectmanager.h"

PathingPlatform::PathingPlatform(float x, float y, float z, float width, float height, float breadth,float groundFriction, float airResistance):
	Platform(x, y, z, width, height, breadth, groundFriction, airResistance),
	mCurrentPathState(kNotPathing),
	mCurrentPathIndex(0),
	mPlatformSpeed(8),
	mClosestPointToNextTarget((numeric_limits<int>::max)(), (numeric_limits<int>::max)(), 0.f),
	mPathingType(kAlwaysPathing),
	mPathForward(true)
{
}

PathingPlatform::~PathingPlatform(void)
{
}

void PathingPlatform::XmlRead(TiXmlElement * element)
{
	Platform::XmlRead(element);

	mPlatformSpeed = XmlUtilities::ReadAttributeAsFloat(element, "platformspeed", "value");

	string platform_type = XmlUtilities::ReadAttributeAsString(element, "platformpathtype", "value");
	
	if (platform_type == "pathalways")
	{
		mPathingType = kAlwaysPathing;
	}
	else if (platform_type == "pathwhentriggered")
	{
		mPathingType = kPathWhenTriggered;
	}

	string platform_state = XmlUtilities::ReadAttributeAsString(element, "platformpathstate", "value");

	if (platform_state == "notpathing")
	{
		mCurrentPathState = kNotPathing;
	}
	else if (platform_state == "pathforward")
	{
		mPathForward = true;
	}
	else if (platform_state == "pathbackward")
	{
		mPathForward = false;
	}
	
	TiXmlElement * sub_elem = element->FirstChildElement();

	while (sub_elem)
	{
		string name = sub_elem->Value();

		if (name == "pathpoints")
		{
			TiXmlElement * path_point = sub_elem->FirstChildElement();
			while (path_point)
			{
				Vector3 point = Vector3(0,0,0);
				
				point.X = XmlUtilities::ReadAttributeAsFloat(path_point, "", "x");
				point.Y = XmlUtilities::ReadAttributeAsFloat(path_point, "", "y");
				mPathPoints.push_back(point);
				path_point = path_point->NextSiblingElement();
			}

			break;
		}

		sub_elem = sub_elem->NextSiblingElement();
	}
}

void PathingPlatform::XmlWrite(TiXmlElement * element)
{
	Platform::XmlWrite(element);

	TiXmlElement * platformspeed = new TiXmlElement("platformspeed");
	platformspeed->SetAttribute("value", Utilities::ConvertDoubleToString(mPlatformSpeed).c_str());
	element->LinkEndChild(platformspeed);

	const char * type = "";
	switch(mPathingType)
	{
		case kAlwaysPathing:
		{
			type = "pathalways";
			break;
		}
		case kPathWhenTriggered:
		{
			type = "pathwhentriggered";
			break;
		}
		default:
			{
				break;
			}
	};

	TiXmlElement * platformpathtype = new TiXmlElement("platformpathtype");
	platformpathtype->SetAttribute("value", type);
	element->LinkEndChild(platformpathtype);

	const char * state = "";
	switch(mCurrentPathState)
	{
	case kNotPathing:
		{
			state = "notpathing";
			break;
		}
	case kLoopingForward:
		{
			state = "pathforward";
			break;
		}
	case kLoopingBackward:
		{
			state = "pathbackward";
			break;
		}
	case kReturningToStart:
		{
			state = "pathbackward";
			break;
		}
		default:
			{
				break;
			}
	};

	if (mPathForward)
	{
		state = "pathforward";
	}
	else
	{
		state = "pathbackward";
	}

	TiXmlElement * platformpathstate = new TiXmlElement("platformpathstate");
	platformpathstate->SetAttribute("value", state);
	element->LinkEndChild(platformpathstate);

	// path points
	TiXmlElement * pathpoints = new TiXmlElement("pathpoints");
	for (int i = 0; i < mPathPoints.size(); ++i)
	{
		TiXmlElement * point = new TiXmlElement("point");
		point->SetDoubleAttribute("x", mPathPoints[i].X);
		point->SetDoubleAttribute("y", mPathPoints[i].Y);
		pathpoints->LinkEndChild(point);
	}

	element->LinkEndChild(pathpoints);
}

void PathingPlatform::Scale(float x, float y, bool scalePosition)
{
	Platform::Scale(x, y, scalePosition);

	// hmmm how should I scale mPlatformSpeed? if aspect ratio changes then speed will change (don't change aspect ratio?)
	mPlatformSpeed *= x;
	
	// scale all of the path points
	for (int i = 0; i < mPathPoints.size(); ++i)
	{
		mPathPoints[i].X = mPathPoints[i].X * x;
		mPathPoints[i].Y = mPathPoints[i].Y * y;
	}
}

void PathingPlatform::OnCollision(SolidMovingSprite * object)
{
	Platform::OnCollision(object);
	
	if (mPathingType == kPathWhenTriggered &&
		(mCurrentPathState == kNotPathing || mCurrentPathState == kReturningToStart) && 
		dynamic_cast<Player*>(object))
	{
		if(object->Bottom() > Y () && object->X() > Left() && object->X() < Right() && object->VelocityY() <= 0.0)
		{
			mClosestPointToNextTarget = Vector3((numeric_limits<int>::max)(), (numeric_limits<int>::max)(), 0);

			if (mPathForward)
			{
				if (mCurrentPathState == kReturningToStart)
				{
					if (mCurrentPathIndex < (mPathPoints.size()-1))
					{
						++mCurrentPathIndex;
					}
					else
					{
						mCurrentPathIndex = 0;
					}
				}
				mCurrentPathState = kLoopingForward;
			}
			else
			{
				if (mCurrentPathState == kReturningToStart)
				{
					if (mCurrentPathIndex > 0)
					{
						--mCurrentPathIndex;
					}
				}
				mCurrentPathState = kLoopingBackward;
			}
		}
	}
}

void PathingPlatform::Initialise()
{
	Platform::Initialise();

	if (mPathingType == kAlwaysPathing)
	{
		// if not set then set to looping forward by default
		if (mCurrentPathState == kNotPathing)
		{
			if (mPathForward)
			{
				mCurrentPathState = kLoopingForward;
			}
			else
			{
				mCurrentPathState = kLoopingBackward;
			}
		}
	}
}

void PathingPlatform::Update(float delta)
{
	SolidMovingSprite::Update(delta);

	switch(mCurrentPathState)
	{
		case kLoopingForward:
			{
				if (mPathingType == kPathWhenTriggered)
				{
					// get the player
					const Player * player = GameObjectManager::Instance()->GetPlayer();

					if (player && player->GetObjectMovingWith() != this)
					{
						// the player is not colliding with us so let's start moving back to the start
						mCurrentPathState = kReturningToStart;

						mClosestPointToNextTarget = Vector3((numeric_limits<int>::max)(), (numeric_limits<int>::max)(), 0);

						// we take 1 away from the current index
						if (mPathForward)
						{
							if (mCurrentPathIndex > 0)
							{
								--mCurrentPathIndex;
							}
						}
						else
						{
							if (mCurrentPathIndex >= (mPathPoints.size()-1))
							{
								mCurrentPathIndex = 0;
							}
							else
							{
								++mCurrentPathIndex;
							}
						}
						
						break;
					}
				}

				PathForward();
				break;
			}
		case kLoopingBackward:
			{
				if (mPathingType == kPathWhenTriggered)
				{
					// get the player
					const Player * player = GameObjectManager::Instance()->GetPlayer();

					if (player && player->GetObjectMovingWith() != this)
					{
						// the player is not colliding with us so let's start moving back to the start
						mCurrentPathState = kReturningToStart;

						mClosestPointToNextTarget = Vector3((numeric_limits<int>::max)(), (numeric_limits<int>::max)(), 0);

						// we take 1 away from the current index
						if (mPathForward)
						{
							if (mCurrentPathIndex > 0)
							{
								--mCurrentPathIndex;
							}
						}
						else
						{
							if (mCurrentPathIndex >= (mPathPoints.size()-1))
							{
								mCurrentPathIndex = 0;
							}
							else
							{
								++mCurrentPathIndex;
							}
						}
						
						break;
					}
				}

				PathBackward();
				break;
			}
		case kReturningToStart:
			{
				ReturnToStart();
				break;
			}
		case kNotPathing:
		default:
			{
				m_velocity.X = 0;
				m_velocity.Y = 0;
				break;
			}
	};
}

void PathingPlatform::ReturnToStart()
{
	if (mPathPoints.size() > 0 && mCurrentPathIndex < (mPathPoints.size()))
	{
		Vector3 direction = mPathPoints[mCurrentPathIndex] - m_position;
		direction.Z = 0;

		if (direction.LengthSquared() < 10 * 10 || direction.LengthSquared() > mClosestPointToNextTarget.LengthSquared())
		{
			mClosestPointToNextTarget = Vector3((numeric_limits<int>::max)(), (numeric_limits<int>::max)(), 0);
			
			if (mCurrentPathIndex == 0)
			{
				mCurrentPathState = kNotPathing;
				return;
			}

			if (mPathForward)
			{
				if (mCurrentPathIndex > 0)
				{
					--mCurrentPathIndex;
					return;
				}
			}
			else
			{
				if (mCurrentPathIndex < mPathPoints.size()-1)
				{
					++mCurrentPathIndex;
					return;
				}
				else
				{
					mCurrentPathIndex = 0;
					return;
				}
			}
		}
		
		mClosestPointToNextTarget = direction;
		direction.Normalise();

		m_velocity.X = direction.X * mPlatformSpeed;
		m_velocity.Y = direction.Y * mPlatformSpeed;

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
	}
}

void PathingPlatform::PathForward()
{
	if (mPathPoints.size() > 0)
	{
		if ( mCurrentPathIndex < (mPathPoints.size()))
		{
			Vector3 direction = mPathPoints[mCurrentPathIndex] - m_position;
			direction.Z = 0;

			if (direction.LengthSquared() < 10 * 10 || direction.LengthSquared() > mClosestPointToNextTarget.LengthSquared())
			{
				mClosestPointToNextTarget = Vector3((numeric_limits<int>::max)(), (numeric_limits<int>::max)(), 0);

				if (mCurrentPathIndex < mPathPoints.size()-1)
				{
					++mCurrentPathIndex;
					return;
				}
				else
				{
					mCurrentPathIndex = 0;
					return;
				}
			}
			
			mClosestPointToNextTarget = direction;
			direction.Normalise();

			m_velocity.X = direction.X * mPlatformSpeed;
			m_velocity.Y = direction.Y * mPlatformSpeed;

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
		}
	}
}

void PathingPlatform::PathBackward()
{
	if (mPathPoints.size() > 0)
	{
		if ( mCurrentPathIndex < (mPathPoints.size()))
		{
			Vector3 direction = mPathPoints[mCurrentPathIndex] - m_position;
			direction.Z = 0;

			if (direction.LengthSquared() < 10 * 10 || direction.LengthSquared() > mClosestPointToNextTarget.LengthSquared())
			{
				mClosestPointToNextTarget = Vector3((numeric_limits<int>::max)(), (numeric_limits<int>::max)(), 0);

				if (mCurrentPathIndex > 0)
				{
					--mCurrentPathIndex;
					return;
				}
				else if (mCurrentPathIndex == 0)
				{
					mCurrentPathIndex = mPathPoints.size()-1;
					return;
				}
			}
			
			mClosestPointToNextTarget = direction;
			direction.Normalise();

			m_velocity.X = direction.X * mPlatformSpeed;
			m_velocity.Y = direction.Y * mPlatformSpeed;

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
		}
	}
}
