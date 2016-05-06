#include "precompiled.h"
#include "AIStateRangeAttack.h"
#include "NPC.h"

static float kRunAwayDelay = 1.0f;

AIStateRangeAttack::AIStateRangeAttack(NPC * npc) :
	AIState(npc),
	mDesiredRange(650),
	mFollowRange(900),
	mRandOffset(0.0f),
	mLastTimeRanAway(0)
{
	mStateType = kRangeAttack;
}

AIStateRangeAttack::~AIStateRangeAttack(void)
{
}

void AIStateRangeAttack::OnTransition()
{
	float randMaxXVelocity = rand() % 3000;
	randMaxXVelocity *= 0.001;
	randMaxXVelocity += 20;

	m_npc->SetMaxVelocityXYZ(randMaxXVelocity, 99999, 0);

	mRandOffset = rand() % 150;
}

void AIStateRangeAttack::Update(float delta)
{
	if (m_npc->m_player)
	{
		Vector3 distanceSquaredVector = m_npc->m_player->Position() - m_npc->Position();
		float currentTime = Timing::Instance()->GetTotalTimeSeconds();
		if (currentTime > mLastTimeRanAway + kRunAwayDelay)
		{
			// get the distance to the player
			float distanceSquared = distanceSquaredVector.LengthSquared();
			if (distanceSquared < (mDesiredRange + mRandOffset) * (mDesiredRange + mRandOffset))
			{
				distanceSquaredVector.Normalise();
				m_npc->AccelerateX(-distanceSquaredVector.X);
			}
			else if (distanceSquared >(mFollowRange + mRandOffset) * (mFollowRange + mRandOffset))
			{
				distanceSquaredVector.Normalise();
				m_npc->AccelerateX(distanceSquaredVector.X);
			}
			else
			{
				m_npc->StopXAccelerating();
				GAME_ASSERT(GameObjectManager::Instance()->GetPlayer());
				m_npc->FireProjectileAtObject(GameObjectManager::Instance()->GetPlayer());

				mLastTimeRanAway = currentTime;
			}
		}
		else
		{
			m_npc->StopXAccelerating();
			GAME_ASSERT(GameObjectManager::Instance()->GetPlayer());
			m_npc->FireProjectileAtObject(GameObjectManager::Instance()->GetPlayer());
		}

		m_npc->SetIsStrafing(true);
		if (distanceSquaredVector.X > 0)
		{
			m_npc->SetStrafeDirectionX(1.0f);
		}
		else
		{
			m_npc->SetStrafeDirectionX(-1.0f);
		}
	}
}
