#include "precompiled.h"
#include "AIStateRangeAttack.h"
#include "NPC.h"

static const float kRunAwayDelay = 1.2f;
static const float kJumpOrRollRandomDelayMin = 6.0f;
static const float kJumpOrRollRandomDelayMax = 10.5f;
static const float kTeleportDelayMin = 2.0f;
static const float kTeleportDelayMax = 7.0f;
static const float kTeleportDistance = 3000.0f;

AIStateRangeAttack::AIStateRangeAttack(NPC * npc) :
	AIState(npc),
	mDesiredRange(500.0f),
	mFollowRange(780.0f),
	mRandOffset(0.0f),
	mLastTimeRanAway(0.0f),
	mTimeUntilRandomlyJumpOrRoll(0.0f),
	mTimeUntilCanTeleport(0.0f)
{
	mStateType = kRangeAttack;
}

AIStateRangeAttack::~AIStateRangeAttack(void)
{
}

void AIStateRangeAttack::OnTransition()
{
	float randMaxXVelocity = rand() % 3000;
	randMaxXVelocity *= 0.001f;
	randMaxXVelocity += 10.0f;

	m_npc->SetMaxVelocityXYZ(randMaxXVelocity, 99999, 0);

	mRandOffset = rand() % 180;

	mTimeUntilRandomlyJumpOrRoll = kJumpOrRollRandomDelayMin + (rand() % (int)((kJumpOrRollRandomDelayMax - kJumpOrRollRandomDelayMin) * 100.0f)) * 0.01f;
}

void AIStateRangeAttack::Update(float delta)
{
	if (m_npc->GetIsRolling() ||
		m_npc->IsStunned() ||
		m_npc->JustFellFromLargeDistance())
	{
		// can't do anything while rolling
		m_npc->StopXAccelerating();
		m_npc->SetIsStrafing(false);
		return;
	}

	if (m_npc->m_player)
	{
		if (!CanAccelerateX(1.0f))
		{
			m_npc->Teleport(m_npc->m_player->Position().X - 300, m_npc->m_player->Position().Y + 300, true);
			mLastTimeRanAway = 0.0f;
			return; // skip this update
		}
		else if (!CanAccelerateX(-1.0f))
		{
			m_npc->Teleport(m_npc->m_player->Position().X + 300, m_npc->m_player->Position().Y + 300, true);
			mLastTimeRanAway = 0.0f;
			return; // skip this update
		}

		bool npcInView = Camera2D::GetInstance()->IsObjectInView(m_npc);

		Vector3 distanceSquaredVector = m_npc->m_player->Position() - m_npc->Position();

		if (npcInView && 
			std::abs(distanceSquaredVector.X > kTeleportDistance) ||
			(std::abs(distanceSquaredVector.Y > 1000) && m_npc->m_player->IsOnSolidSurface() && m_npc->m_player->GetTimeOnSolidSurface() > 1.0f))
		{
			if (mTimeUntilCanTeleport <= 0.0f)
			{
				m_npc->Teleport(m_npc->m_player->Position().X + 5, m_npc->m_player->Position().Y + 500, true);
				mLastTimeRanAway = 0.0f;
				mTimeUntilCanTeleport = kTeleportDelayMin + (rand() % (int)((kTeleportDelayMax - kTeleportDelayMin) * 100.0f)) * 0.01f;
				m_npc->FireProjectileAtObject(m_npc->m_player);
			}
		}

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
			else if (npcInView)
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

		if (mTimeUntilRandomlyJumpOrRoll > 0.0f)
		{
			mTimeUntilRandomlyJumpOrRoll -= delta;
		}

		// let's just randomly jump
		if (m_npc->IsOnSolidSurface())
		{
			if (mTimeUntilRandomlyJumpOrRoll <= 0.0f)
			{
				// N% chance to roll
				int randRoll = rand() % 100;
				if (randRoll < 81)
				{
					m_npc->Roll();
				}
				else
				{
					if (m_npc->Jump(100.0f))
					{
						m_npc->SetVelocityY(0.5f);
						mTimeUntilRandomlyJumpOrRoll = kJumpOrRollRandomDelayMin + (rand() % (int)((kJumpOrRollRandomDelayMax - kJumpOrRollRandomDelayMin) * 100.0f)) * 0.01f;
					}
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

	if (mTimeUntilCanTeleport > 0.0f)
	{
		mTimeUntilCanTeleport -= delta;
	}
}

bool AIStateRangeAttack::CanAccelerateX(float direction)
{
	GAME_ASSERT(std::abs(direction) == 1.0f);

	auto cam = Camera2D::GetInstance();

	if (cam->IsObjectInView(m_npc))
	{
		// should be able to accelerate if in view
		return true;
	}

	if (direction < 0.0f)
	{
		if (m_npc->m_position.X - 10.0f < cam->GetLeftLevelBounds())
		{
			return false;
		}
	}
	else
	{
		if (m_npc->m_position.X + 10.0f > cam->GetRightLevelBounds())
		{
			return false;
		}
	}
		
	return true;
}
