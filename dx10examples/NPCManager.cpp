#include "precompiled.h"
#include "NPCManager.h"

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
		if (cam->IsObjectInView(npc))
		{
			for (NPC * otherNpc : m_npcList)
			{
				if (npc != otherNpc && cam->IsObjectInView(otherNpc))
				{
					// push away from each other
					ResolveCollisions(npc, otherNpc);
				}
			}
		}
	}

	for (NPC * npc : m_npcList)
	{
		npc->ClearNPCCollisionSet();
	}
}

void NPCManager::ResolveCollisions(NPC * npc, NPC * otherNPC)
{
	if (npc->GetCheckNPCOverlapCollisions() && otherNPC->GetCheckNPCOverlapCollisions())
	{
		bool colliding = Utilities::IsSolidSpriteInRectangle(npc, otherNPC->X(), otherNPC->Y(), otherNPC->CollisionDimensions().X, otherNPC->CollisionDimensions().Y);
		if (colliding && !npc->NPCCollisionSetContains(otherNPC))
		{
			// stop accelerating, this is enough to keep NPCs separated
			npc->StopXAccelerating();
			npc->SetVelocityX(0);
			otherNPC->AddToNPCResolvedList(npc);
		}
	}
}

void NPCManager::RemoveNPC(NPC * npc)
{
	m_npcList.remove(npc);
}
