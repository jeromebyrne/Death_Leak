#include "precompiled.h"
#include "NPCManager.h"
#include "DrawUtilities.h"

NPCManager * NPCManager::mInstance = nullptr;

NPCManager::NPCManager(void)
{
}


NPCManager::~NPCManager(void)
{
}

NPCManager * NPCManager::Instance()
{
	if (!mInstance)
	{
		mInstance = new NPCManager();
	}

	return mInstance;
}

void NPCManager::AddNPC(NPC * npc)
{
	m_npcList.push_back(npc);
}

void NPCManager::Update()
{
	Camera2D * cam = Camera2D::GetInstance();
	for (NPC * npc : m_npcList)
	{
		for (NPC * otherNpc : m_npcList)
		{
			if (npc != otherNpc)
			{
				// push away from each other
				ResolveCollisions(npc, otherNpc);
			}
		}
	}

	for (NPC * npc : m_npcList)
	{
		npc->ClearNPCCollisionSet();
	}
}

void NPCManager::Draw()
{
	auto cam = Camera2D::GetInstance();

	float camLeft = cam->Left();
	float camRight = cam->Right();
	float camTop = cam->Top();
	float camBottom = cam->Bottom();

	for (NPC * npc : m_npcList)
	{
		// TODO: instead of doing bounds checks in lots of places we can calculate
		// at the start of the frame and set a flag if is in view
		if (!npc->IsPlayerEnemy() ||
			cam->IsObjectInView(npc))
		{
			continue;
		}

		Vector3 indicatorPos = Vector3(npc->X(), npc->Y(), 3);

		if (npc->Right() < camLeft)
		{
			indicatorPos.X = camLeft + 45;
		}
		else if (npc->Left() > camRight)
		{
			indicatorPos.X = camRight - 45;
		}

		if (npc->Bottom() > (camTop - 45.0f))
		{
			indicatorPos.Y = camTop - 90;
		}
		else if (npc->Top() < camBottom)
		{
			indicatorPos.Y = camBottom + 90;
		}

		// TODO: don't use the draw utilities as it's very slow
		DrawUtilities::DrawTexture(indicatorPos, Vector2(45, 45), "Media//skull_icon.png");
	}
}

void NPCManager::ResolveCollisions(NPC * npc, NPC * otherNPC)
{
	if (npc->GetCheckNPCOverlapCollisions() && otherNPC->GetCheckNPCOverlapCollisions())
	{
		if (!npc->IsOnSolidSurface())
		{
			otherNPC->AddToNPCResolvedList(npc);
			return;
		}

		bool colliding = Utilities::IsSolidSpriteInRectangle(npc, otherNPC->X(), otherNPC->Y(), otherNPC->CollisionDimensions().X * 3.0f, otherNPC->CollisionDimensions().Y);
		if (colliding && !npc->NPCCollisionSetContains(otherNPC))
		{
			// stop accelerating, this is enough to keep NPCs separated
			npc->StopXAccelerating();
			npc->SetVelocityX(0.0f);
			otherNPC->AddToNPCResolvedList(npc);
		}
	}
}

void NPCManager::RemoveNPC(NPC * npc)
{
	m_npcList.remove(npc);
}

bool NPCManager::IsAnyEnemyNPCInWorld()
{
	for (auto e : m_npcList)
	{
		if (e->IsPlayerEnemy())
		{
			return true;
		}
	}

	return false;
}
