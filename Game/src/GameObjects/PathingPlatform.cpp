#include "precompiled.h"
#include "PathingPlatform.h"
#include "gameobjectmanager.h"

PathingPlatform::PathingPlatform(float x, float y, DepthLayer depthLayer, float width, float height,float groundFriction, float airResistance):
	Platform(x, y, depthLayer, width, height, groundFriction, airResistance),
	mCurrentPathState(kNotPathing),
	mCurrentPathIndex(0),
	mPlatformSpeed(8.0f),
	mClosestPointToNextTarget((numeric_limits<int>::max)(), (numeric_limits<int>::max)()),
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

	mOriginalPosition = m_position;

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
				Vector2 point = Vector2(0.0f,0.0f);
				
				point.X = mOriginalPosition.X + XmlUtilities::ReadAttributeAsFloat(path_point, "", "x");
				point.Y = mOriginalPosition.Y + XmlUtilities::ReadAttributeAsFloat(path_point, "", "y");
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
		point->SetDoubleAttribute("x", mPathPoints[i].X - mOriginalPosition.X);
		point->SetDoubleAttribute("y", mPathPoints[i].Y  - mOriginalPosition.Y);
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

bool PathingPlatform::OnCollision(SolidMovingSprite * object)
{
	/*
	if (object->IsSolidLine())
	{
		return;
	}

	// if we are not passive then push ourselves away from the object
	// remember this behaviour can be overwritten in derived function
	if (!m_passive && !object->IsProjectile()) // by default we don't want to be pushed by a projectile
	{
		float otherTop = object->CollisionTop();
		float otherBottom = object->CollisionBottom();
		float otherY = object->Y() + object->CollisionBoxOffset().Y;

		float thisY = this->Y() + CollisionBoxOffset().Y;
		float thisTop = this->CollisionTop();
		float thisBottom = this->CollisionBottom();

		// get the overlap
		float yOverlap = 0.0f;

		// below
		if (thisBottom > otherBottom && thisBottom < otherTop)
		{
			yOverlap = otherBottom - thisTop;
		}
		else
		{
			mObjectMovingWith = nullptr;
			return;
		}

		m_onTopOfOtherSolidObject = true;

		m_position.Y += yOverlap;
		StopYAccelerating();

		if (mBouncable)
		{
			m_velocity.Y = m_velocity.Y * -mBounceDampening;
		}
	}

	if (m_applyDamage)
	{
		if (object->IsCharacter())
		{
			GAME_ASSERT(dynamic_cast<Character*>(object));
			Character * character = static_cast<Character*>(object);
			character->OnDamage(this, m_applyDamageAmount, Vector3(0, 0, 0));
		}
	}
	*/


	// *** Platform triggers ***
	if (mPathingType == kPathWhenTriggered &&
		(mCurrentPathState == kNotPathing || mCurrentPathState == kReturningToStart) && 
		dynamic_cast<Player*>(object))
	{
		if(object->Bottom() > Y () && object->X() > Left() && object->X() < Right() && object->VelocityY() <= 0.0f)
		{
			mClosestPointToNextTarget = Vector2((numeric_limits<int>::max)(), (numeric_limits<int>::max)());

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

	return true;
}

void PathingPlatform::Initialise()
{
	Platform::Initialise();

	mAlwaysUpdate = true;

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

						mClosestPointToNextTarget = Vector2((numeric_limits<int>::max)(), (numeric_limits<int>::max)());

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

						mClosestPointToNextTarget = Vector2((numeric_limits<int>::max)(), (numeric_limits<int>::max)());

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
		Vector2 direction = mPathPoints[mCurrentPathIndex] - m_position;

		if (direction.LengthSquared() < 10.0f * 10.0f || direction.LengthSquared() > mClosestPointToNextTarget.LengthSquared())
		{
			mClosestPointToNextTarget = Vector2((numeric_limits<int>::max)(), (numeric_limits<int>::max)());
			
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
			Vector2 direction = mPathPoints[mCurrentPathIndex] - m_position;

			if (direction.LengthSquared() < 10 * 10 || direction.LengthSquared() > mClosestPointToNextTarget.LengthSquared())
			{
				mClosestPointToNextTarget = Vector2((numeric_limits<int>::max)(), (numeric_limits<int>::max)());

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
			Vector2 direction = mPathPoints[mCurrentPathIndex] - m_position;

			if (direction.LengthSquared() < 10.0f * 10.0f || direction.LengthSquared() > mClosestPointToNextTarget.LengthSquared())
			{
				mClosestPointToNextTarget = Vector2((numeric_limits<int>::max)(), (numeric_limits<int>::max)());

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
