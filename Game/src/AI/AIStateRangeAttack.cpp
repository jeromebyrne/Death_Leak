#include "precompiled.h"
#include "AIStateRangeAttack.h"
#include "NPC.h"

static const float kRunAwayDelay = 0.5f;
static const float kJumpOrRollRandomDelayMin = 2.0f;
static const float kJumpOrRollRandomDelayMax = 6.5f;
static const float kTeleportDelayMin = 10.0f;
static const float kTeleportDelayMax = 7.0f;
static const float kTeleportDistance = 3000.0f;

AIStateRangeAttack::AIStateRangeAttack(NPC * npc) :
	AIState(npc),
	mDesiredRange(200.0f),
	mFollowRange(500.0f),
	mRandOffset(0.0f),
	mLastTimeRanAway(-9999.0f),
	mTimeUntilRandomlyJumpOrRoll(0.0f),
	mTimeUntilCanTeleport(0.0f)
{
	mStateType = kRangeAttack;

	mTimeUntilCanTeleport = kTeleportDelayMin + (rand() % (int)((kTeleportDelayMax - kTeleportDelayMin) * 100.0f)) * 0.01f;
}

AIStateRangeAttack::~AIStateRangeAttack(void)
{
}

void AIStateRangeAttack::OnTransition()
{
	/*
	float randMaxXVelocity = rand() % 800;
	randMaxXVelocity *= 0.001f;
	randMaxXVelocity += 1.0f;

	m_npc->SetMaxVelocityXY(randMaxXVelocity, 99999.0f);
	*/

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
		m_npc->SetVelocityX(0.0f);
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

		Vector2 distanceSquaredVector = m_npc->m_player->Position() - m_npc->Position();

		if (npcInView && 
			std::abs(distanceSquaredVector.X > kTeleportDistance) ||
				(std::abs(distanceSquaredVector.Y > 500.0f) && 
				m_npc->m_player->IsOnSolidSurface() && 
				m_npc->m_player->GetTimeOnSolidSurface() > 1.0f))
		{
			if (mTimeUntilCanTeleport <= 0.0f)
			{
				TeleportBehindPlayer();

				mLastTimeRanAway = 0.0f;
				mTimeUntilCanTeleport = kTeleportDelayMin + (rand() % (int)((kTeleportDelayMax - kTeleportDelayMin) * 100.0f)) * 0.01f;
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

				mTimeStoodStill = 0.0f;
			}
			else if (distanceSquared > (mFollowRange + mRandOffset) * (mFollowRange + mRandOffset))
			{
				distanceSquaredVector.Normalise();

				m_npc->AccelerateX(distanceSquaredVector.X);

				mTimeStoodStill = 0.0f;
			}
			else if (npcInView)
			{
				m_npc->StopXAccelerating();
				mLastTimeRanAway = currentTime;
			}
		}
		else
		{
			m_npc->StopXAccelerating();
			m_npc->SetVelocityX(m_npc->GetVelocity().X * 0.75f);

			auto player = GameObjectManager::Instance()->GetPlayer();
			GAME_ASSERT(player);
			m_npc->FireProjectileAtObject(player);
			mTimeStoodStill += delta;

			if (player->X() < m_npc->X())
			{
				m_npc->FlipHorizontal();
			}
			else
			{
				m_npc->UnFlipHorizontal();
			}
		}

		if (mTimeStoodStill > 3.0f)
		{
			TeleportBehindPlayer();

			mTimeStoodStill = 0.0f;
		}
		if (mTimeUntilRandomlyJumpOrRoll > 0.0f)
		{
			mTimeUntilRandomlyJumpOrRoll -= delta;
		}
		else if (m_npc->IsOnSolidSurface() && m_npc->GetAccelX() > 0.0f)
		{
			if (mTimeUntilRandomlyJumpOrRoll <= 0.0f)
			{
				// if the NPC is below the player then jump up
				if (m_npc->m_player->Y() > (m_npc->Y() + 50.0f))
				{
					if (m_npc->Jump(100.0f))
					{
						m_npc->SetVelocityY(0.5f);
					}
				}
				else
				{
					m_npc->Roll();
				}

				mTimeUntilRandomlyJumpOrRoll = kJumpOrRollRandomDelayMin + (rand() % (int)((kJumpOrRollRandomDelayMax - kJumpOrRollRandomDelayMin) * 100.0f)) * 0.01f;
			}
		}
	}

	if (mTimeUntilCanTeleport > 0.0f)
	{
		mTimeUntilCanTeleport -= delta;
	}
}

void AIStateRangeAttack::TeleportBehindPlayer()
{
	float playerX = m_npc->m_player->X();
	float pOffsetBehind = ((rand() % 300) + 200.0f) * m_npc->m_player->DirectionX();
	m_npc->Teleport(playerX - pOffsetBehind, m_npc->m_player->Position().Y + ((rand() % 200) + 200.0f), true);
	m_npc->FireProjectileAtObject(m_npc->m_player);

}

bool AIStateRangeAttack::CanAccelerateX(float direction)
{
	GAME_ASSERT(std::abs(direction) == 1.0f);

	if (m_npc->IsCrouching())
	{
		return false;
	}

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
