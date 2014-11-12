#include "precompiled.h"
#include "AIStateButterflyWander.h"
#include "NPC.h"

static const float kYVelocityChangeDelay = 0.2f;

AIStateButterflyWander::AIStateButterflyWander(NPC * npc) :
	AIState(npc),
	mCurrentYVelocityDelay(0.0f),
	mLastYVelocityBurst(0.0f),
	mCurrentDirectionChangeDelay(0.0f),
	mLastDirectionChangeDelay(0.0f)
{
	mStateType = kButterflyWander;
}

AIStateButterflyWander::~AIStateButterflyWander(void)
{
}

void AIStateButterflyWander::OnTransition()
{
	// do stuff
	if (m_npc && m_npc->m_position.Z > 99)
	{
		m_npc->m_resistance.Y = 20.1f;
	}
}

void AIStateButterflyWander::Update(float delta)
{
	if (m_npc->m_player && m_npc->m_position.Z < 99) // Hack: don;t be affceted if in the background
	{
		// get the distance to the player
		Vector3 distanceSquaredVector = m_npc->m_player->Position() - m_npc->Position();
		float distanceSquared = distanceSquaredVector.LengthSquared();
		if (distanceSquared < 130 * 130)
		{
			distanceSquaredVector.Normalise();
			m_npc->AccelerateY(1, 2.5f);
		}

		DoWander(delta);
	}
	else
	{
		m_npc->SetSprintActive(false);
		DoWander(delta);
	}
}

void AIStateButterflyWander::DoWander(float delta)
{
	mCurrentYVelocityDelay -= delta;

	if (mCurrentYVelocityDelay < 0.0f && m_npc->VelocityY() < -mLastYVelocityBurst)
	{
		m_npc->StopYAccelerating();

		mLastYVelocityBurst = 1000 + (rand() % 2500);
		mLastYVelocityBurst *= 0.001;

		m_npc->SetVelocityY(mLastYVelocityBurst);

		mCurrentYVelocityDelay = kYVelocityChangeDelay;
	}

	mCurrentDirectionChangeDelay -= delta;

	if (mCurrentDirectionChangeDelay < 0.0f)
	{
		unsigned randSign = rand() % 2;
		if (randSign == 0)
		{
			m_npc->m_direction.X = -1;
		}
		else
		{
			m_npc->m_direction.X = 1;
		}

		mCurrentDirectionChangeDelay = 500 + (rand() % 1500);
		mCurrentDirectionChangeDelay *= 0.001f;

		mLastDirectionChangeDelay = mCurrentDirectionChangeDelay;
	}

	m_npc->AccelerateX(m_npc->m_direction.X);
}
