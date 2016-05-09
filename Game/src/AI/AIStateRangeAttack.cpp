#include "precompiled.h"
#include "AIStateRangeAttack.h"
#include "NPC.h"

static float kRunAwayDelay = 1.0f;

static float kJumpRandomDelayMin = 2.0f;
static float kJumpRandomDelayMax = 5.5f;

AIStateRangeAttack::AIStateRangeAttack(NPC * npc) :
	AIState(npc),
	mDesiredRange(650),
	mFollowRange(900),
	mRandOffset(0.0f),
	mLastTimeRanAway(0.0f),
	mTimeUntilRandomlyJump(0.0f)
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

	mRandOffset = rand() % 220;

	mTimeUntilRandomlyJump = kJumpRandomDelayMin + (rand() % (int)((kJumpRandomDelayMax - kJumpRandomDelayMin) * 100.0f)) * 0.01f;
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
				// run away from player
				distanceSquaredVector.Normalise();
				m_npc->AccelerateX(-distanceSquaredVector.X);
			}
			else if (distanceSquared > (mFollowRange + mRandOffset) * (mFollowRange + mRandOffset))
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

		if (mTimeUntilRandomlyJump > 0.0f)
		{
			mTimeUntilRandomlyJump -= delta;
		}

		// let's just randomly jump
		if (m_npc->IsOnSolidSurface())
		{
			if (mTimeUntilRandomlyJump <= 0.0f)
			{
				if (m_npc->Jump(100.0f))
				{
					m_npc->SetVelocityY(0.5f);
					mTimeUntilRandomlyJump = kJumpRandomDelayMin + (rand() % (int)((kJumpRandomDelayMax - kJumpRandomDelayMin) * 100.0f)) * 0.01f;
				}
			}
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
