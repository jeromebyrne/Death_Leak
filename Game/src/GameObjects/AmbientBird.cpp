#include "precompiled.h"
#include "AmbientBird.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Orb.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"

static const float kFlapWingsMinDelay = 0.9f;
static const float kFlapWingsMaxDelay = 1.9f;
static const float kFlapTimeMinDelay = 2.0f;
static const float kFlapTimeMaxDelay = 3.4f;

AmbientBird::AmbientBird(void) :
	NPC(),
	mIsGliding(false),
	mTimeUntilFlap(kFlapWingsMinDelay),
	mTimeUntilEndFlap(kFlapTimeMinDelay),
	mTravelOffset(1000.0f)
{
	mAlwaysUpdate = true;
	mIsButterfly = true;
	mHealth = 1; // 1 hit kills for rabbits
	mMatchAnimFrameRateWithMovement = false;
	mAddHealthBar = false;
}


AmbientBird::~AmbientBird(void)
{
}

void AmbientBird::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	/*
	if (m_position.Z > 99)
	{
		// hack to stop butterfly's in the background being affected
		return;
	}
	m_alpha = 0.0f;
	mHasExploded = true;

	GameObjectManager::Instance()->RemoveGameObject(this);

	Orb * wing1 = new Orb(nullptr, Vector3(m_position.X, m_position.Y, m_position.Z - 1.1f), Vector3(32, 32, 0), Vector3(15, 15, 0), "Media\\characters\\butterfly\\dead_wing_1.png", true, 0.15f);
	GameObjectManager::Instance()->AddGameObject(wing1);

	Orb * wing2 = new Orb(nullptr, Vector3(m_position.X + 10, m_position.Y, m_position.Z - 1.1f), Vector3(32, 32, 0), Vector3(15, 15, 0), "Media\\characters\\butterfly\\dead_wing_2.png", true, 0.15f);
	GameObjectManager::Instance()->AddGameObject(wing2);

	ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
															m_position,
															Vector3(-m_direction.X, 0, 0),
															0.4,
															Vector3(3200, 1200, 0),
															"Media\\blast_circle.png",
															0.01,
															0.01,
															0.20f,
															0.35f,
															30,
															30,
															0,
															false,
															0.7,
															1.0,
															10000,
															true,
															3,
															0.0f,
															0.0f,
															0.1f,
															0.5f);
															*/
}

void AmbientBird::Initialise()
{
	NPC::Initialise();

	mCheckNPCOverlapCollisions = false;

	mPlayFootsteps = false;

	m_applyGravity = false;

	if (m_position.Z > 99)
	{
		// hack to stop butterfly's in the background being affected
		m_passive = true;
	}

	SetState(AIState::kNone); // let's just handle our own state

	m_direction.X = mTravelOffset;
	m_direction.Normalise();

	mStartPosition = m_position;
}

void AmbientBird::UpdateAnimations()
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	if (bodyPart != nullptr)
	{
		if (mIsGliding)
		{
			if (bodyPart->CurrentSequence()->Name() != "Still")
			{
				bodyPart->SetSequence("Still");

				bodyPart->CurrentSequence()->SetFrameRate(1);
			}
		}
		else
		{
			if (bodyPart->CurrentSequence()->Name() != "Running")
			{
				bodyPart->SetSequence("Running");

				bodyPart->CurrentSequence()->SetFrameRate(7);
			}
		}
		
		bodyPart->AnimateLooped();

		m_texture = bodyPart->CurrentFrame(); // set the current texture
	}

	m_mainBodyTexture = m_texture;
}

bool AmbientBird::OnCollision(SolidMovingSprite * object)
{
	if (m_position.Z > 99)
	{
		// hack to stop butterfly's in the background being affected
		return false;
	}

	return NPC::OnCollision(object);
}

void AmbientBird::Update(float delta)
{
	NPC::Update(delta);

	AccelerateX(m_direction.X);

	if (m_direction.X > 0)
	{
		if (m_position.X > (mStartPosition.X + mTravelOffset))
		{
			if (!Camera2D::GetInstance()->IsObjectInView(this))
			{
				m_position = mStartPosition;
			}
		}
	}
	else
	{
		if (m_position.X < (mStartPosition.X + mTravelOffset))
		{
			if (!Camera2D::GetInstance()->IsObjectInView(this))
			{
				m_position = mStartPosition;
			}
		}
	}

	if (mIsGliding)
	{
		mTimeUntilFlap -= delta;

		if (mTimeUntilFlap <= 0.0f)
		{
			mTimeUntilFlap = (rand() % (unsigned)((kFlapWingsMaxDelay - kFlapWingsMinDelay) * 100)) + (kFlapWingsMinDelay * 100);
			mTimeUntilFlap *= 0.01;

			mIsGliding = false;
		}
	}
	else
	{
		mTimeUntilEndFlap -= delta;
		if (mTimeUntilEndFlap <= 0.0f)
		{
			mTimeUntilEndFlap = (rand() % (unsigned)((kFlapTimeMaxDelay - kFlapTimeMinDelay) * 100)) + (kFlapTimeMinDelay * 100);
			mTimeUntilEndFlap *= 0.01;

			mIsGliding = true;
		}
	}
}

void AmbientBird::XmlRead(TiXmlElement * element)
{
	NPC::XmlRead(element);

	mTravelOffset = XmlUtilities::ReadAttributeAsFloat(element, "travel_offset", "value");
}

void AmbientBird::XmlWrite(TiXmlElement * element)
{
	NPC::XmlWrite(element);

	TiXmlElement * travelOffset = new TiXmlElement("travel_offset");
	travelOffset->SetAttribute("value", Utilities::ConvertDoubleToString(mTravelOffset).c_str());
	element->LinkEndChild(travelOffset);
}

/*
void Character::XmlRead(TiXmlElement * element)
{
	// update base classes
	SolidMovingSprite::XmlRead(element);

	m_maxJumpSpeed = XmlUtilities::ReadAttributeAsFloat(element, "maxjumpspeed", "value");

	mAccelXRate = XmlUtilities::ReadAttributeAsFloat(element, "xaccelrate", "value");

	m_projectileOffset.X = XmlUtilities::ReadAttributeAsFloat(element, "projectileoffsetx", "value");

	m_projectileOffset.Y = XmlUtilities::ReadAttributeAsFloat(element, "projectileoffsety", "value");
}

void Character::XmlWrite(TiXmlElement * element)
{
	SolidMovingSprite::XmlWrite(element);

	TiXmlElement * jumpSpeedElem = new TiXmlElement("maxjumpspeed");
	jumpSpeedElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_maxJumpSpeed).c_str());
	element->LinkEndChild(jumpSpeedElem);

	TiXmlElement * xaccelrateElem = new TiXmlElement("xaccelrate");
	xaccelrateElem->SetAttribute("value", Utilities::ConvertDoubleToString(mAccelXRate).c_str());
	element->LinkEndChild(xaccelrateElem);

	TiXmlElement * projectileoffsetxElem = new TiXmlElement("projectileoffsetx");
	projectileoffsetxElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_projectileOffset.X).c_str());
	element->LinkEndChild(projectileoffsetxElem);

	TiXmlElement * projectileoffsetyElem = new TiXmlElement("projectileoffsety");
	projectileoffsetyElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_projectileOffset.Y).c_str());
	element->LinkEndChild(projectileoffsetyElem);
}
*/