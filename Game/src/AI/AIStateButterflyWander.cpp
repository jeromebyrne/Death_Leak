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
}

void AIStateButterflyWander::Update(float delta)
{
	/*
	if (m_npc->m_player)
	{
		// get the distance to the player
		Vector3 distanceSquaredVector =  m_npc->m_player->Position() - m_npc->Position();
		float distanceSquared = distanceSquaredVector.LengthSquared();
		if (distanceSquared < mFollowRadius * mFollowRadius)
		{
			distanceSquaredVector.Normalise();
			m_npc->AccelerateX(-distanceSquaredVector.X);
		}
		else
		{
			m_npc->StopXAccelerating();
		}
	}
	*/

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